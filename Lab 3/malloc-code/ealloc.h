#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include<stdbool.h>  

//granularity of memory to mmap from OS
#define PAGESIZE 4096 

//minimum allocation size
#define MINALLOC 256
#define MAXPAGE 4

#define AVAILABLE 16   //  
#define _INIT -1


// function declarations to support
void init_alloc(void);
char *alloc(int);
void dealloc(char *);
void cleanup(void);


// https://github.com/preshen-goobiah/Dynamic-Memory-Allocator/blob/master/allocator.h