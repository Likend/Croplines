#pragma once
#include <wx/wx.h>

#include "ui/MainFrame.hpp"

namespace Croplines {
class CroplinesApp : public wxApp {
   private:
    MainFrame* frame;

   public:
    // 这个函数将会在程序启动的时候被调用
    bool OnInit() override;
};
}  // namespace Croplines
