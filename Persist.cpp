// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09feb04	initial version
		01		12may04	add get/write binary, font
		02		14may04	WriteBinary takes LPCVOID, not const LPVOID
		03		28sep04	check GetSafeHandle before GetLogFont or memory leaks
		04		23nov04	use WriteBinary for window placement functions
		05		03jan05	GetFloat wasn't setting size before calling GetBinary
		06		23apr05	add no resize flag
		07		31jul05	add double support
		08		23nov07	support Unicode

		make states persistent using registry
 
*/

#include "stdafx.h"
#include "Persist.h"

LPCTSTR CPersist::WND_PLACE	= _T("WP");

void CPersist::SaveWnd(LPCTSTR Section, const CWnd *Wnd, LPCTSTR Name)
{
	WINDOWPLACEMENT	wp;
	Wnd->GetWindowPlacement(&wp);
	WriteBinary(Section, CString(Name) + WND_PLACE, &wp, sizeof(WINDOWPLACEMENT));
}

int CPersist::LoadWnd(LPCTSTR Section, CWnd *Wnd, LPCTSTR Name, int Options)
{
	WINDOWPLACEMENT	wp;
	DWORD	Size = sizeof(WINDOWPLACEMENT);
	memset(&wp, 0, Size);
	CRect	CurRect;
	Wnd->GetWindowRect(CurRect);
	if (GetBinary(Section, CString(Name) + WND_PLACE, &wp, &Size)) {
		if ((wp.showCmd == SW_SHOWMINIMIZED && (Options & NO_MINIMIZE))
		|| (wp.showCmd == SW_SHOWMAXIMIZED && (Options & NO_MAXIMIZE)))
			wp.showCmd = SW_SHOWNORMAL;
		if (Options & NO_RESIZE) {
			wp.rcNormalPosition.right = wp.rcNormalPosition.left + CurRect.Width();
			wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + CurRect.Height();
		}
		Wnd->SetWindowPlacement(&wp);
	}
	return(wp.showCmd);
}

int CPersist::GetWndShow(LPCTSTR Section, LPCTSTR Name)
{
	WINDOWPLACEMENT	wp;
	DWORD	Size = sizeof(WINDOWPLACEMENT);
	memset(&wp, 0, Size);
	GetBinary(Section, CString(Name) + WND_PLACE, &wp, &Size);
	return(wp.showCmd);
}

BOOL CPersist::GetBinary(LPCTSTR Section, LPCTSTR Entry, LPVOID Buffer, LPDWORD Size)
{
	CString	KeyName;
	KeyName.Format(_T("Software\\%s\\%s\\%s"), AfxGetApp()->m_pszRegistryKey, 
		AfxGetApp()->m_pszAppName, Section);
	HKEY	key;
	LONG	retc;
	DWORD	type;
	if ((retc = RegOpenKeyEx(HKEY_CURRENT_USER, KeyName, 
		0, KEY_READ, &key)) == ERROR_SUCCESS)
		retc = RegQueryValueEx(key, Entry, 0, &type, (const LPBYTE)Buffer, Size);
	return(retc == ERROR_SUCCESS);
}

BOOL CPersist::WriteBinary(LPCTSTR Section, LPCTSTR Entry, LPCVOID Buffer, DWORD Size)
{
	CString	KeyName;
	KeyName.Format(_T("Software\\%s\\%s\\%s"), AfxGetApp()->m_pszRegistryKey, 
		AfxGetApp()->m_pszAppName, Section);
	HKEY	key;
	DWORD	disp;
	LONG	retc;
	if ((retc = RegCreateKeyEx(HKEY_CURRENT_USER, KeyName,
		0, 0, 0, KEY_ALL_ACCESS, 0, &key, &disp)) == ERROR_SUCCESS)
		retc = RegSetValueEx(key, Entry, 0, REG_BINARY, (const LPBYTE)Buffer, Size);
	return(retc == ERROR_SUCCESS);
}

BOOL CPersist::GetFont(LPCTSTR Section, LPCTSTR Entry, CFont *Font)
{
	LOGFONT	lf;
	DWORD	Size = sizeof(LOGFONT);
	if (Font != NULL && CPersist::GetBinary(Section, Entry, &lf, &Size)
		&& Size == sizeof(LOGFONT))
		return(Font->CreateFontIndirect(&lf));
	return(FALSE);
}

BOOL CPersist::WriteFont(LPCTSTR Section, LPCTSTR Entry, CFont *Font)
{
	LOGFONT	lf;
	DWORD	Size = (Font != NULL && Font->GetSafeHandle() 
		&& Font->GetLogFont(&lf) ? sizeof(LOGFONT) : 0);
	return(CPersist::WriteBinary(Section, Entry, &lf, Size));
}

float CPersist::GetFloat(LPCTSTR Section, LPCTSTR Entry, float Default)
{
	float	r;
	DWORD	Size = sizeof(float);
	if (GetBinary(Section, Entry, &r, &Size))
		return(r);
	return(Default);
}

BOOL CPersist::WriteFloat(LPCTSTR Section, LPCTSTR Entry, float Value)
{
	return(WriteBinary(Section, Entry, &Value, sizeof(float)));
}

double CPersist::GetDouble(LPCTSTR Section, LPCTSTR Entry, double Default)
{
	double	r;
	DWORD	Size = sizeof(double);
	if (GetBinary(Section, Entry, &r, &Size))
		return(r);
	return(Default);
}

BOOL CPersist::WriteDouble(LPCTSTR Section, LPCTSTR Entry, double Value)
{
	return(WriteBinary(Section, Entry, &Value, sizeof(double)));
}
