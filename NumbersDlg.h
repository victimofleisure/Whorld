// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version

        display view parameters as numbers
 
*/

#if !defined(AFX_NUMBERSDLG_H__583BF356_FB65_4584_B723_AD9F32A0E27A__INCLUDED_)
#define AFX_NUMBERSDLG_H__583BF356_FB65_4584_B723_AD9F32A0E27A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumbersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumbersDlg dialog

#include "ToolDlg.h"
#include "ParmInfo.h"

class CMainFrame;
class CWhorldView;

class CNumbersDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CNumbersDlg);
// Construction
public:
	CNumbersDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL	Create(UINT nIDTemplate, CMainFrame *pFrame);

// Operations
	void	TimerHook();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumbersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CNumbersDlg)
	enum { IDD = IDD_NUMBERS };
	CStatic	m_Caps;
	CStatic	m_Vals;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CNumbersDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	CMainFrame	*m_Frm;
	CWhorldView	*m_View;

// Helpers
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMBERSDLG_H__583BF356_FB65_4584_B723_AD9F32A0E27A__INCLUDED_)
