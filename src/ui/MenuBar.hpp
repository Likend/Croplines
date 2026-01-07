#pragma once

#include <wx/menu.h>

namespace Croplines {
class MenuBar : public wxMenuBar {
   public:
    wxMenu* menu_file = new wxMenu{};
    wxMenu* menu_edit = new wxMenu{};
    wxMenu* menu_view = new wxMenu{};
    wxMenu* menu_help = new wxMenu{};

    MenuBar();
};
}  // namespace Croplines
