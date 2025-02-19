// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		08feb25	initial version

		
*/

#ifdef MAINDOCKBARDEF

// Don't remove or reorder entries! Append only to avoid incompatibility.

//			   name			width	height	style
MAINDOCKBARDEF(Params,		200,	300,	dwBaseStyle | CBRS_LEFT | WS_VISIBLE | AFX_CBRS_FLOAT)
MAINDOCKBARDEF(Master,		200,	300,	dwBaseStyle | CBRS_RIGHT | WS_VISIBLE | AFX_CBRS_FLOAT)

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
#endif
#undef MAINDOCKBARDEF_WANTEDITCMDS

// list of dockable bar context menus to add to Customize
#ifdef DOCKBARMENUDEF
#endif // DOCKBARMENUDEF
#undef DOCKBARMENUDEF
