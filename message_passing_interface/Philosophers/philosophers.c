#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>



#define REQUEST 1
#define RELEASE 2
#define DONE 3
#define APPROVED 4
#define DENIED 5



//get request from philosopher
//if message is RELEASE, release forks
//if message is DONE, increment conter
//if message is REQUEST, check if left and right forks are available
//if yes, set this to 0 (give them to philosopher) and send APPROVED
//if no, send DENIED message
void server(){
  int forks[5] = {1, 1, 1, 1, 1}; //1 is available, 0 is not
  int request_type; //can be REQUEST (1), RELEASE (2), DONE (3)
  int response_type;
  int philo_finished = 0;
  MPI_Status status;
  int left = 0;
  int right = 0;

  while (philo_finished < 5) {
    MPI_Recv(&request_type, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    int philo_rank = status.MPI_SOURCE;
    if (request_type == DONE) {
      philo_finished++;
      continue;
    }
    left = philo_rank - 1; //left fork
    right = philo_rank % 5; //right fork
    if (request_type == REQUEST) {
      if (forks[left] && forks[right]) {
        forks[left] = forks[right] = 0; //lock corresponding forks
        response_type = APPROVED;
      }
      else {response_type = DENIED;}
      MPI_Send(&response_type, 1, MPI_INT, philo_rank, 0, MPI_COMM_WORLD);
    }
    else if (request_type == RELEASE) {
      forks[left] = forks[right] = 1;
    }
  }
}


//think for random time
//request fork at i and i+1 to eat by sending request
//wait for approval using recv
//if request not approved, retry after thinking
//if request approved, eat for rand amount of time then release forks by sending request
//use a counter so each philosopher will eat n (3) times before returning
//when finished eating the philospher sends a message to server that it is done
 void philosopher(int rank){
  MPI_Status status;
  int message, response;
  int count = 3;
  srand(time(NULL));

  while (count > 0) {
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
    //eat
    sleep(rand() % 2);

    printf("Philosopher %d is releasing forks\n", rank);
    message = RELEASE;
    MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    count--;
  }
  printf("Philosopher %d has finished eating........................................................................\n", rank);
  message = DONE;
  MPI_Send(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
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