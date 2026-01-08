#include "core/Page.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <optional>
#include <set>
#include <vector>

#include <opencv2/opencv.hpp>

#include "core/Document.hpp"
#include "core/DocumentData.hpp"
#include "utils/Asserts.hpp"

using namespace croplines;
namespace fs = std::filesystem;

DocumentConfig& Page::GetConfig() const { return m_doc.GetConfig(); }

bool Page::InsertLine(int line) {
    auto [it, modified] = m_pageData.crop_lines.insert(line);
    m_modified |= modified;
    GetDocument().SetModified();
    return modified;
}

bool Page::EraseLine(int line) {
    bool modified = m_pageData.crop_lines.erase(line) != 0;
    m_modified |= modified;
    GetDocument().SetModified();
    return modified;
}

bool Page::SaveCrops() {
    if (!m_image.IsOk()) return false;

    std::size_t count = 1;
    fs::create_directories(GetConfig().output_dir);
    for (wxRect area : getSelectAreas()) {
        wxImage sub_image = m_image.GetSubImage(area);
        // TODO
        // wxSize border_size = wxSize{static_cast<int>(config.border),
        //                             static_cast<int>(config.border)};
        // wxBitmap bitmap(area.GetSize() + 2 * border_size);
        // wxMemoryDC memDC;
        // memDC.SelectObject(bitmap);
        // memDC.SetBrush(*wxWHITE_BRUSH);
        // memDC.DrawRectangle(wxPoint{}, bitmap.GetSize());
        // memDC.DrawBitmap(wxBitmap(sub_image), wxPoint{} + border_size);

        // optimize compress for tiff
        int sample_per_pixel = m_image.GetOptionInt(wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL);
        int bits_per_sample  = m_image.GetOptionInt(wxIMAGE_OPTION_TIFF_BITSPERSAMPLE);
        sub_image.SetOption(wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL, sample_per_pixel);
        sub_image.SetOption(wxIMAGE_OPTION_TIFF_BITSPERSAMPLE, bits_per_sample);
        if (sample_per_pixel == 1 && bits_per_sample == 1)
            sub_image.SetOption(wxIMAGE_OPTION_TIFF_COMPRESSION, 4);
        else
            sub_image.SetOption(wxIMAGE_OPTION_TIFF_COMPRESSION,
                                m_image.GetOptionInt(wxIMAGE_OPTION_TIFF_COMPRESSION));

        fs::path file_path =
            GetConfig().output_dir / std::format("{}-{}{}", GetImagePath().stem().string(), count,
                                                 GetImagePath().extension().string());
        // bitmap.SaveFile(wxString(file_path), image.GetType());
        sub_image.SaveFile(wxString(file_path), m_image.GetType());
        count++;
    }
    return true;
}

std::optional<int> Page::SearchNearestLine(int searchPosition, int threshold) const {
    const std::set<int>& cropLines = GetCropLines();
    if (cropLines.empty()) return std::nullopt;

    auto it1 = cropLines.lower_bound(searchPosition);
    if (it1 == cropLines.begin()) {
        int d = *it1 - searchPosition;
        if (d < threshold) return *it1;
    } else if (it1 == cropLines.end()) {
        --it1;
        int d = searchPosition - *it1;
        if (d < threshold) return *it1;
    } else {
        int  d1  = *it1 - searchPosition;
        auto it2 = it1;
        --it2;
        int d2 = searchPosition - *it2;
        if (d1 < d2 && d1 < threshold) return *it1;
        if (d2 < d1 && d2 < threshold) return *it2;
    }
    return std::nullopt;
}

/*自动选择黑色像素区域
    filter_noise_size: 忽略黑像素的大小
    expand_size: 留边空白大小
*/
static std::optional<wxRect> CalculateSelectArea(const cv::Mat& image, int filter_noise_size,
                                                 int expand_size, int base_line) {
    cv::Mat img_dst;
    cv::cvtColor(image, img_dst, cv::COLOR_RGB2GRAY);
    cv::threshold(img_dst, img_dst, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img_dst, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    int x_min = std::numeric_limits<int>::max(), x_max = std::numeric_limits<int>::min(),
        y_min = std::numeric_limits<int>::max(), y_max = std::numeric_limits<int>::min();
    bool has_point = false;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) >= filter_noise_size) {
            cv::Rect r = cv::boundingRect(contour);

            // 边缘触碰逻辑（如果靠边则忽略）
            if (r.x == 0 || r.y == 0 || (r.x + r.width) >= image.cols ||
                (r.y + r.height) >= image.rows) {
                continue;
            }
            // 更新全局最小/最大边界
            x_min     = std::min(x_min, r.x);
            y_min     = std::min(y_min, r.y);
            x_max     = std::max(x_max, r.x + r.width);
            y_max     = std::max(y_max, r.y + r.height);
            has_point = true;
        }
    }
    if (!has_point) return std::nullopt;

    int l = x_min - expand_size;
    int t = y_min - expand_size + base_line;
    int r = x_max + expand_size;
    int b = y_max + expand_size + base_line;
    if (l < 0) l = 0;
    if (t < 0) t = 0;
    if (r > image.cols) r = image.cols;
    if (b > image.rows + base_line) b = image.rows + base_line;
    return wxRect{wxPoint{l, t}, wxPoint{r, b}};
}

void Page::CalculateSelectAreas() {
    m_selectAreas.clear();
    ASSERT_WITH(m_image.IsOk(), "Image not load!");
    cv::Mat image(m_image.GetHeight(), m_image.GetWidth(), CV_8UC3,
                  static_cast<void*>(m_image.GetData()));

    int prev_line = 0;

    auto invokeCalculation = [this, &image](int line, int prev_line) {
        cv::Mat sub_image = image.rowRange(prev_line, line);
        auto    area = CalculateSelectArea(sub_image, GetConfig().filter_noise_size, 0, prev_line);
        if (area) m_selectAreas.push_back(*area);
    };

    for (int line : GetCropLines()) {
        invokeCalculation(line, prev_line);
        prev_line = line;
    }
    std::int32_t line = image.rows;
    invokeCalculation(line, prev_line);

    m_modified = false;
}
