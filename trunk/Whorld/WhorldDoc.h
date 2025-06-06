// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		27feb25	customize undo notify to support disabling
		02		03mar25	override set modified flag accessor
		03		12mar25	add zoom center command
		04      11apr25	add antialiasing main property

*/

// WhorldDoc.h : interface of the CWhorldDoc class
//

#pragma once

#include "Patch.h"
#include "Undoable.h"
#include "UndoManager.h"

class CWhorldDoc : public CDocument, public CPatch, public CUndoable
{
protected: // create from serialization only
	CWhorldDoc();
	DECLARE_DYNCREATE(CWhorldDoc)

// Attributes
public:
	void	SetParam(int iParam, int iProp, const CComVariant& prop, CView *pSender = NULL);
	void	SetMasterProp(int iProp, double fProp, CView *pSender = NULL);
	void	SetMainProp(int iProp, const VARIANT_PROP& prop, CView *pSender = NULL);
	void	SetOriginMotion(int iMotionType);
	void	SetDrawMode(UINT nDrawMode);
	void	SetMirror(bool bEnable);
	void	SetReverse(bool bEnable);
	void	SetConvex(bool bEnable);
	void	SetLoopHue(bool bEnable);
	void	SetZoomCenter(bool bEnable);
	void	SetAntialiasing(bool bEnable);
	void	SetZoom(double fZoom, bool bDamping = true);
	void	SetOrigin(DPoint ptOrigin, bool bDamping = true);
	bool	OffsetMasterProp(int iProp, double fDelta, double& fVal);
	BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	CPatch&	GetPatch();
	void	SetPatch(const CPatch& patch);

// Operations
	void	NotifyUndoableEdit(int nCtrlID, int nCode, UINT nFlags = 0);

// Overrides
	virtual BOOL 	OnNewDocument();
	virtual BOOL	OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL	OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL	CanCloseFrame(CFrameWnd* pFrame);
	virtual	void	SaveUndoState(CUndoState& State);
	virtual	void	RestoreUndoState(const CUndoState& State);
	virtual	CString	GetUndoTitle(const CUndoState& State);
	virtual void	SetModifiedFlag(BOOL bModified = TRUE);

// Public types
	class CMyUndoManager : public CUndoManager {
	public:
		CMyUndoManager();
		virtual	void	OnUpdateTitles();
		CString	m_sUndoMenuItem;	// undo's edit menu item string; prefixed undo title
		CString	m_sRedoMenuItem;	// redo's edit menu item string; prefixed redo title
		static	CString	m_sUndoPrefix;	// prefix for undo's edit menu item, from resource
		static	CString	m_sRedoPrefix;	// prefix for redo's edit menu item, from resource
	};
	class CDisableUndo {
	public:
		CDisableUndo(CWhorldDoc *pDoc);
		~CDisableUndo();
		CWhorldDoc *m_pDoc;		// pointer to document
		bool	m_bPrevState;	// previous state of undo disabled flag
	};

// Implementation
public:
	virtual ~CWhorldDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
// Data members
	CMyUndoManager	m_UndoMgr;	// undo manager
	bool	m_bIsUndoDisabled;	// true if undo is disabled

protected:
// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnImageFill();
	afx_msg void OnUpdateImageFill(CCmdUI *pCmdUI);
	afx_msg void OnImageOutline();
	afx_msg void OnUpdateImageOutline(CCmdUI *pCmdUI);
	afx_msg void OnImageMirror();
	afx_msg void OnUpdateImageMirror(CCmdUI *pCmdUI);
	afx_msg void OnImageReverse();
	afx_msg void OnUpdateImageReverse(CCmdUI *pCmdUI);
	afx_msg void OnImageConvex();
	afx_msg void OnUpdateImageConvex(CCmdUI *pCmdUI);
	afx_msg void OnImageLoopHue();
	afx_msg void OnUpdateImageLoopHue(CCmdUI *pCmdUI);
	afx_msg void OnImageZoomCenter();
	afx_msg void OnImageAntialiasing();
	afx_msg void OnUpdateImageAntialiasing(CCmdUI *pCmdUI);
	afx_msg void OnUpdateImageZoomCenter(CCmdUI *pCmdUI);
	afx_msg void OnUpdateImageOrigin(CCmdUI *pCmdUI);
	afx_msg void OnImageOriginCenter();
	afx_msg void OnImageOriginDrag();
	afx_msg void OnUpdateImageOriginDrag(CCmdUI *pCmdUI);
	afx_msg void OnImageOriginRandom();
	afx_msg void OnUpdateImageOriginRandom(CCmdUI *pCmdUI);
};

inline CPatch& CWhorldDoc::GetPatch()
{
	return *this;	// upcast to patch data base class
}

inline CWhorldDoc::CDisableUndo::CDisableUndo(CWhorldDoc *pDoc)
{
	ASSERT(pDoc != NULL);
	m_pDoc = pDoc;
	m_bPrevState = pDoc->m_bIsUndoDisabled;
	pDoc->m_bIsUndoDisabled = true;
}

inline CWhorldDoc::CDisableUndo::~CDisableUndo()
{
	m_pDoc->m_bIsUndoDisabled = m_bPrevState;
}
