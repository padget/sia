
#include <string>
#include <tuple>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <variant>
#include <type_traits>
#include <utility>
#include <sstream>


namespace sia::error {
  const auto CANT_ON_FILE_OPENING    = std::string("can't open the file.") ;
  const auto CANT_READ_FILE          = std::string("can't read file.") ;
  const auto CANT_OPEN_DATABASE      = std::string("can't open the database.") ;
  const auto CANT_CREATE_TOKEN_TABLE = std::string("can't create tokens table.") ;
  const auto CANT_EXECUTE_QUERY      = std::string("can't execute query.") ;
  const auto CANT_INSERT_TOKEN       = std::string("can't insert token.") ;
}

/// //////////////// ///
/// OPTIONAL PATTERN ///
/// //////////////// ///

#include <optional>

namespace sia {

  template <typename option_t>
  auto option (auto && value) {
    return std::optional<option_t>(
      static_cast<decltype(value)&&>(value)) ;
  }

  template <typename option_t>
  auto option () {
    return std::optional<option_t>() ;
  }

  auto has_value (auto && opt) {
    return opt.has_value() ;
  }

  auto on_value(auto && opt, auto && func) {
    if (has_value(static_cast<decltype(opt)&&>(opt))) 
      return static_cast<decltype(func)&&>(func) (
        static_cast<decltype(opt.value())&&>(opt.value())) ;
  }

  auto on_no_value(auto && opt, auto && func) {
    if (!has_value(static_cast<decltype(opt)&&>(opt))) 
      return static_cast<decltype(func)&&>(func) () ;
  }
}

/// //////////////// ///
/// EXPECTED PATTERN ///
/// //////////////// ///

#include <variant> 

namespace sia {

  template <typename ... type_t>
  using variant = std::variant<type_t...> ;

  template <typename type_t, typename error_t>
  using expected = variant<type_t, error_t> ;

  auto is_ok (auto const & expect) {
    return expect.index() == 0 ; 
  }

  auto is_ko (auto const & expect) {
    return !is_ok(expect) ;
  }

  auto on_ok (auto && expect, auto && func) {
    if (is_ok (static_cast<decltype(expect)>(expect))) {
      static_cast<decltype(func)>(func) (
        static_cast<decltype(expect)>(expect)) ;
    }
  }

  auto on_ko (auto && expect, auto && func) { 
    if (is_ko (static_cast<decltype(expect)>(expect))) {
      static_cast<decltype(func)>(func) (
        static_cast<decltype(expect)>(expect)) ;
    }
  }
}

/// /////////////// ///
/// FILE OPERATIONS ///
/// /////////////// ///

#include <fstream>

namespace sia {
  
  auto open_file (auto const & filename) {
    using file_t = std::ifstream ;
    
    auto file = std::ifstream(filename, std::ios::in) ;
    return file.is_open() ? option<file_t>(std::move(file)) : option<file_t>() ;
  }

  auto is_open(auto const & optional_file) {
    return has_value(optional_file) ;
  }

  struct file_content { 
    std::string content ; 
  } ;

  using error_message_t = std::string ;

  auto extract_file_content (auto && ifs) {
    using file_begin_t = std::istreambuf_iterator<char> ;
    using file_end_t = file_begin_t ;

    return option<std::string>(
      std::string(
        file_begin_t(ifs), 
        file_end_t())) ;
  }
}

/// //////////////// ///
/// REGEX CONDITIONS ///
/// //////////////// ///

namespace sia::token {

  struct token {
    enum class type : size_t {
      name, 
      lbrace, 
      rbrace, 
      comma, 
      number
    } ;
    
    std::string filename ;
    int         line     ; 
    int         column   ; 
    std::string value    ; 
    type        tp       ;
  } ; 

  auto between (auto const & min, 
                auto const & value, 
                auto const & max) {
    return min <= value && value <= max ;
  }

  auto equal (auto const & cst, auto const & value) {
    return cst == value ;
  }

  auto oneof (auto const & value, auto const & ... cst) {
    return (... || equal(value, cst)) ;
  }

  auto quoted (auto && str) {
    return std::string("\"") + str + std::string("\"") ; 
  }

  auto next_name (auto begin, auto end) {
    while (begin != end && (
      between('a', *begin, 'z') || 
      between('a', *begin, 'z') || 
      equal('_', *begin))) {
      ++begin ;
    }

    return begin ;
  }

  auto next_lbrace (auto begin, auto end) {
    return begin != end && equal(*begin, '(') ;
  }

  auto next_rbrace (auto begin, auto end) {
    return begin != end && equal(*begin, ')') ;
  }

  auto next_number (auto begin, auto end) {
    while (begin != end && between('0', *begin, '9')) {
      ++begin ;
    }

    return begin ;
  } 

  auto next_comma (auto begin, auto end) {
    return begin != end && equal(*begin, ',') ;
  }


  auto is_blank = [] (auto const & c) {
    switch (c) {
      case '\t': 
      case '\n':
      case ' ' : return true ;
      default  : return false ;  
    }
  } ;

  auto filter_blank (auto begin, auto end) {
    return std::remove_if(begin, end, is_blank) ;
  }

  auto tokenize(auto & content) {
    auto begin  = std::begin(content) ;
    auto end    = filter_blank(begin, std::end(content)) ;
    auto cursor = begin ;

    
    while (begin != end) {
      auto has_advanced = 
        (((cursor = next_name(begin, end)) != begin) || 
         ((cursor = next_number(begin, end)) != begin) || 
         ((cursor = next_lbrace(begin, end)) != begin) || 
         ((cursor = next_rbrace(begin, end)) != begin) || 
         ((cursor = next_comma(begin, end)) != begin)) ;

      if (has_advanced) {
        begin = cursor ; 
      } else { 
        ++begin ;
      }
    }
    
    return std::string(begin, end) ;
  }
}



#include <sqlite3.h>



/*




auto open_file () {
  return std::optional<std::ifstream> {std::ifstream(filename, std::ios::in)} ;
}



auto read_file () {
  return [] (const std::string& filename) {
    expected<file_content, error_message_t> r ;
    match_expected(open_file()) (
      assign(r, extract_file_content()),
      assign(r, right_concat(CANT_READ_FILE))) (filename) ;
    return r ;
  } ;
}

/// /////////////////// ///
/// DATABASE OPERATIONS ///
/// /////////////////// ///

using db_t = sqlite3*;

auto sql_open_database () {
  return [] (const std::string& db_name) -> expected<db_t, error_message_t> {
    db_t db ;
    
    switch(sqlite3_open(db_name.c_str(), &db)) {
      case SQLITE_OK : return {db} ;
      default        : return {CANT_OPEN_DATABASE} ;
    }
  } ;
}

auto sql_create_database () {
  return sql_open_database() ;
}

auto sqlite3_exec_f () {
  return [] (db_t db, const std::string& query, char* buffer) {
    return sqlite3_exec(db, query.data(), NULL, NULL, &buffer) ;
  } ;
}

auto sql_execute_query () {
  return [] (db_t db, const std::string& query) {
    char* error_message_buffer = nullptr ;
    expected<db_t, error_message_t> r ;
    
    match (sqlite3_exec_f()) (
      when(equal(SQLITE_OK), assign(r, [&](auto &&){return db;})), 
      otherwise(assign(r, [&] (auto &&...) { 
        return CANT_EXECUTE_QUERY + " " 
          + quoted(query.c_str()) + " " 
          + error_message_buffer ; }))) 
      (db, query, error_message_buffer) ;

    sqlite3_free(error_message_buffer) ;
    return r ;
  } ;
}

auto sql_drop_tokens_table() {
  return [] (db_t db) {
    return sql_execute_query()(db, "drop table if exists t_token;") ;
  } ;
}

auto sql_create_tokens_table () {
  return [] (db_t db) {    
    return sql_execute_query()(db, 
      "create table if not exists t_token ( "
      "  id        integer  primary key,    "
      "  filename  text     not null   ,    "
      "  line      integer  not null   ,    "
      "  column    integer  not null   ,    "
      "  value     text     not null   ,    "
      "  type      integer  not null   )    ") ;
  } ;
}



auto token_new(
  const std::string& filename, 
  const int line,  
  const int column, 
  const std::string& value, 
  const token::type tp) {
  return token {
    .filename = filename, 
    .line = line, 
    .column = column, 
    .value = value, 
    .tp = tp
  } ;
}

auto sql_insert_one_token(db_t db, const token & tk) {
  auto query_builder = std::stringstream() ;
  query_builder << "insert into t_token (filename," 
                << "line, column, value, type) values (" 
                << std::move(quoted(tk.filename)) << ", "
                << tk.line << ", "
                << tk.column << ", "
                << std::move(quoted(tk.value)) << ", "
                << static_cast<size_t>(tk.tp) << ")" ;
  return sql_execute_query()(db, query_builder.str()) ;
}


/// //////////////////// ///
/// SIA TOKENS DETECTION ///
/// //////////////////// ///

auto next () {
  return [] (auto&& it) {return rstd::next(it) ;} ;
}

auto equal_inner (auto&& inner) {
  return [&] (auto&& left) {
    return *left == inner ; 
  } ;
}

auto next_lbrace(auto begin, auto end) {
  return match_expr(itself()) (
    when(equal(end))
    when(equal_inner('('), next())
    otherwise(itself())) (begin) ;
}
*/

/// /////////// ///
/// MAIN SCRIPT ///
/// /////////// ///

int main (int argc, const char** argv) {
  auto file = sia::open_file("lol.sia") ;
  
  sia::on_no_value(file, [] {
    std::cout << "NOT_OPEN" << std::endl ;
  }) ;

  sia::on_value(file, [] (auto & file_stream) {
    std::cout << "THE FILE IS OPEN" << std::endl ; 
    auto&& content = sia::extract_file_content(file_stream) ;
    
    sia::on_no_value(content, [] {
      std::cout << "PAS DE CONTENU LU" << std::endl ;
    }) ;

    sia::on_value(content, [] (auto & content) {
      std::cout << "THE CONTENT IS '" << content << "'\n";
    
      auto && tokens = sia::token::tokenize(content) ;
      std::cout << tokens << std::endl ;
    }) ;
  }) ; 

  std::cout << std::endl ; 
  return EXIT_SUCCESS ;
}


/// MAIN SCRIPT
