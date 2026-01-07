#include "ui/ToolBar.hpp"

#include <wx/aui/auibar.h>

#include "ui/Defs.hpp"

using namespace Croplines;

ToolBar::ToolBar(wxWindow* parent, wxWindowID id)
    : wxAuiToolBar(parent, id, wxDefaultPosition, wxDefaultSize,
                   wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_OVERFLOW | wxAUI_TB_TEXT) {
    SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    AddTool(wxID_UP, wxT("上一页"), wxBITMAP_PNG(IMG_PREV_PAGE), wxNullBitmap, wxITEM_NORMAL,
            wxT("上一页"), wxT("上一页"), nullptr);
    AddTool(wxID_DOWN, wxT("下一页"), wxBITMAP_PNG(IMG_NEXT_PAGE), wxNullBitmap, wxITEM_NORMAL,
            wxT("下一页"), wxT("下一页"), nullptr);

    AddSeparator();

    AddTool(wxID_SAVE, wxT("保存"), wxBITMAP_PNG(IMG_SAVE), wxNullBitmap, wxITEM_NORMAL,
            wxT("保存"), wxT("保存"), nullptr);
    AddTool(wxID_OPEN, wxT("载入"), wxBITMAP_PNG(IMG_LOAD), wxNullBitmap, wxITEM_NORMAL,
            wxT("载入"), wxT("载入"), nullptr);

    AddSeparator();

    AddTool(wxID_ZOOM_FIT, wxT("缩放合适大小"), wxBITMAP_PNG(IMG_ZOOM_PAGE), wxNullBitmap,
            wxITEM_NORMAL, wxT("缩放合适大小"), wxT("缩放合适大小"), nullptr);
    AddTool(buttonID_CROP_CURR_PAGE, wxT("裁剪当前页"), wxBITMAP_PNG(IMG_CROP_CURR_PAGE),
            wxNullBitmap, wxITEM_NORMAL, wxT("裁剪当前页"), wxT("裁剪当前页"), nullptr);
    AddTool(buttonID_CROP_ALL_PAGE, wxT("裁剪全部"), wxBITMAP_PNG(IMG_CROP_ALL_PAGE), wxNullBitmap,
            wxITEM_NORMAL, wxT("裁剪全部"), wxT("裁剪全部"), nullptr);

    Realize();
}

bool ToolBar::Enable(bool state) {
    // Do not enable wxID_OPEN
    static int needToEnable[] = {
        wxID_UP,
        wxID_DOWN,
        wxID_SAVE,
        wxID_ZOOM_FIT,
        buttonID_CROP_ALL_PAGE,
        buttonID_CROP_CURR_PAGE,
    };

    for (int id : needToEnable) EnableTool(id, state);

    Refresh();
    return true;
}
