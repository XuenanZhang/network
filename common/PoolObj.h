/************************************************************************/
/*                           池对象基类                                 */
/************************************************************************/

#ifndef _POOL_OBJ_H_
#define _POOL_OBJ_H_

class PoolObj
{
public:
	PoolObj() : _poolId(-1){};
	virtual ~PoolObj(){};

public:
	/**
	 *	此方法为内部方法，pooId会经常改变
	 */
	inline int getPoolId() const {return _poolId;}
	inline void setPoolId( const int poolId) {_poolId = poolId;}

	/**
	 *	对象从池中创建和删除时候触发
	 */
	virtual void initByPool() = 0;
	virtual void releaseByPool() = 0;

private:
	int _poolId;
};


#endif