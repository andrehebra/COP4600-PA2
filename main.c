#include <stdio.h>
#include "cparse_input.h"

//TODO:
//Conditional Variables
//Locking Mechanism
//Refining-Debugging Sample Output


int main()
{
  
  if (parse_and_compute_file("command.txt") == 0)
  {
    fprintf(stderr,"Error returning from parse_file"); 
  }

  return 0;
  
}

