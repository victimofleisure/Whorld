// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		anonytmouse

		rev		date	comments
		00		09sep05	initial version

		wrapper for shell function SHMessageBoxCheck

*/

int MessageBoxCheck(HWND hwnd, LPCTSTR pszText, LPCTSTR pszTitle, UINT uType, int iDefault, LPCTSTR pszRegVal);
int MessageBoxCheck(HWND hwnd, int nTextID, int nTitleID, UINT uType, int iDefault, LPCTSTR pszRegVal);
