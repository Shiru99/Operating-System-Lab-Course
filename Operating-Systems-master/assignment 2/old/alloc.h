#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8    //allocations will be 8 bytes or multiples of it
const int WORSTCASE = PAGESIZE / MINALLOC;

char *MemAddress = NULL;
int sizeLeft = 0;

typedef struct Memory Memory;
struct Memory
{
  int size;
  char *address;
  // Memory *nextHead;
  // Memory *prevHead;
};
// char *currentAddress;
// int Headctr = 0;
// all are free initially

typedef struct MaxFreeSpace MaxFreeSpace;
struct MaxFreeSpace
{
  int space;
  int position; //array position
};

Memory Heads[WORSTCASE] = {0, NULL}; // to store the size
int myfree[WORSTCASE] = {MINALLOC};

MaxFreeSpace findMaxFreespace()
{
  int max_so_far = 0, max_ending_here = 0;
  int position = 0, tempPosition = 0;
  for (size_t i = 0; i < WORSTCASE; i++)
  {
    max_ending_here = max_ending_here + myfree[i];
    tempPosition = i;
    if (max_so_far < max_ending_here)
    {
      max_so_far = max_ending_here;
      position = i;
    }
  }
  MaxFreeSpace temp = {max_so_far, position + 1 - max_so_far / MINALLOC};
  return temp;
}

int findSize(char *query)
{
  for (size_t i = 0; i < WORSTCASE; i++)
  {
    if (Heads[i].address == query)
    {
      return Heads[i].size;
    }
  }
  return -1;
}

// function declarations
int init();
int cleanup();
char *alloc(int);
void dealloc(char *);
