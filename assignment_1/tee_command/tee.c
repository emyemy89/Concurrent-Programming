#ifndef REENTRANT
#define REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
bool ready_for_out = false;  // buffer is ready for output
bool processed_by_out = false; //buffer has been processed by output thread

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

    fclose(file);

    return 0;
}


/*The input thread reads data from stdin and writes it to the buffer:
 * - data is collected until end of input;
 * - it locks the buffer to ensure mutual exclusion;
 * - copies input to buffer using strncpy;
 * - it signals new data available to  other threads and unlocks the mutex;
 * - to handle end of input, the flag is set to true and used in the other threads
 */

void *input() {
    while (fgets(input_user, MAX_INPUT, stdin) != NULL) {
        pthread_mutex_lock(&lock_buff);


        //copy input to buffer//
        strncpy(buffer + buff_count, input_user, strlen(input_user));
        buff_count += strlen(input_user);

        //signal when data is available//
        pthread_cond_signal(&data_avail);
        pthread_mutex_unlock(&lock_buff);
    }

    //end of input//
    end_of_in = true;

    return NULL;
}


/*The output thread reads data from the buffer and writes it to stdout:
 * - the process terminates if buffer is empty and there is no more input;
 * - thread waits until out_file has processed buffer;
 * - it writes data from the buffer to stdout using putchar and then resets the buffer to signal termination;
 * - in the end, it unlocks the buffer.
 */
void *output() {
    while (true) {
        pthread_mutex_lock(&lock_buff);

        //check if we should exit//
        if (buff_count == 0 && end_of_in) {
            pthread_mutex_unlock(&lock_buff);
            break;
        }

        // Wait until the buffer has been processed by the out_file thread//
        while (!ready_for_out) {
            pthread_cond_wait(&buff_done, &lock_buff);
        }

        // Write data to std output//
        for (int i = 0; i < buff_count; i++) {
            putchar(buffer[i]);
        }

        buff_count = 0;

        // Signal that output thread has processed the buffer
        processed_by_out = true;
        pthread_cond_signal(&space_avail);

        pthread_mutex_unlock(&lock_buff);
    }

    return NULL;
}


    /*The out_file thread ensures writing the inputed text to the file:
     * - it locks the buffer and wait for data to be available from the input;
     * - if buffer is empty and there is no input, it terminated execution;
     * - it copies data from the buffer using putchar() and immediately flushes to avoid delays;
     * - after that it signals the output thread to start handling data;
     */
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

        // write to file
        for (int i = 0; i < buff_count; i++) {
            fputc(buffer[i], file);
        }
        fflush(file);

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

