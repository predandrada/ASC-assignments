/*
 * Tema 2 ASC
 * 2020 Spring
 */
#include "utils.h"

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

double *get_transpose(int N, double *mat) {
	double *res = (double*) calloc(N * N, sizeof(double));
	int i, j;

	for (i = 0; i < N; ++i) {
		for (j = i; j < N; ++j) {
			res[j * N + i] = mat[i * N + j];
		}
	}

	return res;
}

double *get_squared(int N, double *mat) {
	double *res = (double*) calloc(N * N, sizeof(double));
	int i, j, k;

	for (j = 0; j < N; ++j) {
		for (i = 0; i <= j; ++i) {
			for (k = i; k <= j; ++k) {
				res[i * N + j] += mat[i * N + k] * mat[k * N + j];
			}
		}
	}

	return res;
}

double *multiply_matrices(int N, double *a, double *b) {
	double *res = (double*) calloc(N * N, sizeof(double));

	int i, j, k;
	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			for (k = 0; k < N; ++k) {
				res[i * N + j] += a[i * N + k] * b[k * N + j];
			}
		}
	}
	return res;
}

double *get_sum(int N, double *a, double *b) {
	double *res = (double*) calloc(N * N, sizeof(double));
	int i, j;

	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			res[i * N + j] += a[i * N + j] + b[i * N + j]; 
		}
	}

	return res;
}
/*
 * Add your unoptimized implementation here
 */
double* my_solver(int N, double *A, double* B) {
	// B * A_t
	double *A_transpose = get_transpose(N, A);
	double *first_result = multiply_matrices(N, B, A_transpose);
	
	// A^2 * B
	double *A_squared = get_squared(N, A);
	double *second_result = multiply_matrices(N, A_squared, B);

	return get_sum(N, first_result, second_result);
}
