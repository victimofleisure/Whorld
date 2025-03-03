// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		22apr05	remove undo handling, use doubles
		02		28jan08	support Unicode
		03		29jan08	in SetFormat, add static cast to fix warning
		04		19sep13	add spin control format
		05		19apr18	move spin control creation to helper
		06		24apr18	standardize names
		07		28apr18	make AddSpin virtual
		08		07jun21	rename rounding functions
		09		14dec22	add fraction format

        numeric edit control
 
*/

#pragma once

// NumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumEdit window

#define NEN_CHANGED	2000	// custom notification

class CNumSpin;

class CNumEdit : public CEdit
{
	DECLARE_DYNAMIC(CNumEdit);
// Construction
public:
	CNumEdit();

// Constants
	enum {	// notification target flags
		NTF_PARENT	= 0x01,
		NTF_AUX		= 0x02,
		NTF_NONE	= 0,
		NTF_ALL		= -1
	};
	enum {	// data formats
		DF_REAL		= 0x00,
		DF_INT		= 0x01,
		DF_SPIN		= 0x02,
		DF_FRACTION	= 0x04,
	};

// Attributes
public:
	void	SetVal(double fVal);
	void	SetVal(double fVal, int nNotifyMask);
	double	GetVal() const;
	int		GetIntVal() const;
	void	SetScale(double fScale);
	void	SetLogBase(double fBase);
	void	SetPrecision(int nPrecision);
	void	SetAuxNotify(CWnd *pWnd);
	void	SetRange(double fMinVal, double fMaxVal);
	void	SetFormat(int nType);
	void	SetFractionScale(int nScale);

// Operations
public:
	virtual	void	AddSpin(double fDelta);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNumEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNumEdit)
	afx_msg BOOL OnKillfocus();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	double	m_fVal;			// our current value
	double	m_fScale;		// display value multiplied by this
	double	m_fLogBase;		// if non-zero, display base raised to value's power
	int		m_nPrecision;	// if non-zero, number of decimal places to display
	CWnd	*m_pAuxNotify;	// send notifications to this window as well as parent
	double	m_fMinVal;		// value's lower limit
	double	m_fMaxVal;		// value's upper limit
	bool	m_bHaveRange;	// true if value should be limited
	BYTE	m_nFormat;		// value's data format; see enum above
	int		m_nFracScale;	// fraction scaling factor
	CNumSpin	*m_pSpin;	// associated spin control if any

// Helpers
	virtual	void	StrToVal(LPCTSTR Str);
	virtual	void	ValToStr(CString& Str);
	virtual	bool	IsValidChar(int nChar);
	void	SetText();
	void	GetText();
	void	Notify(int nNotifyMask = NTF_ALL);
	void	CreateSpinCtrl();
};

inline double CNumEdit::GetVal() const
{
	return m_fVal;
}

inline int CNumEdit::GetIntVal() const
{
	return Round(m_fVal);
}

inline void CNumEdit::SetScale(double fScale)
{
	m_fScale = fScale;
}

inline void CNumEdit::SetLogBase(double fBase)
{
	m_fLogBase = fBase;
}

inline void CNumEdit::SetPrecision(int nPrecision)
{
	m_nPrecision = nPrecision;
}

inline void CNumEdit::SetAuxNotify(CWnd *pWnd)
{
	m_pAuxNotify = pWnd;
}

inline void CNumEdit::SetFormat(int Format)
{
	m_nFormat = static_cast<BYTE>(Format);
}

inline void CNumEdit::SetFractionScale(int nScale)
{
	m_nFracScale = nScale;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
