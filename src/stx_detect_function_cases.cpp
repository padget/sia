#include <sia.hpp>
#include <string>

using namespace sia::db ;



auto select_chunk_of_boundaries (
  db_t db, 
  limit_t const limit, 
  limit_t const offset)
{
  auto && select_query      = build_select_boundaries() ;
  auto && boundaries_mapper = fcase_boundaries_mapper() ;
  
  return select(db, select_query, limit, offset, boundaries_mapper) ;
}


void for_each_boundaries (db_t db)
{
  limit_t const limit = sia::config::get_conf_ull("detect_function_cases.chunk.size") ;
  limit_t const count = count(db, "stx_function_case_boundaries")
  limit_t offset      = 0ull ;

  while (offset <= count)
  {
    auto && boundaries = select_chunk_of_boundaries(db, count, offset) ;
    
    for (auto const & bound : boundaries)
    {

    }

    offset += boundaries.size() ;
  }
}


std::string build_fcases_boundaries_query ()
{
  return 
    " insert into stx_function_case_boundaries " 
    "(begin, end)                              "
    " select                                   "
    "	  tk1.id as \"begin\",                   "  
    "	  tk2.id as \"end\"                      "
    " from                                     "
    " 	tkn_token as tk1,                      "  
    " 	tkn_token as tk2                       "
    " where                                    "
    " 	tk1.\"type\" = 'fn'                    "
    " and tk2.\"type\" = 'rbrace'              "
    " and tk2.id > tk1.id                      "
    " group by tk1.id                          "
    " having tk2.id = min(tk2.id);             " ;
}

void detect_function_cases_boundaries (db_t db)
{
  auto && boundaries_query = build_fcases_boundaries_query() ;
  ddl(db, boundaries_query) ;
}

void detect_function_cases (db_t db)
{
  detect_function_cases_boundaries(db) ;
  for_each_boundaries(db) ;
}

int main (int argc, char** argv)
{
  auto && db = open_database("lol2.sia.db") ;
  
  if (is_db_open(db))
  {
    detect_function_cases(db) ;
  }

  return EXIT_SUCCESS ;
}    