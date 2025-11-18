#pragma once

#include <optional>

#include <opencv2/opencv.hpp>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>

#include "prj.h"

namespace Croplines {

class ImageScaleModel {
   public:
    wxSize imageSize;
    wxSize windowSize;
    wxSize scaledSize;
    // matrix trans
    // [[scale,  0,      offset_x,
    //   0,      scale,  offset_y ]]
    double scale;
    wxPoint offset;

    ImageScaleModel() = default;
    ImageScaleModel(wxSize imageSize, wxSize windowSize);
    ImageScaleModel(wxSize imageSize, wxSize windowSize, double scale);

   private:
    void Clamp();

   public:
    void Scale(double factor, wxPoint center);
    void Scale(double factor) { Scale(factor, wxPoint{} + windowSize / 2); }
    void ScaleTo(double scale, wxPoint center);
    void ScaleTo(double scale) { ScaleTo(scale, wxPoint{} + windowSize / 2); }
    void Move(wxPoint dr);
    void MoveToCenter();
    void WindowResize(wxSize windowSizeNew);
    void ImageResize(wxSize imageSizeNew);

    double GetScaleSuitesPage() const;
    double GetScaleSuitesWidth() const;
    double GetScaleSuitesHeight() const;

    cv::Mat GetTransformMatrix() const;

    wxRealPoint Transform(wxRealPoint point) const { return scale * point + wxRealPoint(offset); }
    double TransformX(double x) const { return scale * x + offset.x; }
    double TransformY(double y) const { return scale * y + offset.y; }
    wxRealPoint ReverseTransform(wxRealPoint point) const {
        return (point - wxRealPoint(offset)) / scale;
    }
    double ReverseTransformX(double x) const { return (x - offset.x) / scale; }
    double ReverseTransformY(double y) const { return (y - offset.y) / scale; }

    bool IsInsideImage(wxRealPoint worldPoint) const;

   private:
    friend class Canvas;
    bool modified = true;
};

class Canvas : public wxGLCanvas {
   public:
    Prj* prj = nullptr;
    Prj::Page* page = nullptr;
    ImageScaleModel scaleModel;

   private:
    wxGLContext* context = nullptr;
    GLuint texture;
    cv::UMat uimageSrc;
    wxImage imageDst;
    bool imageModified = false;

   public:
    Canvas(wxWindow* parent, wxWindowID id);
    ~Canvas();
    void SetPrj(Prj& prj) { this->prj = &prj; }
    void SetPage(Prj::Page& pageData);
    void Clear();
    bool IsLoaded() const { return page; }
    ImageScaleModel& GetScaleModel() { return scaleModel; }

    void ZoomIn();
    void ZoomOut();
    void ZoomFit();
    void Zoom(double scale);

   private:
    bool is_deleting = false;
    bool is_mouse_capture = false;
    std::optional<wxPoint> mouse_drag_start;
    std::optional<wxPoint> mouse_position;
    bool initialized = false;
    wxBitmap drawBmp;

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
