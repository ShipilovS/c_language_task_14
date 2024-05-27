#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int num_points = 10000000; 
    FILE *file = fopen("points.txt", "w");
    if (file == NULL) {
        perror("Error open file");
        return EXIT_FAILURE;
    }

    srand48(time(NULL));

    for (int i = 0; i < num_points; i++) {
        double x = drand48() * 1000.0; 
        double y = drand48() * 1000.0; 
        double z = drand48() * 1000.0; 
        fprintf(file, "%f %f %f\n", x, y, z); 
    }

    fclose(file);
    return EXIT_SUCCESS;
}
