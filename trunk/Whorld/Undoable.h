// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
        01      22nov06 derive from WObject
		02		18mar08	remove key support
		03		28may10	add CtrlID to notify and cancel
		04		01may14	widen CtrlID and Code to 32-bit
		05		25apr18	standardize names
        06      22feb25	replace header guard with pragma
		07		27feb25	update return style

        undoable edit interface
 
*/

#pragma once

#include "UndoState.h"
#include "UndoManager.h"

class CUndoable : public WObject {
public:
// Constants
	enum UE_FLAGS {
		UE_COALESCE			= 0x01,
		UE_INSIGNIFICANT	= 0x02
	};

// Construction
	CUndoable();

// Attributes
	CUndoManager	*GetUndoManager() const;
	void	SetUndoManager(CUndoManager *pMgr);
	CUndoable	*GetUndoHandler() const;
	void	SetUndoHandler(CUndoable *pHandler);
	int		GetUndoAction() const;
	bool	UndoMgrIsIdle() const;
	bool	IsUndoing() const;
	bool	IsRedoing() const;

// Operations
	void	NotifyUndoableEdit(int nCtrlID, int nCode, UINT nFlags = 0);
	void	CancelUndoableEdit(int nCtrlID, int nCode);
	void	ClearUndoHistory();

// Overridables
	virtual	void	SaveUndoState(CUndoState& State) = 0;
	virtual	void	RestoreUndoState(const CUndoState& State) = 0;
	virtual	CString	GetUndoTitle(const CUndoState& State) = 0;

private:
// Member data
	CUndoManager	*m_pUndoManager;
};

inline CUndoable::CUndoable()
{
	m_pUndoManager = NULL;
}

inline CUndoManager *CUndoable::GetUndoManager() const
{
	return m_pUndoManager;
}

inline void CUndoable::SetUndoManager(CUndoManager *pMgr)
{
	m_pUndoManager = pMgr;
}

inline void CUndoable::NotifyUndoableEdit(int nCtrlID, int nCode, UINT nFlags)
{
	m_pUndoManager->NotifyEdit(nCtrlID, nCode, nFlags);
}

inline void CUndoable::CancelUndoableEdit(int nCtrlID, int nCode)
{
	m_pUndoManager->CancelEdit(nCtrlID, nCode);
}

inline int CUndoable::GetUndoAction() const
{
	return m_pUndoManager->GetAction();
}

inline bool CUndoable::UndoMgrIsIdle() const
{
	return m_pUndoManager->IsIdle();
}

inline bool CUndoable::IsUndoing() const
{
	return m_pUndoManager->IsUndoing();
}

inline bool CUndoable::IsRedoing() const
{
	return m_pUndoManager->IsRedoing();
}

inline void CUndoable::ClearUndoHistory()
{
	m_pUndoManager->DiscardAllEdits();
}
