/****************************************************************************
								µ¥ÀýÀà
****************************************************************************/

#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include "CommonDefine.h"
template<class T>
class Singleton
{
public:
	static T* pData;
	Singleton() {};
	virtual ~Singleton() {};
	static void	Release();
	static T* Ptr();
	static T& Ref();
};

template<class T>
T* Singleton<T>::pData = NULL;

template<class T>
T* Singleton<T>::Ptr()
{
	if(pData == NULL)
	{
		pData = new T;
	}
	return pData;
}

template<class T>
T& Singleton<T>::Ref()
{
	if(pData == NULL)
	{
		pData = new T;
	}
	return *pData;
}

template<class T>
void Singleton<T>::Release()
{
	if(pData != NULL)
	{
		delete pData; 
		pData = NULL;
	}
}

#define EXTEND_SINGLETON(T) public Singleton<T>

#endif


