// stockView.h : interface of the CStockView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "CCtlColor.h"
#include "atlddxEx.h"
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <atlddx.h>

class CQuoteView;
class CQuoteViewModel
{
public:
	std::wstring m_path;

	void import(std::function<void(int)> func)
	{

		for (int i = 0; i < 100; i++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			func(i);
			//Sleep(100);//win32写法
		}
	}

	void open(HWND hwnd)
	{
		std::wstring sDest;
		//GETDLGITEMTEXTCSTRING(IDC_EDITSOURCE, sDest);
		LPCTSTR pfileName = m_path.empty() ? NULL : m_path.c_str();

		CFileDialog dlg(TRUE, NULL, pfileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("所有文件 (*.*)\0*.*\0Dad文件 (*.Dad)\0*.exe\0Dat文件 (*.dat)\0\0"));
		if (dlg.DoModal() == IDOK)
		{
			m_path = dlg.m_szFileName;
			//m_view->DoDataExchange(false);//将数据交还给
		}
	}
};

class CQuoteView : public CDialogImpl<CQuoteView>
	, public CWinDataExchangeEx<CQuoteView>
	, public CCtlColored<CQuoteView>
{
public:

	CQuoteViewModel model;
	CProgressBarCtrl m_progressBar;

	enum { IDD = IDD_QUOTE_BOX };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BEGIN_DDX_MAP(CQuoteBox)
		DDX_TEXT(IDC_EDIT_PATH, model.m_path)
		//DDX_TEXT(IDC_STATIC_ALLQUOTE, model.m_path)
		//DDX_TEXT(IDC_EDIT_PATH, model.m_path)
	END_DDX_MAP()
	//init_dialog、show都只运行一次。
	BEGIN_MSG_MAP(CQuoteView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)

		COMMAND_HANDLER(IDC_BUTTON_SELECT, BN_CLICKED, OnClickedButtonSelect)
		COMMAND_HANDLER(IDC_BUTTON_INSTALL, BN_CLICKED, OnClickedButtonInstall)
		CHAIN_MSG_MAP(CCtlColored<CQuoteView>)
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


		model.m_path = L"测试DDX";
		m_progressBar = GetDlgItem(IDC_PROGRESS_IMPORT);
		DoDataExchange(FALSE);
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

	LRESULT OnClickedButtonSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		model.open(0);
		DoDataExchange(false, IDC_EDIT_PATH);//将数据交还给
		return 0;
	}

	LRESULT OnClickedButtonInstall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		std::thread t(&CQuoteViewModel::import, &model, [this](int now){
			if (::IsWindow(m_progressBar.m_hWnd))
			{
				m_progressBar.SetPos(now + 1);
			}
		});
		t.detach(); //从主线程分离后执行
		//t.join();//等待子线程执行完毕再执行下一条语句

		//model.import([this](int now){
		//	CProgressBarCtrl bar = GetDlgItem(IDC_PROGRESS_IMPORT);//注意，仅仅是使用进度条
		//	bar.SetPos(now + 1);
		//});
		return 0;
	}
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

};

