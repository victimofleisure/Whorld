// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda
 
		revision history:
		rev		date	comments
		00		12sep13	initial version
		01		24feb20	implement read/write
		02		08jun21	define ATL string length methods if earlier than VS2012
		03		19feb22	refactor to fully emulate profile methods
		04		16feb23	add Unicode string methods
 
		INI file wrapper

*/

#include "stdafx.h"
#include "IniFile.h"

CIniFile::CIniFile()
{
}

CIniFile::CIniFile(LPCTSTR pszFilePath, bool bWrite) : m_fIni(pszFilePath, GetOpenFlags(bWrite))
{
}

inline UINT CIniFile::GetOpenFlags(bool bWrite)
{
	if (bWrite)	// if writing
		return CFile::modeCreate | CFile::modeWrite;
	else	// reading
		return CFile::modeRead;
}

#if _MSC_VER < 1700	// if earlier than Visual Studio 2012
inline int AtlStrLen(_In_opt_z_ const wchar_t *str)
{
	if (str == NULL)
		return 0;
	return static_cast<int>(::wcslen(str));
}
inline int AtlStrLen(_In_opt_z_ const char *str)
{
	if (str == NULL)
		return 0;
	return static_cast<int>(::strlen(str));
}
#endif

BOOL CIniFile::CFastStdioFile::ReadString(CString& rString)
{
	// same as the base class implementation except that when the string 
	// is long enough to require multiple reads, the buffer size increases
	// exponentially, greatly improving performance for huge strings
	ASSERT_VALID(this);

	rString = _T("");    // empty string without deallocating
	const int nMaxSize = 128;
	LPTSTR lpsz = rString.GetBuffer(nMaxSize);
	LPTSTR lpszResult;
	int nLen = 0;
	for (;;)
	{
		// the following line works because CString::GetBuffer increases the
		// buffer size exponentially; see PrepareWrite2 in atlsimpstr.h
		int	nBufAvail = rString.GetAllocLength() - nLen;	// performance gain
		lpszResult = _fgetts(lpsz, nBufAvail + 1, m_pStream);
		rString.ReleaseBuffer();

		// handle error/eof case
		if (lpszResult == NULL && !feof(m_pStream))
		{
			Afx_clearerr_s(m_pStream);
			AfxThrowFileException(CFileException::genericException, _doserrno,
				m_strFileName);
		}

		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = AtlStrLen(lpsz)) < nMaxSize ||
			lpsz[nLen - 1] == '\n')
			break;

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen - 1] == '\n')
		rString.GetBufferSetLength(nLen - 1);

	return nLen != 0;
}

void CIniFile::Read()
{
	m_arrSection.RemoveAll();
	m_mapSection.RemoveAll();
	CString	sLine, sSection;
	while (m_fIni.ReadString(sLine)) {
		if (sLine[0] == '[') {	// if start of section
			int	iEnd = sLine.Find(']', 1);
			if (iEnd >= 0) {	// if section name terminator found
				sSection = sLine.Mid(1, iEnd - 1);
			}
		} else {	// not section
			int	iSep = sLine.Find('=');
			if (iSep >= 0) {	// if key separator found
				LPTSTR	pszLine = const_cast<LPTSTR>(LPCTSTR(sLine));
				pszLine[iSep] = '\0';	// faster than Left and Mid
				WriteString(sSection, pszLine, &pszLine[iSep + 1]);
			}
		}
	}
}

void CIniFile::Write()
{
	INT_PTR	nSections = m_arrSection.GetSize();
	for (INT_PTR iSection = 0; iSection < nSections; iSection++) {	// for each section
		const CSection&	section = m_arrSection[iSection];
		m_fIni.WriteString('[' + section.m_sName + _T("]\n"));
		INT_PTR	nKeys = section.m_arrKey.GetSize();
		for (INT_PTR iKey = 0; iKey < nKeys; iKey++) {	// for each of section's keys
			const CKey&		key = section.m_arrKey[iKey];
			m_fIni.WriteString(key.m_sName + '=' + key.m_sVal + '\n');
		}
	}
}

CString	CIniFile::GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	INT_PTR	iSection;
	if (m_mapSection.Lookup(lpszSection, iSection)) {	// if section mapped
		CSection&	section = m_arrSection[iSection];
		INT_PTR	iKey;
		if (section.m_mapKey.Lookup(lpszEntry, iKey)) {	// if key mapped
			return section.m_arrKey[iKey].m_sVal;
		}
	}
	return lpszDefault;
}

bool CIniFile::GetStringEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString& sValue)
{
	// same as GetString except returns whether value was found
	INT_PTR	iSection;
	if (m_mapSection.Lookup(lpszSection, iSection)) {	// if section mapped
		CSection&	section = m_arrSection[iSection];
		INT_PTR	iKey;
		if (section.m_mapKey.Lookup(lpszEntry, iKey)) {	// if key mapped
			sValue = section.m_arrKey[iKey].m_sVal;
			return true;	// value found
		}
	}
	return false;	// value not found
}

void CIniFile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString sValue)
{
	INT_PTR	iSection;
	if (!m_mapSection.Lookup(lpszSection, iSection)) {	// if section not mapped
		iSection = m_arrSection.GetSize();	// append new section
		m_arrSection.SetSize(iSection + 1);	// grow section array
		CSection&	section = m_arrSection[iSection];
		section.m_sName = lpszSection;	// set section name
		m_mapSection.SetAt(section.m_sName, iSection);	// map section
	}
	CSection&	section = m_arrSection[iSection];	// got section
	INT_PTR	iKey;
	if (!section.m_mapKey.Lookup(lpszEntry, iKey)) {	// if key not mapped
		iKey = section.m_arrKey.GetSize();	// append new key
		section.m_arrKey.SetSize(iKey + 1);	// grow key array
	}
	CKey&	key = section.m_arrKey[iKey];	// got key
	key.m_sName = lpszEntry;	// set key entry
	key.m_sVal = sValue;	// set key value
	section.m_mapKey.SetAt(key.m_sName, iKey);	// map key
}

int CIniFile::GetInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
	int	nValue = nDefault;
	CString	sValue(GetString(lpszSection, lpszEntry));
	_stscanf_s(sValue, _T("%d"), &nValue);	// convert string to integer
	return nValue;
}

void CIniFile::WriteInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	TCHAR szVal[16];
	_stprintf_s(szVal, _countof(szVal), _T("%d"), nValue);	// convert integer to string
	WriteString(lpszSection, lpszEntry, szVal);
}

bool CIniFile::GetBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, void *pData, UINT& nBytes)
{
	CString	str;
	if (!GetStringEx(lpszSection, lpszEntry, str)) {	// if value not found
		nBytes = 0;
		return false;	// failure
	}
	UINT	nLen = str.GetLength();	// two characters per data byte
	ASSERT((nLen & 1) == 0);	// string length should be even
	nLen = min(nLen / 2, nBytes);	// conversion length in bytes
	nBytes = nLen;	// return number of bytes converted to caller
	LPCTSTR	pStr = str;
	BYTE	*pOut = static_cast<BYTE *>(pData);
	for (UINT i = 0; i < nLen; i++) {	// for each output data byte
		pOut[i] = static_cast<BYTE>(((pStr[i * 2 + 1] - 'A') << 4) + (pStr[i * 2] - 'A'));
	}
	return true;	// success
}

void CIniFile::UnencodeBinary(CString str, void *pData, UINT nBytes)
{
	// same as GetBinary except we get string value from caller instead of looking it up
	UINT	nLen = str.GetLength();	// two characters per data byte
	ASSERT((nLen & 1) == 0);	// string length should be even
	nLen = min(nLen / 2, nBytes);	// conversion length in bytes
	LPCTSTR	pStr = str;
	BYTE	*pOut = static_cast<BYTE *>(pData);
	for (UINT i = 0; i < nLen; i++) {	// for each output data byte
		pOut[i] = static_cast<BYTE>(((pStr[i * 2 + 1] - 'A') << 4) + (pStr[i * 2] - 'A'));
	}
}

void CIniFile::WriteBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, const void *pData, UINT nBytes)
{
	CString	str;
	LPTSTR	pStr = str.GetBuffer(nBytes * 2);	// two characters per data byte
	const BYTE	*pBuf = static_cast<const BYTE *>(pData);
	for (UINT i = 0; i < nBytes; i++) {	// for each input data byte
		pStr[i * 2] = (TCHAR)((pBuf[i] & 0x0F) + 'A');	// low nibble
		pStr[i * 2 + 1] = (TCHAR)(((pBuf[i] >> 4) & 0x0F) + 'A');	// high nibble
	}
	str.ReleaseBuffer(nBytes * 2);
	WriteString(lpszSection, lpszEntry, str);
}

void CIniFile::GetUnicodeString(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString& sValue)
{
#ifdef UNICODE
	if (!GetStringEx(lpszSection, lpszEntry, sValue)) {	// if regular string read fails
		CString	sEntry(lpszEntry);	// try Unicode string read
		sEntry += '#';
		CString	str;
		if (GetStringEx(lpszSection, sEntry, str)) {	// if value found
			UINT	nOutBytes = str.GetLength() / 2;	// output length in bytes
			UINT	nOutChars = nOutBytes / sizeof(TCHAR);	// output length in characters
			BYTE	*pOutBuf = reinterpret_cast<BYTE *>(sValue.GetBuffer(nOutChars));
			UnencodeBinary(str, pOutBuf, nOutBytes);
			sValue.ReleaseBuffer(nOutChars);
		}
	}
#else	// not UNICODE
	GetStringEx(lpszSection, lpszEntry, sValue);
#endif
}

void CIniFile::WriteUnicodeString(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString sValue)
{
#ifdef UNICODE
	if (IsPrintableASCII(sValue)) {	// if string is printable ASCII
		WriteString(lpszSection, lpszEntry, sValue);	// do regular string write
	} else {	// string isn't printable ASCII
		CString	sEntry(lpszEntry);
		sEntry += '#';	// tag entry name and write string as binary
		WriteBinary(lpszSection, sEntry, sValue, sValue.GetLength() * sizeof(TCHAR));
	}
#else	// not UNICODE
	WriteString(lpszSection, lpszEntry, sValue);
#endif
}
