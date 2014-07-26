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
	RECT m_rateRect ;

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
		MESSAGE_HANDLER(WM_NCCALCSIZE, OnNCCalcSize)
		MESSAGE_HANDLER(WM_NCPAINT, OnPaint)
		
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		MESSAGE_HANDLER(WM_MOVE, OnMove)

		MESSAGE_HANDLER(WM_ENTERSIZEMOVE, OnEnterSizeMove)
		MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SIZING, OnSizing)
		
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	HMENU m_menu =NULL;

	//rcArea，整个区域
	//left,top：左上角坐标，大于0为固定数值，-1到-100表示按rcArea宽度和高度的百分比。
	//width,height：宽度和高度，大于0为固定数值，等于0则维持不变
	//函数先计算大小，再确定左上角的位置
	void CalcRect(RECT rcArea, RECT rcRate, RECT &rcChild, bool mode = TRUE)
	{
		//初始化：宽度和高度
		int widthArea = rcArea.right - rcArea.left + 1;
		int heightArea = rcArea.bottom - rcArea.top + 1;
		int width = rcChild.right - rcChild.left + 1;
		int height = rcChild.bottom - rcChild.top + 1;

		//首先计算大小
		//mode仅仅对左上角坐标有意义
		//rcRate.right=0,则宽度维持原状
		//rcRate.right>0,则宽度设为rcRate的right值
		//rcRate.right<0，则根据模式，设定为rcArea宽度为基础、或宽度差为基础的相对值，-1代表1%，-100为100%
		if (rcRate.right > 0) width = rcRate.right;
		if (rcRate.right < 0) width = -widthArea*rcRate.right / 100;

		//rcRate.right=0,则高度维持原状
		//rcRate.bottom>0,则高度设为rcRate的bottom值
		//rcRate.bottom<0，则根据模式，设定为rcAre高度为rcArea高度的相对值，-1代表1%，-100为100%
		if (rcRate.bottom > 0) height = rcRate.bottom;
		if (rcRate.bottom < 0) height = -heightArea*rcRate.bottom / 100;


		int widthBase = mode ? (widthArea - width) : widthArea;
		int heightBase = mode ? (heightArea - height) : heightArea;

		//然后确定位置
		if (rcRate.left >= 0)
			rcChild.left = rcArea.left + rcRate.left; //rcRate是相对父RECT的值，我们需要得到窗口坐标
		else
			rcChild.left = rcArea.left - widthBase*rcRate.left / 100;//这里算坐标，不应减去1

		//这里是rcArea为正数，始终会加正数，因此不可能小于0
		//if (rcChild.left < 0) rcChild.left = 0;

		if (rcRate.top >= 0)
			rcChild.top = rcArea.top + rcRate.top;
		else
			rcChild.top = rcArea.top - heightBase*rcRate.top / 100;

		//至此，位置已经正确，此时修改右下坐标
		rcChild.right = rcChild.left + width - 1;
		rcChild.bottom = rcChild.top + height - 1;
	}

	void ShowChild(bool bShow=true)
	{
		if (m_menu == NULL && this->IsWindow())
			m_menu = GetMenu();

		//SetRedraw(False)，则后面所有的更新都被忽略
		//LockWindowUpdate,后面的更新有效，解锁后显示
		if (bShow) LockWindowUpdate();
		::ShowWindow(m_hWndClient, bShow?SW_SHOW:SW_HIDE);
		::ShowWindow(m_hWndToolBar, bShow ? SW_SHOW : SW_HIDE);
		::ShowWindow(m_hWndStatusBar, bShow ? SW_SHOW : SW_HIDE);
		::SetMenu(m_hWnd,bShow?m_menu:NULL);
		if (bShow){
			::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0);
			::InvalidateRect(m_hWndToolBar, NULL, TRUE);
			::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
			::InvalidateRect(m_hWndToolBar, NULL, TRUE);
			LockWindowUpdate(false);

			//解锁即更新，没有必要显示的paint
			//this->UpdateWindow();
		}
	}

	//显示视图
	template <class T>
	CWindow * ShowView(RECT rcRate = {0,0,0,0})
	{
		if (::IsWindow(m_hWndClient))
		{
			if (::GetDlgCtrlID(m_hWndClient) == T::IDD)
				return m_view;
			::DestroyWindow(m_hWndClient);
			m_hWndClient = NULL;
		}

		m_rateRect = rcRate;
		T* pView = new T;
		m_hWndClient = pView->Create(m_hWnd);
		pView->SetDlgCtrlID(pView->IDD);

		//如果需要支持CWindow视图：
		//m_hWndClient = pView->Create(m_hWnd, NULL, NULL, 0, 0, IDD);
		return (CWindow *)pView;
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{		
		if (wParam == SC_MAXIMIZE || wParam==SC_RESTORE)
		{
			ATLTRACE("最大化或恢复消息:wParam %u,%u,lParam: %u,%u \n",
				LOWORD(wParam), HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
			//MessageBox(L"点击了最大化或恢复按钮");
			ShowChild(FALSE);
			//MessageBox(L"最大化或恢复消息：开始默认处理");
			//LRESULT result = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			LRESULT result = DefWindowProc(uMsg, wParam, lParam);

			
			AtlTrace("最大化消息：结束默认处理 \n");
			//MessageBox(L"最大化消息：默认处理结束,现在看看能否显示子窗体");
			
			//这里多余，因为必定会执行OnSize，从而会执行UpdateChild
			//ShowChild(TRUE);			
		}
		else
		{
			return DefWindowProc(uMsg, wParam, lParam);
		}
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		AtlTrace("WM_ERASEBKGND \n");
		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		AtlTrace("WM_PAINT \n");
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
		//MessageBox(L"进入OnGetMinMaxInfo");
		//MINMAXINFO * info = (MINMAXINFO *)lParam;
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
	}

	LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		////MessageBox(L"位置正在改变：WM_WINDOWPOSCHANGING");
		ATLTRACE("OnWindowPosChanging:wParam %u(%u,%u),lParam: %u(%u,%u) \n",
			wParam, LOWORD(wParam), HIWORD(wParam), lParam, LOWORD(lParam), HIWORD(lParam));
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
	}
	

	LRESULT OnNCCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
			////MessageBox(L"计算系统区大小：OnNCCalcSize");
			bHandled = false; //如此设置，则后续的消息接收器会处理
			return TRUE;
	}

	LRESULT OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLTRACE("WM_NCPAINT:wParam %u(%u,%u),lParam: %u(%u,%u) \n",
			wParam, LOWORD(wParam), HIWORD(wParam), lParam, LOWORD(lParam), HIWORD(lParam));
		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLTRACE("OnWindowPosChanged:wParam %u(%u,%u),lParam: %u(%u,%u) \n",
			wParam, LOWORD(wParam), HIWORD(wParam), lParam, LOWORD(lParam), HIWORD(lParam));
		//MessageBox(L"位置已改变：WM_WINDOWPOSCHANGED");
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
	}

	LRESULT OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		////MessageBox(L"移动:WM_MOVE");
		ATLTRACE("WM_MOVE:wParam %u(%u,%u),lParam: %u(%u,%u) \n",
			wParam,LOWORD(wParam), HIWORD(wParam),lParam,LOWORD(lParam), HIWORD(lParam));
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
	}


	LRESULT OnSizing(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLTRACE("WM_SIZING:wParam %u(%u,%u),lParam: %u(%u,%u) \n",
			wParam, LOWORD(wParam), HIWORD(wParam), lParam, LOWORD(lParam), HIWORD(lParam));
		return TRUE;
	}
	//此时大小改变完成，且窗体的各子窗体也正常的绘制
	//由于我们隐藏了对话框，因此该对话框的绘制并未进行，在此改变位置一次绘制，就不会闪烁
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{	
		ATLTRACE("WM_SIZE:wParam %u(%u,%u),lParam: %u(%u,%u) \n",
			wParam, LOWORD(wParam), HIWORD(wParam), lParam, LOWORD(lParam), HIWORD(lParam));
		if (m_hWndClient && m_view->IsWindow())
		{
			RECT rect;
			this->GetWindowRect(&rect);
			int x, y;
			//GetWindowPos(m_hWndClient, &x, &y);
			AtlTrace("进入onsize x=%u,y=%u,right=%u,bottom=%u \n", rect.left,rect.top,rect.right,rect.bottom);
			//MessageBox(L"进入Onsize！");
			//m_view->LockWindowUpdate(FALSE);
			//m_view->CenterWindow(m_hWnd);
			//m_view->ShowWindow(SW_SHOW);
			UpdateChild();
		}

		bHandled = TRUE;
		return 1;
	}

	//补丁：拖动窗体，未改变大小的情况下，由于OnGetMinMaxInfo隐藏了对话框，且onsize显示
	//该对话框的代码没有执行，此时对话框会消失。
	//在这里只需要重新显示该对话框即可，因为大小未变，本来也是居中的，这里无需再处理


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

		m_view = ShowView<CStockView>({-50,-50,0,0});
		
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

	//m_hWndClient未占据整个框架客户区，框架大小改变时状态栏会出现残痕
	//简单的方式是，始终令m_hWndClent为空，不使用它
	//覆盖UpdateLayout是较好的方式
	void UpdateChild(BOOL bResizeBars = TRUE)
	{
		if (m_view)
		{
			////MessageBox(L"进入UpdateLayout");
			


			int nowLeft, nowTop;
			GetWindowPos(m_hWndClient, &nowLeft, &nowTop); //得到现在的坐标
			

			//这样，启动的时候，执行一次；每次大小变更，执行一次。
			//奇怪的是，退出的时候...也执行一次。
			if (m_hWndClient != NULL)
			{
				AtlTrace("UpdateLayout:SetWindowPos \n");
				////MessageBox(L"UpdateLayout:准备SetWindowPos \n");

				if (!m_view->IsWindowVisible())
					ShowChild(TRUE);

				AtlTrace("UpdateLayout start \n");
				RECT rect = { 0 };
				GetClientRect(&rect); //获取整个应用的客户区rect，这只是除去窗口的标题、边框之后，剩下的窗体工作区域

				UpdateBarsPosition(rect, bResizeBars);  //该rect减去菜单、工具栏、状态栏所占区域
				//此处得到的rect是全部客户区，可以在这个范围内居中显示

				//现在我们获取
				RECT clientRect = { 0 };
				m_view->GetClientRect(&clientRect); //这里仅获得大小

				//显示了工具栏状态栏后再计算，否则会漏掉
				CalcRect(rect, m_rateRect, clientRect);

				m_view->SetWindowPos(NULL,clientRect.left,clientRect.top,
					clientRect.right-clientRect.left+1,clientRect.bottom-clientRect.top+1, SWP_NOZORDER | SWP_NOACTIVATE );

			}
		}
		//MessageBox(L"UpdateLayout:结束\n");
		AtlTrace("UpdateLayout end \n");
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
