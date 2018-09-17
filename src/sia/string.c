#include <sia/string.h>
#include <sia/memory.h>

#include <stdlib.h>
#include <string.h>

cstring_iterator string_begin(string str) {
  return str.data ;
}

cstring_iterator string_end(string str) {
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
  char* str = sia_malloc(size + 1, char) ;
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
  return string_it_create(string_begin(str), string_end(str)) ;
}

string string_cs_create(const char* cstr) {
  return string_it_create(cstr, cstr + strlen(cstr)) ;
}

string string_it_copy(cstring_iterator begin, cstring_iterator end) {
  return string_it_create(begin, end) ;
}

string string_copy(const string str) {
  return string_it_copy(string_begin(str), string_end(str)) ;
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
  return string_it_compare(string_begin(str1), string_end(str1), string_begin(str2), string_end(str2)) ;
}

string string_it_concat(cstring_iterator begin, cstring_iterator end, 
                      cstring_iterator obegin, cstring_iterator oend) {
  size_t data_size = string_it_length(begin, end) + 
                     string_it_length(obegin, oend) ;
  char* data = sia_malloc(data_size + 1, char) ;
  char* first_data = data ;
  
  while (begin != end) {
    *data = *begin ;
    ++data ; ++begin ;
  }

  while (obegin != oend) {
    *data = *obegin ;
    ++data ; ++obegin ;
  }

  *data = '\0' ;
  return (string) {.data = first_data, .size = data_size} ;
}

string string_concat(string str1, string str2) {
  return string_it_concat(string_begin(str1), string_end(str1), string_begin(str2), string_end(str2)) ;
}

void string_free(string* str) {
  free(str->data) ;
  str->data = NULL ;  
  str->size = 0 ;
}

#include <stdarg.h>

void string_free_all(size_t num, ...) {
   va_list valist ;
   va_start(valist, num) ;

   for (int i = 0; i < num; i++) {
      string_free(va_arg(valist, string*)) ;
   }

   va_end(valist) ;
}

bool string_is_empty(string str) {
  return str.size == 0 ;
}
