/************************************************************************/
/*                            对象池                                    */
/************************************************************************/

#ifndef _POOL_H_
#define _POOL_H_

#include "Mutex.h"

template <class T>
class Pool
{
public:
	Pool()
	{
		_dict = NULL;
		_maxCount = -1;
		_position = -1;
		_addCount = 0;
		_isLock = false;
	}
	
	~Pool()
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
		_position = 0;
		_dict = new T *[maxCount];

		for (int i = 0; i < _maxCount; ++i)
		{
			_dict[i] = new T;
			if ( _dict[i] == NULL ) return false;
		}

		return true;
	}

	/**
	 * 从池中获取一个新的对象
	 */
	T * newObj()
	{
		CREATE_MUTEX(_isLock ? &_lock : NULL);

		if (_position >= _maxCount)
		{
			if (_addCount <= 0 )
				return NULL;
			else
			{
				if ( !resize(_maxCount + _addCount) )
					return NULL;
			}
		}

		if (_position >= _maxCount || _position < 0 )
			return NULL;

		T * pObj = _dict[_position];
		pObj->setPoolId(_position);
		pObj->initByPool();
		++_position;

		return pObj;
	}

	/**
	 * 归还池中的一个对象
	 */
	void releaseObj( T * pObj)
	{
		CREATE_MUTEX(_isLock ? &_lock : NULL);

		if ( pObj == NULL || _position <= 0 ) return;

		const int delIndex = pObj->getPoolId();
		if ( delIndex >= _position || delIndex < 0 ) return;
		--_position;
		_dict[delIndex] = _dict[_position];
		_dict[_position] = pObj;

		_dict[delIndex]->setPoolId(delIndex);
		pObj->setPoolId(-1);
		pObj->releaseByPool();

		return;
	}

	/**
	 * 重置池大小
	 */
	bool resize ( const int newMaxCount )
	{
		if ( newMaxCount <= _maxCount || ( _addMaxCount != 0 && newMaxCount > _addMaxCount ) ) return false;

		T ** pTemp = new T *[newMaxCount];

		if ( pTemp == NULL ) return false;

		for ( int i = 0; i < newMaxCount; ++i)
		{
			if ( i < _maxCount )
			{
				pTemp[i] = _dict[i];
			}
			else
			{
				pTemp[i] = new T;
				if ( pTemp[i] == NULL)
				{
					delete [] pTemp;
					pTemp = NULL;
					return false;
				}
			}
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
	 * 删除池中所有对象
	 */
	void clear()
	{
		if (_dict != NULL )
		{
			for (int i = 0; i < _maxCount; ++i)
			{
				if (_dict[i] != NULL)
				{
					delete _dict[i];
					_dict[i] = NULL;
				}
			}

			delete [] _dict;
			_dict = NULL;
		}

		_maxCount = -1;
		_position = -1;
	}

	/**
	 * 当前位置
	 */
	inline int getPosition() const
	{
		return _position;
	}

	/**
	 * 获得总数
	 */
	inline int getMaxCount() const
	{
		return _maxCount;
	}

private:
	T ** _dict;
	int _maxCount;
	int _position;
	int _addCount;
	int _addMaxCount;
	int _defaultMaxCount;
	bool _isLock;

	MyLock _lock;
};

#endif 