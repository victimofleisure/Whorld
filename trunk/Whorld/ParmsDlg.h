// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		28jan08	support Unicode

        parameters dialog
 
*/

#if !defined(AFX_PARMSDLG_H__65C902A8_4AEF_484E_B327_638A29A2AE07__INCLUDED_)
#define AFX_PARMSDLG_H__65C902A8_4AEF_484E_B327_638A29A2AE07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParmsDlg.h : header file
//

#include "RowDialog.h"
#include "ParmRow.h"

/////////////////////////////////////////////////////////////////////////////
// CParmsDlg dialog

class CMainFrame;

class CParmsDlg : public CRowDialog
{
	DECLARE_DYNAMIC(CParmsDlg);
// Construction
public:
	CParmsDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL	Create(UINT nIDTemplate, CMainFrame *pFrame);

// Constants
	enum {
		ROWS = CParmInfo::ROWS,
		COLS = 7
	};

// Attributes
	void	GetInfo(CParmInfo& Info) const;
	void	SetInfo(const CParmInfo& Info);
	void	GetRow(int Idx, CParmInfo::ROW& Row) const;
	void	SetRow(int Idx, const CParmInfo::ROW& Row);
	double	GetVal(int Idx) const;
	void	SetVal(int Idx, double Val);
	void	SetDocTitle(LPCTSTR Title);
	void	SetOffsetMode(bool Enable);

// Operations
	void	Reset();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParmsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CParmsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	//}}AFX_MSG
	afx_msg LRESULT OnParmRowEdit(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Dialog Data
	//{{AFX_DATA(CParmsDlg)
	enum { IDD = IDD_PARMS };
	//}}AFX_DATA

// Constants
	enum {
		EDIT_SEL_SYSCMD = 0xff00
	};
	static const COLINFO	m_ColInfo[COLS];

// Member data
	CMainFrame	*m_Frm;			// pointer to main frame
	int		m_RowPos[ROWS];		// display position of each row
	CString	m_Caption;			// dialog caption

// Overrides
	CWnd	*CreateRow(int Idx, int& Pos);

// Helpers
	CParmRow	*GetRow(int Idx) const;
};

inline CParmRow	*CParmsDlg::GetRow(int Idx) const
{
	return((CParmRow *)m_Row[Idx]);
}

inline void CParmsDlg::GetRow(int Idx, CParmInfo::ROW& Row) const
{
	GetRow(Idx)->GetInfo(Row);
}

inline void CParmsDlg::SetRow(int Idx, const CParmInfo::ROW& Row)
{
	GetRow(Idx)->SetInfo(Row);
}

inline double CParmsDlg::GetVal(int Idx) const
{
	return(GetRow(Idx)->GetVal());
}

inline void CParmsDlg::SetVal(int Idx, double Val)
{
	GetRow(Idx)->SetVal(Val);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARMSDLG_H__65C902A8_4AEF_484E_B327_638A29A2AE07__INCLUDED_)
