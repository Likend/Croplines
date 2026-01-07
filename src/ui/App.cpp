#include "ui/App.hpp"

#include <wx/gdicmn.h>
#include <wx/wx.h>

#include "ui/MainFrame.hpp"

using namespace Croplines;

IMPLEMENT_APP(CroplinesApp)

bool CroplinesApp::OnInit() {
    SetAppearance(Appearance::System);
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxTIFFHandler);
    wxImage::AddHandler(new wxJPEGHandler);
    wxImage::AddHandler(new wxWEBPHandler);

    m_frame = new MainFrame(nullptr, wxID_ANY, wxT("Croplines"), wxDefaultPosition, {900, 600});
    m_frame->Show(true);

    return true;
}
