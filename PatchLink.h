// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20oct05	initial version
        01      17apr06	rename from CPatch
        02      03jun06	add image index
		03		26jun06	add serialization
		04		28jan08	support Unicode

		link to patch file
 
*/

#ifndef CPATCHLINK_INCLUDED
#define CPATCHLINK_INCLUDED

#include <afxtempl.h>

class CPatchLink : public CObject {
public:
	DECLARE_SERIAL(CPatchLink);
	CPatchLink() {}
	CPatchLink(int Bank, DWORD HotKey, LPCTSTR Path, int ImgIdx);
	CPatchLink(const CPatchLink& Src);
	CPatchLink& operator=(const CPatchLink& Src);
	void	Serialize(CArchive& ar);

	int		m_Bank;
	DWORD	m_HotKey;
	CString	m_Path;
	int		m_ImgIdx;

private:
	void	Copy(const CPatchLink& Src);
};

template<> inline void AFXAPI SerializeElements<CPatchLink>(CArchive& ar, CPatchLink *pObj, int nCount)
{	// specialize to prevent collections from bitwise copying CString
	for (int i = 0; i < nCount; i++, pObj++)
		pObj->Serialize(ar);
}

#endif
