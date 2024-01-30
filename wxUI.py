# -*- coding: utf-8 -*-

###########################################################################
## Python code generated with wxFormBuilder (version 4.0.0-6-ga75305af)
## http://www.wxformbuilder.org/
##
## PLEASE DO *NOT* EDIT THIS FILE!
###########################################################################

import wx
import wx.xrc
import wx.aui
from wxGLScene import WxGLScene

btnid_PREV_PAGE = 1000
btnid_NEXT_PAGE = 1001
btnid_SAVE = 1002
btnid_LOAD = 1003
btnid_ZOOM_PAGE = 1004
btnid_CROP_CURR_PAGE = 1005
btnid_CROP_ALL_PAGE = 1006

###########################################################################
## Class MainWindow
###########################################################################

class MainWindow ( wx.Frame ):

	def __init__( self, parent ):
		wx.Frame.__init__ ( self, parent, id = wx.ID_ANY, title = u"Crop Lines", pos = wx.DefaultPosition, size = wx.Size( 972,651 ), style = wx.DEFAULT_FRAME_STYLE|wx.TAB_TRAVERSAL )

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.m_mgr = wx.aui.AuiManager()
		self.m_mgr.SetManagedWindow( self )
		self.m_mgr.SetFlags(wx.aui.AUI_MGR_DEFAULT)

		self.tool_bar = wx.aui.AuiToolBar( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.aui.AUI_TB_HORZ_LAYOUT|wx.aui.AUI_TB_OVERFLOW|wx.aui.AUI_TB_TEXT )
		self.tool_bar.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )

		self.btn_prev_page = self.tool_bar.AddTool( btnid_PREV_PAGE, u"上一页", wx.Bitmap( u"asserts/prevslide.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"上一页", u"上一页", None )

		self.btn_next_page = self.tool_bar.AddTool( btnid_NEXT_PAGE, u"下一页", wx.Bitmap( u"asserts/nextslide.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"下一页", u"下一页", None )

		self.tool_bar.AddSeparator()

		self.btn_save = self.tool_bar.AddTool( btnid_SAVE, u"保存", wx.Bitmap( u"asserts/save.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"保存", u"保存", None )

		self.btn_load = self.tool_bar.AddTool( btnid_LOAD, u"载入", wx.Bitmap( u"asserts/loadbasic.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"载入", u"载入", None )

		self.tool_bar.AddSeparator()

		self.btn_zoom_page = self.tool_bar.AddTool( btnid_ZOOM_PAGE, u"缩放合适大小", wx.Bitmap( u"asserts/zoompage.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"缩放合适大小", u"缩放合适大小", None )

		self.btn_crop_curr_page = self.tool_bar.AddTool( btnid_CROP_CURR_PAGE, u"裁剪当前页", wx.Bitmap( u"asserts/crop.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"裁剪当前页", u"裁剪当前页", None )

		self.btn_crop_all_page = self.tool_bar.AddTool( btnid_CROP_ALL_PAGE, u"裁剪全部", wx.Bitmap( u"asserts/cropall.png", wx.BITMAP_TYPE_ANY ), wx.NullBitmap, wx.ITEM_NORMAL, u"裁剪全部", u"裁剪全部", None )

		self.tool_bar.Realize()
		self.m_mgr.AddPane( self.tool_bar, wx.aui.AuiPaneInfo() .Top() .CaptionVisible( False ).CloseButton( False ).PaneBorder( False ).Movable( False ).Dock().Resizable().FloatingSize( wx.Size( -1,-1 ) ).DockFixed( True ).LeftDockable( False ).RightDockable( False ).Floatable( False ).Layer( 10 ).ToolbarPane() )

		self.status_bar = self.CreateStatusBar( 1, wx.STB_SIZEGRIP, wx.ID_ANY )
		pn_page_listChoices = []
		self.pn_page_list = wx.ListBox( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, pn_page_listChoices, wx.LB_NEEDED_SB )
		self.m_mgr.AddPane( self.pn_page_list, wx.aui.AuiPaneInfo() .Left() .Caption( u"页面列表" ).PinButton( True ).Dock().Resizable().FloatingSize( wx.DefaultSize ).BottomDockable( False ).TopDockable( False ).BestSize( wx.Size( 200,500 ) ).MinSize( wx.Size( 130,-1 ) ) )

		self.pn_canvas = wx.Panel( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_mgr.AddPane( self.pn_canvas, wx.aui.AuiPaneInfo() .Left() .CaptionVisible( False ).PinButton( True ).Dock().Resizable().FloatingSize( wx.DefaultSize ).CentrePane() )

		bSizer31 = wx.BoxSizer( wx.VERTICAL )

		self.canvas = WxGLScene(self.pn_canvas)
		bSizer31.Add( self.canvas, 1, wx.EXPAND, 5 )


		self.pn_canvas.SetSizer( bSizer31 )
		self.pn_canvas.Layout()
		bSizer31.Fit( self.pn_canvas )
		self.pn_config = wx.Notebook( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0, u"设置" )
		self.m_mgr.AddPane( self.pn_config, wx.aui.AuiPaneInfo() .Right() .Caption( u"设置" ).PinButton( True ).Dock().Resizable().FloatingSize( wx.DefaultSize ).BottomDockable( False ).TopDockable( False ).BestSize( wx.Size( 350,500 ) ).MinSize( wx.Size( 350,-1 ) ) )

		self.ntbk_cfg_process = wx.Panel( self.pn_config, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer4 = wx.BoxSizer( wx.VERTICAL )

		bSizer9 = wx.BoxSizer( wx.HORIZONTAL )

		self.tx_label_cfg_pix_filter = wx.StaticText( self.ntbk_cfg_process, wx.ID_ANY, u"忽略斑点直径", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.tx_label_cfg_pix_filter.Wrap( -1 )

		bSizer9.Add( self.tx_label_cfg_pix_filter, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		self.tx_value_cfg_pix_filter = wx.StaticText( self.ntbk_cfg_process, wx.ID_ANY, u"00", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.tx_value_cfg_pix_filter.Wrap( -1 )

		bSizer9.Add( self.tx_value_cfg_pix_filter, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM|wx.LEFT, 5 )

		self.slider_cfg_pix_filter = wx.Slider( self.ntbk_cfg_process, wx.ID_ANY, 8, 0, 50, wx.DefaultPosition, wx.DefaultSize, wx.SL_HORIZONTAL )
		bSizer9.Add( self.slider_cfg_pix_filter, 1, wx.TOP|wx.BOTTOM, 5 )

		self.spinbtn_cfg_pix_filter = wx.SpinButton( self.ntbk_cfg_process, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.spinbtn_cfg_pix_filter.SetMaxSize( wx.Size( 20,-1 ) )

		bSizer9.Add( self.spinbtn_cfg_pix_filter, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM|wx.RIGHT, 5 )


		bSizer4.Add( bSizer9, 0, wx.EXPAND, 5 )


		self.ntbk_cfg_process.SetSizer( bSizer4 )
		self.ntbk_cfg_process.Layout()
		bSizer4.Fit( self.ntbk_cfg_process )
		self.pn_config.AddPage( self.ntbk_cfg_process, u"处理", False )
		self.ntbk_cfg_output = wx.Panel( self.pn_config, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer41 = wx.BoxSizer( wx.VERTICAL )

		bSizer91 = wx.BoxSizer( wx.HORIZONTAL )

		self.tx_label_border = wx.StaticText( self.ntbk_cfg_output, wx.ID_ANY, u"空白边距", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.tx_label_border.Wrap( -1 )

		bSizer91.Add( self.tx_label_border, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		self.tx_value_cfg_border = wx.StaticText( self.ntbk_cfg_output, wx.ID_ANY, u"000", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.tx_value_cfg_border.Wrap( -1 )

		bSizer91.Add( self.tx_value_cfg_border, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM|wx.LEFT, 5 )

		self.slider_cfg_border = wx.Slider( self.ntbk_cfg_output, wx.ID_ANY, 10, 0, 500, wx.DefaultPosition, wx.DefaultSize, wx.SL_HORIZONTAL )
		bSizer91.Add( self.slider_cfg_border, 1, wx.TOP|wx.BOTTOM, 5 )

		self.spinbtn_cfg_border = wx.SpinButton( self.ntbk_cfg_output, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0 )
		self.spinbtn_cfg_border.SetMaxSize( wx.Size( 20,-1 ) )

		bSizer91.Add( self.spinbtn_cfg_border, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM|wx.RIGHT, 5 )


		bSizer41.Add( bSizer91, 0, wx.EXPAND, 5 )


		self.ntbk_cfg_output.SetSizer( bSizer41 )
		self.ntbk_cfg_output.Layout()
		bSizer41.Fit( self.ntbk_cfg_output )
		self.pn_config.AddPage( self.ntbk_cfg_output, u" 输出", True )


		self.m_mgr.Update()
		self.Centre( wx.BOTH )

	def __del__( self ):
		self.m_mgr.UnInit()



