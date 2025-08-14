#include "wxUI.h"

#include <wx/event.h>

using namespace Croplines;

MainUI::MainUI(wxWindow* parent, wxWindowID id, const wxString& title,
               const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, parent->FromDIP(size), style) {
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    m_mgr.SetManagedWindow(this);
    m_mgr.SetFlags(wxAUI_MGR_DEFAULT);

    toolbar = new wxAuiToolBar(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_OVERFLOW | wxAUI_TB_TEXT);
    toolbar->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    btn_prev_page = toolbar->AddTool(
        btnid_PREV_PAGE, wxT("上一页"), wxBITMAP_PNG(IMG_PREV_PAGE),
        wxNullBitmap, wxITEM_NORMAL, wxT("上一页"), wxT("上一页"), NULL);

    btn_next_page = toolbar->AddTool(
        btnid_NEXT_PAGE, wxT("下一页"), wxBITMAP_PNG(IMG_NEXT_PAGE),
        wxNullBitmap, wxITEM_NORMAL, wxT("下一页"), wxT("下一页"), NULL);

    toolbar->AddSeparator();

    btn_save = toolbar->AddTool(btnid_SAVE, wxT("保存"), wxBITMAP_PNG(IMG_SAVE),
                                wxNullBitmap, wxITEM_NORMAL, wxT("保存"),
                                wxT("保存"), NULL);

    btn_load = toolbar->AddTool(btnid_LOAD, wxT("载入"), wxBITMAP_PNG(IMG_LOAD),
                                wxNullBitmap, wxITEM_NORMAL, wxT("载入"),
                                wxT("载入"), NULL);

    toolbar->AddSeparator();

    btn_zoom_page = toolbar->AddTool(btnid_ZOOM_PAGE, wxT("缩放合适大小"),
                                     wxBITMAP_PNG(IMG_ZOOM_PAGE), wxNullBitmap,
                                     wxITEM_NORMAL, wxT("缩放合适大小"),
                                     wxT("缩放合适大小"), NULL);

    btn_crop_curr_page = toolbar->AddTool(
        btnid_CROP_CURR_PAGE, wxT("裁剪当前页"),
        wxBITMAP_PNG(IMG_CROP_CURR_PAGE), wxNullBitmap, wxITEM_NORMAL,
        wxT("裁剪当前页"), wxT("裁剪当前页"), NULL);

    btn_crop_all_page = toolbar->AddTool(
        btnid_CROP_ALL_PAGE, wxT("裁剪全部"), wxBITMAP_PNG(IMG_CROP_ALL_PAGE),
        wxNullBitmap, wxITEM_NORMAL, wxT("裁剪全部"), wxT("裁剪全部"), NULL);

    toolbar->Realize();
    m_mgr.AddPane(toolbar, wxAuiPaneInfo()
                               .Top()
                               .CaptionVisible(false)
                               .CloseButton(false)
                               .PaneBorder(false)
                               .Movable(false)
                               .Dock()
                               .Resizable()
                               .FloatingSize(wxSize(-1, -1))
                               .DockFixed(true)
                               .LeftDockable(false)
                               .RightDockable(false)
                               .Floatable(false)
                               .Layer(10)
                               .ToolbarPane());

    status_bar = this->CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);
    pn_page_list = new wxListBox(this, pnid_PAGE_LIST, wxDefaultPosition,
                                 wxDefaultSize, 0, NULL, wxLB_NEEDED_SB);
    m_mgr.AddPane(pn_page_list, wxAuiPaneInfo()
                                    .Left()
                                    .Caption(wxT("页面列表"))
                                    .PinButton(true)
                                    .Dock()
                                    .Resizable()
                                    .FloatingSize(wxDefaultSize)
                                    .BottomDockable(false)
                                    .TopDockable(false)
                                    .BestSize(FromDIP(wxSize(200, 500)))
                                    .MinSize(FromDIP(wxSize(130, -1))));

    pn_canvas = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                            wxTAB_TRAVERSAL);
    m_mgr.AddPane(pn_canvas, wxAuiPaneInfo()
                                 .Left()
                                 .CaptionVisible(false)
                                 .PinButton(true)
                                 .Dock()
                                 .Resizable()
                                 .FloatingSize(wxDefaultSize)
                                 .CentrePane());
    wxBoxSizer* bSizer31 = new wxBoxSizer(wxVERTICAL);
    canvas = new Canvas(pn_canvas, wxID_ANY);
    bSizer31->Add(canvas, 1, wxEXPAND, 5);
    pn_canvas->SetSizer(bSizer31);
    // pn_canvas->Layout();
    // bSizer31->Fit(pn_canvas);

    pn_config = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               0, wxT("设置"));
    m_mgr.AddPane(pn_config, wxAuiPaneInfo()
                                 .Right()
                                 .Caption(wxT("设置"))
                                 .PinButton(true)
                                 .Dock()
                                 .Resizable()
                                 .FloatingSize(wxDefaultSize)
                                 .BottomDockable(false)
                                 .TopDockable(false)
                                 .BestSize(FromDIP(wxSize(350, 500)))
                                 .MinSize(FromDIP(wxSize(350, -1))));

    ntbk_cfg_process = new wxPanel(pn_config, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer4;
    bSizer4 = new wxBoxSizer(wxVERTICAL);

    sld_cfg_pix_filter = new SliderWithSpin(
        ntbk_cfg_process, sldid_cfg_PIX_FILTER, wxT("忽略斑点直径"), 8, 0, 50);
    bSizer4->Add(sld_cfg_pix_filter, 0, wxEXPAND, 5);

    ntbk_cfg_process->SetSizer(bSizer4);
    ntbk_cfg_process->Layout();
    bSizer4->Fit(ntbk_cfg_process);
    pn_config->AddPage(ntbk_cfg_process, wxT("处理"), false);
    ntbk_cfg_output = new wxPanel(pn_config, wxID_ANY, wxDefaultPosition,
                                  wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer41;
    bSizer41 = new wxBoxSizer(wxVERTICAL);

    sld_cfg_border = new SliderWithSpin(ntbk_cfg_output, sldid_cfg_BORDER,
                                        wxT("空白边距"), 10, 0, 100);
    bSizer41->Add(sld_cfg_border, 0, wxEXPAND, 5);

    ntbk_cfg_output->SetSizer(bSizer41);
    ntbk_cfg_output->Layout();
    bSizer41->Fit(ntbk_cfg_output);
    pn_config->AddPage(ntbk_cfg_output, wxT(" 输出"), true);

    m_mgr.Update();
    this->Centre(wxBOTH);
}

MainUI::~MainUI() { m_mgr.UnInit(); }
