// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17oct05	initial version
		01		28jan08	support Unicode
		02		29jan08	in Set, add static cast to fix warning

        read/write keyboard LEDs
 
*/

#include "stdafx.h"
#include "KeyboardLeds.h"
#include <winioctl.h>

#define IOCTL_KEYBOARD_SET_INDICATORS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)

CKeyboardLeds::CKeyboardLeds()
{
	m_Kbd = INVALID_HANDLE_VALUE;
}

CKeyboardLeds::~CKeyboardLeds()
{
	Close();
}

bool CKeyboardLeds::Open(bool Overlapped)
{
	Close();
	// see Mark McGinty's "Manipulating the Keyboard Lights in Windows NT"
	if (!DefineDosDevice(DDD_RAW_TARGET_PATH, _T("Kbd"), _T("\\Device\\KeyboardClass0")))
		return(FALSE);
	m_Kbd = CreateFile(_T("\\\\.\\Kbd"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 
		Overlapped ? FILE_FLAG_OVERLAPPED : 0, NULL);
	return(m_Kbd != INVALID_HANDLE_VALUE);
}

bool CKeyboardLeds::Close()
{
	if (!IsOpen())
		return(FALSE);
	if (!DefineDosDevice(DDD_REMOVE_DEFINITION, _T("Kbd"), NULL))
		return(FALSE);
	if (!CloseHandle(m_Kbd))
		return(FALSE);
	m_Kbd = INVALID_HANDLE_VALUE;	// mark handle closed
	return(TRUE);
}

bool CKeyboardLeds::Set(UINT mask)
{
	if (!IsOpen())
		return(FALSE);
	KEYBOARD_INDICATOR_PARAMETERS	Input;
	Input.UnitId = 0;
	Input.LedFlags = static_cast<USHORT>(mask);
	ULONG	InSize = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
	ULONG	RetBytes;
	ZeroMemory(&m_Ovl, sizeof(m_Ovl));	// don't care when I/O finishes
	bool	retc = DeviceIoControl(m_Kbd, IOCTL_KEYBOARD_SET_INDICATORS,
		&Input, InSize, NULL, NULL, &RetBytes, &m_Ovl) != 0;
	return(retc);
}

bool CKeyboardLeds::Get(UINT& mask)
{
	if (!IsOpen())
		return(FALSE);
	KEYBOARD_INDICATOR_PARAMETERS	Output;
	ULONG	OutSize = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
	ULONG	RetBytes;
	bool	retc = DeviceIoControl(m_Kbd, IOCTL_KEYBOARD_QUERY_INDICATORS,
		NULL, NULL, &Output, OutSize, &RetBytes, NULL) != 0;
	if (retc)
		mask = Output.LedFlags;
	return(retc);
}
