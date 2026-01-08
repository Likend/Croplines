#include "ui/MenuBar.hpp"

#include <wx/wx.h>

#include "ui/Defs.hpp"

using namespace croplines;

MenuBar::MenuBar() : wxMenuBar() {
    m_menuFile->Append(wxID_OPEN, wxT("&Load\tCtrl+O"));
    m_menuFile->Append(wxID_SAVE);
    m_menuFile->AppendSeparator();
    m_menuFile->Append(buttonID_CROP_CURR_PAGE, wxT("Crop &current page"),
                       wxT("Crop current page to subimages and save each one to "
                           "output directory"));
    m_menuFile->Append(buttonID_CROP_ALL_PAGE, wxT("Crop &all pages"),
                       wxT("Crop all pages to subimages and save each one to "
                           "output directory"));
    m_menuFile->AppendSeparator();
    m_menuFile->Append(wxID_CLOSE);
    m_menuFile->Append(wxID_EXIT);

    Append(m_menuFile, wxT("&File"));

    m_menuEdit->Append(wxID_UNDO);
    m_menuEdit->Append(wxID_REDO);
    m_menuEdit->AppendSeparator();
    m_menuEdit->Append(wxID_UP, wxT("Last page\tUp"), wxT("Move to last page"));
    m_menuEdit->Append(wxID_DOWN, wxT("Next page\tDown"), wxT("Move to next page"));

    Append(m_menuEdit, wxT("&Edit"));

    m_menuView->Append(wxID_ZOOM_IN);
    m_menuView->Append(wxID_ZOOM_OUT);
    m_menuView->Append(wxID_ZOOM_FIT);
    m_menuView->Append(wxID_ZOOM_100);

    Append(m_menuView, wxT("&View"));

    m_menuHelp->Append(wxID_ABOUT);

    Append(m_menuHelp, wxT("&Help"));
}

bool MenuBar::Enable(bool enable) {
    // Do not enable wxID_OPEN
    static int needToEnable[] = {
        wxID_SAVE,
        wxID_CLOSE,
        wxID_UP,
        wxID_DOWN,
        wxID_ZOOM_FIT,
        wxID_ZOOM_OUT,
        wxID_ZOOM_IN,
        wxID_ZOOM_100,
        buttonID_CROP_ALL_PAGE,
        buttonID_CROP_CURR_PAGE,
    };

    for (int id : needToEnable) Enable(id, enable);

    return true;
}
