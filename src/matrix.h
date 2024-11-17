#include <assert.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int ncols;
    int nrows;
    void *data;
} double_matrix_t;

static inline double matrix_get(double_matrix_t matrix, int i, int j) {
    assert (i >= 0 && i < matrix.nrows && j >= 0 && j <= matrix.ncols);
    double *plain = (double *) matrix.data;
    return plain[i * matrix.ncols + j];
}

static inline void matrix_set(double_matrix_t matrix, int i, int j, double value) {
    assert (i >= 0 && i < matrix.nrows && j >= 0 && j <= matrix.ncols);
    double *plain = (double *) matrix.data;
    plain[i * matrix.ncols + j] = value;
}

static inline double_matrix_t matrix_allocate(int nrows, int ncols) {
    assert(ncols > 0 && nrows > 0);
    return (double_matrix_t) {
        .ncols = ncols,
        .nrows = nrows,
        .data = malloc(sizeof(double) * ncols * nrows)
    };
}

static inline void matrix_free(double_matrix_t matrix) {
    free(matrix.data);
}

void matrix_fill_random(double_matrix_t matrix);

void matrix_mult3(double_matrix_t m1, double_matrix_t m2, double_matrix_t out);

static inline double_matrix_t matrix_mult2(double_matrix_t m1, double_matrix_t m2) {
    double_matrix_t out = matrix_allocate(m1.nrows, m2.ncols);
    matrix_mult3(m1, m2, out);
    return out;
}
