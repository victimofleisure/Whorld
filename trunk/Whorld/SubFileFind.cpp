// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04aug00 initial version
 		01		16jan03	add get relative path, use backslash
		02		09jan04	fix nasty memory leak; dtor must delete from 0..depth
		03		23nov07	support Unicode

        find files in subfolders
 
*/

#include "StdAfx.h"
#include "SubFileFind.h"

CSubFileFind::CSubFileFind()
{
	depth = 0;
	cff[0] = new CFileFind;
	more[0] = 1;
}

CSubFileFind::~CSubFileFind()
{
	for (int i = 0; i <= depth; i++)
		delete cff[i];
}

BOOL CSubFileFind::FindNextFile(CFileFind*& cffp)
{
	if (more[depth]) {
		while (1) {
			more[depth] = cff[depth]->FindNextFile();
			if (cff[depth]->IsDirectory() && !cff[depth]->IsDots()) {
				if (depth >= MAXDEPTH)
					return(FALSE);
				cff[++depth] = new CFileFind;
				if (!cff[depth]->FindFile(cff[depth - 1]->GetFilePath() += "\\*.*")) {
					delete cff[depth--];
					break;
				}
			} else
				break;
		}
	} else {
		if (depth)
			delete cff[depth--];
	}
	cffp = cff[depth];
	return(depth || more[depth]);
}

BOOL CSubFileFind::FindFile(LPCTSTR path)
{
	return(cff[0]->FindFile(path));
}

CString	CSubFileFind::GetRoot() const
{
	return(cff[0]->GetRoot());
}

CString	CSubFileFind::GetRelativePath() const
{
	return(cff[depth]->GetFilePath().Mid(cff[0]->GetRoot().GetLength() + 1));
}
