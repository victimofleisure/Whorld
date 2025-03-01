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
 
		mapping column and member definitions

*/

#ifdef MAPPINGDEF

//			name			align	width	prefix	member			minval	maxval
#ifdef MAPPINGDEF_INCLUDE_NUMBER
MAPPINGDEF(	NUMBER,			LEFT,	30,		n,		0,				0,		0)
#endif
MAPPINGDEF(	EVENT,			LEFT,	100,	i,		Event,			0,		EVENTS - 1)			// MIDI event
MAPPINGDEF(	CHANNEL,		LEFT,	60,		i,		Channel,		0,		MIDI_CHANNELS - 1)	// MIDI channel
MAPPINGDEF(	CONTROL,		LEFT,	60,		i,		Control,		0,		MIDI_NOTE_MAX)		// MIDI controller
MAPPINGDEF(	TARGET,			LEFT,	120,	i,		Target,			0,		TARGETS - 1)		// mapping target
MAPPINGDEF(	RANGE_START,	LEFT,	60,		n,		RangeStart,		0,		0)					// range start
MAPPINGDEF(	RANGE_END,		LEFT,	60,		n,		RangeEnd,		0,		0)					// range end

#undef MAPPINGDEF
#undef MAPPINGDEF_INCLUDE_NUMBER

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
MAPPINGDEF_MISC_TARGET(OriginDrag)
MAPPINGDEF_MISC_TARGET(OriginRandom)
MAPPINGDEF_MISC_TARGET(Reverse)
MAPPINGDEF_MISC_TARGET(Convex)
MAPPINGDEF_MISC_TARGET(LoopHue)
MAPPINGDEF_MISC_TARGET(RandomPhase)
MAPPINGDEF_MISC_TARGET(Clear)

#undef MAPPINGDEF_MISC_TARGET
#endif // MAPPINGDEF_MISC_TARGET
