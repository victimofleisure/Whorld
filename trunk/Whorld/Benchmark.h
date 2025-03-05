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
		02		10feb25	reset method now returns elapsed time

        compute benchmarks using performance counter
 
*/

#pragma once

class CBenchmark {
public:
	CBenchmark();
	double	Elapsed() const;
	double	Reset();
	static	double	Time();

private:
	double	m_fStart;		// start time, in seconds since boot
	static	LONGLONG	m_nFreq;	// performance counter frequency, in Hz
	static	LONGLONG	InitFreq();
};

inline CBenchmark::CBenchmark()
{
	Reset();
}

inline double CBenchmark::Time()
{
	LARGE_INTEGER	nCount;
	QueryPerformanceCounter(&nCount);
	return static_cast<double>(nCount.QuadPart) / m_nFreq;
}

inline double CBenchmark::Elapsed() const
{
	return Time() - m_fStart;
}

inline double CBenchmark::Reset()
{
	double	fTime = Time();
	double	fElapsed = fTime - m_fStart;
	m_fStart = fTime;
	return fElapsed;
}
