// Copyleft 2015 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24dec15	initial version

*/

template<class T>
class CSaveObj {
public:
	CSaveObj(T& obj);
	CSaveObj(T& obj, const T& objNew);
	~CSaveObj();
	T&	m_obj;		// reference to object
	T	m_objOld;	// object's original state
};

template<class T>
CSaveObj<T>::CSaveObj(T& obj) : m_obj(obj), m_objOld(obj) 
{
}

template<class T>
CSaveObj<T>::CSaveObj(T& obj, const T& objNew) : m_obj(obj), m_objOld(obj)
{
	obj = objNew;	// set temporary state
}

template<class T>
CSaveObj<T>::~CSaveObj()
{
	m_obj = m_objOld;	// restore original state
}
