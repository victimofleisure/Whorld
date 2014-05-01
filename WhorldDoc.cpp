// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		21may05	add parm info object
		02		22may05	post new doc message
		03		28may05	post new doc from save too
        04      17apr06	use CPatch instead of CParmInfo
		05		21dec07	rename GetMainFrame to GetThis

        Whorld document
 
*/

// WhorldDoc.cpp : implementation of the CWhorldDoc class
//

#include "stdafx.h"
#include "Resource.h"
#include "WhorldDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWhorldDoc

IMPLEMENT_DYNCREATE(CWhorldDoc, CDocument)

BEGIN_MESSAGE_MAP(CWhorldDoc, CDocument)
	//{{AFX_MSG_MAP(CWhorldDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhorldDoc construction/destruction

CWhorldDoc::CWhorldDoc()
{
	m_Patch.SetDefaults();
}

CWhorldDoc::~CWhorldDoc()
{
}

BOOL CWhorldDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_Patch.SetDefaults();
	CMainFrame	*frm = CMainFrame::GetThis();
	if (frm != NULL)
		frm->PostMessage(UWM_NEWDOCUMENT);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldDoc serialization

void CWhorldDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldDoc diagnostics

#ifdef _DEBUG
void CWhorldDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWhorldDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWhorldDoc commands

BOOL CWhorldDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!m_Patch.Read(lpszPathName))
		return(FALSE);
	SetModifiedFlag(FALSE);
	CMainFrame	*frm = CMainFrame::GetThis();
	if (frm != NULL)
		frm->PostMessage(UWM_NEWDOCUMENT);
	return(TRUE);
}

BOOL CWhorldDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CMainFrame	*frm = CMainFrame::GetThis();
	if (frm != NULL)
		frm->GetPatch(m_Patch);	// get master and main info
	if (!m_Patch.Write(lpszPathName))
		return(FALSE);
	SetModifiedFlag(FALSE);
	if (frm != NULL)
		frm->PostMessage(UWM_NEWDOCUMENT, TRUE);	// save
	return(TRUE);
}
