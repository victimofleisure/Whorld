// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		revision history:
		rev		date	comments
		00		02nov97 initial version
		01		17aug13	refactor
		02		07oct14	add MIDI clock constants
		03		28mar18	add inline functions
		04		19mar20	add channel voice command to index macro
		05		19mar20	add channel voice and system status messages
		06		21jan22	add stream buffer event macros
		07		23dec23	add message component bitmasks
		08		03mar25	add inverse channel voice macro and pitch bend range

		midi types and constants

*/

#ifndef MIDI_INCLUDED
#define MIDI_INCLUDED

enum {	// channel voice messages
	NOTE_OFF 		= 0x80,
	NOTE_ON			= 0x90,
	KEY_AFT			= 0xa0,
	CONTROL 		= 0xb0,
	PATCH			= 0xc0,
	CHAN_AFT 		= 0xd0,
	WHEEL			= 0xe0,
};

enum {	// system messages
	SYSEX			= 0xf0,
	MTC_QTR_FRAME	= 0xf1,
	SONG_POSITION	= 0xf2,
	SONG_SELECT 	= 0xf3,
	TUNE_REQUEST	= 0xf6,
	EOX 			= 0xf7,
	CLOCK			= 0xf8,
	START			= 0xfa,
	CONTINUE		= 0xfb,
	STOP			= 0xfc,
	ACTIVE_SENSING	= 0xfe,
	SYSTEM_RESET	= 0xff,
};

enum {
	MIDI_CHANNELS	= 16,
	MIDI_CHAN_MAX	= MIDI_CHANNELS - 1,
	MIDI_NOTES		= 128,
	MIDI_NOTE_MAX	= MIDI_NOTES - 1,
	MIDI_PITCH_BEND_STEPS	= 0x4000,
	MIDI_PITCH_BEND_MAX		= MIDI_PITCH_BEND_STEPS - 1,
};

enum {	// MIDI channel voice messages
	#define MIDICHANSTATDEF(name) MIDI_CVM_##name,
	#include "MidiCtrlrDef.h"	// generate enum
	MIDI_CHANNEL_VOICE_MESSAGES
};
enum {	// MIDI system status messages
	#define MIDISYSSTATDEF(name) MIDI_SSM_##name,
	#include "MidiCtrlrDef.h"	// generate enum
	MIDI_SYSTEM_STATUS_MESSAGES
};
enum {	// MIDI controllers
	#define MIDI_CTRLR_DEF(name) name,
	#include "MidiCtrlrDef.h"	// generate enum
};

enum {	// bitmasks for MIDI message components
	MIDI_CHAN_MASK	= 0x00000f,	// channel nibble
	MIDI_CMD_MASK	= 0x0000f0,	// command nibble
	MIDI_STAT_MASK	= 0x0000ff,	// status byte
	MIDI_P1_MASK	= 0x00ff00,	// 1st parameter byte
	MIDI_P2_MASK	= 0xff0000,	// 2nd parameter byte
};

#define MIDI_STAT(msg)		LOBYTE(msg)
#define MIDI_P1(msg)		HIBYTE(msg)
#define MIDI_P2(msg)		LOBYTE(HIWORD(msg))
#define MIDI_CMD(msg)		(msg & 0xf0)
#define MIDI_CHAN(msg)		(msg & 0x0f)
#define MIDI_CMD_IDX(msg)	((MIDI_CMD(msg) >> 4) - 8)
#define MIDI_IDX_CMD(idx)	((idx + 8) << 4)
#define CHAN_MODE_MSG(ctrl)	(ctrl >= 120)
#define MIDI_CLOCK_PPQ		24
#define MIDI_BEAT_CLOCKS	6

// stream buffer events have event type in high byte
#define MIDI_IS_SHORT_MSG(msg)	(!(msg & 0xff000000))
#define MIDI_SHORT_MSG_CMD(msg)	(msg & 0xff0000f0)

union MIDI_MSG {
	struct {
		BYTE	stat;
		char	p1;
		char	p2;
	};
	DWORD	dw;
};

#define MAKE_STAT(cmd, chan) ((cmd << 4) | chan)
#define MIDI_SUCCEEDED(mr) (mr == MMSYSERR_NOERROR)
#define MIDI_FAILED(mr) (mr != MMSYSERR_NOERROR)

inline bool IsMidiCmd(int Cmd)
{
	return(Cmd >= 128 && Cmd < 256);
}

inline bool IsMidiChan(int Chan)
{
	return(Chan >= 0 && Chan < MIDI_CHANNELS);
}

inline bool IsMidiParam(int Param)
{
	return(Param >= 0 && Param < MIDI_NOTES);
}

inline DWORD MakeMidiMsg(int Cmd, int Chan, int P1, int P2)
{
	ASSERT(IsMidiCmd(Cmd));
	ASSERT(IsMidiChan(Chan));
	ASSERT(IsMidiParam(P1));
	ASSERT(IsMidiParam(P2));
	return Cmd + Chan + (P1 << 8) + (P2 << 16);
}

#endif
