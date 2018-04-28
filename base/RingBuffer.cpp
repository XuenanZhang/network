#include "RingBuffer.h"
#include <stdio.h>
#include <string.h>
static const int Buffer_RWPos_Size = 2;//读写指针占位(为了读写永远不重合，比如5个字节buff，会创建数组大小7）

RingBuffer::RingBuffer():
_buffer(NULL),_size(0), _readPos(0),_writePos(1)
{

}

RingBuffer::~RingBuffer()
{
	if (_buffer)
	{
		delete [] _buffer;
		_buffer = NULL;
	}
}

bool RingBuffer::initSize(int size)
{
	if (_buffer) return false;

	_size = size + Buffer_RWPos_Size;
	_buffer = new char[_size];
	memset(_buffer, _size, 0 );

	return true;
}

bool RingBuffer::write( char * data, int dataSize)
{
	const int freeSize = getFreeSize();

	if (dataSize == 0) return true;
	if (freeSize < dataSize ) return false;

	const int readPos = _readPos;
	const int writePos = _writePos;

	if (readPos == writePos) return false;

	if ( writePos > readPos )
	{
		const int endLen = _size - writePos;

		if (endLen >= dataSize)
		{
			memcpy(_buffer + writePos, data, dataSize);
			_writePos = (writePos + dataSize)%_size;
		}
		else
		{
			memcpy(_buffer + writePos, data, endLen);
			const int secondLen = dataSize - endLen;
			memcpy(_buffer, data + endLen, secondLen);
			_writePos = secondLen;
		}
	}
	else
	{
		memcpy(_buffer + writePos, data, dataSize);
		_writePos = writePos + dataSize;
	}
	
	return true;
}

bool RingBuffer::read( char * data, int dataSize, int readSize)
{
	return readAndCopyData(data, dataSize, readSize, true);
}

bool RingBuffer::copy( char * data, int dataSize, int copySize)
{
	return readAndCopyData(data, dataSize, copySize, false);
}

bool RingBuffer::readAndCopyData(char * data, int dataSize, int size, bool isAddReadPos)
{
	const int usedSize = getUsedSize();

	if ( usedSize < size || !data || dataSize < size) return false;

	const int writePos = _writePos;
	const int readPos = _readPos;

	if (readPos == writePos) return false;
	
	if ( writePos > readPos )
	{
		memcpy( data, _buffer + readPos + 1, size);
		//if (isAddReadPos) _readPos = readPos + size;
	}
	else
	{
		const int endLen = _size - readPos - 1;

		if (endLen >= size)
		{
			memcpy(data, _buffer + readPos + 1, size);
			//if (isAddReadPos) _readPos = readPos + size;
		}
		else
		{
			if (endLen > 0 ) memcpy(data, _buffer + readPos + 1, endLen);
			const int secondLen = size - endLen;
			memcpy(data + endLen, _buffer, secondLen);
			//if (isAddReadPos) _readPos = secondLen - 1;
		}
	}

	if (isAddReadPos)_readPos = (readPos + size)%_size;

	return true;
}

bool RingBuffer::remove( int size)
{
	const int usedSize = getUsedSize();

	if ( usedSize < size ) 
	{
		clear();
		return true;
	}

	const int writePos = _writePos;
	const int readPos = _readPos;

	if (readPos == writePos) return false;

	_readPos = (readPos + size)%_size;

	return true;
}

int RingBuffer::getUsedSize()
{
	const int writePos = _writePos;
	const int readPos = _readPos;

	if (writePos > readPos)
		return writePos - readPos - 1;
	else if (writePos < readPos)
		return writePos - readPos - 1 + _size;

	printf("error writePos == readPos on RingBuffer");
	return 0;
}

int RingBuffer::getFreeSize()
{
	const int usedSize = getUsedSize();

	return _size - (usedSize + Buffer_RWPos_Size);
}

bool RingBuffer::isEmpty()
{
	return getUsedSize() == 0;
}

void RingBuffer::clear()
{
	_writePos = 1;
	_readPos = 0;
}
