/*
 * Tema 2 ASC
 * 2020 Spring
 */
#include "utils.h"
#include <cblas.h>
#include <stdlib.h>

/* 
 * Add your BLAS implementation here
 */
double *copy_mat(int N, double *mat) {
	double *res = (double*) calloc(N * N, sizeof(double));
	int i, j;

	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			res[i * N + j] = mat[i * N + j];
		}
	}
	return res;
}

double* my_solver(int N, double *A, double *B) {
	int i, j;

	double *result = copy_mat(N, B);
	// B * A_transpose
	// row_major, right_side, upper_triangular, transpose, non_unit
	// the result is in "result"
	cblas_dtrmm(101, 142, 121, 112, 131, N, N, 1, A, N, result, N);
	
	double *A_squared = copy_mat(N, A);
	// A^2
	cblas_dtrmm(101, 142, 121, 111, 131, N, N, 1, A, N, A_squared, N);

	// A^2 * B
	// the result is in B
	cblas_dtrmm(101, 141, 121, 111, 131, N, N, 1, A_squared, N, B, N);
	
	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			result[i * N + j] += B[i * N + j];
		}
	}	

	return result;
}
