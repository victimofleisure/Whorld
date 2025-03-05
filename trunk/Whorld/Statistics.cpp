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

#include "stdafx.h"
#include "Statistics.h"
#include "math.h"
#include "float.h"	// for DBL_MAX

CStatistics::CStatistics(int nSampleSize)
{
	Reset();
	SetSampleSize(nSampleSize);
}

void CStatistics::Reset()
{
	m_fMax = DBL_MIN;
	m_fMin = DBL_MAX;
	m_fSum = 0;
	m_nSamps = 0;
}

void CStatistics::Add(double fSample)
{
	if (m_nSamps < GetSampleSize()) {
		m_arrSamp[m_nSamps] = fSample;
		if (fSample > m_fMax)
			m_fMax = fSample;
		if (fSample < m_fMin)
			m_fMin = fSample;
		m_fSum += fSample;
		m_nSamps++;
	}
}

void CStatistics::Print(double fElapsed)
{
	Add(fElapsed); 
	if (m_nSamps >= GetSampleSize()) {
		_tprintf(_T("%s\n"), GetStats().GetString());
		Reset();
	}
}

double CStatistics::GetStandardDeviation() const
{
	// sample count can exceed sample array size, so take minimum
	int	nSamps = min(m_nSamps, GetSampleSize());
	if (!nSamps)
		return 0;	// avoid divide by zero
	double	fSum = 0;
	for (int iSamp = 0; iSamp < nSamps; iSamp++)
		fSum += m_arrSamp[iSamp];
	double	fMean = fSum / nSamps;	// mean of samples
	fSum = 0;
	for (int iSamp = 0; iSamp < nSamps; iSamp++) {
		double	fDiff = m_arrSamp[iSamp] - fMean;
		fSum += fDiff * fDiff;
	}
	return sqrt(fSum / nSamps);	// square root of variance
}

CString CStatistics::GetStats() const
{
	CString	s;
	if (m_nSamps) {
		s.Format(_T("cnt=%d tot=%f min=%f max=%f avg=%f sd=%f"),
			m_nSamps, GetSum(), GetMin(), GetMax(), GetAverage(), 
			GetStandardDeviation());
	}
	return s;
}
