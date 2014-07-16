#include "stdafx.h"
#include <atlcrack.h>
#include <atlmisc.h>

template<int ID>
class CBaseDialog : public CDialogImpl<CBaseDialog<ID> >{
public:
	enum { IDD = ID };

	BEGIN_MSG_MAP(BaseDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){
		return TRUE;
	}
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled){
		// 子窗口直接填满整个对话框
#if 1
		CRect rect_(
			0,
			0,
			GET_X_LPARAM(lParam),
			GET_Y_LPARAM(lParam));
#else
		CRect rect_(
			m_client_rect_.left,
			m_client_rect_.top,
			GET_X_LPARAM(lParam) - m_client_rect_.right,
			GET_Y_LPARAM(lParam) - m_client_rect_.bottom);
#endif
		m_view_.MoveWindow(&rect_);
		return TRUE;
	}
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
		int ret_ = m_view_.IsClose();
		if (0xFFFFFFFF != ret_)
			EndDialog(ret_);
		return TRUE;
	}
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
		return TRUE;
	}

protected:
	BASE_VIEW   m_view_;
	//CRect     m_client_rect_;
};




