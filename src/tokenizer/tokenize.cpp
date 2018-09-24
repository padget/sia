#include <sqlite3.h>
#include <string>
#include <string_view>
#include <fstream>
#include <tuple>
#include <iterator>
#include <iostream>
#include <variant>
#include <type_traits>
#include <utility>

const auto ERROR_ON_FILE_OPENING   = std::string("can't open the file.") ;
const auto CANT_READ_FILE          = std::string("can't read file.") ;
const auto CANT_OPEN_DATABASE      = std::string("can't open the database.") ;
const auto CANT_CREATE_TOKEN_TABLE = std::string("can't create tokens table.") ;
const auto CANT_EXECUTE_QUERY      = std::string("can't execute query.") ;

/*
auto quoted(const auto& str) {
  return std::string("\"") + str + std::string("\"") ;
}
*/


auto between (auto && min, auto && max) {
  return [=] (auto && value) {
    return min <= value && value <= max ;
  } ;
}

auto oneof (auto && ... cst) {
  return [=] (auto && value) {
    return (... || (value == cst)) ;
  } ;
}

auto equal (auto && cst) {
  return [=] (auto && value) {
    return cst == value ;
  } ;
}

auto when (auto && cond, auto && callback) {
  return [=] (auto && value) {
    switch (cond(std::forward<decltype(value)>(value))) {
      case true : 
        callback(std::forward<decltype(value)>(value)) ;
        return true ;
      default : 
        return false ; 
    }
  } ;
}

auto otherwise (auto && callback) {
  return [=] (auto && value) {
    callback(static_cast<decltype(value)&&>(value)) ;
    return true ;
  } ;
}

auto match (auto && r) {
  return [&] (auto && ... whens) {
    return (... || whens(r)) ;
  } ;
}

auto print(auto const& message) {
  return [=] (auto &&) {std::cout << message << '\n' ;} ; 
}

auto oops() {
  return print("oops !") ;
}

template <typename type_t, typename error_t> 
using result = std::variant<type_t, error_t> ;

auto is_ok () {
  return [] (auto && res) {
    return res.index() == 0 ; 
  } ;
}

auto is_error () {
  return [] (auto && res) {
    return res.index() == 1 ;
  } ;
}

template <typename type_t>
auto unwrap (auto && callback) {
  return [=] (auto && res) {
    callback(std::get<type_t>(std::forward<decltype(res)>(res))) ; 
  } ;
}

template <size_t index, typename variant_t>
using from_t = std::variant_alternative_t<index, variant_t> ;

auto match_result (auto && r) {
  using r_t  = std::decay_t<decltype(r)> ; 
  using ok_t = from_t<0, r_t> ;
  using ko_t = from_t<1, r_t> ;

  return [&r] (auto && ok_cllbk, auto ko_cllbk) {
    return match(r) (
      when(is_ok(), unwrap<ok_t>(ok_cllbk)), 
      when(is_error(), unwrap<ko_t>(ko_cllbk))) ;
  } ;
}

auto assign (auto & lvalue, auto transformer) {
  return [&lvalue, transformer] (auto && value) {
    lvalue = transformer(std::forward<decltype(value)>(value)) ;
  } ;
}

auto open_file (const std::string& filename) {
  std::ifstream file = std::ifstream(filename, std::ios::in) ;
  result<std::ifstream, std::string> r ;
  
  switch(file.is_open()) {
    case true : r = std::move(file) ; break ;
    default   : r = ERROR_ON_FILE_OPENING ; break ;
  }

  return r ; 
}

struct file_content { 
  std::string content ; 
} ;

using error_message_t = std::string ;
using file_iterator_t = std::istreambuf_iterator<char> ;

auto extract_file_content () {
  return [] (auto && ifs) {
    return file_content {std::string(file_iterator_t(ifs), file_iterator_t())} ;
  } ;
}

auto right_concat(auto && value) {
  return [=] (auto && message) {
    return value + message ;
  } ;
}

auto read_file(const std::string& filename) {
  auto opened = open_file(filename) ;
  result<file_content, error_message_t> r ;

  match(open_file(filename)) (
    when(is_ok(), unwrap<std::ifstream>(assign(r, extract_file_content()))),
    otherwise(unwrap<error_message_t>(assign(r, right_concat(CANT_READ_FILE))))) ;

  return r ;
}

/*
using db_t = sqlite3*;

result<db_t, error_message_t>
open_database(std::string_view db_name) {
  sqlite3* db ;
  
  switch(sqlite3_open(db_name.data(), &db)) {
    case SQLITE_OK : return {db} ;
    default        : return {CANT_OPEN_DATABASE} ;
  }
}

auto sql_exec(db_t db, std::string_view query) {
  char* error_message_buffer = NULL;
  result<int, error_message_t> r ;

  switch(sqlite3_exec(db, query.data(), NULL, NULL, &error_message_buffer)) {
    case SQLITE_OK: r = SQLITE_OK ; break ;
    default       : r = CANT_EXECUTE_QUERY + " " + quoted(query.data()) ; break ;
  } ;

  sqlite3_free(error_message_buffer) ;
  return r ;
}

auto create_tokens_table(db_t db) {
  constexpr const char* query =  
    "create table if not exists t_token (            "
    "  id        integer  primary key,               "
    "  filename  text     not null   ,               "
    "  line      integer  not null   ,               "
    "  column    integer  not null   ,               "
    "  value     text     not null   ,               "
    "  type      integer  not null   ,               "
    "  previous  integer             ,               "
    "  next      integer             ,               "
    "                                                "
    "  foreign key(previous) references t_token(id), "
    "  foreign key(next) references t_token(id))     " ;
  
  return sql_exec(db, query) ;
}

struct token {
  enum class type : size_t {
    name, 
    lbrace, 
    rbrace, 
    comma
  } ;
  
  std::string filename ;
  int         line     ; 
  int         column   ; 
  std::string value    ; 
  type        tp       ;
} ; 


#include <sstream>

auto insert_one_token(db_t db, const token & tk) {
  auto query_builder = std::stringstream() ;
  query_builder << "insert into t_token (filename," 
                << "line, column, value, type) values (" 
                << std::move(quoted(tk.filename)) << ", "
                << tk.line << ", "
                << tk.column << ", "
                << std::move(quoted(tk.value)) << ", "
                << static_cast<size_t>(tk.tp) << ")" ;
  return sql_exec(db, query_builder.str().c_str()) ;
}

*/

/// MAIN SCRIPT

int main(int argc, const char** argv) {
 /* auto r = read_file("lol.sia") ;

  match(r) 
    ([] (file_content& file) {std::cout << "OK " << file.content << std::endl ; },
     [] (const error_message_t& message) {std::cout << "KO " << message << std::endl ; }) ;

  match(open_database("lol.db")) ( // TODO close db !!!!
    [] (db_t& db) {
      match(create_tokens_table(db) ) (
        [&db] (auto) {
          std::cout << "token table created" << std::endl ;
          token tk = {.filename = "lol.sia", 
                     .line = 0, 
                     .column = 0, 
                     .value="yo"} ;

          match(insert_one_token(db, tk)) (
            [] (int res) {std::cout << res << std::endl ; }, 
            [] (const error_message_t& message) {std::cout << "KO " << message << std::endl ;}) ;  
        },
        [] (const error_message_t& message) {std::cout << "KO " << message << std::endl ;});
    }, 
    [] (const error_message_t& message) {std::cout << "KO " << message << std::endl ; }) ;
*/
  
  match ('b') (
    when(equal('b'), print("i'm equal to b")),
    when(between('a', 'z'), print("i'm between a and z")), 
    when(between('A', 'Z'), print("i'm between A and Z")), 
    otherwise(oops())) ;

  match (read_file("lol.sia")) (
    when(is_ok(), unwrap<file_content>([] (auto && fcontent) { std::cout << fcontent.content << std::endl ;})), 
    otherwise(oops())) ;
  
  return EXIT_SUCCESS ;
}


/// MAIN SCRIPT