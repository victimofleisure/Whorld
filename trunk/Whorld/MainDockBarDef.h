// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		08feb25	initial version
		01		26feb25	add mapping
		02		02mar25	add globals
		03		17mar25	add movie
		04		27mar25	add playlist
		
*/

#ifdef MAINDOCKBARDEF

// Don't remove or reorder entries! Append only to avoid incompatibility.

//			   name			width	height	style
MAINDOCKBARDEF(Params,		200,	300,	dwBaseStyle | CBRS_LEFT)
MAINDOCKBARDEF(Master,		200,	300,	dwBaseStyle | CBRS_RIGHT)
MAINDOCKBARDEF(Mapping,		300,	200,	dwBaseStyle | CBRS_BOTTOM)
MAINDOCKBARDEF(Globals,		200,	300,	dwBaseStyle | CBRS_RIGHT)
MAINDOCKBARDEF(Movie,		300,	20,		dwBaseStyle | CBRS_BOTTOM)
MAINDOCKBARDEF(Playlist,	200,	300,	dwBaseStyle | CBRS_LEFT)

// After adding a new dockable bar here:
// 1. Add a resource string IDS_BAR_Foo where Foo is the bar name.
// 2. Add a registry key RK_Foo for the bar in AppRegKey.h.
//
// Otherwise Polymeter.cpp won't compile; it uses the resource strings
// in CreateDockingWindows and the registry keys in ResetWindowLayout.
// The docking bar IDs, member variables, and code to create and dock
// the bars are all generated automatically by the macros above.

#endif	// MAINDOCKBARDEF
#undef MAINDOCKBARDEF

// list of dockable bars that handle standard editing commands
#ifdef MAINDOCKBARDEF_WANTEDITCMDS
MAINDOCKBARDEF_WANTEDITCMDS(Mapping)
MAINDOCKBARDEF_WANTEDITCMDS(Playlist)
#endif // MAINDOCKBARDEF_WANTEDITCMDS
#undef MAINDOCKBARDEF_WANTEDITCMDS

// list of dockable bar context menus to add to Customize
#ifdef DOCKBARMENUDEF
#endif // DOCKBARMENUDEF
#undef DOCKBARMENUDEF
