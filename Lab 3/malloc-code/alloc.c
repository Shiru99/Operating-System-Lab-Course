#include "alloc.h"

long long memoryBluePrint[BLOCKSPERPAGE];
char *pageAddress;

int init_alloc()
{
    pageAddress = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    /*
        mmap    -   We are assuming that the heap is built within some free space acquired via a call to the system call mmap()  

        PROT_READ   - 	Pages may be read.
        PROT_WRITE  -   Pages may be written.
        MAP_PRIVATE -   Create a private copy-on-write mapping. Stores to the region do not affect the original file. It is unspecified whether changes made to the file after the mmap() call are visible in the mapped region.
        MAP_ANONYMOUS - The mapping is not backed by any file; the fd and offset arguments are ignored. The use of this flag in conjunction with MAP_SHARED is only supported on Linux since kernel 2.4.
    */

    if (pageAddress == MAP_FAILED)
        return ERRORCODE;

    memset(memoryBluePrint, (long long)EMPTY, sizeof(memoryBluePrint));
    // memset() is used to fill a block of memory with a particular value (0)

    return SUCCESSCODE;
}

int cleanup()
{
    int status = munmap(pageAddress, PAGESIZE);
    // munmap - system call deletes the mappings for the specified address range
    memset(memoryBluePrint, (long long)EMPTY, sizeof(memoryBluePrint));
    return status;
}

char *alloc(int chunkSize)
{
    if (chunkSize % MINALLOC != 0)
        return NULL;

    int blocks = chunkSize / MINALLOC;
    int block = 0;

    bool isContinuos = false;
    int index = -1; // position in memoryBluePrint for new block

    /* heuristic - FIRST FIT */
    for (int i = 0; i < BLOCKSPERPAGE; i++)
    {
        if (memoryBluePrint[i] == EMPTY)
        {
            if (!isContinuos)
                index = i;

            isContinuos = true;

            if (++block == blocks)
                break;
        }
        else
        {
            block = 0;
            isContinuos = false;
        }
    }
    
    if (block < blocks)
        return NULL;

    long long addressForChunk = (long long)(pageAddress + index * MINALLOC);

    for (int i = 0; i < blocks; i++)
        memoryBluePrint[index + i] = addressForChunk;

    return (char *)addressForChunk;
}

void dealloc(char *chunkAddress)
{
    for (int i = (chunkAddress - pageAddress) / MINALLOC; i < BLOCKSPERPAGE; i++)
    {
        if (memoryBluePrint[i] == (long long)chunkAddress)
        {
            memoryBluePrint[i] = EMPTY;
        }
        else
        {
            break;
        }
    }
}