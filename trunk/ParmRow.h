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

        parameter row form
 
*/

#if !defined(AFX_PARMROW_H3__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_)
#define AFX_PARMROW_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParmRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParmRow dialog

#include "RowDialogRow.h"
#include "NumEdit.h"
#include "NumSpin.h"
#include "ClickSliderCtrl.h"
#include "Oscillator.h"
#include "ParmInfo.h"

class CParmRow : public CRowDialogRow
{
	DECLARE_DYNAMIC(CParmRow);
// Construction
public:
	CParmRow(CWnd* pParent = NULL);   // standard constructor

// Attributes
	double	GetVal();
	void	SetVal(double Val);
	void	GetInfo(CParmInfo::ROW& Info) const;
	void	SetInfo(const CParmInfo::ROW& Info);
	void	SetSliderRange(double MinVal, double MaxVal, int Steps);
	void	SetCaption(LPCTSTR Title);
	void	SetScale(double Scale);
	void	EnableLFO(bool Enable);
	static	int		GetWaveID(int Idx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParmRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CParmRow)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeModWave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog Data
	//{{AFX_DATA(CParmRow)
	enum { IDD = IDD_PARM_ROW };
	CNumSpin	m_ParmSpin;
	CNumSpin	m_ModAmpSpin;
	CNumSpin	m_ModFreqSpin;
	CNumSpin	m_ModPWSpin;
	CStatic	m_Title;
	CNumEdit	m_ParmEdit;
	CClickSliderCtrl	m_ParmSlider;
	CComboBox	m_ModWave;
	CNumEdit	m_ModAmp;
	CNumEdit	m_ModFreq;
	CNumEdit	m_ModPW;
	//}}AFX_DATA

// Constants
	enum {
		WAVEFORMS = COscillator::WAVEFORMS
	};
	static const int m_WaveID[WAVEFORMS];

// Member data
	double	m_MinVal;
	double	m_MaxVal;
	int		m_Steps;

// Helpers
	void	UpdateSlider();
	void	NotifyEdit(int CtrlID);
};

inline int CParmRow::GetWaveID(int Idx)
{
	ASSERT(Idx >= 0 && Idx < COscillator::WAVEFORMS);
	return(m_WaveID[Idx]);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARMROW_H__A2704F05_FC3B_4FF6_AAEF_9A1FB2527928__INCLUDED_)
