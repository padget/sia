#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tokenizer/tokenize.h"



token* tokenize(FILE* file_to_tokenize) {
  int c ;
  
  if (file_to_tokenize != NULL) {
    while ((c = fgetc(file_to_tokenize)) != EOF) {
      printf("%c", c) ;
    }
  }

  return NULL;
}

bool is_between(char c, char b, char e) {
  return b <= c && c <= e ;
}

bool is_lower(char c) {
  return is_between(c, 'a', 'z') ;
}

bool is_upper(char c) {
  return is_between(c, 'A', 'Z') ;
}

bool is_letter(char c) {
  return is_lower(c) || is_upper(c) ;
}

bool is_underscore(char c) {
  return c == '_' ;
}

bool is_letter_or_underscore(char c) {
  return is_letter(c) || is_underscore(c) ;
}

bool is_less(int i, int max) {
  return i < max ;
}

bool is_name_token (char* buffer) {
  size_t buffer_length = strlen(buffer) ;
  size_t index = 0 ;

  while (is_less(index, buffer_length) && 
         is_letter_or_underscore(buffer[index])) 
    index++ ;

  return index == buffer_length ;
}


bool is_lbrace_token (char* buffer) {
  return strcmp(buffer, "(") == 0 ;
}


bool is_rbrace_token (char* buffer) {
  return strcmp(buffer, ")") == 0 ;
}


bool is_comma_token (char* buffer) {
  return strcmp(buffer, ",") == 0 ;
}


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
  FILE* file_to_tokenize = fopen(filename, "r") ;
  tokenize(file_to_tokenize) ;

  return EXIT_SUCCESS ;
}
