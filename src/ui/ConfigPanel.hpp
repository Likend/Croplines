#pragma once

#include <wx/event.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/wx.h>

#include "core/DocumentData.hpp"
#include "ui/components/SliderWithSpin.hpp"

namespace Croplines {

class ProcessConfigPage;
class OutputConfigPage;

class ConfigPanel : public wxNotebook {
   public:
    ConfigPanel(wxWindow* parent, wxWindowID id);

    void SyncUI(const DocumentConfig& config);

   private:
    ProcessConfigPage* m_processPage;
    OutputConfigPage*  m_outputPage;
};

class ProcessConfigPage : public wxPanel {
   public:
    ProcessConfigPage(wxWindow* parent, wxWindowID id);

    void SyncUI(const DocumentConfig& config);

   private:
    SliderWithSpin* m_sliderPixFilter;
};

class OutputConfigPage : public wxPanel {
   public:
    OutputConfigPage(wxWindow* parent, wxWindowID id);

    void SyncUI(const DocumentConfig& config);

   private:
    SliderWithSpin* m_sliderBorder;
};

}  // namespace Croplines
