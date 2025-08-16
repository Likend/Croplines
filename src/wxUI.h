#pragma once

#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinbutt.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/string.h>
#include <wx/xrc/xmlres.h>

#include "canvas.h"
#include "ctrl.h"

namespace Croplines {

enum {
    btnid_CROP_CURR_PAGE = 1000,
    btnid_CROP_ALL_PAGE,

    pnid_PAGE_LIST,

    sldid_cfg_PIX_FILTER,
    sldid_cfg_BORDER
};
class MainUI : public wxFrame {
   public:
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
    Canvas* canvas;
    wxNotebook* pn_config;
    wxPanel* ntbk_cfg_process;
    SliderWithSpin* sld_cfg_pix_filter;
    wxPanel* ntbk_cfg_output;
    SliderWithSpin* sld_cfg_border;

    MainUI(wxWindow* parent, wxWindowID id, const wxString& title,
           const wxPoint& pos, const wxSize& size, long style);
    wxAuiManager m_mgr;

    virtual ~MainUI();
};
}  // namespace Croplines