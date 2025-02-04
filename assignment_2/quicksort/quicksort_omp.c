#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define MAXWORKERS 30
#define MAXSIZE 1000000
#define THRESHOLD 1000

int threadCount = 0;

int size;
int nb_workers;


typedef struct {
    int *arr;
    int min;
    int max;
} Args;


//void quicksort (int arr[], int len);
void quicksort(int arr[], int min, int max);
int split(int arr[], int min, int max);
int swap(int *a, int *b);






 void quicksort(int arr[], int min, int max) {
    if(min<max) {
        int pivot= split(arr,min,max);

        if ((max - min) < THRESHOLD) {
            quicksort(arr, min, pivot - 1);
            quicksort(arr, pivot + 1, max);
        }else {
            #pragma omp task shared(arr)
            quicksort(arr, min, pivot - 1);

            #pragma omp task shared(arr)
            quicksort(arr, pivot + 1, max);

            #pragma omp taskwait
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



int main(int argc, char *argv[]) {
    // int arr[] = {7,1,8,3,2,4};
    // int size= 6;


    /* read command line args if any */
    size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
    nb_workers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
    if (size > MAXSIZE)
        size = MAXSIZE;
    if (nb_workers > MAXWORKERS)
        nb_workers = MAXWORKERS;

    omp_set_num_threads(nb_workers);


    // // memory for the array
    int* arr = (int*)malloc(size * sizeof(int));

    // // random values
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 10000;
    }


    double start = omp_get_wtime();

   // quicksort(arr, 0, size-1);

    #pragma omp parallel
     {
        #pragma omp single
        quicksort(arr, 0, size - 1);
     }

    double end = omp_get_wtime();


    // for (int i = 0; i < size; i++)
    // {
    //     printf("%d", arr[i]);
    // }

    printf("\nExecution time: %f seconds\n", end - start);

     free(arr);
    return 0;
}
