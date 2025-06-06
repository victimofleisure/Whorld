// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version
        01      21nov11	add GetAvail
        02      01jul12	rename Rand argument
        03      06jan13	add cast to GetSize for x64
		04		03mar25	modernize style

		random sequence without duplicates
 
*/

#ifndef CRANDLIST_DEFINED
#define CRANDLIST_DEFINED

class CRandList : public WObject {
public:
	CRandList();
	CRandList(int nSize);
	static	int		Rand(int nVals);
	void	Init(int nSize);
	int		GetNext();
	int		GetSize() const;
	int		GetAvail() const;

protected:
	CDWordArray	m_List;	// array of randomly generated elements
	int		m_nAvail;	// number of elements that haven't been used
};

inline int CRandList::GetSize() const
{
	return static_cast<int>(m_List.GetSize());	// cast to 32-bit
}

inline int CRandList::GetAvail() const
{
	return m_nAvail;
}

#endif
