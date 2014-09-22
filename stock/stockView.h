// stockView.h : interface of the CStockView class
//
/////////////////////////////////////////////////////////////////////////////
#include <functional>
#include <thread>
#include <sstream>
#include "ThreadTask.h"
#include "timer.h"
#include "CCtlColor.h"
#include "db_quote.h"
#pragma once
class CStockView;
class HomeViewModel
{
public:
	void Search(std::function<void(const wchar_t *, int)> func)
	{
		DbQuote dbQuote;
		

		timer time_used; //开始计时
		time_used.reset();

		wostringstream ss;
		ss << L"共" << g_stock.Data.size() << L"只股票，正在载入...";
		func(ss.str().c_str(), 0);
		dbQuote.GetQuote(20, func);


		ss.clear();
		ss << L"完成,耗时";
		auto used = time_used.elapsed_seconds();
		if (used <= 0)
			ss << L"不足1秒!";
		else
			if (used >= 60)
				ss << used / 60 << L"分" << used % 60 << L"秒!";
			else
				ss << used << L"秒!";
		func(ss.str().c_str(), 100);


	}
};
class CStockView : public CDialogImpl<CStockView>
	, public CCtlColored<CStockView>
{
public:
	std::thread::id  import_id_;
	enum { IDD = IDD_HOME_BOX};
	HomeViewModel view_model_;
	CProgressBarCtrl progress_bar_;
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	//init_dialog、show都只运行一次。
	BEGIN_MSG_MAP(CStockView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		COMMAND_HANDLER(IDC_BUTTON_SEARCH, BN_CLICKED, OnClickedButtonSearch)
		CHAIN_MSG_MAP(CCtlColored<CStockView>)
		//MESSAGE_HANDLER(WM_SHOWWINDOW, OnShow)
		//MESSAGE_HANDLER(WM_PAINT,OnPaint)
		//MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){

		//获取背景色
		HBRUSH bk = (HBRUSH)GetClassLong(GetParent(), GCL_HBRBACKGROUND);
		LOGBRUSH brush;
		::GetObjectW(bk, sizeof(LOGBRUSH), &brush);
		DWORD bkColor = brush.lbColor;
		SetTextBackGround(bkColor);  // Lightish kind of blue (Four)
		SetTextColor(RGB(0X60, 0, 0)); // Dark red   

		progress_bar_ = GetDlgItem(IDC_PROGRESS_SEARCH);
		//DeleteObject(bk);//delete居然导致框架窗口颜色设置失效？
		return TRUE;
	}

	LRESULT OnForwardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&	bHandled)
	{
		LPMSG pMsg = (LPMSG)lParam;
		return PreTranslateMessage(pMsg);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//AtlTrace("View:WM_ERASEBKGND \n");
		bHandled = FALSE;
		return this->DefWindowProcW();
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		AtlTrace("View:WM_PAINT \n");
		return this->DefWindowProcW();
		//SetMsgHandled(FALSE);
		//return ::DefWindowProc(m_hWnd,uMsg,wParam,lParam);
	}

	LRESULT OnClickedButtonSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		std::thread *t;
		t=new std::thread(&HomeViewModel::Search, &view_model_, [this, t](const wchar_t *info, int now){
			if (info)
			{
				SetDlgItemTextW(IDC_STATIC_REPORT, info);
				
			}
			if (::IsWindow(progress_bar_.m_hWnd))
			{
				progress_bar_.SetPos(now + 1);
				if ((now == 100) && info) //最后一次,info不能为空
				{
					ThreadTask::Remove(import_id_);
				}
			}
		});
		import_id_ = t->get_id();
		ThreadTask::Add(import_id_, L"载入日线");
		t->detach(); //从主线程分离后执行
		return 0;
	}
};
