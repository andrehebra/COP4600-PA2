#ifndef CHASH_H
#define CHASH_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

// Hash record structure
typedef struct hash_struct
{
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

// Reader-writer lock structure
typedef struct {
    sem_t lock;
    sem_t writelock;
    int readers;
} rwlock_t;

// Function declarations for hash operations
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length);
hashRecord* createRecord(char* name, int salary);
void insertRecord(char* name, int salary, hashRecord** head);
void deleteNode(char* name, hashRecord** head);
int searchNode(char* name, hashRecord* head);
void traverseNode(hashRecord* head);

// Function declarations for reader-writer lock operations
void rwlock_init(rwlock_t *rw);
void rwlock_acquire_readlock(rwlock_t *rw);
void rwlock_release_readlock(rwlock_t *rw);
void rwlock_acquire_writelock(rwlock_t *rw);
void rwlock_release_writelock(rwlock_t *rw);

// Timestamp function declaration
long long current_timestamp();

// Logging function declarations
void log_operation(const char* operation, const char* name, uint32_t salary);
void log_lock(const char* lock_type, const char* action);

#endif // CHASH_H