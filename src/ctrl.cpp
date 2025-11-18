#include "ctrl.h"

#include "wxUI.h"

using namespace Croplines;

constexpr int SLIDER_ID = 1100;
constexpr int SPIN_ID = 1101;

SliderWithSpin::SliderWithSpin(wxWindow* parent, wxWindowID id, const wxString& label, int value,
                               int minValue, int maxValue, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size) {
    m_label = new wxStaticText(this, wxID_ANY, label);
    m_slider = new wxSlider(this, SLIDER_ID, value, minValue, maxValue);
    m_spin = new wxSpinCtrl(this, SPIN_ID, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                            wxSP_ARROW_KEYS, minValue, maxValue, value);

    wxBoxSizer* bSizer = new wxBoxSizer(wxHORIZONTAL);
    bSizer->Add(m_label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bSizer->Add(m_slider, 1, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 5);
    bSizer->Add(m_spin, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, 5);

    SetSizer(bSizer);
}

bool SliderWithSpin::Enable(bool enable) {
    wxWindow* const items[] = {m_label, m_slider, m_spin};
    const bool prevEnable = IsEnabled();
    bool ret = wxPanel::Enable(enable);
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
    this->value = value;
    if (m_spin->GetValue() != value) {
        m_spin->SetValue(value);
    }
    if (m_slider->GetValue() != value) {
        m_slider->SetValue(GetValue());
    }
}

void SliderWithSpin::CallEvent(int value) {
    wxCommandEvent evt(wxEVT_COMMAND_SLIDER_UPDATED, GetId());
    evt.SetEventObject(this);
    evt.SetInt(value);
    ProcessWindowEvent(evt);
}

void SliderWithSpin::OnSliderChanged([[maybe_unused]] wxCommandEvent& event) {
    const int value = m_slider->GetValue();
    if (m_spin->GetValue() != value) {
        m_spin->SetValue(value);
    }
    this->value = value;
    CallEvent(value);
}

void SliderWithSpin::OnSpinChanged([[maybe_unused]] wxSpinEvent& event) {
    const int value = m_spin->GetValue();
    if (m_slider->GetValue() != value) {
        m_slider->SetValue(value);
    }
    this->value = value;
    CallEvent(value);
}

// clang-format off
wxBEGIN_EVENT_TABLE(SliderWithSpin, wxPanel)
    EVT_SLIDER(SLIDER_ID, SliderWithSpin::OnSliderChanged)
    EVT_SPINCTRL(SPIN_ID, SliderWithSpin::OnSpinChanged)
wxEND_EVENT_TABLE();
// clang-format on

MenuBar::MenuBar() : wxMenuBar() {
    menu_file = new wxMenu();
    menu_file->Append(wxID_OPEN, wxT("&Load\tCtrl+O"));
    menu_file->Append(wxID_SAVE);
    menu_file->AppendSeparator();
    menu_file->Append(btnid_CROP_CURR_PAGE, wxT("Crop &current page"),
                      wxT("Crop current page to subimages and save each one to "
                          "output directory"));
    menu_file->Append(btnid_CROP_ALL_PAGE, wxT("Crop &all pages"),
                      wxT("Crop all pages to subimages and save each one to "
                          "output directory"));
    menu_file->AppendSeparator();
    menu_file->Append(wxID_CLOSE);
    menu_file->Append(wxID_EXIT);

    Append(menu_file, wxT("&File"));

    menu_edit = new wxMenu();
    menu_edit->Append(wxID_UNDO);
    menu_edit->Append(wxID_REDO);
    menu_edit->AppendSeparator();
    menu_edit->Append(wxID_UP, wxT("Last page\tUp"), wxT("Move to last page"));
    menu_edit->Append(wxID_DOWN, wxT("Next page\tDown"), wxT("Move to next page"));

    Append(menu_edit, wxT("&Edit"));

    menu_view = new wxMenu();
    menu_view->Append(wxID_ZOOM_IN);
    menu_view->Append(wxID_ZOOM_OUT);
    menu_view->Append(wxID_ZOOM_FIT);
    menu_view->Append(wxID_ZOOM_100);

    Append(menu_view, wxT("&View"));

    menu_help = new wxMenu();
    menu_help->Append(wxID_ABOUT);

    Append(menu_help, wxT("&Help"));
}
