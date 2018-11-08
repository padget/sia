#include <sia.hpp>


int main (int argc, char const **argv)
{
  auto && app_name = argv[0] ;
  sia::script::launching_of(app_name) ;
  sia::log::skip_debug() ;
  
  auto db = sia::db::open_database("lol2.sia.db") ;
  
  if (db != nullptr)
  {
    sia::db::execute_transactional_sql_files(db,
      "./sql/low_syntax_validation/validate.sql") ;
  }
  else 
  {
    sia::log::fatal("lol2.sia.db doesn't exist !!") ;
    sia::script::interruption_of(app_name) ;
    return EXIT_FAILURE ;
  }

  sia::script::stop_of(app_name) ;
  return EXIT_SUCCESS ;
}
