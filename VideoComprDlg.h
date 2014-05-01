// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version
        01      16aug06	move non-GUI implementation to VideoComprList
		02		05mar07	add CreateCompr for non-interactive use

        Video Compression dialog

*/

#if !defined(AFX_VIDEOCOMPRDLG_H__DE80C91B_DF37_45B4_9942_C3C0B37CBC9A__INCLUDED_)
#define AFX_VIDEOCOMPRDLG_H__DE80C91B_DF37_45B4_9942_C3C0B37CBC9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoComprDlg.h : header file
//

#include <afxtempl.h>
#include "DirShowU.h"
#include "VideoComprList.h"

/////////////////////////////////////////////////////////////////////////////
// CVideoComprDlg dialog

class CVideoComprDlg : public CDialog
{
	DECLARE_DYNCREATE(CVideoComprDlg)

// Construction
public:
	CVideoComprDlg(CWnd* pParent = NULL);
	~CVideoComprDlg();

// Attributes
	bool	SetFilters(IBaseFilter *pSource, IBaseFilter *pDest);
	IBaseFilter *GetCompr() const;
	void	GetState(CVideoComprState& State) const;
	void	SetState(const CVideoComprState& State);
	bool	HaveState() const;

// Operations
	bool	CreateCompr();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoComprDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CVideoComprDlg)
	enum { IDD = IDD_VIDEO_COMPR };
	CStatic	m_QualNum;
	CSliderCtrl	m_Quality;
	CComboBox	m_Combo;
	int		m_KeyFrameEdit;
	int		m_PFramesEdit;
	int		m_DataRateEdit;
	int		m_WndSizeEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CVideoComprDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAbout();
	afx_msg void OnConfig();
	afx_msg void OnSelchangeCombo();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKillfocusKeyFrameEdit();
	afx_msg void OnKillfocusPFramesEdit();
	virtual void OnOK();
	afx_msg void OnKillfocusDataRateEdit();
	afx_msg void OnKillfocusWndSizeEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef CVideoComprList::CAPS CAPS;

// Member data
	CVideoComprList		m_vc;		// list of video compressors
	CVideoComprState	m_State;	// saved state of selected compressor

// Helpers
	bool	SelectCompr();
	void	UpdateQuality();
	void	UpdateUI();
};

inline IBaseFilter *CVideoComprDlg::GetCompr() const
{
	return(m_vc.GetCompr());
}

inline void	CVideoComprDlg::GetState(CVideoComprState& State) const
{
	State = m_State;
}

inline void CVideoComprDlg::SetState(const CVideoComprState& State)
{
	m_State = State;
}

inline bool CVideoComprDlg::HaveState() const
{
	return(!m_State.m_Name.IsEmpty());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOCOMPRDLG_H__DE80C91B_DF37_45B4_9942_C3C0B37CBC9A__INCLUDED_)
