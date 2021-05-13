#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce;
int total_items, max_buf_size, num_workers;
// declare any global data structures, variables, etc that are required
// e.g buffer to store items, pthread variables, etc

int *buffer;
int count;
pthread_mutex_t lock_1, lock_2, lock_3;
pthread_cond_t cv_full, cv_empty;

void print_produced(int num) {

  printf("Produced %d\n", num);
}

void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}


/* produce items and place in buffer (array or linked list)
 * add more code here to add items to the buffer (these items will be consumed
 * by worker threads)
 * use locks and condvars suitably
 */
void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data);

  while(1)
    {
        pthread_mutex_lock(&lock_1);

        if(item_to_produce >= total_items){
            pthread_cond_broadcast(&cv_empty);
            pthread_mutex_unlock(&lock_1);
        	break;
        }
      
        while(count == max_buf_size){
        	pthread_cond_wait(&cv_full, &lock_1);
        }

        buffer[count] = item_to_produce;
        count++;

        print_produced(item_to_produce);
        item_to_produce++;

        if(count==1){
        	pthread_cond_signal(&cv_empty);
        }        

        pthread_mutex_unlock(&lock_1);
    }
  return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item

void *satisfy_requests_loop(void *data)
{
    while(1)
    {
    	pthread_mutex_lock(&lock_2);

    	if(item_to_produce >= total_items && count == 0){
    		pthread_mutex_unlock(&lock_2);
    		break;
    	}

        if(count==0){
          pthread_mutex_lock(&lock_3);
    	    pthread_cond_wait(&cv_empty, &lock_3);
          pthread_mutex_unlock(&lock_3);
    	    if(item_to_produce >= total_items && count == 0){
    	    	pthread_mutex_unlock(&lock_2);
    	    	break;
    	    }
        }

        pthread_mutex_lock(&lock_1);

        print_consumed(buffer[count-1], pthread_self());
        count--;

        pthread_mutex_unlock(&lock_1);
        
        if(count == max_buf_size-1){
    	    pthread_cond_signal(&cv_full);
        }

        pthread_mutex_unlock(&lock_2);
    }

    return 0;
}

int main(int argc, char *argv[])
{
 
  int master_thread_id = 0;
  pthread_t master_thread;
  item_to_produce = 0;
  
  if (argc < 4) {
    printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
    exit(1);
  }
  else {
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }
  
  // Initlization code for any data structures, variables, etc

  buffer = (int*) calloc(max_buf_size, sizeof(int));
  count = 0;
  pthread_mutex_init(&lock_1, NULL);
  pthread_mutex_init(&lock_2, NULL);
  pthread_mutex_init(&lock_3, NULL);
  pthread_cond_init(&cv_empty, NULL);
  pthread_cond_init(&cv_full, NULL);
  pthread_t worker[num_workers];


  //create master producer thread
  pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);

  //create worker consumer threads
  for(int i=0; i<num_workers; i++){
  	pthread_create(&(worker[i]), NULL, satisfy_requests_loop, NULL);
  }

  //wait for all threads to complete
  pthread_join(master_thread, NULL);
  printf("master joined\n");
  for(int i=0; i<num_workers; i++){
  	pthread_join(worker[i], NULL);
  } 

  //deallocate and free up any memory you allocated
  free(buffer);
  pthread_mutex_destroy(&lock_1);
  pthread_mutex_destroy(&lock_2);
  pthread_mutex_destroy(&lock_3);
  pthread_cond_destroy(&cv_empty);
  pthread_cond_destroy(&cv_full);
  
  return 0;
}
