// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27sep05	initial version

        movie export options dialog
 
*/

#if !defined(AFX_MOVIERESIZEDLG_H__E2CD3198_8716_496C_888A_22A27B042D8F__INCLUDED_)
#define AFX_MOVIERESIZEDLG_H__E2CD3198_8716_496C_888A_22A27B042D8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovieResizeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMovieResizeDlg dialog

class CMovieResizeDlg : public CDialog
{
// Construction
public:
	CMovieResizeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMovieResizeDlg)
	enum { IDD = IDD_MOVIE_RESIZE };
	int		m_Height;
	int		m_Width;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovieResizeDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMovieResizeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVIERESIZEDLG_H__E2CD3198_8716_496C_888A_22A27B042D8F__INCLUDED_)
