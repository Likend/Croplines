#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>

#include <wx/aui/framemanager.h>
#include <wx/wx.h>

#include "core/Document.hpp"
#include "core/Page.hpp"
#include "ui/Canvas.hpp"
#include "ui/ConfigPanel.hpp"
#include "ui/MenuBar.hpp"
#include "ui/ToolBar.hpp"

namespace croplines {
class MainFrame final : public wxFrame {
   public:
    MainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos,
              const wxSize& size);
    ~MainFrame() override { m_mgr.UnInit(); }

   private:
    wxAuiManager m_mgr;

    MenuBar*     m_menuBar = new MenuBar{};
    ToolBar*     m_toolBar;
    wxStatusBar* m_statusBar;
    Canvas*      m_canvas;
    wxListBox*   m_pageListPanel;
    ConfigPanel* m_configPanel;

    std::size_t         m_currentPageIdx = 0;
    Document            m_doc;
    std::optional<Page> m_currentPage;

    bool Load(const std::filesystem::path& path);
    bool Save();
    bool Close();

    [[nodiscard]] std::size_t CurrentPage() const { return m_currentPageIdx; }
    void                      CurrentPage(std::size_t);
    void                      PrevPage() { CurrentPage(CurrentPage() - 1); }
    void                      NextPage() { CurrentPage(CurrentPage() + 1); }

    void SyncUIPageListPanel();
    void SyncUIConfigPanel();

    void OnPrevPage(wxCommandEvent&) { PrevPage(); }
    void OnNextPage(wxCommandEvent&) { NextPage(); }
    void OnSave(wxCommandEvent&) { Save(); }
    void OnLoad(wxCommandEvent&);
    void OnUndo(wxCommandEvent&);
    void OnRedo(wxCommandEvent&);
    void OnZoomIn(wxCommandEvent&) { m_canvas->ZoomIn(); }
    void OnZoomOut(wxCommandEvent&) { m_canvas->ZoomOut(); }
    void OnZoomFit(wxCommandEvent&) { m_canvas->ZoomFit(); }
    void OnZoom100(wxCommandEvent&) { m_canvas->Zoom(1.0); }
    void OnCropCurrPage(wxCommandEvent&);
    void OnCropAllPage(wxCommandEvent&);
    void OnClose(wxCommandEvent&) { Close(); }
    void OnExit(wxCloseEvent&);
    void OnExit(wxCommandEvent&) { Close(); }
    void OnAbout(wxCommandEvent&);
    void OnClickListBox(wxCommandEvent&);
    void OnChnageCfgFilerPixSize(wxCommandEvent& event);
    void OnChangeCfgBorder(wxCommandEvent& event);

    // Update ui
    void OnUpdateUndo(wxUpdateUIEvent&);
    void OnUpdateRedo(wxUpdateUIEvent&);
    void OnUpdateSave(wxUpdateUIEvent&);

    wxDECLARE_EVENT_TABLE();
};
}  // namespace croplines
