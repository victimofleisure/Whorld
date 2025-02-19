// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24mar05	initial version

        define types for registry template specializations
 
*/

//				type		undoval
UNDOTYPEDEF(	int,		p.x.i)
UNDOTYPEDEF(	long,		p.x.i)
UNDOTYPEDEF(	UINT,		p.x.u)
UNDOTYPEDEF(	DWORD,		p.x.u)
UNDOTYPEDEF(	short,		p.x.s.lo)
UNDOTYPEDEF(	WORD,		p.x.w.lo)
UNDOTYPEDEF(	char,		p.x.c.al)
UNDOTYPEDEF(	BYTE,		p.x.c.al)
UNDOTYPEDEF(	bool,		p.x.b)

#undef UNDOTYPEDEF
