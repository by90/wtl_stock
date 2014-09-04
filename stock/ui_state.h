#include <atlwin.h>

void SetVisble(ATL::CWindow *_dialog,bool _visble,int _first)
{
	::ShowWindow(_dialog->GetDlgItem(_first), _visble ? SW_SHOW : SW_HIDE);
}
template <typename... Args>
void SetVisble(ATL::CWindow *_dialog,bool _visble,int _first,Args ...args)
{
	::ShowWindow(_dialog->GetDlgItem(_first),_visble? SW_SHOW:SW_HIDE);
	SetVisble(_dialog,_visble,...args)
}

void SetEnable(ATL::CWindow *_dialog, bool _visble, int _first)
{
	::EnableWindow(_dialog->GetDlgItem(_first), _visble);
}
template <typename... Args>
void SetEnable(ATL::CWindow *_dialog, bool _visble, int _first, Args ...args)
{
	::EnableWindow(_dialog->GetDlgItem(_first), _visble);
	SetEnable(_dialog, _visble, args...);
}