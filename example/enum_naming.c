enum color {
  red, green, blue
} ;

enum primary {
  red, yellow, bad 
} ;

// example/enum_naming.c:6:3: error: redeclaration of enumerator 'red'
//    red, yellow, bad
//    ^~~
// example/enum_naming.c:2:3: note: previous definition of 'red' was here
//    red, green, blue
//    ^~~