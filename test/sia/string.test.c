#include <stdbool.h>
#include <stdio.h>

#include <sia/string.h>
#include <string.h>

#undef TEST 

#define TEST void

#undef assert
#define assert(condition)                             \
  if (!(condition)) {                                 \
    printf("/!\\ test failed : %s\n", #condition) ;   \
  }                                                   \



TEST string_cs_create_test() {
  string str = string_cs_create("test") ;
  assert(strcmp(str.data, "test") == 0) ;
  string_free(&str) ;
}

TEST string_create_test() {
  string src = string_cs_create("test") ;
  string str = string_create(src) ;
  assert(strcmp(str.data, "test") == 0) ;
  string_free_all(2, &src, &str) ;
}

TEST string_length_test() {
  string str = (string) {.size = 12, .data = "coucou"} ;
  assert(string_length(str) == 12) ;
}

TEST string_it_length_test() {
  string str = string_cs_create("coucou") ;
  assert(string_it_length(string_begin(str), string_end(str)) == 6) ;
  string_free(&str) ;
}

TEST string_concat_test() {
  string str = string_cs_create("hello ") ;
  string str2 = string_cs_create("world !") ;
  string concat = string_concat(str, str2) ;
  assert(strcmp(string_concat(str, str2).data, "hello world !") == 0) ;
  string_free_all(2, &str, &str2) ;
}

TEST string_copy_test() {
  string str = string_cs_create("test") ;
  string str2 = string_copy(str) ;
  assert(strcmp(str2.data, "test") == 0) ;
  string_free_all(2, &str, &str2) ; 
}

TEST string_compare_test() {
  string str = string_cs_create("test") ;
  string str2 = string_copy(str) ;
  assert(string_compare(str, str2) == 0) ;
  string_free_all(2, &str, &str2) ;
}

TEST string_compare_test2() {
  string str = string_cs_create("test") ;
  string str2 = string_cs_create("foo") ;
  assert(string_compare(str, str2) != 0) ;
  string_free_all(2, &str, &str2) ;
}

int main() {
  printf("Begin string test\n") ;
  string_cs_create_test() ;
  string_create_test() ;
  string_length_test() ;
  string_it_length_test() ;
  string_concat_test() ;
  string_copy_test() ;
  string_compare_test() ;

  printf("End string test\n") ;

  return EXIT_SUCCESS ;
}

#undef test
