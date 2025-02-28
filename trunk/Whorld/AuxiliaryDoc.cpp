// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17sep13	initial version
		01		28feb25	add resource ID to ctor

		auxiliary document
 
*/

// AuxiliaryDoc.cpp : implementation of the CAuxiliaryDoc class
//

#include "stdafx.h"
#include "Resource.h"
#include "AuxiliaryDoc.h"

// CAuxiliaryDoc

IMPLEMENT_DYNCREATE(CAuxiliaryDoc, CDocument)

// CAuxiliaryDoc construction/destruction

CAuxiliaryDoc::CAuxiliaryDoc() :
	m_tplDoc(0, RUNTIME_CLASS(CAuxiliaryDoc), NULL, NULL),
	m_listRecentFile(0, NULL, NULL, 0)
{
	// this ctor has to exist or IMPLEMENT_DYNCREATE won't compile
	ASSERT(0);	// but it's private and should never be used
}

CAuxiliaryDoc::CAuxiliaryDoc(UINT nIDResource, UINT nStart, LPCTSTR lpszSection, LPCTSTR lpszEntryFormat, int nSize, int nMaxDispLen) :
	m_tplDoc(nIDResource, RUNTIME_CLASS(CAuxiliaryDoc), NULL, NULL),
	m_listRecentFile(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)
{
	m_bAutoDelete = FALSE;
	m_tplDoc.AddDocument(this);
	m_listRecentFile.ReadList();
}

CAuxiliaryDoc::~CAuxiliaryDoc()
{
	m_tplDoc.RemoveDocument(this);
	m_listRecentFile.WriteList();
}

BOOL CAuxiliaryDoc::New()
{
	if (!Open(NULL))
		return FALSE;
	return TRUE;
}

BOOL CAuxiliaryDoc::OpenPrompt()
{
	CString	path;
	if (!AfxGetApp()->DoPromptFileName(path, AFX_IDS_OPENFILE, 
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, &m_tplDoc))
		return FALSE;
	return Open(path);
}

BOOL CAuxiliaryDoc::Open(LPCTSTR lpszPathName)
{
	return m_tplDoc.OpenDocumentFile(lpszPathName) != NULL;
}

BOOL CAuxiliaryDoc::OpenRecent(int iFile)
{
	CString	path(m_listRecentFile[iFile]);
	if (!Open(path)) {
		m_listRecentFile.Remove(iFile);
		return FALSE;
	}
	return TRUE;
}

void CAuxiliaryDoc::CAuxDocTemplate::InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc, BOOL bMakeVisible)
{
	UNREFERENCED_PARAMETER(pFrame);
	UNREFERENCED_PARAMETER(pDoc);
	UNREFERENCED_PARAMETER(bMakeVisible);
	// we don't have a frame window, so skip base class implementation;
	// this avoids sending a spurious show command to app's main window
}

// CAuxiliaryDoc diagnostics

#ifdef _DEBUG
void CAuxiliaryDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAuxiliaryDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CAuxiliaryDoc message map

BEGIN_MESSAGE_MAP(CAuxiliaryDoc, CDocument)
END_MESSAGE_MAP()

// CAuxiliaryDoc commands

void CAuxiliaryDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	// override avoids adding path to app's recent file list
	CDocument::SetPathName(lpszPathName, FALSE);	// force bAddToMRU false
	if (bAddToMRU)
		m_listRecentFile.Add(lpszPathName);
}
