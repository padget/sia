#include <sia/wstring.h>

#include <stdlib.h>
#include <string.h>

cwstring_iterator wstring_begin(wstring str) {
  return str.data ;
}

cwstring_iterator wstring_end(wstring str) {
  return str.data + str.size ;
}

size_t wstring_it_length(cwstring_iterator begin, cwstring_iterator end) {
  return end - begin ;
}

size_t wstring_length(const wstring str) {
  return str.size ;
}

wstring wstring_it_create(cwstring_iterator begin, cwstring_iterator end) {
  const size_t size = wstring_it_length(begin, end) ;
  wchar_t* str = malloc(sizeof(wchar_t) * (size + 1)) ;
  wchar_t* str_begin = str ;

  while (begin != end) {
    *str = *begin ;
    ++begin ;
    ++str ;
  }

  *str = '\0' ;

  return (wstring) {.data = str_begin, .size = size} ;
}

wstring wstring_create(const wstring str) {
  return wstring_it_create(wstring_begin(str), wstring_end(str)) ;
}

wstring wstring_cs_create(const wchar_t* cstr) {
  return wstring_it_create(cstr, cstr + wcslen(cstr)) ;
}

wstring wstring_it_copy(cwstring_iterator begin, cwstring_iterator end) {
  return wstring_it_create(begin, end) ;
}

wstring wstring_copy(const wstring str) {
  return wstring_it_copy(wstring_begin(str), wstring_end(str)) ;
}

int wstring_it_compare(cwstring_iterator begin, cwstring_iterator end, 
                      cwstring_iterator obegin, cwstring_iterator oend) {
  const size_t size = wstring_it_length(begin, end) ;
  const size_t osize = wstring_it_length(obegin, oend) ;

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

int wstring_compare(wstring str1, wstring str2) {
  return wstring_it_compare(wstring_begin(str1), wstring_end(str1), wstring_begin(str2), wstring_end(str2)) ;
}

wstring wstring_it_concat(cwstring_iterator begin, cwstring_iterator end, 
                      cwstring_iterator obegin, cwstring_iterator oend) {
  //char* data = malloc(wstring_it_length(begin, end) + wstring_it_length(obegin, oend) - +) ;
}

wstring wstring_concat(wstring str1, wstring str2) {
  return wstring_it_concat(wstring_begin(str1), wstring_end(str1), wstring_begin(str2), wstring_end(str2)) ;
}

void wstring_free(wstring* str) {
  free(str->data) ;
  str->data = NULL ;  
}

bool wstring_is_empty(wstring str) {
  return str.size == 0 ;
}