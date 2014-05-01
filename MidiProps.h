// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12sep05	initial version
		01		17oct05	add Origin X and Y
		02		20oct05	add Bank
		03		27oct05	add PatchSecs, XFadePlay, XFadeLoop
		04		21dec05	add HueLoopLength
		05		03mar06	add convex
		06		04mar06	add trail
		07		28mar06	add ZoomCenter
		08		24apr06	add CanvasScale and PatchMode
		09		02jun06	add video properties
		10		24jun06	add Copies and Spread
		11		16jul06	add video origin
		12		23jan08	replace MIDI range scaler with start/end

		miscellaneous MIDI-assignable properties
 
*/

//											range	range
//			name			tag				start	end		controller
MIDI_PROP(	Bank,			BANK,			0,		64,		BANK_SELECT)
MIDI_PROP(	Speed,			SPEED,			-1,		1,		MODULATION_WHEEL)
MIDI_PROP(	Zoom,			ZOOM,			-1,		1,		BREATH_CONTROLLER)
MIDI_PROP(	Damping,		DAMPING,		0,		1,		UNDEFINED_1)
MIDI_PROP(	Trail,			TRAIL,			0,		1,		UNDEFINED_3)
MIDI_PROP(	Rings,			RINGS,			0,		1,		UNDEFINED_2)
MIDI_PROP(	Copies,			COPIES,			1,		21,		UNDEFINED_4)
MIDI_PROP(	Spread,			SPREAD,			0,		600,	UNDEFINED_5)
MIDI_PROP(	PatchSecs,		PATCH_SECS,		0,		60,		FOOT_CONTROLLER)
MIDI_PROP(	XFadeSecs,		XFADE_SECS,		0,		60,		PORTAMENTO_TIME)
MIDI_PROP(	XFadePos,		XFADE_POS,		0,		1,		BALANCE)
MIDI_PROP(	OriginX,		ORIGIN_X,		0,		1,		GENERAL_PURPOSE_CONTROLLER_1)
MIDI_PROP(	OriginY,		ORIGIN_Y,		0,		1,		GENERAL_PURPOSE_CONTROLLER_2)
MIDI_PROP(	Hue,			HUE,			0,		1,		SOUND_VARIATION)
MIDI_PROP(	HueLoopLength,	HUE_LOOP_LENGTH,0,		360,	SOUND_TIMBRE)
MIDI_PROP(	CanvasScale,	CANVAS_SCALE,	1,		3,		UNDEFINED_32)
MIDI_PROP(	PatchMode,		PATCH_MODE,		0,		1,		UNDEFINED_33)
MIDI_PROP(	RandomPatch,	RANDOM_PATCH,	0,		1,		UNDEFINED_34)
MIDI_PROP(	LoadNextA,		LOAD_NEXT_A,	0,		1,		UNDEFINED_35)
MIDI_PROP(	LoadNextB,		LOAD_NEXT_B,	0,		1,		UNDEFINED_36)
MIDI_PROP(	LoopHue,		LOOP_HUE,		0,		1,		DAMPER_PEDAL)
MIDI_PROP(	RotateHue,		ROTATE_HUE,		0,		1,		SUSTENUTO_PEDAL)
MIDI_PROP(	InvertColor,	INVERT_COLOR,	0,		1,		LEGATO_PEDAL)
MIDI_PROP(	Mirror,			MIRROR,			0,		1,		GENERAL_PURPOSE_BUTTON_1)
MIDI_PROP(	Fill,			FILL,			0,		1,		GENERAL_PURPOSE_BUTTON_2)
MIDI_PROP(	Outline,		OUTLINE,		0,		1,		GENERAL_PURPOSE_BUTTON_3)
MIDI_PROP(	XRay,			XRAY,			0,		1,		GENERAL_PURPOSE_BUTTON_4)
MIDI_PROP(	RandomPhase,	RANDOM_PHASE,	0,		1,		UNDEFINED_37)
MIDI_PROP(	Reverse,		REVERSE,		0,		1,		UNDEFINED_38)
MIDI_PROP(	ToggleOrigin,	TOGGLE_ORIGIN,	0,		1,		UNDEFINED_39)
MIDI_PROP(	AutoPlay,		AUTOPLAY,		0,		1,		UNDEFINED_40)
MIDI_PROP(	Pause,			PAUSE,			0,		1,		UNDEFINED_41)
MIDI_PROP(	Clear,			CLEAR,			0,		1,		UNDEFINED_42)
MIDI_PROP(	XFadePlay,		XFADE_PLAY,		0,		1,		UNDEFINED_44)
MIDI_PROP(	XFadeLoop,		XFADE_LOOP,		0,		1,		UNDEFINED_45)
MIDI_PROP(	Convex,			CONVEX,			0,		1,		UNDEFINED_46)
MIDI_PROP(	ZoomCenter,		ZOOM_CENTER,	0,		1,		UNDEFINED_47)
MIDI_PROP(	VideoSelect,	VIDEO_SELECT,	0,		10,		SOUND_RELEASE_TIME)
MIDI_PROP(	VideoBlending,	VIDEO_BLENDING,	0,		10,		SOUND_ATTACK_TIME)
MIDI_PROP(	VideoCycle,		VIDEO_CYCLE,	0,		1,		SOUND_BRIGHTNESS)
MIDI_PROP(	VideoCycleLen,	VIDEO_CYCLE_LEN,1,		10,		SOUND_CONTROL_6)
MIDI_PROP(	VideoOriginX,	VIDEO_ORIGIN_X,	0,		1,		SOUND_CONTROL_7)
MIDI_PROP(	VideoOriginY,	VIDEO_ORIGIN_Y,	0,		1,		SOUND_CONTROL_8)
MIDI_PROP(	VideoRandOrg,	VIDEO_RAND_ORG,	0,		1,		SOUND_CONTROL_9)

