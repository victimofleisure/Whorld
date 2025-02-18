// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct05	initial version
		01		23nov07	support Unicode

        numeric edit control
 
*/

#if !defined(AFX_NOTEEDIT_H__E71F7E37_244A_4AD5_BDA8_926990EBB817__INCLUDED_)
#define AFX_NOTEEDIT_H__E71F7E37_244A_4AD5_BDA8_926990EBB817__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NoteEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNoteEdit window

#include "NumEdit.h"

class CNoteEdit : public CNumEdit
{
	DECLARE_DYNAMIC(CNoteEdit);
// Construction
public:
	CNoteEdit();

// Attributes
public:
	void	SetNoteEntry(bool Enable);
	bool	IsNoteEntry() const;

// Operations
public:
	static	void	NoteToStr(int Note, CString& Str);
	static	int		StrToNote(LPCTSTR Str);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNoteEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNoteEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNoteEdit)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Overrides
	void	StrToVal(LPCTSTR Str);
	void	ValToStr(CString& Str);
	bool	IsValidChar(int Char);

// Data members
	bool	m_IsNoteEntry;
};

inline bool CNoteEdit::IsNoteEntry() const
{
	return(m_IsNoteEntry);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOTEEDIT_H__E71F7E37_244A_4AD5_BDA8_926990EBB817__INCLUDED_)
