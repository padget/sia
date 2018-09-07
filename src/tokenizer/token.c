#include "tokenizer/token.h"
#include <stdlib.h>


token* token_create(char* value, const token_type type) {
  token* tok = malloc(sizeof(token)) ;
  tok->type = type ;
  tok->value = value ;

  return tok ; 
}


token* token_push_back(token* tokens, token* new_token) {
  token* back = token_back(tokens) ;
  back->next = new_token ;

  return new_token ;
}


token* token_insert(token* a_token, token* to_insert) {
  if (a_token != NULL && to_insert != NULL) {
    to_insert->next = a_token->next ;
    a_token->next = to_insert ;
    return to_insert ;
  }

  return NULL;
}


token* token_back(token* a_token) {
  token* back ;
  
  while (a_token->next != NULL) {
    back = a_token->next ;
  }

  return back ;
}