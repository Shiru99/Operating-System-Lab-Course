#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include<stdbool.h>  

#define PAGESIZE 4096   //  size of memory to allocate from OS
#define MINALLOC 8      //  allocations will be 8 bytes or multiples of it
#define AVAILABLE 512   //  
#define _INIT -1
#define ERRORCODE -1
#define SUCCESSCODE 0

// function declarations
int init_alloc();
int cleanup();
char *alloc(int);
void dealloc(char *);