#include <sia.hpp>
#include <boost/assign/list_of.hpp>
#include <vector>
#include <map>
#include <string_view> 
#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/algorithm/transform.hpp>

using namespace sia::db ;

using tokens_t = std::list<sia::token::token> ;
using tokens_it_t = tokens_t::iterator ; 
using limit_t = size_t ;

namespace sia::type
{
  struct type_member 
  {
    std::string name      ;  
    std::string type_name ;
  } ; 

  auto create_type_member (
    std::string const & name, 
    std::string const & type_name)
  {
    return type_member {
      .name      = name, 
      .type_name = type_name
    } ;
  }
  
  struct type 
  {
    std::string              name    ; 
    std::vector<type_member> members ;
  } ; 

  auto create_type (
    std::string const &              name, 
    std::vector<type_member> const & members)
  {
    return type {
      .name    = name, 
      .members = members
    } ;
  }
}

using type_members_t = std::vector<sia::type::type_member> ;

#include <cstdlib>
#include <list>

struct token_mapper 
{
  sia::token::token 
  operator() (
    sia::db::row_t && row) const
  {
    using namespace sia::token ;
    return token {
        .id       = std::stoull(row.at("id")),
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
    return type_boundaries {
      .begin = std::stoi(row.at("begin")), 
      .end   = std::stoi(row.at("end"))
    } ;
  }
} ;

struct match_track
{
  tokens_it_t begin    ; 
  tokens_it_t cursor   ;
  tokens_it_t end      ;
  bool        matched  ;
  std::string expected ; 
} ;

match_track build_track (
  tokens_it_t const & begin  , 
  tokens_it_t const & cursor ,   
  tokens_it_t const & end    , 
  bool matched = true        , 
  std::string const & expected = {})
{
  return match_track {
    .begin    = begin    ,
    .cursor   = cursor   ,
    .end      = end      ,  
    .matched  = matched  ,
    .expected = expected 
  } ;
}

match_track is_sequence_of (
  match_track const & track, 
  auto && close_test, 
  auto && item_test, 
  auto && sep_test)
{
  if (!close_test(track).matched)
  {
    auto && item_track = item_test(track)     ;
    auto && sep_track  = sep_test(item_track) ; 
  
    return 
      sep_track.matched           ? is_sequence_of(sep_track, close_test, item_test, sep_test) :
      item_test(item_track).matched ? sep_track : 
                                      item_track ;
  }
  else 
  {
    return track ;
  }
}

auto is_of_type (
  match_track const & track, 
  std::string_view          type)
{
  return (*track.cursor).type == type ;
}

auto is_not_end (
  match_track const & track)
{
  return track.cursor != track.end ;
}

match_track is_not_end_and_equal_to (
  match_track const & track,
  std::string_view    type)
{
  if (!track.matched)
  {
    return track ;
  } 
  else
  {
    auto && matched  = is_not_end(track) && is_of_type(track, type)                ;
    auto && expected = matched ? track.expected : std::string(type)     ;
    auto && cursor   = matched ? std::next(track.cursor) : track.cursor ;
    
    return build_track(track.begin, cursor, track.end, matched, expected) ;
  } 
}

auto is_name (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "name") ;
}

auto is_lbracket (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "lbracket") ;
}

auto is_rbracket (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "rbracket") ;
}

auto is_colon (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "colon") ;
}

auto is_comma (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "comma") ;
}

auto is_param (
  match_track const & track)
{
  auto && name_tracked     = is_name(track)         ; 
  auto && colon_tracked    = is_colon(name_tracked) ;
  auto && typename_tracked = is_name(colon_tracked) ;

  return typename_tracked ;
}

auto is_params (
  match_track const & track)
  -> std::decay_t<decltype(track)>
{
  return is_sequence_of(track, is_rbracket, is_param, is_comma) ;
}

auto is_type (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "type") ;
}

auto is_type_declaration (
  match_track const & track) 
{
  auto && type_tracked     = is_type(track)                       ;
  auto && name_tracked     = is_name(type_tracked)                ;
  auto && lbracket_tracked = is_lbracket(name_tracked)            ;
  auto && params_tracked   = is_params(lbracket_tracked) ;
  auto && rbracket_tracked = is_rbracket(params_tracked)          ;
  
  return rbracket_tracked ; 
}

namespace std 
{
  std::string to_string(bool const & v) 
  {
    return v ? "true" : "false" ;
  }
}

type_members_t create_vector_of_type_member (
  auto && ... type_members)
{
  return {std::move(type_members)...} ;
}

auto build_member (
  auto const & begin, 
  auto const & end) 
{ 
  return sia::type::create_type_member(
    (*begin).value, (*std::next(begin, 2)).value) ;
}

type_members_t build_members (
  auto const & begin, 
  auto const & end) 
{
  using namespace sia::type ;

  auto && members = begin != end && (*begin).type == "name" ? 
    create_vector_of_type_member(build_member(begin, end)) : 
    create_vector_of_type_member() ;

  auto && member_end = std::next(begin, 3) ;

  if (member_end != end && (*member_end).type == "comma") 
  {
    auto && member_comma = std::next(member_end) ;
    auto && next_members = build_members(member_comma, end) ;
    std::move(
      next_members.begin(), 
      next_members.end(), 
      std::back_inserter(members)) ;
  }
  
  return members ;
}

auto build_name (
  auto const & begin, 
  auto const & end) 
{
  return (*begin).value ; 
}

auto build_type (
  auto const & begin, 
  auto const & end) 
{
  using namespace sia::type ;

  auto name    = build_name(std::next(begin), end)       ;
  auto members = build_members(std::next(begin, 3), end) ;

  return create_type(name, members) ;
}

auto create_token_query (
  type_boundaries const & boundaries) 
{
  return (boost::format(
    " select * from tkn_token as tk "
    " where tk.id between %d and %d "
    " order by tk.id ")
    % boundaries.begin
    % boundaries.end).str() ;
}

auto select_type_boundaries (
  sia::db::db_t db, 
  limit_t const &  limit, 
  limit_t const &  offset) 
{
   return sia::db::select(db, 
    "select * from stx_types_boundaries", 
    limit, offset, 
    type_boundaries_mapper()) ;
}

auto prepare_type_for_insert (
  sia::type::type const & type)
{
  std::stringstream ss ;

  

  auto && insert_type = 
   (boost::format("insert into stx_type (name, nb_members) values (%s, %d);\n")
     % type.name
     % type.members.size()) ;

  ss << insert_type;

  if (!type.members.empty())   
  { 
    ss << "insert into stx_type_member (name, type, parent) values " ;
    
    std::list<std::string> membersstr ;
    
    boost::range::transform(type.members, std::back_inserter(membersstr), [] (auto&&) {return std::string("");});
    ss << boost::join(membersstr, ",") ;
    ss << ";\n" ;
  }

  return ss.str() ;
}

#include <algorithm>

void insert_type_detection_error (
  sia::db::db_t             db,
  match_track const & track_in_error) 
{ 
  auto query = boost::format(
    " insert into stx_type_error " 
    " (expected_type, token_id) "
    " values ('%s', %d);\n")
    % track_in_error.expected 
    % (*track_in_error.cursor).id ;

  sia::db::ddl(db, query.str()) ;
}

auto insert_type (
  sia::db::db_t             db,
  match_track const & track) 
{
  auto && type = build_type(track.begin, track.end) ;
  auto && type_insert_query = prepare_type_for_insert(type) ;
  return sia::db::ddl(db, type_insert_query) ;
}

auto insert_treated_tokens (
  sia::db::db_t             db,
  match_track const & track)
{
  auto && begin = track.begin ;
  auto && end   = std::prev(track.end) ;
  
  auto query = (boost::format(
    " insert into tkn_treated_token_interval "
    " (begin_id, end_id) values (%d, %d);\n")
    % (*begin).id 
    % (*end).id) ;
  
  return sia::db::ddl(db, query.str()) ;
}


auto populate_stx_types_boundaries (
  sia::db::db_t db)
{
  ddl(db, 
      " insert into stx_types_boundaries (begin, end)"
      "	select begin, end from (                  "
      "   select                                  "
      "		  tk1.id as \"begin\",                  "  
      "		  tk2.id as \"end\",                    "
      " 		min(tk2.id - tk1.id)                  "
      " 	from                                    "
      " 		tkn_token as tk1,                     " 
      " 		tkn_token as tk2                      "
      " 	where                                   "
      " 		tk1.\"type\" = 'type'                 "
      " 	and tk2.\"type\" = 'rbracket'           "
      " 	and tk2.id - tk1.id > 0                 "
      " 	group by tk1.id);                       ") ;
}

int main (int argc, char** argv) 
{
  using namespace sia::db ;
  using namespace sia::script ;
  
  launching_of(argv[0]) ;

  auto db = open_database("lol2.sia.db") ;
  
  if (!is_db_open(db)) 
  {
    sia::log::error("can't open the database") ;
    return EXIT_FAILURE ;
  } 

  const auto limit = sia::config::get_conf_ull("detect_type.chunk.size") ; 
  populate_stx_types_boundaries(db) ;
  const auto offset_max = count(db, "stx_types_boundaries") ;
  auto offset = 0u ; 
   
  while (offset <= offset_max) 
  {  
    begin_transaction(db) ;

    auto && types_bounds = select_type_boundaries(db, limit, offset) ;
    
    for (auto && bounds : types_bounds)
    {    
      auto && token_query = create_token_query(bounds) ;
      auto && tokens      = select(db, token_query, token_mapper()) ;
      auto && begin       = tokens.begin() ;
      auto && end         = tokens.end() ;
      auto && cursor      = tokens.begin() ;
      auto && type_decl_track = is_type_declaration(
        build_track(begin, cursor, end));
      
      if (type_decl_track.matched) 
      {
        insert_type(db, type_decl_track) ;
        insert_treated_tokens(db, type_decl_track) ;
      } 
      else
      {
        insert_type_detection_error(db, type_decl_track) ;
      } 
    }

    end_transaction(db) ;
    offset += limit ; 
  }

  sia::script::stop_of("detect_types") ;
  close_database(db) ;

  std::cout << std::endl ; 
  return EXIT_SUCCESS ;
}
