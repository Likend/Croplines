#pragma once

#include <wx/event.h>
#include <wx/object.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

namespace croplines {
class SliderWithSpin : public wxPanel {
   public:
    SliderWithSpin() : wxPanel() {}
    SliderWithSpin(wxWindow* parent, wxWindowID id, const wxString& label, int value, int minValue,
                   int maxValue, const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize);

    bool Enable(bool enable = false) override;

    [[nodiscard]] int GetValue() const { return m_value; }
    void              SetValue(int value);

   private:
    wxStaticText* m_label  = nullptr;
    wxSlider*     m_slider = nullptr;
    wxSpinCtrl*   m_spin   = nullptr;
    int           m_value  = 0;

    void CallEvent(int value);
    void OnSliderChanging(wxCommandEvent&);
    void OnSpinChanged(wxSpinEvent&);
    void OnSliderScrollEnd(wxScrollEvent&);
    void OnSliderChanged(wxScrollEvent&);

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS_NO_COPY(SliderWithSpin);
};
}  // namespace croplines
