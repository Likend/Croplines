///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-6-ga75305af)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxUI.h"

///////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(wxWindow* parent, wxWindowID id, const wxString& title,
                       const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, FromDIP(size), style) {
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    m_mgr.SetManagedWindow(this);
    m_mgr.SetFlags(wxAUI_MGR_DEFAULT);

    toolbar = new wxAuiToolBar(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_OVERFLOW | wxAUI_TB_TEXT);
    toolbar->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    btn_prev_page = toolbar->AddTool(
        btnid_PREV_PAGE, wxT("上一页"),
        wxBitmap(wxT("assets/prevslide.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("上一页"), wxT("上一页"), NULL);

    btn_next_page = toolbar->AddTool(
        btnid_NEXT_PAGE, wxT("下一页"),
        wxBitmap(wxT("assets/nextslide.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("下一页"), wxT("下一页"), NULL);

    toolbar->AddSeparator();

    btn_save = toolbar->AddTool(
        btnid_SAVE, wxT("保存"),
        wxBitmap(wxT("assets/save.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("保存"), wxT("保存"), NULL);

    btn_load = toolbar->AddTool(
        btnid_LOAD, wxT("载入"),
        wxBitmap(wxT("assets/loadbasic.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("载入"), wxT("载入"), NULL);

    toolbar->AddSeparator();

    btn_zoom_page = toolbar->AddTool(
        btnid_ZOOM_PAGE, wxT("缩放合适大小"),
        wxBitmap(wxT("assets/zoompage.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("缩放合适大小"), wxT("缩放合适大小"), NULL);

    btn_crop_curr_page = toolbar->AddTool(
        btnid_CROP_CURR_PAGE, wxT("裁剪当前页"),
        wxBitmap(wxT("assets/crop.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("裁剪当前页"), wxT("裁剪当前页"), NULL);

    btn_crop_all_page = toolbar->AddTool(
        btnid_CROP_ALL_PAGE, wxT("裁剪全部"),
        wxBitmap(wxT("assets/cropall.png"), wxBITMAP_TYPE_ANY), wxNullBitmap,
        wxITEM_NORMAL, wxT("裁剪全部"), wxT("裁剪全部"), NULL);

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
    pn_page_list = new wxListBox(this, wxID_ANY, wxDefaultPosition,
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

    wxBoxSizer* bSizer31;
    bSizer31 = new wxBoxSizer(wxVERTICAL);

    // bSizer31->Add(canvas, 1, wxEXPAND, 5);

    pn_canvas->SetSizer(bSizer31);
    pn_canvas->Layout();
    bSizer31->Fit(pn_canvas);
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

    wxBoxSizer* bSizer9;
    bSizer9 = new wxBoxSizer(wxHORIZONTAL);

    tx_label_cfg_pix_filter =
        new wxStaticText(ntbk_cfg_process, wxID_ANY, wxT("忽略斑点直径"),
                         wxDefaultPosition, wxDefaultSize, 0);
    tx_label_cfg_pix_filter->Wrap(-1);
    bSizer9->Add(tx_label_cfg_pix_filter, 0, wxALL | wxALIGN_CENTER_VERTICAL,
                 5);

    tx_value_cfg_pix_filter =
        new wxStaticText(ntbk_cfg_process, wxID_ANY, wxT("00"),
                         wxDefaultPosition, wxDefaultSize, 0);
    tx_value_cfg_pix_filter->Wrap(-1);
    bSizer9->Add(tx_value_cfg_pix_filter, 0,
                 wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

    slider_cfg_pix_filter =
        new wxSlider(ntbk_cfg_process, wxID_ANY, 8, 0, 50, wxDefaultPosition,
                     wxDefaultSize, wxSL_HORIZONTAL);
    bSizer9->Add(slider_cfg_pix_filter, 1, wxTOP | wxBOTTOM, 5);

    spinbtn_cfg_pix_filter = new wxSpinButton(
        ntbk_cfg_process, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    spinbtn_cfg_pix_filter->SetMaxSize(FromDIP(wxSize(20, -1)));

    bSizer9->Add(spinbtn_cfg_pix_filter, 0,
                 wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, 5);

    bSizer4->Add(bSizer9, 0, wxEXPAND, 5);

    ntbk_cfg_process->SetSizer(bSizer4);
    ntbk_cfg_process->Layout();
    bSizer4->Fit(ntbk_cfg_process);
    pn_config->AddPage(ntbk_cfg_process, wxT("处理"), false);
    ntbk_cfg_output = new wxPanel(pn_config, wxID_ANY, wxDefaultPosition,
                                  wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer41;
    bSizer41 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* bSizer91;
    bSizer91 = new wxBoxSizer(wxHORIZONTAL);

    tx_label_border =
        new wxStaticText(ntbk_cfg_output, wxID_ANY, wxT("空白边距"),
                         wxDefaultPosition, wxDefaultSize, 0);
    tx_label_border->Wrap(-1);
    bSizer91->Add(tx_label_border, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    tx_value_cfg_border =
        new wxStaticText(ntbk_cfg_output, wxID_ANY, wxT("000"),
                         wxDefaultPosition, wxDefaultSize, 0);
    tx_value_cfg_border->Wrap(-1);
    bSizer91->Add(tx_value_cfg_border, 0,
                  wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

    slider_cfg_border =
        new wxSlider(ntbk_cfg_output, wxID_ANY, 10, 0, 500, wxDefaultPosition,
                     wxDefaultSize, wxSL_HORIZONTAL);
    bSizer91->Add(slider_cfg_border, 1, wxTOP | wxBOTTOM, 5);

    spinbtn_cfg_border = new wxSpinButton(ntbk_cfg_output, wxID_ANY,
                                          wxDefaultPosition, wxDefaultSize, 0);
    spinbtn_cfg_border->SetMaxSize(FromDIP(wxSize(20, -1)));

    bSizer91->Add(spinbtn_cfg_border, 0,
                  wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, 5);

    bSizer41->Add(bSizer91, 0, wxEXPAND, 5);

    ntbk_cfg_output->SetSizer(bSizer41);
    ntbk_cfg_output->Layout();
    bSizer41->Fit(ntbk_cfg_output);
    pn_config->AddPage(ntbk_cfg_output, wxT(" 输出"), true);

    m_mgr.Update();
    this->Centre(wxBOTH);
}

MainWindow::~MainWindow() { m_mgr.UnInit(); }
