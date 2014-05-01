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

        numeric edit control
 
*/

#if !defined(AFX_NUMEDIT_H__2E70D361_CC38_42FC_868E_27A60970D10B__INCLUDED_)
#define AFX_NUMEDIT_H__2E70D361_CC38_42FC_868E_27A60970D10B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumEdit window

#define NEN_CHANGED	2000	// custom notification

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
		DF_REAL,
		DF_INT
	};

// Attributes
public:
	void	SetVal(double Val);
	void	SetVal(double Val, int NotifyMask);
	double	GetVal() const;
	int		GetIntVal() const;
	void	SetScale(double Scale);
	void	SetLogBase(double Base);
	void	SetPrecision(int Precision);
	void	SetAuxNotify(CWnd *Wnd);
	void	SetRange(double MinVal, double MaxVal);
	void	SetFormat(int Type);

// Operations
public:
	void	AddSpin(double Delta);

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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	double	m_Val;			// our current value
	double	m_Scale;		// display value multiplied by this
	double	m_LogBase;		// if non-zero, display base raised to value's power
	int		m_Precision;	// if non-zero, number of decimal places to display
	CWnd	*m_AuxNotify;	// send notifications to this window as well as parent
	double	m_MinVal;		// value's lower limit
	double	m_MaxVal;		// value's upper limit
	bool	m_HaveRange;	// true if value should be limited
	BYTE	m_Format;		// value's data format; see enum above

// Helpers
	virtual	void	StrToVal(LPCTSTR Str);
	virtual	void	ValToStr(CString& Str);
	virtual	bool	IsValidChar(int Char);
	void	SetText();
	void	GetText();
	void	Notify(int NotifyMask = NTF_ALL);
};

inline double CNumEdit::GetVal() const
{
	return(m_Val);
}

inline int CNumEdit::GetIntVal() const
{
	return(round(m_Val));
}

inline void CNumEdit::SetScale(double Scale)
{
	m_Scale = Scale;
}

inline void CNumEdit::SetLogBase(double Base)
{
	m_LogBase = Base;
}

inline void CNumEdit::SetPrecision(int Precision)
{
	m_Precision = Precision;
}

inline void CNumEdit::SetAuxNotify(CWnd *Wnd)
{
	m_AuxNotify = Wnd;
}

inline void CNumEdit::SetFormat(int Format)
{
	m_Format = static_cast<BYTE>(Format);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMEDIT_H__2E70D361_CC38_42FC_868E_27A60970D10B__INCLUDED_)
