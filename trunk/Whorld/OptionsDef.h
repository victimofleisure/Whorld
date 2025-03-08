// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27mar18	initial version
		01		21feb25	customize for Whorld
		02		26feb25	add MIDI input
		03		28feb25	add general group
		04      09mar25	add export scaling types

*/

#ifdef GROUPDEF

GROUPDEF(	Midi		)
GROUPDEF(	Export		)
GROUPDEF(	General		)

#undef GROUPDEF
#endif

#ifdef PROPDEF

//			group		subgroup	proptype	type		name				initval		minval		maxval		itemname	items
PROPDEF(	Midi,		NONE,		ENUM,		int,		iInputDevice,		0,			0,			0,			NULL,		0)
PROPDEF(	Export,		NONE,		FOLDER,		CString,	sImageFolder,		"",			"",			"",			NULL,		0)
PROPDEF(	Export,		NONE,		VAR,		UINT,		nImageWidth,		1920,		1,			UINT_MAX,	NULL,		0)
PROPDEF(	Export,		NONE,		VAR,		UINT,		nImageHeight,		1080,		1,			UINT_MAX,	NULL,		0)
PROPDEF(	Export,		NONE,		VAR,		bool,		bUseViewSize,		1,			0,			0,			NULL,		0)
PROPDEF(	Export,		NONE,		ENUM,		UINT,		nScalingType,		0,			0,			0,			m_oiScalingType,	SCALING_TYPES)
PROPDEF(	Export,		NONE,		VAR,		bool,		bPromptUser,		1,			0,			0,			NULL,		0)
PROPDEF(	General,	NONE,		VAR,		int,		nMRUItems,			4,			0,			16,			NULL,		0)

#undef PROPDEF
#endif

#ifdef SCALINGTYPEDEF

SCALINGTYPEDEF(None)
SCALINGTYPEDEF(FitWidth)
SCALINGTYPEDEF(FitHeight)
SCALINGTYPEDEF(FitBoth)

#undef SCALINGTYPEDEF
#endif
