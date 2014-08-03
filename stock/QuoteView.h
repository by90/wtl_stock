// stockView.h : interface of the CStockView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "CCtlColor.h"
#include "atlddxEx.h"
#include <atlctrls.h>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <atlddx.h>
#include "db_quote.h"
#include <sstream>
#include "dad_file_parse.h"

class CQuoteView;
class CQuoteViewModel
{
public:
	enum class State
	{
		init,
		selected,
		pending,
		complete,
	};

	State m_state = State::init;
	std::wstring m_path; //选取的文件路径
	std::wstring m_info=L"请选择文件";

	bool isValidFile = FALSE; //选中的文件是否合法

	//两个相关的业务类
	DbQuote quote;
	dad_file_parse parser;

	void import(std::function<void(const char *,int)> func)
	{	
		m_state = CQuoteViewModel::State::pending;
		parser.open(m_path.c_str());
		quote.bulk_insert(parser.begin(), parser.end(), 2000, func);
		
		

		//for (int i = 0; i < 100; i++)
		//{
		//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//	func(i);
		//	//Sleep(100);//win32写法
		//}

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

	bool is_state_changed()
	{
		auto oldState = m_state;
		//如果文件合法
		if (parser.check(m_path.c_str()))
		{
			m_state = CQuoteViewModel::State::selected;
			m_info = L" ";
		}
		else
		{
			m_state = CQuoteViewModel::State::init;
			m_info = L"您选择的文件，格式不对";
		}
		return (m_state != oldState);
	}
};

class CQuoteView : public CDialogImpl<CQuoteView>
	, public CWinDataExchangeEx<CQuoteView>
	, public CCtlColored<CQuoteView>
{
public:
	
	CQuoteViewModel model;
	CProgressBarCtrl m_progressBar;
	CEdit m_pathctrl;
	
	enum { IDD = IDD_QUOTE_BOX };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BEGIN_DDX_MAP(CQuoteBox)
		DDX_TEXT(IDC_EDIT_PATH, model.m_path)
		DDX_TEXT(IDC_STATIC_INFO,model.m_info)
		//DDX_TEXT(IDC_STATIC_ALLQUOTE, model.m_path)
		//DDX_TEXT(IDC_EDIT_PATH, model.m_path)
	END_DDX_MAP()
	//init_dialog、show都只运行一次。
	BEGIN_MSG_MAP(CQuoteView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		//NOTIFY_ID_HANDLER(IDC_EDIT_PATH,OnEditChanged)
		COMMAND_ID_HANDLER(IDC_EDIT_PATH,OnEditChanged)

		COMMAND_HANDLER(IDC_BUTTON_SELECT, BN_CLICKED, OnClickedButtonSelect)
		COMMAND_HANDLER(IDC_BUTTON_INSTALL, BN_CLICKED, OnClickedButtonInstall)
		CHAIN_MSG_MAP(CCtlColored<CQuoteView>)

	END_MSG_MAP()


	void SetVisible(CQuoteViewModel::State state)
	{
		switch (state)
		{
		case CQuoteViewModel::State::init:
		{
			::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT), true);
			::EnableWindow(GetDlgItem(IDC_EDIT_PATH), true);

			::EnableWindow(GetDlgItem(IDC_BUTTON_INSTALL), FALSE);  //安装按钮不可用

			::ShowWindow(GetDlgItem(IDC_PROGRESS_IMPORT), SW_HIDE);

		}
			break;

		case CQuoteViewModel::State::selected:
		{
			::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT), true);
			::EnableWindow(GetDlgItem(IDC_EDIT_PATH), true);

			::EnableWindow(GetDlgItem(IDC_BUTTON_INSTALL), TRUE); 

			::ShowWindow(GetDlgItem(IDC_PROGRESS_IMPORT), SW_HIDE);

		}
			break;

		case CQuoteViewModel::State::pending:
		{
			::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT), FALSE);
			::EnableWindow(GetDlgItem(IDC_EDIT_PATH),FALSE);

			::EnableWindow(GetDlgItem(IDC_BUTTON_INSTALL), FALSE);  //安装按钮不可见

			::ShowWindow(GetDlgItem(IDC_PROGRESS_IMPORT), SW_SHOW);

		}
			break;

		case CQuoteViewModel::State::complete:
		{
			::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT), TRUE);
			::EnableWindow(GetDlgItem(IDC_EDIT_PATH), TRUE);

			//::EnableWindow(GetDlgItem(IDC_BUTTON_INSTALL), FALSE); //安装按钮不可见

			//::ShowWindow(GetDlgItem(IDC_PROGRESS_IMPORT), SW_SHOW);

		}
			break;
		}

	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){

		SetVisible(model.m_state);
		//获取背景色
		HBRUSH bk = (HBRUSH)GetClassLong(GetParent(), GCL_HBRBACKGROUND);
		LOGBRUSH brush;
		::GetObjectW(bk, sizeof(LOGBRUSH), &brush);
		DWORD bkColor = brush.lbColor;

		//SetTextColor(::GetSysColor(COLOR_WINDOWTEXT)); 
		//SetBkBrush(bk);
		
		SetTextBackGround(bkColor);  // Lightish kind of blue (Four)
		SetTextColor(RGB(0, 0, 0)); // Dark red 
		


		model.m_path = L"";
		m_progressBar = GetDlgItem(IDC_PROGRESS_IMPORT);
		m_pathctrl.Attach(GetDlgItem(IDC_EDIT_PATH));
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
		wchar_t temp[MAX_PATH] = { 0 };
		GetDlgItemText(IDC_EDIT_PATH, temp, MAX_PATH);
		model.open(m_hWnd);
		if (temp != model.m_path)
		{
			
			DoDataExchange(false, IDC_EDIT_PATH);
			//仅在init或selected状态执行
			if (model.is_state_changed())
				SetVisible(model.m_state);
			DoDataExchange(FALSE, IDC_STATIC_INFO);
		}
		return 0;
	}
	LRESULT OnEditChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
	{
		//512=EN_KILLFOCUS WID=1004即控件的id，hwnd是控件的hwnd
		if (wNotifyCode == EN_KILLFOCUS)
		{
			wchar_t temp[MAX_PATH] = { 0 };
			GetDlgItemText(IDC_EDIT_PATH, temp,MAX_PATH);
			if (temp != model.m_path)
			{
				
				DoDataExchange(true, wID);
				//仅在init或selected状态执行
				if (model.is_state_changed())
				{
					SetVisible(model.m_state);
					
				}
				DoDataExchange(FALSE, IDC_STATIC_INFO);
			}
		}
		return 0;
	}


	LRESULT OnClickedButtonInstall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		model.m_state = CQuoteViewModel::State::pending;
		SetVisible(CQuoteViewModel::State::pending);
		wostringstream info;
		info << L"共" << model.parser.m_quote_count << L"条记录，正在安装...";
		model.m_info.clear();
		model.m_info.append(info.str());
		SetDlgItemTextW(IDC_STATIC_INFO, model.m_info.c_str());

		std::thread t(&CQuoteViewModel::import, &model, [this](const char *err,int now){
			if (err)
			{
				MessageBoxA(m_hWnd, err, "导入过程出错", 0);
			}
			else if (::IsWindow(m_progressBar.m_hWnd))
			{	
				m_progressBar.SetPos(now*100/model.parser.m_quote_count + 1);
				if (now == model.parser.m_quote_count)
				{
					model.m_info.append(L"完成！");
					
					model.m_state = CQuoteViewModel::State::complete;
					SetVisible(CQuoteViewModel::State::complete);

					SetDlgItemTextW(IDC_STATIC_INFO, model.m_info.c_str());
					model.parser.close(); //安装完毕后，清除缓存的Dad文件
				}
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

