// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01aug05	initial version

        disable and restore accessibility shortcuts
 
*/

#include "stdafx.h"
#include "NoAccessKeys.h"

CNoAccessKeys::CNoAccessKeys()
{
	ZeroMemory(&m_StickyKeys, sizeof(STICKYKEYS));
	ZeroMemory(&m_FilterKeys, sizeof(FILTERKEYS));
	ZeroMemory(&m_ToggleKeys, sizeof(TOGGLEKEYS));
	STICKYKEYS	sk;
	sk.cbSize = sizeof(sk);
	if (SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(sk), &sk, 0)) {
		m_StickyKeys = sk;
		sk.dwFlags &= ~SKF_HOTKEYACTIVE;
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sk), &sk, 0);
	}
	FILTERKEYS	fk;
	fk.cbSize = sizeof(fk);
	if (SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(fk), &fk, 0)) {
		m_FilterKeys = fk;
		fk.dwFlags &= ~FKF_HOTKEYACTIVE;
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(fk), &fk, 0);
	}
	TOGGLEKEYS	tk;
	tk.cbSize = sizeof(tk);
	if (SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(tk), &tk, 0)) {
		m_ToggleKeys = tk;
		tk.dwFlags &= ~TKF_HOTKEYACTIVE;
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(tk), &tk, 0);
	}
}

CNoAccessKeys::~CNoAccessKeys()
{
	Restore();
}

void CNoAccessKeys::Restore()
{
	if (m_StickyKeys.cbSize)
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_StickyKeys, 0);
	if (m_FilterKeys.cbSize)
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &m_FilterKeys, 0);
	if (m_ToggleKeys.cbSize)
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_ToggleKeys, 0);
}
