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

		automated undo test
 
*/

#include "stdafx.h"

#if UNDO_TEST

#include "Resource.h"
#include "UndoTest.h"
#include "RandList.h"
#include "UndoManager.h"

CUndoTest	*CUndoTest::m_This;	// pointer to instance

const LPCTSTR CUndoTest::m_StateName[STATES] = {
	_T("Stop"), 
	_T("Edit"), 
	_T("Undo"), 
	_T("Redo"),
};

#define LOG_FILE_PATH _T("UndoTest%s.log")

CUndoTest::CUndoTest(bool InitRunning, int TimerPeriod, const EDIT_INFO *EditInfo, int EditInfoSize)
{
	Init();
	m_InitRunning = InitRunning;
	m_TimerPeriod = TimerPeriod;
	// copy edit properties
	m_EditInfo.SetSize(EditInfoSize);
	for (int iInfo = 0; iInfo < EditInfoSize; iInfo++)
		m_EditInfo[iInfo] = EditInfo[iInfo];
	ASSERT(m_This == NULL);
	m_This = this;	// pass our instance to timer proc
	// start timer loop
	m_Timer = SetTimer(NULL, 0, TimerPeriod, TimerProc);	// start timer loop
}

CUndoTest::~CUndoTest()
{
}

void CUndoTest::Init()
{
	m_InitRunning = FALSE;
	m_LogFile = NULL;
	m_UndoMgr = NULL;
	m_Timer = 0;
	m_TimerPeriod = 0;
	m_Cycles = 0;
	m_Passes = 0;
	m_PassEdits = 0;
	m_PassUndos = 0;
	m_MaxEdits = 0;
	m_RandSeed = 0;
	m_State = STOP;
	m_CyclesDone = 0;
	m_PassesDone = 0;
	m_EditsDone = 0;
	m_UndosToDo = 0;
	m_UndosDone = 0;
	m_StepsDone = 0;
	m_LastResult = FAIL;
	m_InTimer = FALSE;
	m_MakeSnapshots = FALSE;
	m_UndoCode.RemoveAll();
	m_ErrorMsg.Empty();
}

int CUndoTest::LogPrintf(LPCTSTR Format, ...)
{
	if (m_LogFile == NULL)
		return(-1);
	va_list arglist;
	va_start(arglist, Format);
	int	retc = _vftprintf(m_LogFile, Format, arglist);
	va_end(arglist);
	if (LOG_SAFE_MODE)
		fflush(m_LogFile);
	return(retc);
}

int CUndoTest::Random(int Vals)
{
	return(CRandList::Rand(Vals));
}

W64INT CUndoTest::RandW64INT(W64INT Vals)
{
	if (Vals <= 0)
		return(-1);
	W64INT	i = TruncW64INT(rand() / double(RAND_MAX) * Vals);
	return(min(i, Vals - 1));
}

double CUndoTest::RandomFloat(double Limit)
{
	return((double)rand() / RAND_MAX * Limit);	// poor granularity but usable
}

int CUndoTest::RandomExcluding(int Vals, int ExcludeVal)
{
	if (Vals <= 1)
		return(Vals - 1);
	int	val;
	do {
		val = Random(Vals);
	} while (val == ExcludeVal);
	return(val);
}

int CUndoTest::RandomRange(CIntRange Range)
{
	Range.Normalize();
	return(Range.Start + Random(Range.Length()));
}

int CUndoTest::GetRandomEdit() const
{
	return(m_UndoCode[Random(static_cast<int>(m_UndoCode.GetSize()))]);
}

LONGLONG CUndoTest::Fletcher64(const void *Buffer, DWORD Length)
{
	DWORD	sum1 = 0, sum2 = 0;
	DWORD	scrap = INT_PTR(Buffer) % sizeof(DWORD);
	if (scrap) {	// if buffer not dword-aligned
		scrap = sizeof(DWORD) - scrap;
		scrap = min(scrap, Length);
		DWORD	val = 0;
		memcpy(&val, Buffer, scrap);
		sum1 += val;
		sum2 += sum1;
		Buffer = ((BYTE *)Buffer) + scrap;
		Length -= scrap;
	}
	// process full dwords
	DWORD	words = Length / sizeof(DWORD);
	DWORD	*pWord = (DWORD *)Buffer;
	for (DWORD i = 0; i < words; i++) {
		sum1 += pWord[i];
		sum2 += sum1;
	}
	scrap = Length % sizeof(DWORD);
	if (scrap) {	// if any single bytes remain
		DWORD	val = 0;
		memcpy(&val, &pWord[words], scrap);
		sum1 += val;
		sum2 += sum1;
	}
	LARGE_INTEGER	result;
	result.LowPart = sum1;
	result.HighPart = sum2;
	return(result.QuadPart);
}

LONGLONG CUndoTest::GetSnapshot() const
{
	return(0);
}

bool CUndoTest::Create()
{
	if (!m_ProgressDlg.Create()) {	// create master progress dialog
		PRINTF(_T("can't create progress dialog\n"));
		return(FALSE);
	}
	if (!OpenClipboard(NULL) || !EmptyClipboard()) {
		PRINTF(_T("can't empty clipboard\n"));
		return(FALSE);
	}
	CloseClipboard();
	int	Steps;
	Steps = m_PassEdits * (m_Passes + 1);
	if (m_Passes > 1)
		Steps += (m_Passes - 1) * (m_PassEdits + m_PassUndos * 2);
	Steps *= m_Cycles;
	m_ProgressDlg.SetRange(0, Steps);
#if LOG_TO_FILE
	CString	LogName;
	LogName.Format(LOG_FILE_PATH,
		CTime::GetCurrentTime().Format(_T("_%Y_%m_%d_%H_%M_%S")));
	m_LogFile = _tfopen(LogName, _T("wc"));	// commit flag
#endif
	if (m_MakeSnapshots) {
		m_Snapshot.RemoveAll();
		m_Snapshot.SetSize(m_PassEdits * m_Passes);
	}
	return(TRUE);
}

void CUndoTest::Destroy()
{
	if (m_LogFile != NULL) {
		fclose(m_LogFile);
		m_LogFile = NULL;
	}
	Init();	// reset defaults
}

void CUndoTest::SetState(int State)
{
	if (State == m_State)
		return;
	CString	s;
	s.Format(_T("Undo Test - Pass %d of %d - %s"), 
		m_PassesDone + 1, m_Passes, m_StateName[State]);
	m_ProgressDlg.SetWindowText(s);
	m_State = State;
}

bool CUndoTest::Run(bool Enable)
{
	if (Enable == IsRunning())
		return(TRUE);
	if (Enable) {	// if running
		if (!Create())	// create instance
			return(FALSE);
		srand(m_RandSeed);
		// build array of undo codes
		for (int i = 0; i < m_EditInfo.GetSize(); i++) {
			// set probability of edits by duplicating them
			int	dups = Round(m_EditInfo[i].Probability * 10);
			for (int j = 0; j < dups; j++)
				m_UndoCode.Add(m_EditInfo[i].UndoCode);
		}
		SetState(EDIT);
	} else {	// stopping
		SetState(STOP);
		if (m_LastResult == DONE) {	// if normal completion
			m_CyclesDone++;
			if (m_CyclesDone < m_Cycles) {	// if more cycles to go
				PRINTF(_T("cycle %d\n"), m_CyclesDone);
				m_PassesDone = 0;
				m_EditsDone = 0;
				SetState(EDIT);	// resume editing
				return(TRUE);
			}
			// success: display test results
			m_ProgressDlg.DestroyWindow();
			CString	s;
			bool	pass = m_PassesDone >= m_Passes;
			s.Format(_T("UndoTest %s: seed=%d edits=%d passes=%d cycles=%d"),
				pass ? _T("pass") : _T("FAIL"),
				m_RandSeed, m_EditsDone, m_PassesDone, m_CyclesDone);
			PRINTF(_T("%s\n"), s);
			AfxMessageBox(s, pass ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
		} else {	// abnormal completion
			m_ProgressDlg.DestroyWindow();
			if (m_LastResult != CANCEL) {	// if not canceled
				if (m_ErrorMsg.IsEmpty())
					m_ErrorMsg = _T("Generic error.");
				AfxMessageBox(m_ErrorMsg);
			}
		}
		Destroy();	// destroy instance
	}
	return(TRUE);
}

VOID CALLBACK CUndoTest::TimerProc(HWND hwnd, UINT uMsg, W64UINT idEvent, DWORD dwTime)
{
	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(idEvent);
	UNREFERENCED_PARAMETER(dwTime);
	m_This->OnTimer();
}

void CUndoTest::OnTimer()
{
	TRY {
		if (m_InTimer)	// if already in OnTimer
			return;	// prevent reentrance
		m_InTimer = TRUE;
		if (m_InitRunning) {
			Run(TRUE);
			m_InitRunning = FALSE;
		}
		if (IsRunning()) {	// if test is running
			if (m_TimerPeriod) {	// if using timer
				m_LastResult = DoPass();	// do a test pass
				if (m_LastResult != PASS)	// if it failed
					Run(FALSE);	// stop test, or start another cycle
			} else {	// not using timer
				// run entire test from this function
				KillTimer(NULL, m_Timer);	// kill timer
				m_Timer = 0;
				while (IsRunning()) {
					// do test passes until failure occurs
					while ((m_LastResult = DoPass()) == PASS) {
						// pump messages after each pass to keep UI responsive
						MSG msg;
						while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
					Run(FALSE);	// stop test, or start another cycle
				}
			}
		}
	}
	CATCH (CUserException, e) {
		m_LastResult = CANCEL;
		Run(FALSE);	// abort test
	}
	CATCH (CException, e) {
		TCHAR	msg[256];
		e->GetErrorMessage(msg, _countof(msg));
		PRINTF(_T("%s\n"), msg);
		m_ErrorMsg = msg;
		m_LastResult = FAIL;
		Run(FALSE);	// abort test
	}
	END_CATCH;
	m_InTimer = FALSE;
}

int CUndoTest::DoPass()
{
	if (m_ProgressDlg.Canceled())
		return(CANCEL);
	switch (m_State) {
	case EDIT:
		{
			int	retc = DISABLED;
			int	pos = m_UndoMgr->GetPos();
			int	tries = 0;
			while (tries < INT_MAX) {
				if (m_MakeSnapshots)
					m_Snapshot[m_EditsDone] = GetSnapshot();	// update snapshot
				if (!tries)
					PRINTF(_T("%d: "), m_EditsDone);
				retc = ApplyEdit(GetRandomEdit());
				tries++;
				if (retc == SUCCESS) {
					// if undo position didn't change, assume edit was coalesced
					if (m_UndoMgr->GetPos() == pos) {
						tries = 0;
						continue;	// do another edit
					}
					// if edit was insignificant
					if (!m_UndoMgr->GetState(m_UndoMgr->GetPos() - 1).IsSignificant()) {
						pos = m_UndoMgr->GetPos();	// update cached undo position
						tries = 0;
						continue;	// do another edit
					}
					break;	// success; exit loop
				}
				if (retc != DISABLED)	// if any result other than disabled
					break;	// failure; exit loop
			}
			switch (retc) {
			case SUCCESS:	// edit succeeded
				m_EditsDone++;
				if (m_EditsDone < m_MaxEdits) {
					if (!(m_EditsDone % m_PassEdits)) {	// if undo boundary
						// for final pass, undo all the way back, else
						// do number of undos specified by m_PassUndos
						m_UndosToDo = LastPass() ? INT_MAX : m_PassUndos;
						m_UndosDone = 0;
						SetState(UNDO);	// start undoing
					}
				} else {	// too many edits
					m_ErrorMsg = _T("Too many edits.");
					return(FAIL);
				}
				break;
			case DISABLED:	// too many retries
				m_ErrorMsg = _T("Too many retries.");
				return(FAIL);
			case ABORT:	// edit failed
				if (m_ErrorMsg.IsEmpty()) {
					m_ErrorMsg.Format(_T("Error during edit."));
				}
				return(FAIL);
			}
		}
		break;
	case UNDO:
		if (m_UndoMgr->CanUndo() && m_UndosDone < m_UndosToDo) {
			PRINTF(_T("%d: Undo %s\n"), m_UndosDone,
				m_UndoMgr->GetUndoTitle());
			m_UndoMgr->Undo();
			m_UndosDone++;
			if (m_MakeSnapshots) {
				LONGLONG	snap = GetSnapshot();
				int	iSnap = m_EditsDone - m_UndosDone;
				if (snap != m_Snapshot[iSnap]) {	// if snapshot mismatch
					m_ErrorMsg.Format(_T("Current state doesn't match snapshot %d\n%I64x != %I64x"), 
						iSnap, snap, m_Snapshot[iSnap]);
					return(FAIL);
				}
			}
		} else {	// undos completed
			if (LastPass()) {
				m_PassesDone++;
				return(DONE);	// all passes completed
			} else {	// not final pass
				SetState(REDO);	// start redoing
			}
		}
		break;
	case REDO:
		if (m_UndoMgr->CanRedo()) {
			PRINTF(_T("%d: Redo %s\n"), m_UndosDone,
				m_UndoMgr->GetRedoTitle());
			m_UndoMgr->Redo();
			m_UndosDone--;
		} else {	// redos completed
			m_PassesDone++;
			SetState(EDIT);	// resume editing
		}
		break;
	}
	if (m_State != STOP) {
		m_StepsDone++;
		// access progress control directly to avoid pumping message
		CWnd	*pProgCtrl = m_ProgressDlg.GetDlgItem(IDC_PROGRESS);
		pProgCtrl->SendMessage(PBM_SETPOS, m_StepsDone);
	}
	return(PASS);	// continue running
}

#endif
