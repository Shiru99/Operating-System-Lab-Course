#include "alloc.h"


block *freeList, *allocList;
char *page, *metaPage;
char metaPageStatus[PAGESIZE];



int init_alloc(){	
	
	metaPage = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if(metaPage){
		//printf("\n\n[MM] Meta_Page of size %d B allocated starting at (%u | %p).",PAGESIZE,metaPage,metaPage);
		meta_Init();
	}else{
		return -1;
	}

	page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if(page){		
		//printf("\n\n[MM] Page of size %d B allocated starting at (%u | %p).\n",PAGESIZE,page,page);	
		freeList = new_Block(page, PAGESIZE);	
		allocList = NULL;			
	}else{
		return -1;
	}
	
	return 0;
}


char *alloc(int allocSize){

	if(allocSize%8 == 0){	

		block* allocFrom = search_freeList(allocSize);
		if(allocFrom){

			char* allocAddress = allocFrom->start;
			allocFrom->start += allocSize;
			allocFrom->size -= allocSize;

			if(allocFrom->size == 0){

				if(allocFrom->prev || allocFrom->next){
					if(allocFrom->prev){
						allocFrom->prev->next = allocFrom->next;
					}
					if(allocFrom->next){
						allocFrom->next->prev = allocFrom->prev;
					}
				}else{
					freeList = NULL;
				}
				
				meta_Free(allocFrom,sizeof(block));
			}

			add_Block(&allocList,new_Block(allocAddress,allocSize));

			return allocAddress;
		}	
	}
	return NULL;
}


void dealloc(char *deallocAddress){

	block *temp, *deallocBlock;
	
	temp = allocList;
	while(temp){
		if(temp->start == deallocAddress){
			deallocBlock = temp;
			break;
		}
		temp = temp->next;
	}

	if(deallocBlock == NULL){
		return;
	}
	
	if(deallocBlock == allocList){
		allocList = deallocBlock->next;
		if(deallocBlock->next){
			deallocBlock->next->prev = NULL;
		}
	}else{
		block *curr = allocList;

		while(curr){
			if(curr == deallocBlock){				
				curr->prev->next = curr->next;

				if(curr->next){
					curr->next->prev = curr->prev;
				}			
			}
			curr = curr->next;
		}
	}

	deallocBlock->next = NULL;
	deallocBlock->prev = NULL;
	add_Block(&freeList, deallocBlock);
	return;
}


int cleanup(){
	return (munmap(page,PAGESIZE) | munmap(metaPage,PAGESIZE));
}


block* new_Block(char *startAddress, int size){

	block* temp = meta_Malloc(sizeof(block));
	
	temp->start = startAddress;
	temp->size = size;
	temp->prev = NULL;
	temp->next = NULL;

	//print_MetaPage();

	return temp;
}


void add_Block(block** list, block* newBlock){	

	if((*list) == NULL){

		(*list) = newBlock;

	}else if(newBlock->start < (*list)->start){

		//At the begining of list
		if(
			(*list) == freeList && 
			newBlock->start + newBlock->size == (*list)->start
		){
			//Merge
			(*list)->start = newBlock->start;
			(*list)->size += newBlock->size;

			meta_Free(newBlock,sizeof(block)); 
		}else{
			newBlock->next = (*list);
			(*list)->prev = newBlock;
			(*list) = newBlock;
		}	

	}else{
		block *curr, *prev;
		prev = (*list);
		curr = (*list)->next;

		while(curr){
			if(newBlock->start < curr->start){

				if((*list) == freeList){
					if(
						prev->start + prev->size == newBlock->start &&
						newBlock->start + newBlock->size == curr->start
					){
						prev->size += (newBlock->size + curr->size);
						prev->next = curr->next;
						if(curr->next){
							curr->next->prev =prev;
						}

						meta_Free(newBlock,sizeof(block));
						meta_Free(curr,sizeof(block));
						return;
					}else if(prev->start + prev->size == newBlock->start){

						prev->size += newBlock->size;

						meta_Free(newBlock,sizeof(block));
						return;
					}else if(newBlock->start + newBlock->size == curr->start){

						curr->start = newBlock->start;
						curr->size += newBlock->size;
						
						meta_Free(newBlock,sizeof(block)); 
						return;
					}
				}
				
				newBlock->next = curr;
				newBlock->prev = prev;

				prev->next = newBlock;
				curr->prev = newBlock;

				return;
			}

			curr = curr->next;
			prev = prev->next;
		}

		//At the end of list
		if(
			(*list) == freeList && 
			prev->start + prev->size == newBlock->start
		){
			//Merge
			prev->size += newBlock->size;
			meta_Free(newBlock,sizeof(block)); 
		}else{
			prev->next = newBlock;
			newBlock->prev = prev;	
		}		
	}
}


block* search_freeList(int allocSize){

	block* temp = freeList;

	while(temp){
		if(allocSize <= temp->size){
			return temp;
		}
		temp = temp->next;
	}

	return NULL;
}


void meta_Init(){

    for(int i = 0; i<PAGESIZE; ++i){
        metaPageStatus[i] = FALSE;
    }
}


void* meta_Malloc(int size){
    
    int currRun = 0;
    char* startAddress = metaPage;

    for(int i=0; i<PAGESIZE; ++i){
        
		if(metaPageStatus[i] == FALSE){
			++currRun;
			if(currRun == size){         
				int startIndex = i-currRun+1;

				while(size--){
					metaPageStatus[startIndex++] = TRUE;
				}

				return startAddress;
			}
		}else{		
			currRun = 0;
			startAddress = metaPage+i+1;
		}       
    }

    return NULL;
}


void meta_Free(char* blockAddress, int size){
    
    int i;
    for(i=0; i<PAGESIZE; ++i){
        if(metaPage+i == blockAddress){
            break;
        }
    }

    while(size--){
        (*blockAddress) = 0;
        metaPageStatus[i++] = FALSE; 
    }

    //print_MetaPage();
}


void print_MetaPage(){

	printf("\n___________________________________\n\n");

	for(int i = 0; i<(32*15); ++i){
        printf("%d",metaPageStatus[i]);
		if(i%32 == 31){
			printf("\n");
		}
    }

	printf("___________________________________\n");
}