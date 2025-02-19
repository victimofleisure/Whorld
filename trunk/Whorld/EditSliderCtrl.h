// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jul05	initial version
		01		04mar09	add static norm/denorm functions

		slider with buddy numeric edit control
 
*/

#if !defined(AFX_EDITSLIDERCTRL_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_)
#define AFX_EDITSLIDERCTRL_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditSliderCtrl window

#include "ClickSliderCtrl.h"
#include "NumEdit.h"

class CEditSliderCtrl : public CClickSliderCtrl
{
	DECLARE_DYNAMIC(CEditSliderCtrl);
// Construction
public:
	CEditSliderCtrl();

// Types
	typedef struct tagINFO {
		int		RangeMin;		// range lower limit
		int		RangeMax;		// range upper limit
		float	LogBase;		// if non-zero, position is exponent of this base
		float	SliderScale;	// slider position is scaled by this factor
		int		DefaultPos;		// default position, in slider coordinates
		int		TicCount;		// if non-zero, number of tics, evenly spaced
		float	EditScale;		// edit control's value is scaled by this factor
		int		EditPrecision;	// edit control's digits to right of decimal point
	} INFO;

// Attributes
public:
	void	SetVal(double Val);
	double	GetVal() const;
	void	SetValNorm(double Val);
	double	GetValNorm() const;
	void	SetScale(double Scale);
	void	SetLogBase(double Base);
	void	SetEditCtrl(CNumEdit *Edit);
	CNumEdit	*GetEditCtrl() const;
	void	SetInfo(const INFO& Info, CNumEdit *Edit);
	double	GetDefaultVal() const;

// Operations
public:
	static	double	Norm(const INFO& Info, double Val);
	static	double	Denorm(const INFO& Info, double Val);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditSliderCtrl)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditSliderCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CEditSliderCtrl)
	afx_msg void HScroll(UINT nSBCode, UINT nPos);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CNumEdit	*m_Edit;	// pointer to numeric edit control; may be null
	double	m_Val;			// slider's value, post scaling and optional log
	double	m_Scale;		// slider's scale; slider pos is scaled by this
	double	m_LogBase;		// slider's log base; if zero, slider is linear

// Helpers
	virtual	double	Norm(double x) const;
	virtual	double	Denorm(double x) const;
};

inline double CEditSliderCtrl::GetVal() const
{
	return(m_Val);
}

inline void CEditSliderCtrl::SetScale(double Scale)
{
	m_Scale = Scale;
}

inline void CEditSliderCtrl::SetLogBase(double Base)
{
	m_LogBase = Base;
}

inline CNumEdit *CEditSliderCtrl::GetEditCtrl() const
{
	return(m_Edit);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSLIDERCTRL_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_)
