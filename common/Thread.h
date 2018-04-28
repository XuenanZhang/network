/****************************************************************************/
/*								线程类										*/
/****************************************************************************/

#ifndef __THREAD_H__
#define __THREAD_H__


#if defined(_WINDOWS)
typedef unsigned int TID;
typedef void * HANDLE;
#elif defined(_LINUX)
typedef pthread_t TID;
#endif


class Thread
{
public:
	enum ThreadStatus
	{
		READY,
		STARTING,
		RUNNING,
		EXITING,
		EXIT
	};

public:
	Thread();
	virtual ~Thread();

public:
	/** 线程开始 **/
	void start();


protected:
	/** 线程退出 **/
	void exit ( void * retval = 0);

	/** 线程开始运行 **/
	virtual void run() = 0;

	void waitForExit();

	/** 线程退出之后调用 **/
	virtual void onExit(){};


public:
	inline TID getTID() { return m_TID; };

	inline ThreadStatus getStatus() { return m_Status;};

	inline void setStatus( ThreadStatus status ){ m_Status = status;};

#if defined(_WINDOWS)
	inline HANDLE getHandle(){return m_hThread;};
#endif


private:

	TID m_TID;

	ThreadStatus m_Status;

#if defined(_WINDOWS)
	HANDLE m_hThread;
#endif

private:
#if defined(_WINDOWS)
	friend unsigned int __stdcall MyThreadProcess ( void * d );
#elif defined(_LINUX)
	friend void * MyThreadProcess ( void * d );
#endif

};

extern unsigned int g_QuitThreadCount;

#if defined(_WINDOWS)
unsigned int __stdcall MyThreadProcess ( void * d );
#elif defined(_LINUX)
void * MyThreadProcess ( void * d );
#endif

#endif
