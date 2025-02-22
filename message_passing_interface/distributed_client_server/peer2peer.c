#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


void teacher(int size) {
    int students[size];
    // students array
    students[0] = 1; // Teacher is 1, not free
    for (int i = 1; i < size; i++) {
        students[i] = 0;// students are 0, free
    }

    // Randomly choose the first student to start
    int first_student = rand() % size;
    while (students[first_student] != 0) {
        first_student = (first_student + 1) % size;
    }
    MPI_Send(students, size, MPI_INT, first_student, 0, MPI_COMM_WORLD);
}

void student(int size, int rank) {
    int students[size];
    MPI_Status status;

    MPI_Recv(students, size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (students[rank] == 0) {// Does not have a partner
        students[rank] = rank;

        // see if there is any student not taken
        int next_exists = 0;
        for (int i = 1; i < size; i++) {
            if (students[i] == 0) {
                next_exists = 1;
                break;
            }
        }

        if (next_exists) {
            // find next partner
            int partner = rand() % size;
            while (students[partner] != 0) { // loop until we find sb free
                partner = (partner + 1) % size;
            }

            students[partner] = rank;
            students[rank] = partner;
            printf("Student %d is paired with Student %d\n", rank, partner);

            // Let partner know it s taken
            MPI_Send(students, size, MPI_INT, partner, 0, MPI_COMM_WORLD);

            // let the next get a partner, if any left
            for (int i = 1; i < size; i++) {
                if (students[i] == 0) { // when we find free student, send him the array
                    MPI_Send(students, size, MPI_INT, i, 0, MPI_COMM_WORLD);
                    break;
                }
            }
        } else {
            // no partner left
            printf("Student %d is working alone\n", rank);
        }
    } else {
        // Already has a partner
        printf("Student %d is paired with Student %d\n", rank, students[rank]);
    }
}

int main(int argc, char* argv[]) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand(time(NULL) + rank);

    if (rank == 0) {
        teacher(size);
    } else {
        student(size, rank);
    }

    MPI_Finalize();
    return 0;
}
