// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		24apr18	standardize names

        spin buddy for numeric edit control
 
*/

#pragma once

// NumSpin.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumSpin window

class CNumSpin : public CSpinButtonCtrl
{
	DECLARE_DYNAMIC(CNumSpin);
// Construction
public:
	CNumSpin();

// Attributes
public:
	void	SetDelta(double fDelta);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumSpin)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNumSpin();

// Generated message map functions
protected:
	//{{AFX_MSG(CNumSpin)
	afx_msg void OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	double	m_fDelta;	// increment numeric edit control by this amount
};

inline void CNumSpin::SetDelta(double fDelta)
{
	m_fDelta = fDelta;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
