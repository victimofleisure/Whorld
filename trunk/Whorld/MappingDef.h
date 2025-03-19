// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda
 
		revision history:
		rev		date	comments
		00		20mar20	initial version
		01		26feb25	adapt for Whorld
		02		27feb25	add undo codes
		03		02mar25	add parameter property
		04		19mar25	make mapping range real instead of integer

		mapping column and member definitions

*/

#ifdef MAPPINGDEF

//			name		align	width	type	prefix	member			initval	minval	maxval
#ifdef MAPPINGDEF_INCLUDE_NUMBER
MAPPINGDEF(	NUMBER,		LEFT,	30,		int,	n,		0,				0,		0,		0)
#endif
#if !defined(MAPPINGDEF_OPTIONAL) || !MAPPINGDEF_OPTIONAL	// mandatory members
#ifndef MAPPINGDEF_EXCLUDE_TAGS
MAPPINGDEF(	EVENT,		LEFT,	100,	int,	i,		Event,			MIDI_CVM_CONTROL,	0,	EVENTS - 1)	// MIDI event
#endif
MAPPINGDEF(	CHANNEL,	LEFT,	60,		int,	i,		Channel,		0,		0,		MIDI_CHANNELS - 1)	// MIDI channel
MAPPINGDEF(	CONTROL,	LEFT,	60,		int,	i,		Control,		1,		0,		MIDI_NOTE_MAX)		// MIDI controller
#ifndef MAPPINGDEF_EXCLUDE_TAGS
MAPPINGDEF(	TARGET,		LEFT,	120,	int,	i,		Target,			0,		0,		TARGETS - 1)		// mapping target
MAPPINGDEF(	PROPERTY,	LEFT,	85,		int,	i,		Prop,			0,		0,		PARAM_PROP_COUNT)	// parameter property
#endif
#endif
#if !defined(MAPPINGDEF_OPTIONAL) || MAPPINGDEF_OPTIONAL	// optional members
MAPPINGDEF(	START,		LEFT,	60,		double,	f,		Start,			0,		0,		0)	// range start, as a percentage
MAPPINGDEF(	END,		LEFT,	60,		double,	f,		End,			100,	0,		0)	// range end, as a percentage
#endif

#undef MAPPINGDEF
#undef MAPPINGDEF_INCLUDE_NUMBER
#undef MAPPINGDEF_EXCLUDE_TAGS
#undef MAPPINGDEF_OPTIONAL

#endif // MAPPINGDEF

#ifdef MAPPINGUNDODEF

MAPPINGUNDODEF(PROPERTY)
MAPPINGUNDODEF(MULTI_PROPERTY)
MAPPINGUNDODEF(CUT)
MAPPINGUNDODEF(PASTE)
MAPPINGUNDODEF(INSERT)
MAPPINGUNDODEF(DELETE)
MAPPINGUNDODEF(MOVE)
MAPPINGUNDODEF(SORT)
MAPPINGUNDODEF(LEARN)
MAPPINGUNDODEF(LEARN_MULTI)

#undef MAPPINGUNDODEF
#endif	// MAPPINGUNDODEF

#ifdef MAPPINGDEF_MISC_TARGET

MAPPINGDEF_MISC_TARGET(OriginX)
MAPPINGDEF_MISC_TARGET(OriginY)
MAPPINGDEF_MISC_TARGET(Fill)
MAPPINGDEF_MISC_TARGET(Outline)
MAPPINGDEF_MISC_TARGET(Mirror)
MAPPINGDEF_MISC_TARGET(OriginDrag)
MAPPINGDEF_MISC_TARGET(OriginRandom)
MAPPINGDEF_MISC_TARGET(Reverse)
MAPPINGDEF_MISC_TARGET(Convex)
MAPPINGDEF_MISC_TARGET(LoopHue)
MAPPINGDEF_MISC_TARGET(ZoomCenter)
MAPPINGDEF_MISC_TARGET(RandomPhase)
MAPPINGDEF_MISC_TARGET(Clear)
MAPPINGDEF_MISC_TARGET(Pause)

#undef MAPPINGDEF_MISC_TARGET
#endif // MAPPINGDEF_MISC_TARGET
