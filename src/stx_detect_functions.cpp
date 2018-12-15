#include <sia.hpp>
#include <sstream>

using namespace sia::db ;

using tokens_t = std::list<sia::token::token> ;
using tokens_it_t = tokens_t::iterator ; 
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

struct match_track
{
  tokens_it_t begin    ; 
  tokens_it_t cursor   ;
  tokens_it_t end      ;
  bool        matched  ;
  std::string expected ; 
} ;

match_track build_mtrack (
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

bool is_not_end (
  match_track const & track)
{
  return track.cursor != track.end ;
}

bool is_of_type (
  match_track const & track, 
  std::string_view    type)
{
  return (*track.cursor).type == type ;
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
    
    return build_mtrack(track.begin, cursor, track.end, matched, expected) ;
  } 
}

match_track is_fn (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "fn") ;
}

match_track is_name (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "name") ;
}

match_track is_lbracket (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "lbracket") ;
}

match_track is_rbracket (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "rbracket") ;
}

match_track is_colon (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "colon") ;
}

match_track is_comma (
  match_track const & track) 
{
  return is_not_end_and_equal_to(track, "comma") ;
}

match_track is_param (
  match_track const & track)
{
  auto && name_tracked     = is_name(track)         ; 
  auto && colon_tracked    = is_colon(name_tracked) ;
  auto && typename_tracked = is_name(colon_tracked) ;

  return typename_tracked ;
}

match_track is_one_of (
  match_track const & track,
  auto && is_first,
  auto && ... is_nexts)
{
  auto && first_track = is_first(track) ;

  if constexpr (sizeof...(is_nexts) > 0)
    return first_track.matched ? first_track : is_one_of(track, is_nexts...) ;
  else 
    return first_track ;   
}

match_track is_simple_sequence_of (
  match_track const & track, 
  auto && has_again_test, 
  auto && item_test)
{
  if (has_again_test(track))
  {
    auto && item_track = item_test(track) ;
    return item_track.matched ? is_simple_sequence_of(item_track, has_again_test, item_test) : track ;
  }
  else 
  {
    return track ;
  }
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

match_track is_params (
  match_track const & track)
{  
  return is_sequence_of(track, is_rbracket, is_param, is_comma) ;
}

match_track is_signature ( 
  match_track const & track)
{
  auto && fn_track       = is_fn(track)              ; 
  auto && fname_track    = is_name(fn_track)         ;
  auto && lbracket_track = is_lbracket(fname_track)  ;
  auto && params_track   = is_params(lbracket_track) ;
  auto && rbracket_track = is_rbracket(params_track) ;
  auto && colon_track    = is_colon(rbracket_track)  ;
  auto && name2_track    = is_name(colon_track)      ;
  
  return name2_track ;
}

match_track is_lbrace ( 
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "lbrace") ;
}

match_track is_rbrace ( 
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "rbrace") ;
}

match_track is_number (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "number") ;
}

match_track is_arg (
  match_track const & track)
{
  auto && name_track = is_name(track) ;

  return name_track ; //is_one_of(track, is_name, is_number) ;
}  

match_track is_args (
  match_track const & track)
{ 
  auto && args_track = is_sequence_of(track, is_rbracket, is_arg, is_comma) ;

  return args_track ;
}

match_track is_point (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "point") ;
}

match_track is_alias (
  match_track const & track)
{
  return is_not_end_and_equal_to(track, "alias") ;
}

match_track is_simple_function_call (
  match_track const & track) 
{
  auto && lbracket_track  = is_lbracket(track)           ;
  auto && args_track      = is_args(lbracket_track)      ;
  auto && rbracket_track  = is_rbracket(args_track)      ;
  auto && point_track     = is_point(rbracket_track)     ;
  auto && name_track      = is_name(point_track)         ; 
  auto && lbracket2_track = is_lbracket(name_track)      ;
  auto && rbracket2_track = is_rbracket(lbracket2_track) ;

  return rbracket2_track ;
}

match_track is_function_call (
  match_track const & track)
{
  auto && alias_track  = is_alias(track)                      ;
  auto && alname_track = is_name(alias_track)                 ;
  auto && colon_track  = is_colon(alname_track)               ; 
  auto && sfcall_track = is_simple_function_call(colon_track) ;

  return sfcall_track ;
}

bool has_again_alias_to_track (
  match_track const & track)
{
  return (*track.cursor).type == "alias" ; 
}

match_track is_aliases (
  match_track const & track)
{
  return is_simple_sequence_of(track, has_again_alias_to_track, is_function_call) ;
}

match_track is_fbody (
  match_track const & track)
{
  auto && lbrace_track  = is_lbrace(track)         ;
  auto && aliases_track = is_aliases(lbrace_track) ;
  auto && sfcall_track  = is_one_of(aliases_track, 
    is_number, is_name, is_simple_function_call)   ;
  auto && rbrace_track  = is_rbrace(sfcall_track)  ;

  return rbrace_track ;
}

match_track is_function (
  match_track const & track)
{
  auto && signature_track = is_signature(track)       ;
  auto && fbody_track     = is_fbody(signature_track) ;

  return fbody_track ; 
}

struct token_mapper 
{
  sia::token::token 
  operator() (
    row_t && row) const
  {
    using namespace sia::token ;
    return token {
        .id       = std::stoull(row.at("id")),
        .filename = std::move(row.at("filename")),
        .line     = std::stoi(row.at("line")),
        .column   = std::stoi(row.at("column")), 
        .value    = std::move(row.at("value")), 
        .type     = std::move(row.at("type"))
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

#include <vector>

std::vector<auto> concat_v (
  std::vector<auto> const & firsts, 
  std::vector<auto> const & nexts)
{
  // FIXME faire une collection permettant le move de manière fonctionnelle
  std::decay_t<decltype(firsts)> res ;
  res.reserve(firsts.size() + nexts.size()) ;
  res.insert(res.end(), firsts.begin(), firsts.end()) ;
  res.insert(res.end(), nexts.begin(), nexts.end()) ;
  
  return res ;
}

struct function_arg 
{
  std::string value ;
} ; 

struct function_call
{
  std::string               name ;
  std::vector<function_arg> args ;
} ;

struct aliased_function_call
{ 
  std::string   alias ;
  function_call fcall ;
} ;

struct function_param
{
  std::string name ;
  std::string type ;
} ;

#include <variant>

struct function 
{
  std::string                        name    ;
  std::string                        type    ;
  std::vector<function_param>        params  ;  
  std::vector<aliased_function_call> aliases ;
  function_call                      result  ;
} ;

template <typename building_t>
struct build_track
{
  building_t  built  ;
  tokens_it_t cursor ;
} ;

template <typename building_t>
build_track<building_t> build_btrack (
  building_t const & b, 
  tokens_it_t const & cursor)
{
  return build_track<building_t> {
    .built  = b , 
    .cursor = cursor  
  } ;
}

auto jump_one (
  auto const & cursor) 
{
  return std::next(cursor) ;
}

auto jump_over (
  auto const & cursor,
  auto const & type, 
  auto const & ... types)
{
  auto && cursor_type = (*cursor).type ;
  
  if (cursor_type  == type)
  {
    auto && jumped_cursor = jump_one(cursor) ;
    
    if constexpr (sizeof...(types) > 0)
    {
      return jump_over(jumped_cursor, types...) ;
    }
    else
    {
      return jumped_cursor ;
    }
  }
  else
  {
    return cursor ;
  }
}

auto maybe_jump_over (
  auto const & cursor,
  auto const & type, 
  auto const & ... types)
{
  return jump_over(cursor, type, types...) ;
}

build_track<std::string> build_name (
  auto const & begin, 
  auto const & end) 
{
  auto && name   = (*begin).value           ;
  auto && cursor = jump_over(begin, "name") ;

  return build_btrack(name, cursor) ;
}

build_track<std::string> build_alias (
  auto const & begin, 
  auto const & end) 
{
  auto && alias  = (*begin).value           ;
  auto && cursor = jump_over(begin, "name") ;

  return build_btrack(alias, cursor) ;
}


build_track<std::string> build_type (
  auto const & begin,
  auto const & end)
{  
  auto && type   = (*begin).value           ;
  auto && cursor = jump_over(begin, "name") ;

  return build_btrack(type, cursor) ;
}

build_track<function_arg> build_arg (
  auto const & begin, 
  auto const & end)
{
  auto && arg    = function_arg {(*begin).value} ;
  auto && cursor = jump_one(begin)               ; 

  return build_btrack(arg, cursor) ;
}

bool has_again_arg_to_build (
  auto const & begin, 
  auto const & end)
{
  auto && type = (*begin).type ;

  return type == "name" || type == "number" ;
}

using function_args_t = std::vector<function_arg> ;

build_track<function_args_t> build_args (
  auto const & begin, 
  auto const & end) 
{
  auto && jumped_comma = maybe_jump_over(begin, "comma") ;
  
  if (has_again_arg_to_build(jumped_comma, end)) 
  {
    auto && arg_track       = build_arg(jumped_comma, end)          ; 
    auto && args            = function_args_t{arg_track.built}      ;
    auto && next_arg_cursor = jump_one(arg_track.cursor)            ;
    auto && next_args_track = build_args(next_arg_cursor, end)      ;
    auto && all_args        = concat_v(args, next_args_track.built) ;
    
    return build_btrack(all_args, next_args_track.cursor) ;
  }
  else 
  {
    return build_btrack(function_args_t{}, begin) ;
  }
}

build_track<function_call> build_function_call (
  auto const & begin, 
  auto const & end)
{
  auto && jumped_lbracket  = jump_over(begin, "lbracket")                         ;
  auto && args_track       = build_args(jumped_lbracket, end)                     ;
  auto && jumped_rbracket  = jump_over(args_track.cursor, "rbracket")             ;
  auto && jumped_point     = jump_over(jumped_rbracket, "point")                  ;
  auto && name_track       = build_name(jumped_point, end)                        ;
  auto && jumped_lrbracket = jump_over(name_track.cursor, "lbracket", "rbracket") ;
  auto && fcall            = function_call {
    .name = name_track.built , 
    .args = args_track.built
  } ;


  return build_btrack(fcall, jumped_lrbracket) ;
}

bool has_again_alias_to_build (
  auto const & begin, 
  auto const & end)
{
  return (*begin).type == "alias" ;
}

using aliases_fcall_t = std::vector<aliased_function_call> ; 

build_track<aliases_fcall_t> build_aliases (
  auto const & begin,
  auto const & end)
{ 
  if (has_again_alias_to_build(begin, end)) 
  {
    auto && jumped_alias  = jump_over(begin, "alias")              ;
    auto && alias_track   = build_alias(jumped_alias, end)         ;
    auto && jumped_colon  = jump_over(alias_track.cursor, "colon") ;
    auto && fcall_track   = build_function_call(jumped_colon, end) ;
    auto && aliased_fcall = aliased_function_call {
      .alias = alias_track.built , 
      .fcall = fcall_track.built
    } ;
    auto && alfcalls            = aliases_fcall_t{aliased_fcall}         ;
    auto && next_alfcalls_track = build_aliases(fcall_track.cursor, end) ;
    auto && all_alfcalls = concat_v(alfcalls, next_alfcalls_track.built) ;
    
    return build_btrack(all_alfcalls, next_alfcalls_track.cursor) ;
  }
  else
  {
    return build_btrack(aliases_fcall_t{}, begin) ;
  }  
}

build_track<function_param> build_param (
  auto const & begin, 
  auto const & end)
{
  auto && name_track   = build_name(begin, end)                ;
  auto && jumped_colon = jump_over(name_track.cursor, "colon") ;
  auto && type_track   = build_type(jumped_colon, end)         ;
  auto && param        = function_param {
    .name = name_track.built , 
    .type = type_track.built
  } ;

  return build_btrack(param, type_track.cursor) ;
}

bool has_again_param_to_build(
  auto const & begin, 
  auto const & end)
{
  auto && type = (*begin).type ;

  return type == "name" || type == "number" ;
}

using function_params_t = std::vector<function_param> ;

build_track<function_params_t> build_params (
  auto const & begin, 
  auto const & end)
{
  auto && jumped_comma = maybe_jump_over(begin, "comma") ;

  if (has_again_param_to_build(jumped_comma, end)) 
  { 
    auto && param_track       = build_param(jumped_comma, end)                   ; 
    auto && params            = function_params_t{param_track.built}      ;
    auto && next_params_track = build_params(param_track.cursor, end)     ;
    auto && all_params        = concat_v(params, next_params_track.built) ;
    
    return build_btrack(all_params, next_params_track.cursor) ;
  }
  else 
  {
    return build_btrack(function_params_t{}, begin) ;
  }
}

build_track<function_call> build_result (
  auto const & begin, 
  auto const & end)
{
  return build_function_call(begin, end) ;
}

build_track<function> build_function (
  auto const & begin, 
  auto const & end) 
{
  auto && jumped_fn       = jump_over(begin, "fn")                     ;
  auto && name_track      = build_name(jumped_fn, end)                 ;
  auto && jumped_lbracket = jump_over(name_track.cursor, "lbracket")   ;
  auto && params_track    = build_params(jumped_lbracket, end)         ;
  auto && jumped_rbracket = jump_over(params_track.cursor, "rbracket") ;
  auto && jumped_colon    = jump_over(jumped_rbracket, "colon")        ;
  auto && type_track      = build_type(jumped_colon, end)              ;
  auto && jumped_lbrace   = jump_over(type_track.cursor, "lbrace")     ;
  auto && aliases_track   = build_aliases(jumped_lbrace, end)          ;
  auto && result_track    = build_result(aliases_track.cursor, end)    ;
  auto && jumped_rbrace   = jump_over(result_track.cursor, "rbrace")   ;
  auto && func            = function {
    .name    = name_track.built    , 
    .type    = type_track.built    ,
    .params  = params_track.built  ,
    .aliases = aliases_track.built ,
    .result  = result_track.built  
  } ;
  
  return build_btrack(func, jumped_rbrace) ;
}

std::string prepare_function_param (
  function_param const & param, 
  std::string const & fname)
{
  std::stringstream ss ;
  ss << " insert into stx_function_param " 
        " (name, type, parent) values ( " 
     << sia::quote(param.name) << ", " 
     << sia::quote(param.type) << ", " 
     << sia::quote(fname)      << ");\n" ;
  
  return ss.str() ;
}

std::string prepare_function_params (
  auto const & begin, 
  auto const & end, 
  auto const & fname)
{ 
  std::stringstream ss ;
  
  if (begin != end)
  {
    auto && param = *begin                                     ;
    ss << prepare_function_param(param, fname)                 ;
    ss << prepare_function_params(jump_one(begin), end, fname) ;
  }

  return ss.str() ;
}


std::string prepare_aliased_function_call_arg (
  function_arg const & arg, 
  std::string const & alias)
{
  std::stringstream ss ; 
  ss << " insert into stx_function_arg " 
        " (value, parent) values "
    << "(" << sia::quote(arg.value) << ", " 
    << sia::quote(alias) << ");\n";
  
  return ss.str() ;
}

std::string prepare_aliased_function_call_args (
  auto const & begin, 
  auto const & end, 
  std::string const & alias)
{
  std::stringstream ss ; 
  
  if (begin != end)
  {
    auto && arg = *begin                                                  ;
    ss << prepare_aliased_function_call_arg(arg, alias)                   ;
    ss << prepare_aliased_function_call_args(jump_one(begin), end, alias) ;
  }

  return ss.str() ;
}

std::string prepare_aliased_function_call ( 
  aliased_function_call const afcall, 
  std::string const & fname)
{
  auto && args = afcall.fcall.args ;
  std::stringstream ss ; 
  ss << " insert into stx_function_call "
        " (alias, fname, parent) values "
     << "(" << sia::quote(afcall.alias) << ", " 
     << sia::quote(afcall.fcall.name) << ", "
     << sia::quote(fname) << ");\n" ;
  ss << prepare_aliased_function_call_args(args.begin(), args.end(), afcall.alias) ;

  return ss.str() ;
}

std::string prepare_aliases_function_calls (
  auto const & begin, 
  auto const & end, 
  std::string const & fname)
{
  std::stringstream ss ;  
  
  if (begin != end)
  {
    auto && alfcall = (*begin)                                        ;
    ss << prepare_aliased_function_call(alfcall, fname)               ;
    ss << prepare_aliases_function_calls(jump_one(begin), end, fname) ;
  }

  return ss.str() ;  
}

std::string prepare_function_signature (
  function const & fn)
{
  std::stringstream ss ;
  ss << " insert into stx_function " 
        " (name, type) values (    "
     << sia::quote(fn.name) << ", "
     << sia::quote(fn.type) << ");\n" ;
  
  return ss.str() ;
}

std::string prepare_result (
  function_call const & result, 
  std::string const fname) 
{
  auto && args  = result.args        ;
  auto && alias = fname + "__result" ; 
  std::stringstream ss ; 
  ss << " insert into stx_function_call   "
        " (alias, fname, parent) values ( "
     << sia::quote(alias)       << ", "  
     << sia::quote(result.name) << ", "
     << sia::quote(fname)       << ");\n" ;
  ss << prepare_aliased_function_call_args(args.begin(), args.end(), alias) ;

  return ss.str() ;
}

std::string prepare_function_to_insert (
  function const & fn)
{
  std::stringstream ss ;
  ss << prepare_function_signature(fn) 
     << prepare_function_params(fn.params.begin(), fn.params.end(), fn.name) 
     << prepare_aliases_function_calls(fn.aliases.begin(), fn.aliases.end(), fn.name) 
     << prepare_result(fn.result, fn.name) ;
 
  return ss.str() ;
}

auto insert_function (
  db_t db, 
  match_track const & track) 
{
  auto && fn_track   = build_function(track.begin, track.end)     ;
  auto && finsertion = prepare_function_to_insert(fn_track.built) ;
  
  return ddl(db, finsertion) ;
}

auto insert_treated_tokens (
  sia::db::db_t             db,
  match_track const & track)
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
  match_track const & track)
{
  std::stringstream ss ;
  ss << "insert into stx_function_error (expected_type, token_id) values (" 
     << sia::quote(track.expected) << ", " 
     << (*track.cursor).id << ");" ;
  
  sia::db::ddl(db, ss.str()) ;
}

auto populate_stx_functions_boundaries (
  db_t db)
{
  ddl(db, 
      " insert into stx_functions_boundaries (begin, end)"
      " select                                  "
      "	  tk1.id as \"begin\",                  "  
      "	  tk2.id as \"end\"                     "
      " from                                    "
      " 	tkn_token as tk1,                     " 
      " 	tkn_token as tk2                      "
      " where                                   "
      " 	tk1.\"type\" = 'fn'                   "
      " and tk2.\"type\" = 'rbrace'             "
      " and tk2.id > tk1.id                     "
      " group by tk1.id                         "
      " having tk2.id = min(tk2.id)", true) ;
}

auto detect_functions (
  db_t db)
{
  using namespace sia::config ;

  limit_t limit  = get_conf_ull("detect_function.chunk.size") ;
  limit_t offset = 0ull ;
  limit_t cnt  = count(db, "stx_functions_boundaries") ;
  
  while (offset < cnt)
  {
    begin_transaction(db) ;
    auto && fbounds = select_functions_boundaries(db, limit, offset) ; 
    
    for (auto const & fbound : fbounds)
    {
      auto && tokens = select_tokens_from_boundaries(db, fbound) ; 
      auto && function_track = is_function(
        build_mtrack(tokens.begin(), tokens.begin(), tokens.end())) ;
     
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

    offset += fbounds.size() ;
    end_transaction(db) ;
  }

  return false ;
}

int main (int argc, char ** argv)
{
  sia::script::launching_of(argv[0]) ;
  
  auto db = open_database("lol2.sia.db") ;
  ddl(db, "pragma journal_mode = off")   ;
  populate_stx_functions_boundaries(db)  ;
  auto has_error = detect_functions(db)  ; 
  
  sia::script::stop_of(argv[0]) ;
  return has_error ? EXIT_FAILURE : EXIT_SUCCESS ;
}
