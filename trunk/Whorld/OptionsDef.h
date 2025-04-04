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
		04      09mar25	add export scale to fit
		05      11mar25	add legacy snapshot sizes
		06      12mar25	add wheel sensitivity

*/

#ifdef GROUPDEF

GROUPDEF(	Midi		)
GROUPDEF(	Input		)
GROUPDEF(	Export		)
GROUPDEF(	Snapshot	)
GROUPDEF(	General		)

#undef GROUPDEF
#endif

#ifdef PROPDEF

//			group		subgroup	proptype	type		name				initval		minval		maxval		itemname	items
PROPDEF(	Midi,		NONE,		ENUM,		int,		iInputDevice,		0,			0,			0,			NULL,		0)
PROPDEF(	Input,		NONE,		VAR,		float,		fWheelSense,		6.25f,		0.1f,		10.0f,		NULL,		0)
PROPDEF(	Export,		NONE,		FOLDER,		CString,	sImageFolder,		"",			"",			"",			NULL,		0)
PROPDEF(	Export,		NONE,		VAR,		UINT,		nImageWidth,		1920,		1,			UINT_MAX,	NULL,		0)
PROPDEF(	Export,		NONE,		VAR,		UINT,		nImageHeight,		1080,		1,			UINT_MAX,	NULL,		0)
PROPDEF(	Export,		NONE,		VAR,		bool,		bUseViewSize,		1,			0,			0,			NULL,		0)
PROPDEF(	Export,		NONE,		ENUM,		UINT,		nScaleToFit,		0,			0,			0,			m_oiScaleToFit,	SCALE_TO_FIT_TYPES)
PROPDEF(	Export,		NONE,		VAR,		bool,		bPromptUser,		1,			0,			0,			NULL,		0)
PROPDEF(	Snapshot,	NONE,		VAR,		bool,		bLetterbox,			1,			0,			0,			NULL,		0)
PROPDEF(	Snapshot,	NONE,		ENUM,		int,		iLegacySize,		1,			0,			0,			m_oiLegacySize,	LEGACY_SIZES)
PROPDEF(	General,	NONE,		VAR,		int,		nMRUItems,			4,			0,			16,			NULL,		0)

#undef PROPDEF
#endif

#ifdef SCALETOFITDEF

SCALETOFITDEF(None)
SCALETOFITDEF(Width)
SCALETOFITDEF(Height)
SCALETOFITDEF(Both)

#undef SCALETOFITDEF
#endif

#ifdef LEGACYSIZEDEF

LEGACYSIZEDEF(640, 480, VGA)
LEGACYSIZEDEF(800, 600, SVGA)
LEGACYSIZEDEF(1024, 768, XGA)

#undef LEGACYSIZEDEF
#endif
