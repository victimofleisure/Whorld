// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		22nov06	add support for double and int64
		02		26nov06	make undo key DWORD, max keys = 2
		03		18dec06	add support for pair of shorts
		04		03jan08	replace CSmartBuf with CRefPtr
		05		18mar08	make CRefPtr a template
		06		28may10	add IsInsignificant
		07		01may14	widen CtrlID and Code to 32-bit
 		08		09sep14	use default memberwise copy
		09		24mar15	upgrade old-school struct definitions
		10		30mar18	overload constructor
		11		09apr18	add value accessors
		12		10apr18	remove pack pragma to fix CRefString alignment bug
		13		25apr18	standardize names
		14		19feb25	add point float
        15      22feb25 replace header guard with pragma
		16		27feb25	update return style

        undo state container
 
*/

#pragma once

#include "RefPtr.h"

class CUndoManager;

// create accessors for an undo state value member
#define CUNDOSTATE_VAL(alias, type, member)						\
	inline static const type& alias(const CUndoState& State)	\
		{ return State.m_Val.member; }							\
	inline static type& alias(CUndoState& State)				\
		{ return State.m_Val.member; }

// reserved control ID for insignificant edits
#define UNDO_CTRL_ID_INSIGNIFICANT INT_MAX

class CUndoState : public CRefPtr<CRefObj> {
public:
// Types
	typedef union tagELEM {
		int		i;
		UINT	u;
		float	f;
		bool	b;
		struct {
			WORD	lo;
			WORD	hi;
		} w;
		struct {
			short	lo;
			short	hi;
		} s;
		struct {
			BYTE	al;
			BYTE	ah;
			BYTE	bl;
			BYTE	bh;
		} c;
	} ELEM;
	struct PAIR {
		ELEM	x;
		ELEM	y;
	};
	union VALUE {
		PAIR	p;
		double	d;
		__int64	i64;
		POINTFLOAT	pt;
	};
	class CRefString : public CRefObj {
	public:
		CString m_str; 
	};

// Member data
	VALUE	m_Val;

// Value accessors
	#define UNDOTYPEDEF(type, undoval) \
		void GetVal(type& val) const { val = m_Val.undoval; } \
		void SetVal(const type& val) { m_Val.undoval = val; }
	#include "UndoTypes.h"	// generate accessors for listed types
	void		GetVal(CString& val) const;
	void		SetVal(const CString& val);

// Attributes
	int		GetCtrlID() const;
	int		GetCode() const;
	bool	IsMatch(int nCtrlID, int nCode) const;
	bool	IsSignificant() const;

// Construction
	CUndoState();
	CUndoState(int nCtrlID, int nCode);

// Operations
	CString	DumpState() const;
	void	Empty();

private:
// Member data
	int		m_nCtrlID;	// ID of notifying control; UNDO_CTRL_ID_INSIGNIFICANT
						// is reserved for flagging insignificant edits
	int		m_nCode;	// edit function code

// Helpers
	friend CUndoManager;
};

inline CUndoState::CUndoState()
{
}

inline CUndoState::CUndoState(int nCtrlID, int nCode)
{
	m_nCtrlID = nCtrlID;
	m_nCode = nCode;
}

inline int CUndoState::GetCtrlID() const
{
	return m_nCtrlID;
}

inline int CUndoState::GetCode() const
{
	return m_nCode;
}

inline bool CUndoState::IsMatch(int nCtrlID, int nCode) const
{
	return nCtrlID == m_nCtrlID && nCode == m_nCode;
}

inline bool CUndoState::IsSignificant() const
{
	return m_nCtrlID != UNDO_CTRL_ID_INSIGNIFICANT;
}

inline void CUndoState::Empty()
{
	CRefPtr<CRefObj>::SetEmpty();
	memset(&m_Val, 0, sizeof(VALUE));
}
