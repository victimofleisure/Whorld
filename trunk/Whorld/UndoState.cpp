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
		02		23nov07	support Unicode
		03		03jan08	replace CSmartBuf with CRefPtr
		04		18mar08	make CRefPtr a template
 		05		09sep14	use default memberwise copy
		06		09apr18	add value accessors
		07		25apr18	standardize names

        undo state container
 
*/

#include "stdafx.h"
#include "UndoState.h"

CString CUndoState::DumpState() const
{
	CString	s;
	s.Format(_T("CtrlID=%d Code=%d a=0x%x b=0x%x"), 
		m_nCtrlID, m_nCode, m_Val.p.x.i, m_Val.p.y.i);
	return(s);
}

void CUndoState::GetVal(CString& val) const
{
	const CRefString	*pStr = static_cast<CRefString *>(GetObj());
	val = pStr->m_str;
}

void CUndoState::SetVal(const CString& val)
{
	CRefPtr<CRefString>	pStr;
	pStr.CreateObj();
	pStr->m_str = val;
	SetObj(pStr);
}
