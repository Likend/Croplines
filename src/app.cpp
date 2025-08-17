#include "app.h"

#include <algorithm>
#include <thread>

#include <opencv2/opencv.hpp>
#include <wx/aboutdlg.h>
#include <wx/event.h>
#include <wx/wx.h>

#include "ctrl.h"

using namespace Croplines;

// // 设置加速器表
// const static wxAcceleratorEntry accel_entries[] = {
//     {wxACCEL_CTRL, 'S', wxID_SAVE},
//     {wxACCEL_CTRL, 'O', wxID_OPEN},
//     {wxACCEL_CTRL, 'W', wxID_EXIT},
//     {wxACCEL_NORMAL, WXK_UP, wxID_UP},
//     {wxACCEL_NORMAL, WXK_DOWN, wxID_DOWN}};
// const static wxAcceleratorTable accel_table(std::size(accel_entries),
//                                             accel_entries);

MainWindow::MainWindow(wxWindow* parent, wxWindowID id, const wxString& title,
                       const wxPoint& pos, const wxSize& size, long style)
    : MainUI(parent, id, title, pos, size, style) {
    SetIcon(wxICON(MAIN_ICON));

    menubar = new MenuBar();
    SetMenuBar(menubar);

    EnableTools(false);
    EnableMenu(false);
    EnableConfigs(false);

    // SetAcceleratorTable(accel_table);
}

void MainWindow::EnableTools(bool state) {
    toolbar->EnableTool(wxID_UP, state);
    toolbar->EnableTool(wxID_DOWN, state);
    toolbar->EnableTool(wxID_SAVE, state);
    // toolbar->EnableTool(wxID_OPEN, state);
    toolbar->EnableTool(wxID_ZOOM_FIT, state);
    toolbar->EnableTool(btnid_CROP_CURR_PAGE, state);
    toolbar->EnableTool(btnid_CROP_ALL_PAGE, state);
    toolbar->Refresh();
}

void MainWindow::EnableConfigs(bool enable) {
    ntbk_cfg_output->Enable(enable);
    ntbk_cfg_process->Enable(enable);
    sld_cfg_border->Enable(enable);
    sld_cfg_pix_filter->Enable(enable);
}

void MainWindow::EnableMenu(bool enable) {
    menubar->Enable(wxID_SAVE, enable);
    menubar->Enable(wxID_CLOSE, enable);
    menubar->Enable(wxID_UP, enable);
    menubar->Enable(wxID_DOWN, enable);
    menubar->Enable(wxID_ZOOM_FIT, enable);
    menubar->Enable(wxID_ZOOM_OUT, enable);
    menubar->Enable(wxID_ZOOM_IN, enable);
    menubar->Enable(wxID_ZOOM_100, enable);
    menubar->Enable(btnid_CROP_CURR_PAGE, enable);
    menubar->Enable(btnid_CROP_ALL_PAGE, enable);
    if (!enable) {
        menubar->Enable(wxID_UNDO, false);
        menubar->Enable(wxID_REDO, false);
    } else {
        menubar->Enable(wxID_UNDO, prj->CanUndo());
        menubar->Enable(wxID_REDO, prj->CanRedo());
    }
}

void MainWindow::Load(std::filesystem::path path) {
    auto prj_res = Prj::Load(path);
    if (prj_res) {
        prj = std::move(prj_res);
        prj->menubar = menubar;

        // panel page list
        std::vector<wxString> file_names(prj->GetPages().size());
        std::ranges::transform(prj->GetPages(), file_names.begin(),
                               [](const Prj::Page& page) {
                                   return wxString(page.image_path.filename());
                               });
        pn_page_list->Set(file_names);
        if (!prj->GetPages().empty()) {
            CurrentPage(0);
            ShowPage();
        }
        sld_cfg_border->SetValue(prj->config.border);
        sld_cfg_pix_filter->SetValue(prj->config.filter_noise_size);

        EnableTools(true);
        EnableConfigs(true);
        EnableMenu(true);
        canvas->SetPrj(*prj);
        SetTitle(wxString(path.filename()));
    }
}

static bool ShowCloseDialog(wxWindow* parent, Prj& prj) {
    wxMessageDialog dialog(parent, wxT("项目已更改，是否保存？"),
                           wxASCII_STR(wxMessageBoxCaptionStr),
                           wxICON_QUESTION | wxYES_NO | wxCANCEL);
    switch (dialog.ShowModal()) {
        case wxID_YES:
            prj.Save();
        case wxID_NO:
            return true;

        case wxID_CANCEL:
        default:
            return false;
    }
}

bool MainWindow::ClosePrj() {
    if (prj) {
        if (prj->IsChange()) {
            if (!ShowCloseDialog(this, *prj)) return false;
        }
        EnableTools(false);
        EnableMenu(false);
        EnableConfigs(false);
        prj->GetPages()[CurrentPage()].get().Close();
        prj.reset();
        canvas->Clear();
        pn_page_list->Clear();
    }
    return true;
}

bool MainWindow::Exit() {
    if (ClosePrj()) {
        Destroy();
        return true;
    } else
        return false;
}

void MainWindow::CurrentPage(std::size_t page) {
    if (prj) {
        __current_page = std::clamp(page, static_cast<std::size_t>(0),
                                    prj->GetPages().size() - 1);
        toolbar->EnableTool(wxID_UP, __current_page != 0);
        toolbar->EnableTool(wxID_DOWN,
                            __current_page != prj->GetPages().size() - 1);
        pn_page_list->SetSelection(__current_page);
    }
}

void MainWindow::PrevPage() {
    if (prj && CurrentPage() != 0) {
        prj->GetPages()[CurrentPage()].get().Close();
        CurrentPage(CurrentPage() - 1);
        ShowPage();
    }
}

void MainWindow::NextPage() {
    if (prj && CurrentPage() < prj->GetPages().size()) {
        prj->GetPages()[CurrentPage()].get().Close();
        CurrentPage(CurrentPage() + 1);
        ShowPage();
    }
}

void MainWindow::ShowPage() {
    canvas->SetPage(prj->GetPages()[CurrentPage()]);
    canvas->Refresh();
}

void MainWindow::OnLoad(wxCommandEvent& event) {
    if (prj && prj->IsChange()) {
        if (!ShowCloseDialog(this, *prj)) return;
    }
    wxDirDialog dir_dialog(this, wxT("选择目录"), wxEmptyString,
                           wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    auto result = dir_dialog.ShowModal();
    if (result == wxID_OK) {
        wxString dir = dir_dialog.GetPath();
        Load(std::filesystem::path(dir.utf8_string()));
    }
}

void MainWindow::OnUndo(wxCommandEvent& event) {
    if (prj) {
        prj->Undo();
        canvas->Refresh();
        menubar->Enable(wxID_UNDO, prj->CanUndo());
        menubar->Enable(wxID_REDO, true);
    }
}

void MainWindow::OnRedo(wxCommandEvent& event) {
    if (prj) {
        prj->Redo();
        canvas->Refresh();
        menubar->Enable(wxID_UNDO, true);
        menubar->Enable(wxID_REDO, prj->CanRedo());
    }
}

void MainWindow::OnCropCurrPage(wxCommandEvent& event) {
    if (canvas->IsLoaded()) {
        Prj::Page& page = *canvas->page;
        SetStatusText(std::format("Page {} is croping...", CurrentPage() + 1));
        if (prj->SaveCrops(page)) {
            SetStatusText(std::format("Page {} finished!", CurrentPage() + 1));
        } else {
            SetStatusText(std::format("Page {} failed!", CurrentPage() + 1));
        }
    }
}

void MainWindow::OnCropAllPage(wxCommandEvent& event) {
    if (canvas->IsLoaded()) {
        std::thread t([this]() {
            std::size_t count = 1;
            for (Prj::Page& page : prj->GetPages()) {
                SetStatusText(std::format("Page {} is croping...", count));
                prj->SaveCrops(page);
                count++;
            }
            SetStatusText(wxT("Croping finised!"));
        });
        t.detach();
    }
}

void MainWindow::OnAbout(wxCommandEvent& event) {
    static const char LICENSE[] = {
#embed "../LICENSE"
    };

    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(wxT("Croplines"));
    aboutInfo.SetIcon(wxICON(MAIN_ICON));
    // aboutInfo.AddArtist(wxT("Likend"));
    aboutInfo.AddDeveloper(wxT("Likend"));
    aboutInfo.SetWebSite(wxT("https://github.com/Likend/Croplines"));
    aboutInfo.SetCopyright(wxT("(C) 2024-2025"));
    aboutInfo.SetLicence(LICENSE);
    wxAboutBox(aboutInfo);
}

void MainWindow::OnClickListBox(wxCommandEvent& event) {
    if (CurrentPage() != event.GetSelection()) {
        CurrentPage(event.GetSelection());
        ShowPage();
    }
}

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(wxID_UP, MainWindow::OnPrevPage)
    EVT_MENU(wxID_DOWN, MainWindow::OnNextPage)
    EVT_MENU(wxID_SAVE, MainWindow::OnSave)
    EVT_MENU(wxID_OPEN, MainWindow::OnLoad)
    EVT_MENU(wxID_UNDO, MainWindow::OnUndo)
    EVT_MENU(wxID_REDO, MainWindow::OnRedo)
    EVT_MENU(wxID_ZOOM_IN, MainWindow::OnZoomIn)
    EVT_MENU(wxID_ZOOM_OUT, MainWindow::OnZoomOut)
    EVT_MENU(wxID_ZOOM_FIT, MainWindow::OnZoomFit)
    EVT_MENU(wxID_ZOOM_100, MainWindow::OnZoom100)
    EVT_MENU(btnid_CROP_CURR_PAGE, MainWindow::OnCropCurrPage)
    EVT_MENU(btnid_CROP_ALL_PAGE, MainWindow::OnCropAllPage)
    EVT_MENU(wxID_CLOSE, MainWindow::OnClose)
    EVT_MENU(wxID_EXIT, MainWindow::OnExit)
    EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)
    EVT_CLOSE(MainWindow::OnExit)
    EVT_LISTBOX(pnid_PAGE_LIST, MainWindow::OnClickListBox)
    EVT_SLIDER(sldid_cfg_PIX_FILTER, MainWindow::OnChnageCfgFilerPixSize)
    EVT_SLIDER(sldid_cfg_BORDER, MainWindow::OnChangeCfgBorder)
wxEND_EVENT_TABLE();
// clang-format on

// 告诉wxWidgets主应用程序是哪个类
IMPLEMENT_APP(MyApp)

// 初始化程序
bool MyApp::OnInit() {
    SetAppearance(Appearance::System);
    // wxImage::AddHandler(new wxBMPHandler);
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxTIFFHandler);
    wxImage::AddHandler(new wxJPEGHandler);
    wxImage::AddHandler(new wxWEBPHandler);

    frame = new MainWindow(nullptr);  // 创建主窗口
    frame->Show(true);                // 显示主窗口

    return true;  // 开始事件处理循环
}
