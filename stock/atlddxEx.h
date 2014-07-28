
#include <atlddx.h>
#include <vector>

//扩展ddx机制，支持string和wstring
#ifndef __ATLDDXEX_H__
#define __ATLDDXEX_H__

#pragma once
namespace WTL
{
	template <class T>
	class CWinDataExchangeEx :public CWinDataExchange<T>
	{
	public:

		//#ifdef _WTL_USE_STD_STRING

		BOOL DDX_Text(UINT nID, std::basic_string<TCHAR>& strText, int /*cbSize*/, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
		{
			T* pT = static_cast<T*>(this);
			BOOL bSuccess = TRUE;

			if (bSave)
			{
				HWND hWndCtrl = pT->GetDlgItem(nID);
				int nLen = ::GetWindowTextLength(hWndCtrl);
				std::vector<TCHAR> buf(nLen + 1);
				int nRetLen = ::GetWindowText(hWndCtrl, &buf[0], buf.size());
				buf.resize(nRetLen);
				strText.assign(buf.begin(), buf.end());
				if (nRetLen < nLen)
					bSuccess = FALSE;
			}
			else
			{
				bSuccess = pT->SetDlgItemText(nID, strText.c_str());
			}

			if (!bSuccess)
			{
				pT->OnDataExchangeError(nID, bSave);
			}
			else if (bSave && bValidate) // validation
			{
				ATLASSERT(nLength > 0);
				if (strText.size() > nLength)
				{
					_XData data;
					data.nDataType = ddxDataText;
					data.textData.nLength = strText.size();
					data.textData.nMaxLength = nLength;
					pT->OnDataValidateError(nID, bSave, data);
					bSuccess = FALSE;
				}
			}
			return bSuccess;
		}
		//#endif // _USE_STD_STRING_
	};

}; // namespace WTL

#endif // __ATLDDXEX_H__