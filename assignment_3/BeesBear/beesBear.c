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

#define PRODUCERS 2
#define CAPACITY 10

int H; //Honey pot


sem_t mutex; //lock
sem_t full_pot; //signaled when pot is full

void* _Bees(void* arg) {

  int worker_id = * (int*)arg;
  printf("Bee number %d is reporting for duty\n", worker_id);

  while(1) {
    sem_wait(&mutex);
    if (H == 10) {
      printf("Bee number %d has filled the pot and waking up the bear\n", worker_id);
      sem_post(&full_pot);
    }
    else {
      printf("Bee number %d is putting one portion of honey in the pot\n", worker_id);
      H++;
      sem_post(&mutex);

      //Sleep(rand() % 10);
    }
  }
}


void* _Bear() {

  while(1) {
    sem_wait(&full_pot);
    printf("Bear has been woken and will eat the honey\n");
    H = 0;
    sem_post(&mutex);
  }

}

int main(int argc, char *argv[]) {

  H  = 0;
  int numWorkers = 2;



  pthread_t bees[numWorkers];
  pthread_t bear;

  sem_init(&mutex, 1, 1); //Initialize lock
  sem_init(&full_pot, 1, 0);//Initialize condition

  printf("Program started\n");
  srand(time(NULL));
  for (long l = 0; l < numWorkers; l++) {
    pthread_create(&bees[l],NULL, _Bees, (void *) l);
  }
  pthread_create(&bear, NULL, _Bear, NULL);

  pthread_join(bear,NULL);
  for (int l = 0; l < numWorkers; l++) {
    pthread_join(bees[l],NULL);
  }


  printf("Programs terminated\n");

}