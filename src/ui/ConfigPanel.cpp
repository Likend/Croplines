#include "ui/ConfigPanel.hpp"

#include <wx/app.h>
#include <wx/cmdproc.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include "core/DocumentData.hpp"
#include "ui/Defs.hpp"

using namespace croplines;

ConfigPanel::ConfigPanel(wxWindow* parent, wxWindowID id) : wxNotebook(parent, id) {
    m_processPage = new ProcessConfigPage{this, wxID_ANY};
    m_outputPage  = new OutputConfigPage{this, wxID_ANY};

    AddPage(m_processPage, wxT("处理"));
    AddPage(m_outputPage, wxT("输出"));
}

ProcessConfigPage::ProcessConfigPage(wxWindow* parent, wxWindowID id) : wxPanel(parent, id) {
    m_sliderPixFilter =
        new SliderWithSpin{this, sliderID_cfg_PIX_FILTER, wxT("忽略斑点直径"), 8, 0, 50};

    auto* bSizer = new wxBoxSizer{wxVERTICAL};
    bSizer->Add(m_sliderPixFilter, 0, wxEXPAND, 5);

    SetSizer(bSizer);
    Layout();
    bSizer->Fit(this);
}

OutputConfigPage::OutputConfigPage(wxWindow* parent, wxWindowID id) : wxPanel(parent, id) {
    m_sliderBorder = new SliderWithSpin{this, sliderID_cfg_BORDER, wxT("空白边距"), 10, 0, 100};

    auto* bSizer = new wxBoxSizer{wxVERTICAL};
    bSizer->Add(m_sliderBorder, 0, wxEXPAND, 5);

    SetSizer(bSizer);
    Layout();
    bSizer->Fit(this);
}

void ConfigPanel::SyncUI(const DocumentConfig& config) {
    m_processPage->SyncUI(config);
    m_outputPage->SyncUI(config);
}

void ProcessConfigPage::SyncUI(const DocumentConfig& config) {
    m_sliderPixFilter->SetValue(config.filter_noise_size);
}

void OutputConfigPage::SyncUI(const DocumentConfig& config) {
    m_sliderBorder->SetValue(config.border);
}
