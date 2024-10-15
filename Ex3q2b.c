#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_SIZE 100
#define SHM_KEY 12345
#define SHM_SIZE sizeof(SharedData)

typedef struct {
    int rows;
    int cols;
    double complex data[MAX_SIZE][MAX_SIZE];
} Matrix;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int head;
    int tail;
    char input1[16][128];
    char input2[16][128];
    char operation[16][16];
    int count;
    int ready;
} SharedData;

Matrix parse_matrix(char *input) {
    Matrix mat;
    sscanf(input, "(%d,%d:", &mat.rows, &mat.cols);
    char *values = strchr(input, ':') + 1;
    char *token = strtok(values, ",)");
    int i = 0, j = 0;
    while (token != NULL) {
        double real, imag;
        if (sscanf(token, "%lf%lfi", &real, &imag) == 2) {
            mat.data[i][j] = real + imag * I;
        } else if (sscanf(token, "%lfi", &imag) == 1) {
            if (strstr(token, "i") != NULL) {
                mat.data[i][j] = imag * I;
            } else {
                mat.data[i][j] = real;
            }
        }
        token = strtok(NULL, ",)");
        j++;
        if (j == mat.cols) {
            j = 0;
            i++;
        }
    }
    return mat;
}

void print_matrix(Matrix mat) {
    printf("(%d,%d:", mat.rows, mat.cols);

    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            double real = creal(mat.data[i][j]);
            double imag = cimag(mat.data[i][j]);

            if (real != 0) {
                if (imag == 0) {
                    if (real == (int)real) {
                        printf("%.0f", real);
                    } else {
                        printf("%.1f", real);
                    }
                } else if (imag < 0) {
                    if (real == (int)real) {
                        printf("%.0f-%.0fi", real, -imag);
                    } else {
                        printf("%.1f-%.1fi", real, -imag);
                    }
                } else {
                    if (real == (int)real) {
                        printf("%.0f+%.0fi", real, imag);
                    } else {
                        printf("%.1f+%.1fi", real, imag);
                    }
                }
            } else if (imag != 0) {
                if (imag == (int)imag) {
                    printf("%.0fi", imag);
                } else {
                    printf("%.1fi", imag);
                }
            } else {
                printf("0");
            }

            if (i != mat.rows - 1 || j != mat.cols - 1) {
                printf(",");
            }
        }
    }
    printf(")");
}

Matrix add_matrices(Matrix a, Matrix b) {
    Matrix result = {a.rows, a.cols, {{0}}};
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.data[i][j] = a.data[i][j] + b.data[i][j];
        }
    }
    return result;
}

Matrix sub_matrices(Matrix a, Matrix b) {
    Matrix result = {a.rows, a.cols, {{0}}};
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.data[i][j] = a.data[i][j] - b.data[i][j];
        }
    }
    return result;
}

Matrix mul_matrices(Matrix a, Matrix b) {
    Matrix result = {a.rows, a.cols, {{0}}};
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < b.cols; j++) {
            for (int k = 0; k < a.cols; k++) {
                result.data[i][j] += a.data[i][k] * b.data[k][j];
            }
        }
    }
    return result;
}

Matrix transpose_matrix(Matrix a) {
    Matrix result = {a.cols, a.rows, {{0}}};
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.data[j][i] = a.data[i][j];
        }
    }
    return result;
}

Matrix and_matrices(Matrix a, Matrix b) {
    Matrix result = {a.rows, a.cols, {{0}}};
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            if ((creal(a.data[i][j]) != 0 && creal(a.data[i][j]) != 1) ||
                (creal(b.data[i][j]) != 0 && creal(b.data[i][j]) != 1)) {
                printf("ERR\n");
                return result;
            }
            result.data[i][j] = ((int)creal(a.data[i][j]) & (int)creal(b.data[i][j])) + 0.0 * I;
        }
    }
    return result;
}

Matrix or_matrices(Matrix a, Matrix b) {
    Matrix result = {a.rows, a.cols, {{0}}};
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            if ((creal(a.data[i][j]) != 0 && creal(a.data[i][j]) != 1) ||
                (creal(b.data[i][j]) != 0 && creal(b.data[i][j]) != 1)) {
                printf("ERR\n");
                return result;
            }
            result.data[i][j] = (int)creal(a.data[i][j]) | (int)creal(b.data[i][j]);
        }
    }
    return result;
}

Matrix not_matrix(Matrix a) {
    Matrix result = {a.rows, a.cols, {{0}}};
    int flag = 0;

    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            if (creal(a.data[i][j]) != 0 && creal(a.data[i][j]) != 1) {
                flag = 1;
                break;
            }
            result.data[i][j] = !(int)creal(a.data[i][j]) + 0.0 * I;
        }
        if (flag) {
            result.rows = 0;
            result.cols = 0;
            break;
        }
    }
    if (flag) {
        printf("ERR\n");
    }
    return result;
}

int main() {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id == -1) {
        perror("shmget2");
        exit(1);
    }

    SharedData *shared_data = (SharedData*)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    while (1) {
        pthread_mutex_lock(&shared_data->mutex);
        while (!shared_data->ready || shared_data->count == 0) {
            pthread_cond_wait(&shared_data->cond, &shared_data->mutex);
        }

        char *input1 = shared_data->input1[shared_data->head];
        char *input2 = shared_data->input2[shared_data->head];
        char *operation = shared_data->operation[shared_data->head];

        if (strcmp(input1, "END") == 0 || strcmp(input2, "END") == 0 || strcmp(operation, "END") == 0) {
            pthread_mutex_unlock(&shared_data->mutex);
            break;
        }

        shared_data->head = (shared_data->head + 1) % 16;
        shared_data->count--;
        if (shared_data->count == 0) {
            shared_data->ready = 0;
        }
        pthread_mutex_unlock(&shared_data->mutex);

        Matrix a = parse_matrix(input1);
        Matrix result;

        if (strcmp(operation, "ADD") == 0) {
            Matrix b = parse_matrix(input2);
            result = add_matrices(a, b);
        } else if (strcmp(operation, "SUB") == 0) {
            Matrix b = parse_matrix(input2);
            result = sub_matrices(a, b);
        } else if (strcmp(operation, "MUL") == 0) {
            Matrix b = parse_matrix(input2);
            result = mul_matrices(a, b);
        } else if (strcmp(operation, "TRANSPOSE") == 0) {
            result = transpose_matrix(a);
        } else if (strcmp(operation, "AND") == 0) {
            Matrix b = parse_matrix(input2);
            result = and_matrices(a, b);
        } else if (strcmp(operation, "OR") == 0) {
            Matrix b = parse_matrix(input2);
            result = or_matrices(a, b);
        } else if (strcmp(operation, "NOT") == 0) {
            result = not_matrix(a);
        } else {
            printf("ERR\n");
            continue;
        }

        print_matrix(result);
        printf("\n");
        if (shared_data->ready == 0) break;
    }

    shmdt(shared_data);

    return 0;
}
