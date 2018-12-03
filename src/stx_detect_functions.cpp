#include <sia.hpp>
#include <sstream>
using namespace sia::db ;

using limit_t = size_t ;

struct function_boundaries 
{
  limit_t begin ; 
  limit_t end   ;
} ;

struct function_boundaries_mapper
{
  auto operator() (row_t const & row) const
  {
    return function_boundaries {
      .begin = std::stoull(row.at("begin")) ,
      .end   = std::stoull(row.at("end")) 
    } ;
  }
} ;

auto select_functions_boundaries (
  db_t    db, 
  limit_t limit, 
  limit_t offset) 
{
  auto query = "select begin, end from stx_functions_boundaries" ;
  return select(db, query, limit, offset, function_boundaries_mapper()) ;
}

template <typename cursor_t>
struct match_track
{
  cursor_t begin   ; 
  cursor_t cursor  ;
  cursor_t end     ;
  bool     matched ; 
} ;

template <typename cursor_t>
auto build_track (
  cursor_t const & begin , 
  cursor_t const & cursor, 
  cursor_t const & end   , 
  bool matched = true)
{
  return match_track<cursor_t> {
    .begin   = begin   ,
    .cursor  = cursor  ,
    .end     = end     ,  
    .matched = matched  
  } ;
}

auto is_function (
  auto const & begin, 
  auto const & end) 
{
  //TODO faire les fonctions de validation et de track d'une fonction  
  return build_track(begin, begin, end) ;
}

struct token_mapper 
{
  sia::token::token 
  operator() (
    sia::db::row_t && row) const
  {
    using namespace sia::token ;
    return (token) {
        .id       = std::stoull(row.at("id")),
        .filename = row.at("filename"),
        .line     = std::stoi(row.at("line")),
        .column   = std::stoi(row.at("column")), 
        .value    = row.at("value"), 
        .type     = row.at("type")
      } ;
  }
} ;

auto create_token_query (
  function_boundaries const & fbound) 
{
  std::stringstream ss ;
  ss << "select * from tkn_token as tk where tk.id between "
     << std::to_string(fbound.begin) 
     << " and " << std::to_string(fbound.end) 
     << " order by tk.id;";
  return ss.str() ;
}

auto select_tokens_from_boundaries (
  db_t                        db, 
  function_boundaries const & fbound)
{
  return select(db, create_token_query(fbound), token_mapper()) ;
}

auto insert_function (
  db_t db, 
  match_track<auto> const & track) 
{
  // TODO faire la table function 
}

auto insert_treated_tokens (
  sia::db::db_t             db,
  match_track<auto> const & track)
{
  auto && begin = track.begin ;
  auto && end   = std::prev(track.end) ;
  
  std::stringstream ss ;
  ss << " insert into tkn_treated_token_interval (begin_id, end_id) values ("
     << (*begin).id << ", " << (*end).id << ");" ;
  
  return ddl(db, ss.str()) ;
}


auto insert_function_detection_error(
  db_t db, 
  match_track<auto> const & track)
{
  std::stringstream ss ;
  ss << "insert into stx_function_error (expected_type, token_id) values (" 
     << sia::quote((*track.cursor).type) << ", " 
     << (*track.cursor).id << ");" ;
  sia::db::ddl(db, ss.str()) ;
}

auto detect_functions (
  db_t db)
{
  using namespace sia::config ;

  limit_t limit  = get_conf_ull("detect_function.chunk.size") ;
  limit_t offset = 0ull ;

  auto && fbounds = select_functions_boundaries(db, limit, offset) ; 
  for (auto const & fbound : fbounds)
  {
     
    auto && tokens = select_tokens_from_boundaries(db, fbound) ; 
    auto && function_track = is_function(tokens.begin(), tokens.end()) ;
    
    if (function_track.matched)
    {
      insert_function(db, function_track) ;
      insert_treated_tokens(db, function_track) ;
    }
    else 
    {
      insert_function_detection_error(db, function_track) ;
    }
  }

  return false ;
}

int main (int argc, char ** argv)
{
  sia::script::launching_of(argv[0]) ;
  
  auto db        = open_database("lol2.sia.db") ;
  auto has_error = detect_functions(db)         ; 
  
  sia::script::stop_of(argv[0]) ;
  return has_error ? EXIT_FAILURE : EXIT_SUCCESS ;
}
