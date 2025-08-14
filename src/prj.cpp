#include "prj.h"

// include std
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <ios>
#include <ranges>
#include <string_view>
// include cv
#include <opencv2/imgcodecs.hpp>

using namespace Croplines;

namespace fs = std::filesystem;

cv::Mat Prj::Page::Load() const {
    std::ifstream image(image_path, std::ios_base::binary | std::ios_base::in);
    if (!image) return {};

    image.seekg(0, std::ios_base::end);
    std::streamsize size = image.tellg();
    image.seekg(0, std::ios_base::beg);

    if (size <= 0) return {};
    std::vector<unsigned char> img_data(size);
    if (!image.read(reinterpret_cast<char*>(img_data.data()), size)) return {};

    return cv::imdecode(img_data, cv::IMREAD_COLOR_RGB);
}

void Prj::Page::SaveCrops() const {
    // TODO
}

std::optional<std::set<std::uint32_t>::iterator> Prj::Page::SearchNearestLine(
    std::uint32_t key, std::uint32_t limit) {
    auto it1 = std::lower_bound(crop_lines.begin(), crop_lines.end(), key);
    std::uint32_t d1 = *it1 - key;
    auto it2 = it1;
    --it2;
    std::uint32_t d2 = key - *it2;
    if (d1 < d2 && d1 < limit) {
        return it1;
    }
    if (d2 < d1 && d2 < limit) {
        return it2;
    }
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
        Page page{
            .image_path = fs::relative(dir_entry.path(), cwd),
        };
        pages.push_back(page);
    }
    // natural sort
    std::ranges::sort(
        pages,
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
