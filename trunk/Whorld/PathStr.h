// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jul05	initial version
		01		23nov07	support Unicode
        02      22feb25	replace header guard with pragma
		03		03mar25	modernize style

		wrap useful shell path functions
 
*/

#pragma once

#include "shlwapi.h"

class CPathStr : public CString {
public:
	CPathStr();
	CPathStr(const CString& sSrc);
	CPathStr(LPCTSTR pszSrc);
	BOOL	Append(LPCTSTR pszMore);
	void	RemoveExtension();
	BOOL	RemoveFileSpec();
	BOOL	RenameExtension(LPCTSTR pszExt);
	void	QuoteSpaces();
};

inline CPathStr::CPathStr()
{
}

inline CPathStr::CPathStr(const CString& sSrc) : CString(sSrc)
{
}

inline CPathStr::CPathStr(LPCTSTR pszSrc) : CString(pszSrc)
{
}
