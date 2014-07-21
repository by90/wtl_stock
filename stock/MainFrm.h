// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <atltypes.h>

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CStockView m_view;
	CCommandBarCtrl m_CmdBar;

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)		
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	//最大化、改变大小、拖动窗体三种情况，均会触发此消息
	//因此在此时机将对话框隐藏，避免onsize之前完成重绘
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MINMAXINFO * info = (MINMAXINFO *)lParam;
		if (m_view.IsWindow())
		{
			m_view.ShowWindow(SW_HIDE);
		}
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
	}

	//此时大小改变完成，且窗体的各子窗体也正常的绘制
	//由于我们隐藏了对话框，因此该对话框的绘制并未进行，在此改变位置一次绘制，就不会闪烁
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_view.IsWindow())
		{
			//m_view.LockWindowUpdate(FALSE);
			m_view.CenterWindow(m_hWnd);
			m_view.ShowWindow(SW_SHOW);
			UpdateLayout();
		}
		bHandled = False;
		return 1;
	}

	//补丁：拖动窗体，未改变大小的情况下，由于OnGetMinMaxInfo隐藏了对话框，且onsize显示
	//该对话框的代码没有执行，此时对话框会消失。
	//在这里只需要重新显示该对话框即可，因为大小未变，本来也是居中的，这里无需再处理
	LRESULT OnWindowPosChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_view.IsWindow() && !m_view.IsWindowVisible())
		{
			m_view.ShowWindow(SW_SHOW);
		}
		bHandled = False;
		return 1;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//this->ModifyStyle(0, WS_CLIPCHILDREN); //修改样式没有效果

		// create command bar window
		HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
		// attach menu
		m_CmdBar.AttachMenu(GetMenu());
		// load command bar images
		m_CmdBar.LoadImages(IDR_MAINFRAME);
		// remove old menu
		SetMenu(NULL);

		HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		AddSimpleReBarBand(hWndCmdBar);
		AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

		CreateSimpleStatusBar();		
		m_hWndClient=m_view.Create(m_hWnd);
		m_view.CenterWindow(m_hWnd);
		//如果使用CDialogResize，可以设置DlgCtrlId
		//m_view.SetDlgCtrlID(m_view.IDD);
		UIAddToolBar(hWndToolBar);
		UISetCheck(ID_VIEW_TOOLBAR, 1);
		UISetCheck(ID_VIEW_STATUS_BAR, 1);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		
		return 0;
	}

	//m_hWndClient未占据整个框架客户区，框架大小改变时状态栏会出现残痕
	//简单的方式是，始终令m_hWndClent为空，不使用它
	//覆盖UpdateLayout是较好的方式
	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
		RECT rect = { 0 };
		GetClientRect(&rect); //获取整个应用的客户区rect，这只是除去窗口的标题、边框之后，剩下的窗体工作区域

		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);  //该rect减去菜单、工具栏、状态栏所占区域
		//此处得到的rect是全部客户区，可以在这个范围内居中显示

		//步骤1：不修改m_hWndClient的大小
		//如果不要铺满视图，则注释掉下面的语句，会出现状态栏残痕，这是UpdateBarsPosition要处理的
		// resize client window
		//if (m_hWndClient != NULL) //这里将客户区铺满。如果注释掉，则大小变化的时候，状态栏会出现异常，前面部分区域没有消除
		//	::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
		//	rect.right - rect.left, rect.bottom - rect.top,
		//	SWP_NOZORDER | SWP_NOACTIVATE);
	}

	void UpdateBarsPosition(RECT& rect, BOOL bResizeBars = TRUE)
	{
		// resize toolbar
		if (m_hWndToolBar != NULL && ((DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE) & WS_VISIBLE))
		{
			if (bResizeBars != FALSE)
			{
				::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0); //相当于调用函数，消息执行完后才执行下一条
				::InvalidateRect(m_hWndToolBar, NULL, TRUE);
			}
			RECT rectTB = { 0 };
			::GetWindowRect(m_hWndToolBar, &rectTB);
			rect.top += rectTB.bottom - rectTB.top;
		}

		// resize status bar
		if (m_hWndStatusBar != NULL && ((DWORD)::GetWindowLong(m_hWndStatusBar, GWL_STYLE) & WS_VISIBLE))
		{

			//这里没让原来区域失效，因为铺满地窗体将覆盖它，但我们若没有铺满窗体，则这里必须同样失效。
			if (bResizeBars != FALSE)
			{
				::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
				::InvalidateRect(m_hWndStatusBar, NULL, TRUE); //步骤2：增加此行代码，另原来的状态栏区域失效
			}


			RECT rectSB = { 0 };
			::GetWindowRect(m_hWndStatusBar, &rectSB);
			rect.bottom -= rectSB.bottom - rectSB.top;
		}
	}

	//基类的OnEraseBackground，禁止了默认的背景擦除，造成当视图未铺满整个客户区，我们改变窗口大小
	//状态栏和先前显示的视图将出现残痕。单纯将视图和框架颜色设为一致不解决问题。
	//这里直接调用默认的处理，同时该消息不往下传递(bHandled默认为ture),基类的消息映射虽然链接
	//但不再能收到消息。
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: add code to initialize document

		return 0;
	}

	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		static BOOL bVisible = TRUE;	// initially visible
		bVisible = !bVisible;
		CReBarCtrl rebar = m_hWndToolBar;
		int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
		rebar.ShowBand(nBandIndex, bVisible);
		UISetCheck(ID_VIEW_TOOLBAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
};
