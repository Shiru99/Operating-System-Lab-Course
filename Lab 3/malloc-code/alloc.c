#include "alloc.h"

#define ll long long

ll validMemory[AVAILABLE];
char *allocation;
ll startAddr;

int init_alloc()
{
    allocation = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    /*
        mmap    -   We are assuming that the heap is built within some free space acquired via a call to the system call mmap()  

        PROT_READ   - 	Pages may be read.
        PROT_WRITE  -   Pages may be written.
        MAP_PRIVATE -   Create a private copy-on-write mapping. Stores to the region do not affect the original file. It is unspecified whether changes made to the file after the mmap() call are visible in the mapped region.
        MAP_ANONYMOUS - The mapping is not backed by any file; the fd and offset arguments are ignored. The use of this flag in conjunction with MAP_SHARED is only supported on Linux since kernel 2.4.
    */

    if (allocation == MAP_FAILED)
        return ERRORCODE;
    startAddr = (ll)allocation;
    memset(validMemory, (ll)_INIT, sizeof(validMemory));
    // memset() is used to fill a block of memory with a particular value (-1)

    return SUCCESSCODE;
}

int cleanup()
{
    ll err = munmap(allocation, PAGESIZE);
    // munmap - system call deletes the mappings for the specified address range
    memset(validMemory, (ll)_INIT, sizeof(validMemory));
    return err;
}

char *alloc(int _size)
{
    if (_size % MINALLOC != 0)
        return NULL;

    int slots = _size / MINALLOC;
    int slot = 0;

    bool isContinuos = false;
    ll index = -1;

    for (int i = 0; i < AVAILABLE; i++)
    {
        if (validMemory[i] == _INIT)
        {
            if (!isContinuos)
                index = i;
            isContinuos = true;
            if (++slot == slots)
            {
                break;
            }
        }
        else
        {
            slot = 0;
            isContinuos = false;
        }
    }
    if (slot < slots)
        return NULL;

    ll retAddress = (ll)(allocation + index * MINALLOC);
    for (int i = 0; i < slots; i++)
    {
        validMemory[index + i] = retAddress;
    }
    return (char *)retAddress;
}

void dealloc(char *addr)
{
    for (int i = 0; i < AVAILABLE; i++)
    {
        if (validMemory[i] == (ll)addr)
        {
            validMemory[i] = _INIT;
        }
    }
    return;
}