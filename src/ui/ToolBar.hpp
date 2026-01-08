#pragma once

#include <wx/aui/auibar.h>
#include <wx/wx.h>
namespace croplines {
class ToolBar final : public wxAuiToolBar {
   public:
    ToolBar(wxWindow* parent, wxWindowID id);

    bool Enable(bool state = true) override;
};
}  // namespace croplines
