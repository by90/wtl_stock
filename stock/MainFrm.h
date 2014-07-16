// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <atltypes.h>


/************************************************************************/
/* WTL版
/* 宏功能: 界面刷新时仅刷新指定控件以外的空白区域;可有效避免窗口闪烁
/* 使用于: WM_ERASEBKGND 消息处理函数
/************************************************************************/
#define ERASE_BKGND_BEGIN \
	CRect bgRect;\
	GetClientRect(&bgRect);\
	CRgn bgRgn;\
	bgRgn.CreateRectRgnIndirect(bgRect);
//#define ERASE_BKGND_BEGIN
// Marco parameter 'IDC' specifies the identifier of the control
//GetDlgItem(IDC)->GetWindowRect(&controlRect);
#define ADD_NOERASE_CONTROL(IDC)\
{\
	CRect controlRect;\
	m_view.GetWindowRect(&controlRect);\
	CRgn controlRgn;\
	controlRgn.CreateRectRgnIndirect(controlRect);\
	if(bgRgn.CombineRgn(bgRgn, controlRgn, RGN_XOR)==ERROR)\
	return false;\
}
// Marco parameter 'noEraseRect' specifies a screen coordinates based RECT,
// which needn't erase.
#define ADD_NOERASE_RECT(noEraseRect)\
{\
	CRgn noEraseRgn;\
	noEraseRgn.CreateRectRgnIndirect(noEraseRect);\
	if(bgRgn.CombineRgn(bgRgn.m_hRgn, noEraseRgn.m_hRgn, RGN_XOR)==ERROR)\
	return false;\
}
// Marco parameter 'pDC' is a kind of (CDC *) type.
// Marco parameter 'clBrushColor' specifies the color to brush the area.
#define ERASE_BKGND_END(pDC, clBrushColor)\
	CBrush brush;\
	brush.CreateSolidBrush(clBrushColor);\
	CPoint saveOrg;\
	(pDC)->GetWindowOrg(&saveOrg);\
	(pDC)->SetWindowOrg(bgRect.TopLeft());\
	(pDC)->FillRgn(bgRgn.m_hRgn, brush.m_hBrush);\
	(pDC)->SetWindowOrg(saveOrg);\
	brush.DeleteObject();\
	//#define ERASE_BKGND_END

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
		//MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		//MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//SetMsgHandled(FALSE);
		//HDC  dc = (HDC)wParam;
		//CDC memDC;
		//memDC.CreateCompatibleDC(dc);
		//CDC *pDC = &memDC;
		//
		//ERASE_BKGND_BEGIN;
		//ADD_NOERASE_CONTROL(m_view.IDD);
		//ERASE_BKGND_END(pDC, GetSysColor(COLOR_3DFACE));
		//SetMsgHandled(FALSE);
		return true;
	}//覆盖，组织背景擦除，没有效果

		
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
		
		m_view.Create(m_hWnd);
		m_view.SetDlgCtrlID(m_view.IDD);


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

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//this->CenterWindow();
		//如果是False，状态栏就没重画


		//CFrameWindowImpl<CMainFrame>::OnSize(uMsg,wParam,lParam,bHandled);
		if (m_view.IsWindow())
		{
			CRect rc;
			m_view.GetWindowRect(&rc);
			::InvalidateRect(m_view.m_hWnd, NULL, TRUE);
			
			m_view.CenterWindow(m_hWnd);
			//CRect Rect,rc;
			//GetClientRect(&Rect);
			//m_view.GetWindowRect(&rc);
			//m_view.SetWindowPos(NULL, (Rect.Width() - rc.Width()) / 2, (Rect.Height() - rc.Height()) / 2, rc.Width(), rc.Height(),SWP_SHOWWINDOW);
			UpdateLayout();
		}
	    
		//bHandled = false; //如此设置，则后续的消息接收器会处理
		return TRUE;
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
