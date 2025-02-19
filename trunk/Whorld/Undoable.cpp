// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		22nov06	rename strings to start with IDS_
		02		26nov06	cast GetUndoKey to WORD
		03		18mar08	remove key support
		04		28may10	add CtrlID to notify and cancel

        undoable edit interface
 
*/

#include "stdafx.h"
#include "Undoable.h"
