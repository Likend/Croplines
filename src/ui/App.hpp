#pragma once
#include <wx/wx.h>

#include "ui/MainFrame.hpp"

namespace croplines {
class CroplinesApp : public wxApp {
   private:
    MainFrame* m_frame;

   public:
    // 这个函数将会在程序启动的时候被调用
    bool OnInit() override;
};
}  // namespace croplines
