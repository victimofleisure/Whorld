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

#pragma once

class CAuxiliaryDoc : public CDocument
{
private:
	CAuxiliaryDoc();	// needed for IMPLEMENT_DYNCREATE
protected:
	DECLARE_DYNCREATE(CAuxiliaryDoc)

// Construction
	CAuxiliaryDoc(UINT nIDResource, UINT nStart, LPCTSTR lpszSection, 
		LPCTSTR lpszEntryFormat, int nSize, int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

// Attributes
public:

// Operations
public:
	BOOL	New();
	BOOL	OpenPrompt();
	BOOL	Open(LPCTSTR lpszPathName);
	BOOL	OpenRecent(int iFile);
	void	UpdateMruMenu(CCmdUI* pCmdUI);

// Public data
	CRecentFileList	m_listRecentFile;	// array of recently used files

// Overrides
	public:
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

// Implementation
public:
	virtual ~CAuxiliaryDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
// Types
	class CAuxDocTemplate : public CSingleDocTemplate {
	public:
		CAuxDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
		virtual	void	InitialUpdateFrame(CFrameWnd* pFrame, CDocument* pDoc, BOOL bMakeVisible = TRUE);
	};

// Data members
	CAuxDocTemplate	m_tplDoc;	// auxiliary document template

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

inline CAuxiliaryDoc::CAuxDocTemplate::CAuxDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
	: CSingleDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
}

inline void CAuxiliaryDoc::UpdateMruMenu(CCmdUI* pCmdUI)
{
	m_listRecentFile.UpdateMenu(pCmdUI);
}
