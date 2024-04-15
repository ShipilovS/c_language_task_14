#include <stdio.h>
#include <stdlib.h>

#define N 9


// Функция для обмена двух строк матрицы
void swapRows(int mat[N][N], int row1, int row2) {
    for (int i = 0; i < N; i++) {
        int temp = mat[row1][i];
        mat[row1][i] = mat[row2][i];
        mat[row2][i] = temp;
    }
}

// Функция для вычисления определителя матрицы 9x9 с использованием метода Гаусса
long long determinant(int mat[N][N]) {
    long long det = 1;
    int sign = 1; // Знак определителя изменяется при каждом переставлении строк
    
    for (int i = 0; i < N; i++) {
        // Поиск максимального элемента в текущем столбце
        int maxRow = i;
        for (int k = i + 1; k < N; k++) {
            if (abs(mat[k][i]) > abs(mat[maxRow][i])) {
                maxRow = k;
            }
        }
        
        // Обмен строк, если необходимо
        if (i != maxRow) {
            swapRows(mat, i, maxRow);
            sign *= -1; // Изменение знака определителя
        }
        
        // Проверка на нулевой ведущий элемент
        if (mat[i][i] == 0) return 0;
        
        // Приведение матрицы к верхнетреугольному виду
        for (int k = i + 1; k < N; k++) {
            double factor = mat[k][i] / (double)mat[i][i];
            for (int j = i; j < N; j++) {
                if (i == j) {
                    mat[k][j] = 0;
                } else {
                    mat[k][j] -= factor * mat[i][j];
                }
            }
        }
        
        det *= mat[i][i];
    }
    
    return det * sign;
}

void printMatrix(int mat[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%2d ", mat[i][j]);
        }
        printf("\n");
    }
}

void generateMatrices(int mat[N][N], int pos) {
    if (pos == N * N) {
        int tempMat[N][N];
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                tempMat[i][j] = mat[i][j];
            }
        }
        
        long long det = determinant(tempMat);
        if (det == 31 || det == 32) {
            printMatrix(mat);
            printf("Определитель: %lld\n\n", det);
        } else { 
            printMatrix(mat);
            printf("Определитель: %lld\n\n", det);
        }
        return;
    }
    int row = pos / N;
    int col = pos % N;
    mat[row][col] = 1;
    generateMatrices(mat, pos + 1);
    mat[row][col] = -1;
    generateMatrices(mat, pos + 1);
}

int main() {
    int matrix[N][N] = {0};
    generateMatrices(matrix, 0);
    return 0;
}