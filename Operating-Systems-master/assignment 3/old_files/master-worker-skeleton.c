#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "wait.h"

int item_to_produce;
int total_items, max_buf_size, num_workers;
// declare any global data structures, variables, etc that are required
// e.g buffer to store items, pthread variables, etc

void print_produced(int num)
{

  printf("Produced %d\n", num);
}

void print_consumed(int num, int worker)
{

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

  while (1)
  {
    if (item_to_produce >= total_items)
      break;
    print_produced(item_to_produce);
    item_to_produce++;
  }
  return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item

int main(int argc, char *argv[])
{

  int master_thread_id = 0;
  pthread_t master_thread;
  item_to_produce = 0;

  if (argc < 4)
  {
    printf("./master-worker #total_items #max_buf_size #num_workers e.g. ./exe 10000 1000 4\n");
    exit(1);
  }
  else
  {
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }

  // Initlization code for any data structures, variables, etc

  //create master producer thread
  pthread_create(&master_thread, NULL, generate_requests_loop, (void *)&master_thread_id);

  //create worker consumer threads
  pthread_t worker[num_workers];
  for (size_t i = 0; i < num_workers; i++)
  {
    // pthread_create(&master_thread, NULL, consume_request,)
  }
  

  //wait for all threads to complete
  pthread_join(master_thread, NULL);
  printf("master joined\n");

  //deallocate and free up any memory you allocated

  return 0;
}
