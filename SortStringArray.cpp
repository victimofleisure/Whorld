// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jul05 initial version
		01		28jan08	support Unicode
 
		sortable string array
 
*/

#include "stdafx.h"
#include "SortStringArray.h"

void CSortStringArray::Sort()
{
	qsort(m_pData, m_nSize, sizeof(CString *), compare);
}

int CSortStringArray::compare(const void *arg1, const void *arg2)
{
	return(_tcscmp(*(LPCTSTR *)arg1, *(LPCTSTR *)arg2));
}


