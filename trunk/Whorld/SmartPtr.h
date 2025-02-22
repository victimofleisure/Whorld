// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03jan08	initial version
        01      22feb25	replace header guard, rename vars

        minimal smart pointer
 
*/

#pragma once

// WARNING: CSmartPtr only works with dynamic pointers, i.e. the instance
// must point to an object on the heap that was allocated via new. If the
// object resides anywhere else, e.g. on the stack, errors will occur. Do
// not attempt to delete the object independently; once you've assigned a
// pointer to a CSmartPtr, the target object is owned by CSmartPtr and is
// automatically deleted by CSmartPtr's destructor. If you wish to regain
// ownership of the object, and delete it yourself, use Detach.

template<class T> class CSmartPtr : public WObject {	// copying NOT allowed
public:
// Construction
	CSmartPtr();
	CSmartPtr(T *pObj);
	~CSmartPtr();

// Accessors
    void	SetPtr(T* pObj);
    T*		GetPtr() const;

// Operations
    void	Attach(T* pObj);
	T*		Detach();
	void	Reset();

// Operators
	T&		operator*() const;
    T*		operator->() const;
	operator T*() const;
	operator bool() const;

protected:
// Member data
	T		*m_pObj;
};

template<class T> 
CSmartPtr<T>::CSmartPtr()
{
	m_pObj = NULL;
}

template<class T> 
CSmartPtr<T>::CSmartPtr(T *pObj)
{
	m_pObj = pObj;
}

template<class T> 
CSmartPtr<T>::~CSmartPtr()
{
	if (m_pObj != NULL)
		delete m_pObj;
}

template<class T> 
void CSmartPtr<T>::SetPtr(T* pObj)
{
	if (m_pObj != NULL)
		delete m_pObj;
	m_pObj = pObj;
}

template<class T>
T* CSmartPtr<T>::GetPtr() const
{
	return(m_pObj);
}

template<class T> 
void CSmartPtr<T>::Attach(T* pObj)
{
	SetPtr(pObj);
}

template<class T> 
T* CSmartPtr<T>::Detach()
{
	T*		pObj = m_pObj;
	m_pObj = NULL;
	return(pObj);	// caller is now responsible for deleting
}

template<class T> 
void CSmartPtr<T>::Reset()
{
	SetPtr(NULL);
}

template<class T> 
T& CSmartPtr<T>::operator*() const
{
	ASSERT(m_pObj != NULL);
	return(*m_pObj);
}

template<class T> 
T* CSmartPtr<T>::operator->() const
{
	ASSERT(m_pObj != NULL);
	return(m_pObj);
}

template<class T>
CSmartPtr<T>::operator T*() const
{
	return(m_pObj);
}

template<class T> 
CSmartPtr<T>::operator bool() const
{
	return(m_pObj != NULL);
}
