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
    struct hash_struct *next;
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

uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length)
{
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length)
    {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

int main()
{

    char test[10];

    printf("%u", jenkins_one_at_a_time_hash("asdf", 50));

    return 0;
}