#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include "matrix.h"
#include <omp.h>

#define HELP \
    "\t-p - set count of OMP processes\n" \
    "\t-d - set matrix dimension size\n" \
    "\t-v - print A after calculations\n" \
    "\t-s - set seed for random matrix fill\n" \
    "\t-t - print elapsed time (only for parallel build!)\n"

#define MAX_ITERS 4

static int verbose = 0;

#ifdef MPI_BUILD
#include "mpi_slave.h"
#include "mpi.h"

void compute_mutliplications(int dimension_size, double_matrix_t *Ni, double_matrix_t B, double_matrix_t x, double_matrix_t y) {
    size_t max_recv_bytes = matrix_serialized_size((double_matrix_t){
        .ncols = dimension_size,
        .nrows = dimension_size,
        .data = NULL
    });
    void *buf = malloc(max_recv_bytes);

    // (1) Compute B^i
    assert(MAX_ITERS == 4 && "code written for this case");
    Ni[1] = B;
    Ni[2] = matrix_mult2(B, B);

    matrix_serialize_to_buffer_inplace(buf, Ni[1]);
    MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ 3, MULTIPLY_ARG1_TAG, MPI_COMM_WORLD);
    matrix_serialize_to_buffer_inplace(buf, Ni[2]);
    MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ 3, MULTIPLY_ARG2_TAG, MPI_COMM_WORLD);

    matrix_serialize_to_buffer_inplace(buf, Ni[2]);
    MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ 4, MULTIPLY_ARG1_TAG, MPI_COMM_WORLD);
    matrix_serialize_to_buffer_inplace(buf, Ni[2]);
    MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ 4, MULTIPLY_ARG2_TAG, MPI_COMM_WORLD);

    for (int i = 3; i < MAX_ITERS + 1; i++) {
        MPI_Status status;
        MPI_Recv(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* src */ i, MULTIPLY_RESULT_TAG, MPI_COMM_WORLD, &status);
        Ni[i] = matrix_deserialize_from_buffer(buf);
    }

    // (2) Compute B^i * x
    for (int i = 1; i < MAX_ITERS + 1; i++) {
        matrix_serialize_to_buffer_inplace(buf, Ni[i]);
        MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ i, MULTIPLY_ARG1_TAG, MPI_COMM_WORLD);
        matrix_serialize_to_buffer_inplace(buf, x);
        MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ i, MULTIPLY_ARG2_TAG, MPI_COMM_WORLD);
    }

    for (int i = 1; i < MAX_ITERS + 1; i++) {
        MPI_Status status;
        MPI_Recv(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* src */ i, MULTIPLY_RESULT_TAG, MPI_COMM_WORLD, &status);
        Ni[i] = matrix_deserialize_from_buffer(buf);
    }

    // (3) Compute <B^i * x, y>
    for (int i = 1; i < MAX_ITERS + 1; i++) {
        matrix_serialize_to_buffer_inplace(buf, y);
        MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ i, MULTIPLY_ARG1_TAG, MPI_COMM_WORLD);
        matrix_serialize_to_buffer_inplace(buf, Ni[i]);
        MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ i, MULTIPLY_ARG2_TAG, MPI_COMM_WORLD);
    }

    for (int i = 1; i < MAX_ITERS + 1; i++) {
        MPI_Status status;
        MPI_Recv(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* src */ i, MULTIPLY_RESULT_TAG, MPI_COMM_WORLD, &status);
        Ni[i] = matrix_deserialize_from_buffer(buf);
    }

    return;
}
#endif

int main(int argc, char *argv[]) {
#ifdef MPI_BUILD
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    assert(size == (MAX_ITERS + 1) && "expected only MAX_ITERS + 1 processes");
#endif

    int pcount = 0;
    int dimension_size = 128;
    int random_seed = 42;

    int print_elapsed_time = 0;

    int opt;
    while ((opt = getopt(argc, argv, "p:d:vs:t")) != -1) {
        switch (opt) {
        case 'p':
            pcount = atoi(optarg);
            assert(pcount >= 0);
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

    if (pcount > 0)
        omp_set_num_threads(pcount);

#ifdef MPI_BUILD
    if (rank != 0) {
        mpi_slave_main(dimension_size);
        MPI_Finalize();
        return 0;
    }
#endif

    // (0) initialize random
    srand(random_seed);

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

    double start_time = omp_get_wtime();


#ifdef MPI_BUILD
    double_matrix_t Ni[MAX_ITERS + 1];
    compute_mutliplications(dimension_size, Ni, B, x, y);
    for (int i = 1; i < MAX_ITERS + 1; i++) {
        A += matrix_get(Ni[i], 0, 0) / matrix_get(scal_prod_x_y, 0, 0);
    }
#else
    double_matrix_t Bi[MAX_ITERS + 1];
    Bi[0] = B;
    for (int i = 1; i < MAX_ITERS + 1; i++) {
        Bi[i] = matrix_mult2(Bi[i - 1], B);
    }

    double partial_results[MAX_ITERS + 1];
    for (int i = 1; i < MAX_ITERS + 1; i++) {
#ifdef PARALLEL
        #pragma omp task shared(partial_results, Bi, x, y)
#endif 
        {
        double_matrix_t Bx = matrix_mult2(Bi[i - 1], x);
        double_matrix_t scal_prod_Bx_y = matrix_mult2(y, Bx);

        partial_results[i] = matrix_get(scal_prod_Bx_y, 0, 0) / matrix_get(scal_prod_x_y, 0, 0);
        }
    }
#ifdef PARALLEL
    #pragma omp taskwait
#endif 
    for (int i = 1; i < MAX_ITERS + 1; i++) {
        A += partial_results[i];
    }
#endif

    double end_time = omp_get_wtime();

    // (3) print A if needed
    if (verbose)
        printf("%.3f\n", A);

    // (4) print elapsed time
    if (print_elapsed_time) {
        printf("%.6f\n", end_time - start_time);
    }

#ifdef MPI_BUILD
    for (int i = 1; i < MAX_ITERS + 1; i++)
        MPI_Send(NULL, 0, MPI_DATATYPE_NULL, /* dest */ i, KILL_YOURSELF_TAG, MPI_COMM_WORLD);
    MPI_Finalize();
#endif
    return 0;
}