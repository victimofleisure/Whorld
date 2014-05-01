// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15jul05	initial version
		01		17feb06	add ReplaceRows and RemoveAllRows
		02		28jan08	support Unicode

        create a form view of row windows within a dialog
 
*/

#if !defined(AFX_ROWDIALOG_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
#define AFX_ROWDIALOG_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RowDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRowDialog dialog

#include "ViewDialog.h"

class CRowDialogForm;

class CRowDialog : public CViewDialog
{
	DECLARE_DYNAMIC(CRowDialog);
// Construction
public:
	CRowDialog(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd* pParent = NULL);

// Types
	typedef struct tagCOLINFO {
		int	CtrlID;
		int	TitleID;
	} COLINFO; 

// Attributes
	int		GetRows() const;
	int		GetCols() const;
	CWnd	*GetRow(int Idx) const;
	int		GetHeaderHeight() const;
	int		GetTopMargin() const;
	int		GetActiveRow() const;

// Operations
	bool	CreateCols(int Cols, const COLINFO *ColInfo);
	bool	CreateRows(int Rows, int TopMargin = 0);
	bool	ReplaceRows(int Rows);
	void	RemoveAllRows();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRowDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRowDialog)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRowDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg LRESULT OnRowDialogTab(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const CRect	m_Margin;	// extra margins around view
	enum {
		HEADER_HEIGHT	= 18,	// height of header control, in logical coords
		SCROLLBAR_WIDTH = 24	// approx width of scrollbar, in logical coords
	};

// Member data
	int		m_Cols;				// number of columns in list
	const	COLINFO	*m_ColInfo;	// array of information about each column
	CRowDialogForm	*m_Form;	// pointer to form view, parent of rows
	CPtrArray	m_Row;			// array of pointers to row objects
	CSize		m_MaxSize;		// maximum size of form view
	CHeaderCtrl	m_Hdr;			// list's header control
	int		m_TopMargin;		// extra top margin for derived controls
	int		m_HdrHeight;		// height of header control

// Overridables
	virtual	CWnd	*CreateRow(int Idx, int& Pos);

// Helpers
	void	MoveHeader();
	bool	IsTabStop(int Col);
	bool	AddRows(int Rows);
};

inline int CRowDialog::GetRows() const
{
	return(m_Row.GetSize());
}

inline int CRowDialog::GetCols() const
{
	return(m_Cols);
}

inline CWnd *CRowDialog::GetRow(int Idx) const
{
	return((CWnd *)m_Row[Idx]);
}

inline int CRowDialog::GetHeaderHeight() const
{
	return(m_HdrHeight);
}

inline int CRowDialog::GetTopMargin() const
{
	return(m_TopMargin);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROWDIALOG_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
