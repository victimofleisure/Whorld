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

        general-purpose waveform generator
 
*/

#ifndef COSCILLATOR_DEFINED
#define COSCILLATOR_DEFINED

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
	COscillator(double TimerFreq, int Waveform, double Freq);

// Attributes
	double	GetTimerFreq() const;
	void	SetTimerFreq(double TimerFreq);	// in Hz
	int		GetWaveform() const;
	void	SetWaveform(int Waveform);
	double	GetFreq() const;
	void	SetFreq(double Freq);		// doesn't change phase, but loses sync
	void	SetFreqSync(double Freq);	// stays in sync, but changes phase
	double	GetPulseWidth() const;
	void	SetPulseWidth(double PulseWidth);	// 0..1, 1 = 100% duty
	double	GetClock() const;
	void	SetClock(double Clock);	// in ticks
	double	GetPhase() const;	// 0..1, 1 = 360 degrees
	void	SetPhase(double Phase);		// 0..1, 1 = 360 degrees
	void	SetPhaseFromVal(double Val);	// -1..1
	double	GetVal();	// -1..1

// Operations
	void	TimerHook();

private:
// Constants
	static const double	pi;

// Member data
	double	m_TimerFreq;	// frequency at which timer hook runs, in Hz
	int		m_Waveform;		// type of waveform to generate; see enum above
	double	m_Freq;			// output frequency, in Hz
	double	m_PulseWidth;	// for pulse waveform: pulse duty cycle, from 0..1
	double	m_RandPrvPhase;	// for random waveform: previous tick's phase
	double	m_RandCurVal;	// for random waveform: current random value
	double	m_RandPrvVal;	// for random waveform: previous random value
	double	m_Clock;		// clock tick count; fractional ticks allowed

// Helpers
	void	Init();
	double	CurPos() const;
	double	CurPhase() const;
	void	Resync(double Phase);
};

inline double COscillator::GetTimerFreq() const
{
	return(m_TimerFreq);
}

inline void COscillator::SetTimerFreq(double TimerFreq)
{
	m_TimerFreq = TimerFreq;
}

inline int COscillator::GetWaveform() const
{
	return(m_Waveform);
}

inline void COscillator::SetWaveform(int Waveform)
{
	m_Waveform = Waveform;
}

inline double COscillator::GetFreq() const
{
	return(m_Freq);
}

inline void COscillator::SetFreqSync(double Freq)
{
	m_Freq = Freq;
}

inline double COscillator::GetPulseWidth() const
{
	return(m_PulseWidth);
}

inline void COscillator::SetPulseWidth(double PulseWidth)
{
	m_PulseWidth = PulseWidth;
}

inline double COscillator::GetClock() const
{
	return(m_Clock);
}

inline void COscillator::SetClock(double Clock)
{
	m_Clock = Clock;
}

inline void COscillator::TimerHook()
{
	m_Clock++;
}

#endif
