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
        02      25nov06 use CArrayishList instead of CList
		03		23nov07	support Unicode
		04		18mar08	remove key support
		05		03feb09	add GetSize, GetPos, GetState
		06		28may10	support insignificant edits
		07		05dec12	add UndoNoRedo
		08		11feb13	add OnUpdateTitles and SetPos
		09		01may14	widen nCtrlID and nCode to 32-bit
		10		25apr18	standardize names
		11		19apr20	return const string reference from title accessors
		12		05nov20	overload cancel edit

        undoable edit interface
 
*/

#ifndef CUNDOMANAGER_INCLUDED
#define CUNDOMANAGER_INCLUDED

#include "ArrayEx.h"
#include "UndoState.h"

class CUndoable;

class CUndoManager : public WObject {
public:
// Construction
	CUndoManager(CUndoable *pRoot = NULL);
	virtual	~CUndoManager();

// Constants
	enum {	// undo actions
		UA_NONE,
		UA_UNDO,
		UA_REDO,
	};

// Attributes
	bool	CanUndo() const;
	bool	CanRedo() const;
	int		GetAction() const;
	bool	IsIdle() const;
	bool	IsUndoing() const;
	bool	IsRedoing() const;
	bool	IsModified() const;
	void	ResetModifiedFlag();
	const CString&	GetUndoTitle();
	const CString&	GetRedoTitle();
	int		GetLevels() const;
	void	SetLevels(int nLevels);
	void	SetRoot(CUndoable *pRoot);
	int		GetSize() const;
	int		GetPos() const;
	void	SetPos(int iPos);
	const	CUndoState& GetState(int iPos) const;
	CUndoState& GetState(int iPos);

// Operations
	void	Undo();
	void	Redo();
	void	UndoNoRedo();
	void	NotifyEdit(int nCtrlID, int nCode, UINT nFlags = 0);
	bool	CancelEdit(int nCtrlID, int nCode);
	bool	CancelEdit();
	void	DiscardAllEdits();

protected:
// Overridables
	virtual	void	OnModify(bool bModified);
	virtual	void	OnUpdateTitles();

private:
// Types
	typedef	CArrayEx<CUndoState, CUndoState&> CUndoStateArray;

// Constants
	enum {
		BLOCK_SIZE = 100	// list grows in blocks of this many elements
	};

// Member data
	CUndoable	*m_pRoot;	// owner of this undo stack
	CUndoStateArray	m_arrState;	// undo stack; array of undo states
	bool	m_bCanUndo;		// true if edit can be undone
	bool	m_bCanRedo;		// true if edit can be redone
	int		m_iPos;			// current position in undo stack
	int		m_nLevels;		// number of undo levels, or -1 for unlimited
	int		m_nEdits;		// total number of edits made so far
	int		m_nAction;		// undo action; see enum above
	CString	m_sUndoTitle;	// current undo title for edit menu
	CString	m_sRedoTitle;	// current redo title for edit menu

// Helpers
	void	SwapState(int iPos);
	void	DumpState(LPCTSTR pszTag, int iPos);
	CString	GetTitle(int iPos);
	void	UpdateTitles();
	int		FindUndoable() const;
	int		FindRedoable() const;
};

inline bool CUndoManager::CanUndo() const
{
	return(m_bCanUndo);
}

inline bool CUndoManager::CanRedo() const
{
	return(m_bCanRedo);
}

inline int CUndoManager::GetAction() const
{
	return(m_nAction);
}

inline bool CUndoManager::IsIdle() const
{
	return(m_nAction == UA_NONE);
}

inline bool CUndoManager::IsUndoing() const
{
	return(m_nAction == UA_UNDO);
}

inline bool CUndoManager::IsRedoing() const
{
	return(m_nAction == UA_REDO);
}

inline bool CUndoManager::IsModified() const
{
	return(m_nEdits > 0);
}

inline void CUndoManager::ResetModifiedFlag()
{
	m_nEdits = 0;
}

inline int CUndoManager::GetLevels() const
{
	return(m_nLevels);
}

inline const CString& CUndoManager::GetUndoTitle()
{
	return(m_sUndoTitle);
}

inline const CString& CUndoManager::GetRedoTitle()
{
	return(m_sRedoTitle);
}

inline void CUndoManager::SetRoot(CUndoable *pRoot)
{
	m_pRoot = pRoot;
}

inline int CUndoManager::GetSize() const
{
	return(m_arrState.GetSize());
}

inline int CUndoManager::GetPos() const
{
	return(m_iPos);
}

inline const CUndoState& CUndoManager::GetState(int iPos) const
{
	return(m_arrState[iPos]);
}

inline CUndoState& CUndoManager::GetState(int iPos)
{
	return(m_arrState[iPos]);
}

#endif
