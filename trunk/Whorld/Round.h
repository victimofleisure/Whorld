// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		04oct12	initial version
		01		07jun21	use title case to avoid conflicts with C++ 11

		optimized rounding and truncation

*/

#pragma once

#ifdef _WIN64
#include "intrin.h"		// for SSE intrinsics
#endif

inline int Round(double x)	// round to 32-bit integer
{
#ifdef _WIN64
    return(_mm_cvtsd_si32(_mm_set_sd(x)));
#else
	int		temp;
	__asm {
		fld		x		// load real
		fistp	temp	// store integer and pop stack
	}
	return(temp);
#endif
}

inline int Trunc(double x)	// truncate to 32-bit integer
{
#ifdef _WIN64
    return(_mm_cvttsd_si32(_mm_set_sd(x)));
#else
	int		temp;
	short	cw, chop;
	__asm {
		fstcw	cw		// save control word
		mov		ax, cw
		or		ax, 0c00h	// set rounding mode to chop
		mov		chop, ax
		fldcw	chop	// load chop control word
		fld		x		// load real
		fistp	temp	// store integer and pop stack
		fldcw	cw		// restore control word
	}
	return(temp);
#endif
}

inline LONGLONG Round64(double x)	// round to 64-bit integer
{
#ifdef _WIN64
    return(_mm_cvtsd_si64x(_mm_set_sd(x)));
#else
	LONGLONG	temp;
	__asm {
		fld		x		// load real
		fistp	temp	// store integer and pop stack
	}
	return(temp);
#endif
}

inline LONGLONG Trunc64(double x)	// truncate to 64-bit integer
{
#ifdef _WIN64
    return(_mm_cvttsd_si64x(_mm_set_sd(x)));
#else
	LONGLONG	temp;
	short	cw, chop;
	__asm {
		fstcw	cw		// save control word
		mov		ax, cw
		or		ax, 0c00h	// set rounding mode to chop
		mov		chop, ax
		fldcw	chop	// load chop control word
		fld		x		// load real
		fistp	temp	// store integer and pop stack
		fldcw	cw		// restore control word
	}
	return(temp);
#endif
}

inline W64INT RoundW64INT(double x)	// round to W64INT
{
#ifdef _WIN64
	return(Round64(x));
#else
	return(Round(x));
#endif
}

inline W64INT TruncW64INT(double x)	// truncate to W64INT
{
#ifdef _WIN64
	return(Trunc64(x));
#else
	return(Trunc(x));
#endif
}
