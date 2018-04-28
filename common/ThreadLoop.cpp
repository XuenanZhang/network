#include "TimeManager.h"
#include "Utils.h"
#include "ThreadLoop.h"


ThreadLoop::ThreadLoop()
{
	_lastTime = 0;
}

ThreadLoop::~ThreadLoop()
{
}

void ThreadLoop::run()
{
	if ( !onStartRunning() )
		return;

	_lastTime = TimeManager::Ptr()->currentTime();

	unsigned int nowTime;
	unsigned int elapseTime;
	unsigned int intervalTime = getTickTime();

	while (true)
	{
		if (getStatus() != RUNNING )
			break;

		nowTime = TimeManager::Ptr()->currentTime();
		elapseTime = nowTime - _lastTime;

		if ( elapseTime < intervalTime)
			Utils::sleep(intervalTime - elapseTime);

		if (!onTick(elapseTime))
			break;
	}
}

