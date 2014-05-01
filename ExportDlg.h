// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28jun05	initial version

        export options dialog
 
*/

#if !defined(AFX_EXPORTDLG_H__A176F6DA_BD7B_419B_8B20_611312AF8243__INCLUDED_)
#define AFX_EXPORTDLG_H__A176F6DA_BD7B_419B_8B20_611312AF8243__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

#include "PersistVal.h"

class CExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CExportDlg);
// Construction
public:
	CExportDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	bool	GetSize(CSize& Size) const;
	bool	GetScaleToFit() const;
	int		GetResolution() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
	BOOL	m_UseWndSize;
	int		m_Width;
	int		m_Height;
	int		m_Resolution;
	int		m_ScaleToFit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnExpUseWndSize();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		DEFAULT_RESOLUTION = 72
	};

// Types
	typedef struct tagSTATE {
		bool	UseWndSize;		// if true, make bitmap the same size as window
		int		Width;			// bitmap width, in pixels
		int		Height;			// bitmap height, in pixels
		bool	ScaleToFit;		// if true, preserve cropping by scaling bitmap
		int		Resolution;		// bitmap resolution, in pixels per inch
	} STATE;

// Member data
	STATE	m_st;
	PersistVal<bool>	m_RegUseWndSize;
	PersistVal<int>		m_RegWidth;
	PersistVal<int>		m_RegHeight;
	PersistVal<bool>	m_RegScaleToFit;
	PersistVal<int>		m_RegResolution;

// Helpers
	void	UpdateUI();
};

bool inline  CExportDlg::GetScaleToFit() const
{
	return(m_st.ScaleToFit);
}

int inline  CExportDlg::GetResolution() const
{
	return(m_st.Resolution);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__A176F6DA_BD7B_419B_8B20_611312AF8243__INCLUDED_)
