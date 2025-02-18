// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04aug00 initial version
		01		13dec02	add get depth
		02		16jan03	add get relative path
		03		23nov07	support Unicode
 
        find files in subfolders
 
*/

#ifndef CSUBFILEFIND_INCLUDED
#define CSUBFILEFIND_INCLUDED

class CSubFileFind : public WObject {
public:
	CSubFileFind();
	~CSubFileFind();
	BOOL	FindFile(LPCTSTR path);
	BOOL	FindNextFile(CFileFind*& cffp);
	const	CFileFind& GetFind();
	CString	GetRoot() const;
	int		GetDepth() const;
	CString	GetRelativePath() const;
private:
	enum {
		MAXDEPTH = 100
	};
	CFileFind	*cff[MAXDEPTH];
	BOOL	more[MAXDEPTH];
	int		depth;
};

inline int CSubFileFind::GetDepth() const
{
	return(depth);
}

#endif
