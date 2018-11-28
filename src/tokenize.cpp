#include <sia.hpp>

/// //////////////// ///
/// REGEX CONDITIONS ///
/// //////////////// ///

#include <list>

namespace sia::token 
{
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

  auto tokenize_line (
    auto && line) 
  {
    auto tokens = std::list<token>() ;
    auto begin  = std::begin(line.line) ;
    auto cursor = begin ;
    auto end    = std::end(line.line) ;
    
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
          create(line.filename, line.num, 
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
    auto const & chunk) 
  { 
    using namespace sia::token ;

    auto tokens = std::list<token>() ;
    
    for (auto const & line : chunk) 
    {
      auto && line_tokens = tokenize_line(line) ;
      
      for (auto const & tk : line_tokens) 
      {
        tokens.push_back(std::move(tk)) ;
      }
    }

    return tokens ;
  }
}



/// /////////////////// ///
/// DATABASE OPERATIONS ///
/// /////////////////// ///



#include <map>
#include <functional>
#include <list>
#include <tuple>

namespace sia::db 
{
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
    return 0 ;
  }

  struct token_types_mapper
  {
    auto operator() (std::map<std::string, std::string> && row)  const
    {
      auto && value = row.at("value") ;
      auto && key   = std::stoi(row.at("key")) ;
      return std::tuple(value, key) ;
    }
  } ;

  auto prepare_one_token_to_be_inserted (
    auto const & token) 
  {
    auto ss = std::stringstream() ;
    ss << '(' 
      << sia::quote(std::move(token.filename)) << ", "
      << token.line << ", "
      << token.column << ", "
      << sia::quote(std::move(token.value)) << ", "
      << sia::quote(token.type)
      << ')' ;
    return ss.str() ;
  }

  auto prepare_tokens_to_be_inserted (
    auto const & tokens) 
  {
    auto ss = std::stringstream() ;
    auto i  = 0ull ;

    for (auto const & token  : tokens) 
    {
      ss << std::move(prepare_one_token_to_be_inserted(token)) ;

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
    auto query = std::stringstream() ;
    query << "insert into tkn_token "
       << "(filename, line, column, value, type) " 
       << " values " << values ;

    return ddl(db, query.str()) ;
  }

  auto update_tokens_for_keywords (db_t db) 
  {
    return ddl(db, 
      "update tkn_token set type='fn' where value = 'fn' ;    "
      "update tkn_token set type='type' where value = 'type' ;") ;
  }

  struct file_line 
  {
    std::string filename ;
    std::string line     ;
    size_t      num      ;
    size_t      length   ;
  } ;

  struct file_line_mapper 
  {
    auto operator() (
      sia::db::row_t const & row) const
    {
      return (file_line) {
        .filename = row.at("filename")         ,
        .line     = row.at("line")             ,
        .num      = std::stoull(row.at("num"))   , 
        .length   = std::stoull(row.at("length"))  
      } ;
    }
  } ;

  auto select_lines(
    sia::db::db_t       db,
    std::string const & filename, 
    auto const &        limit, 
    auto const &        offset)
  {
    using namespace sia::db ;
    std::stringstream ss ;
    ss << " select filename, line, num, length "
       << " from tkn_file_lines " ;
    return select(db, ss.str(), limit, offset, file_line_mapper()) ; 
  }

  auto tokenize_file (
    const std::string & filename,
    auto &              file, 
    db_t                db) 
  {
    using namespace sia::token ;
    using namespace sia::db    ;

    auto const chunk_size = 100ull ;
    auto const nb_lines   = count(
      db, "tkn_file_lines", 
      equal("filename", sia::quote(filename))) ;
    auto offset = 0ull ; 
    
    while (offset <= nb_lines)  
    {
      auto && chunk   = select_lines(db, filename, chunk_size, offset) ; 
      auto && tokens  = tokenize_chunk(chunk) ;
      auto && values  = prepare_tokens_to_be_inserted(tokens) ;
      
      insert_tokens_values(db, values) ;
      offset += chunk_size ; 
    } 

    update_tokens_for_keywords(db) ;
  }
}

/// /////////// ///
/// MAIN SCRIPT ///
/// /////////// ///

#include <fstream>

int main (int argc, const char** argv) 
{
  using namespace sia::db ;
  using namespace sia::script ;
  
  launching_of(argv[0]) ;

  auto filename = std::string("lol2.sia") ;
  auto file     = std::ifstream(filename, std::ios::in) ;
  auto db       = open_database((filename + ".db").c_str()) ;

  if (file.is_open() && is_db_open(db)) 
  {
    tokenize_file(filename, file, db) ;
    close_database(db) ;
    stop_of(argv[0]) ; 
    std::cout << std::endl ; 
    return EXIT_SUCCESS ;
  } 
  else 
  {
    interruption_of(argv[0]) ;
    std::cout << std::endl ; 
    return EXIT_FAILURE ;
  }
}

/// MAIN SCRIPT
