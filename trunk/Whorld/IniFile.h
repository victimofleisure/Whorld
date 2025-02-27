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
		02		19feb22	refactor to fully emulate profile methods
		03		16feb23	add Unicode string methods
 
		INI file wrapper

*/

#pragma once

#include "ArrayEx.h"

class CIniFile : public WObject {
public:
// Construction
	CIniFile();
	CIniFile(LPCTSTR lpszFilePath, bool bWrite = false);

// Operations
	void	Read();
	void	Write();
	CString	GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	bool	GetStringEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString& sValue);
	void	WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString sValue = _T(""));
	int		GetInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault = 0);
	void	WriteInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue = 0);
	bool	GetBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, void *pData, UINT& nBytes);
	void	WriteBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, const void *pData, UINT nBytes);
	void	GetUnicodeString(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString& sValue);
	void	WriteUnicodeString(LPCTSTR lpszSection, LPCTSTR lpszEntry, CString sValue = _T(""));
	static	bool	IsPrintableASCII(LPCTSTR pszIn);

// Templates for default section
	template<class T>
	inline void Get(LPCTSTR lpszKey, T& value)
	{
		DWORD	nSize = sizeof(T);
		GetBinary(REG_SETTINGS, lpszKey, &value, &nSize);
		ASSERT(nSize == sizeof(T));	// ensure all bytes were read
	}
	template<class T>	
	inline void Put(LPCTSTR lpszKey, const T& value)
	{
		WriteBinary(REG_SETTINGS, lpszKey, &value, sizeof(T));
	}
	#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
		inline void Get(LPCTSTR lpszKey, T& value)	\
		{	\
			value = static_cast<T>(prefix GetInt(REG_SETTINGS, lpszKey, value));	\
		}
	#include "RegTemplTypes.h"	// specialize Get for numeric types
	#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
		inline void Put(LPCTSTR lpszKey, const T& value)	\
		{	\
			WriteInt(REG_SETTINGS, lpszKey, value);	\
		}
	#include "RegTemplTypes.h"	// specialize Put for numeric types
	inline void Get(LPCTSTR lpszKey, CString& sValue)
	{
		sValue = GetString(REG_SETTINGS, lpszKey, sValue);
	}
	inline void Put(LPCTSTR lpszKey, const CString& sValue)
	{
		WriteString(REG_SETTINGS, lpszKey, sValue);
	}

// Templates for specified section
	template<class T>
	inline void Get(LPCTSTR lpszSection, LPCTSTR lpszKey, T& value)
	{
		UINT	nSize = sizeof(T);
		GetBinary(lpszSection, lpszKey, &value, nSize);
		ASSERT(nSize == sizeof(T));	// ensure all bytes were read
	}
	template<class T>	
	inline void Put(LPCTSTR lpszSection, LPCTSTR lpszKey, const T& value)
	{
		WriteBinary(lpszSection, lpszKey, &value, sizeof(T));
	}
	#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
		inline void Get(LPCTSTR lpszSection, LPCTSTR lpszKey, T& value)	\
		{	\
			value = static_cast<T>(prefix GetInt(lpszSection, lpszKey, value));	\
		}
	#include "RegTemplTypes.h"	// specialize Get for numeric types
	#define	WINAPPCK_REG_TYPE_DEF(T, prefix) \
		inline void Put(LPCTSTR lpszSection, LPCTSTR lpszKey, const T& value)	\
		{	\
			WriteInt(lpszSection, lpszKey, value);	\
		}
	#include "RegTemplTypes.h"	// specialize Put for numeric types
	inline void Get(LPCTSTR lpszSection, LPCTSTR lpszKey, CString& sValue)
	{
		sValue = GetString(lpszSection, lpszKey, sValue);
	}
	inline void Put(LPCTSTR lpszSection, LPCTSTR lpszKey, const CString& sValue)
	{
		WriteString(lpszSection, lpszKey, sValue);
	}

// Array templates
	template<class TYPE, class ARG_TYPE>
	bool GetArray(CArrayEx<TYPE, ARG_TYPE>& arr, LPCTSTR lpszSection, LPCTSTR lpszEntry)
	{
		CString	str;
		if (!GetStringEx(lpszSection, lpszEntry, str))	// if value not found
			return false;	// failure
		int	nElems = str.GetLength() / (sizeof(TYPE) * 2);	// size in elements
		ASSERT(str.GetLength() % (sizeof(TYPE) * 2) == 0);	// check for partial element
		arr.SetSize(nElems);	// allocate array
		UnencodeBinary(str, arr.GetData(), nElems * sizeof(TYPE));	// size in bytes
		return true;	// success
	}
	template<class TYPE, class ARG_TYPE>
	void WriteArray(const CArrayEx<TYPE, ARG_TYPE>& arr, LPCTSTR lpszSection, LPCTSTR lpszEntry)
	{
		WriteBinary(lpszSection, lpszEntry, arr.GetData(), arr.GetSize() * sizeof(TYPE));
	}

protected:
// Types
	typedef CMap<CString, LPCTSTR, INT_PTR, INT_PTR> CStringIdxMap;
	class CKey {
	public:
		CString	m_sName;	// key name
		CString	m_sVal;		// key value
	};
	typedef CArrayEx<CKey, CKey&> CKeyArray;
	class CSection {
	public:
		CString			m_sName;	// section name
		CKeyArray		m_arrKey;	// array of keys
		CStringIdxMap	m_mapKey;	// hash map of keys
	};
	typedef CArrayEx<CSection, CSection&> CSectionArray;
	class CFastStdioFile : public CStdioFile {
	public:
		CFastStdioFile() {}
		CFastStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags) : CStdioFile(lpszFileName, nOpenFlags) {}
		virtual	BOOL ReadString(CString& rString);
	};
	
// Data members
	CSectionArray	m_arrSection;	// array of sections
	CStringIdxMap	m_mapSection;	// hash map of sections
	CFastStdioFile	m_fIni;			// file instance

// Helpers
	void	UnencodeBinary(CString str, void *pData, UINT nBytes);
	static	UINT	GetOpenFlags(bool bWrite);
};

inline bool CIniFile::IsPrintableASCII(LPCTSTR pszIn)
{
	ASSERT(pszIn != NULL);
	while (*pszIn) {	// while characters remain
		if (*pszIn < 0x20 || *pszIn > 0x7e) {	// if character isn't printable ASCII
			return false;	// failure and early out
		}
		pszIn++;	// next character
	}
	return true;	// string is printable ASCII
}
