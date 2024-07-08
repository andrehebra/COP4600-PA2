#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

#define NUM_BUCKETS 1000

// Struct for hash table record
typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hash_struct;

// Global variables
hash_struct *hashTable[NUM_BUCKETS];
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Function prototypes
unsigned int hashFunction(const char *str);
void insertRecord(const char *name, uint32_t salary);
void deleteRecord(const char *name);
hash_struct *searchRecord(const char *name);
void printTable();

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

    // Execute command
    if (strcmp(commands[0], "insert") == 0) {
        // Insert command
        insertRecord(commands[1], atoi(commands[2]));
    } else if (strcmp(commands[0], "delete") == 0) {
        // Delete command
        deleteRecord(commands[1]);
    } else if (strcmp(commands[0], "search") == 0) {
        // Search command
        hash_struct *record = searchRecord(commands[1]);
        if (record != NULL) {
            printf("%u,%s,%u\n", record->hash, record->name, record->salary);
        } else {
            printf("No Record Found\n");
        }
    } else if (strcmp(commands[0], "print") == 0) {
        // Print command
        printTable();
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
void insertRecord(const char *name, uint32_t salary) {
    unsigned int hash = hashFunction(name) % NUM_BUCKETS;

    pthread_rwlock_wrlock(&rwlock);

    // Search for existing record
    hash_struct *current = hashTable[hash];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            current->salary = salary; // Update salary if name exists
            pthread_rwlock_unlock(&rwlock);
            return;
        }
        current = current->next;
    }

    // Insert new record
    hash_struct *newRecord = malloc(sizeof(hash_struct));
    newRecord->hash = hash;
    strcpy(newRecord->name, name);
    newRecord->salary = salary;
    newRecord->next = hashTable[hash];
    hashTable[hash] = newRecord;

    pthread_rwlock_unlock(&rwlock);
}

// Delete record
void deleteRecord(const char *name) {
    unsigned int hash = hashFunction(name) % NUM_BUCKETS;

    pthread_rwlock_wrlock(&rwlock);

    hash_struct *current = hashTable[hash];
    hash_struct *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                hashTable[hash] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            pthread_rwlock_unlock(&rwlock);
            return;
        }
        prev = current;
        current = current->next;
    }

    pthread_rwlock_unlock(&rwlock);
}

// Search for record
hash_struct *searchRecord(const char *name) {
    unsigned int hash = hashFunction(name) % NUM_BUCKETS;

    pthread_rwlock_rdlock(&rwlock);

    hash_struct *current = hashTable[hash];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            pthread_rwlock_unlock(&rwlock);
            return current;
        }
        current = current->next;
    }

    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

// Print entire hash table
void printTable() {
    pthread_rwlock_rdlock(&rwlock);

    printf("PRINTING HASH TABLE\n");
    for (int i = 0; i < NUM_BUCKETS; i++) {
        hash_struct *current = hashTable[i];
        while (current != NULL) {
            printf("%u,%s,%u\n", current->hash, current->name, current->salary);
            current = current->next;
        }
    }

    pthread_rwlock_unlock(&rwlock);
}

int main() {
    FILE *fp;
    char command[100];
    pthread_t thread[100];
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

    // Output to output.txt (modify based on your output needs)
    freopen("output.txt", "w", stdout);
    printTable();

    return 0;
}
