// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version

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
	void	SetReverse(bool bEnable);
	void	SetConvex(bool bEnable);
	void	SetMirror(bool bEnable);
	void	SetZoom(double fZoom, bool bDamped = true);
	void	SetOrigin(DPoint ptOrigin, bool bDamped = true);
	bool	OffsetMasterProp(int iProp, double fDelta, double& fVal);
	BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

// Operations

// Overrides
	virtual BOOL 	OnNewDocument();
	virtual BOOL	OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL	OnSaveDocument(LPCTSTR lpszPathName);
	virtual	void	SaveUndoState(CUndoState& State);
	virtual	void	RestoreUndoState(const CUndoState& State);
	virtual	CString	GetUndoTitle(const CUndoState& State);
	virtual BOOL	CanCloseFrame(CFrameWnd* pFrame);

// Implementation
public:
	virtual ~CWhorldDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
// Types
	class CMyUndoManager : public CUndoManager {
	public:
		CMyUndoManager();
		virtual	void	OnUpdateTitles();
		CString	m_sUndoMenuItem;	// undo's edit menu item string; prefixed undo title
		CString	m_sRedoMenuItem;	// redo's edit menu item string; prefixed redo title
		static	CString	m_sUndoPrefix;	// prefix for undo's edit menu item, from resource
		static	CString	m_sRedoPrefix;	// prefix for redo's edit menu item, from resource
	};
	CMyUndoManager	m_UndoMgr;	// undo manager

protected:
// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnImageConvex();
	afx_msg void OnUpdateImageConvex(CCmdUI *pCmdUI);
	afx_msg void OnImageReverse();
	afx_msg void OnUpdateImageReverse(CCmdUI *pCmdUI);
	afx_msg void OnImageFill();
	afx_msg void OnUpdateImageFill(CCmdUI *pCmdUI);
	afx_msg void OnImageOutline();
	afx_msg void OnUpdateImageOutline(CCmdUI *pCmdUI);
	afx_msg void OnImageOrigin(UINT nID);
	afx_msg void OnUpdateImageOrigin(CCmdUI *pCmdUI);
	afx_msg void OnImageOriginCenter();
	afx_msg void OnImageOriginDrag();
	afx_msg void OnUpdateImageOriginDrag(CCmdUI *pCmdUI);
	afx_msg void OnImageOriginRandom();
	afx_msg void OnUpdateImageOriginRandom(CCmdUI *pCmdUI);
public:
	afx_msg void OnImageMirror();
	afx_msg void OnUpdateImageMirror(CCmdUI *pCmdUI);
};
