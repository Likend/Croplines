#include "prj.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <fstream>
#include <ranges>
#include <string_view>

#include <cereal/archives/json.hpp>
#include <wx/mstream.h>

using namespace Croplines;

namespace fs = std::filesystem;

using Page = Prj::Page;

wxImage Prj::LoadPage(Page& page) {
    if (page.IsLoaded()) return page.image;
    page.image.LoadFile(wxString(page.image_path));
    return page.image;
}

bool Prj::SaveCrops(Page& page) {
    // if (!page.IsLoaded()) LoadPage(page);
    wxImage image = LoadPage(page);
    std::size_t count = 1;
    fs::create_directories(config.output_dir);
    for (wxRect area : GetSelectArea(page)) {
        wxImage sub_image = image.GetSubImage(area);
        wxSize border_size = wxSize{static_cast<int>(config.border),
                                    static_cast<int>(config.border)};
        wxBitmap bitmap(area.GetSize() + 2 * border_size);
        wxMemoryDC memDC;
        memDC.SelectObject(bitmap);
        memDC.SetBrush(*wxWHITE_BRUSH);
        memDC.DrawRectangle(wxPoint{}, bitmap.GetSize());
        memDC.DrawBitmap(wxBitmap(sub_image), wxPoint{} + border_size);

        fs::path file_path =
            config.output_dir /
            std::format("{}-{}.{}", page.image_path.stem().string(), count,
                        page.image_path.extension().string());
        bitmap.SaveFile(wxString(file_path), image.GetType());
        count++;
    }
    return true;
}

/*自动选择黑色像素区域
    filter_noise_size: 忽略黑像素的大小
    expand_size: 留边空白大小
*/
static std::optional<wxRect> CalcuateSelectArea(cv::Mat image,
                                                std::uint32_t filter_noise_size,
                                                std::uint32_t expand_size,
                                                std::uint32_t base_line) {
    cv::Mat img_dst;
    cv::cvtColor(image, img_dst, cv::COLOR_RGB2GRAY);
    cv::threshold(img_dst, img_dst, 0, 255,
                  cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img_dst, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_NONE);
    int x_min = INT_MAX, x_max = INT_MIN, y_min = INT_MAX, y_max = INT_MIN;
    bool has_point = false;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) >= filter_noise_size) {
            int x_min_inner = x_min, x_max_inner = x_max, y_min_inner = y_min,
                y_max_inner = y_max;
            for (auto& point : contour) {
                if (point.x == 0 || point.x == image.cols - 1 || point.y == 0 ||
                    point.y == image.rows - 1) {
                    goto skip_contour;
                }
                if (point.x < x_min_inner) x_min_inner = point.x;
                if (point.x > x_max_inner) x_max_inner = point.x;
                if (point.y < y_min_inner) y_min_inner = point.y;
                if (point.y > y_max_inner) y_max_inner = point.y;
            }
            x_min = x_min_inner;
            x_max = x_max_inner;
            y_min = y_min_inner;
            y_max = y_max_inner;
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

const std::vector<wxRect>& Prj::GetSelectArea(Page& page) {
    if (page.modified) {
        page.modified = false;
        page.select_area.clear();
        wxImage ii = LoadPage(page);
        cv::Mat image(ii.GetHeight(), ii.GetWidth(), CV_8UC3,
                      static_cast<void*>(ii.GetData()));
        std::uint32_t prev_line = 0;
        for (std::int32_t line : page.crop_lines) {
            cv::Mat sub_image = image.rowRange(prev_line, line);
            auto area = CalcuateSelectArea(sub_image, config.filter_noise_size,
                                           0, prev_line);
            if (area) page.select_area.push_back(*area);
            prev_line = line;
        }
        std::int32_t line = image.rows;
        cv::Mat sub_image = image.rowRange(prev_line, line);
        auto area = CalcuateSelectArea(sub_image, config.filter_noise_size, 0,
                                       prev_line);
        if (area) page.select_area.push_back(*area);
    }
    return page.select_area;
}

std::optional<std::set<std::uint32_t>::iterator> Page::SearchNearestLine(
    std::uint32_t key, std::uint32_t limit) const {
    if (crop_lines.empty()) return std::nullopt;

    auto it1 = std::lower_bound(crop_lines.begin(), crop_lines.end(), key);
    if (it1 == crop_lines.begin()) {
        std::uint32_t d = *it1 - key;
        if (d < limit) return it1;
    }
    if (it1 == crop_lines.end()) {
        --it1;
        std::uint32_t d = key - *it1;
        if (d < limit) return it1;
    }
    std::uint32_t d1 = *it1 - key;
    auto it2 = it1;
    --it2;
    std::uint32_t d2 = key - *it2;
    if (d1 < d2 && d1 < limit) return it1;
    if (d2 < d1 && d2 < limit) return it2;

    return std::nullopt;
}

bool Prj::InsertLineRecord::Do(Prj& prj) {
    auto [__, actual_modified] = page.get().crop_lines.insert(line);
    page.get().modified = true;
    return actual_modified;
}

void Prj::InsertLineRecord::Undo(Prj& prj) {
    page.get().crop_lines.erase(line);
    page.get().modified = true;
}

bool Prj::EraseLineRecord::Do(Prj& prj) {
    page.get().crop_lines.erase(it);
    page.get().modified = true;
    return true;
}

void Prj::EraseLineRecord::Undo(Prj& prj) {
    page.get().crop_lines.insert(line);
    page.get().modified = true;
}

struct UndoVisitor {
    std::reference_wrapper<Prj> prj;

    template <ActionRecord A>
    void operator()(A& action) {
        action.Undo(prj);
    }
};

struct RedoVisitor {
    std::reference_wrapper<Prj> prj;

    template <ActionRecord A>
    void operator()(A& action) {
        action.Do(prj);
    }
};

void Prj::Undo() {
    if (!undo_stack.empty()) {
        auto action = undo_stack.top();
        undo_stack.pop();
        std::visit(UndoVisitor(*this), action);
        redo_stack.push(action);
    }
}

void Prj::Redo() {
    if (!redo_stack.empty()) {
        auto action = redo_stack.top();
        redo_stack.pop();
        std::visit(RedoVisitor(*this), action);
        undo_stack.push(action);
    }
}

static std::strong_ordering NaturalCompare(std::string_view a,
                                           std::string_view b) {
    for (const char *i1 = a.begin(), *i2 = b.begin();
         i1 != a.end() && i2 != b.end(); ++i1, ++i2) {
        if (std::isdigit(*i1) && std::isdigit(*i2)) {
            const char *ii1 = i1, *ii2 = i2;
            while (ii1 != a.end() && *ii1 == '0') ++ii1;
            while (ii2 != b.end() && *ii2 == '0') ++ii2;
            auto zero_count1 = std::distance(i1, ii1);
            auto zero_count2 = std::distance(i2, ii2);

            i1 = ii1;
            i2 = ii2;
            while (ii1 != a.end() && std::isdigit(*ii1)) ++ii1;
            while (ii2 != b.end() && std::isdigit(*ii2)) ++ii2;
            auto num1 = std::string_view(i1, ii1 - i1);
            auto num2 = std::string_view(i2, ii2 - i2);

            if (auto cmp = num1.length() <=> num2.length(); cmp != 0)
                return cmp;
            if (auto cmp = num1 <=> num2; cmp != 0) return cmp;
            if (auto cmp = zero_count1 <=> zero_count2; cmp != 0) return cmp;
        } else {
            return std::toupper(*i1) <=> std::toupper(*i2);
        }
    }
    return std::strong_ordering::equal;
}

static std::vector<std::reference_wrapper<Page>> SortPages(
    std::vector<Page>& pages) {
    std::vector<std::reference_wrapper<Page>> pages_sorted;
    pages_sorted.reserve(pages.size());
    for (Page& page : pages) {
        pages_sorted.push_back(std::ref(page));
    }
    // natural sort
    std::ranges::sort(
        pages_sorted,
        [](std::string_view s1, std::string_view s2) {
            return NaturalCompare(s1, s2) < 0;
        },
        [](const Page& page) { return page.image_path.filename().string(); });
    return pages_sorted;
}

void Prj::Initialize() {
    // Initialize project data with default values
    config.output_dir = DEFAULT_OUTPUT_DIR;
    config.border = 10;
    config.filter_noise_size = 8;

    auto extension_filter = [](const fs::directory_entry& entry) {
        return entry.is_regular_file() &&
               std::find(std::begin(VALID_EXTENSION), std::end(VALID_EXTENSION),
                         entry.path().extension().string()) !=
                   std::end(VALID_EXTENSION);
    };
    for (auto const& dir_entry :
         fs::directory_iterator(cwd) | std::views::filter(extension_filter)) {
        Page page(fs::relative(dir_entry.path(), cwd));
        pages.push_back(page);
    }
    is_change = true;
}

std::optional<Prj> Prj::Load(const fs::path& path) {
    if (!fs::exists(path)) return std::nullopt;
    fs::current_path(path);
    Prj prj;
    prj.cwd = path;
    fs::path prj_path = path / PROJECT_FILE_NAME;
    if (fs::exists(prj_path) && fs::is_regular_file(prj_path)) {
        std::ifstream file(prj_path);
        if (file) {
            cereal::JSONInputArchive archive(file);
            archive(cereal::make_nvp("prj", prj));
            prj.is_change = false;
        } else {
            prj.Initialize();
        }
    } else {
        prj.Initialize();
    }
    prj.pages_sorted = SortPages(prj.pages);
    return prj;
}

void Prj::Save() {
    fs::path prj_path = cwd / PROJECT_FILE_NAME;
    std::ofstream file(prj_path);
    cereal::JSONOutputArchive archive(file);
    archive(cereal::make_nvp("prj", *this));
    is_change = false;
}
