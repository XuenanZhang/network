#ifndef __MYSORT_H__
#define __MYSORT_H__

namespace MySort
{
#define OPEN_PRINT true

	template<typename T>
	inline void swap( T arr[], int i, int j)
	{
		T temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
	}

	template<typename T>
	inline void print( T arr[], int len)
	{
		if ( OPEN_PRINT == false )
			return;

		for ( int i = 0; i < len; ++i)
		{
			std::cout<< arr[i] << "  ";
		}
		std::cout<<std::endl;
	}

	/** 
	 * 直接插入排序: 
	 * 每次从数列中取一个还没有取出过的数，并按照大小关系插入到已经取出的数中使得已经取出的数仍然有序
	 */
	template<typename T>
	void insertSort( T * arr, T  len)
	{
		T temp;
		int pos;

		for ( int i = 1; i < len; ++i)
		{
			temp = arr[i];
			pos = i - 1;
			while( pos >= 0 && temp < arr[pos] )
			{
				arr[pos + 1] = arr[pos];
				--pos;
			}
			print(arr, len);
			arr[pos+1] = temp;
		}
	}

	/** 
	 *	希尔排序: 
	 *	shell排序基本思路是：先取一个小于n的整数d1作为第一个增量，把文件的全部记录分成d1个组。所有距离为dl的倍数的记录放在同一个组中。
	 *	先在各组内进行直接插入排序；然后，取第二个增量d2<d1重复上述的分组和排序，直至所取的增量dt=1(dt<dt-l<…<d2<d1)，即所有记录放在同一组中进行直接插入排序为止。
	 */
	template<typename T>
	void shellSort( T arr[], int  len )
	{
		T temp;
		int pos;
		int d = len; //len >> 1

		do 
		{
			d = d/3 + 1;
			for ( int i = d; i < len; ++i )
			{
				temp = arr[i];
				pos = i - d;

				while ( pos >= 0 && temp < arr[pos] )
				{
					arr[pos + d] = arr[pos];
					pos -= d;
				}

				arr[pos + d] = temp;
			}
		} while ( d > 1);

		/*
		while( d>= 1 )
		{
			for ( int i = d; i < len; ++i )
			{
				temp = arr[i];
				pos = i - d;
				while ( pos >= 0 && temp < arr[pos] )
				{
					arr[pos + d] = arr[pos];
					pos -= d;
				}

				arr[pos + d] = temp;
			}
			print(arr, len);
			d >>= 1;
		}*/
	}

	/**
	 * 选择排序:
	 * 每次从数列中找出一个最小的数放到最前面来，再从剩下的n-1个数中选择一个最小的，不断做下去。
	 */
	template<typename T>
	void selectSort( T arr[], int  len )
	{
		T temp;
		int pos, j;

		for ( int i = 0; i < len; ++i )
		{
			temp = arr[i];
			pos = i;

			for ( j = i + 1; j < len; ++j )
			{
				if ( temp > arr[j] )
				{
					temp = arr[j];
					pos = j;
				}
			}

			if ( pos != i )
			{
				arr[pos] = arr[i];
				arr[i] = temp;
			}
			print(arr, len);
		}
	}

	/** 堆插入排序 **/
	template<typename T>
	void heapSort( T arr[], int  len )
	{
		int n = (len - 1) / 2;
		for ( int i = n; i >= 0; --i )
		{
			heapAdjust(arr, i, len);
		}

		print(arr, len);

		for ( int i = len - 1; i >= 0; --i )
		{
			swap(arr, i, 0);
			heapAdjust(arr, 0, i);
			print(arr, i);
		}

		print(arr, len);
	}

	template<typename T>
	static void heapAdjust( T arr[], int pos, int  len )
	{
		T temp = arr[pos];
		int child = pos * 2 + 1;

		while( child < len )
		{
			if ( child + 1 < len && arr[child] < arr[child + 1] )
				++child;

			if ( temp < arr[child] )
			{
				arr[pos] = arr[child];
				pos = child;
				child = pos * 2 + 1;
			}
			else
				break;
		}

		arr[pos] = temp;
	}


	/** 冒泡排序 **/
	template<typename T>
	void bubbleSort( T arr[], int  len )
	{
		T temp;
		bool flag = false;
		for ( int i = 0; i < len; ++i )
		{
			for (int j = len - 1; j >= i; --j )
			{
				if ( arr[j] < arr[j-1])
				{
					swap(arr, j, j-1);
					flag = true;
				}
			}
			print(arr, len);

			if ( !flag ) return;
		}
	}

	/** 快速排序 1 **/
	template<typename T>
	void quickSort( T arr[], int len )
	{
		quickSplit(arr, 0, len-1 );
	}

	template<typename T>
	void quickSplit(T arr[], int left, int right )
	{
		if ( left >= right )
			return;

		T temp = arr[left];

		int i = left;
		int j = right;

		while ( i < j )
		{
			while( i < j && arr[j] >= temp )
				--j;
			while( i < j && arr[i] <= temp )
				++i;

			if ( i < j )
				swap(arr, i, j);
		}

		if ( left != i )
		{
			arr[left] = arr[i];
			arr[i] = temp;
		}

		quickSplit(arr, left, i - 1);
		quickSplit(arr, i + 1, right );
		
	}
	/** 快速排序 2 **/
	template<typename T>
	void quickSort2( T arr[], int len )
	{
		quickSplit(arr, 0, len-1 );
	}

	template<typename T>
	void quickSplit2(T arr[], int left, int right )
	{
		T middle = arr[ (right + left) / 2 ];

		int i = left;
		int j = right;

		while( i < j )
		{
			while( i < right && arr[i] < middle )
				i++;

			while( j > left && arr[j] > middle )
				j--;

			if ( i <= j )
			{
				if ( i != j )
					swap(arr, i, j);
				i++;
				j--;
			}
		}

		if ( left < j )
			quickSplit( arr, left, j);
		if ( i < right )
			quickSplit(arr, i, right);

		print(arr,10);
	}


}



#endif// __MYSORT_H__
