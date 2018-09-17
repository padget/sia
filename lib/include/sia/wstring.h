#ifndef __sia_wstring_h__
#  define __sia_wstring_h__

#  include <stdlib.h>
#  include <stdbool.h>
#  include <wchar.h>

//////////////////
/// String lib ///
//////////////////

typedef struct wstring { 
  wchar_t* data ;  
  size_t size ;
} wstring ; 

typedef wchar_t* wstring_iterator ;
typedef wchar_t* const wstring_citerator ;
typedef const wchar_t* cwstring_iterator ;
typedef const wchar_t* const cwstring_citerator ;

cwstring_iterator wstring_begin(wstring str);
cwstring_iterator wstring_end(wstring str) ;
size_t wstring_it_length(cwstring_iterator begin, cwstring_iterator end) ;
size_t wstring_length(const wstring str) ;
wstring wstring_it_create(cwstring_iterator begin, cwstring_iterator end) ;
wstring wstring_create(const wstring str) ;
wstring wstring_cs_create(const wchar_t* cstr) ;
wstring wstring_it_copy(cwstring_iterator begin, cwstring_iterator end) ;
wstring wstring_copy(const wstring str) ;
int wstring_it_compare(cwstring_iterator begin, cwstring_iterator end, 
                      cwstring_iterator obegin, cwstring_iterator oend) ;
int wstring_compare(wstring str1, wstring str2) ;
wstring wstring_it_concat(cwstring_iterator begin, cwstring_iterator end, 
                        cwstring_iterator obegin, cwstring_iterator oend) ;
wstring wstring_concat(wstring str1, wstring str2) ;
void wstring_free(wstring* str) ;
bool wstring_is_empty(wstring str) ;

#endif