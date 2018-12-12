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

auto is_not_end (
  match_track<auto> const & track)
{
  return track.cursor != track.end ;
}

auto is_of_type (
  match_track<auto> const & track, 
  std::string_view          type)
{
  return (*track.cursor).type == type ;
}

auto is_not_end_and_equal_to (
  match_track<auto> const & track,
  std::string_view          type)
{
  auto && matched = track.matched && is_not_end(track) && is_of_type(track, type) ;
  auto && cursor  = matched ? std::next(track.cursor) : track.cursor ;
  return build_track(track.begin, cursor, track.end, matched) ;
}

auto is_fn (
  match_track<auto> const & track)
{
  return is_not_end_and_equal_to(track, "fn") ;
}

auto is_name (
  match_track<auto> const & track) 
{
  return is_not_end_and_equal_to(track, "name") ;
}

auto is_lbracket (
  match_track<auto> const & track)
{
  return is_not_end_and_equal_to(track, "lbracket") ;
}

auto is_rbracket (
  match_track<auto> const & track) 
{
  return is_not_end_and_equal_to(track, "rbracket") ;
}

auto is_colon (
  match_track<auto> const & track) 
{
  return is_not_end_and_equal_to(track, "colon") ;
}

auto is_comma (
  match_track<auto> const & track) 
{
  return is_not_end_and_equal_to(track, "comma") ;
}

auto is_param (
  match_track<auto> const & track)
{
  auto && name_tracked     = is_name(track)         ; 
  auto && colon_tracked    = is_colon(name_tracked) ;
  auto && typename_tracked = is_name(colon_tracked) ;

  return typename_tracked ;
}

auto is_params (
  match_track<auto> const & track)
  -> std::decay_t<decltype(track)>
{  
  if (!is_rbracket(track).matched) 
  {
    auto && arg_track   = is_param(track)       ;
    auto && comma_track = is_comma(arg_track) ;
    
    return comma_track.matched ? is_params(comma_track) : arg_track ;
  }
  else 
  {
    return track ;
  }
}

auto is_signature ( 
  match_track<auto> const & track)
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

auto is_lbrace ( 
  match_track<auto> const & track)
{
  return is_not_end_and_equal_to(track, "lbrace") ;
}

auto is_rbrace ( 
  match_track<auto> const & track)
{
  return is_not_end_and_equal_to(track, "rbrace") ;
}

auto is_number (
  match_track<auto> const & track)
{
  return is_not_end_and_equal_to(track, "number") ;
}

auto is_expression(match_track<auto> const & track) -> std::decay_t<decltype(track)> ;

auto is_arg (
  match_track<auto> const & track)
{
  return is_expression(track) ;
}  

auto is_args (
  match_track<auto> const & track)
  -> std::decay_t<decltype(track)>
{
  if (!is_rbracket(track).matched) 
  {
    auto && arg_track   = is_arg(track)       ;
    auto && comma_track = is_comma(arg_track) ;
    
    return comma_track.matched ? is_args(comma_track) : arg_track ;
  }
  else 
  {
    return track ;
  }
}

auto is_point (
  match_track<auto> const & track)
{
  return is_not_end_and_equal_to(track, "point") ;
}

auto is_function_call (
  match_track<auto> const & track)
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

auto is_expression (
  match_track<auto> const & track)
  -> std::decay_t<decltype(track)> 
{
  auto && name_track   = track.matched ? is_name(track) : track                        ;
  auto && number_track = name_track.matched ? name_track : is_number(track)            ;
  auto && fcall_track  = number_track.matched ? number_track : is_function_call(track) ;
  
  return fcall_track ;
}

auto is_fbody (
  match_track<auto> const & track)
{
  auto && lbrace_track = is_lbrace(track)            ;
  auto && expr_track   = is_expression(lbrace_track) ;
  auto && rbrace_track = is_rbrace(expr_track)       ; 
  
  return rbrace_track ;
}

auto is_function (
  match_track<auto> const & track)
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
    return (token) {
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

auto next_token_until (
  auto const & begin, 
  auto const & end, 
  auto && cond) 
  -> std::decay_t<decltype(begin)>
{
  return begin != end && cond(*begin) ? begin : next_token_until(std::next(begin), end, cond) ;
}

#include <vector>

struct function_arg 
{
  std::string value ;
} ; 

struct function_call
{ 
  std::string               alias ;
  std::string               fname ;
  std::vector<function_arg> args  ;
} ;

struct function_param
{
  std::string name ;
  std::string type ;
} ;

struct function 
{
  std::string                 name   ;
  std::string                 type   ;
  std::vector<function_param> params ;  
  std::vector<function_call>  body   ;
} ;

std::string build_fname (
  auto const & begin, 
  auto const & end) ;

std::string build_alias (
  auto const & begin, 
  auto const & end) 
{ 
  static auto i = 0ull ;
  std::string alias = std::string("@func") + std::to_string(i) ;
  // TODO implementer la génération dun alias unique.
  return alias ;
}

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

function_arg build_arg (
  auto const & begin, 
  auto const & end)
{
  return function_arg {
    .value = (*begin).value
  } ;
}

std::vector<function_arg> build_fargs (
  auto const & begin, 
  auto const & end) 
{
  using vec = std::vector<function_arg> ;

  if ((*begin).type == "name") 
  {
    if ((*std::next(begin, 1)).type == "comma")
    {
      return concat_v(vec{build_arg(begin, end)}, build_fargs(std::next(begin, 2), end)) ;
    }
    else 
    {
      return {build_arg(begin, end)} ;
    }
  }
  else 
  {
    return {} ;
  }
}

bool until_is_rbracket (
  sia::token::token const & tk) 
{
  return tk.type == "rbracket" ;
}

std::vector<function_call> build_fbody (
  auto const & begin,
  auto const & end)
{
  // TODO faire le body. 
  auto && alias = build_alias(begin, end) ;
  auto && args  = build_fargs(std::next(begin), end) ;
  auto && fname = build_fname(std::next(next_token_until(begin, end, until_is_rbracket), 2), end) ;
   
  return {
    function_call {
      .alias = alias ,
      .fname = fname ,
      .args  = args
    }
  } ;
}

function_param build_fparam (
  auto const & begin, 
  auto const & end)
{
  return function_param {
    .name = (*begin).value, 
    .type = (*std::next(begin, 2)).value
  } ;
}

std::vector<function_param> build_fparams (
  auto const & begin, 
  auto const & end)
{
  using vec = std::vector<function_param> ;

  if ((*begin).type == "name") 
  {
    if ((*std::next(begin, 3)).type == "comma")
    {
      return concat_v(vec{build_fparam(begin, end)}, build_fparams(std::next(begin, 4), end)) ;
    }
    else 
    {
      return {build_fparam(begin, end)} ;
    }
  }
  else 
  {
    return {} ;
  }
}

std::string build_ftype (
  auto const & begin,
  auto const & end)
{
  return (*begin).value ;
}

std::string build_fname (
  auto const & begin, 
  auto const & end) 
{
  return (*begin).value ;
}

bool until_is_lbrace (
  sia::token::token const & tk) 
{
  return tk.type == "lbrace" ;
}

function build_function (
  auto const & begin, 
  auto const & end) 
{
  auto && fname  = build_fname(std::next(begin), end)   ;
  auto && params = build_fparams(std::next(begin, 2), end) ;
  auto && type   = build_ftype(std::next(next_token_until(begin, end, until_is_rbracket), 2), end)   ;
  auto && body   = build_fbody(std::next(next_token_until(begin, end, until_is_lbrace)), end) ;
  
  return function {
    .name   = fname  , 
    .type   = type   ,
    .params = params ,
    .body   = body
  } ;
}

std::string join (
  auto const & collection,
  auto const & mapper,
  auto const & separator,
  auto const & ... mapper_args)
{
  std::stringstream ss ;
  auto const size  = collection.size() ;
  auto       index = 0ull ;


  for (auto const & item : collection)
  {
    ss << mapper(item, mapper_args...) ;

    if (index < size - 1)
      ss << separator ;

    index++ ;
  }

  return ss.str() ;
}

std::string param_to_values (
  function_param const & param, 
  std::string const & parent_name) 
{
  std::stringstream ss ;
  ss << "(" 
     << sia::quote(param.name) << ", " 
     << sia::quote(param.type) << ", " 
     << sia::quote(parent_name) << ")" ;
  
  return ss.str() ;
}

std::string prepare_function_params_to_insert (
  function const & f)
{
  std::stringstream ss ;
  
  if (!f.params.empty())
  {
    ss << "insert into stx_function_param (name, type, parent) values "
       << join(f.params, &param_to_values, ",", f.name)
       << ";\n" ;
  }

  return ss.str() ;
}

std::string function_call_to_values (
  function_call const & fcall, 
  std::string const &   parent_name) 
{
  std::stringstream ss ;
  ss << "(" 
     << sia::quote(fcall.alias) << ", " 
     << sia::quote(fcall.fname) << ", " 
     << sia::quote(parent_name) << ")" ;

  return ss.str() ;
}

std::string function_call_arg_to_values (
  function_arg const & arg, 
  std::string const & parent_name) 
{
  std::stringstream ss ;
  ss << "(" << sia::quote(arg.value) << ", " << sia::quote(parent_name) << ")" ; 

  return ss.str() ;
}

std::string prepare_function_body_to_insert (
  function const & f)
{
  std::stringstream ss ;
  
  if (!f.body.empty()) 
  {
    ss << "insert into stx_function_call (alias, fname, parent) values "
       << join(f.body, &function_call_to_values, ",", f.name) 
       << ";\n" ;

    for (auto const & fcall : f.body)
    {
      if (!fcall.args.empty()) 
      {
        ss << "insert into stx_function_arg (value, parent) values " ;
        ss << join(fcall.args, &function_call_arg_to_values, ",", f.name) ;
        ss << ";\n" ;
      }
    }
  }

  return ss.str() ;  
}

std::string prepare_function_to_insert (
  function const & f)
{
  std::stringstream ss ;
  ss << "insert into stx_function (name, type) values ("
     << sia::quote(f.name) << ", " << sia::quote(f.type) << ");\n" 
     << prepare_function_params_to_insert(f) 
     << prepare_function_body_to_insert(f) ;

  return ss.str() ;
}

auto insert_function (
  db_t db, 
  match_track<auto> const & track) 
{
  auto && f          = build_function(track.begin, track.end) ;
  auto && finsertion = prepare_function_to_insert(f) ;
  
  std::cout << finsertion << std::endl ; 
  
  return ddl(db, finsertion) ;
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
          build_track(tokens.begin(), tokens.begin(), tokens.end())) ;
     
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

    end_transaction(db) ;
    offset += fbounds.size() ;
  }

  return false ;
}

int main (int argc, char ** argv)
{
  sia::script::launching_of(argv[0]) ;
  
  auto db        = open_database("lol2.sia.db") ;
  ddl(db, "pragma journal_mode = off") ;
  populate_stx_functions_boundaries(db)         ;
  auto has_error = detect_functions(db)         ; 
  
  sia::script::stop_of(argv[0]) ;
  return has_error ? EXIT_FAILURE : EXIT_SUCCESS ;
}
