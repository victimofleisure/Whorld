// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09feb04	initial version
		01		23apr05	add no resize flag
		02		31jul05	add double support
		03		23nov07	support Unicode
		04		29nov08	add GetWndPlacement

		make states persistent using registry
 
*/

#ifndef CPERSIST_INCLUDED
#define	CPERSIST_INCLUDED

class CPersist {
public:
	enum OPTION {
		NO_MINIMIZE = 0x01,
		NO_MAXIMIZE = 0x02,
		NO_RESIZE	= 0x04
	};
	static	UINT	GetInt(LPCTSTR Section, LPCTSTR Entry, int Default);
	static	BOOL	WriteInt(LPCTSTR Section, LPCTSTR Entry, int Value);
	static	CString	GetString(LPCTSTR Section, LPCTSTR Entry, LPCTSTR Default = NULL);
	static	BOOL	WriteString(LPCTSTR Section, LPCTSTR Entry, LPCTSTR Value);
	static	BOOL	GetWndPlacement(LPCTSTR Section, LPCTSTR Name, WINDOWPLACEMENT& wp);
	static	BOOL	WriteWndPlacement(LPCTSTR Section, LPCTSTR Name, const WINDOWPLACEMENT& wp);
	static	void	SaveWnd(LPCTSTR Section, const CWnd *Wnd, LPCTSTR Name);
	static	int		LoadWnd(LPCTSTR Section, CWnd *Wnd, LPCTSTR Name, int Options = 0);
	static	int		GetWndShow(LPCTSTR Section, LPCTSTR Name);
	static	BOOL	GetBinary(LPCTSTR Section, LPCTSTR Entry, LPVOID Buffer, LPDWORD Size);
	static	BOOL	WriteBinary(LPCTSTR Section, LPCTSTR Entry, LPCVOID Buffer, DWORD Size);
	static	BOOL	GetFont(LPCTSTR Section, LPCTSTR Entry, CFont *Font);
	static	BOOL	WriteFont(LPCTSTR Section, LPCTSTR Entry, CFont *Font);
	static	float	GetFloat(LPCTSTR Section, LPCTSTR Entry, float Default);
	static	BOOL	WriteFloat(LPCTSTR Section, LPCTSTR Entry, float Value);
	static	double	GetDouble(LPCTSTR Section, LPCTSTR Entry, double Default);
	static	BOOL	WriteDouble(LPCTSTR Section, LPCTSTR Entry, double Value);

private:
	static	LPCTSTR WND_PLACE;
};

inline UINT	CPersist::GetInt(LPCTSTR Section, LPCTSTR Entry, int Default)
{
	return(AfxGetApp()->GetProfileInt(Section, Entry, Default));
}

inline BOOL	CPersist::WriteInt(LPCTSTR Section, LPCTSTR Entry, int Value)
{
	return(AfxGetApp()->WriteProfileInt(Section, Entry, Value));
}

inline CString CPersist::GetString(LPCTSTR Section, LPCTSTR Entry, LPCTSTR Default)
{
	return(AfxGetApp()->GetProfileString(Section, Entry, Default));
}

inline BOOL	CPersist::WriteString(LPCTSTR Section, LPCTSTR Entry, LPCTSTR Value)
{
	return(AfxGetApp()->WriteProfileString(Section, Entry, Value));
}

#endif
