// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		16apr10		initial version
		01		05mar25		modernize style

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
	bool	IsEmpty() const;
	bool	IsFull() const;
	bool	IsBelowHalfFull() const;

// Operations
	bool	Push(const T& item);
	bool	Pop(T& item);

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
inline bool CILockRingBuf<T>::Push(const T& item)
{
	if (m_nItems >= m_nSize)
		return false;
	*m_pHead++ = item;
	if (m_pHead == m_pEnd)
		m_pHead = m_pStart;
	InterlockedIncrement(&m_nItems);
	return true;
}

template<class T>
inline bool CILockRingBuf<T>::Pop(T& item)
{
	if (!m_nItems)
		return false;
	item = *m_pTail++;
	if (m_pTail == m_pEnd)
		m_pTail = m_pStart;
	InterlockedDecrement(&m_nItems);
	return true;
}

template<class T>
inline int CILockRingBuf<T>::GetSize() const
{
	return m_nSize;
}

template<class T>
inline int CILockRingBuf<T>::GetCount() const
{
	return m_nItems;
}

template<class T>
inline bool CILockRingBuf<T>::IsEmpty() const
{
	return m_nItems <= 0;
}

template<class T>
inline bool CILockRingBuf<T>::IsFull() const
{
	return m_nItems >= m_nSize;
}

template<class T>
inline bool CILockRingBuf<T>::IsBelowHalfFull() const
{
	return m_nItems < m_nSize / 2;
}
