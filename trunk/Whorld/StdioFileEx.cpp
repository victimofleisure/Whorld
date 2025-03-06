// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		06mar25	initial version

*/

#include "stdafx.h"
#include "StdioFileEx.h"

CStdioFileEx::CStdioFileEx(LPCTSTR lpszFileName, UINT nOpenFlags, bool bUTF8)
{
	if (bUTF8) {	// if UTF-8 character encoding is enabled
		static const LPCTSTR arrModeStr[] = {
			_T("rt,ccs=UTF-8"),	// read text mode
			_T("wt,ccs=UTF-8"),	// write text mode
		};
		FILE	*fStream;
		errno_t	nErr =_tfopen_s(&fStream, lpszFileName, arrModeStr[(nOpenFlags & modeWrite) != 0]);
		if (nErr != 0) {	// if open error
			AfxThrowFileException(CFileException::OsErrorToException(nErr), nErr, lpszFileName);
		}
		CommonBaseInit(fStream, NULL);
		m_bCloseOnDelete = true;
	} else {	// default behavior
		CommonInit(lpszFileName, nOpenFlags, NULL);
	}
}

CStdioLogFile::CStdioLogFile(LPCTSTR lpszFileName)
{
	FILE	*fStream;
	// open mode is UTF-8, append to existing file, text, commit to disk
	errno_t	nErr =_tfopen_s(&fStream, lpszFileName, _T("atc,ccs=UTF-8"));
	if (nErr != 0) {	// if open error
		AfxThrowFileException(CFileException::OsErrorToException(nErr), nErr, lpszFileName);
	}
	CommonBaseInit(fStream, NULL);
	m_bCloseOnDelete = true;
}
