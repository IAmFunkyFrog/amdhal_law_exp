#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

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

static inline size_t matrix_serialized_size(double_matrix_t m) {
    return sizeof(double_matrix_t) + sizeof(double) * m.ncols * m.nrows;
}

static inline void matrix_serialize_to_buffer_inplace(void *buf, double_matrix_t m) {
    int *ncols = (int *) buf;
    int *nrows = (int *) (buf + sizeof(int));
    void *data = buf + 2 * sizeof(int);

    *ncols = m.ncols;
    *nrows = m.nrows;
    memcpy(data, m.data, m.ncols * m.nrows * sizeof(double));
}

static inline void *matrix_serialize_to_buffer(double_matrix_t m) {
    size_t size = matrix_serialized_size(m);
    void *buf = malloc(size);
    matrix_serialize_to_buffer_inplace(buf, m);
    return buf;
}

static inline void matrix_deserialize_from_buffer_inplace(void *buf, double_matrix_t *m) {
    int *ncols = (int *) buf;
    int *nrows = (int *) (buf + sizeof(int));
    void *data = buf + 2 * sizeof(int);

    m->ncols = *ncols;
    m->nrows = *nrows;
    m->data = malloc(m->ncols * m->nrows * sizeof(double));
    memcpy(m->data, data, m->ncols * m->nrows * sizeof(double));
}

static inline double_matrix_t matrix_deserialize_from_buffer(void *buf) {
    double_matrix_t m;
    matrix_deserialize_from_buffer_inplace(buf, &m);
    return m;
}
