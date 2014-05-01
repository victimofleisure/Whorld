// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14jul05	initial version

        compute benchmarks using performance counter
 
*/

#ifndef CBENCHMARK_INCLUDED
#define CBENCHMARK_INCLUDED

class CBenchmark {
public:
	CBenchmark();
	double	Elapsed() const;
	void	Reset();
	static	double	Time();

private:
	double	m_Start;		// start time, in seconds since boot
	static	__int64	m_Freq;	// performance counter frequency, in Hz
	static	__int64	InitFreq();
};

inline double CBenchmark::Time()
{
	__int64	pc;
	QueryPerformanceCounter((LARGE_INTEGER *)&pc);
	return((double)pc / m_Freq);
}

inline double CBenchmark::Elapsed() const
{
	return(Time() - m_Start);
}

inline void CBenchmark::Reset()
{
	m_Start = Time();
}

#endif
