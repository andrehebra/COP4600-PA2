#ifndef DBASE_H
#define DBASE_H

#include <string.h>
#include "chash.h"

hashRecord* createRecord(char* name, int salary);
void deleteNode(char* name, hashRecord** head);
int searchNode(char* name, hashRecord* head);
void traverseNode(hashRecord* head);


#endif
