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

// Operations
	static void	FrameToTime(int iFrame, COleDateTime& dt, float fFrameRate);
	static int TimeToFrame(const COleDateTime& dt, float fFrameRate);
	static void	FrameToTimeString(int iFrame, CString& sTime, float fFrameRate);
	static bool	TimeStringToFrame(CString sTime, int& iFrame, float fFrameRate);

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

// Public data
	int		m_iFrameSizePreset;	// frame size preset; see enum above
	CSize	m_szFrame;			// size of frame, in device-independent pixels
	int		m_iScaleToFit;		// scale to fit type; see enum in COptions
	int		m_iFrameSelType;	// frame selection type; see enum above
	int		m_nRangeStart;		// first frame of range
	int		m_nRangeEnd;		// last frame of range, inclusive
	int		m_nDuration;		// number of frames in range: last - first + 1
	int		m_iTimeUnit;		// time unit; see enum above
	float	m_fFrameRate;		// frame rate in frames per second

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Dialog Data
	enum { IDD = IDD_MOVIE_EXPORT };
	CComboBox m_comboFrameSize;		// preset frame size drop list
	CComboBox m_comboScaleToFit;	// scale to fit options drop list
	int		m_nFrameCount;		// actual number of frames in movie file

// Overrides
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Helpers
	void	UpdateFrameSize(int iFrameSizePreset);
	void	FrameToTimeString(int iFrame, CString& sTime) const;
	bool	TimeStringToFrame(CString sTime, int& iFrame) const;
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
