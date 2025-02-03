#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#define MAXWORKERS 10

int threadCount = 0;
pthread_mutex_t mutex;
typedef struct {
    int *arr;
    int min;
    int max;
} Args;


void quicksort (int arr[], int len);
void quicksortrec(int arr[], int min, int max);
int split(int arr[], int min, int max);
int swap(int *a, int *b);



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



void quicksort (int arr[], int len){
    quicksortrec(arr, 0, len-1);
}



 void quicksortrec(int arr[], int min, int max) {
    if(min<max) {
        int pivot= split(arr,min,max);
        #pragma omp parallel sections
        {
            #pragma omp section
            quicksortrec(arr,min,pivot-1);

            #pragma omp section
            quicksortrec(arr,pivot+1, max);
         }

    }
}


int split(int arr[], int min, int max){
    int pivot= arr[max];
    int i=min;
    for(int j=min;j<max; j++){
        if(arr[j]<=pivot){
            swap(&arr[i], &arr[j]);
            i++;
        }
    }
    swap(&arr[i], &arr[max]); // bring pivot in the middle
    return i; // this is where the pivot is
}
int swap(int *a, int *b){
    int temp= *a;
    *a= *b;
    *b=temp;
}

int main() {
    //int arr[] = {7,1,8,3,2,4};
    //int length= 6;

    int length = 10000;

    // Dynamically allocate memory for the array
    int* arr = (int*)malloc(length * sizeof(int));

    // Initialize the array with random values
    for (int i = 0; i < length; i++) {
        arr[i] = rand() % 10000;
    }


    omp_set_num_threads(32);

    double start = omp_get_wtime();

    quicksort(arr, length);
    double end = omp_get_wtime();


    //for (int i = 0; i < length; i++)
    //{
     //   printf("%d", arr[i]);
    //}

    printf("\nExecution time: %f seconds\n", end - start);


    return 0;
}
