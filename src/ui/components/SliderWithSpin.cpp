#include "ui/components/SliderWithSpin.hpp"

#include <wx/wx.h>

using namespace croplines;

constexpr int SLIDER_ID = 1100;
constexpr int SPIN_ID   = 1101;

SliderWithSpin::SliderWithSpin(wxWindow* parent, wxWindowID id, const wxString& label, int value,
                               int minValue, int maxValue, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size) {
    m_label  = new wxStaticText(this, wxID_ANY, label);
    m_slider = new wxSlider(this, SLIDER_ID, value, minValue, maxValue);
    m_spin   = new wxSpinCtrl(this, SPIN_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                              wxSP_ARROW_KEYS, minValue, maxValue, value);

    auto* bSizer = new wxBoxSizer(wxHORIZONTAL);
    bSizer->Add(m_label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bSizer->Add(m_slider, 1, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 5);
    bSizer->Add(m_spin, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, 5);

    SetSizer(bSizer);
}

bool SliderWithSpin::Enable(bool enable) {
    wxWindow* const items[]    = {m_label, m_slider, m_spin};
    const bool      prevEnable = IsEnabled();
    bool            ret        = wxPanel::Enable(enable);
    if (!ret) {
        return false;
    }
    for (const auto& item : items) {
        ret = item->Enable(enable);
        if (!ret) {
            wxPanel::Enable(prevEnable);
            for (const auto* it = items; it != &item; it++) {
                (*it)->Enable(prevEnable);
            }
            return false;
        }
    }
    return true;
}

void SliderWithSpin::SetValue(int value) {
    this->m_value = value;
    m_spin->SetValue(value);
    m_slider->SetValue(value);
}

void SliderWithSpin::CallEvent(int value) {
    wxCommandEvent evt(wxEVT_COMMAND_SLIDER_UPDATED, GetId());
    evt.SetEventObject(this);
    evt.SetInt(value);
    ProcessWindowEvent(evt);
}

void SliderWithSpin::OnSliderChanging(wxCommandEvent&) {
    const int value = m_slider->GetValue();
    if (m_spin->GetValue() != value) {
        m_spin->SetValue(value);
    }
    this->m_value = value;
}

void SliderWithSpin::OnSpinChanged(wxSpinEvent&) {
    const int value = m_spin->GetValue();
    if (m_slider->GetValue() != value) {
        m_slider->SetValue(value);
    }
    this->m_value = value;
    CallEvent(value);
}

void SliderWithSpin::OnSliderChanged(wxScrollEvent& event) {
    const int value = m_slider->GetValue();
    CallEvent(value);
    event.Skip();
}

IMPLEMENT_DYNAMIC_CLASS(SliderWithSpin, wxFrame);

// clang-format off
wxBEGIN_EVENT_TABLE(SliderWithSpin, wxPanel)
    EVT_SLIDER(SLIDER_ID, SliderWithSpin::OnSliderChanging)
    EVT_SPINCTRL(SPIN_ID, SliderWithSpin::OnSpinChanged)
    EVT_SCROLL_CHANGED(SliderWithSpin::OnSliderChanged)
wxEND_EVENT_TABLE();
