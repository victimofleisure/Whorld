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
		03		04mar25	modernize style

		automated undo test
 
*/

#pragma once

#include "ProgressDlg.h"
#include "ArrayEx.h"
#include "Range.h"
typedef CRange<int> CIntRange;

#define LOG_TO_FILE 0	// set non-zero to redirect log to a file
#define LOG_SAFE_MODE 0	// set non-zero to flush log after every write
#define QUIET_MODE 1	// suppress console natter
#if LOG_TO_FILE
#define PRINTF LogPrintf
#else
#if QUIET_MODE
#define PRINTF sizeof
#else
#define PRINTF _tprintf
#endif
#endif

class CUndoManager;

class CUndoTest : public WObject {
public:
// Types
	struct EDIT_INFO {
		int		nUndoCode;		// undo code
		float	fProbability;	// relative probability
	};

// Construction
	CUndoTest(bool bInitRunning, int nTimerPeriod, const EDIT_INFO *pEditInfo, int nEditInfoSize);
	virtual ~CUndoTest();

// Attributes
	bool	IsRunning() const;

// Operations
	bool	Run(bool bEnable);

protected:
// Types
	typedef CArrayEx<EDIT_INFO, EDIT_INFO&> CEditInfoArray;
	typedef CArrayEx<LONGLONG, LONGLONG> CSnapshotArray;

// Constants
	enum {	// command returns
		SUCCESS,
		DISABLED,
		ABORT,
	};
	enum {	// states
		STOP,
		EDIT,
		UNDO,
		REDO,
		STATES
	};
	enum {	// pass returns
		PASS,
		FAIL,
		DONE,
		CANCEL,
	};
	static const LPCTSTR	m_arrStateName[STATES];

// Member data
	bool	m_bInitRunning;		// true if initally running
	FILE	*m_pLogFile;		// log file for test results
	CUndoManager	*m_pUndoMgr;	// pointer to undo manager
	W64UINT	m_nTimer;			// timer instance
	int		m_nTimerPeriod;		// timer period, in milliseconds
	int		m_nCycles;			// number of test cycles
	int		m_nPasses;			// total number of passes
	int		m_nPassEdits;		// number of edits per pass
	int		m_nPassUndos;		// number of undos per pass
	int		m_nMaxEdits;		// maximum number of edits
	int		m_nRandSeed;		// random number generator seed
	int		m_nState;			// current state
	int		m_nCyclesDone;		// number of cycles completed
	int		m_nPassesDone;		// number of passes completed
	int		m_nEditsDone;		// number of edits completed
	int		m_nUndosDone;		// number of undos completed
	int		m_nUndosToDo;		// number of undos to do
	int		m_nStepsDone;		// number of steps completed
	int		m_nLastResult;		// most recent pass result
	bool	m_bInTimer;			// true if we're in OnTimer
	bool	m_bMakeSnapshots;	// if true, create and test snapshots
	CDWordArray	m_arrUndoCode;	// array of undo codes
	CProgressDlg	m_dlgProgress;	// progress dialog
	CString	m_sErrorMsg;			// error message
	CSnapshotArray	m_arrSnapshot;	// array of checksums
	CEditInfoArray	m_arrEditInfo;	// array of edit properties
	static	CUndoTest	*m_pThis;	// pointer to one and only instance

// Overridables
	virtual	bool	Create();
	virtual	void	Destroy();
	virtual	int		ApplyEdit(int nUndoCode) = 0;
	virtual	LONGLONG	GetSnapshot() const;

// Helpers
	void	Init();
	int		LogPrintf(LPCTSTR pszFormat, ...);
	static	int		Random(int nVals);
	static	W64INT	RandW64INT(W64INT nVals);
	static	double	RandomFloat(double fLimit);
	static	int		RandomExcluding(int nVals, int nExcludeVal);
	static	int		RandomRange(CIntRange range);
	static	LONGLONG	Fletcher64(const void *pBuffer, DWORD nLength);
	int		GetRandomEdit() const;
	bool	LastPass() const;
	void	SetState(int nState);
	void	OnTimer();
	int		DoPass();
	static	VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, W64UINT idEvent, DWORD dwTime);
};

inline bool CUndoTest::IsRunning() const
{
	return m_nState != STOP;
}

inline bool CUndoTest::LastPass() const
{
	return m_nPassesDone >= m_nPasses - 1;
}
