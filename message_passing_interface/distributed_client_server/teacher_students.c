#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void teacher(int n) {
    int student_rank;
    int arr[n];
    MPI_Status status;

    // Request each student rank one by one
    for (int i = 0; i < n; i++) {
        MPI_Recv(&student_rank, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD, &status);
        arr[i] = student_rank;
    }

    // Pair students
    for (int i = 0; i < n - 1; i += 2) {
        MPI_Send(&arr[i + 1], 1, MPI_INT, arr[i], 0, MPI_COMM_WORLD);
        MPI_Send(&arr[i], 1, MPI_INT, arr[i + 1], 0, MPI_COMM_WORLD);
    }

    // for odd number of students
    if (n % 2 == 1) {
        MPI_Send(&arr[n - 1], 1, MPI_INT, arr[n - 1], 0, MPI_COMM_WORLD);
    }
}

void students(int rank) {
    int partner;
    MPI_Status status;

    MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    // Receive partner info from the teacher
    MPI_Recv(&partner, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);


    printf("Student number %d is paired with student %d\n", rank, partner);
    fflush(stdout);
}

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int n = size - 1; //exclude teacher

    if (rank == 0) {
        teacher(n);
    } else {
        students(rank);
    }

    MPI_Finalize();
    return 0;
}



