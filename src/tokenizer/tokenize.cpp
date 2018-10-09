
#include <string>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <utility>
#include <sstream>


namespace sia::error {
  const auto CANT_OPEN_FILE          = std::string("can't open the file.") ;
  const auto CANT_READ_FILE          = std::string("can't read file.") ;
  const auto CANT_OPEN_DATABASE      = std::string("can't open the database.") ;
  const auto CANT_CREATE_TOKEN_TABLE = std::string("can't create tokens table.") ;
  const auto CANT_EXECUTE_QUERY      = std::string("can't execute query.") ;
  const auto CANT_INSERT_TOKEN       = std::string("can't insert token.") ;
}

namespace sia {
  auto const endl = '\n' ; 

  auto quote (const auto & str) {
    auto ss = std::stringstream() ;
    ss << '\'' ;
    ss << str  ;
    ss << '\'' ;
    return ss.str() ;
  }
}

/// //////////////// ///
/// REGEX CONDITIONS ///
/// //////////////// ///

#include <list>

namespace sia::token {

  struct compilation_context {
    std::string filename ;
  } ;

  struct token {
    enum class type : size_t {
      name, 
      lbrace, 
      rbrace, 
      lbracket, 
      rbracket, 
      point, 
      colon, 
      semi_colon,
      comma, 
      number
    } ;
    
    std::string filename ;
    int         line     ; 
    int         column   ; 
    std::string value    ; 
    type        tp       ;
  } ; 

  auto create(const std::string& filename, 
              int line, int column, 
              const std::string& value, 
              token::type tp) {
    return (token) {
      .filename = filename,
      .line     = line, 
      .column   = column, 
      .value    = value, 
      .tp       = tp
    } ;
  }

  auto between (auto const & min, 
                auto const & value, 
                auto const & max) {
    return min <= value && value <= max ;
  }

  auto equal (auto const & cst, 
              auto const & value) {
    return cst == value ;
  }

  template <typename cursor_t>
  struct match_result {
    cursor_t cursor ; 
    token::type tp ;
  } ;

  template <typename cursor_t>
  auto result (cursor_t cursor, token::type tp) {
    return match_result<cursor_t> {cursor, tp} ;
  } 

  constexpr auto next_name = [] (auto begin, auto end) {
    while (begin != end && (
      between('a', *begin, 'z') || 
      between('A', *begin, 'Z') || 
      equal('_', *begin))) {
      begin = std::next(begin);
    }

    return result(begin, token::type::name) ;
  } ;

  constexpr auto next_lbracket = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, '(') ? 
      std::next(begin) : begin), token::type::lbracket) ;
  } ;

  constexpr auto next_rbracket = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, ')') ? 
      std::next(begin) : begin), token::type::rbracket) ;
  } ;

  constexpr auto next_lbrace = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, '{') ? 
      std::next(begin) : begin), token::type::lbrace) ;
  } ;

  constexpr auto next_rbrace = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, '}') ? 
      std::next(begin) : begin), token::type::rbrace) ;
  } ;


  constexpr auto next_number = [] (auto begin, auto end) {
    while (begin != end && 
           between('0', *begin, '9')) {
      begin = std::next(begin);
    }

    return result(begin, token::type::number) ;
  } ;

  constexpr auto next_comma = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, ',') ? 
      std::next(begin) : begin), token::type::comma) ;
  } ;

  constexpr auto next_colon = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, ':') ?
      std::next(begin) : begin), token::type::colon) ;
  } ;

  constexpr auto next_semi_colon = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, ';') ?
      std::next(begin) : begin), token::type::semi_colon) ;
  } ;
  
  constexpr auto next_point = [] (auto begin, auto end) {
    return result((begin != end && equal(*begin, '.') ?
      std::next(begin) : begin), token::type::point) ;
  } ;

  auto is_blank (auto const & c) {
    switch (c) {
      case '\t' : 
      case ' '  : return true ;
      default   : return false ;  
    }
  } ;

  auto next_blank (auto begin, auto end) {
    while (begin != end && is_blank(*begin)) {
      begin = std::next(begin);
    }

    return begin ;
  }

  inline auto choose_first_match (
      auto begin, auto end, 
      auto && matcher, auto && ... matchers) {
    auto result = matcher(begin, end) ;
   
    if (result.cursor != begin) {
      return result ;
    } else {
      if constexpr (sizeof...(matchers) > 0) {
        return choose_first_match(begin, end, static_cast<decltype(matchers)&&>(matchers)...) ;
      } else {
        return result ; 
      }
    } 
  }

  auto read_chunk (auto & file, auto max_size) {
    auto chunk = std::list<std::string>() ;
    auto index = 0ull ;
    auto line  = std::string() ;

    while (index < max_size && std::getline(file, line)) { 
      chunk.push_back(line) ;
      index++ ;
    }
    
    std::cout << "chunk size read " << chunk.size() << std::endl ;
    return chunk ;
  }

  auto tokenize_line (auto && line, auto linenum, const auto & context) {
    auto tokens = std::list<token>() ;
    auto begin  = std::begin(line) ;
    auto cursor = begin ;
    auto end    = std::end(line) ;
    
    while ((cursor = next_blank(cursor, end)) != end) {  
      auto && [tk_cursor, tp] = choose_first_match(cursor, end, 
        next_name, next_number, next_comma, 
        next_lbrace, next_rbrace, next_lbracket, 
        next_rbracket, next_point, next_semi_colon, 
        next_colon) ;
      auto has_advanced = tk_cursor != cursor ; 

      if (has_advanced) { 
        tokens.push_back(
          create(context.filename, linenum, 
            std::distance(begin, cursor) + 1, 
            std::string(cursor, tk_cursor), tp)) ;
        cursor = tk_cursor ; 
      } else {
        cursor = std::next(cursor) ;
      }
    }

    return tokens ;
  }
    
  struct chunk_context {
    std::string filename ; 
    size_t      first_line_num ; 
  } ;

  auto tokenize_chunk (auto const & chunk, auto const & context) {
    auto tokens = std::list<sia::token::token>() ;
    auto linenum = context.first_line_num ;
    
    for (auto const & line : chunk) {
      auto && line_tokens = sia::token::tokenize_line(line, linenum, context) ;
      
      for (auto const & tk : line_tokens) {
        tokens.push_back(std::move(tk)) ;
      }

      linenum = linenum + 1 ;
    }

    return tokens ;
  }
}



/// /////////////////// ///
/// DATABASE OPERATIONS ///
/// /////////////////// ///


#include <sqlite3.h>

namespace sia::db {

  using db_t = sqlite3*;
  
  auto execute_query (db_t db, const std::string& query) {
    char* error_message_buffer = nullptr ;
    sqlite3_exec(db, query.data(), nullptr, nullptr, &error_message_buffer) ;
    sqlite3_free(error_message_buffer) ;
  }

  auto begin_transaction(db_t db) {
    return execute_query(db, "begin transaction") ;
  }

  auto end_transaction(db_t db) {
    return execute_query(db, "commit") ;
  }

  auto open_database (const std::string& db_name) {
    db_t db ;
    sqlite3_open(db_name.c_str(), &db) ;
    return db ;
  }

  auto drop_tokens_table_if_exists(db_t db) {
    return execute_query(db, "drop table if exists t_token") ;
  }

  auto create_tokens_table_if_not_exists (db_t db) {    
    return execute_query(db, 
      "create table if not exists t_token ( "
      "  id        integer  primary key,    "
      "  filename  text     not null   ,    "
      "  line      integer  not null   ,    "
      "  column    integer  not null   ,    "
      "  value     text     not null   ,    "
      "  type      integer  not null   )    ") ;
  }

  auto prepare_one_token_to_be_inserted (auto const & token) {
    auto ss = std::stringstream() ;
    ss << '(' 
      << sia::quote(std::move(token.filename)) << ", "
      << token.line << ", "
      << token.column << ", "
      << sia::quote(std::move(token.value)) << ", "
      << static_cast<size_t>(token.tp) 
      << ')' ;
    return ss.str() ;
  }

  auto prepare_tokens_to_be_inserted (auto const & tokens) {
    auto ss = std::stringstream() ;
    auto i  = 0ull ;

    for (auto const & token  : tokens) {
      ss << std::move(prepare_one_token_to_be_inserted(token)) ;

      if (i < tokens.size() - 1) {
        ss << ',' ;
      }

      i = i + 1 ; 
    }

    return ss.str() ; 
  }

  using db_t = sqlite3*;

  auto insert_tokens_values(db_t db, const std::string & values) {
    auto ss = std::stringstream() ;
    ss << "insert into t_token (filename, line, column, value, type) values "
      << values ;

    return sia::db::execute_query(db, ss.str()) ;  
  }
}

/// /////////// ///
/// MAIN SCRIPT ///
/// /////////// ///




#include <fstream>

int main (int argc, const char** argv) {

  auto chunk_size      = 5ull ;
  auto global_line_num = 1ull ;
  auto filename        = std::string("lol2.sia") ;
  auto file            = std::ifstream(filename, std::ios::in) ;
  auto db_name         = filename + ".db" ;
  auto db              = sia::db::open_database(db_name.c_str()) ;

  if (file.is_open() && db != nullptr) {
    sia::db::begin_transaction(db) ;
    sia::db::drop_tokens_table_if_exists(db) ;
    sia::db::create_tokens_table_if_not_exists(db) ;
    sia::db::end_transaction(db) ;

    decltype(sia::token::read_chunk(file, chunk_size)) chunk ;

    while (!(chunk = std::move(sia::token::read_chunk(file, chunk_size))).empty()) {
      auto && context = sia::token::chunk_context{filename, global_line_num} ;
      auto && tokens  = sia::token::tokenize_chunk(chunk, context) ;
      auto && values  = sia::db::prepare_tokens_to_be_inserted(tokens) ;
      
      sia::db::begin_transaction(db) ;
      sia::db::insert_tokens_values(db, values) ;
      sia::db::end_transaction(db) ;

      global_line_num = global_line_num + chunk.size() ; 
    }
  }
   
  std::cout << std::endl ; 
  return EXIT_SUCCESS ;
}

/// MAIN SCRIPT
