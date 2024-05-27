#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITER_MAX 1000
#define ESCAPE_RADIUS 2


int is_in_mandelbrot(double x, double y) {
    double xr = 0.0, xi = 0.0;
    int iter = 0;
    while (xr*xr + xi*xi < ESCAPE_RADIUS*ESCAPE_RADIUS  && iter < ITER_MAX) {
        double temp = xr*xr - xi*xi + x;
        xi = 2*xr*xi + y;
        xr = temp;
        iter++;
    }
    return iter == ITER_MAX;
}

int main(int argc, char** argv) {
    int rank, size;
    double start_time, end_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    start_time = MPI_Wtime();
    
    srand(time(NULL) + rank);

    long long int total_points = 10000000; 
    long long int points_per_process = total_points / size;

    long long int local_count = 0, total_count = 0;
    double local_inertia = 0.0, total_inertia = 0.0;

    for (long long int i = 0; i <= total_points; i += size) {
        double x = 4.0 * rand() / RAND_MAX - 2.0; 
        double y = 4.0 * rand() / RAND_MAX - 2.0;
        if (is_in_mandelbrot(x, y)) {
            local_count++;
            local_inertia += x*x + y*y;
        }
    }

    MPI_Reduce(&local_count, &total_count, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_inertia, &total_inertia, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double area = (double)total_count / total_points * 16.0;
        double inertia = total_inertia / total_count;
        
        printf("\n\nArea of Mandelbrot Set: %f\n", area);
        printf("Moment of Inertia: %f\n", inertia);
        
        end_time = MPI_Wtime();
    	printf("\n\nTotal time taken: %lf seconds\n", end_time - start_time);
    	printf("\n################ ––– End program ––– ################\n\n");
    }

    MPI_Finalize();
    return 0;
}

