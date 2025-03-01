/* matrix summation using OpenMP
usage with gcc (version 4.2 or higher required):
gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
./matrixSum-openmp size numWorkers
*/


#include <omp.h>
double start_time, end_time;
#include <stdio.h>
#include <stdlib.h>


#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 8 /* maximum number of workers */


int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];

int min=10000;
int max=0;

int min_row;
int min_col;

int max_row;
int max_col;



void *Worker(void *);


/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
    int i, j, total=0;
    /* read command line args if any */
    size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;

    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

    omp_set_num_threads(numWorkers);

    /* initialize the matrix */
    for (i = 0; i < size; i++) {
         //printf("[ ");//
        for (j = 0; j < size; j++) {
            matrix[i][j] = rand()%99;
             //printf(" %d", matrix[i][j]);//
        }
         //printf(" ]\n");//
    }

    start_time = omp_get_wtime();
    #pragma omp parallel for reduction (+:total) private(j)
    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++){
            total += matrix[i][j];
            //min
            if(matrix[i][j]<min){
                #pragma omp critical
                min_row=i;
                min_col=j;
                min=matrix[i][j];
            }
            //max
            if(matrix[i][j]>max){
                #pragma omp critical
                max_row = i;
                max_col = j;
                max = matrix[i][j];
            }
        }
    // implicit barrier


    end_time = omp_get_wtime();


    printf("the total is %d\n", total);
    printf("Max is %d, having position %d,%d.\n", max, max_row, max_col);
    printf("Min is %d, having position %d,%d.\n", min, min_row, min_col);
    printf("it took %g seconds\n", end_time - start_time);
}
