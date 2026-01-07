#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>

#include <wx/cmdproc.h>
#include <wx/datetime.h>
#include <wx/docview.h>
#include <wx/event.h>
#include <wx/xrc/xmlres.h>

#include "core/DocumentData.hpp"
#include "core/Event.hpp"
#include "core/Page.hpp"

namespace Croplines {

class Document {
   public:
    constexpr static const char* VALID_EXTENSION[] = {".png",  ".jpg", ".jpeg", ".bmp",
                                                      ".tiff", ".tif", ".webp"};
    constexpr static const char* PROJECT_FILE_NAME = "croplines.json";

    bool Load(const std::filesystem::path& path);
    bool Save();
    bool Close();
    bool isLoad() const { return m_data.has_value(); };

    const std::filesystem::path& GetPath() const { return cwd; }

    wxCommandProcessor* GetProcessor() const { return m_processor; }

    bool isModified() const { return m_modified; }
    void setModified(bool modified = true);

    size_t PagesSize() const { return m_data->pages.size(); };
    Page   LoadPage(size_t index);

    bool SaveAllCrops();

    DocumentData&   getData() { return m_data.value(); }
    DocumentConfig& getConfig() { return getData().config; }

   private:
    bool m_modified = false;

    std::optional<DocumentData> m_data;

    std::filesystem::path cwd;

    wxCommandProcessor* m_processor = new wxCommandProcessor();

    void InitializeEmptyProject();
};

}  // namespace Croplines
