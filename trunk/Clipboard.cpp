// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
		01		10jan05	add ownership
		02		23oct05	in ctor, don't zero window handle
		03		23nov07	support Unicode

		generic read/write interface for windows clipboard
 
*/

#include "stdafx.h"
#include "Clipboard.h"

const DWORD	CClipboard::HDRSIZE = sizeof(DWORD) + sizeof(GUID);

CClipboard::CClipboard(HWND hWnd, LPCTSTR Format) :
	m_hWnd(hWnd)
{
	SetFormat(Format);
	memset(&m_Owner, 0, sizeof(GUID));
}

CClipboard::~CClipboard()
{
}

void CClipboard::SetFormat(LPCTSTR Format)
{
	m_Format = (Format != NULL ? RegisterClipboardFormat(Format) : 0);
}

void CClipboard::SetOwner(LPGUID Owner)
{
	memcpy(&m_Owner, Owner, sizeof(GUID));
}

bool CClipboard::Write(const LPVOID Data, DWORD Length) const
{
	bool	retc = FALSE;
	if (m_Format) {
		if (OpenClipboard(m_hWnd)) {
			if (EmptyClipboard()) {
				HGLOBAL gmem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
					HDRSIZE + Length);
				if (gmem != NULL) {
					BYTE	*p = (BYTE *)GlobalLock(gmem);
					if (p != NULL) {
						*((DWORD *)p) = Length;
						CopyMemory(p + sizeof(DWORD), &m_Owner, sizeof(GUID));
						CopyMemory(p + HDRSIZE, Data, Length);
						GlobalUnlock(p);
						if (SetClipboardData(m_Format, gmem) != NULL)
							retc = TRUE;
					} else
						GlobalFree(gmem);
				}
			}
		}
	}
	CloseClipboard();
	return(retc);
}

LPVOID CClipboard::Read(DWORD& Length, LPGUID Owner) const
{
	const int HDRSIZE = sizeof(DWORD) + sizeof(GUID);
	LPVOID	Data = NULL;
	Length = 0;
	if (m_Format) {
		if (OpenClipboard(m_hWnd)) {
			HANDLE gmem = GetClipboardData(m_Format);
			if (gmem != NULL) {
				BYTE	*p = (BYTE *)GlobalLock(gmem);
				if (p != NULL) {
					Length = *((DWORD *)p);
					Data = new BYTE[Length];
					if (Owner != NULL)
						CopyMemory(Owner, p + sizeof(DWORD), sizeof(GUID));
					CopyMemory(Data, p + HDRSIZE, Length);
					GlobalUnlock(p);
				}
			}
		}
	}
	CloseClipboard();
	return(Data);
}

bool CClipboard::HasData() const
{
	return(IsClipboardFormatAvailable(m_Format) != 0);
}
