#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tokenizer/tokenize.h"



int main(int argc, const char** argv) {

  if (argc != 2) {
    perror("arguments list invalid (tokenize <filename>.sia)\n" ) ;

    return EXIT_FAILURE ;
  }

  const char* expected_extension = "sia" ;
  const char* filename = argv[1] ;
  const char* extension = strrchr(argv[1], '.') ;
  
  if (extension == NULL 
      || strlen(extension) != strlen(expected_extension) + 1
      || !strcmp(extension, expected_extension)) {
    perror("the argument filename is not a sia file (*.sia)\n") ;

    return EXIT_FAILURE ;
  }

  printf("extract tokens from %s", argv[1]) ;


  return EXIT_SUCCESS ;
}
