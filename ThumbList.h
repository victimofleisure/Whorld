// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03jun06	initial version
		01		28jan08	support Unicode

        thumbnail manager
 
*/

#ifndef CTHUMBLIST_INCLUDED
#define CTHUMBLIST_INCLUDED

class CThumbList : public WObject
{
public:
// Construction
	CThumbList();

// Attributes
	bool	IsCreated() const;
	int		GetCount() const;
	CSize	GetThumbSize() const;
	CImageList	*GetImageList();

// Operations
	bool	Create(CSize ThmSize);
	int		Add(LPCTSTR Path);
	bool	Add(const CStringArray& PathList, CDWordArray& ThmIdxList);
	void	RemoveAll();

private:
// Constants
	enum {
		VIEW_TICKS	= 400,	// number of timer ticks per drawing
		VIEW_WIDTH	= 320,	// width of drawing area, in pixels
		VIEW_HEIGHT	= 240	// height of drawing area, in pixels
	};

// Member data
	CImageList	m_ThmList;		// thumbnail bitmaps
	CStringArray	m_SrcPath;	// source file paths, in thumbnail order
	CSize	m_ThmSize;			// thumbnail size

// Helpers
	static	bool	IsVideo(LPCTSTR Path);
	static	bool	CreateThumb(CDC& ThumbDC, CBitmap& Thumb, CSize sz);
	static	bool	DrawThumb(LPCTSTR Path, CDC& ThumbDC, CBitmap& Thumb, CSize sz);
	int		Find(LPCTSTR Path) const;
};

inline bool CThumbList::IsCreated() const
{
	return(m_ThmList.GetSafeHandle() != NULL);
}

inline int CThumbList::GetCount() const
{
	return(m_SrcPath.GetSize());
}

inline CSize CThumbList::GetThumbSize() const
{
	return(m_ThmSize);
}

inline CImageList *CThumbList::GetImageList()
{
	return(&m_ThmList);
}

#endif
