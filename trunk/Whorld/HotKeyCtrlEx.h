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

#if !defined(AFX_HOTKEYCTRLEX_H__7ED61862_3490_4A02_9A2B_73BA3187F4D4__INCLUDED_)
#define AFX_HOTKEYCTRLEX_H__7ED61862_3490_4A02_9A2B_73BA3187F4D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HotKeyCtrlEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHotKeyCtrlEx window

#define HKN_KILLFOCUS	0x100	// non-standard notification

class CHotKeyCtrlEx : public CHotKeyCtrl
{
	DECLARE_DYNAMIC(CHotKeyCtrlEx);
// Construction
public:
	CHotKeyCtrlEx();

// Attributes
public:
	DWORD	GetHotKey() const;	// overrides defective base class function
	void	SetHotKey(DWORD Key);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHotKeyCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHotKeyCtrlEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHotKeyCtrlEx)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOTKEYCTRLEX_H__7ED61862_3490_4A02_9A2B_73BA3187F4D4__INCLUDED_)
