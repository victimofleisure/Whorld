// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
        01      22nov06 derive from WObject
		02		23nov07	support Unicode

		generic read/write interface for windows clipboard
 
*/

#ifndef CCLIPBOARD_INCLUDED
#define CCLIPBOARD_INCLUDED

class CClipboard : WObject {
public:
	CClipboard(HWND hWnd, LPCTSTR Format);
	~CClipboard();
	void	SetFormat(LPCTSTR Format);
	void	SetOwner(LPGUID Owner);
	bool	Write(const LPVOID Data, DWORD Length) const;
	LPVOID	Read(DWORD& Length, LPGUID Owner = NULL) const;
	bool	HasData() const;

private:
	static	const DWORD	HDRSIZE;
	HWND	m_hWnd;
	int		m_Format;
	GUID	m_Owner;
};

#endif 
