// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version

		random sequence without duplicates
 
*/

#ifndef CRANDLIST_DEFINED
#define CRANDLIST_DEFINED

class CRandList : public WObject {
public:
	CRandList();
	CRandList(int Size);
	static	int		Rand(int Limit);
	void	Init(int Size);
	int		GetNext();
	int		GetSize() const;

private:
	CDWordArray	m_List;	// array of randomly generated elements
	int		m_Avail;	// number of elements that haven't been used
};

inline int CRandList::GetSize() const
{
	return(m_List.GetSize());
}

#endif
