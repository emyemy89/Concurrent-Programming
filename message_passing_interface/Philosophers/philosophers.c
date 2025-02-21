#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>



#define REQUEST 1
#define RELEASE 2
#define APPROVED 3
#define DENIED 4


void server(){
  int forks[5] = {1, 1, 1, 1, 1}; //1 is available, 0 is not
  int request_type; //can be REQUEST (1) or RELEASE (2)
  int response_type;
  MPI_Status status;
  int first = 0;
  int second = 0;

  while (1) {
    MPI_Recv(&request_type, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    int philo_rank = status.MPI_SOURCE;
    //if message tag is REQUEST,

    if ((philo_rank % 2)==0){ //if rank is even
      first = philo_rank - 1; //left fork
      second = philo_rank % 5; //right fork
    }
    else if ((philo_rank % 2)==1){ //if rank is odd
      first = philo_rank % 5; //right fork
      second = philo_rank - 1; //left fork
    }

    if (request_type == REQUEST) {
      if (forks[first] && forks[second]) {
        forks[first] = forks[second] = 0; //lock corresponding forks
        response_type = APPROVED;
      }
      else {response_type = DENIED;}

      MPI_Send(&response_type, 1, MPI_INT, philo_rank, 0, MPI_COMM_WORLD);

    }
    else if (request_type == RELEASE) {
      forks[first] = forks[second] = 1;
    }
  }

  }



 void philosopher(int rank){

   //think for random time
   //request fork at i and i+1 to eat by sending request
   //wait for approval using recv
   //if request not approved, retry using infinite while loop
   //if request approved, eat for rand amount of time then release forks by sending request
   //use a counter so each philosopher will eat n times before returning



  //TASK:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //make philosophers eat 5 times then exit program

  MPI_Status status;
  int message, response;
  srand(time(NULL));

  while (1) {
    //Think
    sleep(rand() % 4);


    printf("Philosopher %d is requesting forks\n", rank);
    message = REQUEST;
    MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Recv(&response, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    if (response == DENIED) {
      printf("Philosopher %d could not acquire forks, retrying later\n", rank);
      continue;
    }

    printf("Philosopher %d has acquired forks and started eating\n", rank);
    sleep(rand() % 4);

    printf("Philosopher %d is releasing forks\n", rank);
    message = RELEASE;
    MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

  }

}



int main(int argc, char** argv) {
  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  int n = size - 1; // Exclude teacher

  if (rank == 0) {
    server();
  } else {
    philosopher(rank);
  }

  MPI_Finalize();
  return 0;
}