#include <atldlgs.h>
#include <atlctrls.h>
#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <atlddx.h>
#include <sstream>
#include "resource.h"
#include "dad_file_parse.h"
#include "ThreadTask.h"
#include "stdmore.h"
#include <iomanip>
#include "CCtlColor.h"
#include "timer.h"
#include "atlddxEx.h"
#include "db_quote.h"
#include "import_view_model.h"

#ifndef STOCK_STOCK_IMPORT_VIEW_H
#define STOCK_STOCK_IMPORT_VIEW_H

class ImportView : public CDialogImpl<ImportView>
	, public CWinDataExchangeEx<ImportView>
	, public CCtlColored<ImportView>
{
public:
	ImportViewModel view_model_; //包含一个该对象
	CProgressBarCtrl m_progressBar;
	CEdit m_pathctrl;

	enum { IDD = IDD_QUOTE_BOX };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);		
	}

	BEGIN_DDX_MAP(CQuoteBox)
		DDX_TEXT(IDC_EDIT_PATH, view_model_.selected_file_)
		DDX_TEXT(IDC_STATIC_INFO, view_model_.progress_info_)
		DDX_TEXT(IDC_STATIC_SAVED, view_model_.installed_info_)
		DDX_TEXT(IDC_STATIC_OPENED, view_model_.selected_info_)


		//DDX_RADIO(IDC_RADIO_QUOTE, model.install_type_quote)
		//DDX_RADIO(IDC_RADIO_EXRIGHT, model.install_type_exright)
		//DDX_RADIO(IDC_RADIO_BASE, model.install_type_base)
		//DDX_RADIO(IDC_RADIO_CATALOG, model.install_type_catalog)


	END_DDX_MAP()
	//init_dialog、show都只运行一次。
	BEGIN_MSG_MAP(ImportView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		//NOTIFY_ID_HANDLER(IDC_EDIT_PATH,OnEditChanged)
		COMMAND_ID_HANDLER(IDC_EDIT_PATH, OnEditChanged)
		COMMAND_RANGE_HANDLER(IDC_RADIO_QUOTE, IDC_RADIO_CATALOG, OnRadioClick)

		COMMAND_HANDLER(IDC_BUTTON_SELECT, BN_CLICKED, OnClickedButtonSelect)
		COMMAND_HANDLER(IDC_BUTTON_INSTALL, BN_CLICKED, OnClickedButtonInstall)
		COMMAND_HANDLER(IDC_BUTTON_REMOVE, BN_CLICKED, OnClickedButtonRemove)
		CHAIN_MSG_MAP(CCtlColored<ImportView>)

	END_MSG_MAP()


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){


		//获取背景色
		HBRUSH bk = (HBRUSH)GetClassLong(GetParent(), GCL_HBRBACKGROUND);
		LOGBRUSH brush;
		::GetObjectW(bk, sizeof(LOGBRUSH), &brush);
		DWORD bkColor = brush.lbColor;

		//SetTextColor(::GetSysColor(COLOR_WINDOWTEXT)); 
		//SetBkBrush(bk);		
		SetTextBackGround(bkColor);
		SetTextColor(RGB(0, 0, 0));
		//DeleteObject(bk);//delete居然导致框架窗口颜色设置失效？

		m_progressBar = GetDlgItem(IDC_PROGRESS_IMPORT);
		//m_pathctrl.Attach(GetDlgItem(IDC_EDIT_PATH));
		//初始化Radio Button
		CheckRadioButton(IDC_RADIO_QUOTE, IDC_RADIO_CATALOG, IDC_RADIO_QUOTE);

		view_model_ = ImportViewModel(this);
		view_model_.Init();
		//DoDataExchange(FALSE);

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
		view_model_.SelectFile();
		return 0;
	}
	LRESULT OnEditChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
	{
		//512=EN_KILLFOCUS WID=1004即控件的id，hwnd是控件的hwnd
		if (wNotifyCode == EN_KILLFOCUS)
			view_model_.CheckFile();
		return 0;
	}


	LRESULT OnClickedButtonInstall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		view_model_.RunImportFile();
		return 0;
	}

	LRESULT OnClickedButtonRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		view_model_.RemoveInstalledQuote();
		return 0;
	}
	LRESULT OnRadioClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		view_model_.ChangeImportFileType(wID);
		return 0;
	}
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
};

#endif