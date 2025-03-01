/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 1000  /* maximum matrix size */
#define MAXWORKERS 20  /* maximum number of workers */

//define locks
pthread_mutex_t lockTotal;
pthread_mutex_t lockMax;
pthread_mutex_t lockMin;
pthread_mutex_t lockBag;

typedef struct {
  long id;
  int Local_Min;
  int Local_Max;
  int Local_Sum;
  int iMax, jMax, iMin, jMin;
}local_data;


int total = 0;

int Min;
int Max;

int iMax = 0;
int iMin = 0;

int jMin = 0;
int jMax = 0;

int rowBag = 0;

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
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

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  //Initialize locks
  pthread_mutex_init(&lockTotal, NULL);
  pthread_mutex_init(&lockMax, NULL);
  pthread_mutex_init(&lockMin, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;


  srand(time(NULL));
  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
    }
  }
  Min = INT_MAX;
  Max= 0;
  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

local_data localdata[numWorkers];

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++) {
    localdata[l].id = l;
    localdata[l].Local_Min = INT_MAX;
    localdata[l].Local_Max = 0;
    localdata[l].Local_Sum = 0;
    pthread_create(&workerid[l], &attr, Worker, (void *) &localdata[l]);
  }

  for (l = 0; l < numWorkers; l++) {
    pthread_join(workerid[l],NULL);
  }


//Destroy locks
  pthread_mutex_destroy(&lockTotal);
  pthread_mutex_destroy(&lockMax);
  pthread_mutex_destroy(&lockMin);


  /* get end time */
  end_time = read_timer();
  /* print results */
  printf("The total is %d\n", total);
  printf("The execution time is %g sec\n", end_time - start_time);

  printf("The global max %d is at row %d and column %d\n ", Max, iMax, jMax);

  printf("The global min %d is at row %d and column %d\n ", Min, iMin, jMin);

  printf("Matrix size: %d. Thread count: %d\n", size, numWorkers);
}




/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  local_data *Local = (local_data *) arg;
  long myid = Local->id;
  int  i, j, first, last;

  int row = 0;

while (true) {

  pthread_mutex_lock(&lockBag);
  row = rowBag;
  rowBag++;
  pthread_mutex_unlock(&lockBag);

  if (row >= size) {break;}

    for (j = 0; j < size; j++) {

      Local->Local_Sum += matrix[row][j];

      if (matrix[row][j] > Local->Local_Max) {
        Local->Local_Max = matrix[row][j];
        Local->iMax = row;
        Local->jMax = j;
      }
      else if (matrix[row][j] < Local->Local_Min) {
        Local->Local_Min = matrix[row][j];
        Local->iMin = row;
        Local->jMin = j;
      }
    }



  //global variables calculation

  pthread_mutex_lock(&lockTotal);
  total += Local->Local_Sum;

  if (Max < Local->Local_Max) {
    Max = Local->Local_Max;
    iMax = Local->iMax;
    jMax = Local->jMax;
  }

  else if (Min > Local->Local_Min) {
    Min = Local->Local_Min;
    iMin = Local->iMin;
    jMin = Local->jMin;
  }
  pthread_mutex_unlock(&lockTotal);
  pthread_exit(NULL);

}



}