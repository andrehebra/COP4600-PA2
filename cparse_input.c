#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "cthread.h"
#include "dbase.h"

#define MAX_NAME_LENGTH 51

//Need to add time stamp to print stmts

long long timestampTest()
{

  struct timeval te;
  gettimeofday(&te, NULL); // Get Current Time
  long long microseconds = (te.tv_sec * 1000000) + te.tv_usec; // Calculate Milliseconds
  return microseconds;
  
}

int parse_and_compute_file(char *filename) 
{
    char line[256], name[MAX_NAME_LENGTH];
    int salary, threads, some_val;
    hashRecord *Hash_Map = NULL;

    //pthread_s *thread_arr = NULL;

    FILE *file = fopen(filename, "r");


    // Open the file for reading
    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }

    // Read the file line by line
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "threads", 7) ==0)
        {
            sscanf(line, "threads, %d, %d",&threads, &some_val);
            printf("\nnumber of threads: %d\n",threads);
            //thread_arr = threads_init(threads); 
        }
        // Check if the line starts with "insert"
        else if (strncmp(line, "insert", 6) == 0) {
            sscanf(line, "insert, %[^,], %d",name, &salary);
            printf("\nname: %s, salary: %d\n",name, salary);
            
            //create insertion thread
            //global counter for insertion threads?/CV
            //figure out where to insert it at
            Hash_Map = createRecord(name,salary);

            //print success or fail

        }
        else if (strncmp(line, "search", 6) == 0)
        {
            sscanf(line, "search, %[^,],%d",name, &some_val);
            printf("\nsearching for \"%s\", %d\n",name, some_val);
            //create search thread
            /*if (searchNode(name, Hash_Map) == 0)
            {
                printf("\n\"%s\" not found\n", name);
            }*/
        }
        else if (strncmp(line, "delete", 6) == 0)
        {
            sscanf(line, "delete, %[^,],%d", name, &some_val);
            printf("\ndeleting \"%s\" %d\n",name,some_val);
            //create deletion thread but must wait until all insertions are done
            //deleteNode(name, Hash_Map);

            //print success or fail
        }
    }

    // Close the file
    fclose(file);
    return 1;

    // Print the count of insert operations

}