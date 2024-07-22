#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_NAME_LEN 50

typedef struct hash_struct {
    uint32_t hash;
    char name[MAX_NAME_LEN];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

typedef struct {
    pthread_rwlock_t lock;
    hashRecord *head;
} hashTable;

hashTable table = { PTHREAD_RWLOCK_INITIALIZER, NULL };
FILE *outputFile;
int lockAcquisitions = 0;
int lockReleases = 0;

uint32_t hashFunction(const char *key) {
    uint32_t hash = 0;
    while (*key) {
        hash += *key++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void getTimeStamp(char *buffer) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(buffer, 20, "%ld%06ld", tv.tv_sec, tv.tv_usec);
}

void insert(const char *name, uint32_t salary) {
    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: INSERT,%u,%s,%u\n", timeStamp, hashFunction(name), name, salary);

    pthread_rwlock_wrlock(&table.lock);
    lockAcquisitions++;
    fprintf(outputFile, "%s: WRITE LOCK ACQUIRED\n", timeStamp);

    uint32_t hash = hashFunction(name);
    hashRecord *newNode = (hashRecord *)malloc(sizeof(hashRecord));
    newNode->hash = hash;
    strncpy(newNode->name, name, MAX_NAME_LEN);
    newNode->salary = salary;
    newNode->next = table.head;
    table.head = newNode;

    fprintf(outputFile, "%s: WRITE LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);
}

void delete(const char *name) {
    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: DELETE,%s\n", timeStamp, name);

    pthread_rwlock_wrlock(&table.lock);
    lockAcquisitions++;
    fprintf(outputFile, "%s: WRITE LOCK ACQUIRED\n", timeStamp);

    uint32_t hash = hashFunction(name);
    hashRecord *current = table.head;
    hashRecord *prev = NULL;

    while (current && current->hash != hash) {
        prev = current;
        current = current->next;
    }

    if (current) {
        if (prev) {
            prev->next = current->next;
        } else {
            table.head = current->next;
        }
        free(current);
    }

    fprintf(outputFile, "%s: WRITE LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);
}

hashRecord *search(const char *name) {
    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: SEARCH,%s\n", timeStamp, name);

    pthread_rwlock_rdlock(&table.lock);
    lockAcquisitions++;
    fprintf(outputFile, "%s: READ LOCK ACQUIRED\n", timeStamp);

    uint32_t hash = hashFunction(name);
    hashRecord *current = table.head;

    while (current && current->hash != hash) {
        current = current->next;
    }

    fprintf(outputFile, "%s: READ LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);

    return current;
}

void printTable() {
    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: PRINT\n", timeStamp);

    pthread_rwlock_rdlock(&table.lock);
    lockAcquisitions++;
    fprintf(outputFile, "%s: READ LOCK ACQUIRED\n", timeStamp);

    hashRecord *current = table.head;
    while (current) {
        fprintf(outputFile, "%u,%s,%u\n", current->hash, current->name, current->salary);
        current = current->next;
    }

    fprintf(outputFile, "%s: READ LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);
}

void *processCommand(void *arg) {
    char *command = (char *)arg;
    char cmd[10], name[MAX_NAME_LEN];
    uint32_t salary;

    sscanf(command, "%[^,],%[^,],%u", cmd, name, &salary);

    if (strcmp(cmd, "insert") == 0) {
        insert(name, salary);
    } else if (strcmp(cmd, "delete") == 0) {
        delete(name);
    } else if (strcmp(cmd, "search") == 0) {
        hashRecord *result = search(name);
        if (result) {
            fprintf(outputFile, "%u,%s,%u\n", result->hash, result->name, result->salary);
        } else {
            fprintf(outputFile, "No Record Found\n");
        }
    } else if (strcmp(cmd, "print") == 0) {
        printTable();
    }

    return NULL;
}

int main() {
    FILE *inputFile = fopen("commands.txt", "r");
    outputFile = fopen("output.txt", "w");

    if (!inputFile || !outputFile) {
        perror("Error opening file");
        return 1;
    }

    char line[256];
    int numThreads;
    fgets(line, sizeof(line), inputFile);
    sscanf(line, "threads,%d,0", &numThreads);

    pthread_t threads[numThreads];
    char *commands[numThreads];
    int commandCount = 0;

    while (fgets(line, sizeof(line), inputFile)) {
        commands[commandCount] = strdup(line);
        pthread_create(&threads[commandCount], NULL, processCommand, commands[commandCount]);
        commandCount++;
    }

    for (int i = 0; i < commandCount; i++) {
        pthread_join(threads[i], NULL);
        free(commands[i]);
    }

    fprintf(outputFile, "Finished all threads.\n");
    fprintf(outputFile, "Number of lock acquisitions: %d\n", lockAcquisitions);
    fprintf(outputFile, "Number of lock releases: %d\n", lockReleases);

    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: READ LOCK ACQUIRED\n", timeStamp);

    pthread_rwlock_rdlock(&table.lock);
    lockAcquisitions++;
    hashRecord *current = table.head;
    while (current) {
        fprintf(outputFile, "%u,%s,%u\n", current->hash, current->name, current->salary);
        current = current->next;
    }
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: READ LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);

    fclose(inputFile);
    fclose(outputFile);
    pthread_rwlock_destroy(&table.lock);

    return 0;
}
