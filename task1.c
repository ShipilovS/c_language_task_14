#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int** generage_matrix(int m) {
    int minimum_number = -1;
    int max_number = 1;

    srand(time(NULL));
    
    int **arr = (int**)malloc(sizeof(int*) * m);
    for (int i = 0; i < m; ++i) {
        arr[i] = (int*)malloc(sizeof(int) * m);
    }

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < m; j++) {
            float rand_value = rand() % (max_number + 1 - minimum_number) + minimum_number;
            arr[i][j] = rand_value;
            printf(" %d ", arr[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    return arr;
}

//Возвращает матрицу matrix без row-ой строки и col-того столбца, результат в newMatrix
void getMatrixWithoutRowAndCol(int **matrix, int size, int row, int col, int **newMatrix) {
    int offsetRow = 0; //Смещение индекса строки в матрице
    int offsetCol = 0; //Смещение индекса столбца в матрице
    for(int i = 0; i < size-1; i++) {
        //Пропустить row-ую строку
        if(i == row) {
            offsetRow = 1; //Как только встретили строку, которую надо пропустить, делаем смещение для исходной матрицы
        }

        offsetCol = 0; //Обнулить смещение столбца
        for(int j = 0; j < size-1; j++) {
            //Пропустить col-ый столбец
            if(j == col) {
                offsetCol = 1; //Встретили нужный столбец, проускаем его смещением
            }

            newMatrix[i][j] = matrix[i + offsetRow][j + offsetCol];
        }
    }
}

//Вычисление определителя матрицы разложение по первой строке
int matrixDet(int **matrix, int size) {
    int det = 0;
    int degree = 1; // (-1)^(1+j) из формулы определителя

    //Условие выхода из рекурсии
    if(size == 1) {
        return matrix[0][0];
    }
    //Условие выхода из рекурсии
    else if(size == 2) {
        return matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0];
    }
    else {
        int** newMatrix = (int**)malloc(sizeof(int*) * (size - 1));
        for (int i = 0; i < size-1; ++i) {
            newMatrix[i] = (int*)malloc(sizeof(int) * size - 1);
        }

        //Раскладываем по 0-ой строке, цикл бежит по столбцам
        for(int j = 0; j < size; j++) {
            //Удалить из матрицы i-ю строку и j-ый столбец
            //Результат в newMatrix
            getMatrixWithoutRowAndCol(matrix, size, 0, j, newMatrix);

            //Рекурсивный вызов
            //По формуле: сумма по j, (-1)^(1+j) * matrix[0][j] * minor_j (это и есть сумма из формулы)
            //где minor_j - дополнительный минор элемента matrix[0][j]
            // (напомню, что минор это определитель матрицы без 0-ой строки и j-го столбца)
            det = det + (degree * matrix[0][j] * matrixDet(newMatrix, size-1));
            //"Накручиваем" степень множителя
            degree = -degree;
        }

        //Чистим память на каждом шаге рекурсии(важно!)
        for(int i = 0; i < size-1; i++) {
            free(newMatrix[i]);
        }
        free(newMatrix);
    }

    return det;
}




int main(int argc, char** argv){
    int m = 9;
    int** arr = generage_matrix(m);

    int det = matrixDet(arr, m);
    printf("det = %d\n", det);

    for (int i = 0; i < m; ++i) {
        free(arr[i]);
    }
    free(arr);
  

    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}