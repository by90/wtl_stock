#ifndef multi_view_frame_h
#define multi_view_frame_h
#pragma once
#include <map>
/*
1.切换视图
2.相对于主窗体的位置显示视图
3.相对主窗体设置视图大小
4.设置框架最小的尺寸
5.设置框架背景色
6.消除最大化、还原引发的闪烁
7.工具栏按钮显示文字
*/
template <class T>
class CMultiViewFrame :public CFrameWindowImpl<T>
{
public:
	CWindow *m_view = NULL; //当前视图指针
	HBRUSH m_hDialogBrush;  //框架背景画刷
	HMENU m_menu = NULL;

	RECT m_rateRect; //针对位置和大小的比例设置
	bool m_rateMode = TRUE;//默认以视图和框架大小之差为基准，设置相对位置。

	BEGIN_MSG_MAP(CMainFrame)		
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		//删除m_views里保存的视图
		for (auto p : m_views)
		{
			if ((p.second) && ::IsWindow(p.second->m_hWnd))
				::DestroyWindow(p.second->m_hWnd);
		}
		bHandled = FALSE;
		return 1;
	}
	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 800; // min width
		lpMMI->ptMinTrackSize.y = 600; // min height
		//lpMMI->ptMaxTrackSize.x = 600; // max width
		//lpMMI->ptMaxTrackSize.y = 450; // max height
		bHandled = false; //如此设置，则后续的消息接收器会处理
		return 0;
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return DefWindowProc(uMsg, wParam, lParam);
	}

	//此时大小改变完成，且窗体的各子窗体也正常的绘制
	//由于我们隐藏了对话框，因此该对话框的绘制并未进行，在此改变位置一次绘制，就不会闪烁
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_hWndClient &&  m_view->IsWindow())
			UpdateChild();
		return 1;
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == SC_MAXIMIZE || wParam == SC_RESTORE)
		{
			ShowChild(FALSE);
		}
		return  DefWindowProc(uMsg, wParam, lParam);		
	}

	//不执行
	void UpdateLayout(BOOL bResizeBars = TRUE){}

	//m_hWndClient未占据整个框架客户区，框架大小改变时状态栏会出现残痕
	//简单的方式是，始终令m_hWndClent为空，不使用它
	//覆盖UpdateLayout是较好的方式
	void UpdateChild(BOOL bResizeBars = TRUE)
	{
		if (m_hWndClient != NULL && m_view)
		{
			if (!m_view->IsWindowVisible())
				ShowChild(TRUE);
			RECT rect = { 0 };
			GetClientRect(&rect); //获取整个应用的客户区rect，这只是除去窗口的标题、边框之后，剩下的窗体工作区域
			UpdateBarsPosition(rect, bResizeBars);  //该rect减去菜单、工具栏、状态栏所占区域

			RECT clientRect = { 0 };
			m_view->GetClientRect(&clientRect); //这里仅获得大小
			CalcRect(rect, m_rateRect, clientRect, m_rateMode);
			m_view->SetWindowPos(NULL, clientRect.left, clientRect.top,
					clientRect.right - clientRect.left + 1, clientRect.bottom - clientRect.top + 1, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

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

	void ShowChild(bool bShow = true)
	{
		if (m_menu == NULL && this->IsWindow())
			m_menu = GetMenu();
		if (bShow) LockWindowUpdate();
		::ShowWindow(m_hWndClient, bShow ? SW_SHOW : SW_HIDE);
		::ShowWindow(m_hWndToolBar, bShow ? SW_SHOW : SW_HIDE);
		::ShowWindow(m_hWndStatusBar, bShow ? SW_SHOW : SW_HIDE);
		::SetMenu(m_hWnd, bShow ? m_menu : NULL);
		if (bShow){
			::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0);
			::InvalidateRect(m_hWndToolBar, NULL, TRUE);
			::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
			::InvalidateRect(m_hWndToolBar, NULL, TRUE);
			LockWindowUpdate(false);
		}
	}


	std::map<int, CWindow *> m_views;
	//显示视图
	template <class T>
	CWindow * ShowView(RECT rcRate = { 0, 0, 0, 0 },bool rateMode=TRUE,bool autoDelete=true)
	{
		if (::IsWindow(m_hWndClient) )
		{
			if (::GetDlgCtrlID(m_hWndClient) == T::IDD)
				return m_view;
			if (m_views.find(::GetDlgCtrlID(m_hWndClient))==m_views.end()) //返回迭代器，为空表示未找到？
			//if (!m_views[::GetDlgCtrlID(m_hWndClient)]) //如果当前视图在列表中不存在
			{
				::DestroyWindow(m_hWndClient);
				m_hWndClient = NULL;
			}
			else
				::ShowWindow(m_hWndClient, SW_HIDE);
		}//此时，前一窗口或销毁或隐藏

		m_rateRect = rcRate;
		m_rateMode = rateMode;

		if (m_views.find(T::IDD)!=m_views.end()) //如果本视图已经创建
		{
			m_view = m_views[T::IDD];
			m_hWndClient = m_view->m_hWnd;
			::ShowWindow(m_view->m_hWnd, SW_SHOW);
			UpdateChild();
			return m_view;
		}

		T* pView = new T;
		m_hWndClient = pView->Create(m_hWnd);
		pView->SetDlgCtrlID(pView->IDD);
		m_view = pView;
		//如果不准备自动删除，则加入到视图列表
		if (!autoDelete)
			m_views[T::IDD] = pView;
		//如果需要支持CWindow视图：
		//m_hWndClient = pView->Create(m_hWnd, NULL, NULL, 0, 0, IDD);
		UpdateChild();
		return (CWindow *)pView;
	}

	void AddToolbarButtonText(HWND hWndToolBar, UINT nID, LPCTSTR lpsz)
	{
		CToolBarCtrl toolbar(hWndToolBar);
		// Set extended style
		if ((toolbar.GetExtendedStyle() & TBSTYLE_EX_MIXEDBUTTONS) !=
			TBSTYLE_EX_MIXEDBUTTONS)
			toolbar.SetExtendedStyle(toolbar.GetExtendedStyle() |
			TBSTYLE_EX_MIXEDBUTTONS);
		// Get the button index
		int nIndex = toolbar.CommandToIndex(nID);
		TBBUTTON tb;
		memset(&tb, 0, sizeof(tb));
		toolbar.GetButton(nIndex, &tb);
		// Add the string to the main toolbar list
		//std::wstring str(lpsz, _tcslen(lpsz) + 1);
		int nStringID = toolbar.AddStrings(lpsz);
		// Alter the button style
		tb.iString = nStringID;
		tb.fsStyle |= TBSTYLE_AUTOSIZE | BTNS_SHOWTEXT;
		// Delete and re-insert the button
		toolbar.DeleteButton(nIndex);
		toolbar.InsertButton(nIndex, &tb);
	}

	void SetBkColor(COLORREF cl = RGB(204, 232, 207))
	{
		m_hDialogBrush = CreateSolidBrush(cl);
		SetClassLong(m_hWnd, GCL_HBRBACKGROUND, (LONG)m_hDialogBrush);
	}

	COLORREF GetBkColor()
	{
		//设置背景色后才能获取...因为之前并未设置
		HBRUSH bk = (HBRUSH)GetClassLong(m_hWnd, GCL_HBRBACKGROUND);
		LOGBRUSH brush;
		::GetObjectW(bk, sizeof(LOGBRUSH), &brush);
		return brush.lbColor;
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
};
#endif