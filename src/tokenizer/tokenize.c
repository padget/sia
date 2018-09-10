#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sqlite3.h>


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

typedef struct compilation_context {
  char* filename ;
  int line ; 
  int column ; 
} compilation_context ;


token* token_create(char* value, const token_type type) {
  token* tok = malloc(sizeof(token)) ;
  tok->type = type ;
  tok->value = value ;

  return tok ; 
}


token* token_back(token* a_token) {
  token* back ;
  
  while (a_token->next != NULL) {
    back = a_token->next ;
  }

  return back ;
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


token* tokenize(FILE* file_to_tokenize) {
  int c ;
  
  if (file_to_tokenize != NULL) {
    while ((c = fgetc(file_to_tokenize)) != EOF) {
      printf("%c", c) ;
    }
  }

  return NULL;
}


bool is_between(char c, char b, char e) {
  return b <= c && c <= e ;
}


bool is_lower(char c) {
  return is_between(c, 'a', 'z') ;
}


bool is_upper(char c) {
  return is_between(c, 'A', 'Z') ;
}


bool is_letter(char c) {
  return is_lower(c) || is_upper(c) ;
}


bool is_underscore(char c) {
  return c == '_' ;
}


bool is_letter_or_underscore(char c) {
  return is_letter(c) || is_underscore(c) ;
}

bool is_less(int i, int max) {
  return i < max ;
}


bool is_name_token(char* buffer) {
  size_t buffer_length = strlen(buffer) ;
  size_t index = 0 ;

  while (is_less(index, buffer_length) && 
         is_letter_or_underscore(buffer[index])) 
    index++ ;

  return index == buffer_length ;
}


bool is_lbrace_token(char* buffer) {
  return strcmp(buffer, "(") == 0 ;
}


bool is_rbrace_token(char* buffer) {
  return strcmp(buffer, ")") == 0 ;
}


bool is_comma_token(char* buffer) {
  return strcmp(buffer, ",") == 0 ;
}


int sia_sqlite3_exec(sqlite3* db, const char* sql) {
  printf("[sqlite 3] Execution de la requete : \n%s\n", sql) ;
  char* error_message_buffer ;
  int rc ;

  switch (rc = sqlite3_exec(db, sql, NULL, NULL, &error_message_buffer)) {
    case SQLITE_OK: 
      return rc ;
    default: 
      printf("Une erreur est survenue : %s\n", error_message_buffer) ;
      sqlite3_free(error_message_buffer) ;
      return rc ;
  }
}


typedef struct token_db {
  char*       filename ;
  int         line     ; 
  int         column   ; 
  char*       value    ; 
  token_type  type     ;
  int         previous ;  
  int         next     ;  
} token_db ; 


token_db* token_to_db(const token* a_token, const compilation_context* cmp_context) {
  token_db* tkdb_ptr = malloc(sizeof(token_db)) ;

  *tkdb_ptr = (token_db) {
    .filename = cmp_context->filename,
    .line = cmp_context->line,
    .column = cmp_context->column,
    .value = a_token->value,
    .type = a_token->type,
    .previous = 0, // TODO replace 0 by good value
    .next = 0 // TODO replace 0 by good value
  } ;

  return tkdb_ptr ;
}


const char* create_table_tokens_query() {
  return 
  "create table if not exists t_token (           \n\
    id        integer  primary key,               \n\
    filename  text     not null   ,               \n\
    line      integer  not null   ,               \n\
    column    integer  not null   ,               \n\
    value     text     not null   ,               \n\
    type      integer  not null   ,               \n\
    previous  integer             ,               \n\
    next      integer             ,               \n\
                                                  \n\
    foreign key(previous) references t_token(id), \n\
    foreign key(next) references t_token(id)      \n\
  )" ;
}


const char* sia_db_name() {
  return "sia.db" ;
}


int create_table_token(sqlite3* db) {
  return sia_sqlite3_exec(db, create_table_tokens_query()) ;
}

char* insert_one_token_query(const token_db* a_token) {
  const char* fmt = 
  "insert into t_token\
    (filename, line, column, value, type, previous, next)\n \
    values (\"%s\", %d, %d, \"%s\", %d, %d, %d)" ;
  const int fmt_size      = strlen(fmt), 
            filename_size = strlen(a_token->filename),
            value_size    = strlen(a_token->value) ;
  char* query = malloc(fmt_size + filename_size + value_size + 50) ;

  sprintf(query, fmt, 
    a_token->filename, a_token->line, 
    a_token->column, a_token->value, 
    a_token->type, a_token->previous, 
    a_token->next) ;

  return query ;
}


int insert_one_token(sqlite3* db,  token* a_token, const compilation_context* cmp_context) {
  token_db* a_token_db = token_to_db(a_token, cmp_context) ;
  char* query = insert_one_token_query(a_token_db) ;

  sia_sqlite3_exec(db, query) ;

  free(query) ;
  free(a_token_db) ;
}




/// MAIN SCRIPT

int main(int argc, const char** argv) {
  if (argc != 2) {
    perror("arguments list invalid (tokenize <filename>.sia)\n" ) ;
    return EXIT_FAILURE ;
  }

  sqlite3* db ; 

  switch(sqlite3_open(sia_db_name(), &db)) {
    case SQLITE_OK: printf("Base sia.db ouverte avec succes\n") ; break ;
    default: return EXIT_FAILURE ;
  }

  switch (create_table_token(db)) {
    case SQLITE_OK: printf("Table t_token créée avec succes\n") ; break ;
    default: return EXIT_FAILURE ;
  }

  token t = (token) {
    .value = "name",
    .type = token_name 
  } ;

  compilation_context ctx = (compilation_context ) {
    .filename = "lol.sia",
    .line = 1, 
    .column = 0
  } ;

  insert_one_token(db, &t, &ctx) ;

  const char* expected_extension = "sia" ;
  const char* filename = argv[1] ;
  const char* extension = strrchr(argv[1], '.') ;
  
  if (extension == NULL 
      || strlen(extension) != strlen(expected_extension) + 1
      || !strcmp(extension, expected_extension)) {
    perror("the argument filename is not a sia file (*.sia)\n") ;

    return EXIT_FAILURE ;
  }

  printf("extract tokens from %s", argv[1]) ;
  FILE* file_to_tokenize = fopen(filename, "r") ;
  tokenize(file_to_tokenize) ;

  sqlite3_close(db) ;

  return EXIT_SUCCESS ;
}


/// MAIN SCRIPT