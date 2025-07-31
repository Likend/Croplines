#pragma once

#include <wx/wx.h>

#include "wxUI.h"

namespace Croplines {
class MyApp : public wxApp {
   private:
    MainWindow* frame;

   public:
    // 这个函数将会在程序启动的时候被调用
    virtual bool OnInit();

    void EnableTools(bool state);
    void EnableConfigs(bool state);
};

// 有了这一行就可以使用 MyApp& wxGetApp()了
DECLARE_APP(MyApp)
}  // namespace Croplines