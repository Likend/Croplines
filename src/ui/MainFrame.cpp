#include "ui/MainFrame.hpp"

#include <iterator>
#include <memory>

// #include <thread>

#include <wx/aboutdlg.h>
#include <wx/aui/framemanager.h>
#include <wx/chartype.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include "core/Document.hpp"
#include "ui/Canvas.hpp"
#include "ui/components/SliderWithSpin.hpp"
#include "ui/ConfigPanel.hpp"
#include "ui/Defs.hpp"
#include "ui/ToolBar.hpp"
#include "utils/Asserts.hpp"

using namespace croplines;

MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos,
                     const wxSize& size)
    : wxFrame(parent, id, title, pos) {
    SetClientSize(FromDIP(size));
    SetIcon(wxICON(MAIN_ICON));
    SetMenuBar(m_menuBar);

    m_mgr.SetManagedWindow(this);

    m_toolBar = new ToolBar{this, wxID_ANY};
    m_mgr.AddPane(m_toolBar, wxAuiPaneInfo()
                                 .Top()
                                 .CaptionVisible(false)
                                 .CloseButton(false)
                                 .PaneBorder(false)
                                 .Movable(false)
                                 .Dock()
                                 .Resizable()
                                 .FloatingSize(wxSize(-1, -1))
                                 .DockFixed(true)
                                 .LeftDockable(false)
                                 .RightDockable(false)
                                 .Floatable(false)
                                 .Layer(10)
                                 .ToolbarPane());

    m_statusBar = CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);

    auto* m_canvasPanel = new wxPanel{this, wxID_ANY};
    m_mgr.AddPane(m_canvasPanel, wxAuiPaneInfo()
                                     .Left()
                                     .CaptionVisible(false)
                                     .PinButton(true)
                                     .Dock()
                                     .Resizable()
                                     .FloatingSize(wxDefaultSize)
                                     .CentrePane());
    auto* bSizer = new wxBoxSizer{wxVERTICAL};
    m_canvas     = new Canvas{m_canvasPanel, wxID_ANY};
    bSizer->Add(m_canvas, 1, wxEXPAND, 5);
    m_canvasPanel->SetSizer(bSizer);
    m_canvasPanel->Layout();
    bSizer->Fit(m_canvasPanel);

    m_configPanel = new ConfigPanel{this, wxID_ANY};
    m_mgr.AddPane(m_configPanel, wxAuiPaneInfo()
                                     .Right()
                                     .Caption(wxT("设置"))
                                     .PinButton(true)
                                     .Dock()
                                     .Resizable()
                                     .FloatingSize(wxDefaultSize)
                                     .BottomDockable(false)
                                     .TopDockable(false)
                                     .BestSize(FromDIP(wxSize(350, 500)))
                                     .MinSize(FromDIP(wxSize(350, -1))));

    m_pageListPanel = new wxListBox{this, panelID_PAGE_LIST, wxDefaultPosition, wxDefaultSize,
                                    0,    nullptr,           wxLB_NEEDED_SB};
    m_mgr.AddPane(m_pageListPanel, wxAuiPaneInfo()
                                       .Left()
                                       .Caption(wxT("页面列表"))
                                       .PinButton(true)
                                       .Dock()
                                       .Resizable()
                                       .FloatingSize(wxDefaultSize)
                                       .BottomDockable(false)
                                       .TopDockable(false)
                                       .BestSize(FromDIP(wxSize(200, 500)))
                                       .MinSize(FromDIP(wxSize(130, -1))));

    m_mgr.Update();
    Center(wxBOTH);

    m_toolBar->Disable();
    m_configPanel->Disable();
    m_menuBar->Disable();
}

bool MainFrame::Load(const std::filesystem::path& path) {
    if (m_doc.IsLoad()) {
        if (!Close()) return false;
    }

    m_doc.Load(path);
    m_toolBar->Enable();
    m_configPanel->Enable();
    m_menuBar->Enable();
    SyncUIPageListPanel();
    SyncUIConfigPanel();
    if (m_doc.PagesSize() != 0) CurrentPage(0);
    SetTitle(wxString(path.filename()));
    return true;
}

static bool ShowCloseDialog(wxWindow* parent, Document& doc) {
    wxMessageDialog dialog(parent, wxT("项目已更改，是否保存？"),
                           wxASCII_STR(wxMessageBoxCaptionStr),
                           wxICON_QUESTION | wxYES_NO | wxCANCEL);
    switch (dialog.ShowModal()) {
        case wxID_YES:
            return doc.Save();
        case wxID_NO:
            return true;
        case wxID_CANCEL:
            return false;
        default:
            UNREACHABLE();
    }
}

bool MainFrame::Save() {
    if (m_doc.IsLoad()) {
        m_doc.Save();
        return true;
    } else
        return false;
}

bool MainFrame::Close() {
    if (m_doc.IsLoad()) {
        if (m_doc.IsModified()) {
            if (!ShowCloseDialog(this, m_doc)) return false;
        }
        SetFocus();  // 防止焦点在需要禁用的组件中
        m_toolBar->Disable();
        m_menuBar->Disable();
        m_configPanel->Disable();
        m_doc.Close();
        m_canvas->Clear();
        m_pageListPanel->Clear();
    }
    return true;
}

void MainFrame::CurrentPage(std::size_t page) {
    if (m_doc.IsLoad()) {
        m_currentPageIdx = std::clamp(page, static_cast<std::size_t>(0), m_doc.PagesSize() - 1);
        m_toolBar->EnableTool(wxID_UP, m_currentPageIdx != 0);
        m_toolBar->EnableTool(wxID_DOWN, m_currentPageIdx != m_doc.PagesSize() - 1);
        m_currentPage.emplace(m_doc.LoadPage(m_currentPageIdx));
        m_pageListPanel->SetSelection(static_cast<int>(m_currentPageIdx));
        m_canvas->SetPage(*m_currentPage);
        m_canvas->Refresh();
    }
}

void MainFrame::SyncUIPageListPanel() {
    m_pageListPanel->Clear();
    if (m_doc.IsLoad() && m_doc.PagesSize() != 0) {
        std::vector<wxString> file_names;
        file_names.reserve(m_doc.PagesSize());
        std::ranges::transform(
            m_doc.GetData().pages, std::back_inserter(file_names),
            [](const std::unique_ptr<PageData>& page) { return wxString(page->path.filename()); });

        m_pageListPanel->Set(file_names);
    }
}

void MainFrame::SyncUIConfigPanel() { m_configPanel->SyncUI(m_doc.GetConfig()); }

void MainFrame::OnLoad(wxCommandEvent&) {
    if (m_doc.IsLoad() && m_doc.IsModified()) {
        if (!ShowCloseDialog(this, m_doc)) return;
    }
    wxDirDialog dir_dialog(this, wxT("选择目录"), wxEmptyString,
                           wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    auto        result = dir_dialog.ShowModal();
    if (result == wxID_OK) {
        wxString dir = dir_dialog.GetPath();
        Load(std::filesystem::path(dir.utf8_string()));
    }
}

void MainFrame::OnUndo(wxCommandEvent&) {
    if (m_doc.IsLoad()) {
        m_doc.GetProcessor()->Undo();
        m_canvas->Refresh();
    }
}

void MainFrame::OnRedo(wxCommandEvent&) {
    if (m_doc.IsLoad()) {
        m_doc.GetProcessor()->Redo();
        m_canvas->Refresh();
    }
}

void MainFrame::OnCropCurrPage(wxCommandEvent&) {
    if (m_currentPage) {
        SetStatusText(std::format("Page {} is croping...", CurrentPage() + 1));
        if (m_currentPage->SaveCrops()) {
            SetStatusText(std::format("Page {} finished!", CurrentPage() + 1));
        } else {
            SetStatusText(std::format("Page {} failed!", CurrentPage() + 1));
        }
    }
}

void MainFrame::OnCropAllPage(wxCommandEvent&) {
    if (!m_doc.IsLoad()) return;

    if (m_canvas->IsLoaded()) {
        // std::thread t([this]() {
        for (size_t i = 0; i < m_doc.PagesSize(); i++) {
            Page page = m_doc.LoadPage(i);
            SetStatusText(std::format("Page {} is croping...", i + 1));
            page.SaveCrops();
        }
        SetStatusText(wxT("Croping finised!"));
        // });
        // t.detach();
        // TODO!
    }
}

void MainFrame::OnExit(wxCloseEvent& event) {
    if (Close())
        Destroy();
    else
        event.Veto();
}

void MainFrame::OnAbout(wxCommandEvent&) {
#include "License.hpp"
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(wxT("Croplines"));
    aboutInfo.SetIcon(wxICON(MAIN_ICON));
    aboutInfo.AddDeveloper(wxT("Likend"));
    aboutInfo.SetWebSite(wxT("https://github.com/Likend/Croplines"));
    aboutInfo.SetCopyright(wxT("(C) 2024-2026"));
    aboutInfo.SetLicence(LICENSE);
    wxAboutBox(aboutInfo);
}

void MainFrame::OnClickListBox(wxCommandEvent& event) {
    if (CurrentPage() != event.GetSelection()) {
        CurrentPage(event.GetSelection());
    }
}

template <typename T>
class ChangeConfigCommand : public wxCommand {
   public:
    ChangeConfigCommand(SliderWithSpin* slider, Document& doc, T& placeToMidify, T newValue)
        : wxCommand(true, wxT("更改设置")),
          m_doc(doc),
          m_slider(slider),
          m_placeToModify(placeToMidify),
          m_newValue(newValue),
          m_oldValue(placeToMidify) {
        std::cout << "ChangeConfigCommand " << m_oldValue << " to " << m_newValue << std::endl;
    }

    bool Do() override {
        std::cout << "Change current " << m_placeToModify << " from " << m_oldValue << " to "
                  << m_newValue << std::endl;
        if (m_slider && m_slider->GetValue() != m_newValue) m_slider->SetValue(m_newValue);
        m_placeToModify = m_newValue;
        m_doc.SetModified();
        return true;
    }

    bool Undo() override {
        std::cout << "Change current " << m_placeToModify << " from " << m_newValue << " to "
                  << m_oldValue << std::endl;
        if (m_slider && m_slider->GetValue() != m_oldValue) m_slider->SetValue(m_oldValue);
        m_placeToModify = m_oldValue;
        m_doc.SetModified();
        return true;
    }

   private:
    Document&       m_doc;
    SliderWithSpin* m_slider;

    T& m_placeToModify;
    T  m_newValue;
    T  m_oldValue;
};

void MainFrame::OnChnageCfgFilerPixSize(wxCommandEvent& event) {
    wxWindow*       win    = FindWindowById(sliderID_cfg_PIX_FILTER);
    SliderWithSpin* slider = wxDynamicCast(win, SliderWithSpin);
    auto* command = new ChangeConfigCommand<int>{slider, m_doc, m_doc.GetConfig().filter_noise_size,
                                                 event.GetInt()};
    m_doc.GetProcessor()->Submit(command);
}

void MainFrame::OnChangeCfgBorder(wxCommandEvent& event) {
    wxWindow*       win    = FindWindowById(sliderID_cfg_BORDER);
    SliderWithSpin* slider = wxDynamicCast(win, SliderWithSpin);
    auto*           command =
        new ChangeConfigCommand<int>{slider, m_doc, m_doc.GetConfig().border, event.GetInt()};
    m_doc.GetProcessor()->Submit(command);
}

void MainFrame::OnUpdateUndo(wxUpdateUIEvent&) {
    m_menuBar->Enable(wxID_UNDO, m_doc.GetProcessor()->CanUndo());
}
void MainFrame::OnUpdateRedo(wxUpdateUIEvent&) {
    m_menuBar->Enable(wxID_REDO, m_doc.GetProcessor()->CanRedo());
}
void MainFrame::OnUpdateSave(wxUpdateUIEvent&) {
    m_menuBar->Enable(wxID_SAVE, m_doc.IsModified());
    m_toolBar->EnableTool(wxID_SAVE, m_doc.IsModified());
    m_toolBar->Refresh();
}

// clang-format off
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_UP, MainFrame::OnPrevPage)
    EVT_MENU(wxID_DOWN, MainFrame::OnNextPage)
    EVT_MENU(wxID_SAVE, MainFrame::OnSave)
    EVT_MENU(wxID_OPEN, MainFrame::OnLoad)
    EVT_MENU(wxID_UNDO, MainFrame::OnUndo)
    EVT_MENU(wxID_REDO, MainFrame::OnRedo)
    EVT_MENU(wxID_ZOOM_IN, MainFrame::OnZoomIn)
    EVT_MENU(wxID_ZOOM_OUT, MainFrame::OnZoomOut)
    EVT_MENU(wxID_ZOOM_FIT, MainFrame::OnZoomFit)
    EVT_MENU(wxID_ZOOM_100, MainFrame::OnZoom100)
    EVT_MENU(buttonID_CROP_CURR_PAGE, MainFrame::OnCropCurrPage)
    EVT_MENU(buttonID_CROP_ALL_PAGE, MainFrame::OnCropAllPage)
    EVT_MENU(wxID_CLOSE, MainFrame::OnClose)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_CLOSE(MainFrame::OnExit)
    EVT_LISTBOX(panelID_PAGE_LIST, MainFrame::OnClickListBox)
    EVT_SLIDER(sliderID_cfg_PIX_FILTER, MainFrame::OnChnageCfgFilerPixSize)
    EVT_SLIDER(sliderID_cfg_BORDER, MainFrame::OnChangeCfgBorder)

    // Update ui
    EVT_UPDATE_UI(wxID_UNDO, MainFrame::OnUpdateUndo)
    EVT_UPDATE_UI(wxID_REDO, MainFrame::OnUpdateRedo)
    EVT_UPDATE_UI(wxID_SAVE, MainFrame::OnUpdateSave)

wxEND_EVENT_TABLE();
