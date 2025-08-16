#pragma once

#include <filesystem>

#include <wx/event.h>
#include <wx/wx.h>

#include "wxUI.h"

namespace Croplines {

class MainWindow final : public MainUI {
   public:
    MainWindow(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = wxT("Crop Lines"),
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxSize(972, 651),
               long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~MainWindow();

    void EnableTools(bool state);
    void EnableConfigs(bool enable);

    void Load(std::filesystem::path path);
    void Save() {
        if (prj) prj->Save();
    }
    bool Close();  // true if close succesfully; false if cancelled

    std::size_t CurrentPage() { return __current_page; }
    void CurrentPage(std::size_t);
    void PrevPage();
    void NextPage();

   private:
    std::size_t __current_page = 0;

   private:
    void ShowPage();

    void OnPrevPage(wxCommandEvent& event) { PrevPage(); }
    void OnNextPage(wxCommandEvent& event) { NextPage(); }
    void OnSave(wxCommandEvent& event) { Save(); }
    void OnLoad(wxCommandEvent& event);
    void OnZoomPage(wxCommandEvent& event);
    void OnCropCurrPage(wxCommandEvent& event);
    void OnCropAllPage(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event) {
        if (!Close()) event.Veto();
    }
    void OnClose(wxCommandEvent& event) { Close(); }
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