// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda
 
		revision history:
		rev		date	comments
		00		29aug13	initial version
		01		06feb25	rename vars
 
*/

#pragma once

#include "ArrayEx.h"

class CStatistics {
public:
// Construction
	CStatistics(int nSamps = 0x1000);

// Attributes
	double	GetMax() const;
	double	GetMin() const;
	double	GetSum() const;
	int		GetCount() const;
	double	GetAverage() const;
	int		GetSampleSize() const;
	void	SetSampleSize(int nSamps);
	double	GetSample(int iSamp) const;
	double	GetStandardDeviation() const;
	CString	GetStats() const;

// Operations
	void	Add(double fSample);
	void	Reset();
	void	Print(double fSample);

protected:
// Types
	typedef CArrayEx<double, double> CDblArray;

// Member data
	CDblArray	m_arrSamp;	// array of samples
	double	m_fMax;			// maximum sample
	double	m_fMin;			// minimum sample
	double	m_fSum;			// sum of samples
	int		m_nSamps;		// number of samples
};

inline double CStatistics::GetMin() const
{
	return m_fMin;
}

inline double CStatistics::GetMax() const
{
	return m_fMax;
}

inline double CStatistics::GetSum() const
{
	return m_fSum;
}

inline int CStatistics::GetCount() const
{
	return m_nSamps;
}

inline double CStatistics::GetAverage() const
{
	ASSERT(m_nSamps);	// else divide by zero
	return m_fSum / m_nSamps;
}

inline int CStatistics::GetSampleSize() const
{
	return (int)m_arrSamp.GetSize();
}

inline void CStatistics::SetSampleSize(int nSamps)
{
	m_arrSamp.SetSize(nSamps);
}

inline double CStatistics::GetSample(int iSamp) const
{
	return m_arrSamp[iSamp];
}
