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
 
		mapping column and member definitions

*/

#ifdef MAPPINGDEF

//			name			align			width	member			minval	maxval
#ifdef MAPPINGDEF_INCLUDE_NUMBER
MAPPINGDEF(	NUMBER,			LVCFMT_LEFT,	30,		0,				0,		0)
#endif
MAPPINGDEF(	IN_EVENT,		LVCFMT_LEFT,	100,	InEvent,		0,		CMapping::INPUT_EVENTS - 1)
MAPPINGDEF(	IN_CHANNEL,		LVCFMT_LEFT,	60,		InChannel,		0,		MIDI_CHANNELS - 1)
MAPPINGDEF(	IN_CONTROL,		LVCFMT_LEFT,	60,		InControl,		0,		MIDI_NOTE_MAX)
MAPPINGDEF(	OUT_EVENT,		LVCFMT_LEFT,	100,	OutEvent,		0,		CMapping::OUTPUT_EVENTS - 1)
MAPPINGDEF(	RANGE_START,	LVCFMT_LEFT,	60,		RangeStart,		0,		0)
MAPPINGDEF(	RANGE_END,		LVCFMT_LEFT,	60,		RangeEnd,		0,		0)

#undef MAPPINGDEF
#undef MAPPINGDEF_INCLUDE_NUMBER

#endif // MAPPINGDEF

#ifdef MAPPINGDEF_SPECIAL_TARGET

#undef MAPPINGDEF_SPECIAL_TARGET

#endif // MAPPINGDEF_SPECIAL_TARGET
