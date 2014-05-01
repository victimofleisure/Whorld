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

// NoteEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "NoteEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNoteEdit

IMPLEMENT_DYNAMIC(CNoteEdit, CNumEdit);

CNoteEdit::CNoteEdit()
{
	m_IsNoteEntry = TRUE;
}

CNoteEdit::~CNoteEdit()
{
}

void CNoteEdit::NoteToStr(int Note, CString& Str)
{
	static	const	LPCTSTR NoteName[] = 
		{_T("C"), _T("C#"), _T("D"), _T("D#"), _T("E"), _T("F"), 
		_T("F#"), _T("G"), _T("G#"), _T("A"), _T("A#"), _T("B")};
	Str.Format(_T("%s%d"), NoteName[Note % 12], Note / 12);
}

int CNoteEdit::StrToNote(LPCTSTR Str)
{
	static	const	int	NoteVal[] = {9, 11, 0, 2, 4, 5, 7};
	if (isdigit(Str[0]))
		return(_ttoi(Str));	// assume Str is MIDI note number
	int	Note = toupper(Str[0]) - 'A';	// assume Str[0] is note letter
	Note = NoteVal[CLAMP(Note, 0, 6)];
	int	OctIdx = 2;	// assume Str + 2 is octave number
	switch (tolower(Str[1])) {	// assume Str[1] is accidental
	case '#':
		Note++;
		break;
	case 'b':
		Note--;
		break;
	default:
		OctIdx--;	// no accidental: assume Str + 1 is octave number
	}
	return(_ttoi(Str + OctIdx) * 12 + Note);
}

void CNoteEdit::StrToVal(LPCTSTR Str)
{
	if (m_IsNoteEntry)
		m_Val = StrToNote(Str);
	else
		CNumEdit::StrToVal(Str);
}

void CNoteEdit::ValToStr(CString& Str)
{
	if (m_IsNoteEntry)
		NoteToStr((int)m_Val, Str);
	else
		CNumEdit::ValToStr(Str);
}

bool CNoteEdit::IsValidChar(int Char)
{
	if (m_IsNoteEntry)
		return(TRUE);
	else
		return(CNumEdit::IsValidChar(Char));
}

void CNoteEdit::SetNoteEntry(bool Enable)
{
	m_IsNoteEntry = Enable;
	SetText();
}

BEGIN_MESSAGE_MAP(CNoteEdit, CNumEdit)
	//{{AFX_MSG_MAP(CNoteEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoteEdit message handlers
