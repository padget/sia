#include <sia.hpp>
#include <vector>
#include <map>

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
  auto drop_type_table_if_exists (db_t db) 
  {
    return execute_sql_file (db, 
      "") ;
  }

  auto create_type_table_if_not_exists (db_t db) 
  {
    return execute_sql_file(db, 
      "./sql/detect_types/create_type_table_if_not_exists.sql") ;
  }

  auto drop_type_member_table_if_exists (db_t db) 
  {
    return execute_sql_file(db, 
      "./sql/detect_types/drop_type_member_table_if_exists.sql") ;
  }

  auto create_type_member_table_if_not_exists (db_t db)
  {
    return execute_sql_file(db, 
      "./sql/detect_types/create_type_member_table_if_not_exists.sql") ;
  }

  auto insert_ref_values_in_type_table (db_t db) 
  {
    return execute_sql_file(db, 
      "./sql/detect_types/insert_ref_values_in_type_table.sql") ;
  }

  auto prepare_database (db_t db) 
  {
    begin_transaction(db) ;
    drop_type_table_if_exists(db) ;
    create_type_table_if_not_exists(db) ;
    insert_ref_values_in_type_table(db) ;
    drop_type_member_table_if_exists(db) ;
    create_type_member_table_if_not_exists(db) ;
    end_transaction(db) ;
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

  auto select_types_from_tokens(db_t db) 
  {
    // return execute_select_query(db, 
    //   "select                   " 
	  //   "  tk1.id as \"type\",    " 
	  //   "  tk2.id as \"rbracket\"," 
	  //   "  min(tk2.id - tk1.id)   "
    //   "from                     " 
	  //   "  t_token as tk1,        "  
	  //   "  t_token as tk2         "
    //   "where                    " 
	  //   "  tk1.\"type\" = 12      " 
    //   "and tk2.\"type\" = 4     "
    //   "and tk2.id - tk1.id > 0  "
    //   "group by tk1.id          ") ; // TODO: finir ici
  }
}

#include <cstdlib>

int main (int argc, char** argv) 
{
  using namespace sia::db ;
  using namespace sia::script ;
  
  launching_of(argv[0]) ;
  //sia::log::skip_debug() ;

  auto db = open_database("lol2.sia.db") ;
  
  if (db != nullptr) 
  {
    prepare_database(db) ;
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