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
		RC_SET_FULLSCREEN,		// Param: bool; Data: none
		BASE_RENDER_COMMANDS	// start of derived class commands
	};
	union VARIANT_PROP {	// subset of VARIANT types that fit in 8 bytes
		LONGLONG	llVal;	// first so initializer list sets all bytes
		ULONGLONG	ullVal;
		bool	boolVal;
		char	cVal;
		BYTE	bVal;
		short	iVal;
		USHORT	uiVal;
		int		intVal;
		UINT	uintVal;
		float	fltVal;
		double	dblVal;
		PVOID	byref;
		POINT	ptVal;
		SIZE	szVal;
		POINTFLOAT	fltPt;	// pair of float; non-standard
		float	floatVal;	// alias, useful in macros
		double	doubleVal;	// alias, useful in macros
	};
	typedef CArrayEx<VARIANT_PROP, VARIANT_PROP> CVariantPropArray;
	class CRenderCmd {
	public:
		CRenderCmd() {}
		CRenderCmd(UINT nCmd, UINT nParam = 0);
		CRenderCmd(UINT nCmd, UINT nParam, int& iProp);
		CRenderCmd(UINT nCmd, UINT nParam, UINT& uProp);
		CRenderCmd(UINT nCmd, UINT nParam, double& fProp);
		CRenderCmd(UINT nCmd, UINT nParam, bool& bProp);
		CRenderCmd(UINT nCmd, UINT nParam, LONGLONG& iProp);
		UINT	m_nCmd;			// render command; see enum
		UINT	m_nParam;		// optional parameter
		VARIANT_PROP	m_prop;	// variant property
	};
};

inline CRenderThreadBase::CRenderCmd::CRenderCmd(UINT nCmd, UINT nParam)
	: m_nCmd(nCmd), m_nParam(nParam)
{
	m_prop.llVal = 0;
}

inline CRenderThreadBase::CRenderCmd::CRenderCmd(UINT nCmd, UINT nParam, int& iProp)
	: m_nCmd(nCmd), m_nParam(nParam)
{
	m_prop.intVal = iProp;
}

inline CRenderThreadBase::CRenderCmd::CRenderCmd(UINT nCmd, UINT nParam, UINT& uProp)
	: m_nCmd(nCmd), m_nParam(nParam)
{
	m_prop.uintVal = uProp;
}

inline CRenderThreadBase::CRenderCmd::CRenderCmd(UINT nCmd, UINT nParam, double& fProp)
	: m_nCmd(nCmd), m_nParam(nParam)
{
	m_prop.dblVal = fProp;
}

inline CRenderThreadBase::CRenderCmd::CRenderCmd(UINT nCmd, UINT nParam, bool& bProp)
	: m_nCmd(nCmd), m_nParam(nParam)
{
	m_prop.boolVal = bProp;
}

inline CRenderThreadBase::CRenderCmd::CRenderCmd(UINT nCmd, UINT nParam, LONGLONG& iProp)
	: m_nCmd(nCmd), m_nParam(nParam)
{
	m_prop.llVal = iProp;
}

class CRenderThread : protected CD2DDevCtx, public CRenderThreadBase {
public:
// Construction
	CRenderThread();
	virtual ~CRenderThread();
	bool	CreateCommandQueue(int nCmdQueueSize);
	bool	CreateThread(HWND hRenderWnd, CWnd* pNotifyWnd = NULL);
	void	DestroyThread();

// Constants
	enum {
		COMMAND_QUEUE_SIZE = 1024,	// default size
	};

// Attributes
	bool	IsCommandQueueEmpty() const;
	bool	IsCommandQueueFull() const;
	bool	IsCommandQueueBelowHalfFull() const;

// Commands
	bool	Resize();
	bool	SetFullScreen(bool bEnable);

// Operations
	bool	PushCommand(const CRenderCmd& cmd);

protected:
// Member data
	CILockRingBuf<CRenderCmd>	m_qCmd;	// command queue as interlocked ring buffer
	CWinThread	*m_pThread;		// pointer to render thread
	BOOL	m_bThreadExit;		// true if thread should exit
	HWND	m_hRenderWnd;		// handle of render window
	CWnd*	m_pNotifyWnd;		// pointer to window that receives notifications

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
	bool	OnSetFullScreen(bool bEnable);
	BOOL	PostMsgToMainWnd(UINT nMsg, WPARAM wParam = 0, LPARAM lParam = 0) const;
};

inline bool CRenderThread::PushCommand(const CRenderCmd& cmd)
{
	return m_qCmd.Push(cmd);
}

inline bool CRenderThread::Resize()
{
	return PushCommand(CRenderCmd(RC_RESIZE));
}

inline bool CRenderThread::SetFullScreen(bool bEnable)
{
	return PushCommand(CRenderCmd(RC_SET_FULLSCREEN, bEnable));
}

inline bool CRenderThread::IsCommandQueueEmpty() const
{
	return m_qCmd.IsEmpty();
}

inline bool CRenderThread::IsCommandQueueFull() const
{
	return m_qCmd.IsFull();
}

inline bool CRenderThread::IsCommandQueueBelowHalfFull() const
{
	return m_qCmd.IsBelowHalfFull();
}
