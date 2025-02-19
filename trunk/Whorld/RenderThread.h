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

#pragma once

#include "D2DDevCtx.h"
#include "ILockRingBuf.h"

class CRenderThreadBase {
public:
	enum {	// base render commands
		RC_RESIZE,				// Param: none; Data: none
		RC_SET_FULLSCREEN,		// Param: BOOL; Data: none
		BASE_RENDER_COMMANDS	// start of derived class commands
	};
	union VARIANT_PROP {	// subset of VARIANT types
		bool	boolVal;
		char	cVal;
		BYTE	bVal;
		short	iVal;
		USHORT	uiVal;
		int		intVal;
		UINT	uintVal;
		float	fltVal;
		double	dblVal;
		LONGLONG	llVal;
		ULONGLONG	ullVal;
		PVOID	byref;
		POINTFLOAT	fltPt;	// pair of float; non-standard
	};
	class CRenderCmd {
	public:
		CRenderCmd() {}
		CRenderCmd(UINT nCmd, UINT nParam = 0)
			: m_nCmd(nCmd), m_nParam(nParam) { m_prop.llVal = 0; }
		UINT	m_nCmd;			// render command; see enum
		UINT	m_nParam;		// optional parameter
		VARIANT_PROP	m_prop;	// variant property
	};
};

class CRenderThread : protected CD2DDevCtx, public CRenderThreadBase {
public:
// Construction
	CRenderThread();
	virtual ~CRenderThread();
	bool	CreateThread(HWND hWnd);
	void	DestroyThread();

// Operations
	bool	PushCommand(const CRenderCmd& cmd);

protected:
// Constants
	enum {
		COMMAND_QUEUE_SIZE = 1024,
	};

// Member data
	CILockRingBuf<CRenderCmd>	m_qCmd;	// command queue as interlocked ring buffer
	CWinThread	*m_pThread;		// pointer to render thread
	BOOL	m_bThreadExit;		// true if thread should exit
	HWND	m_hRenderWnd;		// handle to render window

// Overrideables
	virtual bool	OnThreadCreate();
	virtual bool	OnDraw();
	virtual void	OnRenderCommand(const CRenderCmd& cmd);
	virtual void	Log(CString sMsg);

// Helpers
	UINT	ThreadFunc();
	static UINT	ThreadFunc(LPVOID pParam);
	bool	RenderFrame();
	bool	HandleDeviceLost();
	void	ProcessCommand(const CRenderCmd& cmd);
	bool	LogDeviceInfo();
	bool	SetFullScreen(bool bEnable);
};

inline bool CRenderThread::PushCommand(const CRenderCmd& cmd)
{
	return m_qCmd.Push(cmd);
}
