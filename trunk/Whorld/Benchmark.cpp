// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14jul05	initial version
		01		12apr18	move ctor to header
		02		10feb25	rename variables

        compute benchmarks using performance counter
 
*/

#include "stdafx.h"
#include "Benchmark.h"

LONGLONG CBenchmark::m_nFreq = InitFreq();

LONGLONG CBenchmark::InitFreq()
{
	LARGE_INTEGER	nFreq;
	QueryPerformanceFrequency(&nFreq);
	return nFreq.QuadPart;
}
