#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <stack>
#include <utility>
#include <variant>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/macros.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/vector.hpp>
#include <opencv2/opencv.hpp>
#include <wx/wx.h>

namespace Croplines {
constexpr const char* VALID_EXTENSION[] = {".png",  ".jpg", ".jpeg", ".bmp",
                                           ".tiff", ".tif", ".webp"};
constexpr const char* PROJECT_FILE_NAME = "croplines.json";
constexpr const char* DEFAULT_OUTPUT_DIR = "out";

class Prj;

template <typename T>
concept ActionRecord = requires(T record, Prj& prj) {
    { record.Do(prj) } -> std::same_as<bool>;
    { record.Undo(prj) } -> std::same_as<void>;
};

class Prj {
   public:
    class Page {
        using u32 = std::uint32_t;
        friend class Prj;
        std::set<u32> crop_lines;  // 从小到大排序
        std::vector<wxRect> select_area;
        bool modified = true;
        wxImage image;  // can be empty
       public:
        std::filesystem::path image_path;

        Page(std::filesystem::path image_path)
            : image_path(std::move(image_path)) {}
        Page() = default;

        bool IsLoaded() const { return image.IsOk(); };
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

    class InsertLineRecord {
        std::uint32_t line;
        std::reference_wrapper<Page> page;

       public:
        InsertLineRecord(std::uint32_t line, Page& page)
            : line(line), page(page) {}
        bool Do(Prj& prj);
        void Undo(Prj& prj);
    };

    class EraseLineRecord {
        using SetIteratorType = decltype(Page::crop_lines)::iterator;
        SetIteratorType it;
        std::reference_wrapper<Page> page;
        std::uint32_t line;

       public:
        EraseLineRecord(SetIteratorType it, Page& page)
            : it(it), page(page), line(*it) {}
        bool Do(Prj& prj);
        void Undo(Prj& prj);
    };

    using Action = std::variant<InsertLineRecord, EraseLineRecord>;

   private:
    std::vector<Page> pages;
    std::vector<std::reference_wrapper<Page>> pages_sorted;
    std::filesystem::path cwd;  // current working directory
    bool is_change;

    std::stack<Action> undo_stack;
    std::stack<Action> redo_stack;

   public:
    wxMenuBar* menubar = nullptr;

   public:
    static std::optional<Prj> Load(const std::filesystem::path& path);
    void Save();
    std::vector<std::reference_wrapper<Page>> GetPages() const {
        return pages_sorted;
    }
    inline bool IsChange() { return is_change; }
    inline void Change() { is_change = true; }

    wxImage LoadPage(Page& page);
    bool SaveCrops(Page& page);
    const std::vector<wxRect>& GetSelectArea(Page& page);

    template <ActionRecord A>
    void Execute(A action) {
        bool success = action.Do(*this);
        if (success) {
            is_change = true;
            undo_stack.push(action);
            redo_stack = std::stack<Action>();
            if (menubar) {
                menubar->Enable(wxID_UNDO, true);
                menubar->Enable(wxID_REDO, false);
            }
        }
    }
    bool CanUndo() const { return !undo_stack.empty(); }
    bool CanRedo() const { return !redo_stack.empty(); }
    void Undo();
    void Redo();

    template <class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(config));
        archive(CEREAL_NVP(pages));
    }

   private:
    void Initialize();
};

}  // namespace Croplines

// Add support for cereal serializing std::filesystem::path
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
