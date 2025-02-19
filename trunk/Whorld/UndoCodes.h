// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30may10	initial version

		define undo codes
 
*/

#pragma once

// define macro to enumerate undo codes
#define UCODE_DEF(name) UCODE_##name,
enum {	// enumerate undo codes
	#include "UndoCodeData.h"
	UNDO_CODES,
};

