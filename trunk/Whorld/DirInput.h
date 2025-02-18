// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		19jul05	initial version
		01		28jan08	support Unicode

		wrap DirectInput for mouse and keyboard

*/

#ifndef CDIRINPUT_INCLUDED
#define CDIRINPUT_INCLUDED

#include "dinput.h"
#include "WEvent.h"
#include "WThread.h"

class CDirInput : public WObject {
public:
// Constants
	enum {
		DEV_MOUSE,
		DEV_KEYBD
	};
	enum {
		OPT_MOUSE_ACQUIRE	= 0x01,	// acquire the mouse device
		OPT_MOUSE_NOTIFY	= 0x02,	// notify of mouse events via callback
		OPT_MOUSE_ABSOLUTE	= 0x04,	// use absolute mouse coordinates
		OPT_SYS_MOUSE_ACCEL	= 0x08,	// get mouse acceleration from system
		OPT_KEYBD_ACQUIRE	= 0x10,	// acquire the keyboard device
		OPT_KEYBD_NOTIFY	= 0x12,	// notify of keyboard events via callback
	};
	enum {
		MAX_KEYS = 256
	};

// Types
	typedef union tagSTATE {
		DIMOUSESTATE	Mouse;
		char	Keybd[MAX_KEYS];
	} STATE;
	typedef union tagMOUSE_ACCEL {
		int		a[3];	// windows-style, see mouse_event in MSDN
		struct {
			int		Thresh1;	// distance threshold for first doubling
			int		Thresh2;	// distance threshold for second doubling
			int		Levels;		// number of doubling tests, from 0..2
		};
	} MOUSE_ACCEL;

// Construction
	typedef	void (*LPCALLBACK)(int Device, const STATE& State, LPVOID Cookie);
	CDirInput();
	~CDirInput();
	bool	Create(HWND hWnd, int Options, int Priority, 
		LPCALLBACK Callback, LPVOID Cookie);
	void	Destroy();

// Operations
	HRESULT	GetLastError() const;
	LPCTSTR	GetLastErrorString() const;
	bool	GetState(int Device, STATE& State);
	bool	SetMouseAccel(const MOUSE_ACCEL *Parms);
	bool	GetSystemMouseAccel();
	static	bool	KeyDown(const STATE& State, int KeyCode);

private:
// Types
	typedef	struct tagERRTRAN {
		HRESULT	Code;
		LPCTSTR	Text;
	} ERRTRAN;

// Constants
	static	const	ERRTRAN	m_ErrTran[];	// map DirectInput error codes to text

// Member data
	WEvent	m_MouseEvent;	// if mouse notify, set when mouse event occurs
	WEvent	m_KeybdEvent;	// if keyboard notify, set when keyboard event occurs
	WEvent	m_LaunchEvent;	// set when thread is launched and devices are created
	WEvent	m_DieEvent;		// forces thread to exit
	WThread	m_Thread;		// thread that waits for input events
	HRESULT	m_hr;			// most recent DirectInput result code
	LPDIRECTINPUT  m_di;	// pointer to DirectInput instance
	LPDIRECTINPUTDEVICE  m_Keybd;	// pointer to keyboard device instance
	LPDIRECTINPUTDEVICE  m_Mouse;	// pointer to mouse device instance
	HWND	m_hWnd;			// parent window handle
	DWORD	m_Options;		// option bitmask; see enum above
	LPCALLBACK	m_Callback;	// pointer to user notification function; may be null
	LPVOID	m_Cookie;		// user-defined data passed to notification function
	bool	m_LaunchOK;		// true if thread successfully created devices
	MOUSE_ACCEL	m_MouseAccel;	// mouse acceleration parameters

// Helpers
	static	unsigned __stdcall ThreadFunc(LPVOID pParam);
	void	Main();
	bool	CreateDevice(REFGUID guid, LPCDIDATAFORMAT fmt,
		LPDIRECTINPUTDEVICE& dev, HANDLE hEvent);
	bool	CreateDevices();
	void	DestroyDevice(LPDIRECTINPUTDEVICE& dev);
	bool	SetProperty(LPDIRECTINPUTDEVICE dev, REFGUID Property, 
		DWORD Obj, DWORD How, DWORD Data);
	bool	GetProperty(LPDIRECTINPUTDEVICE dev, REFGUID Property, 
		DWORD Obj, DWORD How, DWORD& Data);
};

inline HRESULT CDirInput::GetLastError() const
{
	return(m_hr);
}

inline bool CDirInput::KeyDown(const STATE& State, int KeyCode)
{
	ASSERT(KeyCode >= 0 && KeyCode < MAX_KEYS);
	return((State.Keybd[KeyCode] & 0x80) != 0);
}

#endif
