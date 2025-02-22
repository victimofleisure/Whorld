// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		20feb25	add file export
		02		21feb25	add options

*/

// WhorldView.cpp : implementation of the CWhorldView class
//

#include "stdafx.h"
#include "Whorld.h"

#include "WhorldDoc.h"
#include "WhorldView.h"
#include "MainFrm.h"
#include "PathStr.h"
#include "ExportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWhorldView

IMPLEMENT_DYNCREATE(CWhorldView, CView)

// CWhorldView construction/destruction

CWhorldView::CWhorldView()
{
	theApp.SetView(this);	// SDI shortcut
}

CWhorldView::~CWhorldView()
{
}

// CWhorldView overrides

BOOL CWhorldView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CWhorldView::OnDraw(CDC* /*pDC*/)
{
	// do nothing; actual drawing is done by CRenderWnd
}

// CWhorldView operations

void CWhorldView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// handle view updates by delegating them to the render thread
	CWhorldDoc	*pDoc = GetDocument();
	switch (lHint) {
	case HINT_NONE:
		{
			CRenderCmd	cmd(RC_SET_PATCH);
			// dynamically allocate patch data and enqueue a pointer to it;
			// recipient (render thread) is responsible for deleting buffer
			CPatch&	patch = *pDoc;	// upcast from document to patch data
			CPatch	*pPatch = new CPatch(patch);	// allocate new patch on heap
			cmd.m_prop.byref = pPatch;	// command property is pointer to patch
			theApp.PushRenderCommand(cmd);	// patch belongs to render thread now
		}
		break;
	case HINT_PARAM:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			CRenderCmd	cmd(RC_SET_PARAM_Val + pParamHint->m_iProp, pParamHint->m_iParam);
			pDoc->GetParamRC(pParamHint->m_iParam, pParamHint->m_iProp, cmd.m_prop);
			theApp.PushRenderCommand(cmd);
		}
		break;
	case HINT_MASTER:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			CRenderCmd	cmd(RC_SET_MASTER, pParamHint->m_iParam);
			cmd.m_prop.dblVal = pDoc->GetMasterProp(pParamHint->m_iParam);
			theApp.PushRenderCommand(cmd);
		}
		break;
	case HINT_MAIN:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			CRenderCmd	cmd(RC_SET_MAIN, pParamHint->m_iParam);
			pDoc->GetMainProp(pParamHint->m_iParam, cmd.m_prop);
			theApp.PushRenderCommand(cmd);
		}
		break;
	}
	// relay update to dockable panes via main frame
	theApp.GetMainFrame()->OnUpdate(pSender, lHint, pHint);
}

bool CWhorldView::PromptForExportData(CString& sExportPath)
{
	static const LPCTSTR pszPngFilter = _T("PNG Files (*.png)|All Files (*.*)|*.*||");
	CFileDialog	fd(false, _T("png"), NULL, OFN_OVERWRITEPROMPT, pszPngFilter);
	if (fd.DoModal() != IDOK) {	// display folder dialog
		return false;	// user canceled
	}
	sExportPath = fd.GetPathName();
	CExportDlg	dlg;
	if (dlg.DoModal() != IDOK) {	// display export options dialog
		return false;	// user canceled
	}
	return true;
}

bool CWhorldView::MakeExportPath(CString& sExportPath)
{
	static const LPCTSTR pszPngExt = _T(".png");
	CPathStr	sPath(theApp.m_options.m_Export_sImageFolder);
	if (!PathFileExists(sPath)) {	// if export image folder doesn't exist
		AfxMessageBox(IDS_APP_ERR_BAD_EXPORT_IMAGE_FOLDER);
		return false;
	}
	// automatically assign a filename based on date and time
	sPath.Append(theApp.GetTimestampFileName());	// append filename to folder
	sExportPath = sPath + pszPngExt;	// append file extension to filename
	return true;
}

// CWhorldView diagnostics

#ifdef _DEBUG
void CWhorldView::AssertValid() const
{
	CView::AssertValid();
}

void CWhorldView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWhorldDoc* CWhorldView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWhorldDoc)));
	return (CWhorldDoc*)m_pDocument;
}
#endif //_DEBUG

// CWhorldView message map

BEGIN_MESSAGE_MAP(CWhorldView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
	ON_COMMAND(ID_WINDOW_PAUSE, OnWindowPause)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PAUSE, OnUpdateWindowPause)
	ON_COMMAND(ID_WINDOW_STEP, OnWindowStep)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_STEP, OnUpdateWindowStep)
	ON_COMMAND(ID_WINDOW_CLEAR, OnWindowClear)
	ON_COMMAND(ID_IMAGE_RANDOM_PHASE, OnImageRandomPhase)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
END_MESSAGE_MAP()

// CWhorldView message handlers

LRESULT CWhorldView::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	CRect	rc;
	GetClientRect(rc);
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
	theApp.CreateRenderWnd(dwStyle, rc, this);
	return 0;
}

void CWhorldView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0) {
		if (!theApp.IsDetached()) {
			theApp.ResizeRenderWnd(cx, cy);
		}
	}
}

void CWhorldView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWhorldView::OnContextMenu(CWnd* /* pWnd */, CPoint /* point */)
{
	// stock project displays a popup edit menu; disable that for now
	// theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

void CWhorldView::OnWindowPause()
{
	CRenderCmd	cmd(RC_SET_PAUSE, !theApp.IsPaused());
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnUpdateWindowPause(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.IsPaused());
}

void CWhorldView::OnWindowStep()
{
	CRenderCmd	cmd(RC_SINGLE_STEP);
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnUpdateWindowStep(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.IsPaused());
}

void CWhorldView::OnWindowClear()
{
	CRenderCmd	cmd(RC_SET_EMPTY, !theApp.IsPaused());
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnImageRandomPhase()
{
	CRenderCmd	cmd(RC_RANDOM_PHASE);
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnFileExport()
{
	CString	sExportPath;
	if (theApp.m_options.m_Export_bPromptUser	// if user wants to be prompted
	&& !theApp.IsFullScreenSingleMonitor()) {	// and prompting is permissible
		// prompt user for destination folder, filename, and export options
		if (!PromptForExportData(sExportPath))
			return;	// user canceled
	}
	// render thread captures bitmap and posts it to our main window for writing
	CRenderCmd	cmd(RC_CAPTURE_BITMAP, theApp.m_options.GetExportFlags());
	cmd.m_prop.szVal = theApp.m_options.GetExportImageSize();
	theApp.PushRenderCommand(cmd);	// start capture ASAP in case we're unpaused
	if (sExportPath.IsEmpty()) {	// if export path is unspecified
		if (!MakeExportPath(sExportPath))	// generate path from date and time
			return;	// unable to generate path
	}
	theApp.GetMainFrame()->AddImageExportPath(sExportPath);
	// bitmap capture message may already be waiting for us in message queue
}
