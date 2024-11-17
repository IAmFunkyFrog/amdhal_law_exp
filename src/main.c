#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include "matrix.h"
#include <omp.h>

#define HELP \
    "\t-p - set count of OMP processes\n" \
    "\t-d - set matrix dimension size\n" \
    "\t-v - print A after calculations" \
    "\t-s - set seed for random matrix fill" \
    "\t-t - print elapsed time (only for parallel build!)"

int main(int argc, char *argv[]) {
    int pcount = 0;
    int dimension_size = 128;
    int verbose = 0;
    int random_seed = 42;

    int print_elapsed_time = 0;

    int opt;
    while ((opt = getopt(argc, argv, "p:d:vs:t")) != -1) {
        switch (opt) {
        case 'p':
            pcount = atoi(optarg);
            assert(pcount > 0);
            break;
        case 'd':
            dimension_size = atoi(optarg);
            assert(dimension_size > 0);
            break;
        case 'v':
            verbose = 1;
            break;
        case 's':
            random_seed = atoi(optarg);
            break;
        case 't':
            print_elapsed_time = 1;
            break;
        default:
            fprintf(stderr, HELP);
            return -1;
        }
    }

    // (0) initialize random and num threads
    srand(random_seed);
    if (pcount > 0)
        omp_set_num_threads(pcount);

    // (1) allocate and randomly fill matrices
    double_matrix_t B = matrix_allocate(dimension_size, dimension_size);
    double_matrix_t x = matrix_allocate(dimension_size, 1);
    double_matrix_t y = matrix_allocate(1, dimension_size);
    matrix_fill_random(B);
    matrix_fill_random(x);
    matrix_fill_random(y);

    // (2) calculate A
    double_matrix_t scal_prod_x_y = matrix_mult2(y, x);

    double A = 1.0;
    double_matrix_t Bi = B;

    double start_time = omp_get_wtime();


    for (int i = 1; i < 5; i++) {
        double_matrix_t Bx = matrix_mult2(Bi, x);
        double_matrix_t scal_prod_Bx_y = matrix_mult2(y, Bx);

        A += matrix_get(scal_prod_Bx_y, 0, 0) / matrix_get(scal_prod_x_y, 0, 0);
        Bi = matrix_mult2(Bi, B);
    }

    double end_time = omp_get_wtime();

    // (3) print A if needed
    if (verbose)
        printf("%.3f\n", A);

    // (4) print elapsed time
    if (print_elapsed_time) {
        printf("%.6f\n", end_time - start_time);
    }

    return 0;
}