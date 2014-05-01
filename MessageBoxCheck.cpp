// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		anonytmouse

		rev		date	comments
		00		09sep05	initial version
		01		23nov07	support Unicode

		wrapper for shell function SHMessageBoxCheck

*/

#include "stdafx.h"
#include "MessageBoxCheck.h"

// NOTE: the registry key specified by pszRegVal is created in the following location:
// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\DontShowMeThisDialogAgain
// To avoid collisions with values used by Microsoft, this string should include a GUID.

int MessageBoxCheck(HWND hwnd, LPCTSTR pszText, LPCTSTR pszTitle, UINT uType, int iDefault, LPCTSTR pszRegVal)
{
	typedef (WINAPI *PSHMBC)(HWND, LPCTSTR, LPCTSTR, UINT, int, LPCTSTR);
	HMODULE	hShlwapi = NULL;
	PSHMBC	shMessageBoxCheck = NULL;
	int		retValue = -1;
	// SHMessageBoxCheck is a Settlement function: the Final Judgment in United
	// States v. Microsoft required the evil empire to document it in 2004, but
	// they don't export it by name, so ordinals were provided by a usenet post:
	// http://groups-beta.google.com/group/microsoft.public.platformsdk.shell/msg/d638e49f001752b4
	WORD	ordinalSHMBC = (sizeof(TCHAR) == sizeof(WCHAR) ? 191 : 185);
	hShlwapi = LoadLibrary(TEXT("shlwapi.dll"));
	if (hShlwapi) {
		shMessageBoxCheck = (PSHMBC)GetProcAddress(hShlwapi, (LPCSTR)ordinalSHMBC);
		if (shMessageBoxCheck) {
			// For some reason SHMessageBoxCheck seems to require that the thread has
			// a message queue to work correctly. So we call PeekMessage to force the
			// creation of a message queue.
			MSG msg;
			PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
			retValue = shMessageBoxCheck(hwnd, pszText, pszTitle, uType, iDefault, pszRegVal);
		}
		FreeLibrary(hShlwapi);
	}
	return retValue;
}

int MessageBoxCheck(HWND hwnd, int nTextID, int nTitleID, UINT uType, int iDefault, LPCTSTR pszRegVal)
{
	CString	Text((LPCTSTR)nTextID);
	CString	Title((LPCTSTR)nTitleID);
	return(MessageBoxCheck(hwnd, Text, Title, uType, iDefault, pszRegVal));
}
