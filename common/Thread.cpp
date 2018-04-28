#if defined(_WINDOWS)
#include <windows.h>
#include <process.h>
#elif defined(_LINUX)
#include <pthread.h>
#include <signal.h>
#endif

#include "Thread.h"
#include "Mutex.h"

unsigned int g_QuitThreadCount = 0;
MyLock g_thread_lock;

Thread::Thread()
{
	m_TID = 0;
	m_Status = Thread::READY;

#ifdef _WINDOWS
	m_hThread = 0;
#endif
}

Thread::~Thread()
{
}

void Thread::start()
{
	if ( m_Status != Thread::READY)
		return;
	setStatus(Thread::STARTING);
#if defined(_LINUX)
	 pthread_create( &m_TID, NULL , MyThreadProcess , this );
#elif defined(_WINDOWS)
	//param : 1、内核对象安全属性（null为默认值）2、线程栈空间大小（默认1MB）3、函数地址4、函数参数5、额外标志，为0线程创建立即执行，
	//如果为CREATE_SUPENDED则表示线程创建后暂停运行，知道调用ResumeThread() 6、返回线程id
	m_hThread = (HANDLE)_beginthreadex( NULL, 0, MyThreadProcess, this, 0, &m_TID);
#endif
}


void Thread::exit( void * retval)
{
	try
	{
#if defined(_LINUX)
		pthread_exit( retval );
#elif defined(_WINDOWS)
		::CloseHandle( m_hThread ) ;
		m_hThread = NULL;
#endif
	}
	catch (...)
	{
	}
}

#if defined(_LINUX)
void * MyThreadProcess( void * derivedThread )
{
	sigset_t mask;
	sigfillset(&mask);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);
	Thread * thread = (Thread *)derivedThread;
	if( thread==NULL )
		return NULL;

	thread->setStatus(Thread::RUNNING);

	thread->run();

	thread->setStatus(Thread::EXIT);

	thread->exit(NULL);

	thread->onExit();

	g_thread_lock.Lock() ;
	g_QuitThreadCount++ ;
	g_thread_lock.Unlock() ;

	return NULL;
}
#elif defined(_WINDOWS)
unsigned int __stdcall MyThreadProcess( void * d )
{
	Thread * thread = ( Thread * ) d;
	if ( thread == NULL )
		return 0;
	
	if ( thread->getStatus() != Thread::STARTING )
		return 0;

	thread->setStatus(Thread::RUNNING);

	thread->run();

	thread->setStatus(Thread::EXIT);

	thread->exit(NULL);

	thread->onExit();

	g_thread_lock.Lock() ;
	g_QuitThreadCount++ ;
	g_thread_lock.Unlock() ;
	return 0;
}
#endif


void Thread::waitForExit()
{
#if defined(_LINUX)
	pthread_join(m_TID, NULL);
#endif

#if defined(_WINDOWS)
	if(m_hThread != NULL)
	{
		//等待函数-使线程进入等待状态，直到线程结束，知道制定的内核对象被触发，参数2为等待最长时间(毫秒），成功返回WAIT_OBJECT_0
		WaitForSingleObject(m_hThread, INFINITE);
		m_hThread = NULL;
	}
#endif
}

