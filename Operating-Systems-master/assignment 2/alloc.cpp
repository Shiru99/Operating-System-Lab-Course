#include <iostream>
#include "alloc.h"


/* Code to allocate page of 4KB size with mmap() call and
* initialization of other necessary data structures.
* return 0 on success and 1 on failure (e.g if mmap() fails)
*/
Node* head = NULL;
Node* unhead = NULL;
void push(struct Node** head_ref,char* addr,int len) 
	{ 
	    struct Node* nnode = (struct Node*) malloc(sizeof(struct Node)); 
	    nnode->length= len;
	    nnode->add=addr;
	    nnode->next = (*head_ref); 
	    (*head_ref)= nnode; 
	}
void deleteNode(struct Node **head_ref, char* addr) 
	{ 
	    struct Node* temp = *head_ref, *prev; 
	    if (temp != NULL && temp->add == addr) 
	    { 
	        *head_ref = temp->next;    
	        free(temp);               
	        return; 
	    } 
	    while (temp != NULL && temp->add != addr) 
	    { 
	        prev = temp; 
	        temp = temp->next; 
	    } 
	    if (temp == NULL) return; 
	    prev->next = temp->next;   
	    free(temp); 
	}
	char* abo;

int init()
{
	// Write your code below
	char* abo=(char*)mmap(NULL,PAGESIZE,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_ANON|MAP_PRIVATE,0,0);
	head  = (struct Node*)malloc(sizeof(struct Node)); 
	unhead  = (struct Node*)malloc(sizeof(struct Node)); 
	head->add=abo;
	head->length=PAGESIZE;
	head->next=NULL;
	if(abo==(void *) -1){
		return 1;}
	else 
		return 0;
}

/* optional cleanup with munmap() call
* return 0 on success and 1 on failure (if munmap() fails)
*/
int cleanup()
{
	int b=munmap(abo,PAGESIZE);
	// Write your code below
	if(b==-1)
		return 1;
	else 
		return 0;

  
}

/* Function to allocate memory of given size
* argument: bufSize - size of the buffer
* return value: on success - returns pointer to starting address of allocated memory
*               on failure (not able to allocate) - returns NULL
*/
char *alloc(int bufSize)
{
	Node* headi=head;
	// write your code below
	while(headi!=NULL){
		if(headi->length>=bufSize){
			push(& unhead,headi->add,bufSize);
			char* adsen=headi->add;
			headi->add= (headi->add)+(bufSize);
			headi->length=headi->length-bufSize;
			return adsen;
		}
		headi=headi->next;

	}
	return NULL;	
}


/* Function to free the memory
* argument: takes the starting address of an allocated buffer
*/
void dealloc(char *memAddr)
{
	// write your code below
	Node* headi=unhead;
	// write your code below
	while(headi!=NULL){
		if(headi->add!=memAddr){
			push(& head,headi->add,headi->length);
			break;
		}
		headi=headi->next;
	}
	deleteNode(&unhead,memAddr);

}
