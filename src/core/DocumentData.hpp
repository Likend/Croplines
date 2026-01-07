#pragma once

#include <filesystem>
#include <memory>
#include <set>

#include <cereal/cereal.hpp>
#include <cereal/details/helpers.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/vector.hpp>

namespace croplines {
struct DocumentConfig {
    constexpr static const char* DEFAULT_OUTPUT_DIR = "out";

    std::filesystem::path output_dir        = DEFAULT_OUTPUT_DIR;
    int                   border            = 10;
    int                   filter_noise_size = 8;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("border", border));
        archive(cereal::make_nvp("filter_noise_size", filter_noise_size));
        archive(cereal::make_nvp("output_dir", output_dir));
    }
};

struct PageData {
    std::filesystem::path path;
    std::set<int>         crop_lines;

    PageData() = default;
    PageData(std::filesystem::path path) : path(std::move(path)) {}

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("image_path", path));
        archive(cereal::make_nvp("crop_lines", crop_lines));
    }
};

struct DocumentData {
    DocumentConfig config;

    std::vector<std::unique_ptr<PageData>> pages;

    struct PagesProxy {
        std::vector<std::unique_ptr<PageData>>& pages;

        template <class Archive>
        void save(Archive& ar) const {
            ar(cereal::make_size_tag(static_cast<cereal::size_type>(pages.size())));
            for (auto const& p : pages) {
                if (p) ar(*p);
            }
        }

        template <class Archive>
        void load(Archive& ar) {
            cereal::size_type size;
            ar(cereal::make_size_tag(size));
            pages.clear();
            pages.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                auto p = std::make_unique<PageData>();
                ar(*p);
                pages.push_back(std::move(p));
            }
        }
    };

    template <class Archive>
    void serialize(Archive& ar) {
        ar(cereal::make_nvp("config", config));
        ar(cereal::make_nvp("pages", PagesProxy{pages}));
    }
};
}  // namespace croplines

// Add support for cereal serializing std::filesystem::path
namespace std {
namespace filesystem {
template <class Archive>
inline void load_minimal(const Archive&, std::filesystem::path& path, const std::string& name) {
    path = name;
}

template <class Archive>
inline std::string save_minimal(const Archive&, const std::filesystem::path& path) {
    return path.string();
}
}  // namespace filesystem
}  // namespace std
