#ifndef REENTRANT
#define REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define MAX_BUFFER_SIZE 4096
#define MAX_INPUT 100

pthread_mutex_t lock_buff;
pthread_cond_t data_avail; //data is available in the buffer
pthread_cond_t space_avail; //space is available in the buffer
pthread_cond_t buff_done; //buffer has been processed

int buff_count;
char input_user[MAX_INPUT];
char buffer[MAX_BUFFER_SIZE];
bool end_of_in = false;
bool ready_for_out = false;  // Flag to indicate buffer is ready for output
bool processed_by_out = false; // Flag to indicate buffer has been processed by output thread

FILE *file;

void *input();
void *output();
void *out_file();

int main(int argc, char *argv[]) {

    pthread_mutex_init(&lock_buff, NULL);
    pthread_cond_init(&data_avail, NULL);
    pthread_cond_init(&space_avail, NULL);
    pthread_cond_init(&buff_done, NULL);

    file = fopen(argv[1], "w");


    pthread_t in_tid, out_tid, file_tid;
    pthread_create(&in_tid, NULL, input, NULL);
    pthread_create(&out_tid, NULL, output, NULL);
    pthread_create(&file_tid, NULL, out_file, NULL);

    pthread_join(in_tid, NULL);
    pthread_join(out_tid, NULL);
    pthread_join(file_tid, NULL);

    pthread_mutex_destroy(&lock_buff);
    pthread_cond_destroy(&data_avail);
    pthread_cond_destroy(&space_avail);
    pthread_cond_destroy(&buff_done);

    fclose(file);

    return 0;
}

void *input() {
    while (fgets(input_user, MAX_INPUT, stdin) != NULL) {
        pthread_mutex_lock(&lock_buff);


        //copy input to buffer//
        strncpy(buffer + buff_count, input_user, strlen(input_user));
        buff_count += strlen(input_user);

        //signal when data is available//
        ready_for_out = false;
        processed_by_out = false;
        pthread_cond_signal(&data_avail);
        pthread_mutex_unlock(&lock_buff);
    }

    //end of input//
    pthread_mutex_lock(&lock_buff);
    end_of_in = true;
    pthread_cond_signal(&data_avail);
    pthread_mutex_unlock(&lock_buff);

    return NULL;
}

void *output() {
    while (true) {
        pthread_mutex_lock(&lock_buff);

        // Wait for data to be available
        while (buff_count == 0 && !end_of_in) {
            pthread_cond_wait(&data_avail, &lock_buff);
        }

        //check if we should exit
        if (buff_count == 0 && end_of_in) {
            pthread_mutex_unlock(&lock_buff);
            break;
        }

        // Wait until the buffer has been processed by the out_file thread
        while (!ready_for_out) {
            pthread_cond_wait(&buff_done, &lock_buff);
        }

        // Write data to standard output
        for (int i = 0; i < buff_count; i++) {
            putchar(buffer[i]);
        }

        buff_count = 0;

        // Signal that the buffer has been processed by the output thread
        processed_by_out = true;
        pthread_cond_signal(&space_avail);

        pthread_mutex_unlock(&lock_buff);
    }

    return NULL;
}

void *out_file() {
    while (true) {
        pthread_mutex_lock(&lock_buff);

        // Wait for data to be available
        while (buff_count == 0 && !end_of_in) {
            pthread_cond_wait(&data_avail, &lock_buff);
        }

        // Check if we should exit
        if (buff_count == 0 && end_of_in) {
            pthread_mutex_unlock(&lock_buff);
            break;
        }

        // Write data to file
        for (int i = 0; i < buff_count; i++) {
            fputc(buffer[i], file);
        }
        fflush(file);

        //set the flag
        ready_for_out = true;
        pthread_cond_signal(&buff_done);

        // let the output thread process the buffer
        while (processed_by_out==0) {
            pthread_cond_wait(&space_avail, &lock_buff);
        }

        pthread_mutex_unlock(&lock_buff);
    }

    return NULL;
}
