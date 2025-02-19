// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		05aug06	initial version
		01		20oct07	change GetEdit to detect non-MFC edit controls
		02		17nov20	add check for property grid in-place edit
		03		20jun21	refactor for centralized use

		handlers for focused edit control

*/

#include "stdafx.h"
#include "resource.h"
#include "FocusEdit.h"

// We use the SDK GetClassName function because MFC IsKindOf fails if no CEdit
// variable has been created for the control we're trying to test. An example
// of this is the edit control created by a list control for label editing.
CEdit *CFocusEdit::GetEdit(HWND hWnd)
{
	TCHAR	szClassName[6];
	if (GetClassName(hWnd, szClassName, 6) 
	&& _tcsicmp(szClassName, _T("Edit")) == 0) {
		// This check prevents MFC property grid control's in-place edit
		// from disabling undo/redo when edit is read-only (drop list)
		if (GetDlgCtrlID(hWnd) == AFX_PROPLIST_ID_INPLACE	// if control ID matches
		&& GetWindowLong(hWnd, GWL_STYLE) & ES_READONLY) {	// and edit is read-only
			return(NULL);	// not considered an edit
		}
		return static_cast<CEdit *>(CWnd::FromHandle(hWnd));
	}
	return(NULL);
}

void CFocusEdit::OnCmdMsg(int nID, int nCode, void *pExtra, CEdit *pEdit)
{
	switch (nCode) {
	case CN_COMMAND:
		switch (nID) {
		case ID_EDIT_UNDO:
			pEdit->Undo();
			break;
		case ID_EDIT_COPY:
			pEdit->Copy();
			break;
		case ID_EDIT_CUT:
			pEdit->Cut();
			break;
		case ID_EDIT_PASTE:
			pEdit->Paste();
			break;
		case ID_EDIT_INSERT:
			break;
		case ID_EDIT_DELETE:
			if (HasSelection(pEdit))
				pEdit->Clear();
			else
				pEdit->SendMessage(WM_KEYDOWN, VK_DELETE);	// delete character at cursor
			break;
		case ID_EDIT_SELECT_ALL:
			pEdit->SetSel(0, -1);
			break;
		}
		break;
	case CN_UPDATE_COMMAND_UI:
		{
			ASSERT(pExtra != NULL);
			CCmdUI	*pCmdUI = static_cast<CCmdUI *>(pExtra);
			switch (nID) {
			case ID_EDIT_UNDO:
				pCmdUI->Enable(pEdit->CanUndo());
				break;
			case ID_EDIT_COPY:
				pCmdUI->Enable(HasSelection(pEdit));
				break;
			case ID_EDIT_CUT:
				pCmdUI->Enable(HasSelection(pEdit) && !IsReadOnly(pEdit));
				break;
			case ID_EDIT_PASTE:
				pCmdUI->Enable(!IsReadOnly(pEdit) && IsClipboardFormatAvailable(CF_TEXT));
				break;
			case ID_EDIT_INSERT:
				pCmdUI->Enable(FALSE);
				break;
			case ID_EDIT_DELETE:
				{
					bool	Enab = FALSE;
					if (!IsReadOnly(pEdit)) {
						int	nBeg, nEnd;
						pEdit->GetSel(nBeg, nEnd);
						if (nBeg != nEnd || nBeg < pEdit->LineLength())
							Enab = TRUE;	// has selection, or cursor is on a character
					}
					pCmdUI->Enable(Enab);
				}
				break;
			case ID_EDIT_SELECT_ALL:
				pCmdUI->Enable(pEdit->LineLength());
				break;
			}
			break;
		}
	}
}
