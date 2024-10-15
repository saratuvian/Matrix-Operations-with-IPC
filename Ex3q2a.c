#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <pthread.h>

#define SHM_KEY 12345
#define SHM_SIZE sizeof(SharedData)

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

int main() {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget1");
        exit(1);
    }

    SharedData *shared_data = (SharedData*)shmat(shm_id, NULL, 0);
    if (shared_data == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_condattr_init(&cond_attr);

    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&shared_data->mutex, &mutex_attr);
    pthread_cond_init(&shared_data->cond, &cond_attr);

    shared_data->head = 0;
    shared_data->tail = 0;
    shared_data->count = 0;
    shared_data->ready = 0;

    while (1) {
        scanf("%s", shared_data->input1[shared_data->tail]);
        scanf("%s", shared_data->input2[shared_data->tail]);

        if (strcmp(shared_data->input1[shared_data->tail], "END") == 0 ||
            strcmp(shared_data->input2[shared_data->tail], "END") == 0) {
            break;
        }

        if (strcmp(shared_data->input2[shared_data->tail], "TRANSPOSE") != 0 &&
            strcmp(shared_data->input2[shared_data->tail], "NOT") != 0) {
            scanf("%s", shared_data->operation[shared_data->tail]);
            if(strcmp(shared_data->operation[shared_data->tail], "END") == 0) {
                break;
            }
        } else {
            strcpy(shared_data->operation[shared_data->tail], shared_data->input2[shared_data->tail]);
            strcpy(shared_data->input2[shared_data->tail], "");
        }

        pthread_mutex_lock(&shared_data->mutex);

        shared_data->tail = (shared_data->tail + 1) % 16;
        shared_data->count++;
        shared_data->ready = 1;
        pthread_cond_signal(&shared_data->cond);
        pthread_mutex_unlock(&shared_data->mutex);
    }

    pthread_mutex_lock(&shared_data->mutex);
    shared_data->ready = 1;
    strcpy(shared_data->input1[shared_data->tail], "END");
    pthread_cond_signal(&shared_data->cond);
    pthread_mutex_unlock(&shared_data->mutex);

    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);

    if (shmdt(shared_data) == -1) {
        perror("shmdt");
        exit(1);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    return 0;
}
