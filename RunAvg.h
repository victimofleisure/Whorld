// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda
 
		revision history:
		rev		date	comments
		00		04jul03	initial version
		01		08oct03	begin revision history
		02		13oct03	add reset
		03		08mar05	port to C++
		04		17jul05	convert to template
 
		maintain a running average
 
*/

#ifndef CRUNAVG_INCLUDED
#define	CRUNAVG_INCLUDED

template<class T> class CRunAvg : public WObject {
public:
	CRunAvg(int Size);
	CRunAvg();
	~CRunAvg();
	void	Create(int Size);
	void	Reset();
	void	Update(T Val);
	T		GetSum() const;
	T		GetAvg() const;
	int		GetCount() const;

private:
	T		m_Sum;			// sum of samples
	T		*m_History;		// array of samples
	int		m_Size;			// maximum number of samples
	int		m_Tail;			// index of oldest sample
	int		m_Samples;		// current number of samples
};

template<class T>
inline CRunAvg<T>::CRunAvg(int Size)
{
	m_History = NULL;
	Create(Size);
}

template<class T>
inline CRunAvg<T>::CRunAvg()
{
	m_History = NULL;
	Create(0);
}

template<class T>
inline CRunAvg<T>::~CRunAvg()
{
	delete [] m_History;
}

template<class T>
inline void CRunAvg<T>::Create(int Size)
{
	delete [] m_History;
	m_Size = Size;
	m_History = new T[Size];
	Reset();
}

template<class T>
inline void CRunAvg<T>::Reset()
{
	m_Tail = 0;
	m_Sum = 0;
	m_Samples = 0;
	memset(m_History, 0, m_Size * sizeof(T));
}

template<class T>
inline void CRunAvg<T>::Update(T Val)
{
	int	i = m_Tail;			// index of oldest sample
	m_Sum -= m_History[i];	// subtract oldest sample from sum
	m_History[i] = Val;		// overwrite oldest sample with new sample
	m_Sum += Val;			// add new sample to sum
	if (++m_Tail >= m_Size)	// increment index and wrap around if needed
		m_Tail = 0;
	if (m_Samples < m_Size)	// increment number of samples
		m_Samples++;
}

template<class T>
inline T CRunAvg<T>::GetSum() const
{
	return(m_Sum);
}

template<class T>
inline T CRunAvg<T>::GetAvg() const
{
	return(m_Samples ? m_Sum / m_Samples : 0);
}

template<class T>
inline int CRunAvg<T>::GetCount() const
{
	return(m_Samples);
}

#endif
