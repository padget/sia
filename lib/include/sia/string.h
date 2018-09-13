#ifndef __sia_string_h__
#  define __sia_string_h__

#  include <stdlib.h>
#  include <stdbool.h>

//////////////////
/// String lib ///
//////////////////

typedef struct string { 
  char* data ;  
  size_t size ;
} string ; 

typedef char* string_iterator ;
typedef char* const string_citerator ;
typedef const char* cstring_iterator ;
typedef const char* const cstring_citerator ;

cstring_iterator begin(string str);
cstring_iterator end(string str) ;
size_t string_it_length(cstring_iterator begin, cstring_iterator end) ;
size_t string_length(const string str) ;
string string_it_create(cstring_iterator begin, cstring_iterator end) ;
string string_create(const string str) ;
string string_cs_create(const char* cstr) ;
string string_it_copy(cstring_iterator begin, cstring_iterator end) ;
string string_copy(const string str) ;
int string_it_compare(cstring_iterator begin, cstring_iterator end, 
                      cstring_iterator obegin, cstring_iterator oend) ;
int string_compare(string str1, string str2) ;
string string_it_concat(cstring_iterator begin, cstring_iterator end, 
                        cstring_iterator obegin, cstring_iterator oend) ;
string string_concat(string str1, string str2) ;
void string_free(string* str) ;
bool string_is_empty(string str) ;

#endif