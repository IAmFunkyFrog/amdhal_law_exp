#include "matrix.h"
#include <omp.h>
#include <stdio.h>

static double random_double() {
    union {
        double d;
        unsigned char uc[sizeof(double)];
    } u;

    do {
        for (unsigned i = 0; i < sizeof(u.uc); i++) {
            u.uc[i] = (unsigned char) rand();
        }
    } while (!isfinite(u.d));

    while (u.d > 10.0 || u.d < -10.0) u.d /= 10.0;

    return u.d;
}

void matrix_fill_random(double_matrix_t matrix) {
    for (int i = 0; i < matrix.nrows; i++)
        for (int j = 0; j < matrix.ncols; j++)
            matrix_set(matrix, i, j, random_double());
}

#ifndef PARALLEL 

void matrix_mult3(double_matrix_t m1, double_matrix_t m2, double_matrix_t out) {
    assert(out.nrows == m1.nrows && out.ncols == m2.ncols);

    for (int i = 0; i < out.nrows; i++) {
        for (int j = 0; j < out.ncols; j++) {
            double val = 0.0;
            for (int k = 0; k < m2.nrows; k++)
                val += matrix_get(m1, i, k) * matrix_get(m2, k, j);
            matrix_set(out, i, j, val);
        }
    }
}

#else

void matrix_mult3(double_matrix_t m1, double_matrix_t m2, double_matrix_t out) {
    assert(out.nrows == m1.nrows && out.ncols == m2.ncols);

    int i, j, k;
    #pragma omp parallel for private(i, j, k)
    for (i = 0; i < out.nrows; i++) {
        for (j = 0; j < out.ncols; j++) {
            double val = 0.0;
            for (k = 0; k < m2.nrows; k++)
                val += matrix_get(m1, i, k) * matrix_get(m2, k, j);
            matrix_set(out, i, j, val);
        }
    }
}

#endif