// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		24feb25	fix outline menu item not displaying check
		02		27feb25	disable undo/redo while in snapshot mode
		03		03mar25	don't touch modified flag while in snapshot mode
		04      12mar25	get wheel sensitivity from options

*/

// WhorldDoc.cpp : implementation of the CWhorldDoc class
//

#include "stdafx.h"
#include "Whorld.h"
#include "WhorldDoc.h"
#include "UndoCodes.h"
#include "MasterRowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWhorldDoc

IMPLEMENT_DYNCREATE(CWhorldDoc, CDocument)

// CWhorldDoc construction/destruction

CString	CWhorldDoc::CMyUndoManager::m_sUndoPrefix;
CString	CWhorldDoc::CMyUndoManager::m_sRedoPrefix;

CWhorldDoc::CWhorldDoc()
{
	theApp.SetDocument(this);	// SDI shortcut
	m_UndoMgr.SetRoot(this);
	SetUndoManager(&m_UndoMgr);
	m_bIsUndoDisabled = false;
}

CWhorldDoc::~CWhorldDoc()
{
}

BOOL CWhorldDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	SetDefaults();
	m_UndoMgr.DiscardAllEdits();
	return TRUE;
}

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

// CWhorldDoc commands

BOOL CWhorldDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return false;
	if (!Read(lpszPathName))
		return false;
	return true;
}

BOOL CWhorldDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnSaveDocument(lpszPathName))
		return false;
	if (!Write(lpszPathName))
		return false;
	return true;
}

BOOL CWhorldDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	BOOL	bRet = CDocument::CanCloseFrame(pFrame);
	if (!bRet)	// if close was canceled
		theApp.m_bCleanStateOnExit = false;	// reset clean on exit flag
	return bRet;
}

void CWhorldDoc::SetParam(int iParam, int iProp, const CComVariant& prop, CView *pSender)
{
	NotifyUndoableEdit(MAKELONG(iParam, iProp), UCODE_PARAM, UE_COALESCE);
	CPatch::SetParam(iParam, iProp, prop);	// scope required to avoid recursion
	SetModifiedFlag();
	CParamHint	hint(iParam, iProp);
	UpdateAllViews(pSender, HINT_PARAM, &hint);
}

void CWhorldDoc::SetMasterProp(int iProp, double fProp, CView *pSender)
{
	ASSERT(IsValidMasterProp(iProp));
	NotifyUndoableEdit(iProp, UCODE_MASTER, UE_COALESCE);
	m_master.a[iProp] = fProp;
	SetModifiedFlag();
	CPropHint	hint(iProp);	// master property index
	UpdateAllViews(pSender, HINT_MASTER, &hint);
}

void CWhorldDoc::SetMainProp(int iProp, const VARIANT_PROP& prop, CView *pSender)
{
	ASSERT(IsValidMainProp(iProp));
	NotifyUndoableEdit(iProp, UCODE_MAIN, UE_COALESCE);
	CPatch::SetMainProp(iProp, prop);
	SetModifiedFlag();
	CPropHint	hint(iProp);	// master property index
	UpdateAllViews(pSender, HINT_MAIN, &hint);
}

void CWhorldDoc::SetDrawMode(UINT nDrawMode)
{
	MAKE_VARIANT_PROP(uintVal, nDrawMode);
	SetMainProp(MAIN_DrawMode, prop);
}

void CWhorldDoc::SetMirror(bool bEnable)
{
	MAKE_VARIANT_PROP(boolVal, bEnable);
	SetMainProp(MAIN_Mirror, prop);
}

void CWhorldDoc::SetReverse(bool bEnable)
{
	MAKE_VARIANT_PROP(boolVal, bEnable);
	SetMainProp(MAIN_Reverse, prop);
}

void CWhorldDoc::SetConvex(bool bEnable)
{
	MAKE_VARIANT_PROP(boolVal, bEnable);
	SetMainProp(MAIN_Convex, prop);
}

void CWhorldDoc::SetLoopHue(bool bEnable)
{
	MAKE_VARIANT_PROP(boolVal, bEnable);
	SetMainProp(MAIN_LoopHue, prop);
}

void CWhorldDoc::SetZoomCenter(bool bEnable)
{
	MAKE_VARIANT_PROP(boolVal, bEnable);
	SetMainProp(MAIN_ZoomCenter, prop);
}

void CWhorldDoc::SetOriginMotion(int iMotionType)
{
	ASSERT(iMotionType >= 0 && iMotionType < ORIGIN_MOTION_TYPES);
	if (iMotionType != m_main.nOrgMotion) {
		MAKE_VARIANT_PROP(intVal, iMotionType);
		SetMainProp(MAIN_OrgMotion, prop);
	}
}

void CWhorldDoc::SetZoom(double fZoom, bool bDamping)
{
	if (UndoMgrIsIdle()) {
		NotifyUndoableEdit(0, UCODE_ZOOM, UE_COALESCE);
	}
	theApp.m_thrRender.SetZoom(fZoom, bDamping);
	m_master.fZoom = fZoom;
	SetModifiedFlag();
	UpdateAllViews(NULL, HINT_ZOOM, NULL);
}

void CWhorldDoc::SetOrigin(DPoint ptOrigin, bool bDamping)
{
	if (UndoMgrIsIdle()) {
		NotifyUndoableEdit(0, UCODE_ORIGIN, UE_COALESCE);
	}
	theApp.m_thrRender.SetOrigin(ptOrigin, bDamping);
	m_main.dptOrigin = ptOrigin;
	SetModifiedFlag();
	UpdateAllViews(NULL, HINT_ORIGIN, NULL);
}

bool CWhorldDoc::OffsetMasterProp(int iProp, double fDelta, double& fVal)
{
	double	fOldVal = GetMasterProp(iProp);	// retrieve old value
	fVal = CMasterRowDlg::Norm(iProp, fOldVal);	// normalize value
	fVal += fDelta;	// offset normalized value
	fVal = CLAMP(fVal, 0, 1);	// clamp in normalized space
	fVal = CMasterRowDlg::Denorm(iProp, fVal);	// denormalize updated value
	return fVal != fOldVal;	// return true if value changed
}

BOOL CWhorldDoc::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	UNREFERENCED_PARAMETER(pt);
	double	fWheelScale = theApp.m_options.m_Input_fWheelSense / 100;
	double	fDelta = static_cast<double>(zDelta) / WHEEL_DELTA * fWheelScale;
	if (nFlags & MK_SHIFT) {
		double	fDamping;
		if (OffsetMasterProp(MASTER_Damping, fDelta, fDamping)) {
			SetMasterProp(MASTER_Damping, fDamping);
		}
	} else {
		double	fZoom;
		if (OffsetMasterProp(MASTER_Zoom, fDelta, fZoom)) {
			SetZoom(fZoom);
		}
	}
	return true;
}

void CWhorldDoc::SetModifiedFlag(BOOL bModified)
{
	// Don't touch the modified flag if undo is disabled, for
	// example during remote edits. If we're in snapshot mode, 
	// don't touch the modified flag then either, because any 
	// changes made to the document during snapshot mode are 
	// undone by restoring a backup when snapshot mode ends.
	if (!(m_bIsUndoDisabled || theApp.IsSnapshotMode())) {
		CDocument::SetModifiedFlag(bModified);	// do base class behavior
	}
}

void CWhorldDoc::SetPatch(const CPatch& patch)
{
	CPatch*	pPatch = this;	// upcast to base class
	*pPatch = patch;
	UpdateAllViews(NULL);
	SetModifiedFlag(false);
}

// CWhorldDoc undo handling

CWhorldDoc::CMyUndoManager::CMyUndoManager()
{
	if (m_sUndoPrefix.IsEmpty()) {	// if prefixes not loaded yet
		m_sUndoPrefix.LoadString(IDS_EDIT_UNDO_PREFIX);
		m_sUndoPrefix += ' ';	// add separator
		m_sRedoPrefix.LoadString(IDS_EDIT_REDO_PREFIX);
		m_sRedoPrefix += ' ';	// add separator
	}
	OnUpdateTitles();
}

void CWhorldDoc::CMyUndoManager::OnUpdateTitles()
{
	// append here instead of in undo/redo update command UI handlers,
	// to reduce high-frequency memory reallocation when mouse moves
	m_sUndoMenuItem = m_sUndoPrefix + GetUndoTitle();
	m_sRedoMenuItem = m_sRedoPrefix + GetRedoTitle();
}

void CWhorldDoc::SaveUndoState(CUndoState& State)
{
	switch (LOWORD(State.GetCode())) {
	case UCODE_PARAM:
		{
			int	iParam = LOWORD(State.GetCtrlID());
			int	iProp = HIWORD(State.GetCtrlID());
			VARIANT_PROP	prop;
			GetParamRC(iParam, iProp, prop);
			State.m_Val.i64 = prop.llVal;
		}
		break;
	case UCODE_MASTER:
		{
			int	iProp = State.GetCtrlID();
			State.m_Val.d = GetMasterProp(iProp);
		}
		break;
	case UCODE_MAIN:
		{
			int	iProp = State.GetCtrlID();
			VARIANT_PROP	prop;
			GetMainProp(iProp, prop);
			State.m_Val.i64 = prop.llVal;
		}
		break;
	case UCODE_ZOOM:
		State.m_Val.d = m_master.fZoom;
		break;
	case UCODE_ORIGIN:
		State.m_Val.pt = theApp.m_thrRender.GetOrigin();	// render thread writes origin
		break;
	default:
		NODEFAULTCASE;	// missing undo case
	}
}

void CWhorldDoc::RestoreUndoState(const CUndoState& State)
{
	switch (LOWORD(State.GetCode())) {
	case UCODE_PARAM:
		{
			int	iParam = LOWORD(State.GetCtrlID());
			int	iProp = HIWORD(State.GetCtrlID());
			MAKE_VARIANT_PROP(llVal, State.m_Val.i64);
			SetParamRC(iParam, iProp, prop);
			CParamHint	hint(iParam, iProp);
			UpdateAllViews(NULL, HINT_PARAM, &hint);
		}
		break;
	case UCODE_MASTER:
		{
			int	iProp = State.GetCtrlID();
			SetMasterProp(iProp, State.m_Val.d);
		}
		break;
	case UCODE_MAIN:
		{
			int	iProp = State.GetCtrlID();
			MAKE_VARIANT_PROP(llVal, State.m_Val.i64);
			SetMainProp(iProp, prop);
		}
		break;
	case UCODE_ZOOM:
		SetZoom(State.m_Val.d, false);	// no damping
		break;
	case UCODE_ORIGIN:
		SetOrigin(State.m_Val.pt, false);	// no damping
		break;
	default:
		NODEFAULTCASE;	// missing undo case
	}
}

CString	CWhorldDoc::GetUndoTitle(const CUndoState& State)
{
	CString	sTitle;
	switch (LOWORD(State.GetCode())) {
	case UCODE_PARAM:
		{
			int	iParam = LOWORD(State.GetCtrlID());
			int	iProp = HIWORD(State.GetCtrlID());
			sTitle = GetParamName(iParam);	// title starts with parameter name
			if (iProp != PARAM_PROP_Val) {	// for parameter properties other than value
				sTitle += ' ' + GetParamPropName(iProp);	// also append property name
			}
		}
		break;
	case UCODE_MASTER:
		{
			int	iProp = State.GetCtrlID();
			sTitle = GetMasterName(iProp);
		}
		break;
	case UCODE_MAIN:
		{
			int	iProp = State.GetCtrlID();
			sTitle = GetMainName(iProp);
		}
		break;
	case UCODE_ZOOM:
		sTitle = GetMasterName(MASTER_Zoom);
		break;
	case UCODE_ORIGIN:
		sTitle = GetMainName(MAIN_Origin);
		break;
	default:
		NODEFAULTCASE;	// missing undo case
	}
	return sTitle;
}

void CWhorldDoc::NotifyUndoableEdit(int nCtrlID, int nCode, UINT nFlags)
{
	// if undo is disabled, or we're in snapshot mode
	if (m_bIsUndoDisabled || theApp.IsSnapshotMode()) {
		return;	// disable undo notification
	}
	CUndoable::NotifyUndoableEdit(nCtrlID, nCode, nFlags);
}

// CWhorldDoc message map

BEGIN_MESSAGE_MAP(CWhorldDoc, CDocument)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_IMAGE_FILL, OnImageFill)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_FILL, OnUpdateImageFill)
	ON_COMMAND(ID_IMAGE_OUTLINE, OnImageOutline)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_OUTLINE, OnUpdateImageOutline)
	ON_COMMAND(ID_IMAGE_MIRROR, OnImageMirror)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_MIRROR, OnUpdateImageMirror)
	ON_COMMAND(ID_IMAGE_REVERSE, OnImageReverse)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_REVERSE, OnUpdateImageReverse)
	ON_COMMAND(ID_IMAGE_CONVEX, OnImageConvex)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_CONVEX, OnUpdateImageConvex)
	ON_COMMAND(ID_IMAGE_LOOP_HUE, OnImageLoopHue)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_LOOP_HUE, OnUpdateImageLoopHue)
	ON_COMMAND(ID_IMAGE_ZOOM_CENTER, OnImageZoomCenter)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_ZOOM_CENTER, OnUpdateImageZoomCenter)
	ON_COMMAND(ID_IMAGE_ORIGIN_CENTER, OnImageOriginCenter)
	ON_COMMAND(ID_IMAGE_ORIGIN_DRAG, OnImageOriginDrag)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_ORIGIN_DRAG, OnUpdateImageOriginDrag)
	ON_COMMAND(ID_IMAGE_ORIGIN_RANDOM, OnImageOriginRandom)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_ORIGIN_RANDOM, OnUpdateImageOriginRandom)
END_MESSAGE_MAP()

// CWhorldDoc message handlers

void CWhorldDoc::OnEditUndo()
{
	m_UndoMgr.Undo();
	SetModifiedFlag();	// undo counts as modification
}

void CWhorldDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(m_UndoMgr.m_sUndoMenuItem);
	pCmdUI->Enable(m_UndoMgr.CanUndo() && !theApp.IsSnapshotMode());
}

void CWhorldDoc::OnEditRedo()
{
	m_UndoMgr.Redo();
	SetModifiedFlag();	// redo counts as modification
}

void CWhorldDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(m_UndoMgr.m_sRedoMenuItem);
	pCmdUI->Enable(m_UndoMgr.CanRedo() && !theApp.IsSnapshotMode());
}

void CWhorldDoc::OnImageFill()
{
	SetDrawMode(m_main.nDrawMode ^ DM_FILL);
}

void CWhorldDoc::OnUpdateImageFill(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((m_main.nDrawMode & DM_FILL) != 0);
}

void CWhorldDoc::OnImageOutline()
{
	SetDrawMode(m_main.nDrawMode ^ DM_OUTLINE);
}

void CWhorldDoc::OnUpdateImageOutline(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((m_main.nDrawMode & DM_OUTLINE) != 0);
}

void CWhorldDoc::OnImageMirror()
{
	SetMirror(!m_main.bMirror);
}

void CWhorldDoc::OnUpdateImageMirror(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.bMirror);
}

void CWhorldDoc::OnImageReverse()
{
	SetReverse(!m_main.bReverse);
}

void CWhorldDoc::OnUpdateImageReverse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.bReverse);
}

void CWhorldDoc::OnImageConvex()
{
	SetConvex(!m_main.bConvex);
}

void CWhorldDoc::OnUpdateImageConvex(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.bConvex);
}

void CWhorldDoc::OnImageLoopHue()
{
	SetLoopHue(!m_main.bLoopHue);
}

void CWhorldDoc::OnUpdateImageLoopHue(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.bLoopHue);
}

void CWhorldDoc::OnImageZoomCenter()
{
	SetZoomCenter(!m_main.bZoomCenter);
}

void CWhorldDoc::OnUpdateImageZoomCenter(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.bZoomCenter);
}

void CWhorldDoc::OnImageOriginCenter()
{
	SetOriginMotion(OM_PARK);
	SetOrigin(DPoint(0.5, 0.5), false);	// center, no damping
}

void CWhorldDoc::OnImageOriginDrag()
{
	SetOriginMotion(m_main.nOrgMotion == OM_DRAG ? OM_PARK : OM_DRAG);
}

void CWhorldDoc::OnUpdateImageOriginDrag(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.nOrgMotion == OM_DRAG);
}

void CWhorldDoc::OnImageOriginRandom()
{
	SetOriginMotion(m_main.nOrgMotion == OM_RANDOM ? OM_PARK : OM_RANDOM);
}

void CWhorldDoc::OnUpdateImageOriginRandom(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_main.nOrgMotion == OM_RANDOM);
}
