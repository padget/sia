enum token_type {
  id, lbrace, rbrace, comma
} ;

typedef struct token {
  char* value ; 
  enum token_type type ;  
} token ;