// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version
        01      01jul12	fix Rand to avoid possible overrun
        02      06jan13	add cast to GetSize for x64
		03		07jun21	rename rounding functions

		random sequence without duplicates
 
*/

#include "stdafx.h"
#include "RandList.h"

CRandList::CRandList()
{
	m_Avail = 0;
}

CRandList::CRandList(int Size)
{
	Init(Size);
}

void CRandList::Init(int Size)
{
	m_List.SetSize(Size);
	for (int i = 0; i < Size; i++)
		m_List[i] = i;
	m_Avail = 0;
}

int CRandList::Rand(int Vals)
{
	if (Vals <= 0)
		return(-1);
	int	i = Trunc(rand() / double(RAND_MAX) * Vals);
	return(min(i, Vals - 1));
}

int CRandList::GetNext()
{
	if (!m_Avail)
		m_Avail = GetSize();
	ASSERT(m_Avail > 0);
	int	idx = Rand(m_Avail);
	m_Avail--;
	int	tmp = m_List[idx];
	m_List[idx] = m_List[m_Avail];
	m_List[m_Avail] = tmp;
	return(tmp);
}
