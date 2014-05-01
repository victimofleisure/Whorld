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

#include "stdafx.h"
#include "PatchLink.h"

IMPLEMENT_SERIAL(CPatchLink, CObject, 1)

CPatchLink::CPatchLink(int Bank, DWORD HotKey, LPCTSTR Path, int ImgIdx)
{
	m_Bank = Bank;
	m_HotKey = HotKey;
	m_Path = Path;
	m_ImgIdx = ImgIdx;
}

CPatchLink::CPatchLink(const CPatchLink& Src)
{
	Copy(Src);
}

CPatchLink& CPatchLink::operator=(const CPatchLink& Src)
{
	Copy(Src);
	return(*this);
}

void CPatchLink::Copy(const CPatchLink& Src)
{
	m_Bank = Src.m_Bank;
	m_HotKey = Src.m_HotKey;
	m_Path = Src.m_Path;
	m_ImgIdx = Src.m_ImgIdx;
}

void CPatchLink::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Bank;
		ar << m_HotKey;
		ar << m_Path;
		ar << m_ImgIdx;
	} else {
		ar >> m_Bank;
		ar >> m_HotKey;
		ar >> m_Path;
		ar >> m_ImgIdx;
	}
}
