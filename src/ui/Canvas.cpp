#include "ui/Canvas.hpp"

#include <memory>

#include <wx/cmdproc.h>

using namespace croplines;

Canvas::Canvas(wxWindow* parent, wxWindowID id)
    : wxGLCanvas(parent, id, nullptr, wxDefaultPosition, wxDefaultSize,
                 wxFULL_REPAINT_ON_RESIZE | wxVSCROLL | wxHSCROLL),
      m_glContext(std::make_unique<wxGLContext>(this)) {
    AlwaysShowScrollbars();

    // disable on default
    SetScrollbar(wxHORIZONTAL, -1, -1, -1);
    SetScrollbar(wxVERTICAL, -1, -1, -1);
}

Canvas::~Canvas() {
    if (m_glTexture) glDeleteTextures(1, &m_glTexture);
}

static void SetTextrue(GLuint texture, void* pixels, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // 解决glTexImage2D崩溃问题
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void Canvas::SetPage(Page& page) {
    wxImage img = page.GetImage();
    if (!img.IsOk()) {
        return;
    }
    if (IsLoaded()) {
        GetScaleModel().OnImageResize(img.GetSize());
    } else {
        m_scaleModel = ImageScaleModel{img.GetSize(), GetClientSize()};
    }
    m_page = &page;

    SetCurrent(*m_glContext);
    if (m_glTexture) glDeleteTextures(1, &m_glTexture);
    glGenTextures(1, &m_glTexture);
    SetTextrue(m_glTexture, img.GetData(), img.GetWidth(), img.GetHeight());
    Refresh();
}

void Canvas::Clear() {
    if (m_glTexture) glDeleteTextures(1, &m_glTexture);
    m_page = nullptr;
    m_scaleModel.reset();
    Refresh();
}

void Canvas::ZoomIn() {
    if (!IsLoaded()) return;
    GetScaleModel().Scale(ImageScaleModel::ZOOM_IN_RATE);
    Refresh();
}

void Canvas::ZoomOut() {
    if (!IsLoaded()) return;
    GetScaleModel().Scale(ImageScaleModel::ZOOM_OUT_RATE);
    Refresh();
}

void Canvas::ZoomFit() {
    if (!IsLoaded()) return;
    GetScaleModel().ScaleTo(GetScaleModel().GetScaleSuitesPage());
    GetScaleModel().MoveToCenter();
    Refresh();
}

void Canvas::Zoom(double scale) {
    if (IsLoaded()) {
        GetScaleModel().ScaleTo(scale);
        Refresh();
    }
}

void Canvas::UpdateScrollbars() {
    if (!IsLoaded()) {
        SetScrollbar(wxHORIZONTAL, -1, -1, -1);
        SetScrollbar(wxVERTICAL, -1, -1, -1);
        return;
    }
    if (GetScaleModel().scaledSize.GetWidth() > GetScaleModel().windowSize.GetWidth()) {
        SetScrollbar(wxHORIZONTAL, -GetScaleModel().offset.x, GetScaleModel().windowSize.GetWidth(),
                     GetScaleModel().scaledSize.GetWidth());
    } else {
        SetScrollbar(wxHORIZONTAL, -1, -1, -1);
    }
    if (GetScaleModel().scaledSize.GetHeight() > GetScaleModel().windowSize.GetHeight()) {
        SetScrollbar(wxVERTICAL, -GetScaleModel().offset.y, GetScaleModel().windowSize.GetHeight(),
                     GetScaleModel().scaledSize.GetHeight());
    } else {
        SetScrollbar(wxVERTICAL, -1, -1, -1);
    }
}

static void InitGL() {
    // 设置OpenGL状态
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
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

static void UpdateProjection(wxSize size) {
    if (size.GetWidth() <= 0 || size.GetHeight() <= 0) return;

    glViewport(0, 0, size.GetWidth(), size.GetHeight());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.GetWidth(), size.GetHeight(), 0, -1, 1);
}

void Canvas::OnPaint(wxPaintEvent&) {
    wxPaintDC dc(this);

    SetCurrent(*m_glContext);

    // 初始化OpenGL（首次绘制时执行）
    if (!m_isInitialized) {
        InitGL();
        m_isInitialized = true;
    }

    // 清除缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置正交投影（模拟视口坐标系）
    UpdateProjection(GetClientSize());

    // 设置投影（透视投影）
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(GetScaleModel().offset.x, GetScaleModel().offset.y, 0);
    glScaled(GetScaleModel().scale, GetScaleModel().scale, 1.0);

    if (IsLoaded()) {
        // 绑定纹理
        wxSize size = GetScaleModel().imageSize;
        glBegin(GL_QUADS);
        glColor3d(1.0, 1.0, 1.0);
        glBindTexture(GL_TEXTURE_2D, m_glTexture);
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

        glColor4d(0.8, 0.84, 0.8, 0.25);
        for (const wxRect& area : m_page->getSelectAreas()) {
            glBegin(GL_QUADS);
            glVertex2d(area.GetLeft(), area.GetTop());
            glVertex2d(area.GetLeft(), area.GetBottom());
            glVertex2d(area.GetRight(), area.GetBottom());
            glVertex2d(area.GetRight(), area.GetTop());
            glEnd();
        }

        auto DrawLine = [&size, this](int width, double line_y) {
            double w = FromDIP(width) / GetScaleModel().scale;
            glBegin(GL_QUADS);
            glVertex2d(0, line_y - w);
            glVertex2d(0, line_y + w);
            glVertex2d(size.GetWidth(), line_y + w);
            glVertex2d(size.GetWidth(), line_y - w);
            glEnd();
        };

        // draw crop lines
        std::optional<int> deleting_line;
        if (m_isDeleting && m_mouseCurrentPosition) {
            int y = m_mouseCurrentPosition->y;
            y     = std::lround(GetScaleModel().ReverseTransformY(y));

            auto search_line = m_page->SearchNearestLine(
                y, FromDIP(static_cast<int>(5.0 / GetScaleModel().scale) + 1));
            if (search_line.has_value()) {
                deleting_line = *search_line;
                glColor4d(0.15, 0.58, 0.36, 0.5);
                DrawLine(4, *deleting_line);
            }
        }

        glColor4d(0.30, 0.74, 0.52, 0.5);
        for (int line : m_page->GetCropLines()) {
            if (deleting_line == line) continue;
            DrawLine(2, line);
        }

        // draw mouse line
        if (m_mouseCurrentPosition) {
            if (m_isDeleting)
                glColor4d(0.90, 0.08, 0, 0.5);
            else
                glColor4d(0.34, 0.61, 0.84, 0.5);
            double y = GetScaleModel().ReverseTransformY(m_mouseCurrentPosition->y);
            DrawLine(2, y);
        }
    }

    SwapBuffers();
}

void Canvas::OnSize(wxSizeEvent& event) {
    if (m_glContext) {
        SetCurrent(*m_glContext);
        UpdateProjection(GetClientSize());
    }
    if (IsLoaded()) GetScaleModel().OnWindowResize(event.GetSize());
    Refresh();
}

void Canvas::OnMouseWheel(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    double factor;
    if (event.GetWheelRotation() > 0) {  // zoom in
        factor = ImageScaleModel::ZOOM_IN_RATE;
    } else {  // zoom out
        factor = ImageScaleModel::ZOOM_OUT_RATE;
    }
    GetScaleModel().Scale(factor, event.GetPosition());
    Refresh();
}

void Canvas::OnMouseLeftDown(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    if (!m_isMouseCaptured) {
        CaptureMouse();
        m_isMouseCaptured = true;
    }
    // m_parent->GetParent()->SetFocus();
    SetFocus();
    m_mouseDragStartPosition = event.GetPosition();
}

void Canvas::OnMouseLeftUp(wxMouseEvent&) {
    if (!IsLoaded()) return;

    if (m_isMouseCaptured) {
        ReleaseMouse();
        m_isMouseCaptured = false;
    }
    m_mouseDragStartPosition.reset();
}

void Canvas::OnMouseLeftUp(wxMouseCaptureLostEvent&) {
    if (!IsLoaded()) return;

    if (m_isMouseCaptured) {
        ReleaseMouse();
        m_isMouseCaptured = false;
    }
    m_mouseDragStartPosition.reset();
}

void Canvas::OnMouseRightDown(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    SetFocus();
    event.Skip();
}

struct EraseLineCommand final : public wxCommand {
    Page& page;
    int   line;
    EraseLineCommand(Page& page, int line)
        : wxCommand(true, wxT("删除直线")), page(page), line(line) {}
    bool Do() override { return page.EraseLine(line); }
    bool Undo() override { return page.InsertLine(line); }
};

struct InsertLineCommand final : public wxCommand {
    Page& page;
    int   line;
    InsertLineCommand(Page& page, int line)
        : wxCommand(true, wxT("添加直线")), page(page), line(line) {}
    bool Do() override { return page.InsertLine(line); }
    bool Undo() override { return page.EraseLine(line); }
};

void Canvas::OnMouseRightUp(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    wxPoint mousePosition = event.GetPosition();
    if (!GetScaleModel().IsInsideImage(mousePosition)) return;

    int y = mousePosition.y;
    y     = std::lround(GetScaleModel().ReverseTransformY(y));
    if (m_isDeleting) {
        int  threshold = static_cast<int>(5.0 / GetScaleModel().scale) + 1;
        auto line      = m_page->SearchNearestLine(y, FromDIP(threshold));
        if (line.has_value()) GetProcessor()->Submit(new EraseLineCommand{GetPage(), *line});
    } else {
        GetProcessor()->Submit(new InsertLineCommand{GetPage(), y});
    }
    Refresh();
}

void Canvas::OnMouseMotion(wxMouseEvent& event) {
    if (!IsLoaded()) return;

    wxPoint mouse_drag = event.GetPosition();
    if (m_mouseDragStartPosition && event.Dragging()) {
        auto dr = mouse_drag - *m_mouseDragStartPosition;
        GetScaleModel().Move(dr);
        *m_mouseDragStartPosition = mouse_drag;
        Refresh();
    }

    // if mouse inside image
    wxPoint mouse_position = event.GetPosition();
    if (GetScaleModel().IsInsideImage(mouse_position)) {
        m_mouseCurrentPosition = mouse_position;
        Refresh();
    } else if (m_mouseCurrentPosition) {
        m_mouseCurrentPosition = std::nullopt;
        Refresh();
    }
}

void Canvas::OnScroll(wxScrollWinEvent& event) {
    if (!IsLoaded()) return;

    const unsigned orientation = static_cast<unsigned>(event.GetOrientation()) & wxORIENTATION_MASK;
    switch (orientation & wxORIENTATION_MASK) {
        case wxHORIZONTAL: {
            const int pos0 = GetScrollPos(wxHORIZONTAL);
            const int pos1 = event.GetPosition();
            GetScaleModel().Move(wxPoint{pos0 - pos1, 0});
            Refresh();
            return;
        }
        case wxVERTICAL: {
            const int pos0 = GetScrollPos(wxVERTICAL);
            const int pos1 = event.GetPosition();
            GetScaleModel().Move(wxPoint{0, pos0 - pos1});
            Refresh();
            return;
        }
        default:
            return;  // ignore other orientations
    }
}

void Canvas::OnKeyUp(wxKeyEvent& event) {
    if (!IsLoaded()) return;

    switch (event.GetKeyCode()) {
        case 'D':
            if (m_isDeleting) {
                m_isDeleting = false;
                Refresh();
            }
            break;
    }
}

void Canvas::OnKeyDown(wxKeyEvent& event) {
    if (!IsLoaded()) return;

    switch (event.GetKeyCode()) {
        case 'D':
            if (!m_isDeleting) {
                m_isDeleting = true;
                Refresh();
            }
            break;
    }
}

void Canvas::OnKillFocus(wxFocusEvent& event) {
    if (!IsLoaded()) return;

    if (m_isDeleting) {
        m_isDeleting = false;
        Refresh();
    }
    if (m_isMouseCaptured) {
        ReleaseMouse();
        m_isMouseCaptured = false;
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
