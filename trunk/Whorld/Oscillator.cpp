// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		28apr05	change frequency without affecting phase
		02		05may05	add SetPhaseFromVal
		03		27aug05	change m_Clock type to double
		04		28aug05	random case must test current phase
		05		08mar06	in GetVal, default case now returns zero
		06		15mar06	add random ramp waveform
		07		12feb25	remove pi

        general-purpose waveform generator
 
*/

#include "stdafx.h"
#include "Oscillator.h"
#define _USE_MATH_DEFINES
#include <math.h>

COscillator::COscillator()
{
	Init();
}

COscillator::COscillator(double fTimerFreq, int nWaveform, double fFreq)
{
	Init();
	m_fTimerFreq = fTimerFreq;
	m_nWaveform = nWaveform;
	m_fFreq = fFreq;
}

void COscillator::Init()
{
	m_fTimerFreq = 0;
	m_nWaveform = 0;
	m_fFreq = 0;
	m_fPulseWidth = 0.5;
	m_fRandPrvPhase = 0;
	m_fRandCurVal = 0;
	m_fRandPrvVal = 0;
	m_fClock = 0;
}

inline double COscillator::CurPhase() const
{
	return fmod(CurPos(), 1);
}

void COscillator::SetPhaseFromVal(double fVal)
{
	double	r;
	switch (m_nWaveform) {
	case TRIANGLE:
		r = (fVal + 1) / 2 + 10.5;
		break;
	case SINE:
		r = asin(fVal) / M_PI;
		break;
	case RAMP_UP:
		r = fVal + 1;
		break;
	case RAMP_DOWN:
		r = 1 - fVal;
		break;
	case SQUARE:
	case PULSE:
		r = (fVal + 1) / 2 + 1;
		break;
	default:
		r = 0;
	}
	SetPhase(r / 2);
}

double COscillator::GetVal()
{
	double	r;
	switch (m_nWaveform) {
	case TRIANGLE:
		r = fmod(CurPos() * 2 + 0.5, 2);
		r = r < 1 ? r * 2 - 1 : 3 - r * 2;
		break;
	case SINE:
		r = sin(CurPos() * M_PI * 2);
		break;
	case RAMP_UP:
		r = CurPhase() * 2 - 1;
		break;
	case RAMP_DOWN:
		r = 1 - CurPhase() * 2;
		break;
	case SQUARE:
		r = CurPhase() < 0.5 ? 1 : -1;
		break;
	case PULSE:
		r = CurPhase() < m_fPulseWidth ? 1 : -1;
		break;
	case RANDOM:
		r = CurPhase();
		if (r < 0.5 && m_fRandPrvPhase > 0.5)
			m_fRandCurVal = double(rand()) / RAND_MAX * 2 - 1;
		m_fRandPrvPhase = r;
		r = m_fRandCurVal;
		break;
	case RANDOM_RAMP:
		r = CurPhase();
		if (r < 0.5 && m_fRandPrvPhase > 0.5) {
			m_fRandPrvVal = m_fRandCurVal;
			m_fRandCurVal = double(rand()) / RAND_MAX * 2 - 1;
		}
		m_fRandPrvPhase = r;
		r = m_fRandPrvVal + (m_fRandCurVal - m_fRandPrvVal) * r;
		break;
	default:
		r = 0;
	}
	return r;
}
