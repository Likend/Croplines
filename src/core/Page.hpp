#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

#include <wx/wx.h>

#include "core/DocumentData.hpp"

namespace croplines {

class Document;

class Page {
   public:
    ~Page() { m_image.Destroy(); }

    [[nodiscard]] Document&                    GetDocument() const { return m_doc; }
    [[nodiscard]] DocumentConfig&              GetConfig() const;
    [[nodiscard]] const std::filesystem::path& GetImagePath() const { return m_pageData.path; }
    [[nodiscard]] const std::set<int>& GetCropLines() const { return m_pageData.crop_lines; }
    wxImage&                           GetImage() { return m_image; }

    const std::vector<wxRect>& getSelectAreas() {
        if (m_modified) CalculateSelectAreas();
        return m_selectAreas;
    }

    bool InsertLine(int line);
    bool EraseLine(int line);

    bool SaveCrops();

    [[nodiscard]] std::optional<int> SearchNearestLine(int searchPosition, int threshold) const;

   private:
    friend class Document;

    Page(Document& doc, PageData& data) : m_doc(doc), m_pageData(data) { LoadImageFromFile(); }

    bool m_modified = true;  // 用来提示 CalculateSelectAreas 的惰性求值

    Document& m_doc;
    PageData& m_pageData;

    std::vector<wxRect> m_selectAreas;
    wxImage             m_image;

    void LoadImageFromFile() { m_image.LoadFile(wxString(GetImagePath())); }
    void CalculateSelectAreas();
};
}  // namespace croplines
