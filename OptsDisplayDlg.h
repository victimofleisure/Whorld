// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22jun05	initial version
		01		06feb08	increase frame rate range

        display options property page
 
*/

#if !defined(AFX_OPTSDISPLAYDLG_H__81298FE8_B61C_49BB_9AED_59E8AEAE9287__INCLUDED_)
#define AFX_OPTSDISPLAYDLG_H__81298FE8_B61C_49BB_9AED_59E8AEAE9287__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsDisplayDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsDisplayDlg dialog

#include "PersistVal.h"
#include "NumEdit.h"
#include "NumSpin.h"

class COptsDisplayDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(COptsDisplayDlg)
// Construction
public:
	COptsDisplayDlg();
	
// Constants
	enum {	// back buffer location
		BB_AUTO,	// we decide
		BB_SYSTEM,	// system memory
		BB_VIDEO	// video memory
	};

// Attributes
	bool	UseDirDraw() const;
	int		GetOptions() const;
	int		GetFrameRate() const;
	bool	AllowExclusive() const;

// Operations
	void	SetDefaults();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsDisplayDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsDisplayDlg)
	enum { IDD = IDD_OPTS_DISPLAY };
	CNumSpin	m_FrameRateSpin;
	CNumEdit	m_FrameRate;
	BOOL	m_UseDirDraw;
	int		m_BackBufLoc;
	BOOL	m_PrintFromBitmap;
	BOOL	m_MirrorPrecise;
	BOOL	m_AllowExclusive;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsDisplayDlg)
	afx_msg void OnUseDirdraw();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBackBufLoc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSTATE {
		bool	UseDirDraw;		// if true, use DirectDraw
		bool	MirrorPrecise;	// if true, DirectDraw mirrors to intermediate buffer
		bool	AllowExclusive;	// if true, DirectDraw's exclusive mode is allowed
		bool	PrintFromBitmap;	// if true, print draws to device-independent 
									// bitmap and blits; see CWhorldView::OnPrint
		int		BackBufLoc;		// back buffer location; see enum above
		int		FrameRate;		// frame rate, in frames per second
	} STATE;

// Constants
	enum {
		FPS_MIN = 1,	// minimum frame rate
		FPS_MAX = 200,	// maximum frame rate
		FPS_DEF = 25	// default frame rate
	};
	static	const STATE	m_Default;

// Member data
	STATE	m_st;
	PersistVal<bool>	m_RegUseDirDraw;
	PersistVal<bool>	m_RegMirrorPrecise;
	PersistVal<bool>	m_RegAllowExclusive;
	PersistVal<bool>	m_RegPrintFromBitmap;
	PersistVal<int>		m_RegBackBufLoc;
	PersistVal<int>		m_RegFrameRate;

// Helpers
	void	UpdateUI();
};

inline bool COptsDisplayDlg::UseDirDraw() const
{
	return(m_st.UseDirDraw);
}

inline int COptsDisplayDlg::GetFrameRate() const
{
	return(m_st.FrameRate);
}

inline bool COptsDisplayDlg::AllowExclusive() const
{
	return(m_st.AllowExclusive);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSDISPLAYDLG_H__81298FE8_B61C_49BB_9AED_59E8AEAE9287__INCLUDED_)
