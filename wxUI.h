///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-6-ga75305af)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/string.h>
#include <wx/xrc/xmlres.h>
// from wxGLScene import WxGLScene
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinbutt.h>
#include <wx/stattext.h>

///////////////////////////////////////////////////////////////////////////

#define btnid_PREV_PAGE 1000
#define btnid_NEXT_PAGE 1001
#define btnid_SAVE 1002
#define btnid_LOAD 1003
#define btnid_ZOOM_PAGE 1004
#define btnid_CROP_CURR_PAGE 1005
#define btnid_CROP_ALL_PAGE 1006

///////////////////////////////////////////////////////////////////////////////
/// Class MainWindow
///////////////////////////////////////////////////////////////////////////////
class MainWindow : public wxFrame {
   private:
   protected:
    wxAuiToolBar* toolbar;
    wxAuiToolBarItem* btn_prev_page;
    wxAuiToolBarItem* btn_next_page;
    wxAuiToolBarItem* btn_save;
    wxAuiToolBarItem* btn_load;
    wxAuiToolBarItem* btn_zoom_page;
    wxAuiToolBarItem* btn_crop_curr_page;
    wxAuiToolBarItem* btn_crop_all_page;
    wxStatusBar* status_bar;
    wxListBox* pn_page_list;
    wxPanel* pn_canvas;
    wxNotebook* pn_config;
    wxPanel* ntbk_cfg_process;
    wxStaticText* tx_label_cfg_pix_filter;
    wxStaticText* tx_value_cfg_pix_filter;
    wxSlider* slider_cfg_pix_filter;
    wxSpinButton* spinbtn_cfg_pix_filter;
    wxPanel* ntbk_cfg_output;
    wxStaticText* tx_label_border;
    wxStaticText* tx_value_cfg_border;
    wxSlider* slider_cfg_border;
    wxSpinButton* spinbtn_cfg_border;

   public:
    MainWindow(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = wxT("Crop Lines"),
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxSize(972, 651),
               long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    wxAuiManager m_mgr;

    ~MainWindow();
};
