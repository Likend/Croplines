#pragma once
#include <wx/spinctrl.h>
#include <wx/wx.h>

namespace Croplines {
class SliderWithSpin : public wxPanel {
   private:
    wxStaticText* m_label;
    wxSlider* m_slider;
    wxSpinCtrl* m_spin;

   public:
    SliderWithSpin(wxWindow* parent, wxWindowID id, const wxString& label,
                   int value, int minValue, int maxValue,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize);

    virtual bool Enable(bool enable = false) override;

   private:
    int value;

   public:
    inline int GetValue() { return value; }
    void SetValue(int value);

   private:
    void CallEvent(int value);

    void OnSliderChanged(wxCommandEvent& event);

    void OnSpinChanged(wxSpinEvent& event);

    wxDECLARE_EVENT_TABLE();
};

class MenuBar : public wxMenuBar {
   public:
    wxMenu* menu_file;
    wxMenu* menu_edit;
    wxMenu* menu_view;
    wxMenu* menu_help;

    MenuBar();
};
}  // namespace Croplines