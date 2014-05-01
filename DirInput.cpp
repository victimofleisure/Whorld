// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		19jul05	initial version
		01		05aug05	add mouse acceleration
		02		28jan08	support Unicode
		03		29jan08	in GetState, add default case to fix warning

		wrap DirectInput for mouse and keyboard

*/

#include "stdafx.h"
#include "DirInput.h"

#define DIRINPUTERR(x) {x, _T(#x)},
const CDirInput::ERRTRAN CDirInput::m_ErrTran[] = {
#include "DirInputErrs.h"
{0, NULL}};

CDirInput::CDirInput()
{
	m_hr = 0;
	m_di = NULL;
	m_Keybd = NULL;
	m_Mouse = NULL;
	m_hWnd = NULL;
	m_Options = 0;
	m_Callback = NULL;
	m_Cookie = NULL;
	m_LaunchOK = FALSE;
	ZeroMemory(&m_MouseAccel, sizeof(m_MouseAccel));
}

CDirInput::~CDirInput()
{
	SetEvent(m_DieEvent);
	WaitForSingleObject(m_Thread, INFINITE);
	Destroy();
}

bool CDirInput::Create(HWND hWnd, int Options, int Priority, 
					  LPCALLBACK Callback, LPVOID Cookie)
{
	if (m_di != NULL || m_Thread != NULL || hWnd == NULL)
		return(FALSE);
	if ((Options & (OPT_MOUSE_NOTIFY | OPT_KEYBD_NOTIFY)) && Callback == NULL)
		return(FALSE);
	m_hWnd = hWnd;
	m_Options = Options;
	m_Callback = Callback;
	m_Cookie = Cookie;
	m_hr = DirectInputCreate(AfxGetApp()->m_hInstance,
		DIRECTINPUT_VERSION, &m_di, NULL); 
	if (FAILED(m_hr))
		return(FALSE);
	if (!m_MouseEvent.Create(NULL, FALSE, FALSE, NULL)
	|| !m_KeybdEvent.Create(NULL, FALSE, FALSE, NULL)
	|| !m_LaunchEvent.Create(NULL, FALSE, FALSE, NULL)
	|| !m_DieEvent.Create(NULL, FALSE, FALSE, NULL))
		return(FALSE);
	if (!m_Thread.Create(NULL, 0, ThreadFunc, this, CREATE_SUSPENDED, NULL))
		return(FALSE);
	if (!SetThreadPriority(m_Thread, Priority))
		return(FALSE);
	if (!ResumeThread(m_Thread))
		return(FALSE);
	if (WaitForSingleObject(m_LaunchEvent, 1000) != WAIT_OBJECT_0)
		return(FALSE);
	if ((Options & OPT_SYS_MOUSE_ACCEL) && !GetSystemMouseAccel())
		return(FALSE);
	return(m_LaunchOK);
}

void CDirInput::Destroy()
{
	if (m_di != NULL) {
		DestroyDevice(m_Keybd);
		DestroyDevice(m_Mouse);
		m_di->Release();
		m_di = NULL;
	}
}

bool CDirInput::CreateDevice(REFGUID guid, LPCDIDATAFORMAT fmt, 
						 LPDIRECTINPUTDEVICE& dev, HANDLE hEvent)
{
	DestroyDevice(dev);
	m_hr = m_di->CreateDevice(guid, &dev, NULL); 
	if (FAILED(m_hr))
		return(FALSE);
	m_hr = dev->SetDataFormat(fmt); 
	if (FAILED(m_hr))
		return(FALSE);
	m_hr = dev->SetCooperativeLevel(m_hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(m_hr))
		return(FALSE);
	if (hEvent != NULL) {
		m_hr = dev->SetEventNotification(hEvent);
		if (FAILED(m_hr))
			return(FALSE);
	}
	return(TRUE);
}

void CDirInput::DestroyDevice(LPDIRECTINPUTDEVICE& dev)
{
	if (dev != NULL) {
		dev->Unacquire();
		dev->Release();
		dev = NULL;
	}
}

bool CDirInput::CreateDevices()
{
	// create mouse and keyboard device objects, with optional notification
	HANDLE	ev;
	if (m_Options & OPT_MOUSE_ACQUIRE) {
		ev = (m_Options & OPT_MOUSE_NOTIFY) ? (HANDLE)m_MouseEvent : NULL;
		if (!CreateDevice(GUID_SysMouse, &c_dfDIMouse, m_Mouse, ev))
			return(FALSE);
		// set mouse axis mode to relative or absolute
		m_hr = SetProperty(m_Mouse, DIPROP_AXISMODE, 0, DIPH_DEVICE, 
			(m_Options & OPT_MOUSE_ABSOLUTE) ? DIPROPAXISMODE_ABS : DIPROPAXISMODE_REL);
		if (FAILED(m_hr))
			return(FALSE);
		m_hr = m_Mouse->Acquire();	// acquire mouse
		if (FAILED(m_hr))
			return(FALSE);
	}
	if (m_Options & OPT_KEYBD_ACQUIRE) {
		ev = (m_Options & OPT_KEYBD_NOTIFY) ? (HANDLE)m_KeybdEvent : NULL;
		if (!CreateDevice(GUID_SysKeyboard, &c_dfDIKeyboard, m_Keybd, ev))
			return(FALSE);
		m_hr = m_Keybd->Acquire();	// acquire keyboard
		if (FAILED(m_hr))
			return(FALSE);
	}
	return(TRUE);
}

void CDirInput::Main()
{
	m_LaunchOK = CreateDevices();	// must create background devices in worker thread
	SetEvent(m_LaunchEvent);	
	STATE	State;
	HRESULT hr;
	bool	more = TRUE;
	while (more) {
		HANDLE	ha[3] = {m_DieEvent, m_MouseEvent, m_KeybdEvent};
		int	retc = WaitForMultipleObjects(3, ha, FALSE, INFINITE);
		switch (retc) {
		case WAIT_OBJECT_0:
			more = FALSE;
			break;
		case WAIT_OBJECT_0 + 1:
			hr = m_Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &State.Mouse);
			if (SUCCEEDED(hr))
				m_Callback(DEV_MOUSE, State, m_Cookie);
			break;
		case WAIT_OBJECT_0 + 2:
			hr = m_Keybd->GetDeviceState(256, State.Keybd);
			if (SUCCEEDED(hr))
				m_Callback(DEV_KEYBD, State, m_Cookie);
			break;
		}
 	}
}

unsigned __stdcall CDirInput::ThreadFunc(LPVOID pParam)
{
	((CDirInput *)pParam)->Main();
	return(0);
}

LPCTSTR CDirInput::GetLastErrorString() const
{
	HRESULT	Code = GetLastError();
	for (int i = 0; m_ErrTran[i].Text != NULL; i++) {
		if (m_ErrTran[i].Code == Code)
			return(m_ErrTran[i].Text);
	}
	return(_T("unknown error"));
}

bool CDirInput::GetState(int Device, STATE& State)
{
	HRESULT	hr;
	switch (Device) {
	case DEV_MOUSE:
		hr = m_Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &State.Mouse);
		if (m_MouseAccel.Levels > 0) {	// first level of acceleration
			int	dx = abs(State.Mouse.lX);
			int	dy = abs(State.Mouse.lY);
			if (dx > m_MouseAccel.Thresh1)
				State.Mouse.lX *= 2;
			if (dy > m_MouseAccel.Thresh1)
				State.Mouse.lY *= 2;
			if (m_MouseAccel.Levels > 1) {	// second level of acceleration
				if (dx > m_MouseAccel.Thresh2)
					State.Mouse.lX *= 2;
				if (dy > m_MouseAccel.Thresh2)
					State.Mouse.lY *= 2;
			}
		}
		break;
	case DEV_KEYBD:
		hr = m_Keybd->GetDeviceState(256, State.Keybd);
		break;
	default:
		ASSERT(0);
		hr = 0;
	}
	return(SUCCEEDED(hr));
}

bool CDirInput::SetProperty(LPDIRECTINPUTDEVICE dev, REFGUID Property, 
						   DWORD Obj, DWORD How, DWORD Data)
{
	DIPROPDWORD	pw;
	ZeroMemory(&pw, sizeof(pw));
	pw.diph.dwSize = sizeof(DIPROPDWORD);
	pw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	pw.diph.dwHow = DIPH_DEVICE;
	pw.dwData = Data;
	HRESULT	hr = dev->SetProperty(Property, &pw.diph);
	return(SUCCEEDED(hr));
}

bool CDirInput::GetProperty(LPDIRECTINPUTDEVICE dev, REFGUID Property, 
						   DWORD Obj, DWORD How, DWORD& Data)
{
	DIPROPDWORD	pw;
	ZeroMemory(&pw, sizeof(pw));
	pw.diph.dwSize = sizeof(DIPROPDWORD);
	pw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	pw.diph.dwHow = DIPH_DEVICE;
	HRESULT	hr = dev->GetProperty(Property, &pw.diph);
	Data = pw.dwData;
	return(SUCCEEDED(hr));
}

bool CDirInput::SetMouseAccel(const MOUSE_ACCEL *Parms)
{
	if (Parms != NULL && !(m_Options & OPT_MOUSE_ABSOLUTE))
		m_MouseAccel = *Parms;
	else
		ZeroMemory(&m_MouseAccel, sizeof(m_MouseAccel));
	return(TRUE);
}

bool CDirInput::GetSystemMouseAccel()
{
	MOUSE_ACCEL	Parms;
	if (SystemParametersInfo(SPI_GETMOUSE, 0, Parms.a, 0))
		return(SetMouseAccel(&Parms));
	return(FALSE);
}
