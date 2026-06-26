#include "core/Page.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <generator>
#include <limits>
#include <optional>
#include <ranges>
#include <set>
#include <vector>

#include <wx/gdicmn.h>

#include "core/algorithm/Image.hpp"
#include "core/Document.hpp"
#include "core/DocumentData.hpp"

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

static std::optional<wxRect> CalculateSelectArea(const wxImage& image, int filter_noise_size,
                                                 int base_line) {
    auto gray_data = ConvertToGrayscale(image.GetData(), image.GetWidth(), image.GetHeight());

    auto binary_data = ThresholdOstu(gray_data.get(), image.GetWidth(), image.GetHeight());

    auto fill_areas =
        // 链式处理
        FloodFillArea(binary_data.get(), image.GetWidth(), image.GetHeight()) |
        // 忽略小块的黑像素
        std::views::filter(
            [filter_noise_size](auto area) { return area.area > filter_noise_size; }) |
        // 边缘触碰：如果靠边则忽略
        std::views::filter([&image](auto area) {
            const auto& r = area.range;
            return r.GetLeft() > 0 && r.GetRight() < image.GetWidth() - 1 &&  //
                   r.GetTop() > 0 && r.GetBottom() < image.GetHeight() - 1;
        });

    int  x_min = std::numeric_limits<int>::max(), x_max = std::numeric_limits<int>::min(),
         y_min = std::numeric_limits<int>::max(), y_max = std::numeric_limits<int>::min();
    bool has_point = false;
    for (const FillArea& fill_area : fill_areas) {
        x_min     = std::min(fill_area.range.GetLeft(), x_min);
        x_max     = std::max(fill_area.range.GetRight(), x_max);
        y_min     = std::min(fill_area.range.GetTop(), y_min);
        y_max     = std::max(fill_area.range.GetBottom(), y_max);
        has_point = true;
    }

    if (!has_point) return std::nullopt;
    return wxRect{wxPoint{x_min, y_min + base_line}, wxPoint{x_max, y_max + base_line}};
}

void Page::CalculateSelectAreas() {
    m_selectAreas.clear();
    assert(m_image.IsOk() && "Image not load!");

    auto concat_view = [&]() -> std::generator<int> {
        for (int x : GetCropLines()) co_yield x;
        co_yield m_image.GetHeight();
    };
    int prev_line = 0;
    for (int line : concat_view()) {
        wxImage subImage = m_image.GetSubImage(
            wxRect{wxPoint{0, prev_line}, wxSize{m_image.GetWidth(), line - prev_line}});
        auto area = CalculateSelectArea(subImage, GetConfig().filter_noise_size, prev_line);
        if (area) m_selectAreas.push_back(*area);
        prev_line = line;
    }

    m_modified = false;
}
