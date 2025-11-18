#pragma once

#include <filesystem>
#include <optional>

#include <wx/event.h>
#include <wx/wx.h>

#include "wxUI.h"

namespace Croplines {

class MainWindow final : public MainUI {
   public:
    MainWindow(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = wxT("Crop Lines"), const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxSize(972, 651),
               long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~MainWindow() {};

    void EnableTools(bool state);
    void EnableConfigs(bool enable);
    void EnableMenu(bool enable);

    void Load(std::filesystem::path path);
    void Save() {
        if (prj) prj->Save();
    }
    bool ClosePrj();
    bool Exit();  // true if close succesfully; false if cancelled

    std::size_t CurrentPage() { return current_page; }
    void CurrentPage(std::size_t);
    void PrevPage();
    void NextPage();

   private:
    std::size_t current_page = 0;
    wxMenuBar* menubar;

   private:
    void ShowPage();

    void OnPrevPage([[maybe_unused]] wxCommandEvent& event) { PrevPage(); }
    void OnNextPage([[maybe_unused]] wxCommandEvent& event) { NextPage(); }
    void OnSave([[maybe_unused]] wxCommandEvent& event) { Save(); }
    void OnLoad(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnZoomIn([[maybe_unused]] wxCommandEvent& event) { canvas->ZoomIn(); }
    void OnZoomOut([[maybe_unused]] wxCommandEvent& event) { canvas->ZoomOut(); }
    void OnZoomFit([[maybe_unused]] wxCommandEvent& event) { canvas->ZoomFit(); }
    void OnZoom100([[maybe_unused]] wxCommandEvent& event) { canvas->Zoom(1.0); }
    void OnCropCurrPage(wxCommandEvent& event);
    void OnCropAllPage(wxCommandEvent& event);
    void OnClose([[maybe_unused]] wxCommandEvent& event) { ClosePrj(); }
    void OnExit(wxCloseEvent& event) {
        if (!Exit()) event.Veto();
    }
    void OnExit([[maybe_unused]] wxCommandEvent& event) { Close(); }
    void OnAbout(wxCommandEvent& event);
    void OnClickListBox(wxCommandEvent& event);
    void OnChnageCfgFilerPixSize(wxCommandEvent& event) {
        prj->config.filter_noise_size = event.GetInt();
        prj->Change();
    }
    void OnChangeCfgBorder(wxCommandEvent& event) {
        prj->config.border = event.GetInt();
        prj->Change();
    }

    wxDECLARE_EVENT_TABLE();

   private:
    std::optional<Prj> prj;
};

class MyApp : public wxApp {
   private:
    MainWindow* frame;

   public:
    // 这个函数将会在程序启动的时候被调用
    virtual bool OnInit();
};

// 有了这一行就可以使用 MyApp& wxGetApp()了
DECLARE_APP(MyApp)
}  // namespace Croplines
