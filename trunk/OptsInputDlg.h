// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06jul05	initial version

        color options property page
 
*/

#if !defined(AFX_OPTSINPUTDLG_H__8C04F63F_1BFB_4B90_9575_007075E28466__INCLUDED_)
#define AFX_OPTSINPUTDLG_H__8C04F63F_1BFB_4B90_9575_007075E28466__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsInputDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsInputDlg dialog

#include "EditSliderCtrl.h"
#include "PersistVal.h"

class COptsInputDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(COptsInputDlg)
// Construction
public:
	COptsInputDlg();

// Attributes
	double	GetMouseSens() const;
	double	GetWheelSens() const;
	double	GetTempoNudge() const;

// Operations
	void	SetDefaults();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsInputDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsInputDlg)
	enum { IDD = IDD_OPTS_INPUT };
	CEditSliderCtrl	m_WheelSensSlider;
	CNumEdit	m_WheelSensEdit;
	CEditSliderCtrl	m_TempoNudgeSlider;
	CNumEdit	m_TempoNudgeEdit;
	CEditSliderCtrl	m_MouseSensSlider;
	CNumEdit	m_MouseSensEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsInputDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSTATE {
		double	MouseSens;	// mouse sensitivity; bigger = more sensitive, 1 = normal
		double	WheelSens;	// wheel sensitivity; bigger = more sensitive, 1 = normal
		double	TempoNudge;	// tempo up/down amount, as a fraction of current tempo
	} STATE;

// Constants
	enum {
		MOUSE_SENS,
		WHEEL_SENS,
		TEMPO_NUDGE,
		SLIDERS
	};
	static	const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];
	static	const STATE	m_Default;

// Member data
	STATE	m_st;
	PersistVal<double>	m_RegMouseSens;
	PersistVal<double>	m_RegWheelSens;
	PersistVal<double>	m_RegTempoNudge;
};

inline double COptsInputDlg::GetMouseSens() const
{
	return(m_st.MouseSens);
}

inline double COptsInputDlg::GetWheelSens() const
{
	return(m_st.WheelSens);
}

inline double COptsInputDlg::GetTempoNudge() const
{
	return(m_st.TempoNudge);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSINPUTDLG_H__8C04F63F_1BFB_4B90_9575_007075E28466__INCLUDED_)
