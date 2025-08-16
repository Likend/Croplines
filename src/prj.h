#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/macros.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/vector.hpp>
#include <opencv2/opencv.hpp>
#include <wx/wx.h>

namespace Croplines {

using namespace std::string_literals;
constexpr const char* VALID_EXTENSION[] = {".png", ".jpg",  ".jpeg",
                                           ".bmp", ".tiff", ".tif"};
constexpr const char* PROJECT_FILE_NAME = "croplines.json";
constexpr const char* DEFAULT_OUTPUT_DIR = "out";

class Prj {
   public:
    class Page {
        using u32 = std::uint32_t;
        friend class Prj;

       public:
       private:
        std::set<u32> crop_lines;  // 从小到大排序
        std::vector<wxRect> select_area;
        bool modified = true;

        wxImage image;  // can be empty
        bool IsLoaded() const { return image.IsOk(); };

       public:
        std::filesystem::path image_path;

        Page(std::filesystem::path image_path)
            : image_path(std::move(image_path)) {}

        Page() = default;

        void Close() { image.Destroy(); }

        const std::set<u32> GetCropLines() const { return crop_lines; }
        std::optional<std::set<u32>::iterator> SearchNearestLine(
            u32 key, u32 limit) const;

        template <class Archive>
        void serialize(Archive& archive) {
            archive(cereal::make_nvp("image_path", image_path));
            archive(CEREAL_NVP(crop_lines));
        }
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

        template <class Archive>
        void serialize(Archive& archive) {
            archive(CEREAL_NVP(border));
            archive(CEREAL_NVP(filter_noise_size));
            archive(cereal::make_nvp("output_dir", output_dir));
        }
    } config;

   private:
    std::filesystem::path cwd;  // current working directory
    bool is_change;

   public:
    static std::optional<Prj> Load(const std::filesystem::path& path);
    void Save();
    inline bool IsChange() { return is_change; }
    inline void Change() { is_change = true; }

    wxImage LoadPage(Page& page);
    bool SaveCrops(Page& page);
    const std::vector<wxRect>& GetSelectArea(Page& page);
    void InsertLine(std::uint32_t line, Page& page) {
        page.crop_lines.insert(line);
        page.modified = true;
        is_change = true;
    }
    void EraseLine(decltype(Page::crop_lines)::iterator it, Page& page) {
        page.crop_lines.erase(it);
        page.modified = true;
        is_change = true;
    }

   private:
    void Initialize();

   public:
    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(config));
        archive(CEREAL_NVP(pages));
    }
};

}  // namespace Croplines

namespace std {
namespace filesystem {
template <class Archive>
inline void load_minimal(const Archive&, std::filesystem::path& path,
                         const std::string& name) {
    path = name;
}

template <class Archive>
inline std::string save_minimal(const Archive&,
                                const std::filesystem::path& path) {
    return path.string();
}
}  // namespace filesystem
}  // namespace std