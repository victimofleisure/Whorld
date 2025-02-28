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

#if UNDO_TEST

#include "Whorld.h"
#include "WhorldUndoTest.h"
#include "WhorldDoc.h"
#include "MainFrm.h"
#include "UndoState.h"
#include "UndoCodes.h"
#include "RandList.h"
#include "MasterRowDlg.h"

#define TIMER_PERIOD 0			// timer period, in milliseconds

static CWhorldUndoTest gUndoTest(TRUE);	// one and only instance, initially running

const CWhorldUndoTest::EDIT_INFO CWhorldUndoTest::m_EditInfo[] = {
	{UCODE_PARAM,				1},
	{UCODE_MASTER,				0.1f},
};

CWhorldUndoTest::CWhorldUndoTest(bool InitRunning) :
	CUndoTest(InitRunning, TIMER_PERIOD, m_EditInfo, _countof(m_EditInfo))
{
	m_pDoc = NULL;
#if 0
	m_Cycles = 1;
	m_Passes = 2;
	m_PassEdits = 10;
	m_PassUndos = 5;
	m_MaxEdits = INT_MAX;
	m_RandSeed = 666;
	m_MakeSnapshots = 1;
#else
	m_Cycles = 1;
	m_Passes = 10;
	m_PassEdits = 250;
	m_PassUndos = 100;
	m_MaxEdits = INT_MAX;
	m_RandSeed = 666;
	m_MakeSnapshots = 1;
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
	return(nSum);
}

int CWhorldUndoTest::ApplyEdit(int UndoCode)
{
	CUndoState	state(0, UndoCode);
	CString	sUndoTitle(m_pDoc->GetUndoTitle(state));
	switch (UndoCode) {
	case UCODE_PARAM:
		{
			int	iParam = Random(PARAM_COUNT);
			int	iProp = Random(PARAM_PROP_COUNT);
			CComVariant	prop;
			switch (iProp) {
			case PARAM_PROP_Val:
				{
					const PARAM_INFO& info = GetParamInfo(iParam);
					prop = RandomFloat(info.fMaxVal - info.fMinVal) + info.fMinVal;
				}
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
			}
			m_pDoc->SetParam(iParam, iProp, prop);
		}
		break;
	case UCODE_MASTER:
		{
			int	iProp = Random(MASTER_COUNT);
			CMasterRowDlg::DBL_RANGE	range;
			CMasterRowDlg::GetPropRange(iProp, range);
			double	prop = RandomFloat(range.fMaxVal - range.fMinVal) + range.fMinVal;
			m_pDoc->SetMasterProp(iProp, prop);
		}
		break;
	default:
		NODEFAULTCASE;
		return(ABORT);
	}
	return(SUCCESS);
}

bool CWhorldUndoTest::Create()
{
	m_pDoc = theApp.GetDocument();
	m_UndoMgr = m_pDoc->GetUndoManager();
	m_UndoMgr->SetLevels(-1);	// unlimited undo
	m_pDoc->UpdateAllViews(NULL);
	if (!CUndoTest::Create())
		return(FALSE);
	return(TRUE);
}

void CWhorldUndoTest::Destroy()
{
	CUndoTest::Destroy();
	m_pDoc->SetModifiedFlag(FALSE);
}

#endif
