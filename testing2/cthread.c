#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


//threads[i] = pthread_create() @the function that assigns its command


typedef struct pthread_s{// array of threads
    pthread_t *threads;
    int num_of_threads;
}pthread_s;

typedef struct _rwlock_t
{
    sem_t writelock;
    sem_t lock;
    int readers;
}rwlock_t;

pthread_s *threads_init(int num)
{
    pthread_s *temp_t = malloc(sizeof(pthread_s));
     if (temp_t == NULL) {
        perror("Failed to allocate memory for pthread_s");
        return NULL;
    }

    temp_t->threads = malloc(num * sizeof(pthread_t));
    if (temp_t->threads == NULL) {
        perror("Failed to allocate memory for thread array");
        free(temp_t);  // Free previously allocated memory
        return NULL;
    }
    temp_t->num_of_threads = num;
    return temp_t;
}

void rwlock_inti(rwlock_t *lock)
{
    lock->readers = 0;
    sem_wait(&lock->lock);
    lock->readers++;
    if (lock->readers == 1)
        sem_wait(&lock->writelock);
    sem_post(&lock->lock);
}

void rwlock_release_readlock(rwlock_t *lock)
{
    sem_wait(&lock->lock);
    lock->readers--;
    if(lock->readers == 0)
        sem_post(&lock->writelock);
    sem_post(&lock->lock);   
}

void rwlock_relase_writelokc(rwlock_t *lock)
{
    sem_post(&lock->writelock);
}

int searches; // adapt to searches
int write_loops;// adapt to insertions
int counter; //active threads??

rwlock_t mutex; //no deletions/insetions and searches at the same time
sem_t cond; //deletions come after insertions
//remember must initialize sem_t