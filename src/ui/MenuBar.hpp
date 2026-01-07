#pragma once

#include <wx/menu.h>
#include <wx/msw/menu.h>

namespace Croplines {
class MenuBar : public wxMenuBar {
   public:
    wxMenu* m_menuFile = new wxMenu{};
    wxMenu* m_menuEdit = new wxMenu{};
    wxMenu* m_menuView = new wxMenu{};
    wxMenu* m_menuHelp = new wxMenu{};

    MenuBar();

    using wxMenuBar::Enable;
    bool Enable(bool enable = true) override;
};
}  // namespace Croplines
