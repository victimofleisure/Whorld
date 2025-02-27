// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		15mar06	add random ramp waveform
		02		12feb25	remove pi

        general-purpose waveform generator
 
*/

#pragma once

class COscillator {
public:
// Constants
	enum WAVEFORM {
		TRIANGLE,
		SINE,
		RAMP_UP,
		RAMP_DOWN,
		SQUARE,
		PULSE,
		RANDOM,
		RANDOM_RAMP,
		WAVEFORMS
	};

// Construction
	COscillator();
	COscillator(double fTimerFreq, int nWaveform, double fFreq);

// Attributes
	double	GetTimerFreq() const;
	void	SetTimerFreq(double fTimerFreq);	// in Hz
	int		GetWaveform() const;
	void	SetWaveform(int nWaveform);
	double	GetFreq() const;
	void	SetFreq(double fFreq);		// doesn't change phase, but loses sync
	void	SetFreqSync(double fFreq);	// stays in sync, but changes phase
	double	GetPulseWidth() const;
	void	SetPulseWidth(double fPulseWidth);	// 0..1, 1 = 100% duty
	double	GetClock() const;
	void	SetClock(double fClock);	// in ticks
	double	GetPhase() const;	// 0..1, 1 = 360 degrees
	void	SetPhase(double fPhase);		// 0..1, 1 = 360 degrees
	void	SetPhaseFromVal(double fVal);	// -1..1
	double	GetVal();	// -1..1

// Operations
	void	TimerHook();

protected:
// Member data
	double	m_fTimerFreq;	// frequency at which timer hook runs, in Hz
	int		m_nWaveform;	// type of waveform to generate; see enum above
	double	m_fFreq;		// output frequency, in Hz
	double	m_fPulseWidth;	// for pulse waveform: pulse duty cycle, from 0..1
	double	m_fRandPrvPhase;	// for random waveform: previous tick's phase
	double	m_fRandCurVal;	// for random waveform: current random value
	double	m_fRandPrvVal;	// for random waveform: previous random value
	double	m_fClock;		// clock tick count; fractional ticks allowed

// Helpers
	void	Init();
	double	CurPos() const;
	double	CurPhase() const;
	void	Resync(double fPhase);
};

inline double COscillator::GetTimerFreq() const
{
	return m_fTimerFreq;
}

inline void COscillator::SetTimerFreq(double fTimerFreq)
{
	m_fTimerFreq = fTimerFreq;
}

inline int COscillator::GetWaveform() const
{
	return m_nWaveform;
}

inline void COscillator::SetWaveform(int nWaveform)
{
	m_nWaveform = nWaveform;
}

inline double COscillator::GetFreq() const
{
	return m_fFreq;
}

inline void COscillator::SetFreq(double fFreq)
{
	double	fPhase = CurPhase();
	m_fFreq = fFreq;
	Resync(fPhase);
}

inline void COscillator::SetFreqSync(double fFreq)
{
	m_fFreq = fFreq;
}

inline double COscillator::GetPulseWidth() const
{
	return m_fPulseWidth;
}

inline void COscillator::SetPulseWidth(double fPulseWidth)
{
	m_fPulseWidth = fPulseWidth;
}

inline double COscillator::GetClock() const
{
	return m_fClock;
}

inline void COscillator::SetClock(double fClock)
{
	m_fClock = fClock;
}

inline double COscillator::GetPhase() const
{
	return CurPhase();
}

inline void COscillator::SetPhase(double fPhase)
{
	Resync(fPhase);
}

inline void COscillator::TimerHook()
{
	m_fClock++;
}

inline double COscillator::CurPos() const
{
	return m_fFreq / m_fTimerFreq * m_fClock;
}

inline void COscillator::Resync(double fPhase)
{
	m_fClock = m_fFreq ? fPhase * (m_fTimerFreq / m_fFreq) : 0;
}

class CTriggerOscillator : public COscillator {
public:
	CTriggerOscillator();
	CTriggerOscillator(double fTimerFreq, double fFreq);
	bool	IsTrigger();
	void	Reset();

protected:
	bool	m_bIsHigh;	// true if previous output was high
};

inline CTriggerOscillator::CTriggerOscillator()
{
}

inline CTriggerOscillator::CTriggerOscillator(double fTimerFreq, double fFreq) 
	: COscillator(fTimerFreq, SQUARE, fFreq)
{
	m_bIsHigh = false;
}

inline bool CTriggerOscillator::IsTrigger()
{
	TimerHook();
	bool	bIsHigh = GetVal() >= 1 - DPoint::Epsilon;	// true if output is high
	bool	bIsTrigger = bIsHigh && !m_bIsHigh;	// true if transition from low to high
	m_bIsHigh = bIsHigh;	// update previous output state
	return bIsTrigger;
}

inline void CTriggerOscillator::Reset()
{
	SetClock(0);
	m_bIsHigh = false;
}
