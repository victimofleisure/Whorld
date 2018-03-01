// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05feb05	initial version
		01		29jul05	override SetHotKey too

		extended hot key control
 
*/

// HotKeyCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "HotKeyCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHotKeyCtrlEx

IMPLEMENT_DYNAMIC(CHotKeyCtrlEx, CHotKeyCtrl);

CHotKeyCtrlEx::CHotKeyCtrlEx()
{
}

CHotKeyCtrlEx::~CHotKeyCtrlEx()
{
}

DWORD CHotKeyCtrlEx::GetHotKey() const
{
	// Our override returns a DWORD, with the virtual key code in the low word,
	// and the modifier flags in the high word, which is what the base class is
	// SUPPOSED to do; the base class function returns a WORD, with the virtual
	// key code in the low *byte*, and the modifier flags in the high *byte*.
	WORD	VKey, Mods;
	CHotKeyCtrl::GetHotKey(VKey, Mods);
	return(MAKELONG(VKey, Mods));
}

void CHotKeyCtrlEx::SetHotKey(DWORD Key)
{
	CHotKeyCtrl::SetHotKey(LOWORD(Key), HIWORD(Key));
}

BEGIN_MESSAGE_MAP(CHotKeyCtrlEx, CHotKeyCtrl)
	//{{AFX_MSG_MAP(CHotKeyCtrlEx)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHotKeyCtrlEx message handlers

void CHotKeyCtrlEx::OnKillFocus(CWnd* pNewWnd) 
{
	CHotKeyCtrl::OnKillFocus(pNewWnd);
	CWnd	*Parent = GetParent();
	if (Parent != NULL) {
		NMHDR	nmh;
		nmh.hwndFrom = m_hWnd;
		nmh.idFrom = GetDlgCtrlID();
		nmh.code = HKN_KILLFOCUS;
		Parent->SendMessage(WM_NOTIFY, nmh.idFrom, long(&nmh));
	}
}
