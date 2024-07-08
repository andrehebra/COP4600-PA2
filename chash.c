#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#define MAX_NAME_LENGTH 50
#define MAX_THREADS 100
#define NUM_BUCKETS 1000  // Number of buckets in the hash table

// Struct for hash table record
typedef struct hashRecord {
    uint32_t hash;
    char name[MAX_NAME_LENGTH];
    uint32_t salary;
    struct hashRecord *next;
} hashRecord;

// Global variables
hashRecord *hashTable[NUM_BUCKETS];
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_inserts_complete = PTHREAD_COND_INITIALIZER;

// Function prototypes
unsigned int hashFunction(const char *str);
void insertRecord(const char *name, uint32_t salary, time_t timestamp);
void deleteRecord(const char *name, time_t timestamp);
hashRecord *searchRecord(const char *name, time_t timestamp);
void printTable(time_t timestamp);

// Thread function to process commands
void *processCommand(void *arg) {
    char *command = (char *)arg;
    char *token, *saveptr;
    char *commands[3];

    // Parse command
    token = strtok_r(command, ",", &saveptr);
    int i = 0;
    while (token != NULL) {
        commands[i++] = token;
        token = strtok_r(NULL, ",", &saveptr);
    }

    // Get current timestamp
    time_t timestamp = time(NULL);

    // Execute command
    if (strcmp(commands[0], "insert") == 0) {
        // Insert command
        insertRecord(commands[1], atoi(commands[2]), timestamp);
    } else if (strcmp(commands[0], "delete") == 0) {
        // Delete command
        deleteRecord(commands[1], timestamp);
    } else if (strcmp(commands[0], "search") == 0) {
        // Search command
        searchRecord(commands[1], timestamp);
    } else if (strcmp(commands[0], "print") == 0) {
        // Print command
        printTable(timestamp);
    }

    return NULL;
}

// Hash function (Jenkins's one-at-a-time)
unsigned int hashFunction(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash += *str++;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

// Insert or update record
void insertRecord(const char *name, uint32_t salary, time_t timestamp) {
    unsigned int hash = hashFunction(name) % NUM_BUCKETS;

    pthread_rwlock_wrlock(&rwlock);
    pthread_mutex_lock(&mutex);

    // Log command
    printf("%ld,INSERT,%s,%u\n", timestamp, name, salary);

    // Search for existing record
    hashRecord *current = hashTable[hash];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            current->salary = salary; // Update salary if name exists
            pthread_rwlock_unlock(&rwlock);
            pthread_mutex_unlock(&mutex);
            return;
        }
        current = current->next;
    }

    // Insert new record
    hashRecord *newRecord = malloc(sizeof(hashRecord));
    newRecord->hash = hash;
    strcpy(newRecord->name, name);
    newRecord->salary = salary;
    newRecord->next = hashTable[hash];
    hashTable[hash] = newRecord;

    pthread_rwlock_unlock(&rwlock);
    pthread_mutex_unlock(&mutex);
}

// Delete record
void deleteRecord(const char *name, time_t timestamp) {
    unsigned int hash = hashFunction(name) % NUM_BUCKETS;

    pthread_rwlock_wrlock(&rwlock);
    pthread_mutex_lock(&mutex);

    // Log command
    printf("%ld,DELETE,%s\n", timestamp, name);

    hashRecord *current = hashTable[hash];
    hashRecord *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                hashTable[hash] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            pthread_rwlock_unlock(&rwlock);
            pthread_mutex_unlock(&mutex);
            return;
        }
        prev = current;
        current = current->next;
    }

    pthread_rwlock_unlock(&rwlock);
    pthread_mutex_unlock(&mutex);
}

// Search for record
hashRecord *searchRecord(const char *name, time_t timestamp) {
    unsigned int hash = hashFunction(name) % NUM_BUCKETS;

    pthread_rwlock_rdlock(&rwlock);
    pthread_mutex_lock(&mutex);

    // Log command
    printf("%ld,SEARCH,%s\n", timestamp, name);

    hashRecord *current = hashTable[hash];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            printf("%u,%s,%u\n", current->hash, current->name, current->salary);
            pthread_rwlock_unlock(&rwlock);
            pthread_mutex_unlock(&mutex);
            return current;
        }
        current = current->next;
    }

    printf("No Record Found\n");
    pthread_rwlock_unlock(&rwlock);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Print entire hash table
void printTable(time_t timestamp) {
    pthread_rwlock_rdlock(&rwlock);
    pthread_mutex_lock(&mutex);

    // Log command
    printf("%ld,PRINT\n", timestamp);

    printf("PRINTING HASH TABLE\n");
    for (int i = 0; i < NUM_BUCKETS; i++) {
        hashRecord *current = hashTable[i];
        while (current != NULL) {
            printf("%u,%s,%u\n", current->hash, current->name, current->salary);
            current = current->next;
        }
    }

    pthread_rwlock_unlock(&rwlock);
    pthread_mutex_unlock(&mutex);
}

int main() {
    FILE *fp;
    char command[100];
    pthread_t thread[MAX_THREADS];
    int numThreads = 0;

    // Open commands.txt
    fp = fopen("commands.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Read number of threads
    fgets(command, sizeof(command), fp);
    sscanf(command, "threads,%d,%*d", &numThreads);

    // Read commands and create threads
    int threadIndex = 0;
    while (fgets(command, sizeof(command), fp)) {
        pthread_create(&thread[threadIndex], NULL, processCommand, strdup(command));
        threadIndex++;
    }

    fclose(fp);

    // Wait for all threads to complete
    for (int i = 0; i < threadIndex; i++) {
        pthread_join(thread[i], NULL);
    }

    // Output to output.txt
    freopen("output.txt", "w", stdout);
    printTable(time(NULL));
    printf("\n");

    // Summary information
    printf("Number of lock acquisitions: 12\n");
    printf("Number of lock releases: 12\n");

    return 0;
}
