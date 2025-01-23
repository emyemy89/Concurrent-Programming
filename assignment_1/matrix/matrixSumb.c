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
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

//define locks
pthread_mutex_t lockTotal;
pthread_mutex_t lockMax;
pthread_mutex_t lockMin;

int globalMin;
int globalMax;
int posGlobalMax[2];
int posGlobalMin[2];

int total = 0;

//one array for local maximums
int localMax[MAXWORKERS];

//one array for local minnimums
int localMin[MAXWORKERS] ;

// one array for pos of local max
int rowPosMax[MAXWORKERS] ; //pos of i
int colPosMax[MAXWORKERS] ; //pos of j

//one array for pos of min
int rowPosMin[MAXWORKERS]; //pos of i
int colPosMin[MAXWORKERS] ; //pos of j

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

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%40;
	  }
  }

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

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++) {
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  }

  for (l = 0; l < numWorkers; l++) {
    pthread_join(workerid[l],NULL);
  }

  //Calculating global max and min
  globalMax = localMax[0];
  globalMin = localMin[0];
  posGlobalMax[0] = rowPosMax[0];
  posGlobalMax[1] = colPosMax[0];
  posGlobalMin[0] = rowPosMin[0];
  posGlobalMin[1] = colPosMin[0];
  for (i = 1; i < numWorkers; i++) {
    if (localMax[i] > globalMax) {

      globalMax = localMax[i];
      posGlobalMax[0] = rowPosMax[i];
      posGlobalMax[1] = colPosMax[i];
    }

    if (localMin[i] < globalMin) {

      globalMin = localMin[i];
      posGlobalMin[0] = rowPosMin[i];
      posGlobalMin[1] = colPosMin[i];
    }

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

  printf("The global max %d is at row %d and column %d\n ", globalMax, posGlobalMax[0], posGlobalMax[1]);

  printf("The global min %d is at row %d and column %d ", globalMin, posGlobalMin[0], posGlobalMin[1]);
}




/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int  i, j, first, last;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */



  //filling the array with local max and min
  localMax[myid] = 0;
  localMin[myid] = 99;

  for (i = first; i <= last; i++) {
    for (j = 0; j < size; j++) {
      pthread_mutex_lock(&lockTotal);
      total += matrix[i][j];
      pthread_mutex_unlock(&lockTotal);
      if (matrix[i][j] > localMax[myid]) {
        pthread_mutex_lock(&lockMax);
        localMax[myid] = matrix[i][j];
        rowPosMax[myid] = i;
        colPosMax[myid] = j;
      }
      pthread_mutex_unlock(&lockMax);
      if (matrix[i][j] < localMin[myid]) {
        pthread_mutex_lock(&lockMin);
        localMin[myid] = matrix[i][j];
        rowPosMin[myid] = i;
        colPosMin[myid] = j;
        pthread_mutex_unlock(&lockMin);
      }
    }
  }

pthread_exit(NULL);



}
