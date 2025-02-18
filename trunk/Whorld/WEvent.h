// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		15aug04	initial version

		wrap Win32 event object

*/

#ifndef WEVENT_INCLUDED
#define WEVENT_INCLUDED

class WEvent : public WObject {
public:
//
// Constructs an event.
//
	WEvent();
//
// Destroys the event.
//
	~WEvent();
//
// Creates the event.
//
// Returns: True if successful.
//
	bool	Create(
		LPSECURITY_ATTRIBUTES lpEventAttributes,	// Pointer to security attributes.
		BOOL	bManualReset,		// Flag for manual-reset event.
		BOOL	bInitialState,		// Flag for initial state.
		LPCTSTR lpName				// Pointer to event-object name.
	);
//
// Closes the event handle.
//
	void	Close();
//
// Sets the event.
//
	void	Set();
//
// Resets the event.
//
	void	Reset();
//
// Retrieves the event's handle.
//
// Returns: The handle.
//
	operator HANDLE() const;

private:
	HANDLE	m_Event;			// The event handle.
};

inline void WEvent::Set()
{
	SetEvent(m_Event);
}

inline void WEvent::Reset()
{
	ResetEvent(m_Event);
}

inline WEvent::operator HANDLE() const
{
	return(m_Event);
}

#endif
