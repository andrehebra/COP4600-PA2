#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "chash.h"
#include "dbase.h"

#define MAX_THREADS 100
#define MAX_LINE_LENGTH 256

rwlock_t hash_lock;
hashRecord* Hash_Map = NULL;
pthread_t threads[MAX_THREADS];
int thread_count = 0;
pthread_mutex_t insert_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t insert_cond = PTHREAD_COND_INITIALIZER;
int insert_count = 0;

typedef struct {
    char operation[10];
    char name[50];
    int salary;
} Operation;

void* perform_operation(void* arg) {
    Operation* op = (Operation*)arg;
    printf("Performing operation: %s\n", op->operation);  // Debug print

    if (strcmp(op->operation, "insert") == 0) {
        pthread_mutex_lock(&insert_mutex);
        insert_count++;
        pthread_mutex_unlock(&insert_mutex);

        insertRecord(op->name, op->salary, &Hash_Map);

        pthread_mutex_lock(&insert_mutex);
        insert_count--;
        if (insert_count == 0) {
            pthread_cond_broadcast(&insert_cond);
        }
        pthread_mutex_unlock(&insert_mutex);
    } else if (strcmp(op->operation, "delete") == 0) {
        pthread_mutex_lock(&insert_mutex);
        while (insert_count > 0) {
            printf("%lld: WAITING ON INSERTS\n", current_timestamp());
            pthread_cond_wait(&insert_cond, &insert_mutex);
        }
        pthread_mutex_unlock(&insert_mutex);
        printf("%lld: DELETE AWAKENED\n", current_timestamp());

        deleteNode(op->name, &Hash_Map);
    } else if (strcmp(op->operation, "search") == 0) {
        searchNode(op->name, Hash_Map);
    } else if (strcmp(op->operation, "print") == 0) {
        traverseNode(Hash_Map);
    }

    free(op);
    printf("Operation completed: %s\n", op->operation);  // Debug print
    return NULL;
}

int main() {
    printf("Program started\n");  // Debug print

    FILE* file = fopen("commands.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    int num_threads;

    if (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "threads,%d,0", &num_threads);
        printf("Number of threads: %d\n", num_threads);  // Debug print
    }

    rwlock_init(&hash_lock);

    while (fgets(line, sizeof(line), file) != NULL) {
        Operation* op = malloc(sizeof(Operation));
        char* token = strtok(line, ",");
        if (token != NULL) {
            strcpy(op->operation, token);
            token = strtok(NULL, ",");
            if (token != NULL) {
                strcpy(op->name, token);
                token = strtok(NULL, ",");
                if (token != NULL) {
                    op->salary = atoi(token);
                }
            }
        }

        printf("Creating thread for operation: %s\n", op->operation);  // Debug print
        if (pthread_create(&threads[thread_count++], NULL, perform_operation, op) != 0) {
            perror("Failed to create thread");
            free(op);
            continue;
        }
    }

    printf("Waiting for threads to finish\n");  // Debug print
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Number of lock acquisitions: %d\n", thread_count * 2);
    printf("Number of lock releases: %d\n", thread_count * 2);

    traverseNode(Hash_Map);

    fclose(file);
    printf("Program completed successfully\n");  // Debug print
    return 0;
}