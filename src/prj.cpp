#include "prj.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <filesystem>
#include <fstream>
#include <ios>
#include <ranges>
#include <string_view>
#include <vector>

#include <opencv2/imgcodecs.hpp>

using namespace Croplines;

namespace fs = std::filesystem;

cv::Mat Prj::LoadPage(Page& page) {
    if (page.IsLoaded()) return page.image;

    std::ifstream file(page.image_path,
                       std::ios_base::binary | std::ios_base::in);
    if (!file) return {};

    file.seekg(0, std::ios_base::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios_base::beg);

    if (size <= 0) return {};
    std::vector<unsigned char> img_data(size);
    if (!file.read(reinterpret_cast<char*>(img_data.data()), size)) return {};

    return page.image = cv::imdecode(img_data, cv::IMREAD_COLOR_RGB);
}

bool Prj::SaveCrops(Page& page) {
    // if (!page.IsLoaded()) LoadPage(page);
    cv::Mat image = LoadPage(page);
    std::size_t count = 1;
    fs::path output_dir = cwd / config.output_dir;
    fs::create_directories(output_dir);
    for (Area area : GetSelectArea(page)) {
        cv::Mat sub_image = image({area.t, area.b}, {area.l, area.r});
        cv::copyMakeBorder(sub_image, sub_image, config.border, config.border,
                           config.border, config.border, cv::BORDER_CONSTANT,
                           {255, 255, 255});
        std::vector<std::uint8_t> buf;
        cv::imencode(page.image_path.extension().string(), sub_image, buf);
        std::string new_file_name =
            std::format("{}-{}.{}", page.image_path.stem().string(), count,
                        page.image_path.extension().string());
        fs::path file_path = cwd / config.output_dir / new_file_name;
        std::ofstream file(file_path,
                           std::ios_base::binary | std::ios_base::out);
        if (!file) return false;
        file.write(reinterpret_cast<char*>(buf.data()), buf.size());
        count++;
    }
    return true;
}

/*自动选择黑色像素区域
    filter_noise_size: 忽略黑像素的大小
    expand_size: 留边空白大小
*/
static std::optional<Area> CalcuateSelectArea(cv::Mat image,
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
            for (auto& point : contour) {
                if (point.x < x_min) x_min = point.x;
                if (point.x > x_max) x_max = point.x;
                if (point.y < y_min) y_min = point.y;
                if (point.y > y_max) y_max = point.y;
            }
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
    return Area{l, t, r, b};
}

const std::vector<Area>& Prj::GetSelectArea(Page& page) {
    if (page.modified) {
        page.modified = false;
        page.select_area.clear();
        cv::Mat image = LoadPage(page);
        std::uint32_t prev_line = 0;
        for (std::int32_t line : page.crop_lines) {
            cv::Mat sub_image = image.rowRange(prev_line, line);
            // cv::imshow("sub", sub_image);
            auto area = CalcuateSelectArea(sub_image, config.filter_noise_size,
                                           0, prev_line);
            if (area) page.select_area.push_back(*area);
            prev_line = line;
        }
        std::int32_t line = image.rows;
        cv::Mat sub_image = image.rowRange(prev_line, line);
        auto area = CalcuateSelectArea(sub_image, config.filter_noise_size,
                                       config.border, prev_line);
        if (area) page.select_area.push_back(*area);
    }
    return page.select_area;
}

std::optional<std::set<std::uint32_t>::iterator> Prj::Page::SearchNearestLine(
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
}

std::optional<Prj> Prj::Load(const fs::path& path) {
    Prj prj;
    if (!fs::exists(path)) {
        return std::nullopt;
    }
    fs::current_path(path);
    prj.cwd = path;
    fs::path prj_path = path / PROJECT_FILE_NAME;
    if (!fs::exists(prj_path) || !fs::is_regular_file(prj_path)) {
        prj.Initialize();
        return prj;
    }

    // Read the project file and populate `data`
    std::ifstream file(prj_path);
    if (!file.is_open()) {
        prj.Initialize();
        prj.is_change = true;
        return prj;
    }
    // TODO

    return prj;
}

void Prj::Save() const {
    fs::path prj_path = cwd / PROJECT_FILE_NAME;
    // std::ofstream
    // TODO

    if (!fs::exists(config.output_dir)) {
        fs::create_directory(config.output_dir);
    }
}
