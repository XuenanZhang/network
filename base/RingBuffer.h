/************************************************************************/
/*			环形buffer（write一个线程，read copy remove一个线程			*/
/************************************************************************/
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

class RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();

public:
	bool initSize(int size);
	bool write( char * data, int dataSize);
	bool read( char * data, int dataSize, int readSize);
	bool copy( char * data, int dataSize, int copySize);
	bool remove( int size);
	int getUsedSize();
	int getFreeSize();
	bool isEmpty();
	void clear();
	char * _buffer;
private:
	bool readAndCopyData(char * data, int dataSize, int size, bool isAddReadPos);

private:
	
	unsigned int _size;
	unsigned int _readPos;
	unsigned int _writePos;
};


#endif