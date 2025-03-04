// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08oct13	initial version
        01      07may14	refactor into abstract base class
		02		09sep14	in TimerProc, change idEvent type to fix compiler error
		03		07jun21	rename rounding functions
		04		04mar25	modernize style

		automated undo test
 
*/

#include "stdafx.h"

#if UNDO_TEST

#include "Resource.h"
#include "UndoTest.h"
#include "RandList.h"
#include "UndoManager.h"

CUndoTest	*CUndoTest::m_pThis;	// pointer to instance

const LPCTSTR CUndoTest::m_arrStateName[STATES] = {
	_T("Stop"), 
	_T("Edit"), 
	_T("Undo"), 
	_T("Redo"),
};

#define LOG_FILE_PATH _T("UndoTest%s.log")

CUndoTest::CUndoTest(bool bInitRunning, int nTimerPeriod, const EDIT_INFO *pEditInfo, int nEditInfoSize)
{
	Init();
	m_bInitRunning = bInitRunning;
	m_nTimerPeriod = nTimerPeriod;
	// copy edit properties
	m_arrEditInfo.SetSize(nEditInfoSize);
	for (int iInfo = 0; iInfo < nEditInfoSize; iInfo++)
		m_arrEditInfo[iInfo] = pEditInfo[iInfo];
	ASSERT(m_pThis == NULL);
	m_pThis = this;	// pass our instance to timer proc
	// start timer loop
	m_nTimer = SetTimer(NULL, 0, nTimerPeriod, TimerProc);	// start timer loop
}

CUndoTest::~CUndoTest()
{
}

void CUndoTest::Init()
{
	m_bInitRunning = false;
	m_pLogFile = NULL;
	m_pUndoMgr = NULL;
	m_nTimer = 0;
	m_nTimerPeriod = 0;
	m_nCycles = 0;
	m_nPasses = 0;
	m_nPassEdits = 0;
	m_nPassUndos = 0;
	m_nMaxEdits = 0;
	m_nRandSeed = 0;
	m_nState = STOP;
	m_nCyclesDone = 0;
	m_nPassesDone = 0;
	m_nEditsDone = 0;
	m_nUndosToDo = 0;
	m_nUndosDone = 0;
	m_nStepsDone = 0;
	m_nLastResult = FAIL;
	m_bInTimer = false;
	m_bMakeSnapshots = false;
	m_arrUndoCode.RemoveAll();
	m_sErrorMsg.Empty();
}

int CUndoTest::LogPrintf(LPCTSTR pszFormat, ...)
{
	if (m_pLogFile == NULL)
		return -1;
	va_list arglist;
	va_start(arglist, pszFormat);
	int	retc = _vftprintf(m_pLogFile, pszFormat, arglist);
	va_end(arglist);
	if (LOG_SAFE_MODE)
		fflush(m_pLogFile);
	return retc;
}

int CUndoTest::Random(int nVals)
{
	return CRandList::Rand(nVals);
}

W64INT CUndoTest::RandW64INT(W64INT nVals)
{
	if (nVals <= 0)
		return -1;
	W64INT	i = TruncW64INT(rand() / double(RAND_MAX) * nVals);
	return min(i, nVals - 1);
}

double CUndoTest::RandomFloat(double fLimit)
{
	return (double)rand() / RAND_MAX * fLimit;	// poor granularity but usable
}

int CUndoTest::RandomExcluding(int nVals, int nExcludeVal)
{
	if (nVals <= 1)
		return nVals - 1;
	int	val;
	do {
		val = Random(nVals);
	} while (val == nExcludeVal);
	return val;
}

int CUndoTest::RandomRange(CIntRange range)
{
	range.Normalize();
	return range.Start + Random(range.Length());
}

int CUndoTest::GetRandomEdit() const
{
	return m_arrUndoCode[Random(static_cast<int>(m_arrUndoCode.GetSize()))];
}

LONGLONG CUndoTest::Fletcher64(const void *pBuffer, DWORD nLength)
{
	DWORD	sum1 = 0, sum2 = 0;
	DWORD	scrap = INT_PTR(pBuffer) % sizeof(DWORD);
	if (scrap) {	// if buffer not dword-aligned
		scrap = sizeof(DWORD) - scrap;
		scrap = min(scrap, nLength);
		DWORD	val = 0;
		memcpy(&val, pBuffer, scrap);
		sum1 += val;
		sum2 += sum1;
		pBuffer = ((BYTE *)pBuffer) + scrap;
		nLength -= scrap;
	}
	// process full dwords
	DWORD	words = nLength / sizeof(DWORD);
	DWORD	*pWord = (DWORD *)pBuffer;
	for (DWORD i = 0; i < words; i++) {
		sum1 += pWord[i];
		sum2 += sum1;
	}
	scrap = nLength % sizeof(DWORD);
	if (scrap) {	// if any single bytes remain
		DWORD	val = 0;
		memcpy(&val, &pWord[words], scrap);
		sum1 += val;
		sum2 += sum1;
	}
	LARGE_INTEGER	result;
	result.LowPart = sum1;
	result.HighPart = sum2;
	return result.QuadPart;
}

LONGLONG CUndoTest::GetSnapshot() const
{
	return 0;
}

bool CUndoTest::Create()
{
	if (!m_dlgProgress.Create()) {	// create master progress dialog
		PRINTF(_T("can't create progress dialog\n"));
		return false;
	}
	if (!OpenClipboard(NULL) || !EmptyClipboard()) {
		PRINTF(_T("can't empty clipboard\n"));
		return false;
	}
	CloseClipboard();
	int	nSteps;
	nSteps = m_nPassEdits * (m_nPasses + 1);
	if (m_nPasses > 1)
		nSteps += (m_nPasses - 1) * (m_nPassEdits + m_nPassUndos * 2);
	nSteps *= m_nCycles;
	m_dlgProgress.SetRange(0, nSteps);
#if LOG_TO_FILE
	CString	sLogName;
	sLogName.Format(LOG_FILE_PATH,
		CTime::GetCurrentTime().Format(_T("_%Y_%m_%d_%H_%M_%S")));
	m_pLogFile = _tfopen(sLogName, _T("wc"));	// commit flag
#endif
	if (m_bMakeSnapshots) {
		m_arrSnapshot.RemoveAll();
		m_arrSnapshot.SetSize(m_nPassEdits * m_nPasses);
	}
	return true;
}

void CUndoTest::Destroy()
{
	if (m_pLogFile != NULL) {
		fclose(m_pLogFile);
		m_pLogFile = NULL;
	}
	Init();	// reset defaults
}

void CUndoTest::SetState(int nState)
{
	if (nState == m_nState)
		return;
	CString	s;
	s.Format(_T("Undo Test - Pass %d of %d - %s"), 
		m_nPassesDone + 1, m_nPasses, m_arrStateName[nState]);
	m_dlgProgress.SetWindowText(s);
	m_nState = nState;
}

bool CUndoTest::Run(bool bEnable)
{
	if (bEnable == IsRunning())
		return true;
	if (bEnable) {	// if running
		if (!Create())	// create instance
			return false;
		srand(m_nRandSeed);
		// build array of undo codes
		for (int iEdit = 0; iEdit < m_arrEditInfo.GetSize(); iEdit++) {
			// set probability of edits by duplicating them
			int	nDups = Round(m_arrEditInfo[iEdit].fProbability * 10);
			for (int iDup = 0; iDup < nDups; iDup++)
				m_arrUndoCode.Add(m_arrEditInfo[iEdit].nUndoCode);
		}
		SetState(EDIT);
	} else {	// stopping
		SetState(STOP);
		if (m_nLastResult == DONE) {	// if normal completion
			m_nCyclesDone++;
			if (m_nCyclesDone < m_nCycles) {	// if more cycles to go
				PRINTF(_T("cycle %d\n"), m_nCyclesDone);
				m_nPassesDone = 0;
				m_nEditsDone = 0;
				SetState(EDIT);	// resume editing
				return true;
			}
			// success: display test results
			m_dlgProgress.DestroyWindow();
			CString	s;
			bool	bPass = m_nPassesDone >= m_nPasses;
			s.Format(_T("UndoTest %s: seed=%d edits=%d passes=%d cycles=%d"),
				bPass ? _T("pass") : _T("FAIL"),
				m_nRandSeed, m_nEditsDone, m_nPassesDone, m_nCyclesDone);
			PRINTF(_T("%s\n"), s);
			AfxMessageBox(s, bPass ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
		} else {	// abnormal completion
			m_dlgProgress.DestroyWindow();
			if (m_nLastResult != CANCEL) {	// if not canceled
				if (m_sErrorMsg.IsEmpty())
					m_sErrorMsg = _T("Generic error.");
				AfxMessageBox(m_sErrorMsg);
			}
		}
		Destroy();	// destroy instance
	}
	return true;
}

VOID CALLBACK CUndoTest::TimerProc(HWND hwnd, UINT uMsg, W64UINT idEvent, DWORD dwTime)
{
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(idEvent);
	UNREFERENCED_PARAMETER(dwTime);
	m_pThis->OnTimer();
}

void CUndoTest::OnTimer()
{
	TRY {
		if (m_bInTimer)	// if already in OnTimer
			return;	// prevent reentrance
		m_bInTimer = true;
		if (m_bInitRunning) {
			Run(true);
			m_bInitRunning = false;
		}
		if (IsRunning()) {	// if test is running
			if (m_nTimerPeriod) {	// if using timer
				m_nLastResult = DoPass();	// do a test pass
				if (m_nLastResult != PASS)	// if it failed
					Run(false);	// stop test, or start another cycle
			} else {	// not using timer
				// run entire test from this function
				KillTimer(NULL, m_nTimer);	// kill timer
				m_nTimer = 0;
				while (IsRunning()) {
					// do test passes until failure occurs
					while ((m_nLastResult = DoPass()) == PASS) {
						// pump messages after each pass to keep UI responsive
						MSG msg;
						while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
					Run(false);	// stop test, or start another cycle
				}
			}
		}
	}
	CATCH (CUserException, e) {
		m_nLastResult = CANCEL;
		Run(false);	// abort test
	}
	CATCH (CException, e) {
		TCHAR	msg[256];
		e->GetErrorMessage(msg, _countof(msg));
		PRINTF(_T("%s\n"), msg);
		m_sErrorMsg = msg;
		m_nLastResult = FAIL;
		Run(false);	// abort test
	}
	END_CATCH;
	m_bInTimer = false;
}

int CUndoTest::DoPass()
{
	if (m_dlgProgress.Canceled())
		return CANCEL;
	switch (m_nState) {
	case EDIT:
		{
			int	retc = DISABLED;
			int	pos = m_pUndoMgr->GetPos();
			int	nTries = 0;
			while (nTries < INT_MAX) {
				if (m_bMakeSnapshots)
					m_arrSnapshot[m_nEditsDone] = GetSnapshot();	// update snapshot
				if (!nTries)
					PRINTF(_T("%d: "), m_nEditsDone);
				retc = ApplyEdit(GetRandomEdit());
				nTries++;
				if (retc == SUCCESS) {
					// if undo position didn't change, assume edit was coalesced
					if (m_pUndoMgr->GetPos() == pos) {
						nTries = 0;
						continue;	// do another edit
					}
					// if edit was insignificant
					if (!m_pUndoMgr->GetState(m_pUndoMgr->GetPos() - 1).IsSignificant()) {
						pos = m_pUndoMgr->GetPos();	// update cached undo position
						nTries = 0;
						continue;	// do another edit
					}
					break;	// success; exit loop
				}
				if (retc != DISABLED)	// if any result other than disabled
					break;	// failure; exit loop
			}
			switch (retc) {
			case SUCCESS:	// edit succeeded
				m_nEditsDone++;
				if (m_nEditsDone < m_nMaxEdits) {
					if (!(m_nEditsDone % m_nPassEdits)) {	// if undo boundary
						// for final pass, undo all the way back, else
						// do number of undos specified by m_nPassUndos
						m_nUndosToDo = LastPass() ? INT_MAX : m_nPassUndos;
						m_nUndosDone = 0;
						SetState(UNDO);	// start undoing
					}
				} else {	// too many edits
					m_sErrorMsg = _T("Too many edits.");
					return FAIL;
				}
				break;
			case DISABLED:	// too many retries
				m_sErrorMsg = _T("Too many retries.");
				return FAIL;
			case ABORT:	// edit failed
				if (m_sErrorMsg.IsEmpty()) {
					m_sErrorMsg.Format(_T("Error during edit."));
				}
				return FAIL;
			}
		}
		break;
	case UNDO:
		if (m_pUndoMgr->CanUndo() && m_nUndosDone < m_nUndosToDo) {
			PRINTF(_T("%d: Undo %s\n"), m_nUndosDone,
				m_pUndoMgr->GetUndoTitle());
			m_pUndoMgr->Undo();
			m_nUndosDone++;
			if (m_bMakeSnapshots) {
				LONGLONG	snap = GetSnapshot();
				int	iSnap = m_nEditsDone - m_nUndosDone;
				if (snap != m_arrSnapshot[iSnap]) {	// if snapshot mismatch
					m_sErrorMsg.Format(_T("Current state doesn't match snapshot %d\n%I64x != %I64x"), 
						iSnap, snap, m_arrSnapshot[iSnap]);
					return FAIL;
				}
			}
		} else {	// undos completed
			if (LastPass()) {
				m_nPassesDone++;
				return DONE;	// all passes completed
			} else {	// not final pass
				SetState(REDO);	// start redoing
			}
		}
		break;
	case REDO:
		if (m_pUndoMgr->CanRedo()) {
			PRINTF(_T("%d: Redo %s\n"), m_nUndosDone,
				m_pUndoMgr->GetRedoTitle());
			m_pUndoMgr->Redo();
			m_nUndosDone--;
		} else {	// redos completed
			m_nPassesDone++;
			SetState(EDIT);	// resume editing
		}
		break;
	}
	if (m_nState != STOP) {
		m_nStepsDone++;
		// access progress control directly to avoid pumping message
		CWnd	*pProgCtrl = m_dlgProgress.GetDlgItem(IDC_PROGRESS);
		pProgCtrl->SendMessage(PBM_SETPOS, m_nStepsDone);
	}
	return PASS;	// continue running
}

#endif
