// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
        01      17apr06	use CPatch instead of CParmInfo

        Whorld document
 
*/

// WhorldDoc.h : interface of the CWhorldDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHORLDDOC_H__99C3DBAC_EA0D_4598_BBA4_F4E7E976B5EA__INCLUDED_)
#define AFX_WHORLDDOC_H__99C3DBAC_EA0D_4598_BBA4_F4E7E976B5EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Patch.h"

class CWhorldDoc : public CDocument
{
	DECLARE_DYNCREATE(CWhorldDoc)
public:
	CWhorldDoc();

// Attributes
public:
	CPatch	m_Patch;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhorldDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWhorldDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CWhorldDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHORLDDOC_H__99C3DBAC_EA0D_4598_BBA4_F4E7E976B5EA__INCLUDED_)
