#include "canvas.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

#include <GL/gl.h>
#include <wx/graphics.h>

#include "config.h"

using namespace Croplines;

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
    if (scaleNew > Config::zoom_max) {
        factor = Config::zoom_max / scale;
    } else if (scaleNew < Config::zoom_min) {
        factor = Config::zoom_min / scale;
    }
    scale *= factor;
    offset = center + factor * (offset - center);
    scaledSize = imageSize * scale;
    Clamp();
    modified = true;
}

void ImageScaleModel::ScaleTo(double scale, wxPoint center) {
    scale = std::clamp(scale, Config::zoom_min, Config::zoom_max);
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
    : wxGLCanvas(parent, id, nullptr, wxDefaultPosition, wxDefaultSize,
                 wxFULL_REPAINT_ON_RESIZE | wxVSCROLL | wxHSCROLL) {
    AlwaysShowScrollbars();

    // // disable on default
    SetScrollbar(wxHORIZONTAL, -1, -1, -1);
    SetScrollbar(wxVERTICAL, -1, -1, -1);

    context = new wxGLContext(this);
}

Canvas::~Canvas() {
    if (texture) glDeleteTextures(1, &texture);
    if (context) delete context;
}

static void SetTextrue(GLuint texture, void* pixels, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // 解决glTexImage2D崩溃问题
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, pixels);
}

void Canvas::SetPage(Prj::Page& page) {
    wxImage img = prj->LoadPage(page);
    if (!img.IsOk()) {
        return;
    }
    if (IsLoaded()) {
        scaleModel.ImageResize(img.GetSize());
    } else {
        scaleModel = ImageScaleModel{img.GetSize(), this->GetClientSize()};
    }
    this->page = &page;

    SetCurrent(*context);
    if (texture) glDeleteTextures(1, &texture);
    glGenTextures(1, &texture);
    SetTextrue(texture, img.GetData(), img.GetWidth(), img.GetHeight());
    Refresh();
}

void Canvas::Clear() {
    if (texture) glDeleteTextures(1, &texture);
    prj = nullptr;
    page = nullptr;
    Refresh();
}

void Canvas::ZoomIn() {
    if (!IsLoaded()) return;
    scaleModel.Scale(Config::zoom_in_rate);
    Refresh();
}

void Canvas::ZoomOut() {
    if (!IsLoaded()) return;
    scaleModel.Scale(Config::zoom_out_rate);
    Refresh();
}

void Canvas::ZoomFit() {
    if (!IsLoaded()) return;
    scaleModel.ScaleTo(scaleModel.GetScaleSuitesPage());
    scaleModel.MoveToCenter();
    Refresh();
}

void Canvas::Zoom(double scale) {
    if (IsLoaded()) {
        scaleModel.ScaleTo(scale);
        Refresh();
    }
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

static void InitGL() {
    // 设置OpenGL状态
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    // 设置深度测试函数
    glDepthFunc(GL_LEQUAL);
    // GL_SMOOTH(光滑着色)/GL_FLAT(恒定着色)
    glShadeModel(GL_SMOOTH);
    // 开启混合
    glEnable(GL_BLEND);
    // 设置混合函数
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);  // 启用Alpha测试
    // 设置Alpha测试条件为大于0.05则通过
    glAlphaFunc(GL_GREATER, 0.05);
    // 设置逆时针索引为正面（GL_CCW/GL_CW）
    glFrontFace(GL_CW);
    // 开启线段反走样
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void UpdateProjection(wxSize size) {
    if (size.GetWidth() <= 0 || size.GetHeight() <= 0) return;

    glViewport(0, 0, size.GetWidth(), size.GetHeight());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.GetWidth(), size.GetHeight(), 0, -1, 1);
}

void Canvas::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);

    SetCurrent(*context);

    // 初始化OpenGL（首次绘制时执行）
    if (!initialized) {
        InitGL();
        initialized = true;
    }

    // 清除缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置正交投影（模拟视口坐标系）
    UpdateProjection(GetClientSize());

    // 设置投影（透视投影）
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(scaleModel.offset.x, scaleModel.offset.y, 0);
    glScalef(scaleModel.scale, scaleModel.scale, 1.0);

    if (IsLoaded()) {
        // 绑定纹理
        wxSize size = scaleModel.imageSize;
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexCoord2d(0, 0);
        glVertex2d(0, 0);
        glTexCoord2d(1, 0);
        glVertex2d(size.GetWidth(), 0);
        glTexCoord2d(1, 1);
        glVertex2d(size.GetWidth(), size.GetHeight());
        glTexCoord2d(0, 1);
        glVertex2d(0, size.GetHeight());
        glEnd();

        UpdateScrollbars();

        glColor4f(0.8, 0.84, 0.8, 0.25);
        for (auto area : prj->GetSelectArea(*page)) {
            glBegin(GL_QUADS);
            glVertex2d(area.GetLeft(), area.GetTop());
            glVertex2d(area.GetLeft(), area.GetBottom());
            glVertex2d(area.GetRight(), area.GetBottom());
            glVertex2d(area.GetRight(), area.GetTop());
            glEnd();
        }

        auto DrawLine = [&size, this](int width, double line_y) {
            double w = FromDIP(width) / scaleModel.scale;
            glBegin(GL_QUADS);
            glVertex2d(0, line_y - w);
            glVertex2d(0, line_y + w);
            glVertex2d(size.GetWidth(), line_y + w);
            glVertex2d(size.GetWidth(), line_y - w);
            glEnd();
        };

        // draw crop lines
        std::optional<std::uint32_t> deleting_line;
        if (is_deleting && mouse_position) {
            int y = mouse_position->y;
            y = std::lround(scaleModel.ReverseTransformY(y));
            auto search_line =
                page->SearchNearestLine(y, FromDIP(5 / scaleModel.scale + 1));
            if (search_line) {
                deleting_line = **search_line;
                glColor4f(0.15, 0.58, 0.36, 0.5);
                DrawLine(4, *deleting_line);
            }
        }

        glColor4f(0.30, 0.74, 0.52, 0.5);
        for (std::uint32_t line : page->GetCropLines()) {
            if (deleting_line == line) continue;
            DrawLine(2, line);
        }

        // draw mouse line
        if (mouse_position) {
            if (is_deleting)
                glColor4f(0.90, 0.08, 0, 0.5);
            else
                glColor4f(0.34, 0.61, 0.84, 0.5);
            double y = scaleModel.ReverseTransformY(mouse_position->y);
            DrawLine(2, y);
        }
    }

    SwapBuffers();
}

void Canvas::OnSize(wxSizeEvent& event) {
    if (context) {
        SetCurrent(*context);
        UpdateProjection(GetClientSize());
    }
    if (IsLoaded()) scaleModel.WindowResize(event.GetSize());
    Refresh();
}

void Canvas::OnMouseWheel(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    double factor;
    if (event.GetWheelRotation() > 0) {  // zoom in
        factor = Config::zoom_in_rate;
    } else {  // zoom out
        factor = Config::zoom_out_rate;
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
        if (it) prj->Execute(Prj::EraseLineRecord(*it, *page));
    } else {
        prj->Execute(Prj::InsertLineRecord(y, *page));
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
