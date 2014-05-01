// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18apr06	initial version
		01		21jun06	add tag
		02		24jun06	add Copies and Spread

        patch master member I/O list

*/

// Master members must be initialized in CPatch::m_MasterDefaults,
// and packed/unpacked in CMainFrame::GetMasterInfo/SetMasterInfo.
//
//		  tag		name			type
MASTERDEF(SPEED,	Speed,			DOUBLE)	// master speed, as a fraction; 1 = nominal
MASTERDEF(ZOOM,		Zoom,			DOUBLE)	// zoom, as a fraction; 1 = nominal
MASTERDEF(DAMPING,	Damping,		DOUBLE)	// origin/zoom damping; 1 = none, 0 = stuck
MASTERDEF(TRAIL,	Trail,			DOUBLE)	// origin trail; 0 = none, 1 = stuck
MASTERDEF(RINGS,	Rings,			DOUBLE)	// maximum number of rings
MASTERDEF(TEMPO,	Tempo,			DOUBLE)	// tempo, in BMP
MASTERDEF(HUE_LOOP,	HueLoopLength,	DOUBLE)	// if looping hue, loop length in degrees
MASTERDEF(CANVAS,	CanvasScale,	DOUBLE)	// canvas size, as fraction of window size
MASTERDEF(COPIES,	Copies,			DOUBLE)	// number of instances
MASTERDEF(SPREAD,	Spread,			DOUBLE)	// radius of instance origins, in pixels
