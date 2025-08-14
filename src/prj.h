#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

#include <opencv2/opencv.hpp>

namespace Croplines {

using namespace std::string_literals;
constexpr const char* VALID_EXTENSION[] = {".png", ".jpg",  ".jpeg",
                                           ".bmp", ".tiff", ".tif"};
constexpr const char* PROJECT_FILE_NAME = "croplines.cpln";
constexpr const char* DEFAULT_OUTPUT_DIR = "out";

class Prj final {
   public:
    struct Page {
        std::set<std::uint32_t> crop_lines;  // 从小到大排序
        struct Area {
            std::uint32_t l, t, r, b;
        };
        std::vector<Area> select_area;
        std::filesystem::path image_path;

        cv::Mat Load() const;
        void SaveCrops() const;
        std::optional<std::set<std::uint32_t>::iterator> SearchNearestLine(
            std::uint32_t key, std::uint32_t limit);
    };
    std::vector<Page> pages;

    struct Config {
        std::filesystem::path output_dir;
        std::uint32_t border;
        std::uint32_t filter_noise_size;
    } config;

   private:
    std::filesystem::path cwd;  // current working directory
    bool is_change;

   public:
    static std::optional<Prj> Load(const std::filesystem::path& path);
    void Save() const;
    inline bool IsChange() { return is_change; }

   private:
    void Initialize();
};
}  // namespace Croplines