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

// WhorldRCDoc.cpp : implementation of the CWhorldRCDoc class
//

#include "stdafx.h"
#include "Resource.h"
#include "WhorldRCDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWhorldRCDoc

IMPLEMENT_DYNCREATE(CWhorldRCDoc, CDocument)

BEGIN_MESSAGE_MAP(CWhorldRCDoc, CDocument)
	//{{AFX_MSG_MAP(CWhorldRCDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhorldRCDoc construction/destruction

CWhorldRCDoc::CWhorldRCDoc()
{
	m_ThumbSize = CSize(0, 0);
}

CWhorldRCDoc::~CWhorldRCDoc()
{
}

BOOL CWhorldRCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	for (int i = 0; i < BANKS; i++)
		m_Bank[i].RemoveAll();
	m_ImgList.DeleteImageList();
	m_ThumbSize = CSize(0, 0);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWhorldRCDoc serialization

void CWhorldRCDoc::Serialize(CArchive& ar)
{
	int	Banks;
	if (ar.IsStoring())
	{
		ar << DOC_SIG;
		ar << DOC_VERSION;
		ar << BANKS;
		for (int i = 0; i < BANKS; i++)
			m_Bank[i].Serialize(ar);
		m_ImgList.Write(&ar);
		ar << m_ThumbSize;
	}
	else
	{
		int	Sig, Version;
		ar >> Sig;
		ar >> Version;
		ar >> Banks;
		for (int i = 0; i < Banks; i++)
			m_Bank[i].Serialize(ar);
		m_ImgList.DeleteImageList();
		m_ImgList.Read(&ar);
		ar >> m_ThumbSize;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldRCDoc diagnostics

#ifdef _DEBUG
void CWhorldRCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWhorldRCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWhorldRCDoc commands

