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

		wrap useful shell path functions
 
*/

#ifndef CPATHSTR_INCLUDED
#define CPATHSTR_INCLUDED

#include "shlwapi.h"

class CPathStr : public CString {
public:
	CPathStr();
	CPathStr(const CString& stringSrc);
	CPathStr(LPCTSTR lpsz);
	BOOL	Append(LPCTSTR More);
	void	RemoveExtension();
	BOOL	RemoveFileSpec();
	BOOL	RenameExtension(LPCTSTR Ext);
	void	QuoteSpaces();
};

inline CPathStr::CPathStr()
{
}

inline CPathStr::CPathStr(const CString& stringSrc) : CString(stringSrc)
{
}

inline CPathStr::CPathStr(LPCTSTR lpsz) : CString(lpsz)
{
}

#endif
