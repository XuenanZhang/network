#ifndef _MUTEX_H
#define _MUTEX_H

#ifndef NULL
#define NULL 0
#endif

class MyLock
{
public:
	void * m_Lock;
public:
	MyLock();
	~MyLock();
	void Lock();
	void Unlock();
};

class Mutex
{
public:
	explicit Mutex ( MyLock * pLock) : m_pLock(pLock)
	{
		if ( m_pLock != NULL )
			m_pLock->Lock();
	}

	~Mutex()
	{
		if ( m_pLock != NULL )
			m_pLock->Unlock();
	}

private:
	MyLock * m_pLock;
};

#define  CREATE_MUTEX(pLock) Mutex mutex((pLock));
#define  ENTER_MUTEX(pLock) { Mutex mutex((pLock));
#define  EXIT_MUTEX() }

#endif