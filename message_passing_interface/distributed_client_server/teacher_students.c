#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void teacher(int n) {
    int student_rank;
    int arr[n];
    MPI_Status status;

    // Request each student's rank one by one
    for (int i = 0; i < n; i++) {
        //int request_signal = 1;
       // MPI_Ssend(&request_signal, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&student_rank, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD, &status);
        arr[i] = student_rank;
    }

    // Pair students
    for (int i = 0; i < n - 1; i += 2) {
        MPI_Ssend(&arr[i + 1], 1, MPI_INT, arr[i], 0, MPI_COMM_WORLD);
        MPI_Ssend(&arr[i], 1, MPI_INT, arr[i + 1], 0, MPI_COMM_WORLD);
    }

    // Handle the odd student if n is odd
    if (n % 2 == 1) {
        MPI_Ssend(&arr[n - 1], 1, MPI_INT, arr[n - 1], 0, MPI_COMM_WORLD);
    }
}

void students(int rank) {
    int partner;
    MPI_Status status;
    //int request_signal;

    // Wait for the teacher to request the rank
    //MPI_Recv(&request_signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Ssend(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    // Receive partner info from the teacher
    MPI_Recv(&partner, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    // Print the result (flush ensures it prints immediately)
    printf("Student %d is paired with Student %d\n", rank, partner);
    fflush(stdout);
}

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int n = size - 1; // Exclude teacher

    if (rank == 0) {
        teacher(n);
    } else {
        students(rank);
    }

    MPI_Finalize();
    return 0;
}



