// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03jan08	initial version
		01		14feb08	convert to a template
		02		28oct08	check for self-assignment

        reference-counting pointer
 
*/

#ifndef CREFPTR_INCLUDED
#define CREFPTR_INCLUDED

class CRefObj : public WObject {
public:
// Construction
	CRefObj();
	virtual	~CRefObj();	// so derived destructor gets called

// Attributes
	int		GetRefCount() const;

// Operations
	void	AddRef();
	void	Release();

protected:
// Member data
	int		m_RefCount;		// reference count
};

inline CRefObj::CRefObj()
{
	m_RefCount = 0;
}

inline CRefObj::~CRefObj()
{
}

inline void CRefObj::AddRef()
{
	m_RefCount++;
}

inline void CRefObj::Release()
{
	m_RefCount--;
	if (!m_RefCount)
		delete this;
}

inline int CRefObj::GetRefCount() const
{
	return(m_RefCount);
}

template<class T>
class CRefPtr : public WObject {
public:
// Construction
	CRefPtr();
	CRefPtr(T *Obj);
	~CRefPtr();
	CRefPtr(const CRefPtr& Ptr);
	CRefPtr& operator=(const CRefPtr& Ptr);

// Attributes
	T		*GetObj() const;
	void	SetObj(T *Obj);
	bool	IsEmpty() const;
	void	SetEmpty();

// Operations
	void	CreateObj();
	void	CreateObj(const T& Obj);
	void	Copy(const CRefPtr& Ptr);
	void	Serialize(CArchive& ar);

// Operators
	T	*operator->() const;
	operator T&() const;
	operator T*() const;

protected:
// Member data
	T		*m_Ptr;	// pointer to referenced object
};

template<class T>
inline CRefPtr<T>::CRefPtr()
{
	m_Ptr = NULL;
}

template<class T>
inline CRefPtr<T>::CRefPtr(T* Obj)
{
	m_Ptr = NULL;
	SetObj(Obj);
}

template<class T>
inline CRefPtr<T>::CRefPtr(const CRefPtr& Ptr)
{
	m_Ptr = NULL;
	Copy(Ptr);
}

template<class T>
inline CRefPtr<T>::~CRefPtr()
{
	SetEmpty();
}

template<class T>
inline void CRefPtr<T>::Copy(const CRefPtr& Ptr)
{
	SetObj(Ptr.m_Ptr);
}

template<class T>
inline CRefPtr<T>& CRefPtr<T>::operator=(const CRefPtr& Ptr)
{
	if (&Ptr != this)	// avoid self-assignment
		Copy(Ptr);
	return(*this);
}

template<class T>
void CRefPtr<T>::CreateObj()
{
	SetObj(new T);
}

template<class T>
void CRefPtr<T>::CreateObj(const T& Obj)
{
	SetObj(new T(Obj));
}

template<class T>
inline T *CRefPtr<T>::GetObj() const
{
	return(m_Ptr);
}

template<class T>
void CRefPtr<T>::SetObj(T *Obj)
{
	if (m_Ptr != NULL)
		m_Ptr->Release();
	m_Ptr = Obj;
	if (Obj != NULL)
		Obj->AddRef();
}

template<class T>
inline bool CRefPtr<T>::IsEmpty() const
{
	return(m_Ptr == NULL);
}

template<class T>
void CRefPtr<T>::SetEmpty()
{
	if (m_Ptr != NULL) {
		m_Ptr->Release();
		m_Ptr = NULL;
	};
}

template<class T>
inline T *CRefPtr<T>::operator->() const
{
	return(m_Ptr);
}

template<class T>
inline CRefPtr<T>::operator T&() const
{
	return(*m_Ptr);
}

template<class T>
inline CRefPtr<T>::operator T*() const
{
	return(m_Ptr);
}

template<class T>
inline void CRefPtr<T>::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		bool	GotObj = !IsEmpty();
		StoreBool(ar, GotObj);
		if (GotObj)
			m_Ptr->Serialize(ar);
	} else {
		bool	GotObj;
		LoadBool(ar, GotObj);
		if (GotObj) {
			CreateObj();
			m_Ptr->Serialize(ar);
		}
	}
}

#endif
