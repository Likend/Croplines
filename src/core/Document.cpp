#include "core/Document.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <ranges>

#include <cereal/archives/json.hpp>

#include "core/DocumentData.hpp"
#include "utils/Asserts.hpp"
#include "utils/Compare.hpp"

using namespace Croplines;
namespace fs = std::filesystem;

bool Document::Load(const fs::path& path) {
    cwd = path;

    if (!fs::exists(path)) return false;
    fs::current_path(path);

    m_data.emplace();

    fs::path prj_path = path / PROJECT_FILE_NAME;
    if (fs::exists(prj_path) && fs::is_regular_file(prj_path)) {
        std::ifstream file(prj_path);
        if (file) {
            cereal::JSONInputArchive archive(file);
            archive(cereal::make_nvp("prj", GetData()));
            m_modified = false;
            return true;
        }
    }
    InitializeEmptyProject();
    return true;
}

void Document::InitializeEmptyProject() {
    auto extensionFilter = [](const fs::directory_entry& entry) {
        return entry.is_regular_file() &&
               std::ranges::find(VALID_EXTENSION, entry.path().extension().string()) !=
                   std::end(VALID_EXTENSION);
    };
    for (auto const& entry : fs::directory_iterator(cwd) | std::views::filter(extensionFilter)) {
        fs::path relativePath = fs::relative(entry.path(), cwd);
        m_data->pages.push_back(std::make_unique<PageData>(relativePath));
    }

    // sort pages
    auto pred = [](const std::unique_ptr<PageData>& pageData) {
        return pageData->path.filename().string();
    };
    std::ranges::sort(
        m_data->pages,
        [](std::string_view s1, std::string_view s2) { return NaturalCompare(s1, s2) < 0; }, pred);

    SetModified();
}

bool Document::Save() {
    ASSERT_WITH(IsLoad(), "Project not loaded!");
    fs::path prj_path = cwd / PROJECT_FILE_NAME;

    std::ofstream             file(prj_path);
    cereal::JSONOutputArchive archive(file);
    archive(cereal::make_nvp("prj", GetData()));
    m_modified = false;
    return true;
}

bool Document::Close() {
    ASSERT_WITH(IsLoad(), "Project not loaded!");
    m_modified = false;
    m_data.reset();
    m_processor->ClearCommands();
    return true;
}

void Document::SetModified(bool modified) { m_modified = modified; }

Page Document::LoadPage(size_t index) {
    ASSERT_WITH(index < PagesSize(), "indx out of range");
    auto& pageData = *m_data->pages[index];
    return Page{*this, pageData};
}

bool Document::SaveAllCrops() {
    ASSERT_WITH(IsLoad(), "Project not loaded!");
    bool success = true;
    for (size_t i = 0; i < PagesSize(); i++) {
        Page page = LoadPage(i);
        success &= page.SaveCrops();
    }
    return success;
}
