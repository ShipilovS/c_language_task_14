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
            printf("%2d ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    double count = pow(2, 81);
    int count_length = 0;
    int mat[N][N];
    int i = 0;
    while (i <= count) {
        generateMatrix(mat, world_rank);

        int det = determinant(mat, N);
        if (world_rank == 0) {
            MPI_Send(&det, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("det with = %d\n",det);
        } else {
            MPI_Recv(&det, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Process 1 received number %d from process 0\n", det);
        }
        i++;
    }
    printf("total count = %d", i);

    // for (int i = 0; i < count; i++) {
    //     generateMatrix(mat, world_rank);

    //     int det = determinant(mat, N);

    //     if (det == 31 || det == 32) {
    //         if (world_rank == 0) {
    //             printf("Найден определитель: %d у процесса с рангом %d\n", det, world_rank);
    //             // Здесь можно отправить данные о матрице на другие процессы или записать в файл
    //         } else {
    //             MPI_Send(&det, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //         }
    //     }
    //     if (world_rank == 0) {
    //         // int recv_det;
    //         // MPI_Status status;
    //         // for (int i = 1; i < world_size; i++) {
    //         //     MPI_Recv(&recv_det, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    //         //     if (recv_det == 31 || recv_det == 32) {
    //         //         printf("Найден определитель: %d у процесса с рангом %d\n", recv_det, status.MPI_SOURCE);
    //         //     }
    //         // }
    //         MPI_Send(&det, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //     } else {
    //             MPI_Recv(&det, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
    //                         MPI_STATUS_IGNORE);
    //                 printf("Process det received number %d from process 0\n",det);
    //     }
    //     count_length++;
    //     printf("\n=======");
    // }
    printf("count_length = %d", count_length);

    MPI_Finalize();
    return 0;
}