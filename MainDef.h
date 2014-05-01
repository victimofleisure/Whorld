// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18apr06	initial version

        patch main member I/O list

*/

// Main members must be initialized in CPatch::m_MainDefaults, 
// and packed/unpacked in CMainFrame::GetMainInfo/SetMainInfo.
//
//		name			type
MAINDEF(Origin,			DPOINT)	// view origin, in normalized coordinates
MAINDEF(DrawMode,		INT)	// draw mode; see enum in CWhorldView
MAINDEF(OrgMotion,		INT)	// origin motion; see enum in CMainFrame
MAINDEF(Hue,			DOUBLE)	// current hue in degrees
MAINDEF(Mirror,			bool)	// true if mirroring
MAINDEF(Reverse,		bool)	// true if rings growing inward
MAINDEF(Convex,			bool)	// true if drawing in descending size order
MAINDEF(InvertColor,	bool)	// true if inverting color
MAINDEF(LoopHue,		bool)	// true if looping hue
MAINDEF(Pause,			bool)	// true if paused
MAINDEF(ZoomCenter,		bool)	// true if zoom is window-centered
