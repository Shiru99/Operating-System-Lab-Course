#include "alloc.h"

/* Code to allocate page of 4KB size with mmap() call and
* initialization of other necessary data structures.
* return 0 on success and 1 on failure (e.g if mmap() fails)
*/

int init()
{
	MemAddress = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, -1, 0);
	sizeLeft = PAGESIZE;
	// currentAddress = MemAddress;
	return 0;
}

/* optional cleanup with munmap() call
* return 0 on success and 1 on failure (if munmap() fails)
*/
int cleanup()
{
	int mun = munmap(MemAddress, PAGESIZE);
	if (mun == 0)
	{
		sizeLeft = 0;
	}
	return mun;
}

/* Function to allocate memory of given size
* argument: bufSize - size of the buffer
* return value: on success - returns pointer to starting address of allocated memory
*               on failure (not able to allocate) - returns NULL
*/
char *alloc(int bufSize)
{
	int sizeTobeGiven;
	if (bufSize % MINALLOC == 0)
	{
		sizeTobeGiven = bufSize;
	}
	else
	{
		sizeTobeGiven = ((bufSize % MINALLOC) + 1) * MINALLOC;
	}

	MaxFreeSpace ds = findMaxFreespace();
	if (sizeTobeGiven > ds.space)
	{
		return NULL;
	}

	int tempPosition = ds.position;
	Heads[tempPosition].size = sizeTobeGiven;
	Heads[tempPosition].address = MemAddress + ds.position * MINALLOC;
	while (sizeTobeGiven)
	{
		myfree[tempPosition++] = 0;
		sizeTobeGiven = sizeTobeGiven - MINALLOC;
	}

	return MemAddress + ds.position * MINALLOC;
}

/* Function to free the memory
* argument: takes the starting address of an allocated buffer
*/
void dealloc(char *memAddr)
{
	int size = findSize(memAddr);
	int freeIndex = (memAddr - MemAddress) / MINALLOC;
	while (size)
	{
		myfree[freeIndex++] = 0;
		size = size - MINALLOC;
	}
}
