#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_NAME_LEN 50

typedef struct hashRecord
{
    uint32_t hash;
    char name[MAX_NAME_LEN];
    uint32_t salary;
    struct hashRecord *next;
} hashRecord;

typedef struct hashTable
{
    pthread_rwlock_t lock;
    hashRecord *head;
} hashTable;

hashTable table = {PTHREAD_RWLOCK_INITIALIZER, NULL};
FILE *outputFile;
int lockAcquisitions = 0;
int lockReleases = 0;

pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
int waitingOnInserts = 0;

uint32_t jenkins_one_at_a_time_hash(const char *key)
{
    uint32_t hash = 0;
    while (*key)
    {
        hash += *key++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void getTimeStamp(char *buffer)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(buffer, 20, "%ld%06ld", tv.tv_sec, tv.tv_usec);
}

void insert(const char *name, uint32_t salary)
{
    char timeStamp[20];
    uint32_t hash = jenkins_one_at_a_time_hash(name);

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: INSERT,%u,%s,%u\n", timeStamp, hash, name, salary);

    // Attempt to acquire the write lock
    pthread_mutex_lock(&waitMutex);
    int wasWaiting = waitingOnInserts;
    waitingOnInserts = 1;
    pthread_mutex_unlock(&waitMutex);

    if (wasWaiting)
    {
        getTimeStamp(timeStamp);
        fprintf(outputFile, "%s: WAITING ON INSERTS\n", timeStamp);
    }

    int lock_acquired = pthread_rwlock_wrlock(&table.lock);
    if (lock_acquired != 0)
    {
        fprintf(stderr, "%s: WRITE LOCK ACQUISITION FAILED\n", timeStamp);
        return; // Handle the failure as appropriate
    }
    lockAcquisitions++;

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: WRITE LOCK ACQUIRED\n", timeStamp);

    // Allocate memory for the new node
    hashRecord *newNode = (hashRecord *)malloc(sizeof(hashRecord));
    if (newNode == NULL)
    {
        fprintf(stderr, "%s: MEMORY ALLOCATION FAILED\n", timeStamp);
        pthread_rwlock_unlock(&table.lock); // Ensure the lock is released in case of failure
        lockReleases++;

        getTimeStamp(timeStamp);
        fprintf(outputFile, "%s: WRITE LOCK RELEASED\n", timeStamp);
        return;
    }

    // Initialize the new node
    newNode->hash = hash;
    strncpy(newNode->name, name, MAX_NAME_LEN);
    newNode->name[MAX_NAME_LEN - 1] = '\0'; // Ensure null termination
    newNode->salary = salary;
    newNode->next = table.head;
    table.head = newNode;

    // Release the write lock
    pthread_rwlock_unlock(&table.lock);
    lockReleases++;

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: WRITE LOCK RELEASED\n", timeStamp);

    pthread_mutex_lock(&waitMutex);
    waitingOnInserts = 0;
    pthread_mutex_unlock(&waitMutex);
}
void delete(const char *name)
{
    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: DELETE AWAKENED\n", timeStamp);

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: DELETE,%s\n", timeStamp, name);

    pthread_rwlock_wrlock(&table.lock);
    lockAcquisitions++;

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: WRITE LOCK ACQUIRED\n", timeStamp);

    uint32_t hash = jenkins_one_at_a_time_hash(name);
    hashRecord *current = table.head;
    hashRecord *prev = NULL;

    while (current && current->hash != hash)
    {
        prev = current;
        current = current->next;
    }

    if (current)
    {
        if (prev)
        {
            prev->next = current->next;
        }
        else
        {
            table.head = current->next;
        }
        free(current);
    }

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: WRITE LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);
}

hashRecord *search(const char *name)
{
    char timeStamp[20];
    getTimeStamp(timeStamp);

    // Acquire the read lock
    int lock_acquired = pthread_rwlock_rdlock(&table.lock);
    if (lock_acquired != 0)
    {
        fprintf(stderr, "%s: READ LOCK ACQUISITION FAILED\n", timeStamp);
        return NULL; // Handle the failure as appropriate
    }
    lockAcquisitions++;
    fprintf(outputFile, "%s: READ LOCK ACQUIRED\n", timeStamp);

    // Perform the search
    uint32_t hash = jenkins_one_at_a_time_hash(name);
    hashRecord *current = table.head;
    hashRecord *found = NULL;

    while (current)
    {
        if (current->hash == hash && strcmp(current->name, name) == 0)
        {
            found = current;
            break;
        }
        current = current->next;
    }

    // Log search results
    if (found)
    {
        fprintf(outputFile, "%s: SEARCH: FOUND,%u,%s,%u\n", timeStamp, found->hash, found->name, found->salary);
    }
    else
    {
        fprintf(outputFile, "%s: SEARCH: NOT FOUND NOT FOUND\n", timeStamp);
    }

    // Release the read lock
    pthread_rwlock_unlock(&table.lock);
    lockReleases++;
    fprintf(outputFile, "%s: READ LOCK RELEASED\n", timeStamp);

    return found;
}

void *processCommand(void *arg)
{
    char *command = (char *)arg;
    char cmd[10], name[MAX_NAME_LEN];
    uint32_t salary;

    sscanf(command, "%[^,],%[^,],%u", cmd, name, &salary);

    if (strcmp(cmd, "insert") == 0)
    {
        insert(name, salary);
    }
    else if (strcmp(cmd, "delete") == 0)
    {
        delete (name);
    }
    else if (strcmp(cmd, "search") == 0)
    {
        hashRecord *result = search(name);
        
    }
    

    return NULL;
}

int compareRecords(const void *a, const void *b)
{
    hashRecord *recordA = *(hashRecord **)a;
    hashRecord *recordB = *(hashRecord **)b;
    return (recordA->hash - recordB->hash);
}

void printSortedTable()
{
    pthread_rwlock_rdlock(&table.lock);
    lockAcquisitions++;
    char timeStamp[20];
    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: READ LOCK ACQUIRED\n", timeStamp);

    // Count the number of records
    int count = 0;
    hashRecord *current = table.head;
    while (current)
    {
        count++;
        current = current->next;
    }

    // Collect all records in an array
    hashRecord **records = (hashRecord **)malloc(count * sizeof(hashRecord *));
    current = table.head;
    for (int i = 0; i < count; i++)
    {
        records[i] = current;
        current = current->next;
    }

    // Sort the array by hash value
    qsort(records, count, sizeof(hashRecord *), compareRecords);

    // Print the sorted records
    for (int i = 0; i < count; i++)
    {
        fprintf(outputFile, "%u,%s,%u\n", records[i]->hash, records[i]->name, records[i]->salary);
    }

    free(records);

    getTimeStamp(timeStamp);
    fprintf(outputFile, "%s: READ LOCK RELEASED\n", timeStamp);
    lockReleases++;
    pthread_rwlock_unlock(&table.lock);
}

int main()
{
    FILE *inputFile = fopen("commands.txt", "r");
    outputFile = fopen("output.txt", "w");

    if (!inputFile || !outputFile)
    {
        perror("Error opening file");
        return 1;
    }

    char line[256];
    int numThreads;
    fgets(line, sizeof(line), inputFile);
    sscanf(line, "threads,%d,0", &numThreads);

    // Write the number of threads at the top of the output file
    fprintf(outputFile, "Running %d threads\n", numThreads);

    pthread_t threads[numThreads];
    char *commands[numThreads];
    int commandCount = 0;

    while (fgets(line, sizeof(line), inputFile))
    {
        commands[commandCount] = strdup(line);
        pthread_create(&threads[commandCount], NULL, processCommand, commands[commandCount]);
        commandCount++;
    }

    for (int i = 0; i < commandCount; i++)
    {
        pthread_join(threads[i], NULL);
        free(commands[i]);
    }

    fprintf(outputFile, "Finished all threads.\n");
    fprintf(outputFile, "Number of lock acquisitions: %d\n", lockAcquisitions);
    fprintf(outputFile, "Number of lock releases: %d\n", lockReleases);

    printSortedTable();

    fclose(inputFile);
    fclose(outputFile);
    pthread_rwlock_destroy(&table.lock);

    return 0;
}
