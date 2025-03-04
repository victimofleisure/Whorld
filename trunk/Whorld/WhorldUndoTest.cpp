// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08oct13	initial version
        01      07may14	move generic functionality to base class
		02		09sep14	move enable flag to Globals.h
		03		15mar15	remove 16-bit assert in rename case
		04		06apr15	in DoPageEdit, skip disabled controls
		05		02jan19	add case for range type property
		06		26feb19	change master property default to fail gracefully
		07		02dec19	remove sort function, array now provides it
		08		20mar20	add mapping
		09		29mar20	add sort and learn mapping
		10		07apr20	add move steps
		11		19nov20	use set channel property methods
		12		19nov20	add randomized docking bar visibility
		13		21jun21	route track editing commands directly to document
		14		22jan22	limit channel tests to event properties
		15		10sep24	add method to randomize channel property
		16		11feb25	adapt for Whorld

		automated undo test for patch editing
 
*/

#include "stdafx.h"

// to enable this test, edit Globals.h and set UNDO_TEST to 1

#if UNDO_TEST == 1	// do not change this unique identifier

#include "Whorld.h"
#include "WhorldUndoTest.h"
#include "WhorldDoc.h"
#include "MainFrm.h"
#include "UndoState.h"
#include "UndoCodes.h"
#include "RandList.h"
#include "MasterRowDlg.h"

#define TIMER_PERIOD 10	// timer period, in milliseconds

#define PAUSE_VIEW_DURING_TEST false	// unpaused is a tougher test

static CWhorldUndoTest gUndoTest(true);	// one and only instance, initially running

const CWhorldUndoTest::EDIT_INFO CWhorldUndoTest::m_arrEditInfo[] = {
	{UCODE_PARAM,				1},
	{UCODE_MASTER,				0.1f},
	{UCODE_MAIN,				0.1f},
};

CWhorldUndoTest::CWhorldUndoTest(bool bInitRunning) :
	CUndoTest(bInitRunning, TIMER_PERIOD, m_arrEditInfo, _countof(m_arrEditInfo))
{
	m_pDoc = NULL;
#if 0
	m_nCycles = 1;
	m_nPasses = 2;
	m_nPassEdits = 10;
	m_nPassUndos = 5;
	m_nMaxEdits = INT_MAX;
	m_nRandSeed = 666;
	m_bMakeSnapshots = true;
#else
	m_nCycles = 1;
	m_nPasses = 10;
	m_nPassEdits = 250;
	m_nPassUndos = 100;
	m_nMaxEdits = INT_MAX;
	m_nRandSeed = 666;
	m_bMakeSnapshots = true;
#endif
}

CWhorldUndoTest::~CWhorldUndoTest()
{
}

LONGLONG CWhorldUndoTest::GetSnapshot() const
{
	LONGLONG	nSum = 0;
	const CPatch&	patch = *m_pDoc;
	nSum += Fletcher64(&patch, sizeof(CPatch));
//	_tprintf(_T("%I64x\n"), nSum);
	return nSum;
}

int CWhorldUndoTest::ApplyEdit(int nUndoCode)
{
	CUndoState	state(0, nUndoCode);
	CString	sUndoTitle(m_pDoc->GetUndoTitle(state));
	switch (nUndoCode) {
	case UCODE_PARAM:
		{
			int	iParam = Random(PARAM_COUNT);
			int	iProp = Random(PARAM_PROP_COUNT);
			CComVariant	prop;
			const PARAM_INFO& info = GetParamInfo(iParam);
			switch (iProp) {
			case PARAM_PROP_Val:
				prop = RandomFloat(info.fMaxVal - info.fMinVal) + info.fMinVal;
				break;
			case PARAM_PROP_Wave:
				prop = Random(WAVEFORM_COUNT);
				break;
			case PARAM_PROP_Amp:
				prop = RandomFloat(2) - 1;
				break;
			case PARAM_PROP_Freq:
				prop = RandomFloat(1);
				break;
			case PARAM_PROP_PW:
				prop = RandomFloat(1);
				break;
			case PARAM_PROP_Global:
				prop = RandomFloat(info.fMaxVal * 2) - info.fMaxVal;
				break;
			}
			m_pDoc->SetParam(iParam, iProp, prop);
		}
		break;
	case UCODE_MASTER:
		{
			int	iProp = Random(MASTER_COUNT);
			CMasterRowDlg::DBL_RANGE	range;
			switch (iProp) {
			case MASTER_Zoom:
				// keep zoom on the low side, otherwise giant rings bog down
				// rendering so badly that the render command queue fills up
				range.fMinVal = 1e-3;
				range.fMaxVal = 1e-1;
				break;
			default:
				CMasterRowDlg::GetPropEditRange(iProp, range);
			}
			double	prop = RandomFloat(range.fMaxVal - range.fMinVal) + range.fMinVal;
			m_pDoc->SetMasterProp(iProp, prop);
		}
		break;
	case UCODE_MAIN:
		{
			VARIANT_PROP	prop;
			int	iProp = Random(MAIN_COUNT);
			switch (iProp) {
			case MAIN_Origin:
				prop.fltPt.x = static_cast<float>(RandomFloat(1));
				prop.fltPt.y = static_cast<float>(RandomFloat(1));
				break;
			case MAIN_DrawMode:
				prop.intVal = Random(3);
				break;
			case MAIN_OrgMotion:
				prop.intVal = Random(ORIGIN_MOTION_TYPES);
				break;
			case MAIN_Hue:
				prop.dblVal = RandomFloat(360);
				break;
			default:
				prop.boolVal = Random(2) != 0;	// assume boolean
			}
			m_pDoc->SetMainProp(iProp, prop);
		}
		break;
	default:
		NODEFAULTCASE;
		return ABORT;
	}
	return SUCCESS;
}

bool CWhorldUndoTest::Create()
{
	theApp.SetPause(PAUSE_VIEW_DURING_TEST);
	m_pDoc = theApp.GetDocument();
	m_pUndoMgr = m_pDoc->GetUndoManager();
	m_pUndoMgr->SetLevels(-1);	// unlimited undo
	m_pDoc->UpdateAllViews(NULL);
	if (!CUndoTest::Create())
		return false;
	return true;
}

void CWhorldUndoTest::Destroy()
{
	CUndoTest::Destroy();
	m_pDoc->SetModifiedFlag(false);
}

#endif
