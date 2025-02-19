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
		int		UndoCode;		// undo code
		float	Probability;	// relative probability
	};

// Construction
	CUndoTest(bool InitRunning, int TimerPeriod, const EDIT_INFO *EditInfo, int EditInfoSize);
	virtual ~CUndoTest();

// Attributes
	bool	IsRunning() const;

// Operations
	bool	Run(bool InitRunning);

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
	static const LPCTSTR	m_StateName[STATES];

// Member data
	bool	m_InitRunning;		// true if initally running
	FILE	*m_LogFile;			// log file for test results
	CUndoManager	*m_UndoMgr;	// pointer to undo manager
	W64UINT	m_Timer;			// timer instance
	int		m_TimerPeriod;		// timer period, in milliseconds
	int		m_Cycles;			// number of test cycles
	int		m_Passes;			// total number of passes
	int		m_PassEdits;		// number of edits per pass
	int		m_PassUndos;		// number of undos per pass
	int		m_MaxEdits;			// maximum number of edits
	int		m_RandSeed;			// random number generator seed
	int		m_State;			// current state
	int		m_CyclesDone;		// number of cycles completed
	int		m_PassesDone;		// number of passes completed
	int		m_EditsDone;		// number of edits completed
	int		m_UndosDone;		// number of undos completed
	int		m_UndosToDo;		// number of undos to do
	int		m_StepsDone;		// number of steps completed
	int		m_LastResult;		// most recent pass result
	bool	m_InTimer;			// true if we're in OnTimer
	bool	m_MakeSnapshots;	// if true, create and test snapshots
	CDWordArray	m_UndoCode;		// array of undo codes
	CProgressDlg	m_ProgressDlg;	// progress dialog
	CString	m_ErrorMsg;			// error message
	CSnapshotArray	m_Snapshot;	// array of checksums
	CEditInfoArray	m_EditInfo;	// array of edit properties
	static	CUndoTest	*m_This;	// pointer to one and only instance

// Overridables
	virtual	bool	Create();
	virtual	void	Destroy();
	virtual	int		ApplyEdit(int UndoCode) = 0;
	virtual	LONGLONG	GetSnapshot() const;

// Helpers
	void	Init();
	int		LogPrintf(LPCTSTR Format, ...);
	static	int		Random(int Vals);
	static	W64INT	RandW64INT(W64INT Vals);
	static	double	RandomFloat(double Limit);
	static	int		RandomExcluding(int Vals, int ExcludeVal);
	static	int		RandomRange(CIntRange Range);
	static	LONGLONG	Fletcher64(const void *Buffer, DWORD Length);
	int		GetRandomEdit() const;
	bool	LastPass() const;
	void	SetState(int State);
	void	OnTimer();
	int		DoPass();
	static	VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, W64UINT idEvent, DWORD dwTime);
};

inline bool CUndoTest::IsRunning() const
{
	return(m_State != STOP);
}

inline bool CUndoTest::LastPass() const
{
	return(m_PassesDone >= m_Passes - 1);
}
