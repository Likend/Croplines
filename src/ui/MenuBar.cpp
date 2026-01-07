#include "ui/MenuBar.hpp"

#include "ui/Defs.hpp"

using namespace Croplines;

MenuBar::MenuBar() : wxMenuBar() {
    menu_file->Append(wxID_OPEN, wxT("&Load\tCtrl+O"));
    menu_file->Append(wxID_SAVE);
    menu_file->AppendSeparator();
    menu_file->Append(buttonID_CROP_CURR_PAGE, wxT("Crop &current page"),
                      wxT("Crop current page to subimages and save each one to "
                          "output directory"));
    menu_file->Append(buttonID_CROP_ALL_PAGE, wxT("Crop &all pages"),
                      wxT("Crop all pages to subimages and save each one to "
                          "output directory"));
    menu_file->AppendSeparator();
    menu_file->Append(wxID_CLOSE);
    menu_file->Append(wxID_EXIT);

    Append(menu_file, wxT("&File"));

    menu_edit->Append(wxID_UNDO);
    menu_edit->Append(wxID_REDO);
    menu_edit->AppendSeparator();
    menu_edit->Append(wxID_UP, wxT("Last page\tUp"), wxT("Move to last page"));
    menu_edit->Append(wxID_DOWN, wxT("Next page\tDown"), wxT("Move to next page"));

    Append(menu_edit, wxT("&Edit"));

    menu_view->Append(wxID_ZOOM_IN);
    menu_view->Append(wxID_ZOOM_OUT);
    menu_view->Append(wxID_ZOOM_FIT);
    menu_view->Append(wxID_ZOOM_100);

    Append(menu_view, wxT("&View"));

    menu_help->Append(wxID_ABOUT);

    Append(menu_help, wxT("&Help"));
}
