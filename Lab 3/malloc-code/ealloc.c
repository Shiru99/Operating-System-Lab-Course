#include "ealloc.h"

#define ll long long
int PAGEUSED = 0;

ll validMemory[4][AVAILABLE];
char *allocation[4];
ll startAddr;

void init_alloc()
{
    // allocation = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    /*
        mmap    -   We are assuming that the heap is built within some free space acquired via a call to the system call mmap()  

        PROT_READ   - 	Pages may be read.
        PROT_WRITE  -   Pages may be written.
        MAP_PRIVATE -   Create a private copy-on-write mapping. Stores to the region do not affect the original file. It is unspecified whether changes made to the file after the mmap() call are visible in the mapped region.
        MAP_ANONYMOUS - The mapping is not backed by any file; the fd and offset arguments are ignored. The use of this flag in conjunction with MAP_SHARED is only supported on Linux since kernel 2.4.
    */

    // if (allocation == MAP_FAILED)
    //     return ERRORCODE;
    // startAddr = (ll)allocation;
    memset(validMemory, (ll)_INIT, sizeof(validMemory));

    // memset() is used to fill a block of memory with a particular value (-1)

    // return SUCCESSCODE;
}

void cleanup()
{
    memset(validMemory, (ll)_INIT, sizeof(validMemory));
}

char *alloc(int _size)
{
    if (_size % MINALLOC != 0)
        return NULL;

    if (allocation[0] == NULL)
    {
        allocation[0] = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        PAGEUSED++;
    }

    int slots = _size / MINALLOC;

    int slot = 0;
    int pageNumber = 0;
    ll index = -1;
    bool isAvailableOnPage = false;

    while (pageNumber < PAGEUSED)
    {
        bool isContinuos = false;
        for (int i = 0; i < AVAILABLE; i++)
        {
            if (validMemory[pageNumber][i] == _INIT)
            {
                if (!isContinuos)
                    index = i;
                isContinuos = true;
                if (++slot == slots)
                {
                    isAvailableOnPage = true;
                    break;
                }
            }
            else
            {
                slot = 0;
                isContinuos = false;
            }
        }

        if (isAvailableOnPage)
        {
            break;
        }
        pageNumber++;
        index = -1;
        slot = 0;
    }

    if (!isAvailableOnPage && PAGEUSED == MAXPAGE)
    {
        return NULL;
    }
    else if (!isAvailableOnPage && PAGEUSED < MAXPAGE)
    {
        allocation[PAGEUSED] = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
        pageNumber = PAGEUSED++;
        index = 0;
    }
    
    ll retAddress = (ll)(allocation[pageNumber] + index * MINALLOC);
    for (int i = 0; i < slots; i++)
    {
        validMemory[pageNumber][index + i] = retAddress;
    }
    return (char *)retAddress;
}

void dealloc(char *addr)
{
    for (int i = 0; i < PAGEUSED*AVAILABLE; i++)
    {
        if (validMemory[i/AVAILABLE][i%AVAILABLE] == (ll)addr)
        {
           validMemory[i/AVAILABLE][i%AVAILABLE]= _INIT;
        }
    }
    return;
}