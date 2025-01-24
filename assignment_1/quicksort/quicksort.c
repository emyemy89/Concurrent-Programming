#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>



void quicksort (int arr[], int len);
void quicksortrec(int arr[], int min, int max);

//          7 1 8 3 6 2 |4|
//                       |
//             1 3 |2|   4     7 8 |6|
//                  |               |
//                1   3               7 8
// 



void quicksort (int arr[], int len){

quicksortrec(arr, 0, len-1);

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
