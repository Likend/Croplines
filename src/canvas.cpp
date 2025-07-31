#include "canvas.h"

#include <wx/event.h>

#include <cassert>
#include <cstddef>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>

using namespace Croplines;

constexpr double ZOOM_IN_RATE = 1.2;
constexpr double ZOOM_OUT_RATE = 1 / ZOOM_IN_RATE;
constexpr double ZOOM_MIN = 0.1;
constexpr double ZOOM_MAX = 100;

ImageScaleModel::ImageScaleModel(wxSize imageSize, wxSize windowSize,
                                 double scale)
    : imageSize(imageSize), windowSize(windowSize), scale(scale) {
    scaledSize = imageSize * scale;
    offset = wxPoint{} + (windowSize - scaledSize) / 2;
}

ImageScaleModel::ImageScaleModel(wxSize imageSize, wxSize windowSize)
    : imageSize(imageSize), windowSize(windowSize) {
    scale = GetScaleSuitesPage();
    scaledSize = imageSize * scale;
    offset = wxPoint{} + (windowSize - scaledSize) / 2;
}

bool ImageScaleModel::Clip() {
    bool ret = true;
    if (scaledSize.x >= windowSize.x) {
        if (offset.x > 0)
            offset.x = 0;
        else if (offset.x < windowSize.x - scaledSize.x)
            offset.x = windowSize.x - scaledSize.x;
        else
            ret = false;
    } else {
        if (offset.x < 0)
            offset.x = 0;
        else if (offset.x > windowSize.x - scaledSize.x)
            offset.x = windowSize.x - scaledSize.x;
        else
            ret = false;
    }
    if (scaledSize.y >= windowSize.y) {
        if (offset.y > 0)
            offset.y = 0;
        else if (offset.y < windowSize.y - scaledSize.y)
            offset.y = windowSize.y - scaledSize.y;
        else
            ret = false;
    } else {
        if (offset.y < 0)
            offset.y = 0;
        else if (offset.y > windowSize.y - scaledSize.y)
            offset.y = windowSize.y - scaledSize.y;
        else
            ret = false;
    }
    return ret;
}

void ImageScaleModel::Scale(double factor, wxPoint center) {
    double scaleNew = scale * factor;
    if (scaleNew > ZOOM_MAX) {
        factor = ZOOM_MAX / scale;
    } else if (scaleNew < ZOOM_MIN) {
        factor = ZOOM_MIN / scale;
    }
    scale *= factor;
    offset = center + factor * (offset - center);
    scaledSize = imageSize * scale;
    Clip();
}

void ImageScaleModel::Scale(double scale) {
    Scale(scale, wxPoint{} + windowSize / 2);
}

void ImageScaleModel::ScaleTo(double scale, wxPoint center) {
    if (scale > ZOOM_MAX) {
        scale = ZOOM_MAX;
    } else if (scale < ZOOM_MIN) {
        scale = ZOOM_MIN;
    }
    double factor = scale / this->scale;
    this->scale = scale;
    offset = center + factor * (offset - center);
    scaledSize = imageSize * scale;
    Clip();
}

void ImageScaleModel::ScaleTo(double scale) {
    ScaleTo(scale, wxPoint{} + windowSize / 2);
}

void ImageScaleModel::Move(wxPoint dr) {
    offset += dr;
    Clip();
}

void ImageScaleModel::WindowResize(wxSize windowSizeNew) {
    offset += (windowSizeNew - windowSize) / 2;
    windowSize = windowSizeNew;
    Clip();
}

double ImageScaleModel::GetScaleSuitesWidth() const {
    return static_cast<double>(windowSize.GetWidth()) / imageSize.GetWidth();
}

double ImageScaleModel::GetScaleSuitesHeight() const {
    return static_cast<double>(windowSize.GetHeight()) / imageSize.GetHeight();
}

double ImageScaleModel::GetScaleSuitesPage() const {
    return std::min(GetScaleSuitesWidth(), GetScaleSuitesHeight());
}

Canvas::Canvas(wxWindow* parent, wxWindowID id) : wxPanel(parent, id) {
    AlwaysShowScrollbars();

    // disable on default
    SetScrollbar(wxHSCROLL, -1, -1, 1);
    SetScrollbar(wxVSCROLL, -1, -1, -1);

    // cv::ocl::setUseOpenCL(true);
    cv::setUseOptimized(true);
}

Canvas::~Canvas() {}

void Canvas::SetImage(wxImage image) {
    ImageBundle bundle = {
        .imageSrc = image,
        .scaleModel = {image.GetSize(), this->GetClientSize()}};
    // imageSrc = image;
    cv::Mat img(image.GetHeight(), image.GetWidth(), CV_8UC3,
                static_cast<void*>(image.GetData()));
    // uimageSrc = img.getUMat(cv::ACCESS_READ);
    img.copyTo(bundle.uimageSrc);

    imageInfo = bundle;
    Refresh();
}

void Canvas::OnPaint(wxPaintEvent& event) {
    // draw
    wxPaintDC dc(this);

    wxSize windowsSize = dc.GetSize();
    if (windowsSize.GetHeight() == 0 || windowsSize.GetWidth() == 0) {
        return;
    }
    if (imageInfo) {
        // matrix trans
        // [[scale,  0,      offset_x,
        //   0,      scale,  offset_y ]]
        cv::UMat uimageDst(windowsSize.GetHeight(), windowsSize.GetWidth(),
                           CV_8UC3);
        cv::Mat trans =
            (cv::Mat_<double>(2, 3) << imageInfo->scaleModel.scale, 0,
             imageInfo->scaleModel.offset.x, 0, imageInfo->scaleModel.scale,
             imageInfo->scaleModel.offset.y);
        cv::warpAffine(imageInfo->uimageSrc, uimageDst, trans, uimageDst.size(),
                       cv::INTER_AREA);
        cv::Mat imageDst = uimageDst.getMat(cv::ACCESS_READ);

        wxBitmap drawBmp = wxBitmap(wxImage(windowsSize, imageDst.data, true));
        dc.DrawBitmap(drawBmp, 0, 0);
    }
}

void Canvas::OnSize(wxSizeEvent& event) {
    if (imageInfo) imageInfo->scaleModel.WindowResize(event.GetSize());
    Refresh();
    event.Skip();
}

void Canvas::OnMouseWheel(wxMouseEvent& event) {
    double factor;
    if (event.GetWheelRotation() > 0) {  // zoom in
        factor = ZOOM_IN_RATE;
    } else {  // zoom out
        factor = ZOOM_OUT_RATE;
    }
    imageInfo->scaleModel.Scale(factor, event.GetPosition());
    Refresh();
}

void Canvas::OnMouseLeftDown(wxMouseEvent& event) {
    CaptureMouse();
    // m_parent->GetParent()->SetFocus();
    mouse_drag_start = event.GetPosition();
}

void Canvas::OnMouseLeftUp(wxMouseEvent& event) {
    ReleaseMouse();
    mouse_drag_start.reset();
}

void Canvas::OnMouseMotion(wxMouseEvent& event) {
    wxPoint mouse_drag = event.GetPosition();
    if (mouse_drag_start && event.Dragging() && event.LeftIsDown()) {
        auto dr = mouse_drag - *mouse_drag_start;
        imageInfo->scaleModel.Move(dr);
        *mouse_drag_start = mouse_drag;
        Refresh();
    }
}

void Canvas::OnScroll(wxScrollWinEvent& event) {
    const int orientation = event.GetOrientation() | wxORIENTATION_MASK;
    if (orientation & wxHORIZONTAL) {
        const int pos0 = GetScrollPos(wxHORIZONTAL);
        const int pos1 = event.GetPosition();
        imageInfo->scaleModel.Move(wxPoint{pos0 - pos1, 0});
    }
    if (orientation & wxVERTICAL) {
        const int pos0 = GetScrollPos(wxVERTICAL);
        const int pos1 = event.GetPosition();
        imageInfo->scaleModel.Move(wxPoint{0, pos0 - pos1});
    }
}

wxBEGIN_EVENT_TABLE(Canvas, wxPanel)
    // Paint events
    EVT_PAINT(Canvas::OnPaint)
    // Size event
    EVT_SIZE(Canvas::OnSize)
    // Mouse wheel event
    EVT_MOUSEWHEEL(Canvas::OnMouseWheel)
    // Mouse left down
    EVT_LEFT_DOWN(Canvas::OnMouseLeftDown)
    // Mouse left up
    EVT_LEFT_UP(Canvas::OnMouseLeftUp)
    // Mouse motion && Mouse draging
    EVT_MOTION(Canvas::OnMouseMotion)
    // EVT_SCROLLWIN(Canvas::OnScroll)
    // End table
    wxEND_EVENT_TABLE()
