#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define MAXWORKERS 10 

int threadCount=0;  
int numArrived=0;

pthread_mutex_t mutex;
pthread_cond_t go;
pthread_mutex_t barrier;



typedef struct {
    int *arr;
    int min;
    int max;
} Args;




void quicksort (int arr[], int len);
//void quicksortrec(int arr[], int min, int max);
int split(int arr[], int min, int max);
int swap(int *a, int *b);
void *Worker(void *);

//          7 1 8 3 6 2 |4|
//                       |
//             1 3 |2|   4     7 8 |6|
//                  |               |
//                1   3               7 8
// 

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == threadCount) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}



void quicksort (int arr[], int len){
    Args args = {arr, 0, len - 1};
    Worker(&args);
}



void *Worker(void *arg) {

    Args *par = (Args *)arg; //pointer to the struct {arr, min, max}
    int *arr= (*par).arr;
    int min= (*par).min;
    int max= (*par).max;

    if(min<max){
        int pivot= split(arr,min,max);

        //lock to modify count
        pthread_mutex_lock(&mutex);

        if (threadCount < MAXWORKERS) {

            //left thread (smaller nb) and right (bigger nb)
            pthread_t leftT, rightT;


            // Args leftArgs={arr,min,pivot-1};
            // Args rightArgs={arr,pivot+1,max};

            Args *leftArgs = (Args *)malloc(sizeof(Args));
            Args *rightArgs = (Args *)malloc(sizeof(Args));
            if (!leftArgs || !rightArgs) {
                perror("Can't allocate");
                pthread_mutex_unlock(&mutex);
                return NULL;
            }

            *leftArgs = (Args){arr, min, pivot - 1};
            *rightArgs = (Args){arr, pivot + 1, max};


            pthread_create(&leftT, NULL, Worker, &leftArgs);
            pthread_create(&rightT, NULL, Worker, &rightArgs);

            threadCount+=2;
            pthread_mutex_unlock(&mutex);

            // wait for both thread to finish
            pthread_join(leftT, NULL);
            pthread_join(rightT, NULL);


            //decrement count at the end
            pthread_mutex_lock(&mutex);
            threadCount-=2;
            pthread_mutex_unlock(&mutex);

            free(leftArgs);
            free(rightArgs);
        }else{
            // threadCount=MAXWORKERS;
            pthread_mutex_unlock(&mutex);
            //fallback, use same thread:
            Args leftArgs = {arr, min, pivot - 1};
            Args rightArgs = {arr, pivot + 1, max};
            Worker(&leftArgs);
            Worker(&rightArgs);
        }
    }
    //Barrier();
    return NULL;
}






int split(int arr[], int min, int max){
    int pivot= arr[max];
    int i=min;
    int j;
    for(j=min;j<max; j++){
        if(arr[j]<=pivot){
            swap(&arr[i], &arr[j]);
            i++;
        }
    }
    swap(&arr[i], &arr[max]); // bring pivot in the middle
    return i; // pos of pivot 
}

int swap(int *a, int *b){
    int temp= *a;
    *a= *b;
    *b=temp;
}


/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}



int main(int argc, char *argv[]) {

    int length = 10000; 
    threadCount = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
    
    int *arr = (int *)malloc(length * sizeof(int));
    if (arr == NULL) {
        printf("Failed!!!\n");
        return 1;
    }

    // random integers
    srand(time(NULL));
    int i;
    for (i = 0; i < length; i++) {
        arr[i] = rand() % 10000;
    }

    double startTime = read_timer();
    quicksort(arr, length);
    double endTime = read_timer();

    double elapsedTime = endTime - startTime;

    //debug
//     for (int i = 0; i < 10; i++) {
//         printf("%d%s", arr[i], (i < length - 1) ? " " : "");
//     }

    printf("Time elapsed %.6f seconds\n", elapsedTime);

    free(arr); 

    return 0;

}
