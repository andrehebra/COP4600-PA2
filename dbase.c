#include "chash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Creating New Hash Record For Employee
hashRecord* createRecord(char* name, int salary) 
{
  printf("\n!!REACHED createRecord!!\n");
  hashRecord* newRecord = (hashRecord*)malloc(sizeof(hashRecord));
  strcpy(newRecord->name, name);
  newRecord->salary = salary;
  int len = strlen(name);
  newRecord->hash = jenkins_one_at_a_time_hash((const uint8_t *)name, len);
  newRecord->next = NULL;
  return newRecord;
}

//Decide Which One Should Be Implemented Beginning or Ending
/*

//Inserting New Employee Node To Beginning Of List
void insertBeginNode(char* name, int salary,struct hashRecord** head)
{
  struct hashRecord* newRecord = createRecord(name, salary);
  newRecord->next = *head;
  *head = newRecord;
}

//Inserting New Employee Node To The End Of List
void insertEndNode(char* name, int salary, struct hashRecord** head) {
    struct hashRecord* newRecord = createNode(name, salary);
    if (*head == NULL) {
        *head = newRecord;
        return;
    }
    struct hashRecord* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newRecord;
}

*/

//Removing Employee Entry Given A Hash Value
void deleteNode(char* name, hashRecord** head)
{
  printf("\n!!REACHED deleteNode!!\n");
  hashRecord* temp = *head;
  hashRecord* prev = NULL;
  
  //For The Case If The Head Holds The Hash
  if (temp != NULL && (strcmp(temp->name, name) == 0)) 
  {
    *head = temp->next;
    free(temp);
    return;
  }
  
  //Searching For The Darn Key
  while (temp != NULL && (strcmp(temp->name, name) != 0)) 
  {
    prev = temp;
    temp = temp->next;
  }
  
  //If The Key Doesn't Even Exist
  if (temp == NULL)
  {
   return;
  }
  
    //Remove The Employee From The List
    prev->next = temp->next;
    free(temp);

}

//Can People Have The Same Name?
int searchNode(char* name, hashRecord* head)
{
  printf("\n!!REACHED searchNode!!\n");
  hashRecord* current = head;
  while (current != NULL) {
        if (strcmp(current->name,name) == 0) {
            printf("%u, %s, %u",current->hash, current->name, current->salary);
            return 1;
        }
        current = current->next;
    }
    return 0;

}

//Transversing The Employe Node
void traverseNode(hashRecord* head) {
    hashRecord* temp = head;
    while (temp != NULL) {
        printf("%u, %s, %u", temp->hash, temp->name, temp->salary);
        temp = temp->next;
    }
    printf("\n");
}