import wxUI
import wx
import ctypes
import traceback
from threading import Thread
from typing import overload
from prj import Prj, PrjNoImgFileError, PrjFileFormatError


class MainWindow(wxUI.MainWindow):
    def __init__(self, parent):
        super().__init__(parent)
        self.SetIcon(wx.Icon("asserts/mainicon.ico", wx.BITMAP_TYPE_ICO))

        self.status_bar: wx.StatusBar  # typing config

        # self.m_scrolledWindow2.GetClientSize

        self.enableTool(False)

        # toolbar bind event
        self.tool_bar.Bind(wx.EVT_TOOL, self.onClickBtnSave,
                           id=wxUI.btnid_SAVE)
        self.tool_bar.Bind(wx.EVT_TOOL, self.onClickBtnLoad,
                           id=wxUI.btnid_LOAD)
        self.tool_bar.Bind(
            wx.EVT_TOOL, self.onClickBtnPrevPage, id=wxUI.btnid_PREV_PAGE)
        self.tool_bar.Bind(
            wx.EVT_TOOL, self.onClickBtnNextPage, id=wxUI.btnid_NEXT_PAGE)
        self.tool_bar.Bind(
            wx.EVT_TOOL, self.onClickBtnZoomPage, id=wxUI.btnid_ZOOM_PAGE)
        self.tool_bar.Bind(
            wx.EVT_TOOL, self.onClickBtnCropCurrPage, id=wxUI.btnid_CROP_CURR_PAGE)
        self.tool_bar.Bind(
            wx.EVT_TOOL, self.onClickBtnCropAllPage, id=wxUI.btnid_CROP_ALL_PAGE)

        # page list bind event
        self.pn_page_list.Bind(wx.EVT_LISTBOX, self.onClickPageList)

        # bind window close
        self.Bind(wx.EVT_CLOSE, self.onWindowClose)
        # bind keyboard event
        # detect key pressed
        self.SetFocus()
        self.Bind(wx.EVT_KEY_DOWN, self.onKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.onKeyUp)

        # default value
        self.prj_location = ''
        self.prj = None

    def enableTool(self, state: bool):
        '''enable all the tool except LOAD or the opposiite'''
        if state:
            self.tool_bar.EnableTool(wxUI.btnid_SAVE, True)
            self.tool_bar.EnableTool(wxUI.btnid_CROP_ALL_PAGE, True)
            self.tool_bar.EnableTool(wxUI.btnid_CROP_CURR_PAGE, True)
            self.tool_bar.EnableTool(wxUI.btnid_ZOOM_PAGE, True)
            self.tool_bar.EnableTool(wxUI.btnid_NEXT_PAGE, True)
            self.tool_bar.EnableTool(wxUI.btnid_PREV_PAGE, True)
        else:
            self.tool_bar.EnableTool(wxUI.btnid_SAVE, False)
            self.tool_bar.EnableTool(wxUI.btnid_CROP_ALL_PAGE, False)
            self.tool_bar.EnableTool(wxUI.btnid_CROP_CURR_PAGE, False)
            self.tool_bar.EnableTool(wxUI.btnid_ZOOM_PAGE, False)
            self.tool_bar.EnableTool(wxUI.btnid_NEXT_PAGE, False)
            self.tool_bar.EnableTool(wxUI.btnid_PREV_PAGE, False)

    def setPagePrev(self):
        page_index_curr = self.pn_page_list.GetSelection()
        if (page_index_curr == 0 or page_index_curr == -1):
            return
        self.pn_page_list.SetSelection(page_index_curr-1)
        self.SetFocus()
        self.canvas.setPage(page_index_curr-1)

    def setPageNext(self):
        page_index_curr = self.pn_page_list.GetSelection()
        if (page_index_curr == self.pn_page_list.GetCount()-1):
            return
        self.pn_page_list.SetSelection(page_index_curr+1)
        self.SetFocus()
        self.canvas.setPage(page_index_curr+1)

    def onClickBtnLoad(self, event):
        dir_dialog = wx.DirDialog(parent=self, message="选择目录")
        res = dir_dialog.ShowModal()
        if res == wx.ID_CANCEL:
            dir_dialog.Destroy()
            return

        # enable all tool btn
        self.enableTool(True)

        prj_dir = dir_dialog.GetPath()
        self.prj = Prj()
        try:
            self.prj.loadImgFromDir(prj_dir)
        except PrjNoImgFileError:
            wx.MessageBox(message="此文件目录下没有图片", caption="导入异常",
                          parent=self, style=wx.ICON_ERROR)
            self.enableTool(False)
            return
        except PrjFileFormatError:
            wx.MessageBox(message="项目文件出错", caption="导入异常",
                          parent=self, style=wx.ICON_ERROR)
            self.enableTool(False)
            return

        self.canvas.bindPrj(
            self.prj, set_page_handler=lambda index: self.pn_page_list.SetSelection(index))
        self.SetFocus()

        # clear listbox
        listbox_cout = self.pn_page_list.GetCount()
        for _ in range(listbox_cout):
            self.pn_page_list.Delete(0)

        # insert all image filename
        self.pn_page_list.InsertItems(self.prj.img_filenames, 0)

        dir_dialog.Destroy()

    def onClickBtnSave(self, event: None | wx.CommandEvent = None):
        try:
            self.prj.save()
        except Exception as e:
            self.showErrorMsgBox(e, caption="保存出错")

    def onClickBtnPrevPage(self, event: wx.CommandEvent):
        self.setPagePrev()
        event.Skip()

    def onClickBtnNextPage(self, event: wx.CommandEvent):
        self.setPageNext()
        event.Skip()

    def onClickPageList(self, event: wx.CommandEvent) -> None:
        print(type(event))
        self.canvas.setPage(event.Selection)
        self.SetFocus()
        event.Skip()

    def onClickBtnZoomPage(self, event: wx.CommandEvent) -> None:
        self.canvas.scale(zoom=1.0)
        event.Skip()

    def onClickBtnCropCurrPage(self, event: wx.CommandEvent) -> None:
        print(type(event))
        thread = Thread(target=self.prj.cropPage,
                        kwargs={"index": self.prj.curr_page,
                                "finish_callback": lambda: self.SetStatusText(f"第 {self.prj.curr_page+1} 页裁剪完成！")})
        thread.start()
        event.Skip()

    def onClickBtnCropAllPage(self, event: wx.CommandEvent) -> None:
        thread = Thread(target=self.prj.cropAllPage,
                        kwargs={"finish_callback": lambda: self.SetStatusText(f"全部裁剪完成！"),
                                "processing_callback": lambda index: self.SetStatusText(f"正在处理第 {index+1} 页...")})
        thread.start()
        event.Skip()

    def onWindowClose(self, event) -> None:
        if self.prj is None:
            event.Skip()
            return
        if not self.prj.is_change:
            event.Skip()
            return
        dialog = wx.MessageDialog(
            parent=self, message="项目已更改，是否保存？", style=wx.YES_NO | wx.CANCEL | wx.ICON_QUESTION)
        match dialog.ShowModal():
            case wx.ID_YES:
                self.onClickBtnSave()
                event.Skip()
                return
            case wx.ID_NO:
                event.Skip()
                return
            case wx.ID_CANCEL:
                return

    def showErrorMsgBox(self, e: Exception, caption=wx.MessageBoxCaptionStr):
        wx.MessageBox(message=e.args[0] +
                      "\n=================\n" +
                      traceback.format_exc(), caption=caption, parent=self)

    def onKeyDown(self, event: wx.KeyEvent):
        match event.KeyCode:
            case 68:
                # 快捷键D，配合鼠标右键使用，删除已经绘制的横线
                self.canvas.is_D_key_down = True
            case 317:
                # 快捷键M，下一页
                if not self.canvas.is_DOWN_key_down:
                    # 保证每次按下只触发一次
                    self.canvas.is_DOWN_key_down = True
                    if self.prj.curr_page != self.prj.total_num-1:
                        self.canvas.setPage(self.prj.curr_page+1)
            case 315:
                # 快捷键N，上一页
                if not self.canvas.is_UP_key_down:
                    self.canvas.is_UP_key_down = True
                    if self.prj.curr_page != 0:
                        self.canvas.setPage(self.prj.curr_page-1)

    def onKeyUp(self, event: wx.KeyEvent):
        match event.KeyCode:
            case 68:
                self.canvas.is_D_key_down = False
            case 317:
                self.canvas.is_DOWN_key_down = False
            case 315:
                self.canvas.is_UP_key_down = False

    def __del__(self):
        super().__del__()
        del self.canvas


class App(wx.App):
    def __init__(self):
        # Enable high Dpi awareness
        try:
            # Turn on high-DPI awareness to make sure rendering is sharp on big monitors with font scaling enabled.
            ctypes.OleDLL('shcore').SetProcessDpiAwareness(1)
        except AttributeError:
            # We're on a non-Windows box.
            pass
        except OSError:
            # exc.winerror is often E_ACCESSDENIED (-2147024891/0x80070005).
            # This occurs after the first run, when the parameter is reset in the executable's manifest and then subsequent calls raise this exception
            # See last paragraph of Remarks at [https://msdn.microsoft.com/en-us/library/dn302122(v=vs.85).aspx](https://msdn.microsoft.com/en-us/library/dn302122(v=vs.85).aspx)
            pass

        super().__init__()

    # ["with ... as ..." grammer]
    def __enter__(self):
        self.main_win = MainWindow(None)
        self.main_win.Show()
        return self

    def __exit__(self, type, value, trace):
        del self.main_win

    @staticmethod
    def run():
        return App()
    # end ["with ... as ..." grammer]


if __name__ == '__main__':
    with App.run() as app:
        app.MainLoop()
