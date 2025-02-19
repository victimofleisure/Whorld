// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		31jul04		initial version
		01		30nov07		handle close signal by closing main window
		02		29jan08		change SetScreenBufferSize arg type to fix warning
		03		08jun21		fix handle cast warning in redirect method
		04		27apr24		replace Redirect with standard file reopen
		05		06feb25		fix sporadic reopen failure in Release

		Create a Win32 console and redirect standard I/O to it

*/

#include "stdafx.h"
#include "Win32Console.h"

bool Win32Console::m_IsOpen;

BOOL WINAPI Win32Console::SignalHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT) {
		// don't use GetMainWnd because we're in a worker thread
		CWinApp	*pApp = AfxGetApp();
		if (pApp != NULL) {
			if (pApp->m_pMainWnd != NULL) {
				pApp->m_pMainWnd->PostMessage(WM_CLOSE);	// close main window
			}
		}
		return true;
	}
	return false;
}

bool Win32Console::Create()
{
	if (m_IsOpen)
		return true;
	if (!AllocConsole())
		return false;
	FILE	*pStream = NULL;
	if (freopen_s(&pStream, "CONIN$", "r", stdin))
		return false;
	if (freopen_s(&pStream, "CONOUT$", "w", stderr))
		return false;
	if (freopen_s(&pStream, "CONOUT$", "w", stdout))
		return false;
	// for reliable results, also need the following (GitHub allocterminal.cpp)
	HANDLE	hStdOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hStdOut == INVALID_HANDLE_VALUE)
		return false;
	if (!SetStdHandle(STD_OUTPUT_HANDLE, hStdOut))
		return false;
	if (!SetStdHandle(STD_ERROR_HANDLE, hStdOut))
		return false;
	HANDLE	hStdIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hStdIn == INVALID_HANDLE_VALUE)
		return false;
	if (!SetStdHandle(STD_INPUT_HANDLE, hStdIn))
		return false;
	if (!SetConsoleCtrlHandler(SignalHandler, true))
		return false;
	m_IsOpen = TRUE;
	return true;
}

bool Win32Console::SetScreenBufferSize(WORD Cols, WORD Rows)
{
	HANDLE	hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hCon == INVALID_HANDLE_VALUE)
		return false;
	COORD	dwSize;
	dwSize.X = Cols;
	dwSize.Y = Rows;
	if (!SetConsoleScreenBufferSize(hCon, dwSize))
		return false;
	return true;
}
