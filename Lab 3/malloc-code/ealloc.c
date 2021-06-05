#include "ealloc.h"

long long memoryBluePrint[4][BLOCKSPERPAGE];
char *pageAddress[4];
int PAGEUSED = 0;

void init_alloc()
{
    memset(memoryBluePrint, (long long)EMPTY, sizeof(memoryBluePrint));
    // memset() is used to fill a block of memory with a particular value (0)
}

void cleanup()
{
    memset(memoryBluePrint, (long long)EMPTY, sizeof(memoryBluePrint));
    // memset() is used to fill a block of memory with a particular value (0)
}

char *alloc(int chunkSize)
{
    if (chunkSize % MINALLOC != 0)
        return NULL;

    if (pageAddress[0] == NULL)
    {
        pageAddress[0] = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        PAGEUSED++;

        /*
        mmap    -   We are assuming that the heap is built within some free space acquired via a call to the system call mmap()  

        PROT_READ   - 	Pages may be read.
        PROT_WRITE  -   Pages may be written.
        MAP_PRIVATE -   Create a private copy-on-write mapping. Stores to the region do not affect the original file. It is unspecified whether changes made to the file after the mmap() call are visible in the mapped region.
        MAP_ANONYMOUS - The mapping is not backed by any file; the fd and offset arguments are ignored. The use of this flag in conjunction with MAP_SHARED is only supported on Linux since kernel 2.4.
        */
    }

    int blocks = chunkSize / MINALLOC;

    int block = 0;
    int pageNumber = 0;
    int index = -1;
    bool isAvailableOnPage = false;

    /* heuristic - FIRST FIT */
    while (pageNumber < PAGEUSED)
    {
        bool isContinuos = false;
        for (int i = 0; i < BLOCKSPERPAGE; i++)
        {
            if (memoryBluePrint[pageNumber][i] == EMPTY)
            {
                if (!isContinuos)
                    index = i;
                isContinuos = true;
                if (++block == blocks)
                {
                    isAvailableOnPage = true;
                    break;
                }
            }
            else
            {
                block = 0;
                isContinuos = false;
            }
        }

        if (isAvailableOnPage)
            break;
        
        pageNumber++;
        index = -1;
        block = 0;
    }

    if (!isAvailableOnPage && PAGEUSED == MAXPAGES)
    {
        return NULL;
    }
    else if (!isAvailableOnPage && PAGEUSED < MAXPAGES)
    {
        pageAddress[PAGEUSED] = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        pageNumber = PAGEUSED++;
        index = 0;
    }

    long long addressForChunk = (long long)(pageAddress[pageNumber] + index * MINALLOC);
    for (int i = 0; i < blocks; i++)
    {
        memoryBluePrint[pageNumber][index + i] = addressForChunk;
    }
    return (char *)addressForChunk;
}

void dealloc(char *chunkAddress)
{

    bool found = false;
    for (int p = 0; p < PAGEUSED; p++)
    {
        for (int i = (chunkAddress - pageAddress[p]) / MINALLOC; i < BLOCKSPERPAGE; i++)
        {
            if (memoryBluePrint[p][i] == (long long)chunkAddress)
            {
                memoryBluePrint[p][i] = EMPTY;
                found = true;
            }
        }
        if (found)
            break;
    }

    // for (int i = 0; i < PAGEUSED*BLOCKSPERPAGE; i++)
    // {
    //     if (memoryBluePrint[i/BLOCKSPERPAGE][i%BLOCKSPERPAGE] == (long long)addr)
    //     {
    //        memoryBluePrint[i/BLOCKSPERPAGE][i%BLOCKSPERPAGE]= EMPTY;
    //     }
    // }
}