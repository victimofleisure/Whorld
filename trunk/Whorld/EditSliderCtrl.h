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
        02      22feb25	replace header guard with pragma
		03		03mar25	modernize style

		slider with buddy numeric edit control
 
*/

#pragma once

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
		int		nRangeMin;		// range lower limit
		int		nRangeMax;		// range upper limit
		float	fLogBase;		// if non-zero, position is exponent of this base
		float	fSliderScale;	// slider position is scaled by this factor
		int		nDefaultPos;	// default position, in slider coordinates
		int		nTicCount;		// if non-zero, number of tics, evenly spaced
		float	fEditScale;		// edit control's value is scaled by this factor
		int		nEditPrecision;	// edit control's digits to right of decimal point
	} INFO;

// Attributes
public:
	void	SetVal(double fVal);
	double	GetVal() const;
	void	SetValNorm(double fVal);
	double	GetValNorm() const;
	void	SetScale(double fScale);
	void	SetfLogBase(double fBase);
	void	SetEditCtrl(CNumEdit *pEdit);
	CNumEdit	*GetEditCtrl() const;
	void	SetInfo(const INFO& info, CNumEdit *pEdit);
	double	GetDefaultVal() const;

// Operations
public:
	static	double	Norm(const INFO& info, double pVal);
	static	double	Denorm(const INFO& info, double pVal);

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
	CNumEdit	*m_pEdit;	// pointer to numeric edit control; may be null
	double	m_fVal;			// slider's value, post scaling and optional log
	double	m_fScale;		// slider's scale; slider pos is scaled by this
	double	m_fLogBase;		// slider's log base; if zero, slider is linear

// Helpers
	virtual	double	Norm(double x) const;
	virtual	double	Denorm(double x) const;
};

inline double CEditSliderCtrl::GetVal() const
{
	return m_fVal;
}

inline void CEditSliderCtrl::SetScale(double fScale)
{
	m_fScale = fScale;
}

inline void CEditSliderCtrl::SetfLogBase(double fBase)
{
	m_fLogBase = fBase;
}

inline CNumEdit *CEditSliderCtrl::GetEditCtrl() const
{
	return m_pEdit;
}
