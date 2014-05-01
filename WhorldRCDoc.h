// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28may06	initial version
        01      26jun06	remove import

        remote control document
 
*/

// WhorldRCDoc.h : interface of the CWhorldRCDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHORLDRCDOC_H__3BF048B2_7747_46B4_92C6_83602DB8C3A6__INCLUDED_)
#define AFX_WHORLDRCDOC_H__3BF048B2_7747_46B4_92C6_83602DB8C3A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PatchLink.h"
#include "PlaylistDlg.h"

class CWhorldRCDoc : public CDocument
{
// Construction
public:
	CWhorldRCDoc();
	DECLARE_DYNCREATE(CWhorldRCDoc)

// Attributes
public:
	enum {
		HINT_NONE,
		HINT_BANK_CHANGE
	};
	enum {
		BANKS = CPlaylistDlg::BANKS
	};
	typedef CArray<CPatchLink, CPatchLink&> PATCH_LIST;
	PATCH_LIST	m_Bank[BANKS];
	CImageList	m_ImgList;
	CSize	m_ThumbSize;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhorldRCDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWhorldRCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWhorldRCDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	enum {
		DOC_SIG = 0x43524857,	// signature: WHRC (WHorld Remote Control)
		DOC_VERSION = 1
	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHORLDRCDOC_H__3BF048B2_7747_46B4_92C6_83602DB8C3A6__INCLUDED_)
