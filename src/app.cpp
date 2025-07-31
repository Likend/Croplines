#include "app.h"

#include <wx/wx.h>

#include "wxUI.h"


using namespace Croplines;

// 告诉wxWidgets主应用程序是哪个类
IMPLEMENT_APP(MyApp)

// 初始化程序
bool MyApp::OnInit() {
    SetAppearance(Appearance::System);
    wxImage::AddHandler(new wxPNGHandler);

    frame = new MainWindow(nullptr);  // 创建主窗口
    frame->SetIcon(wxICON(MAIN_ICON));
    frame->Show(true);  // 显示主窗口

    // bind tool bar
    EnableTools(false);
    EnableConfigs(false);

    frame->pn_canvas->SetImage(wxImage(wxT("hello.png"), wxBITMAP_TYPE_PNG));

    return true;  // 开始事件处理循环
}

void MyApp::EnableTools(bool state) {
    frame->toolbar->EnableTool(btnid_PREV_PAGE, state);
    frame->toolbar->EnableTool(btnid_NEXT_PAGE, state);
    frame->toolbar->EnableTool(btnid_SAVE, state);
    frame->toolbar->EnableTool(btnid_LOAD, state);
    frame->toolbar->EnableTool(btnid_ZOOM_PAGE, state);
    frame->toolbar->EnableTool(btnid_CROP_CURR_PAGE, state);
    frame->toolbar->EnableTool(btnid_CROP_ALL_PAGE, state);
}

void MyApp::EnableConfigs(bool state) {
    frame->ntbk_cfg_output->Enable(state);
    frame->ntbk_cfg_process->Enable(state);
}