#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define WORKERS 3 // input, output and file
#define MAX_BUFFER_SIZE 4096
#define MAX_INPUT 100


pthread_mutex_t lock_buff;

pthread_cond_t data_avail;
pthread_cond_t space_avail;


int buff_count;
char input_user[MAX_INPUT];
char buffer[MAX_BUFFER_SIZE];
bool end_of_in=0;

const char *filename;

void *input ();
void *output ();
void *out_file ();


int main(int argc, char *argv[]){

    pthread_mutex_init(&lock_buff, NULL);
    pthread_cond_init(&data_avail, NULL);
    pthread_cond_init(&space_avail, NULL);

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




    return 0;
}

void *input (){
    while(fgets(input_user, MAX_INPUT, stdin) != NULL){
        pthread_mutex_lock(&lock_buff);

        //if full wait with cond var
        if(buff_count==MAX_BUFFER_SIZE){
            while(true){
                pthread_cond_wait(&space_avail,&lock_buff);
                if(buff_count<MAX_BUFFER_SIZE)
                    break;
            }
        }

        int i;
        for (i = 0; i < strlen(input_user); i++) {
            buffer[buff_count + i] = input_user[i];
        }
        buff_count += strlen(input_user);


        //data is the buffer, so signal others
        pthread_cond_signal(&data_avail);
        pthread_mutex_unlock(&lock_buff);
    }
    //handle end of input
    pthread_mutex_lock(&lock_buff);
    end_of_in=1;
    pthread_cond_signal(&data_avail);
    pthread_mutex_unlock(&lock_buff);
    return NULL;

}


void *output (){

    /* 
    Acquire a lock before accessing the shared buffer.
    Wait on a condition variable if the buffer is empty.
    Read data from the buffer.
    Release the lock.
    Write the data to standard output.
    Signal a condition variable to notify the input function that there is space available in the buffer.
    */

    while (true) {
        if (buff_count != 0 && !end_of_in) {
            break;
        }
        pthread_mutex_lock(&lock_buff);

        while(buff_count==0 && !end_of_in){
            pthread_cond_wait(&data_avail,&lock_buff);
        }

        for (int i = 0; i < buff_count; i++) {
            putchar(buffer[i]);  // write standard output
        }

        //reset
        buff_count=0;

        pthread_cond_signal(&space_avail);
        pthread_mutex_unlock(&lock_buff);
    }
    return NULL;

}

/*
* open
* read
* write
* */
void *out_file (){

  FILE *out_file;
  out_file=fopen("text.txt", "w");

      while(1){
          pthread_mutex_lock(&lock_buff);

          while(buff_count==0 && !end_of_in){
              pthread_cond_wait(&data_avail,&lock_buff);
          }

          if (buff_count==0 && end_of_in) {
              pthread_mutex_unlock(&lock_buff);
              break;
          }

          int i;
          for(i=0;i<buff_count; i++){
            fputc(buffer[i],out_file);
          }
          buff_count=0;
          pthread_cond_signal(&space_avail);
          pthread_mutex_unlock(&lock_buff);
      }

    fclose(out_file);

  return NULL;

}



























// // Shared data structure
// typedef struct {
//     char buffer[MAX_BUFFER_SIZE];
//     int count;
//     pthread_mutex_t lock;
//     pthread_cond_t data_available;
//     pthread_cond_t space_available;
//     bool end_of_input;
// } SharedBuffer;



// // Input thread
// void *input(void *arg) {
//     while (true) {
//         char data[MAX_LINE_LENGTH];
//         if (fgets(data, MAX_LINE_LENGTH, stdin) == NULL) {
//             // End of input
//             pthread_mutex_lock(&buffer.lock);
//             buffer.end_of_input = true;
//             pthread_cond_broadcast(&buffer.data_available); // Signal all threads
//             pthread_mutex_unlock(&buffer.lock);
//             break;
//         }
//         pthread_mutex_lock(&buffer.lock);
//         while (buffer.count == MAX_BUFFER_SIZE) {
//             pthread_cond_wait(&buffer.space_available, &buffer.lock);
//         }
//         add_to_buffer(&buffer, data);
//         pthread_cond_signal(&buffer.data_available); // Signal output and file threads
//         pthread_mutex_unlock(&buffer.lock);
//     }
//     return NULL;
// }

// // Output thread
// void *output(void *arg) {
//     while (true) {
//         pthread_mutex_lock(&buffer.lock);
//         while (buffer.count == 0 && !buffer.end_of_input) {
//             pthread_cond_wait(&buffer.data_available, &buffer.lock);
//         }
//         if (buffer.count == 0 && buffer.end_of_input) {
//             pthread_mutex_unlock(&buffer.lock);
//             break;
//         }
//         char data[MAX_LINE_LENGTH];
//         remove_from_buffer(&buffer, data);
//         pthread_cond_signal(&buffer.space_available); // Signal input thread
//         pthread_mutex_unlock(&buffer.lock);
//         printf("%s", data); // Write to stdout
//     }
//     return NULL;
// }




// // File thread
// void *file(void *arg) {
//     FILE *file = fopen(filename, "w");
//     if (file == NULL) {
//         perror("Error opening file");
//         exit(1);
//     }
//     while (true) {
//         pthread_mutex_lock(&buffer.lock);
//         while (buffer.count == 0 && !buffer.end_of_input) {
//             pthread_cond_wait(&buffer.data_available, &buffer.lock);
//         }
//         if (buffer.count == 0 && buffer.end_of_input) {
//             pthread_mutex_unlock(&buffer.lock);
//             break;
//         }
//         char data[MAX_LINE_LENGTH];
//         remove_from_buffer(&buffer, data);
//         pthread_cond_signal(&buffer.space_available); // Signal input thread
//         pthread_mutex_unlock(&buffer.lock);
//         fprintf(file, "%s", data); // Write to file
//     }
//     fclose(file);
//     return NULL;
// }




