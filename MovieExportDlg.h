// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03sep05	initial version
		01		03may06	add optional path and frame range
		02		04may06	add FrameRange arg to UpdateUI
		03		02oct06	make output frame rate a float
		04		28jan08	support Unicode

        movie export options dialog
 
*/

#if !defined(AFX_MOVIEEXPORTDLG_H__4CC2AC1D_27E4_4E72_BE34_1CA65053853D__INCLUDED_)
#define AFX_MOVIEEXPORTDLG_H__4CC2AC1D_27E4_4E72_BE34_1CA65053853D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovieExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg dialog

#include "PersistVal.h"

class CWhorldView;

class CMovieExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CMovieExportDlg);
// Construction
public:
	CMovieExportDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	CSize	GetFrameSize() const;
	bool	GetScaleToFit() const;
	void	GetUncomprSize(ULARGE_INTEGER& Size) const;
	void	SetInput(CSize FrameSize, int FrameCount, int FrameRate);

// Operations
	int		Export(LPCTSTR SrcPath = NULL, int FirstFrame = -1, int LastFrame = -1);
	static	void	FrameToTime(int Frame, int FrameRate, CString& Time);
	static	int		TimeToFrame(LPCTSTR Time, int FrameRate);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovieExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMovieExportDlg)
	enum { IDD = IDD_MOVIE_EXPORT };
	CComboBox	m_OutSize;
	int		m_ScaleToFit;
	int		m_OutHeight;
	int		m_OutWidth;
	CString	m_OutLength;
	int		m_RangeSubset;
	BOOL	m_ExportBitmaps;
	BOOL	m_DisplayOutput;
	CString	m_RangeFirstEdit;
	CString	m_RangeLastEdit;
	float	m_OutFrameRate;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMovieExportDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeSizeCombo();
	afx_msg void OnKillfocusOutFrameRate();
	afx_msg void OnRangeType();
	afx_msg void OnKillfocusRange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSTATE {
		int		Width;		// frame width, in pixels
		int		Height;		// frame height, in pixels
		bool	ScaleToFit;	// if true, preserve cropping by scaling frames
	} STATE;

// Constants
	static const SIZE	m_StandardSize[];

// Member data
	STATE	m_st;
	PersistVal<int>		m_RegWidth;
	PersistVal<int>		m_RegHeight;
	PersistVal<bool>	m_RegScaleToFit;
	CSize	m_InFrameSize;	// input frame size, in pixels
	int		m_InFrameCount;	// input frame count
	int		m_InFrameRate;	// input frame rate, in frames per second
	int		m_RangeFirst;	// if range subset, first frame to export
	int		m_RangeLast;	// if range subset, last frame to export

// Helpers
	void	UpdateUI(bool RangeChange = FALSE);
	bool	IsCustomSize() const;
	static	bool	CheckDiskSpace(LPCTSTR Folder, ULARGE_INTEGER Size);
};

inline bool CMovieExportDlg::IsCustomSize() const
{
	return(m_OutSize.GetCurSel() == m_OutSize.GetCount() - 1);
}

inline CSize CMovieExportDlg::GetFrameSize() const
{
	return(CSize(m_st.Width, m_st.Height));
}

bool inline  CMovieExportDlg::GetScaleToFit() const
{
	return(m_st.ScaleToFit);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVIEEXPORTDLG_H__4CC2AC1D_27E4_4E72_BE34_1CA65053853D__INCLUDED_)
