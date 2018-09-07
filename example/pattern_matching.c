#include <stdio.h>
#include <stdlib.h>

typedef struct result {
  union {
    struct {} success ;
    struct { char* message ; } error ;
  } ;

  enum { error, success } tag ;
  
} result ;

result return_success () {
  result res = { .success = {}, .tag = success } ;
  return res ;
}

result return_error() {
  result res = { .error = {"oops an error occured"}, .tag = error } ;
  return res ; 
}

void interpret_result(result res) {
  switch (res.tag) {
    case success : printf("success\n") ; break ;
    case error : printf("error : %s\n", res.error.message) ; break ; 
  }
}

int main (int argc, char** argv) {
  interpret_result(return_success());
  interpret_result(return_error());
}