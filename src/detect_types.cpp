#include <sia.hpp>
#include <vector>
#include <map>
#include <string_view> 

namespace sia::type
{
  struct type_member 
  {
    std::string name      ;  
    std::string type_name ;
  } ; 
  
  struct type 
  {
    std::string              name    ; 
    std::vector<type_member> members ;
  } ; 
}

namespace sia::db 
{
  auto prepare_database (db_t db) 
  {
    return execute_transactional_sql_files(db,
      "./sql/detect_types/create_view_types_boundaries.sql",
      "./sql/detect_types/drop_type_table_if_exists.sql", 
      "./sql/detect_types/create_type_table_if_not_exists.sql", 
      "./sql/detect_types/insert_ref_values_in_type_table.sql", 
      "./sql/detect_types/drop_type_member_table_if_exists.sql",
      "./sql/detect_types/create_type_member_table_if_not_exists.sql") ;
  }

  using token_types_t = std::map<std::string, int> ;
  
  auto & to_buffer (void* buffer) 
  {
    return *static_cast<token_types_t*>(buffer) ;
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
}

#include <cstdlib>
#include <list>

struct token_mapper 
{
  sia::token::token 
  operator() (
    sia::db::row_t && row) const
  {
    using namespace sia::token ;
    return (token) {
        .filename = row.at("filename"),
        .line     = std::stoi(row.at("line")),
        .column   = std::stoi(row.at("column")), 
        .value    = row.at("value"), 
        .type     = row.at("type")
      } ;
  }
} ;

struct type_boundaries 
{
  long begin = 0l, end = 0l ;
} ;

struct type_boundaries_mapper
{
  type_boundaries
  operator() (sia::db::row_t && row) const
  {
    return (type_boundaries) {
      .begin = std::stoi(row.at("begin")), 
      .end   = std::stoi(row.at("end"))
    } ;
  }
} ;

struct token_types_mapper
{
  auto operator() (std::map<std::string, std::string> && row)  const
  {
    auto && value = row.at("value") ;
    auto && key   = std::stoi(row.at("key")) ;
    return std::tuple(value, key) ;
  }
} ;

auto load_token_type_table_into_map (sia::db::db_t db) 
{
  using namespace sia::db ;

  std::map<std::string, int> token_types ;
  auto && __token_types = select(db, 
    "select * from t_token_type",  token_types_mapper()) ;
  
  for (auto && tpl : __token_types)
    token_types[std::get<0>(tpl)] = std::get<1>(tpl) ;

  return token_types ;
}

template <typename cursor_t>
struct match_track
{
  cursor_t cursor  ;
  cursor_t end     ; 
  bool     matched ;
} ;

template <typename cursor_t>
auto build_track (
  cursor_t const & cursor, 
  cursor_t const & end, 
  bool matched = true)
{
  return match_track<cursor_t> {
    .cursor  = cursor,
    .end     = end,  
    .matched = matched
  } ;
}

auto is_of_type (
  match_track<auto> const & track, 
  std::string_view          type, 
  auto const &              token_types)
{
  return std::stoi((*track.cursor).type) == token_types.at(std::string(type)) ;
}

auto is_not_end (match_track<auto> const & track)
{
  return track.cursor != track.end ;
}

auto is_not_end_and_equal_to (
  match_track<auto> const & track,
  std::string_view          type, 
  auto const &              token_types)
{
  auto const & matched = is_not_end(track) && is_of_type(track, type, token_types) ;
  auto const & cursor  = matched ? std::next(track.cursor) : track.cursor ;
  return build_track(cursor, track.end, track.matched && matched) ;
}

auto is_name (
  match_track<auto> const & track,
  auto const &              token_types) 
{
  return is_not_end_and_equal_to(track, "name", token_types) ;
}

auto is_lbracket (
  match_track<auto> const & track,  
  auto const &              token_types)
{
  return is_not_end_and_equal_to(track, "lbracket", token_types) ;
}

auto is_rbracket (
  match_track<auto> const & track,
  auto const &              token_types) 
{
  return is_not_end_and_equal_to(track, "rbracket", token_types) ;
}

auto is_colon (
  match_track<auto> const & track,
  auto const &              token_types) 
{
  return is_not_end_and_equal_to(track, "colon", token_types) ;
}

auto is_comma (
  match_track<auto> const & track,
  auto const &              token_types) 
{
  return is_not_end_and_equal_to(track, "comma", token_types) ;
}

auto is_type_param(
  match_track<auto> const & track, 
  auto const &              token_types)
{
  auto && name_tracked     = is_name(track, token_types)         ; 
  auto && colon_tracked    = is_colon(name_tracked, token_types) ;
  auto && typename_tracked = is_name(colon_tracked, token_types) ;

  return typename_tracked ;
}

auto is_type_param_list(
  match_track<auto> const & track, 
  auto const &              token_types) 
{
  auto result_track = track ;
  auto current_track = track ;

  while (current_track.matched && (current_track = is_type_param(current_track, token_types)).matched) 
  {    
    auto comma_track = current_track ;
    
    if (!(comma_track = is_comma(comma_track, token_types)).matched) 
    { 
      result_track = current_track ;
      break ;
    }
    else 
    {
      current_track = comma_track ;
    } 
  }

  return result_track ;
}

auto is_type (
  match_track<auto> const & track,
  auto const &              token_types)
{
  return is_not_end_and_equal_to(track, "type", token_types) ;
}

auto is_type_declaration (
  match_track<auto> const & track,
  auto const &              token_types) 
{
  auto && type_tracked     = is_type(track, token_types) ;
  auto && name_tracked     = is_name(type_tracked, token_types) ;
  auto && lbracket_tracked = is_lbracket(name_tracked, token_types) ;
  auto && params_tracked   = is_type_param_list(lbracket_tracked, token_types) ;
  auto && rbracket_tracked = is_rbracket(params_tracked, token_types) ;
  
  return rbracket_tracked ; 
}

int main (int argc, char** argv) 
{
  using namespace sia::db ;
  using namespace sia::script ;
  
  launching_of(argv[0]) ;
  sia::log::skip_debug() ;

  auto db = open_database("lol2.sia.db") ;
  
  if (!is_db_open(db)) 
  {
    std::cout << std::endl ; 
    return EXIT_FAILURE ;
  } 

  prepare_database(db) ;

  std::cout << "nb element in tokens : " << count(db, "t_token") << "\n" ;
  const auto limit = 100u ; 
  const auto offset_max = count(db, "v_types_boundaries") ;
  auto offset = 0u ; 
  auto token_types = load_token_type_table_into_map(db) ; 
   
  while (offset <= offset_max) 
  {  
    auto types_bounds = select(db, 
      "select * from v_types_boundaries", 
      limit, offset, 
      type_boundaries_mapper()) ;
    
    for (auto && bounds : types_bounds)
    {    
      auto token_query = std::string("select * from t_token as tk where tk.id between ")
        + std::to_string(bounds.begin)
        + " and "
        + std::to_string(bounds.end) ;

      auto tokens = select(db, token_query, token_mapper()) ;

      std::cout << std::boolalpha 
                << is_type_declaration(build_track(tokens.begin(), tokens.end()), token_types).matched 
                << "\n" ;
    }

    offset += limit ; 
  }



  close_database(db) ;

  std::cout << std::endl ; 
  return EXIT_SUCCESS ;
}