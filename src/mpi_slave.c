#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include "matrix.h"
#include "mpi.h"
#include "mpi_slave.h"

void mpi_slave_main(int dimension_size) {
    size_t max_recv_bytes = matrix_serialized_size((double_matrix_t){
        .ncols = dimension_size,
        .nrows = dimension_size,
        .data = NULL
    });
    void *buf = malloc(max_recv_bytes);

    int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while(true) {
        MPI_Status status;
        MPI_Recv(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* src */ 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int tag = status.MPI_TAG;
        assert(status.MPI_SOURCE == 0 && "only main thread should send requests");
        
        if (tag == KILL_YOURSELF_TAG) break;

        if (tag == MULTIPLY_ARG1_TAG) {
            double_matrix_t m1 = matrix_deserialize_from_buffer(buf);
            MPI_Recv(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* src */ 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            double_matrix_t result;
            if (status.MPI_TAG == MULTIPLY_ARG2_TAG) {
                double_matrix_t m2 = matrix_deserialize_from_buffer(buf);
                result = matrix_mult2(m1, m2);
            } else {
                assert(status.MPI_TAG == MULTIPLY_POW_TAG);
                int pow = *(int *)buf;
                result = m1;
                for (int i = 0; i < pow - 1; i++) {
                    result = matrix_mult2(result, m1);
                }
            }
            matrix_serialize_to_buffer_inplace(buf, result);
            MPI_Send(buf, max_recv_bytes, MPI_UNSIGNED_CHAR, /* dest */ 0, MULTIPLY_RESULT_TAG, MPI_COMM_WORLD);
        }
    }
}