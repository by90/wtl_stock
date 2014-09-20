#ifndef __TEXTPROGRESSCTRL_WTL_H__
#define __TEXTPROGRESSCTRL_WTL_H__

#pragma once

//////////////////////////////////////////////////////////////////////////
//
// Text Progress Ctrl for WTL made by Go Geon-young
// This is open source. So don't hassle me about it.
// 
//
//
//


#ifndef __cplusplus
#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
#error TextProgressCtrl_WTL.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
#error TextProgressCtrl_WTL.h requires atlctrls.h to be included first
#endif

#if (_WIN32_IE < 0x0400)
#error TextProgressCtrl_WTL.h requires _WIN32_IE >= 0x0400
#endif

//////////////////////////////////////////////////////////////////////////
//
//CMemDC Class http://dvinogradov.blogspot.com/2008/08/easy-double-buffering.html
//
//
//////////////////////////////////////////////////////////////////////////

class CMemDC : public WTL::CDC
{
public:    
	CMemDC(HDC PaintDC, const RECT &Area) : _PaintDC(PaintDC), _Area(Area)    
	{        
		CreateCompatibleDC(PaintDC);        
		POINT Viewport;        
		::GetViewportOrgEx(PaintDC, &Viewport);        
		SetViewportOrg(Viewport);        
		_Bitmap.CreateCompatibleBitmap(PaintDC, _Area.Width(), _Area.Height());       
		SelectBitmap(_Bitmap);    
	};    
	~CMemDC()    
	{        
		POINT OriginalViewport;        
		SetViewportOrg(0, 0);        
		_PaintDC.SetViewportOrg(0, 0, &OriginalViewport);       
		_PaintDC.BitBlt(_Area.left, _Area.top, _Area.Width(), _Area.Height(), m_hDC, 0, 0, SRCCOPY);       
		_PaintDC.SetViewportOrg(OriginalViewport.x, OriginalViewport.y);    
	}
private:    
	WTL::CBitmap _Bitmap;    
	WTL::CDCHandle _PaintDC;    
	CRect _Area;
};

//End of CMemDC



template< class T, class TBase = CProgressBarCtrl, class TWinTraits = CControlWinTraits >
class ATL_NO_VTABLE CTextProgressCtrl_WTLImpl : public CWindowImpl< T, TBase, TWinTraits >
{
public:
	typedef CTextProgressCtrl_WTLImpl< T, TBase, TWinTraits > thisClass;

	COLORREF	m_colFore,
		m_colBk,
		m_colTextFore,
		m_colTextBk;

	CToolTipCtrl m_tip;
	CFont m_font;

	CString m_strText;

	int	m_nPos, 
		m_nStepSize, 
		m_nMax, 
		m_nMin;

	BOOL m_bShowText;

	CTextProgressCtrl_WTLImpl() : m_colBk(CLR_INVALID), m_colFore(CLR_INVALID), m_colTextFore(CLR_INVALID), 
		m_colTextBk(CLR_INVALID)
	{
		m_nPos			= 0;
		m_nStepSize		= 1;
		m_nMax			= 100;
		m_nMin			= 0;
		m_strText.Empty();
		m_bShowText = TRUE;
	}


	// Operations


	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd==NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
		if( bRet )
			_Init();	//Initialize
		return bRet;
	}

	void SetText(LPCSTR str)
	{
		m_strText = str;
	}

	void SetForeColour(COLORREF col)
	{
		m_colFore = col;
	}

	void SetBkColour(COLORREF col)
	{
		m_colBk = col;
	}

	void SetTextForeColour(COLORREF col)
	{
		m_colTextFore = col;
	}

	void SetTextBkColour(COLORREF col)
	{
		m_colTextBk = col;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//Enable a Text in Progressbar
	//
	//////////////////////////////////////////////////////////////////////////

	void SetShowText(BOOL bShow)
	{ 
		if (::IsWindow(m_hWnd) && m_bShowText != bShow)
			Invalidate();

		m_bShowText = bShow;
	}


	void SetRange(int nLower, int nUpper)
	{
		m_nMax = nUpper;
		m_nMin = nLower;
	}

	int SetPos(int nPos) 
	{	
		if (!::IsWindow(m_hWnd))
			return -1;

		int nOldPos = m_nPos;
		m_nPos = nPos;

		CRect rect;
		GetClientRect(rect);

		double Fraction = (double)(m_nPos - m_nMin) / ((double)(m_nMax - m_nMin));
		int nBarWidth = (int) (Fraction * rect.Width());

		InvalidateRect(NULL);

		return nOldPos;
	}

	void _Init()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		CWindow wnd = GetParent();
		CFontHandle font = wnd.GetFont();
		if( !font.IsNull() ) {
			LOGFONT lf;
			font.GetLogFont(&lf);
			lf.lfWeight = FW_BOLD;
			m_font.CreateFontIndirect(&lf);
			SetFont(m_font);
		}

		// Set label (defaults to window text)
		int nLen = GetWindowTextLength();
		if( nLen > 0 ) {
			LPTSTR lpszText = (LPTSTR) _alloca((nLen + 1) * sizeof(TCHAR));
			if( GetWindowText(lpszText, nLen + 1) ) {
				// create a tool tip
				m_tip.Create(m_hWnd);
				ATLASSERT(m_tip.IsWindow());
				m_tip.Activate(TRUE);
				m_tip.AddTool(m_hWnd, lpszText);
			}
		}
	}

	BEGIN_MSG_MAP( thisClass )
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1; // We're painting it all
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CRect LeftRect, RightRect, ClientRect;
		GetClientRect(ClientRect);

		double Fraction = (double)(m_nPos - m_nMin) / ((double)(m_nMax - m_nMin));

		CPaintDC PaintDC(m_hWnd); // device context for painting
		CMemDC dc(PaintDC,ClientRect);
		//CPaintDC dc(this);    // device context for painting (if not double buffering)

		CFont font;
		font.CreatePointFont( 80, _T( "arial" ));
		HFONT hOldFont = dc.SelectFont(font);
		LeftRect = RightRect = ClientRect;

		LeftRect.right = LeftRect.left + (int)((LeftRect.right - LeftRect.left)*Fraction);
		dc.FillSolidRect(LeftRect, m_colFore);

		RightRect.left = LeftRect.right;
		dc.FillSolidRect(RightRect, m_colBk);

		if (m_bShowText)
		{
			CString str;
			if (m_strText.GetLength())
				str = m_strText;
			else
				str.Format(_T( "%d%%" ) , (int)(Fraction*100.0));

			dc.SetBkMode(TRANSPARENT);

			CRgn rgn;
			rgn.CreateRectRgn(LeftRect.left, LeftRect.top, LeftRect.right, LeftRect.bottom);
			dc.SelectClipRgn(rgn);
			dc.SetTextColor(m_colTextBk);

			dc.DrawText(str, -1, ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			rgn.DeleteObject();
			rgn.CreateRectRgn(RightRect.left, RightRect.top, RightRect.right, RightRect.bottom);
			dc.SelectClipRgn(rgn);
			dc.SetTextColor(m_colTextFore);

			dc.DrawText(str, -1, ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		dc.SelectFont( hOldFont );
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		_Init();
		return 0;
	}

};

class CTextProgressCtrl_WTL : public CTextProgressCtrl_WTLImpl<CTextProgressCtrl_WTL>
{
public:
	DECLARE_WND_CLASS(_T("WTL_TextProgressCtrl"))
};

#endif