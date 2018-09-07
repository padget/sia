#ifndef __sia_tokenizer_tokenize_h__
#  define __sia_tokenizer_tokenize_h__

#include <stdio.h>
#include <stdbool.h>


#include "tokenizer/token.h"

token* tokenize (FILE* file_to_tokenize) ;

bool is_name_token (char* buffer) ;
bool is_lbrace_token (char* buffer) ;
bool is_rbrace_token (char* buffer) ;
bool is_comma_token (char* buffer) ;


#endif 