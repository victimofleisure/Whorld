// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28apr05	initial version
		01		02oct05	add doc to match main frame selection indices
		02		23mar06	change INFO booleans from int to bool
		03		17apr06	add CPatch
		04		28jan08	support Unicode

        crossfade between two patches
 
*/

#if !defined(AFX_CROSSDLG_H__CBF72A40_C24E_4251_B47F_31AC94E6A83C__INCLUDED_)
#define AFX_CROSSDLG_H__CBF72A40_C24E_4251_B47F_31AC94E6A83C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CrossDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCrossDlg dialog

#include "ToolDlg.h"
#include "NumSpin.h"
#include "ClickSliderCtrl.h"
#include "NumEdit.h"
#include "Oscillator.h"
#include "Patch.h"

class CMainFrame;

class CCrossDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CCrossDlg);
// Construction
public:
	CCrossDlg(CWnd* pParent = NULL);
	BOOL	Create(UINT nIDTemplate, CMainFrame *pFrame);

// Constants
	enum {	// selection indices: shared with CMainFrame
		SEL_DOC,	// document (placeholder only, not used)
		SEL_A,		// input A
		SEL_B,		// input B
		SEL_MIX,	// output mix
		SELS		// number of selections
	};

// Types
	typedef struct tagINFO {
		double	Pos;		// crossfader position; 0 = A, 1 = B, .5= 50/50 mix
		double	Seconds;	// duration of automated crossfade, in seconds 
		bool	Loop;		// if true, automated crossfade loops repeatedly
		int		Waveform;	// oscillator waveform type for automated crossfade
	} INFO;

// Attributes
	void	GetDefaults(INFO& Info) const;
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	void	SetViewSel(int Sel);
	void	SetEditSel(int Sel);
	CParmInfo& GetInfo(int Sel);
	void	GetInfo(int Sel, CParmInfo& Info) const;
	void	SetInfo(int Sel, const CParmInfo& Info);
	void	GetName(int Sel, CString& Name) const;
	void	SetName(int Sel, LPCTSTR Name);
	bool	GetModifiedFlag(int Sel) const;
	void	SetModifiedFlag(int Sel, bool Modified);
	bool	IsPlaying() const;
	void	Play(bool Enable);
	bool	IsLooped() const;
	void	Loop(bool Enable);
	double	GetPos() const;
	void	SetPos(double Pos);
	void	SetSeconds(double Seconds);
	void	SetWaveform(int Waveform);
	void	SetTimerFreq(double Freq);

// Operations
	void	RestoreDefaults();
	void	TimerHook();
	bool	SaveCheck();
	bool	SaveCheck(int Sel);
	void	OnParamUpdate(int Sel);
	void	RandomPatch();
	bool	Open(int Sel, LPCTSTR Path);
	bool	Open(int Sel);
	bool	Save(int Sel);
	bool	FadeTo(const CPatch& Patch, LPCTSTR Name, double Secs, bool RandPhase);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrossDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CCrossDlg)
	enum { IDD = IDD_CROSS };
	CComboBox	m_NameB;
	CComboBox	m_NameA;
	CNumSpin	m_SecsSpin;
	CNumSpin	m_PosSpin;
	CButton	m_EditMix;
	CButton	m_MixView;
	CButton	m_EditB;
	CButton	m_EditA;
	CComboBox	m_Waveform;
	CButton	m_PlayBtn;
	CButton	m_StopBtn;
	CButton	m_LoopBtn;
	CClickSliderCtrl	m_PosSlider;
	CNumEdit	m_PosEdit;
	CNumEdit	m_SecsEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CCrossDlg)
	afx_msg void OnChangedSecsEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedPosEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpenA();
	afx_msg void OnSaveA();
	afx_msg void OnOpenB();
	afx_msg void OnSaveB();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnLoop();
	afx_msg void OnSelchangeWaveform();
	afx_msg void OnMixView();
	afx_msg void OnEditA();
	afx_msg void OnEditB();
	afx_msg void OnMixSave();
	afx_msg void OnEditMix();
	afx_msg void OnSelchangeNameA();
	afx_msg void OnSelchangeNameB();
	afx_msg void OnSetfocusNameA();
	afx_msg void OnSetfocusNameB();
	//}}AFX_MSG
	afx_msg LRESULT	OnSetText(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		ROWS = CParmInfo::ROWS,		// number of parameter rows
		WAVEFORMS = COscillator::WAVEFORMS,	// number of oscillator waveforms
		SLIDER_RANGE = 100,			// range of position slider
		DEFAULT_SECS = 10,			// default crossfade duration in seconds
		DEFAULT_WAVEFORM = COscillator::TRIANGLE,	// default control waveform
		UWM_SETTEXT = WM_APP + 1000	// wParam: window handle, lParam: LPCTSTR text
	};

// Data members
	CMainFrame	*m_Frm;			// pointer to main frame window
	CParmInfo	m_Info[SELS];	// array of parameter info containers
	bool	m_Modified[SELS];	// for each parameter info, true if it was modified
	COscillator	m_Osc;			// waveform generator for automated crossfade
	CString	m_MixName;			// name to display when an input is a mix
	CString	m_ComboText;		// SelectCombo posts a UWM_SETTEXT pointing to this
	bool	m_Play;				// true if automated crossfade is in progress
	bool	m_Loop;				// true if automated crossfade should loop
	double	m_MinSecs;			// minimum seconds: reciprocal of timer's nyquist freq
	double	m_PrevPos;			// previous position, for detecting direction change
	int		m_PrevDir;			// previous direction; 0 = L, 1 = R, -1 = invalid
	bool	m_OneShot;			// true during first tick of non-looped automation

// Helpers
	void	UpdateMix(double Pos);
	double	Interpolate(double a, double b, double Pos);
	double	Randomize(double a, double b);
	double	SecsToFreq(double Seconds);
	void	OnEditButton(int Sel, BOOL Check);
	void	PopulateCombo(int Sel);
	void	SelectCombo(int Sel);
};

inline CParmInfo& CCrossDlg::GetInfo(int Sel)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	return(m_Info[Sel]);
}

inline void CCrossDlg::GetInfo(int Sel, CParmInfo& Info) const
{
	ASSERT(Sel >= 0 && Sel < SELS);
	Info = m_Info[Sel];
}

inline bool CCrossDlg::GetModifiedFlag(int Sel) const
{
	ASSERT(Sel >= 0 && Sel < SELS);
	return(m_Modified[Sel]);
}

inline void CCrossDlg::SetModifiedFlag(int Sel, bool Modified)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	m_Modified[Sel] = Modified;
}

inline bool CCrossDlg::IsPlaying() const
{
	return(m_Play);
}

inline bool CCrossDlg::IsLooped() const
{
	return(m_Loop);
}

inline double CCrossDlg::GetPos() const
{
	return(m_PosEdit.GetVal());
}

inline double CCrossDlg::Interpolate(double a, double b, double Pos)
{
	return(a + Pos * (b - a));
}

inline double CCrossDlg::Randomize(double a, double b)
{
	return(a + double(rand()) / RAND_MAX * (b - a));
}

inline double CCrossDlg::SecsToFreq(double Seconds)
{
	return(Seconds ? .5 / Seconds : 0);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CROSSDLG_H__CBF72A40_C24E_4251_B47F_31AC94E6A83C__INCLUDED_)
