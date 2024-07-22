#include "chash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

extern rwlock_t hash_lock;

long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void log_operation(const char* operation, const char* name, uint32_t salary) {
    printf("%lld,%s,%s,%u\n", current_timestamp(), operation, name, salary);
}

void log_lock(const char* lock_type, const char* action) {
    printf("%lld,%s LOCK %s\n", current_timestamp(), lock_type, action);
}

hashRecord* createRecord(char* name, int salary) 
{
    hashRecord* newRecord = (hashRecord*)malloc(sizeof(hashRecord));
    strcpy(newRecord->name, name);
    newRecord->salary = salary;
    int len = strlen(name);
    newRecord->hash = jenkins_one_at_a_time_hash((const uint8_t *)name, len);
    newRecord->next = NULL;
    return newRecord;
}

void insertRecord(char* name, int salary, hashRecord** head)
{
    log_operation("INSERT", name, salary);
    log_lock("WRITE", "ACQUIRED");
    rwlock_acquire_writelock(&hash_lock);

    hashRecord* newRecord = createRecord(name, salary);
    if (*head == NULL) {
        *head = newRecord;
    } else {
        hashRecord* current = *head;
        hashRecord* prev = NULL;
        while (current != NULL && current->hash < newRecord->hash) {
            prev = current;
            current = current->next;
        }
        if (prev == NULL) {
            newRecord->next = *head;
            *head = newRecord;
        } else {
            newRecord->next = current;
            prev->next = newRecord;
        }
    }

    rwlock_release_writelock(&hash_lock);
    log_lock("WRITE", "RELEASED");
}

void deleteNode(char* name, hashRecord** head)
{
    log_operation("DELETE", name, 0);
    log_lock("WRITE", "ACQUIRED");
    rwlock_acquire_writelock(&hash_lock);

    hashRecord* temp = *head;
    hashRecord* prev = NULL;
    
    while (temp != NULL && strcmp(temp->name, name) != 0) 
    {
        prev = temp;
        temp = temp->next;
    }
    
    if (temp == NULL)
    {
        rwlock_release_writelock(&hash_lock);
        log_lock("WRITE", "RELEASED");
        return;
    }
    
    if (prev == NULL) {
        *head = temp->next;
    } else {
        prev->next = temp->next;
    }
    free(temp);

    rwlock_release_writelock(&hash_lock);
    log_lock("WRITE", "RELEASED");
}

int searchNode(char* name, hashRecord* head)
{
    log_operation("SEARCH", name, 0);
    log_lock("READ", "ACQUIRED");
    rwlock_acquire_readlock(&hash_lock);

    hashRecord* current = head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            printf("%u,%s,%u\n", current->hash, current->name, current->salary);
            rwlock_release_readlock(&hash_lock);
            log_lock("READ", "RELEASED");
            return 1;
        }
        current = current->next;
    }

    rwlock_release_readlock(&hash_lock);
    log_lock("READ", "RELEASED");
    return 0;
}

void traverseNode(hashRecord* head) {
    log_operation("PRINT", "", 0);
    log_lock("READ", "ACQUIRED");
    rwlock_acquire_readlock(&hash_lock);

    hashRecord* temp = head;
    while (temp != NULL) {
        printf("%u,%s,%u\n", temp->hash, temp->name, temp->salary);
        temp = temp->next;
    }
    printf("\n");

    rwlock_release_readlock(&hash_lock);
    log_lock("READ", "RELEASED");
}