/************************************************************************/
/*                            环形数据结构                               */
/************************************************************************/

#ifndef _RINGQUEUE_H_
#define _RINGQUEUE_H_

#include "Mutex.h"

template <class T>
class RingQueue
{
public:
	RingQueue()
	{
		_dict = NULL;
		_maxCount = -1;
		_addCount = 0;
		_isLock = false;
	}
	
	~RingQueue()
	{

	}

	/**
	 * 初始化池 
	 */
	bool init( const int maxCount, const int addCount = 0, bool isLock = false, const int addMaxCount = 0 )
	{
		_isLock = isLock;
		if (maxCount <= 0 ) return false;

		_maxCount = maxCount;
		_defaultMaxCount = maxCount;
		_addCount = addCount;
		if (_addCount < 0) _addCount = 0;
		_addMaxCount = addMaxCount;
		_headIndex = 0;
		_tailIndex = 0;
		_dict = new T *[maxCount];

		for (int i = 0; i < _maxCount; ++i)
		{
			_dict[i] =  NULL;
		}

		return true;
	}

	/**
	 *  放入队列
	 */
	bool push( T * pObj)
	{
		CREATE_MUTEX(_isLock ? &_lock : NULL);
	

		if (_headIndex == _tailIndex && _dict[_headIndex] )
		{
			if (_addCount <= 0 )
				return false;
			else
			{
				if ( !resize(_maxCount + _addCount) )
					return false;
			}
		}

		_dict[_headIndex] = pObj;
		++_headIndex;
		if ( _headIndex >= _maxCount )
			_headIndex = 0;

		return true;
	}

	/**
	 * 从队列中取出
	 */
	T * pop()
	{
		CREATE_MUTEX(_isLock ? &_lock : NULL);

		T * pObj = _dict[_tailIndex];

		if ( !pObj ) return NULL;
			

		_dict[_tailIndex] = NULL;
		++_tailIndex;
		
		if (_tailIndex >= _maxCount) 
			_tailIndex = 0;

		return pObj;
	}

	/**
	 * 重置队列大小
	 */
	bool resize ( const int newMaxCount )
	{
		if ( newMaxCount <= _maxCount || ( _addMaxCount != 0 && newMaxCount > _addMaxCount ) ) return false;

		T ** pTemp = new T *[newMaxCount];

		if ( pTemp == NULL ) return false;
		int i = 0;
		do 
		{
			pTemp[i] = _dict[_tailIndex];
			++i;
			++_tailIndex;

			if (_tailIndex >= _maxCount) 
				_tailIndex = 0;

		} while (_headIndex != _tailIndex);
		
		_headIndex = _maxCount + 1;
		_tailIndex = 0;

		for ( ; i < newMaxCount; ++i)
		{
			pTemp[i] = NULL;
		}

		delete [] _dict;
		_dict = pTemp;
		_maxCount = newMaxCount;
		

		return true;
	}

	bool resetDefaultSize()
	{
		if ( _maxCount <= _defaultMaxCount ) return false;

		clear();

		return init(_defaultMaxCount, _addCount, _isLock, _addMaxCount);
	}

	/**
	 * 清理队列（不删除对象）;
	 */
	void clear()
	{
		if (_dict)
		{
			delete [] _dict;
			_dict = NULL;
		}

		_maxCount = 0;
		_defaultMaxCount = 0;
		_addCount = 0;
		_addMaxCount = 0;
		_headIndex = 0;
		_tailIndex = 0;
	}

	/**
	 * 净化队列（不删除对象）;
	 */
	void purge()
	{
		for (int i = 0; i < _maxCount; ++i)
		{
			_dict[i] =  NULL;
		}

		_headIndex = 0;
		_tailIndex = 0;
	}

	/**
	 * 获得总数
	 */
	inline int getMaxCount() const
	{
		if (_headIndex > _tailIndex)
			return _headIndex - _tailIndex;
		else if (_headIndex < _tailIndex)
			return _tailIndex - _headIndex;
		else
		{
			if (_dict[_tailIndex])
				return _maxCount;
			else
				return 0;
		}
	}

private:
	T ** _dict;
	int _maxCount;
	int _headIndex;
	int _tailIndex;
	int _addCount;
	int _addMaxCount;
	int _defaultMaxCount;
	bool _isLock;

	MyLock _lock;
};

#endif 