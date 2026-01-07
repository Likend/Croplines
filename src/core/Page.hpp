#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <vector>

#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/image.h>

#include "core/DocumentData.hpp"

namespace Croplines {

class Document;

class Page {
   public:
    ~Page() { m_image.Destroy(); }

    Document&                    getDocument() const { return m_doc; }
    DocumentConfig&              getConfig() const;
    const std::filesystem::path& getImagePath() const { return m_pageData.path; }
    const std::set<int>&         getCropLines() const { return m_pageData.crop_lines; }
    wxImage&                     getImage() { return m_image; }

    const std::vector<wxRect>& getSelectAreas() {
        if (m_modified) CalculateSelectAreas();
        return m_selectAreas;
    }

    bool InsertLine(int line);
    bool EraseLine(int line);

    bool SaveCrops();

    std::optional<int> SearchNearestLine(int searchPosition, int threshold) const;

   private:
    friend class Document;

    Page(Document& doc, PageData& data) : m_doc(doc), m_pageData(data) { LoadImage(); }

    bool m_modified = true; // 用来提示 CalculateSelectAreas 的惰性求值

    Document& m_doc;
    PageData& m_pageData;

    std::vector<wxRect> m_selectAreas;
    wxImage             m_image;

    void LoadImage() { m_image.LoadFile(wxString(getImagePath())); }
    void CalculateSelectAreas();
};
}  // namespace Croplines
