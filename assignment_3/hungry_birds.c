#ifndef _REENTRANT
#define _REENTRANT
#endif
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>

#define MAX_BABY_BIRDS 30

void *Parent_Bird(void *); // producer
void *Baby_Birds(void *); // consumers

int num_babies;
int W; // nb worms

sem_t empty;
sem_t full;




main(int argc, char *argv[]){
    num_babies = (argc > 1) ? atoi(argv[1]) : MAX_BABY_BIRDS;

    sem_init(&empty, 0, 0);  // Initially empty when W runs out
    sem_init(&full, 0, W);   // Assume W is the initial number of worms


    pthread_t parent_th;
    pthread_t baby_id[MAX_BABY_BIRDS]; // array of babies

    //create
    pthread_create(&parent_th, NULL, Parent_Bird, NULL);
    for (int i = 0; i < num_babies; i++) {
        pthread_create(&baby_id[i], NULL, Baby_Birds,  (void *) i);
    }


    //join

}

void *Parent_Bird(void *) {

}

void *Baby_Birds(void *) {

}