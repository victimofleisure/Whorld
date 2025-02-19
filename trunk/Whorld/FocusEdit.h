// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		05aug06	initial version
		01		20oct07	move GetEdit into .cpp
		02		20jun21	refactor for centralized use

		handlers for focused edit control

*/

#pragma once

class CFocusEdit {
public:
	static	CEdit	*GetEdit();
	static	CEdit	*GetEdit(HWND hWnd);
	static	void	OnCmdMsg(int nID, int nCode, void *pExtra, CEdit *pEdit);

protected:
	static	bool	IsReadOnly(CEdit *ep);
	static	bool	HasSelection(CEdit *ep);
};

inline CEdit *CFocusEdit::GetEdit()
{
	return GetEdit(::GetFocus());
}

inline bool CFocusEdit::IsReadOnly(CEdit *pEdit)
{
	return((pEdit->GetStyle() & ES_READONLY) != 0);
}

inline bool CFocusEdit::HasSelection(CEdit *pEdit)
{
	int	nBeg, nEnd;
	pEdit->GetSel(nBeg, nEnd);
	return(nBeg != nEnd);
}
