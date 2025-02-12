#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <ctype.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <windows.h>
#include <semaphore.h>

#define PRODUCERS 5
#define CAPACITY 10

int H = 0; //Honey pot
int capac;

sem_t mutex; //lock
sem_t full_pot; //signaled when pot is full

//threads will enter and get the lock, if pot is still under capacity it will add into the pot, release lock and sleep.
//When pot is full, a thread (that has the lock) will signal full_pot condition that is in _Bear and continue to release the lock,
//and the bear will empty the pot.
void* _Bees(void* arg) {
  int worker_id = *((int*)arg);
  free(arg); // Free dynamically allocated memory
  printf("Bee number %d is reporting for duty\n", worker_id);
  while(1) {
    sem_wait(&mutex);
     if (H < capac) {
       printf("Bee number %d is putting one portion of honey in the pot\n", worker_id);
       H++;

       if (H >= capac) {
         printf("Bee number %d has filled the pot and waking up the bear\n", worker_id);
         sem_post(&full_pot);
       }
     }
    sem_post(&mutex);
    Sleep( 10);
  }
}

void* _Bear() {
  while(1) {
    sem_wait(&full_pot);
    printf("Bear has been woken and will eat the honey\n");
    H = 0;
    Sleep( 10);
  }
}

int main(int argc, char *argv[]) {

//Inputs from command line:
  //First integer is number of bees. If nothing is specified then it is 5.
  //Second integer is capacity of honey pot. If nothing is specified then it is 10.

  sem_init(&mutex, 0, 1); //Initialize lock
  sem_init(&full_pot, 0, 0);//Initialize condition

  int numWorkers = (argc > 1) ? atoi(argv[1]) : PRODUCERS; //Gets number of threads from command line. If not, use PRODUCERS
  capac = (argc > 1) ? atoi(argv[2]) : CAPACITY; //Gets number of threads from command line. If not, use PRODUCERS
  if (numWorkers > PRODUCERS) numWorkers = PRODUCERS;
  if (capac > CAPACITY) capac = CAPACITY;

  pthread_t bees[numWorkers];
  pthread_t bear;

  printf("Capacity of honey pot is %d\n", capac);

  printf("Program started\n");
  srand(time(NULL));
  for (int l = 0; l < numWorkers; l++) {
    int* id = malloc(sizeof(int)); // Allocate memory for thread argument
    *id = l; // Unique worker ID
    pthread_create(&bees[l], NULL, _Bees, id);
  }
  pthread_create(&bear, NULL, _Bear, NULL);

  pthread_join(bear,NULL);
  for (int l = 0; l < numWorkers; l++) {
    pthread_join(bees[l],NULL);
  }
  sem_destroy(&mutex);
  sem_destroy(&full_pot);
  return 0;

}