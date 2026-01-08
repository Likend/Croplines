#pragma once

#include <wx/wx.h>

#include "ui/MainFrame.hpp"

namespace croplines {
class CroplinesApp : public wxApp {
   private:
    MainFrame* m_frame;

   public:
    bool OnInit() override;
};
}  // namespace croplines
