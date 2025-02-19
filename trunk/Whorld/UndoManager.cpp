// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		07mar04	add limit
		02		08mar04	in SwapState, set busy before finding undoable
		03		08mar04	in NotifyEdit, ctor isn't called when array shrinks
		04		12mar04	if coalescing, remove states above current position
		05		29sep04	cancel edit must update titles
		06		19mar05	bump m_nEdits regardless of number of undo levels
        07      22nov06 rename strings to start with IDS_
        08      25nov06 use CArrayishList instead of CList
		09		23nov07	support Unicode
		10		03jan08	replace CSmartBuf with CRefPtr
		11		18mar08	remove key support
		12		19feb09	in NotifyEdit, fix stack size limit case
		13		28may10	support insignificant edits
		14		05dec12	add UndoNoRedo
		15		11feb13	add OnUpdateTitles and SetPos
		16		01may14	widen CtrlID and Code to 32-bit
		17		07may15	in DumpState, use address format for object pointer
		18		23jun15	in SwapState, save to temporary in case restore throws
		19		25apr18	standardize names
		20		19apr20	remove unused resource header
		21		05nov20	overload cancel edit
		22		08jun21	fix warning for CString as variadic argument

        undoable edit interface
 
*/

#include "stdafx.h"
#include "UndoManager.h"
#include "Undoable.h"

#if UNDO_NATTER
#define	UNDO_DUMP_STATE(Tag, Pos) DumpState(Tag, Pos);
#else
#define	UNDO_DUMP_STATE(Tag, Pos)
#endif

CUndoManager::CUndoManager(CUndoable *pRoot) :
	m_pRoot(pRoot)
{
	m_bCanUndo = FALSE;
	m_bCanRedo = FALSE;
	m_iPos = 0;
	m_nLevels = INT_MAX;
	m_nEdits = 0;
	m_nAction = UA_NONE;
}

CUndoManager::~CUndoManager()
{
}

inline CString CUndoManager::GetTitle(int iPos)
{
	return(m_pRoot->GetUndoTitle(m_arrState[iPos]));
}

int CUndoManager::FindUndoable() const
{
	int	iPos = m_iPos;
	while (iPos > 0) {
		iPos--;
		if (m_arrState[iPos].IsSignificant())
			return(iPos);
	}
	return(-1);
}

int CUndoManager::FindRedoable() const
{
	int	iPos = m_iPos;
	while (iPos < GetSize()) {
		if (m_arrState[iPos].IsSignificant())
			return(iPos);
		iPos++;
	}
	return(-1);
}

void CUndoManager::Undo()
{
	int	iPrevUndo = FindUndoable();
	if (iPrevUndo >= 0) {
		// undo previous significant edit, and all insignificant edits back to it
		m_nAction = UA_UNDO;
		for (int iState = m_iPos - 1; iState >= iPrevUndo; iState--) {
			UNDO_DUMP_STATE(_T("Undo"), iState);
			SwapState(iState);
			if (IsIdle())
				return;	// edits discarded
		}
		m_iPos = iPrevUndo;
		UpdateTitles();
		m_nAction = UA_NONE;
	}
}

void CUndoManager::Redo()
{
	int	iNextRedo = FindRedoable();
	if (iNextRedo >= 0) {
		// redo next significant edit, and all insignificant edits up to it
		m_nAction = UA_REDO;
		for (int iState = m_iPos; iState <= iNextRedo; iState++) {
			UNDO_DUMP_STATE(_T("Redo"), iState);
			SwapState(iState);
			if (IsIdle())
				return;	// edits discarded
		}
		m_iPos = iNextRedo + 1;
		UpdateTitles();
		m_nAction = UA_NONE;
	}
}

void CUndoManager::UndoNoRedo()
{
	Undo();
	m_arrState.SetSize(m_iPos);
	UpdateTitles();
}

void CUndoManager::SwapState(int iPos)
{
	ASSERT(iPos >= 0 && iPos < GetSize());
	CUndoState	PrevState = m_arrState[iPos];
	CUndoable	*uap = m_pRoot;
	CUndoState	NewState(PrevState);	// initialize temporary
	uap->SaveUndoState(NewState);	// save to temporary in case restore throws
	uap->RestoreUndoState(PrevState);
	m_arrState[iPos] = NewState;
#if UNDO_NATTER
	if (uap == NULL)
		_tprintf(_T("Can't find instance.\n"));
#endif
}

void CUndoManager::NotifyEdit(int nCtrlID, int nCode, UINT nFlags)
{
	ASSERT(nCtrlID != UNDO_CTRL_ID_INSIGNIFICANT);	// reserved control ID
	if (IsIdle()) {
		// do insignificance test first; can modify nCtrlID
		if (nFlags & CUndoable::UE_INSIGNIFICANT)
			nCtrlID = UNDO_CTRL_ID_INSIGNIFICANT;
		else {
			if (!m_nEdits++)			// if first modification
				OnModify(TRUE);		// call derived handler
		}
		// if coalesce requested and notifier's key matches top of stack
		if ((nFlags & CUndoable::UE_COALESCE) && m_iPos 
		&& m_arrState[m_iPos - 1].IsMatch(nCtrlID, nCode)) {
			if (GetSize() > m_iPos) {	// don't resize array needlessly
				m_arrState.SetSize(m_iPos);	// remove states above current position
				UpdateTitles();
			}
			return;
		}
		if (m_nLevels <= 0)
			return;
		if (m_iPos >= m_nLevels) {	// if stack size at limit
			m_arrState.RemoveAt(0);	// remove bottom state
			m_iPos--;
		}
		m_arrState.SetSize(m_iPos);	// array shrinks if we've undone
		CUndoState	us;
		us.m_Val.i64 = 0;
		us.m_nCtrlID = nCtrlID;
		us.m_nCode = nCode;
		m_arrState.Add(us);
		CUndoState	*pState = &m_arrState[m_iPos];
		m_pRoot->SaveUndoState(*pState);
		UNDO_DUMP_STATE(_T("Notify"), m_iPos);
		m_iPos++;
		UpdateTitles();
	} else {
#if UNDO_NATTER
		_tprintf(_T("Ignoring notify.\n"));
#endif
	}
}

bool CUndoManager::CancelEdit(int nCtrlID, int nCode)
{
#if UNDO_NATTER
	_tprintf(_T("CancelEdit CtrlID=%d Code=%d\n"), nCtrlID, nCode);
#endif
	int	iState;
	for (iState = m_iPos - 1; iState >= 0; iState--) {
		if (m_arrState[iState].IsMatch(nCtrlID, nCode))
			break;
	}
	if (iState >= 0) {
		m_arrState.RemoveAt(iState);
		m_iPos--;
		if (!--m_nEdits)			// if last modification
			OnModify(FALSE);	// call derived handler
		UpdateTitles();
	}
#if UNDO_NATTER
	if (iState < 0)
		_tprintf(_T("Can't cancel edit.\n"));
#endif
	return(iState >= 0);
}

bool CUndoManager::CancelEdit()
{
	if (m_iPos > 0 && m_iPos <= GetSize()) {
		const CUndoState&	state = m_arrState[m_iPos - 1];
		return(CancelEdit(state.m_nCtrlID, state.m_nCode));	// cancel last edit
	}
	return(false);
}

void CUndoManager::DiscardAllEdits()
{
#if UNDO_NATTER
	_tprintf(_T("DiscardAllEdits\n"));
#endif
	m_arrState.SetSize(0);
	m_bCanUndo = FALSE;
	m_bCanRedo = FALSE;
	m_iPos = 0;
	m_nEdits = 0;
	m_nAction = UA_NONE;
	OnModify(FALSE);	// call derived handler
	UpdateTitles();
}

void CUndoManager::DumpState(LPCTSTR pszTag, int iPos)
{
	_tprintf(_T("%s '%s' Pos=%d %s Obj=%p\n"), pszTag, GetTitle(iPos).GetString(), iPos, 
		m_arrState[iPos].DumpState().GetString(), m_arrState[iPos].GetObj());
}

void CUndoManager::UpdateTitles()
{
	int	iPrevUndo = FindUndoable();
	m_bCanUndo = iPrevUndo >= 0;
	if (m_bCanUndo)
		m_sUndoTitle = GetTitle(iPrevUndo);
	else
		m_sUndoTitle.Empty();
	int	iNextRedo = FindRedoable();
	m_bCanRedo = iNextRedo >= 0;
	if (m_bCanRedo)
		m_sRedoTitle = GetTitle(iNextRedo);
	else
		m_sRedoTitle.Empty();
	OnUpdateTitles();
}

void CUndoManager::SetLevels(int Levels)
{
	if (Levels < 0)
		Levels = INT_MAX;
	if (Levels < GetSize()) {	// if shrinking history
		if (m_iPos < Levels)	// if undo depth is below new size
			DiscardAllEdits();	// history can't be saved
		else {	// trim excess history from the bottom
			int	Excess = GetSize() - Levels;
			m_arrState.RemoveAt(0, Excess);
			m_iPos = max(m_iPos - Excess, 0);
		}
	}
	m_nLevels = Levels;
}

void CUndoManager::SetPos(int iPos)
{
	ASSERT(iPos >= 0 && iPos <= GetSize());
	int	iStart = m_iPos;
	if (iPos == iStart)
		return;	// nothing to do
	int	nSteps = abs(iPos - iStart);
	bool	undoing = iPos < iStart;
	for (int iStep = 0; iStep < nSteps; iStep++) {
		if (undoing)
			Undo();
		else
			Redo();
	}
}

void CUndoManager::OnModify(bool bModified)
{
	UNREFERENCED_PARAMETER(bModified);
}

void CUndoManager::OnUpdateTitles()
{
}
