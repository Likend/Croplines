#include "core/Page.hpp"

#include <optional>
#include <set>

#include <opencv2/opencv.hpp>

#include "core/Document.hpp"
#include "core/DocumentData.hpp"
#include "utils/Asserts.hpp"

using namespace Croplines;
namespace fs = std::filesystem;

DocumentConfig& Page::getConfig() const { return m_doc.getConfig(); }

bool Page::InsertLine(int line) {
    auto [it, modified] = m_pageData.crop_lines.insert(line);
    m_modified |= modified;
    getDocument().setModified();
    return modified;
}

bool Page::EraseLine(int line) {
    bool modified = m_pageData.crop_lines.erase(line) != 0;
    m_modified |= modified;
    getDocument().setModified();
    return modified;
}

bool Page::SaveCrops() {
    if (!m_image.IsOk()) return false;

    std::size_t count = 1;
    fs::create_directories(getConfig().output_dir);
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
            getConfig().output_dir / std::format("{}-{}{}", getImagePath().stem().string(), count,
                                                 getImagePath().extension().string());
        // bitmap.SaveFile(wxString(file_path), image.GetType());
        sub_image.SaveFile(wxString(file_path), m_image.GetType());
        count++;
    }
    return true;
}

std::optional<int> Page::SearchNearestLine(int searchPosition, int threshold) const {
    const std::set<int>& cropLines = getCropLines();
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
static std::optional<wxRect> CalculateSelectArea(cv::Mat image, int filter_noise_size,
                                                 int expand_size, int base_line) {
    cv::Mat img_dst;
    cv::cvtColor(image, img_dst, cv::COLOR_RGB2GRAY);
    cv::threshold(img_dst, img_dst, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img_dst, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    int  x_min = INT_MAX, x_max = INT_MIN, y_min = INT_MAX, y_max = INT_MIN;
    bool has_point = false;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) >= filter_noise_size) {
            int x_min_inner = x_min, x_max_inner = x_max, y_min_inner = y_min, y_max_inner = y_max;
            for (const auto& point : contour) {
                if (point.x == 0 || point.x == image.cols - 1 || point.y == 0 ||
                    point.y == image.rows - 1) {
                    goto skip_contour;
                }
                if (point.x < x_min_inner) x_min_inner = point.x;
                if (point.x > x_max_inner) x_max_inner = point.x;
                if (point.y < y_min_inner) y_min_inner = point.y;
                if (point.y > y_max_inner) y_max_inner = point.y;
            }
            x_min     = x_min_inner;
            x_max     = x_max_inner;
            y_min     = y_min_inner;
            y_max     = y_max_inner;
            has_point = true;
        skip_contour:
            (void)0;
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
        auto    area = CalculateSelectArea(sub_image, getConfig().filter_noise_size, 0, prev_line);
        if (area) m_selectAreas.push_back(*area);
    };

    for (int line : getCropLines()) {
        invokeCalculation(line, prev_line);
        prev_line = line;
    }
    std::int32_t line = image.rows;
    invokeCalculation(line, prev_line);

    m_modified = false;
}
