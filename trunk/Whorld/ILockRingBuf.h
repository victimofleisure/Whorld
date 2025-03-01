// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		16apr10		initial version

		interlocked ring buffer template

*/

#pragma once

template<class T>
class CILockRingBuf {
public:
// Construction
	CILockRingBuf();
	CILockRingBuf(int nSize);
	~CILockRingBuf();
	void	Create(int nSize);
	void	Destroy();

// Attributes
	int		GetSize() const;
	int		GetCount() const;

// Operations
	bool	Push(const T& Item);
	bool	Pop(T& Item);

protected:
// Member data
	T		*m_pStart;	// points to first element
	T		*m_pEnd;	// points to last element
	T		*m_pHead;	// the next element to write to
	T		*m_pTail;	// the next element to read from
	int		m_nSize;	// maximum number of elements
	LONG	m_nItems;	// number of elements in use

// Helpers
	void	Init();
	void	Alloc(int nSize);
};

template<class T>
inline CILockRingBuf<T>::CILockRingBuf()
{
	Init();
}

template<class T>
inline CILockRingBuf<T>::CILockRingBuf(int nSize)
{
	Alloc(nSize);
}

template<class T>
inline CILockRingBuf<T>::~CILockRingBuf()
{
	delete [] m_pStart;
}

template<class T>
inline void CILockRingBuf<T>::Init()
{
	m_pStart = NULL;
	m_pEnd = NULL;
	m_pHead = NULL;
	m_pTail = NULL;
	m_nSize = 0;
	m_nItems = 0;
}

template<class T>
inline void CILockRingBuf<T>::Alloc(int nSize)
{
	m_pStart = new T[nSize];
	m_pEnd = m_pStart + nSize;
	m_pHead = m_pStart;
	m_pTail = m_pStart;
	m_nSize = nSize;
	m_nItems = 0;
}

template<class T>
inline void CILockRingBuf<T>::Create(int nSize)
{
	delete [] m_pStart;	// in case undestroyed instance is reused
	Alloc(nSize);
}

template<class T>
inline bool CILockRingBuf<T>::Push(const T& Item)
{
	if (m_nItems >= m_nSize)
		return(FALSE);
	*m_pHead++ = Item;
	if (m_pHead == m_pEnd)
		m_pHead = m_pStart;
	InterlockedIncrement(&m_nItems);
	return(TRUE);
}

template<class T>
inline bool CILockRingBuf<T>::Pop(T& Item)
{
	if (!m_nItems)
		return(FALSE);
	Item = *m_pTail++;
	if (m_pTail == m_pEnd)
		m_pTail = m_pStart;
	InterlockedDecrement(&m_nItems);
	return(TRUE);
}
