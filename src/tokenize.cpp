#include <sia.hpp>

namespace sia::error 
{
  const auto CANT_OPEN_FILE          = std::string("can't open the file.") ;
  const auto CANT_READ_FILE          = std::string("can't read file.") ;
  const auto CANT_OPEN_DATABASE      = std::string("can't open the database.") ;
  const auto CANT_CREATE_TOKEN_TABLE = std::string("can't create tokens table.") ;
  const auto CANT_EXECUTE_QUERY      = std::string("can't execute query.") ;
  const auto CANT_INSERT_TOKEN       = std::string("can't insert token.") ;
}

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

  auto load_token_type_table_into_map (db_t db) 
  {
    std::map<std::string, int> token_types ;
    auto && __token_types = select(db, 
      "select * from t_token_type",  token_types_mapper()) ;
    
    for (auto && tpl : __token_types)
      token_types[std::get<0>(tpl)] = std::get<1>(tpl) ;

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
    auto query = std::stringstream() ;
    query << "insert into t_token "
       << "(filename, line, column, value, type) " 
       << " values " << values ;

    return execute_transactional_query(db, query.str()) ;
  }

  auto prepare_database (db_t db) 
  {
    return execute_transactional_sql_files(db,
      "./sql/tokenize/drop_token_type_table_if_exists.sql", 
      "./sql/tokenize/create_token_type_table_if_not_exists.sql", 
      "./sql/tokenize/drop_tokens_table_if_exists.sql", 
      "./sql/tokenize/create_tokens_table_if_not_exists.sql",
      "./sql/tokenize/insert_ref_values_in_token_type_table.sql") ;
  }

  auto update_tokens_for_keywords (db_t db) 
  {
    return execute_transactional_sql_file(db, 
      "./sql/tokenize/update_tokens_for_keywords.sql") ;
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
      
      insert_tokens_values(db, values) ;
      current_line_num = current_line_num + chunk.size() ; 
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
  //sia::log::skip_debug() ;

  auto filename = std::string("lol2.sia") ;
  auto file     = std::ifstream(filename, std::ios::in) ;
  auto db       = open_database((filename + ".db").c_str()) ;

  if (file.is_open() && db != nullptr) 
  {
    prepare_database(db) ;
    tokenize_file(filename, file, db) ;
    close_database(db) ;
  } 
  else 
  {
    std::cout << std::endl ; 
    return EXIT_FAILURE ;
  }
   
  std::cout << std::endl ; 
  return EXIT_SUCCESS ;
}

/// MAIN SCRIPT
