// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01aug05	initial version

        disable and restore accessibility shortcuts
 
*/

#ifndef CNOACCESSKEYS_INCLUDED
#define CNOACCESSKEYS_INCLUDED

class CNoAccessKeys : public WObject {
public:
	CNoAccessKeys();
	~CNoAccessKeys();
	void	Restore();

private:
	STICKYKEYS	m_StickyKeys;
	FILTERKEYS	m_FilterKeys;
	TOGGLEKEYS	m_ToggleKeys;
};

#endif
