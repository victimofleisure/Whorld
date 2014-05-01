// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17oct05	initial version

        read/write keyboard LEDs
 
*/

#ifndef CKEYBOARDLEDS_INCLUDED
#define CKEYBOARDLEDS_INCLUDED

class CKeyboardLeds : public WObject {
public:
	enum {	// windows constants, don't change
		CAPS_LOCK	= 0x04,
		NUM_LOCK	= 0x02,
		SCROLL_LOCK	= 0x01
	};
	CKeyboardLeds();
	~CKeyboardLeds();
	// The synchronous wait is 6 milliseconds, during which Windows presumably
	// blocks while the PS/2 serial transfers complete. Using overlapped I/O
	// gets the wait down to 10-30 microseconds, but the caller is responsible
	// for calling Set at a reasonable rate, otherwise collisions may occur.
	bool	Open(bool Overlapped);
	bool	IsOpen() const;
	bool	Close();
	bool	Set(UINT mask);
	bool	Get(UINT& mask);

private:
	typedef struct _KEYBOARD_INDICATOR_PARAMETERS {
		USHORT UnitId;		// Unit identifier.
		USHORT LedFlags;	// LED indicator state.
	} KEYBOARD_INDICATOR_PARAMETERS, *PKEYBOARD_INDICATOR_PARAMETERS;

	HANDLE	m_Kbd;
	OVERLAPPED	m_Ovl;
};

inline bool CKeyboardLeds::IsOpen() const
{
	return(m_Kbd != INVALID_HANDLE_VALUE);
}

#endif
