#include "canvas.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

#include <wx/graphics.h>

using namespace Croplines;

constexpr double ZOOM_IN_RATE = 1.2;
constexpr double ZOOM_OUT_RATE = 1 / ZOOM_IN_RATE;
constexpr double ZOOM_MIN = 0.1;
constexpr double ZOOM_MAX = 100;

ImageScaleModel::ImageScaleModel(wxSize imageSize, wxSize windowSize,
                                 double scale)
    : imageSize(imageSize), windowSize(windowSize), scale(scale) {
    scaledSize = imageSize * scale;
    MoveToCenter();
}

ImageScaleModel::ImageScaleModel(wxSize imageSize, wxSize windowSize)
    : imageSize(imageSize), windowSize(windowSize) {
    scale = GetScaleSuitesPage();
    scaledSize = imageSize * scale;
    MoveToCenter();
}

void ImageScaleModel::Clamp() {
    wxSize border = windowSize - scaledSize;
    if (border.x < 0) {
        offset.x = std::clamp(offset.x, border.x, 0);
    } else {
        offset.x = std::clamp(offset.x, 0, border.x);
    }
    if (border.y < 0) {
        offset.y = std::clamp(offset.y, border.y, 0);
    } else {
        offset.y = std::clamp(offset.y, 0, border.y);
    }
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
    Clamp();
    modified = true;
}

void ImageScaleModel::ScaleTo(double scale, wxPoint center) {
    scale = std::clamp(scale, ZOOM_MIN, ZOOM_MAX);
    double factor = scale / this->scale;
    this->scale = scale;
    offset = center + factor * (offset - center);
    scaledSize = imageSize * scale;
    Clamp();
    modified = true;
}

void ImageScaleModel::Move(wxPoint dr) {
    offset += dr;
    Clamp();
    modified = true;
}

void ImageScaleModel::MoveToCenter() {
    offset = wxPoint{} + (windowSize - scaledSize) / 2;
    modified = true;
}

void ImageScaleModel::WindowResize(wxSize windowSizeNew) {
    offset += (windowSizeNew - windowSize) / 2;
    windowSize = windowSizeNew;
    Clamp();
    modified = true;
}

void ImageScaleModel::ImageResize(wxSize imageSizeNew) {
    offset += (imageSize - imageSizeNew) / 2;
    imageSize = imageSizeNew;
    scaledSize = scale * imageSize;
    Clamp();
    modified = true;
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

cv::Mat ImageScaleModel::GetTransformMatrix() const {
    return (cv::Mat_<double>(2, 3) << scale, 0, offset.x, 0, scale, offset.y);
}

bool ImageScaleModel::IsInsideImage(wxRealPoint worldPoint) const {
    wxRealPoint imagePoint = ReverseTransform(worldPoint);
    return 0 <= imagePoint.x && imagePoint.x <= imageSize.GetWidth() &&
           0 <= imagePoint.y && imagePoint.y <= imageSize.GetHeight();
}

Canvas::Canvas(wxWindow* parent, wxWindowID id)
    : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize,
               wxVSCROLL | wxHSCROLL) {
    AlwaysShowScrollbars();

    // // disable on default
    SetScrollbar(wxHORIZONTAL, -1, -1, -1);
    SetScrollbar(wxVERTICAL, -1, -1, -1);

    // cv::ocl::setUseOpenCL(true);
    cv::setUseOptimized(true);
}

Canvas::~Canvas() {}

void Canvas::SetPage(Prj::Page& page) {
    cv::Mat img = prj->LoadPage(page);
    if (img.empty()) {
        return;
    }
    if (IsLoaded()) {
        scaleModel.ImageResize(wxSize{img.cols, img.rows});
    } else {
        scaleModel =
            ImageScaleModel{wxSize{img.cols, img.rows}, this->GetClientSize()};
    }
    this->page = &page;

    // imageSrc = image;
    // cv::Mat img(image.GetHeight(), image.GetWidth(), CV_8UC3,
    //             static_cast<void*>(image.GetData()));
    // uimageSrc = img.getUMat(cv::ACCESS_READ);
    img.copyTo(uimageSrc);
    imageModified = true;
    Refresh();
}

void Canvas::UpdateScrollbars() {
    if (!IsLoaded()) {
        SetScrollbar(wxHORIZONTAL, -1, -1, -1);
        SetScrollbar(wxVERTICAL, -1, -1, -1);
        return;
    }
    if (scaleModel.scaledSize.GetWidth() > scaleModel.windowSize.GetWidth()) {
        SetScrollbar(wxHORIZONTAL, -scaleModel.offset.x,
                     scaleModel.windowSize.GetWidth(),
                     scaleModel.scaledSize.GetWidth());
    } else {
        SetScrollbar(wxHORIZONTAL, -1, -1, -1);
    }
    if (scaleModel.scaledSize.GetHeight() > scaleModel.windowSize.GetHeight()) {
        SetScrollbar(wxVERTICAL, -scaleModel.offset.y,
                     scaleModel.windowSize.GetHeight(),
                     scaleModel.scaledSize.GetHeight());
    } else {
        SetScrollbar(wxVERTICAL, -1, -1, -1);
    }
}

void Canvas::OnPaint(wxPaintEvent& event) {
    // draw
    wxPaintDC dc(this);

    wxSize windowsSize = dc.GetSize();
    if (windowsSize.GetHeight() == 0 || windowsSize.GetWidth() == 0) {
        return;
    }
    if (IsLoaded()) {
        if (scaleModel.modified || imageModified) {
            cv::UMat uimageDst(windowsSize.GetHeight(), windowsSize.GetWidth(),
                               CV_8UC3);
            cv::warpAffine(uimageSrc, uimageDst,
                           scaleModel.GetTransformMatrix(), uimageDst.size(),
                           cv::INTER_LINEAR);
            cv::Mat imageDst = uimageDst.getMat(cv::ACCESS_READ);

            drawBmp = wxBitmap(wxImage(windowsSize, imageDst.data, true));

            UpdateScrollbars();
            scaleModel.modified = false;
            imageModified = false;
        }
        dc.DrawBitmap(drawBmp, 0, 0);

        // Create graphics context from it
        wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
        if (!gc) return;

        // draw crop areas
        gc->SetPen({wxColor(0, 0, 0, 0), 0});
        gc->SetBrush({wxColor(204, 210, 204, 56)});
        for (auto area : prj->GetSelectArea(*page)) {
            auto lt = scaleModel.Transform(
                {static_cast<double>(area.l), static_cast<double>(area.t)});
            auto rb = scaleModel.Transform(
                {static_cast<double>(area.r), static_cast<double>(area.b)});
            auto size = rb - lt;
            gc->DrawRectangle(lt.x, lt.y, size.x, size.y);
        }
        gc->SetBrush(wxNullBrush);

        // draw lines
        double l = scaleModel.offset.x;
        double r = scaleModel.TransformX(scaleModel.imageSize.GetWidth());
        l = std::clamp(l, 0.0, static_cast<double>(windowsSize.GetWidth()));
        r = std::clamp(r, 0.0, static_cast<double>(windowsSize.GetWidth()));

        // draw crop lines
        std::optional<std::uint32_t> deleting_line;
        if (is_deleting && mouse_position) {
            int y = mouse_position->y;
            y = std::lround(scaleModel.ReverseTransformY(y));
            auto search_line =
                page->SearchNearestLine(y, FromDIP(5 / scaleModel.scale + 1));
            if (search_line) {
                deleting_line = **search_line;
                gc->SetPen({wxColor(38, 148, 93, 128), FromDIP(4)});
                double y = scaleModel.TransformY(*deleting_line);
                gc->StrokeLine(l, y, r, y);
            }
        }
        gc->SetPen({wxColor(78, 188, 133, 128), FromDIP(2)});
        for (std::uint32_t line : page->GetCropLines()) {
            if (deleting_line == line) continue;
            double y = scaleModel.TransformY(line);
            if (y >= 0 && y <= windowsSize.GetHeight()) {
                gc->StrokeLine(l, y, r, y);
            }
        }

        // draw mouse line
        if (mouse_position) {
            if (is_deleting)
                gc->SetPen({wxColor(229, 20, 0, 128), FromDIP(2)});
            else
                gc->SetPen({wxColor(86, 156, 214, 128), FromDIP(2)});
            int y = mouse_position->y;
            gc->StrokeLine(l, y, r, y);
        }

        delete gc;
    }
}

void Canvas::OnSize(wxSizeEvent& event) {
    if (IsLoaded()) scaleModel.WindowResize(event.GetSize());
    Refresh();
}

void Canvas::OnMouseWheel(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    double factor;
    if (event.GetWheelRotation() > 0) {  // zoom in
        factor = ZOOM_IN_RATE;
    } else {  // zoom out
        factor = ZOOM_OUT_RATE;
    }
    scaleModel.Scale(factor, event.GetPosition());
    Refresh();
}

void Canvas::OnMouseLeftDown(wxMouseEvent& event) {
    if (!is_mouse_capture) {
        CaptureMouse();
        is_mouse_capture = true;
    }
    // m_parent->GetParent()->SetFocus();
    SetFocus();
    mouse_drag_start = event.GetPosition();
}

void Canvas::OnMouseLeftUp(wxMouseEvent& event) {
    if (is_mouse_capture) {
        ReleaseMouse();
        is_mouse_capture = false;
    }
    mouse_drag_start.reset();
}

void Canvas::OnMouseLeftUp(wxMouseCaptureLostEvent& event) {
    if (is_mouse_capture) {
        ReleaseMouse();
        is_mouse_capture = false;
    }
    mouse_drag_start.reset();
}

void Canvas::OnMouseRightDown(wxMouseEvent& event) {
    SetFocus();
    event.Skip();
}

void Canvas::OnMouseRightUp(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    wxPoint mouse_position = event.GetPosition();
    if (!scaleModel.IsInsideImage(mouse_position)) return;

    int y = mouse_position.y;
    y = std::lround(scaleModel.ReverseTransformY(y));
    if (is_deleting) {
        std::optional<std::set<std::uint32_t>::iterator> it =
            page->SearchNearestLine(y, FromDIP(5 / scaleModel.scale + 1));
        if (it) page->EraseLine(*it);
    } else {
        page->InsertLine(y);
    }
    Refresh();
}

void Canvas::OnMouseMotion(wxMouseEvent& event) {
    if (!IsLoaded()) return;
    wxPoint mouse_drag = event.GetPosition();
    if (mouse_drag_start && event.Dragging()) {
        auto dr = mouse_drag - *mouse_drag_start;
        scaleModel.Move(dr);
        *mouse_drag_start = mouse_drag;
        Refresh();
    }

    // if mouse inside image
    wxPoint mouse_position = event.GetPosition();
    if (scaleModel.IsInsideImage(mouse_position)) {
        this->mouse_position = mouse_position;
        Refresh();
    } else if (this->mouse_position) {
        this->mouse_position = std::nullopt;
        Refresh();
    }
}

void Canvas::OnScroll(wxScrollWinEvent& event) {
    if (!IsLoaded()) return;

    const int orientation = event.GetOrientation() & wxORIENTATION_MASK;
    switch (orientation & wxORIENTATION_MASK) {
        case wxHORIZONTAL: {
            const int pos0 = GetScrollPos(wxHORIZONTAL);
            const int pos1 = event.GetPosition();
            scaleModel.Move(wxPoint{pos0 - pos1, 0});
            Refresh();
            return;
        }
        case wxVERTICAL: {
            const int pos0 = GetScrollPos(wxVERTICAL);
            const int pos1 = event.GetPosition();
            scaleModel.Move(wxPoint{0, pos0 - pos1});
            Refresh();
            return;
        }
            // default:
            //     return;  // ignore other orientations
    }
}

void Canvas::OnKeyUp(wxKeyEvent& event) {
    switch (event.GetKeyCode()) {
        case 'D':
            if (is_deleting == true) {
                is_deleting = false;
                Refresh();
            }
            break;
    }
}

void Canvas::OnKeyDown(wxKeyEvent& event) {
    switch (event.GetKeyCode()) {
        case 'D':
            if (is_deleting == false) {
                is_deleting = true;
                Refresh();
            }
            break;
    }
}

void Canvas::OnKillFocus(wxFocusEvent& event) {
    if (is_deleting) {
        is_deleting = false;
        Refresh();
    }
    if (is_mouse_capture) {
        ReleaseMouse();
        is_mouse_capture = false;
    }
    event.Skip();
}

// clang-format off
wxBEGIN_EVENT_TABLE(Canvas, wxPanel)
    EVT_PAINT(Canvas::OnPaint)
    EVT_SIZE(Canvas::OnSize)

    EVT_MOUSEWHEEL(Canvas::OnMouseWheel)
    EVT_LEFT_DOWN(Canvas::OnMouseLeftDown)
    EVT_LEFT_UP(Canvas::OnMouseLeftUp)
    EVT_RIGHT_DOWN(Canvas::OnMouseRightDown)
    EVT_RIGHT_UP(Canvas::OnMouseRightUp)
    EVT_MOUSE_CAPTURE_LOST(Canvas::OnMouseLeftUp)
    EVT_MOTION(Canvas::OnMouseMotion)

    EVT_SCROLLWIN(Canvas::OnScroll)

    EVT_KEY_DOWN(Canvas::OnKeyDown)
    EVT_KEY_UP(Canvas::OnKeyUp)

    EVT_KILL_FOCUS(Canvas::OnKillFocus)
wxEND_EVENT_TABLE();