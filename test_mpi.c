#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 9

int determinant(int mat[N][N], int n) {
    int det = 0;
    int submat[N][N];

    if (n == 2)
        return ((mat[0][0] * mat[1][1]) - (mat[1][0] * mat[0][1]));
    else {
        for (int x = 0; x < n; x++) {
            int subi = 0;
            for (int i = 1; i < n; i++) {
                int subj = 0;
                for (int j = 0; j < n; j++) {
                    if (j == x)
                        continue;
                    submat[subi][subj] = mat[i][j];
                    subj++;
                }
                subi++;
            }
            det = det + (pow(-1, x) * mat[0][x] * determinant(submat, n - 1));
        }
    }
    return det;
}

void generateMatrix(int mat[N][N], int rank) {
    // Простая иллюстрация: установка значений матрицы на основе ранга процесса
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int rand_num = rand() & 2;
            // printf("rand = %d\n", rand_num);
            mat[i][j] = (rand_num == 0) ? 1 : -1;
            // printf("%2d ", mat[i][j]);
        }
        // printf("\n");
    }
    // printf("\n");
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Status status;   
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int count = pow(2, 8);
    int count_length = 0;
    int arr[(int)count];
    int mat[N][N];
    int i = 0;
    int det = 0;
    while (i <= count) {
        if(world_rank == 0)
        {
            for (int i = 1; i < world_size; i++) {
                generateMatrix(mat, world_rank);
                MPI_Send(mat, N*N, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        else
        {
            MPI_Recv(mat, N*N, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            det = determinant(mat, N);
        }
        i++;
        printf("process %d det = %d\n\n", world_rank, det);
    }
    
    // if (world_rank == 0) {
    //         // generateMatrix(mat, world_rank);
    //     MPI_Send(&test, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    //     printf("send test = %d\n", test);
    // }
    // if(world_rank == 1) {
    //     MPI_Recv(&test, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //     printf("recv test = %d\n", test);
    // }
    // for (int i = 0; i < count; i++) {
        // if (world_rank == 0) {
        //     generateMatrix(mat, world_rank);
        //     MPI_Send(&test, 1, MPI_INT, 0, world_rank, MPI_COMM_WORLD);
        //     printf("send test = %d\n", test);
        // }
        // if(world_rank == 1) {
        //     MPI_Recv(&test, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //     printf("recv test = %d\n", test);
        // }
        // if (world_rank == 0) {
        //     generateMatrix(mat, world_rank);
        //     // int recv_det;
        //     // MPI_Status status;
        //     // for (int i = 1; i < world_size; i++) {
        //     //     MPI_Recv(&recv_det, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        //     //     if (recv_det == 31 || recv_det == 32) {
        //     //         printf("Найден определитель: %d у процесса с рангом %d\n", recv_det, status.MPI_SOURCE);
        //     //     }
        //     // }
        //     det = determinant(mat, N);
        //     MPI_Send(&det, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        // } else {
        //         MPI_Recv(&det, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
        //                     MPI_STATUS_IGNORE);
        //             printf("Process det received number %d from process 0\n",det);
        // }
        // count_length++;
        // printf("\n=======");
    // }
    // printf("count_length = %d", count_length);

    MPI_Finalize();
    return 0;
}