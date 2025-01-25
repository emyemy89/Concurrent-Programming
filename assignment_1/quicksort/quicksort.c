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

//          7 1 8 3 6 2 |4|
//                       |
//             1 3 |2|   4     7 8 |6|
//                  |               |
//                1   3               7 8
// 



void quicksort (int arr[], int len){

quicksortrec(arr, 0, len-1);

}


// void quicksortrec(int arr[], int min, int max){
//     if(min<max){
//         int pivot= split(arr,min,max);
//         quicksortrec(arr,min,pivot-1);
//         quicksortrec(arr,pivot+1, max);
//     }

// }


void *Worker(void *arg) {

Args *par = (Args *)args; //pointer to the struct
int *arr= (*par).arr;
int min= (*par).min;
int max= (*par).max;

if(min<max){
       int pivot= split(arr,min,max);

       //left thread (smaller) and right (bigger)
       pthread_t leftT, rightT;


       Args leftArgs={arr,min,pivot-1};
       Args rightArgs={arr,pivot+1,max};



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
    int arr[] = {7,1,8,3,2,4};
    int length= 6;

    quicksort(arr, length);


    for (int i = 0; i < length; i++)
    {
        printf("%d", arr[i]);
    }
    

    return 0;
}
