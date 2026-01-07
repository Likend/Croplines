#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>

#include <wx/cmdproc.h>
#include <wx/docview.h>
#include <wx/xrc/xmlres.h>

#include "core/DocumentData.hpp"
#include "core/Event.hpp"
#include "core/Page.hpp"

namespace croplines {

class Document {
   public:
    constexpr static const char* VALID_EXTENSION[] = {".png",  ".jpg", ".jpeg", ".bmp",
                                                      ".tiff", ".tif", ".webp"};
    constexpr static const char* PROJECT_FILE_NAME = "croplines.json";

    bool               Load(const std::filesystem::path& path);
    bool               Save();
    bool               Close();
    [[nodiscard]] bool IsLoad() const { return m_data.has_value(); };

    [[nodiscard]] const std::filesystem::path& GetPath() const { return cwd; }

    [[nodiscard]] wxCommandProcessor* GetProcessor() const { return m_processor; }

    [[nodiscard]] bool IsModified() const { return m_modified; }
    void               SetModified(bool modified = true);

    [[nodiscard]] size_t PagesSize() const { return m_data->pages.size(); };
    Page                 LoadPage(size_t index);

    bool SaveAllCrops();

    DocumentData&   GetData() { return m_data.value(); }
    DocumentConfig& GetConfig() { return GetData().config; }

   private:
    bool m_modified = false;

    std::optional<DocumentData> m_data;

    std::filesystem::path cwd;

    wxCommandProcessor* m_processor = new wxCommandProcessor();

    void InitializeEmptyProject();
};

}  // namespace croplines
