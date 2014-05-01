// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11may05	initial version

        GDI back buffer for off-screen drawing
 
*/

#ifndef CBACKBUFGDI_INCLUDED
#define CBACKBUFGDI_INCLUDED

class CBackBufGDI : public WObject {
public:
	CBackBufGDI();
	virtual	~CBackBufGDI();
	bool	Create(CWnd *Wnd);
	void	Destroy();
	bool	CreateSurface(int Width, int Height);
	void	DeleteSurface();
	HDC 	GetDC();
	bool	Blt(CDC *pFrontDC);
	bool	IsCreated() const;
	void	SetMirror(bool Enable);
	bool	IsMirrored() const;

private:
	CDC		m_BackDC;			// back buffer device context
	CBitmap	m_BackBitmap;		// back buffer bitmap
	CDC		m_MirrorDC;			// mirroring device context
	CBitmap	m_MirrorBitmap;		// mirroring bitmap
	HGDIOBJ	m_DefaultBitmap;	// back buffer DC's default bitmap
	CWnd	*m_Wnd;				// parent window
	int		m_Width;			// width of window, in client coords
	int		m_Height;			// height of window, in client coords
	bool	m_IsMirrored;		// true if we're mirroring
};

inline HDC CBackBufGDI::GetDC()
{
	return(m_BackDC.m_hDC);
}

inline bool CBackBufGDI::IsCreated() const
{
	return(m_BackDC.m_hDC != NULL);
}

inline bool CBackBufGDI::IsMirrored() const
{
	return(m_IsMirrored);
}

#endif
