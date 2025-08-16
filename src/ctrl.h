#pragma once
#include <wx/spinbutt.h>
#include <wx/wx.h>

namespace Croplines {
class SliderWithSpin : public wxPanel {
   private:
    wxStaticText* m_label;
    wxStaticText* m_value_text;
    wxSlider* m_slider;
    wxSpinButton* m_spin;

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