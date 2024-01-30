# -*- coding: utf-8 -*-
# edit base on source https://blog.csdn.net/xufive/article/details/97020456
import wx
from wx import glcanvas
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.raw.GLU import gluUnProject, gluProject
import cv2
import numpy as np
from typing import overload, Any, Optional
from collections.abc import Callable
from threading import Thread
from prj import Prj


# const config
ZOOM_MIN = 0.1
ZOOM_MAX = 100
ZOOM_IN_RATE = 1.1
ZOOM_OUT_RATE = 0.9
VIEW_ZNEAR = 5
VIEW_ZFAR = 10
DELETE_LINE_FILTER = 5  # 删除横线屏幕像素误差阈值


class WxGLScene(glcanvas.GLCanvas, wx.Window):
    """GL场景类"""

    def __init__(self, parent):
        """构造函数

        parent      - 父级窗口对象
        """

        glcanvas.GLCanvas.__init__(self, parent, -1, style=glcanvas.WX_GL_RGBA |
                                   glcanvas.WX_GL_DOUBLEBUFFER | glcanvas.WX_GL_DEPTH_SIZE | wx.VSCROLL | wx.HSCROLL)

        self.parent = parent  # 父级窗口对象

        self.AlwaysShowScrollbars()  # 滑动条始终可见
        self.SetScrollbar(wx.HORIZONTAL, position=-1,
                          thumbSize=-1, range=-1)  # 默认禁用滑动条
        self.SetScrollbar(wx.VERTICAL, position=-1, thumbSize=-1, range=-1)

        self.viewport_size: wx.Size = self.GetClientSize()        # OpenGL窗口的大小
        self.context = glcanvas.GLContext(self)  # OpenGL上下文

        self.trans_mat = np.mat(np.identity(3, dtype=np.float64))  # 三阶单位矩阵
        self.zoom = 1.0    # 视口缩放因子
        self.mpos_dragstart: Optional[wx.Point] = None   # 鼠标位置 拖拽图片最开始的位置

        self.draw_mouse_x = None
        self.draw_mouse_y = None

        self.prj = None
        self.canvas_lines: list[float] = []
        self.prj_curr_lines_ls: list[int] = []
        '''The line list of current page from Prj, please do not directly act on it.'''

        self.set_page_handler: Optional[Callable[[int], Any]] = None
        '''The call back function of `self.setPage`, activated only when using `self.bindPrj`'''

        self.canvas_select_areas = []

        self.pic = None
        self.pic_textrue = None

        self.Bind(wx.EVT_SIZE, self.onResize)   # 绑定窗口尺寸改变事件
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.onErase)  # 绑定背景擦除事件
        self.Bind(wx.EVT_PAINT, self.onPaint)   # 绑定重绘事件

        # keyboard event
        self.is_D_key_down = False
        self.is_DOWN_key_down = False
        self.is_UP_key_down = False

        self.initGL()      # 画布初始化

    def onResize(self, event):
        """响应窗口尺寸改变事件"""
        if self.context:
            self.SetCurrent(self.context)
            self.viewport_size = self.GetClientSize()
            self.Refresh(False)

        event.Skip()

    def onErase(self, event):
        """响应背景擦除事件"""
        pass

    def onPaint(self, event: Optional[wx.PaintEvent] = None):
        """响应重绘事件"""
        self.SetCurrent(self.context)
        self.drawGL()                                       # 绘图
        self.SwapBuffers()                                  # 切换缓冲区，以显示绘制内容
        if isinstance(event, wx.PaintEvent):
            event.Skip()

    def initGL(self):
        """初始化GL"""

        self.SetCurrent(self.context)

        # 设置画布背景色
        glClearColor(0, 0, 0, 0)
        # 开启深度测试，实现遮挡关系
        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LEQUAL)                                      # 设置深度测试函数
        # GL_SMOOTH(光滑着色)/GL_FLAT(恒定着色)
        glShadeModel(GL_SMOOTH)
        glEnable(GL_BLEND)                                          # 开启混合
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)           # 设置混合函数
        glEnable(GL_ALPHA_TEST)                                     # 启用Alpha测试
        # 设置Alpha测试条件为大于0.05则通过
        glAlphaFunc(GL_GREATER, 0.05)
        # 设置逆时针索引为正面（GL_CCW/GL_CW）
        glFrontFace(GL_CW)
        glEnable(GL_LINE_SMOOTH)                                    # 开启线段反走样
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST)
        glEnable(GL_TEXTURE_2D)
        # 创建纹理

        # 设置纹理参数

    def setTextrue(self):
        if isinstance(self.pic, type(None)):
            return
        if not isinstance(self.pic_textrue, type(None)):
            glDeleteTextures(1, [self.pic_textrue])
        self.pic_textrue = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.pic_textrue)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)  # 解决glTexImage2D崩溃问题
        glTexImage2D(GL_TEXTURE_2D, 0, 3, self.pic.shape[1], self.pic.shape[0],
                     0, GL_BGR, GL_UNSIGNED_BYTE, self.pic)

    def __del__(self):
        if not isinstance(self.pic_textrue, type(None)):
            glDeleteTextures(1, [self.pic_textrue])
            # print("safe delete textrue!")

    def drawGL(self):
        """绘制"""

        # 清除屏幕及深度缓存
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        # 设置视口
        glViewport(0, 0, self.viewport_size[0], self.viewport_size[1])

        # 设置投影（透视投影）
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()

        if isinstance(self.pic_textrue, type(None)) or isinstance(self.pic, type(None)):
            return

        w_rate = self.pic.shape[1]/self.viewport_size[0]
        h_rate = self.pic.shape[0]/self.viewport_size[1]
        k = w_rate/h_rate
        if k < 1:  # 此时高度铺满屏幕
            proj_mat = np.mat(np.diag([1/k, 1, 1]))
        else:  # 此时长度铺满屏幕
            proj_mat = np.mat(np.diag([1, k, 1]))
        trans_mat_final = self.trans_mat * proj_mat
        view_right, view_top, _ = trans_mat_final * \
            np.array([[1], [1], [1]], dtype=np.float64)
        view_left, view_bottom, _ = trans_mat_final * \
            np.array([[-1], [-1], [1]], dtype=np.float64)
        glFrustum(
            view_left[0, 0],
            view_right[0, 0],
            view_bottom[0, 0],
            view_top[0, 0],
            VIEW_ZNEAR, VIEW_ZFAR
        )

        # 设置视点
        gluLookAt(
            0, 0, 5,  # 观察者的位置
            0, 0, 0,  # 观察目标（默认在坐标原点）
            0, 1, 0  # 对观察者而言的上方
        )

        # 开始绘制
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0)
        glVertex2f(-1.0, -1.0)
        glTexCoord2f(1.0, 0.0)
        glVertex2f(1.0, -1.0)
        glTexCoord2f(1.0, 1.0)
        glVertex2f(1.0, 1.0)
        glTexCoord2f(0.0, 1.0)
        glVertex2f(-1.0, 1.0)
        glEnd()

        # 绘制选择区域
        for ((l, t), (r, b)) in self.canvas_select_areas:
            glBegin(GL_QUADS)
            glColor4f(0, 0.5, 0, 0.2)

            glVertex2f(l, t)
            glVertex2f(r, t)
            glVertex2f(r, b)
            glVertex2f(l, b)
            glEnd()

        glLineWidth(4)
        glBegin(GL_LINES)
        # 绘制鼠标所在的水平横线
        if self.draw_mouse_x is not None and self.draw_mouse_y is not None:
            glColor4f(0, 0, 1, 0.8)
            glVertex2f(-1, self.draw_mouse_y)
            glVertex2f(1, self.draw_mouse_y)

        # 绘制已储存的横线
        glColor4f(0, 1, 0, 0.8)
        for line in self.canvas_lines:
            glVertex2f(-1, line)
            glVertex2f(1, line)

        # 颜色恢复
        glColor4f(1, 1, 1, 1)
        glEnd()

    @overload
    def scale(self, x: float = 0.0, y: float = 0.0, zoom: float = 1, mouse_stick=False):
        ...

    @overload
    def scale(self,  x: float, y: float, scale: float):
        ...

    def scale(self,  x: float = 0.0, y: float = 0.0, zoom: Optional[float] = None, scale: Optional[float] = None, mouse_stick=False):
        '''缩放'''
        if not self.checkCoordInside(x, y):
            return
        if not isinstance(zoom, type(None)):
            self.zoom = zoom
            if mouse_stick:
                self.trans_mat[0, 0] = zoom
                self.trans_mat[1, 1] = zoom
            else:
                self.trans_mat = np.mat([[zoom, 0, x*(1-zoom)],
                                         [0, zoom, y*(1-zoom)],
                                         [0, 0, 1]],
                                        dtype=np.float64)

        elif not isinstance(scale, type(None)):
            self.zoom *= scale
            scale_mat = np.mat([[scale, 0, x*(1-scale)],
                                [0, scale, y*(1-scale)],
                                [0, 0, 1]],
                               dtype=np.float64)
            self.trans_mat = scale_mat*self.trans_mat

        # reset the scrollbar
        canvas_left, canvas_bottom = self.getWinCoord(-1, -1)
        canvas_right, canvas_top = self.getWinCoord(1, 1)
        if canvas_left >= 0:
            canvas_left = 0
        if canvas_top >= 0:
            canvas_top = 0
        if canvas_right <= self.viewport_size[0]:
            canvas_right = self.viewport_size[0]
        if canvas_bottom <= self.viewport_size[1]:
            canvas_bottom = self.viewport_size[1]
        if canvas_left == 0 and canvas_right == self.viewport_size[0]:
            self.SetScrollbar(wx.HORIZONTAL, position=-1,
                              thumbSize=-1, range=-1)  # 满屏，禁用滑动条
        else:
            self.SetScrollbar(wx.HORIZONTAL, position=-canvas_left,
                              thumbSize=self.viewport_size[0], range=canvas_right-canvas_left)
            # print("self.SetScrollbar wx.HORIZONTAL", "position=", -canvas_left,
            #       "thumbSize=", self.viewport_size[0], "range=", canvas_right-canvas_left)
        if canvas_bottom == self.viewport_size[1] and canvas_top == 0:
            self.SetScrollbar(wx.VERTICAL, position=-1,
                              thumbSize=-1, range=-1)  # 满屏，禁用滑动条
        else:
            self.SetScrollbar(wx.VERTICAL, position=-canvas_top,
                              thumbSize=self.viewport_size[1], range=canvas_bottom-canvas_top)
            # print("self.SetScrollbar wx.VERTICAL",  "position=", -canvas_top,
            #       "thumbSize=", self.viewport_size[1], "range=", canvas_bottom-canvas_top)

        self.Refresh(False)

    def move(self, dx: float, dy: float, refresh=True):
        '''平移 dx, dy
        dx>0向右
        dy>0向上
        '''
        move_mat = np.mat(np.array([[1, 0, -dx],
                                    [0, 1, -dy],
                                    [0, 0, 1]],
                                   dtype=np.float64))
        self.trans_mat = move_mat * self.trans_mat

        # reset the scrollbar
        win_p1 = self.getWinCoord(dx, dy)
        win_p0 = self.getWinCoord(0, 0)

        pos_h_0 = self.GetScrollPos(wx.HORIZONTAL)
        pos_v_0 = self.GetScrollPos(wx.VERTICAL)
        self.SetScrollPos(wx.HORIZONTAL, pos=pos_h_0+win_p0[0]-win_p1[0])
        self.SetScrollPos(wx.VERTICAL, pos=pos_v_0+win_p0[1]-win_p1[1])
        if refresh:
            self.Refresh(False)

    def getWorldCoord(self, wxwin_x: int, wxwin_y: int) -> tuple[float, float]:
        '''从wxwindow屏幕坐标到gl的世界坐标
        采用gluUnProject方法
        （需注意：wx窗口的坐标原点是左上角，而gl的坐标原点是左下角）'''
        modelview = glGetDoublev(GL_MODELVIEW_MATRIX)
        projection = glGetDoublev(GL_PROJECTION_MATRIX)
        viewport = np.array([0, 0, self.viewport_size[0], self.viewport_size[1]],
                            dtype=np.int32)
        win_x = float(wxwin_x)
        win_y = float(self.viewport_size[1]-wxwin_y)
        win_z = 1.0

        obj_x = ctypes.c_double()
        obj_y = ctypes.c_double()
        obj_z = ctypes.c_double()
        gluUnProject(win_x, win_y, win_z, modelview,
                     projection, viewport, obj_x, obj_y, obj_z)
        # print("gluUnProject", win_x, win_y, win_z, obj_x, obj_y, obj_z)
        return obj_x.value/2, obj_y.value/2

    def getWinCoord(self, world_x: float, world_y: float) -> tuple[int, int]:
        '''从gl的世界坐标到wxwindow屏幕坐标
        采用gluProject方法
        （需注意：wx窗口的坐标原点是左上角，而gl的坐标原点是左下角）'''
        obj_x = world_x * 2
        obj_y = world_y * 2
        obj_z = -VIEW_ZNEAR
        win_x = ctypes.c_double()
        win_y = ctypes.c_double()
        win_z = ctypes.c_double()
        modelview = glGetDoublev(GL_MODELVIEW_MATRIX)
        projection = glGetDoublev(GL_PROJECTION_MATRIX)
        viewport = np.array([0, 0, self.viewport_size[0], self.viewport_size[1]],
                            dtype=np.int32)
        gluProject(obj_x, obj_y, obj_z, modelview,
                   projection, viewport, win_x, win_y, win_z)
        # print("gluProject", win_x, win_y, win_z, obj_x, obj_y, obj_z)

        win_x = win_x.value
        win_y = self.viewport_size[1] - win_y.value
        return int(win_x + 0.5), int(win_y + 0.5)  # 四舍五入

    def checkCoordInside(self, x, y):
        return -1 <= x <= 1 and -1 <= y <= 1

    def onMouseWheel(self, event: wx.MouseEvent):
        """响应鼠标滚轮事件 Zoom with mouse wheel"""
        x = event.x
        y = event.y
        obj_x, obj_y = self.getWorldCoord(x, y)

        if event.WheelRotation < 0:
            if self.zoom * ZOOM_IN_RATE > ZOOM_MAX:
                self.scale(zoom=ZOOM_MAX, mouse_stick=True)
            else:
                self.scale(obj_x, obj_y, scale=ZOOM_IN_RATE)
        elif event.WheelRotation > 0:
            if self.zoom * ZOOM_OUT_RATE < ZOOM_MIN:
                self.scale(zoom=ZOOM_MIN, mouse_stick=True)
            else:
                self.scale(obj_x, obj_y, scale=ZOOM_OUT_RATE)

    def onLeftDown(self, event: wx.MouseEvent):
        """响应鼠标左键按下事件"""
        self.CaptureMouse()
        self.mpos_dragstart = event.Position
        self.parent.Parent.SetFocus()

    def onLeftUp(self, event):
        """响应鼠标左键弹起事件"""
        try:
            self.ReleaseMouse()
        except:
            pass

    def onRightUp(self, event: wx.MouseEvent):
        """响应鼠标右键弹起事件"""
        self.parent.Parent.SetFocus()
        mpos = event.Position
        x, y = self.getWorldCoord(*tuple(mpos))
        if not self.checkCoordInside(x, y):
            return
        # print("onRightUp", self.is_D_down)
        if self.is_D_key_down:
            win_y_max = mpos.y + DELETE_LINE_FILTER  # 删除横线的最大纵坐标
            win_y_min = mpos.y - DELETE_LINE_FILTER  # 删除横线的最小纵坐标
            for i in range(len(self.prj_curr_lines_ls)):
                _, win_y = self.getWinCoord(0, self.canvas_lines[i])
                if win_y_min <= win_y <= win_y_max:
                    self.prj.line_listsPop(self.prj.curr_page, i)
                    self.canvas_lines.pop(i)
                    self.refreshSelectAreas()
                    break
        else:
            self.canvas_lines.append(y)
            self.prj.line_listsAppend(self.prj.curr_page,
                                      int(self.pic.shape[0] * (1-y) / 2 + 0.5))
            self.refreshSelectAreas()

    def onMouseMotion(self, event: wx.MouseEvent):
        """响应鼠标移动事件 拖拽图片
        """
        mpos: wx.Point = event.Position
        is_dragging = False
        if self.mpos_dragstart is not None:
            mpos_dragstart_coord = self.getWorldCoord(
                *tuple(self.mpos_dragstart))
            if event.Dragging() and event.LeftIsDown() and self.checkCoordInside(*mpos_dragstart_coord):
                is_dragging = True
                win_dx, win_dy = tuple(mpos - self.mpos_dragstart)
                canvas_x, canvas_y = self.getWorldCoord(win_dx, win_dy)
                canvas_x0, canvas_y0 = self.getWorldCoord(0, 0)
                dx, dy = canvas_x - canvas_x0, canvas_y - canvas_y0

                # not alow img come across the border
                canvas_left, canvas_top = self.getWorldCoord(0, 0)
                canvas_right, canvas_bottom = self.getWorldCoord(
                    *tuple(self.viewport_size))
                img_met_border_left = (canvas_left >= -1 + dx)
                img_met_border_right = (canvas_right <= 1 + dx)
                img_met_border_bottom = (canvas_bottom >= -1 + dy)
                img_met_border_top = (canvas_top <= 1 + dy)
                if img_met_border_top and not img_met_border_bottom and dy > 0:
                    dy = 0
                if img_met_border_bottom and not img_met_border_top and dy < 0:
                    dy = 0
                if img_met_border_left and not img_met_border_right and dx < 0:
                    dx = 0
                if img_met_border_right and not img_met_border_left and dx > 0:
                    dx = 0

                self.move(dx, dy, refresh=True)

                self.mpos_dragstart = mpos
        if not is_dragging:
            if self.checkCoordInside(*self.getWorldCoord(*tuple(mpos))):
                self.draw_mouse_x, self.draw_mouse_y = self.getWorldCoord(
                    *tuple(mpos))
            else:
                self.draw_mouse_x, self.draw_mouse_y = None, None
        self.Refresh()

    def onScroll(self, event: wx.ScrollWinEvent):
        match event.Orientation:
            case wx.HORIZONTAL:
                pos_0 = self.GetScrollPos(wx.HORIZONTAL)
                pos_1 = event.Position
                dx = self.getWorldCoord(pos_0, 0)[0] \
                    - self.getWorldCoord(pos_1, 0)[0]
                self.move(dx, 0)
            case wx.VERTICAL:
                pos_0 = self.GetScrollPos(wx.VERTICAL)
                pos_1 = event.Position
                dy = self.getWorldCoord(0, pos_0)[1] \
                    - self.getWorldCoord(0, pos_1)[1]
                self.move(0, dy)

    def bindPrj(self, prj: Prj, set_page_handler: Callable[[int], Any]):
        self.prj = prj
        self.setPage(0)
        self.set_page_handler = set_page_handler
        # bind mouse event
        self.Bind(wx.EVT_LEFT_DOWN, self.onLeftDown)  # 绑定鼠标左键按下事件
        self.Bind(wx.EVT_LEFT_UP, self.onLeftUp)  # 绑定鼠标左键弹起事件
        self.Bind(wx.EVT_RIGHT_UP, self.onRightUp)  # 绑定鼠标右键弹起事件
        self.Bind(wx.EVT_MOTION, self.onMouseMotion)   # 绑定鼠标移动事件
        self.Bind(wx.EVT_MOUSEWHEEL, self.onMouseWheel)  # 绑定鼠标滚轮事件

        # bind scrollbar event
        self.Bind(wx.EVT_SCROLLWIN, self.onScroll)
        # self.Bind(wx.EVT_SCROLL_THUMBTRACK, self.onScroll)

    def refreshSelectAreas(self):
        tmp = self.prj.calcSelectArea(self.prj.curr_page)
        self.canvas_select_areas = []
        for ((l, t), (r, b)) in tmp:
            self.canvas_select_areas.append(((2*l/self.pic.shape[1] - 1, 1 - 2*t/self.pic.shape[0]),
                                             (2*r/self.pic.shape[1] - 1, 1 - 2*b/self.pic.shape[0])))

    def setPage(self, index):
        '''显示第index页
        目前无法异步调用'''
        if self.prj is None:
            return
        __pic = self.prj.getPage(index)
        self.pic = cv2.flip(__pic, 0)

        self.prj_curr_lines_ls = self.prj.getLineList(index)
        self.canvas_lines = [1 - 2*line/self.pic.shape[0]
                             for line in self.prj_curr_lines_ls]
        self.refreshSelectAreas()
        self.setTextrue()
        self.Refresh(False)
        if self.set_page_handler is not None:
            self.set_page_handler(index)
