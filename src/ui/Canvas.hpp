#pragma once

#include <memory>
#include <optional>

#include <opencv2/opencv.hpp>
#include <wx/glcanvas.h>
#include <wx/wx.h>
// include wx before gl
#include <GL/gl.h>


#include "core/Document.hpp"
#include "core/ImageScaleModel.hpp"
#include "core/Page.hpp"

namespace croplines {
class Canvas : public wxGLCanvas {
   public:
    Canvas(wxWindow* parent, wxWindowID id);
    ~Canvas() override;
    void  SetPage(Page& page);
    Page& GetPage() { return *m_page; }

    Document&           GetDocument() { return GetPage().GetDocument(); }
    wxCommandProcessor* GetProcessor() { return GetDocument().GetProcessor(); }

    void               Clear();
    [[nodiscard]] bool IsLoaded() const { return m_page != nullptr; }
    ImageScaleModel&   GetScaleModel() { return m_scaleModel.value(); }

    void ZoomIn();
    void ZoomOut();
    void ZoomFit();
    void Zoom(double scale);

   private:
    bool m_isDeleting    = false;
    bool m_isInitialized = false;

    bool                   m_isMouseCaptured = false;
    std::optional<wxPoint> m_mouseDragStartPosition;
    std::optional<wxPoint> m_mouseCurrentPosition;

    Page* m_page = nullptr;

    std::optional<ImageScaleModel> m_scaleModel;

    std::unique_ptr<wxGLContext> m_glContext;
    GLuint                       m_glTexture = 0;
    wxImage                      m_imageDst;
    bool                         m_isImageModified = false;

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
}  // namespace croplines
