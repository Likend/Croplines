#include "app.h"

#include <algorithm>
#include <iterator>

#include <opencv2/opencv.hpp>
#include <wx/event.h>
#include <wx/wx.h>

#include "wxUI.h"

using namespace Croplines;

// 设置加速器表
const static wxAcceleratorEntry accel_entries[] = {
    {wxACCEL_CTRL, 'S', btnid_SAVE},
    {wxACCEL_CTRL, 'O', btnid_LOAD},
    {wxACCEL_CTRL, 'W', btnid_CLOSE},
    {wxACCEL_NORMAL, WXK_UP, btnid_PREV_PAGE},
    {wxACCEL_NORMAL, WXK_DOWN, btnid_NEXT_PAGE}};
const static wxAcceleratorTable accel_table(std::size(accel_entries),
                                            accel_entries);

MainWindow::MainWindow(wxWindow* parent, wxWindowID id, const wxString& title,
                       const wxPoint& pos, const wxSize& size, long style)
    : MainUI(parent, id, title, pos, size, style) {
    SetIcon(wxICON(MAIN_ICON));
    EnableTools(false);
    EnableConfigs(false);

    SetAcceleratorTable(accel_table);
}

MainWindow::~MainWindow() {}

void MainWindow::EnableTools(bool state) {
    toolbar->EnableTool(btnid_PREV_PAGE, state);
    toolbar->EnableTool(btnid_NEXT_PAGE, state);
    toolbar->EnableTool(btnid_SAVE, state);
    // toolbar->EnableTool(btnid_LOAD, state);
    toolbar->EnableTool(btnid_ZOOM_PAGE, state);
    toolbar->EnableTool(btnid_CROP_CURR_PAGE, state);
    toolbar->EnableTool(btnid_CROP_ALL_PAGE, state);
}

void MainWindow::EnableConfigs(bool enable) {
    ntbk_cfg_output->Enable(enable);
    ntbk_cfg_process->Enable(enable);
    sld_cfg_border->Enable(enable);
    sld_cfg_pix_filter->Enable(enable);
}

void MainWindow::Load(std::filesystem::path path) {
    auto prj_res = Prj::Load(path);
    if (prj_res) {
        prj = std::move(prj_res);

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
        canvas->SetPrj(*prj);
    }
}

bool MainWindow::Close() {
    if (prj && prj->IsChange()) {
        wxMessageDialog dialog(this, wxT("项目已更改，是否保存？"),
                               wxASCII_STR(wxMessageBoxCaptionStr),
                               wxICON_QUESTION | wxYES_NO | wxCANCEL);
        switch (dialog.ShowModal()) {
            case wxID_YES:
                prj->Save();
            case wxID_NO:
                break;

            case wxID_CANCEL:
            default:
                return false;
        }
    }
    Destroy();
    return true;
}

void MainWindow::CurrentPage(std::size_t page) {
    if (prj) {
        __current_page = std::clamp(page, static_cast<std::size_t>(0),
                                    prj->GetPages().size() - 1);
        toolbar->EnableTool(btnid_PREV_PAGE, __current_page != 0);
        toolbar->EnableTool(btnid_NEXT_PAGE,
                            __current_page != prj->GetPages().size() - 1);
        pn_page_list->SetSelection(__current_page);
    }
}

void MainWindow::PrevPage() {
    if (prj && CurrentPage() != 0) {
        CurrentPage(CurrentPage() - 1);
        ShowPage();
    }
}

void MainWindow::NextPage() {
    if (prj && CurrentPage() < prj->GetPages().size()) {
        CurrentPage(CurrentPage() + 1);
        ShowPage();
    }
}

void MainWindow::ShowPage() {
    canvas->SetPage(prj->GetPages()[CurrentPage()]);
    canvas->Refresh();
}

void MainWindow::OnLoad(wxCommandEvent& event) {
    wxDirDialog dir_dialog(this, wxT("选择目录"), wxEmptyString,
                           wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    auto result = dir_dialog.ShowModal();
    if (result == wxID_OK) {
        wxString dir = dir_dialog.GetPath();
        Load(std::filesystem::path(dir.utf8_string()));
    }
}

void MainWindow::OnZoomPage(wxCommandEvent& event) {
    if (canvas->IsLoaded()) {
        ImageScaleModel& scaleModel = canvas->scaleModel;
        scaleModel.ScaleTo(scaleModel.GetScaleSuitesPage());
        scaleModel.MoveToCenter();
        canvas->Refresh();
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
        std::size_t count = 1;
        for (Prj::Page& page : prj->GetPages()) {
            SetStatusText(std::format("Page {} is croping...", count));
            prj->SaveCrops(page);
            count++;
        }
        SetStatusText(wxT("Croping finised!"));
    }
}

void MainWindow::OnClickListBox(wxCommandEvent& event) {
    if (CurrentPage() != event.GetSelection()) {
        CurrentPage(event.GetSelection());
        ShowPage();
    }
}

// clang-format off
wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(btnid_PREV_PAGE, MainWindow::OnPrevPage)
    EVT_MENU(btnid_NEXT_PAGE, MainWindow::OnNextPage)
    EVT_MENU(btnid_SAVE, MainWindow::OnSave)
    EVT_MENU(btnid_LOAD, MainWindow::OnLoad)
    EVT_MENU(btnid_ZOOM_PAGE, MainWindow::OnZoomPage)
    EVT_MENU(btnid_CROP_CURR_PAGE, MainWindow::OnCropCurrPage)
    EVT_MENU(btnid_CROP_ALL_PAGE, MainWindow::OnCropAllPage)
    EVT_MENU(btnid_CLOSE, MainWindow::OnClose)
    EVT_CLOSE(MainWindow::OnClose)
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
    wxImage::AddHandler(new wxPNGHandler);

    frame = new MainWindow(nullptr);  // 创建主窗口
    frame->Show(true);                // 显示主窗口

    return true;  // 开始事件处理循环
}
