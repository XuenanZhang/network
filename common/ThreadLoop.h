/************************************************************************/
/*                           循环调用线程类				                */
/************************************************************************/

#ifndef _DB_THREADLOOP_H_
#define _DB_THREADLOOP_H_

#include "Thread.h"
#include "CommonDefine.h"

class ThreadLoop : public Thread
{
public:
	ThreadLoop();
	virtual ~ThreadLoop();

protected:
	virtual void run() final;

	/** 第一次开始运行 **/
	virtual bool onStartRunning(){ return true; };

	/** 
	 * 线程运行中循环调用函数
	 * t : 流逝时间
	 * 返回值为false则退出线程
	 */
	virtual bool onTick(unsigned int t) = 0;

	/**
	 * 每次调用tick间隔
	 */
	virtual unsigned int getTickTime() { return FRAME_RATE;};

private:
	unsigned int _lastTime;
};
#endif