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

	CWindow *m_view=NULL;
	CCommandBarCtrl m_CmdBar;

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		//向视图发送WM_FORWARDMSG消息，视图如需截获消息，就响应，否则不用响应。
		if (m_hWndClient != NULL)
			return ::SendMessage(m_hWndClient, WM_FORWARDMSG, 0, (LPARAM)pMsg);
		return FALSE;
	}

	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		//UPDATE_ELEMENT(IDD_STOCK_FORM, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_SYSCOMMAND,OnSysCommand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)		
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)

		MESSAGE_HANDLER(WM_ENTERSIZEMOVE, OnEnterSizeMove)
		MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SIZING, OnSizing)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == SC_MAXIMIZE)
		{
			ATLTRACE("最大化消息：点击按钮后。uMsg: %u, wParam LOWORD: %u, HIWORD: %u\n", uMsg, LOWORD
				(wParam), HIWORD(wParam));
			MessageBox(L"点击了最大化按钮");
		}
		else
		{
			ATLTRACE("uMsg: %u, wParam LOWORD: %u, HIWORD: %u\n lParam LOWORD: %u, HIWORD: %u\n", uMsg, LOWORD
				(wParam), HIWORD(wParam, LOWORD(lParam), HIWORD(lParam)));
			return DefWindowProc(uMsg, wParam, lParam);
			//return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		}
		//AtlTrace("最大化消息:调用默认处理 \n");
		m_view->ShowWindow(SW_HIDE);
		//::ShowWindow( m_CmdBar.m_hWnd, SW_HIDE);
		//::ShowWindow(m_hWndToolBar, SW_HIDE);
		//::ShowWindow(m_hWndStatusBar, SW_HIDE);
		//this->ShowWindow(SW_HIDE);
		MessageBox(L"最大化消息：开始默认处理");

		//LRESULT result = DefWindowProc(uMsg, wParam, lParam);
		LRESULT result = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		MessageBox(L"最大化消息：结束默认处理");
		//UpdateLayout1();
		//AtlTrace("最大化消息：结束默认处理 \n");
		
		return result;
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AtlTrace("WM_ERASEBKGND \n");
		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AtlTrace("WM_PAINT \n");
		//SetMsgHandled(FALSE);
		return DefWindowProc(uMsg, wParam, lParam);
	}
	LRESULT OnEnterSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AtlTrace("WM_ENTERSIZEMOVE \n\r");
		return TRUE;
	}

	LRESULT OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AtlTrace("WM_EXITSIZEMOVE \n\r");
		return True;
	}

	//最大化、改变大小、拖动窗体三种情况，均会触发此消息
	//因此在此时机将对话框隐藏，避免onsize之前完成重绘
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MessageBox(L"进入OnGetMinMaxInfo");
		//MINMAXINFO * info = (MINMAXINFO *)lParam;
		if (m_hWndClient && m_view->IsWindow())
		{
			//this->SetRedraw(False);
		}
		//LockWindowUpdate(true);
		
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
	}

	LRESULT OnSizing(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AtlTrace("WM_SIZING \n");
		return TRUE;
	}
	//此时大小改变完成，且窗体的各子窗体也正常的绘制
	//由于我们隐藏了对话框，因此该对话框的绘制并未进行，在此改变位置一次绘制，就不会闪烁
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{	
		
		AtlTrace("WM_SIZE \n");
		if (m_hWndClient && m_view->IsWindow())
		{
			RECT rect;
			this->GetWindowRect(&rect);
			int x, y;
			//GetWindowPos(m_hWndClient, &x, &y);
			AtlTrace("进入onsize x=%u,y=%u,right=%u,bottom=%u \n", rect.left,rect.top,rect.right,rect.bottom);
			MessageBox(L"进入Onsize！");
			//m_view->LockWindowUpdate(FALSE);
			//m_view->CenterWindow(m_hWnd);
			//m_view->ShowWindow(SW_SHOW);
			UpdateLayout1();


			//m_view->SetRedraw(TRUE);
			//MessageBox(L"开始重画");
			//AtlTrace("Start Redraw x=%d,y=%d \n", x, y);
			//this->SetRedraw();
			//this->RedrawWindow();
			//AtlTrace("End Redraw x=%d,y=%d \n", x, y);
			//m_view->LockWindowUpdate(FALSE);
		}
		//LockWindowUpdate(FALSE);
		
		//UpdateWindow();

		bHandled = TRUE;
		return 1;
	}

	//补丁：拖动窗体，未改变大小的情况下，由于OnGetMinMaxInfo隐藏了对话框，且onsize显示
	//该对话框的代码没有执行，此时对话框会消失。
	//在这里只需要重新显示该对话框即可，因为大小未变，本来也是居中的，这里无需再处理
	LRESULT OnWindowPosChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_hWndClient && m_view->IsWindow() && !m_view->IsWindowVisible())
		{
			//m_view->ShowWindow(SW_SHOW);
		}
		bHandled = False;
		return 1;
	}

	//Win32获取左上角相对父窗口的坐标
	void GetWindowPos(HWND hWnd, int *x, int *y)
	{
		HWND hWndParent = ::GetParent(hWnd);
		POINT p = { 0 };

		::MapWindowPoints(hWnd, hWndParent, &p, 1);

		(*x) = p.x;
		(*y) = p.y;
	}


	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//this->ModifyStyle(0, WS_CLIPCHILDREN); //修改样式没有效果
		AtlTrace("WM_CREATE start \n");
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

		m_view = ShowView<CStockView>();
		
		//m_hWndClient=m_view->Create(m_hWnd);

		//m_view->CenterWindow(m_hWnd);
		//如果使用CDialogResize，可以设置DlgCtrlId
		//m_view->SetDlgCtrlID(m_view->IDD);
		UIAddToolBar(hWndToolBar);
		UISetCheck(ID_VIEW_TOOLBAR, 1);
		UISetCheck(ID_VIEW_STATUS_BAR, 1);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		AtlTrace("WM_CREATE end \n");
		return 0;
	}

	//不执行
	void UpdateLayout(BOOL bResizeBars = TRUE)
	{

	}

	void UpdateBarsPosition(RECT& rect, BOOL bResizeBars = TRUE)
	{
		// resize toolbar
		if (m_hWndToolBar != NULL && ((DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE) & WS_VISIBLE))
		{
			if (bResizeBars != FALSE)
			{
				::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0);
				::InvalidateRect(m_hWndToolBar, NULL, TRUE);
			}
			RECT rectTB = { 0 };
			::GetWindowRect(m_hWndToolBar, &rectTB);
			rect.top += rectTB.bottom - rectTB.top;
		}

		// resize status bar
		if (m_hWndStatusBar != NULL && ((DWORD)::GetWindowLong(m_hWndStatusBar, GWL_STYLE) & WS_VISIBLE))
		{
			if (bResizeBars != FALSE)
				::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
			RECT rectSB = { 0 };
			::GetWindowRect(m_hWndStatusBar, &rectSB);
			rect.bottom -= rectSB.bottom - rectSB.top;
		}
	}

	//m_hWndClient未占据整个框架客户区，框架大小改变时状态栏会出现残痕
	//简单的方式是，始终令m_hWndClent为空，不使用它
	//覆盖UpdateLayout是较好的方式
	void UpdateLayout1(BOOL bResizeBars = TRUE)
	{
		if (m_view)
		{
		AtlTrace("UpdateLayout start \n");
		RECT rect = { 0 };
		GetClientRect(&rect); //获取整个应用的客户区rect，这只是除去窗口的标题、边框之后，剩下的窗体工作区域

		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);  //该rect减去菜单、工具栏、状态栏所占区域
		//此处得到的rect是全部客户区，可以在这个范围内居中显示

		//现在我们获取
		RECT clientRect = { 0 };

			m_view->GetClientRect(&clientRect); //这里仅获得大小

			int left = rect.left + ((rect.right - rect.left) - (clientRect.right - clientRect.left)) / 2;
			int top = rect.top + ((rect.bottom - rect.top) - (clientRect.bottom - clientRect.top)) / 2;
			int nowLeft, nowTop;
			GetWindowPos(m_hWndClient, &nowLeft, &nowTop);

			//这样，启动的时候，执行一次；每次大小变更，执行一次。
			//奇怪的是，退出的时候...也执行一次。
			if (m_hWndClient != NULL && (nowLeft != left || nowTop != top))
			{
				AtlTrace("UpdateLayout:SetWindowPos \n");
				MessageBox(L"UpdateLayout:准备SetWindowPos \n");

				if (!m_view->IsWindowVisible())
				{
					m_view->ShowWindow(SW_SHOW);
					//::ShowWindow(m_CmdBar.m_hWnd, SW_SHOW);
					//::ShowWindow(m_hWndToolBar, SW_SHOW);
					//::ShowWindow(m_hWndStatusBar, SW_SHOW);
				}

				m_view->SetWindowPos(NULL, left, top, -1, -1,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

				
				MessageBox(L"UpdateLayout:结束SetWindowPos，准备显示视图和工具栏 \n");


				//this->LockWindowUpdate(False);
				MessageBox(L"UpdateLayout:SetWindowPos结束 \n");
				//::SetWindowPos(m_hWndClient, NULL, left, top, -1, -1,
				//	SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		AtlTrace("UpdateLayout end \n");
	}



	//显示视图
	template <class T>
	CWindow * ShowView()
	{
		if (::IsWindow(m_hWndClient))
		{
			if (::GetDlgCtrlID(m_hWndClient) == T::IDD)
				return m_view;
			::DestroyWindow(m_hWndClient);
			m_hWndClient = NULL;
		}

		T* pView = new T;
		m_hWndClient = pView->Create(m_hWnd);
		pView->SetDlgCtrlID(pView->IDD);
	   
		//如果需要支持CWindow视图：
		//m_hWndClient = pView->Create(m_hWnd, NULL, NULL, 0, 0, IDD);
		return (CWindow *)pView;
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
		AtlTrace("OnViewToolBar \n");
		static BOOL bVisible = TRUE;	// initially visible
		bVisible = !bVisible;
		CReBarCtrl rebar = m_hWndToolBar;
		int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
		rebar.ShowBand(nBandIndex, bVisible);
		UISetCheck(ID_VIEW_TOOLBAR, bVisible);
		//UpdateLayout();
		return 0;
	}

	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		AtlTrace("OnViewStatusBar \n");
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		//UpdateLayout();
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
};
