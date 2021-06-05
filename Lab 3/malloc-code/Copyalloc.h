#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>


#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8 //allocations will be 8 bytes or multiples of it

// function declarations
int init_alloc();
int cleanup();
char *alloc(int allocSize);
void dealloc(char *deallocAddress);


struct node{
    int size;
    char* start;    
    struct node* prev;
    struct node* next;
};
typedef struct node block;

enum listType {FREE, ALLOC};
enum boolType {FALSE, TRUE};



void print_List(block* list, int listType);
void drawMap(block *list);
void draw(char c, int size);

block* new_Block(char *start, int size);
void add_Block(block** list, block* newBlock);
block* search_freeList(int size);

void meta_Init();
void* meta_Malloc(int size);
void meta_Free(char* blockAddress, int size);
void print_MetaPage();