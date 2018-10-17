
#include <string>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <utility>
#include <sstream>


namespace sia::error 
{
  const auto CANT_OPEN_FILE          = std::string("can't open the file.") ;
  const auto CANT_READ_FILE          = std::string("can't read file.") ;
  const auto CANT_OPEN_DATABASE      = std::string("can't open the database.") ;
  const auto CANT_CREATE_TOKEN_TABLE = std::string("can't create tokens table.") ;
  const auto CANT_EXECUTE_QUERY      = std::string("can't execute query.") ;
  const auto CANT_INSERT_TOKEN       = std::string("can't insert token.") ;
}

namespace sia 
{
  auto const endl = '\n' ; 

  auto quote (const auto & str) 
  {
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

namespace sia::token 
{
  struct token 
  {
    std::string filename ;
    int         line     ; 
    int         column   ; 
    std::string value    ; 
    std::string type     ;
  } ; 

  auto create(const std::string& filename, 
              int line, int column, 
              const std::string& value, 
              std::string type) 
  {
    return (token) 
    {
      .filename = filename,
      .line     = line, 
      .column   = column, 
      .value    = value, 
      .type     = type
    } ;
  }

  auto between (auto const & min, 
                auto const & value, 
                auto const & max) 
  {
    return min <= value && value <= max ;
  }

  auto equal (auto const & cst, 
              auto const & value)
  {
    return cst == value ;
  }

  template <typename cursor_t>
  struct match_result
  {
    cursor_t    cursor ; 
    std::string tp     ;
  } ;

  template <typename cursor_t>
  auto result (cursor_t cursor, std::string const & tp) 
  {
    return match_result<cursor_t> {cursor, tp} ;
  } 

  constexpr auto next_name = 
  [] (auto const & begin, 
      auto const & end) 
  {
    auto cursor = begin ;

    while (cursor != end && (
      between('a', *cursor, 'z') || 
      between('A', *cursor, 'Z') || 
      equal('_', *cursor))) 
    {
      cursor = std::next(cursor);
    }

    return result(cursor, "name") ; // Replace by string
  } ;

  auto not_end_and_equal_to (
    auto const & begin, 
    auto const & end, 
    auto const & c, 
    auto const & type) 
  {
    return result((begin != end && equal(*begin, c) ? 
      std::next(begin) : begin), type) ;  
  }


  constexpr auto next_lbracket = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, '(', "lbracket") ;
  } ;

  constexpr auto next_rbracket = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, ')', "rbracket") ;
  } ;

  constexpr auto next_lbrace = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, '{', "lbrace") ;
  } ;

  constexpr auto next_rbrace = 
  [] (auto const & begin, 
      auto const & end) 
  {
    return not_end_and_equal_to (
      begin, end, '}', "rbrace") ;
  } ;


  constexpr auto next_number = 
  [] (auto const & begin, 
      auto const & end)
  {
    auto cursor = begin ;

    while (cursor != end && 
           between('0', *cursor, '9')) 
    {
      cursor = std::next(cursor);
    }

    return result(cursor, "number") ;
  } ;

  constexpr auto next_comma = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, ',', "comma") ;
  } ;

  constexpr auto next_colon = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, ':', "colon") ;
  } ;

  constexpr auto next_semi_colon = 
 [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, ';', "semi_colon") ;
  } ;
  
  constexpr auto next_point = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, '.', "point") ;
  } ;

  constexpr auto next_equal = 
  [] (auto const & begin, 
      auto const & end)
  {
    return not_end_and_equal_to (
      begin, end, '=', "equal") ;
  } ;

  auto is_blank (auto const & c) 
  {
    switch (c) 
    {
      case '\t' : 
      case ' '  : return true ;
      default   : return false ;  
    }
  } ;

  auto next_blank (
    auto const & begin, 
    auto const & end) 
  {
    auto cursor = begin ;
    
    while (cursor != end && is_blank(*cursor))
    {
      cursor = std::next(cursor);
    }

    return cursor ;
  }

  inline auto choose_first_match (
      auto const & begin, 
      auto const & end, 
      auto &&      matcher, 
      auto && ...  matchers) 
  {
    auto result = matcher(begin, end) ;
   
    if (result.cursor != begin) 
    {
      return result ;
    } 
    else 
    {
      if constexpr (sizeof...(matchers) > 0) 
      {
        return choose_first_match(
          begin, end, 
          static_cast<decltype(matchers)&&>(matchers)...) ;
      } 
      else 
      {
        return result ; 
      }
    } 
  }

  auto read_chunk (auto & file, auto max_size) 
  {
    auto chunk = std::list<std::string>() ;
    auto index = 0ull ;
    auto line  = std::string() ;

    while (index < max_size && std::getline(file, line)) 
    { 
      chunk.push_back(line) ;
      index++ ;
    }
    
    return chunk ;
  }

  auto tokenize_line (
    auto &&      line, 
    auto         linenum, 
    auto const & context) 
  {
    auto tokens = std::list<token>() ;
    auto begin  = std::begin(line) ;
    auto cursor = begin ;
    auto end    = std::end(line) ;
    
    while ((cursor = next_blank(cursor, end)) != end) 
    {  
      auto && [tk_cursor, tp] = 
        choose_first_match(cursor, end, 
          next_name, next_number, next_comma, 
          next_lbrace, next_rbrace, next_lbracket, 
          next_rbracket, next_point, next_semi_colon, 
          next_colon, next_equal) ;
      auto has_advanced = tk_cursor != cursor ; 

      if (has_advanced) 
      { 
        tokens.push_back(
          create(context.filename, linenum, 
            std::distance(begin, cursor) + 1, 
            std::string(cursor, tk_cursor), tp)) ;
        cursor = tk_cursor ; 
      } 
      else 
      {
        cursor = std::next(cursor) ;
      }
    }

    return tokens ;
  }
    
  struct chunk_context 
  {
    std::string filename ; 
    size_t      first_line_num ; 
  } ;

  auto create_chunk_context (
    const std::string & filename, 
    size_t              first_line_num) 
  {
    return chunk_context 
    {
      filename, 
      first_line_num
    } ;
  }

  auto tokenize_chunk (
    auto const & chunk, 
    auto const & context) 
  {
    auto tokens = std::list<sia::token::token>() ;
    auto linenum = context.first_line_num ;
    
    for (auto const & line : chunk) 
    {
      auto && line_tokens = 
        sia::token::tokenize_line(
          line, linenum, context) ;
      
      for (auto const & tk : line_tokens) 
      {
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
#include <map>
#include <functional>

namespace sia::db {

  using db_t = sqlite3*;
  using select_callback_t = 
    int(void *, int, char **, char **) ;
  
  auto execute_query (
    db_t               db, 
    const std::string& query) 
  {
    char* error_message_buffer = nullptr ;
    sqlite3_exec(
      db, query.data(), nullptr, 
      nullptr, &error_message_buffer) ;
    sqlite3_free(error_message_buffer) ;
  }

  auto execute_select_query (
    db_t               db, 
    const std::string& query, 
    select_callback_t callback, 
    void* data_buffer = nullptr) 
  {
    char* error_message_buffer = nullptr ;
    sqlite3_exec(
      db, query.data(), callback, 
      data_buffer, &error_message_buffer) ;

    if (error_message_buffer != nullptr) 
    {
      std::cout << error_message_buffer << std::endl ; 
    }

    sqlite3_free(error_message_buffer) ;
  }

  auto begin_transaction(db_t db) 
  {
    return execute_query(db, "begin transaction") ;
  }

  auto end_transaction(db_t db) 
  {
    return execute_query(db, "commit") ;
  }

  auto open_database (const std::string& db_name) 
  {
    db_t db ;
    sqlite3_open(db_name.c_str(), &db) ;
    return db ;
  }

  auto close_database(db_t db) 
  {
    return sqlite3_close(db) ;
  }

  auto drop_tokens_table_if_exists(db_t db) 
  {
    return execute_query(db, 
      "drop table if exists t_token") ;
  }

  auto create_tokens_table_if_not_exists (db_t db) 
  {    
    return execute_query(db, 
      "create table if not exists t_token ( "
      "  id        integer  primary key,    "
      "  filename  text     not null   ,    "
      "  line      integer  not null   ,    "
      "  column    integer  not null   ,    "
      "  value     text     not null   ,    "
      "  type      integer  not null   )    ") ;
  }

  
  auto drop_tokens_table_type_if_exists (db_t db) 
  {
    return execute_query(db, 
      "drop table if exists t_token_type") ;
  }

  auto create_token_type_table_if_not_exists (db_t db) 
  {
    return execute_query(db, 
      "create table if not exists t_token_type ("
      "  key      integer not_null ,            "
      "  value    text    not null )            ") ;
  }

  auto insert_ref_values_in_token_type_table (db_t db) 
  {
    return execute_query(db, 
      "insert into t_token_type (key, value) " 
      " values (0, 'name'),                  "
      "        (1, 'lbrace'),                "
      "        (2, 'rbrace'),                "
      "        (3, 'lbracket'),              "
      "        (4, 'rbracket'),              "
      "        (5, 'point'),                 "
      "        (6, 'colon'),                 "
      "        (7, 'semi_colon'),            "
      "        (8, 'comma'),                 "
      "        (9, 'number'),                "
      "        (10, 'equal'),                "
      "        (11, 'fn'),                   "
      "        (12, 'type')                  ") ;  
  }
  
  using token_types_t = std::map<std::string, int> ;
  
  auto & to_buffer (void* buffer) 
  {
    return (*(token_types_t*) buffer) ;
  }

  int select_token_types_callback (
    void *  token_types_buffer, 
    int     nb_column, 
    char ** values, 
    char ** columns)
  {
    to_buffer(token_types_buffer)[values[1]] = 
      std::stoi(values[0]) ;
    return 0 ;
  }

  auto load_token_type_table_into_map (db_t db) 
  {
    std::map<std::string, int> token_types ;
    execute_select_query(
      db, 
      "select * from t_token_type", 
      select_token_types_callback, 
      &token_types) ;

    return token_types ;
  }

  auto prepare_one_token_to_be_inserted (
    auto const & token, 
    auto const & token_types) 
  {
    auto ss = std::stringstream() ;
    ss << '(' 
      << sia::quote(std::move(token.filename)) << ", "
      << token.line << ", "
      << token.column << ", "
      << sia::quote(std::move(token.value)) << ", "
      << token_types.at(token.type)
      << ')' ;
    return ss.str() ;
  }

  auto prepare_tokens_to_be_inserted (
    auto const & tokens, 
    auto const & token_types) 
  {
    auto ss = std::stringstream() ;
    auto i  = 0ull ;

    for (auto const & token  : tokens) 
    {
      ss << std::move(
        prepare_one_token_to_be_inserted(
          token, token_types)) ;

      if (i < tokens.size() - 1) 
      {
        ss << ',' ;
      }

      i = i + 1 ; 
    }

    return ss.str() ; 
  }

  using db_t = sqlite3*;

  auto insert_tokens_values(
    db_t                db, 
    const std::string & values) 
  {
    auto ss = std::stringstream() ;
    ss << "insert into t_token "
       << "(filename, line, column, value, type) " 
       << " values " << values ;

    return execute_query(db, ss.str()) ;  
  }

  auto prepare_database (db_t db) 
  {
    begin_transaction(db) ;
    drop_tokens_table_type_if_exists(db) ;
    create_token_type_table_if_not_exists(db) ; 
    insert_ref_values_in_token_type_table(db) ; 
    drop_tokens_table_if_exists(db) ;
    create_tokens_table_if_not_exists(db) ;
    end_transaction(db) ;
  }

  auto build_update_keyword_query (
    std::string const & keyword, 
    auto const &        token_types) 
  {
    auto query = std::stringstream() ;
    query << "update t_token set type=" 
          << token_types.at(keyword) 
          << " where value = '"<< keyword << "'" ;

    std::cout << query.str() << std::endl ;

    return query.str() ;
  }

  auto update_tokens_for_keyword (
    db_t         db,
    auto const & kw, 
    auto const & token_types)
  {
    return execute_query(
      db, build_update_keyword_query(
        kw, token_types)) ;
  }

  auto update_tokens_for_keywords (
    db_t             db, 
    auto const &     token_types, 
    auto const & ... keywords) 
  {
    (update_tokens_for_keyword(
      db, keywords, token_types), ...) ;
  }

  auto detect_keywords (
    db_t         db, 
    auto const & token_types) 
  { 
    update_tokens_for_keywords(
      db, token_types, "type", "fn") ;
  }

  auto tokenize_file (
    const std::string & filename,
    auto &              file, 
    db_t                db) 
  {
    using namespace sia::token ;

    auto current_line_num = 1ull ;
    auto chunk_size      = 100ull ;
    
    decltype(read_chunk(file, chunk_size)) chunk ;
    auto token_types = load_token_type_table_into_map(db) ;
    
    while (!(chunk = std::move(read_chunk(file, chunk_size))).empty()) 
    {
      auto && context = create_chunk_context(filename, current_line_num) ;
      auto && tokens  = tokenize_chunk(chunk, context) ;
      auto && values  = prepare_tokens_to_be_inserted(tokens, token_types) ;
      
      begin_transaction(db) ;
      insert_tokens_values(db, values) ;
      end_transaction(db) ;

      current_line_num = current_line_num + chunk.size() ; 
    }

    begin_transaction(db) ; 
    detect_keywords(db, token_types) ;
    end_transaction(db) ;
  }
}

/// /////////// ///
/// MAIN SCRIPT ///
/// /////////// ///

#include <fstream>

int main (int argc, const char** argv) 
{
  using namespace sia::db ;
  
  auto filename = std::string("lol2.sia") ;
  auto file     = std::ifstream(filename, std::ios::in) ;
  auto db       = open_database((filename + ".db").c_str()) ;

  if (file.is_open() && db != nullptr) 
  {
    prepare_database(db) ;
    tokenize_file(filename, file, db) ;
    close_database(db) ;
  }
   
  std::cout << std::endl ; 
  return EXIT_SUCCESS ;
}

/// MAIN SCRIPT
