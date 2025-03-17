// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03sep05	initial version
        01      15mar25	refactor

        movie export options dialog
 
*/

#pragma once

// MovieExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg dialog

class CMovieExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CMovieExportDlg);
// Construction
public:
	CMovieExportDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	UINT	GetExportFlags() const;

// Public data
	int		m_iFrameSizePreset;
	CSize	m_szFrame;
	int		m_iScaleToFit;
	int		m_iFrameSelType;
	int		m_nRangeStart;
	int		m_nRangeEnd;
	int		m_nDuration;
	int		m_iTimeUnit;
	float	m_fFrameRate;

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Constants
	enum {	// frame size presets
		FSP_MOVIE,		// use movie's frame size
		FSP_VIEW,		// use view's frame size
		FSP_CUSTOM,		// specify a custom frame size
		FRAME_SIZE_PRESETS
	};
	enum {	// frame selection types
		FST_ALL,		// select all frames
		FST_RANGE,		// select specified range of frames
		FRAME_SELECTION_TYPES
	};
	enum {	// time units
		UNIT_TIME,		// time in hours, minutes and seconds
		UNIT_FRAMES,	// time in frames at specifed frame rate
		TIME_UNITS
	};
	enum {
		SECONDS_PER_DAY = 24 * 3600		// for COleDateTime conversion
	};

// Dialog Data
	enum { IDD = IDD_MOVIE_EXPORT };
	CComboBox m_comboFrameSize;		// preset frame size drop list
	CComboBox m_comboScaleToFit;	// scale to fit options drop list
	int		m_nFrameCount;	// actual number of frames in the movie file

// Overrides
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Helpers
	void	UpdateFrameSize(int iFrameSizePreset);
	static void	FrameToTime(int nFrame, COleDateTime& dt, float fFrameRate);
	static int	TimeToFrame(const COleDateTime& dt, float fFrameRate);
	static CString	FrameToTimeString(int nFrame, float fFrameRate);
	static bool	TimeStringToFrame(CString sTime, int& nFrame, float fFrameRate);
	CString	FrameToTimeString(int nFrame) const;
	bool	TimeStringToFrame(CString sTime, int& nFrame) const;
	void	DDX_FrameTime(CDataExchange* pDX, int nIDC, int& value) const;
	void	UpdateDuration();
	void	UpdateRangeEnd();
	void	UpdateFrameSelection(int iFrameSelType);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnUpdateFrameSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateScaleToFit(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFrameSelType(CCmdUI *pCmdUI);
	afx_msg void OnSelchangeFrameSize();
	afx_msg void OnClickedFrameSelType(UINT nID);
	afx_msg void OnClickedTimeUnit(UINT nID);
	afx_msg void OnKillFocusRange();
	afx_msg void OnKillFocusDuration();
	afx_msg void OnUpdateFrameRate(CCmdUI *pCmdUI);
};
