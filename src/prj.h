#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

namespace Croplines {

using namespace std::string_literals;
constexpr const char* VALID_EXTENSION[] = {".png", ".jpg",  ".jpeg",
                                           ".bmp", ".tiff", ".tif"};
constexpr const char* PROJECT_FILE_NAME = "croplines.cpln";
constexpr const char* DEFAULT_OUTPUT_DIR = "out";

struct Area {
    int l, t, r, b;
};
class Prj {
   public:
    class Page {
        using u32 = std::uint32_t;
        friend class Prj;

       public:
       private:
        std::set<u32> crop_lines;  // 从小到大排序
        std::vector<Area> select_area;
        bool modified = true;

        cv::Mat image;  // can be empty
        bool IsLoaded() const { return !image.empty(); };

       public:
        const std::filesystem::path image_path;

        void Close() { image = cv::Mat{}; }

        const std::set<u32> GetCropLines() const { return crop_lines; }
        void InsertLine(u32 line) {
            crop_lines.insert(line);
            modified = true;
        }
        void EraseLine(decltype(crop_lines)::iterator it) {
            crop_lines.erase(it);
            modified = true;
        }
        std::optional<std::set<u32>::iterator> SearchNearestLine(
            u32 key, u32 limit) const;

       private:
        Page(std::filesystem::path image_path)
            : image_path(std::move(image_path)) {}
    };

   private:
    std::vector<Page> pages;
    std::vector<std::reference_wrapper<Page>> pages_sorted;

   public:
    std::vector<std::reference_wrapper<Page>> GetPages() const {
        return pages_sorted;
    }

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

    cv::Mat LoadPage(Page& page);
    bool SaveCrops(Page& page);
    const std::vector<Area>& GetSelectArea(Page& page);

   private:
    void Initialize();
};

}  // namespace Croplines