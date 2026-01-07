#pragma once

#include <opencv2/opencv.hpp>
#include <wx/cmdproc.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>

#include "core/Document.hpp"
#include "core/ImageScaleModel.hpp"
#include "core/Page.hpp"

namespace Croplines {
class Canvas : public wxGLCanvas {
   public:
    Canvas(wxWindow* parent, wxWindowID id);
    ~Canvas() override;
    void  SetPage(Page& page);
    Page& GetPage() { return *m_page; }

    Document&           GetDocument() { return GetPage().getDocument(); }
    wxCommandProcessor* GetProcessor() { return GetDocument().GetProcessor(); }

    void             Clear();
    bool             IsLoaded() const { return m_page != nullptr; }
    ImageScaleModel& getScaleModel() { return m_scaleModel; }

    void ZoomIn();
    void ZoomOut();
    void ZoomFit();
    void Zoom(double scale);

   private:
    bool is_deleting = false;
    bool initialized = false;

    bool                   is_mouse_capture = false;
    std::optional<wxPoint> mouse_drag_start;
    std::optional<wxPoint> mouse_position;

    wxBitmap drawBmp;

    Page*           m_page = nullptr;
    ImageScaleModel m_scaleModel;

    wxGLContext* context;
    GLuint       texture;
    cv::UMat     uimageSrc;
    wxImage      imageDst;
    bool         imageModified = false;

   private:
    void UpdateScrollbars();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);

    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseCaptureLostEvent& event);
    void OnMouseRightDown(wxMouseEvent& event);
    void OnMouseRightUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);

    void OnScroll(wxScrollWinEvent& event);

    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);

    void OnKillFocus(wxFocusEvent& event);

    wxDECLARE_EVENT_TABLE();
};
}  // namespace Croplines
