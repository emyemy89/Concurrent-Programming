
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> // for mac error
#include <sys/stat.h> // Required for mode constants

#define MAX_BABY_BIRDS 30
#define WORM_NUM 10

void *Parent_Bird(void *);
void *Baby_Birds(void *);

int num_babies;
int worm_count = WORM_NUM;

sem_t *empty;
sem_t *mutex;

int main(int argc, char *argv[]) {
    num_babies = (argc > 1) ? atoi(argv[1]) : MAX_BABY_BIRDS;

    // Create semaphores
    empty = sem_open("/empty_sem", O_CREAT, 0644, 0);
    mutex = sem_open("/mutex_sem", O_CREAT, 0644, 1);

    // if (empty == SEM_FAILED || mutex == SEM_FAILED) {
    //     perror("Failed to open semaphores");
    //     exit(EXIT_FAILURE);
    // }

    pthread_t parent_th;
    pthread_t baby_id[MAX_BABY_BIRDS];

    // Create threads
    pthread_create(&parent_th, NULL, Parent_Bird, NULL);
    for (int i = 0; i < num_babies; i++) {
        int *index = malloc(sizeof(int));
        *index = i;
        pthread_create(&baby_id[i], NULL, Baby_Birds, (void *)index);
    }

    // Join threads
    pthread_join(parent_th, NULL);
    for (int i = 0; i < num_babies; i++) {
        pthread_join(baby_id[i], NULL);
    }

    // Close and unlink semaphores
    sem_close(empty);
    sem_close(mutex);
    sem_unlink("/empty_sem");
    sem_unlink("/mutex_sem");

    return 0;
}

void *Parent_Bird(void *arg) {
    while (1) {
        sem_wait(empty);  // Wait for a baby bird to signal an empty dish
        printf("Refilling dish\n");
        worm_count = WORM_NUM;
    }
    return NULL;
}

void *Baby_Birds(void *arg) {
    int id = *(int *)arg;
    free(arg);

    while (1) {
        sem_wait(mutex);  // Enter critical section
        if (worm_count > 0) {
            worm_count--;
            printf("Baby bird %d is eating a worm. Worms left: %d\n", id, worm_count);

            if (worm_count == 0) {
                printf("Baby bird %d signals that the dish is empty.\n", id);
                sem_post(empty);  // Signal the parent bird when the dish is empty
            }
        }
        sem_post(mutex);  // Leave critical section

        sleep(1);
    }
    return NULL;
}
