#include <sia/string.h>

#include <stdlib.h>
#include <string.h>

cstring_iterator begin(string str) {
  return str.data ;
}

cstring_iterator end(string str) {
  return str.data + str.size ;
}

size_t string_it_length(cstring_iterator begin, cstring_iterator end) {
  return end - begin ;
}

size_t string_length(const string str) {
  return str.size ;
}

string string_it_create(cstring_iterator begin, cstring_iterator end) {
  const size_t size = string_it_length(begin, end) ;
  char* str = malloc(size + 1) ;
  char* str_begin = str ;

  while (begin != end) {
    *str = *begin ;
    ++begin ;
    ++str ;
  }

  *str = '\0' ;

  return (string) {.data = str_begin, .size = size} ;
}

string string_create(const string str) {
  return string_it_create(begin(str), end(str)) ;
}

string string_cs_create(const char* cstr) {
  return string_it_create(cstr, cstr + strlen(cstr)) ;
}

string string_it_copy(cstring_iterator begin, cstring_iterator end) {
  return string_it_create(begin, end) ;
}

string string_copy(const string str) {
  return string_it_copy(begin(str), end(str)) ;
}

int string_it_compare(cstring_iterator begin, cstring_iterator end, 
                      cstring_iterator obegin, cstring_iterator oend) {
  const size_t size = string_it_length(begin, end) ;
  const size_t osize = string_it_length(obegin, oend) ;

  if (size != osize) {
    return size < osize ? -1 : 1 ;
  }

  while (begin != end && obegin != oend) {
    if (*begin == *obegin) {
      ++begin ; 
      ++obegin ;
    } else if (*begin > *obegin) {
      return 1 ;
    } else if (*begin < *obegin) {
      return -1 ; 
    } 
  }

  return 0 ;
}

int string_compare(string str1, string str2) {
  return string_it_compare(begin(str1), end(str1), begin(str2), end(str2)) ;
}

string string_it_concat(cstring_iterator begin, cstring_iterator end, 
                      cstring_iterator obegin, cstring_iterator oend) {
  //char* data = malloc(string_it_length(begin, end) + string_it_length(obegin, oend) - +) ;
}

string string_concat(string str1, string str2) {
  return string_it_concat(begin(str1), end(str1), begin(str2), end(str2)) ;
}

void string_free(string* str) {
  free(str->data) ;
  str->data = NULL ;  
}

bool string_is_empty(string str) {
  return str.size == 0 ;
}