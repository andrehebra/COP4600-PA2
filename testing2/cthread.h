#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>
#include <semaphore.h> // For POSIX semaphores

typedef struct _pthread_s{// array of threads
    pthread_t *threads;
    int num_of_threads;
}pthread_s;

// Define the read-write lock structure
typedef struct _rwlock_t {
    sem_t writelock;   // Semaphore for write locks
    sem_t lock;        // Semaphore for reader count protection
    int readers;       // Number of active readers
} rwlock_t;

// Function prototypes
pthread_s *threads_init(int num);
/**
 * Initialize the read-write lock.
 * @param lock Pointer to the rwlock_t structure to initialize.
 */
void rwlock_init(rwlock_t *lock);

/**
 * Acquire a read lock.
 * @param lock Pointer to the rwlock_t structure to lock.
 */
void rwlock_acquire_readlock(rwlock_t *lock);

/**
 * Release a read lock.
 * @param lock Pointer to the rwlock_t structure to unlock.
 */
void rwlock_release_readlock(rwlock_t *lock);

/**
 * Acquire a write lock.
 * @param lock Pointer to the rwlock_t structure to lock.
 */
void rwlock_acquire_writelock(rwlock_t *lock);

/**
 * Release a write lock.
 * @param lock Pointer to the rwlock_t structure to unlock.
 */
void rwlock_release_writelock(rwlock_t *lock);

extern rwlock_t mutex;
extern sem_t cond;


#endif // RWLOCK_H
