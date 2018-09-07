#ifndef __sia_tokenizer_token_h__
#  define __sia_tokenizer_token_h__

typedef enum token_type {
  token_name, 
  token_lbrace, 
  token_rbrace, 
  token_comma
} token_type ;

typedef struct token {
  char* value ; 
  enum token_type type ; 
  struct token* next ;
} token ;



token* token_create(char* value, const token_type type) ;
token* token_push_back(token* tokens, token* new_token) ;
token* token_back(token* a_token) ;
token* token_insert(token* a_token, token* to_insert) ;

#endif