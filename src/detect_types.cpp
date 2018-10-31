#include <sia.hpp>
#include <vector>

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
    return execute_query (db, 
      "drop table if exists t_type") ;
  }

  auto create_type_table_if_not_exists (db_t db) 
  {
    return execute_query(db, 
      "create table if not exists t_type ("
	    "id integer primary key,            "
	    "name text not null)                ") ;  
  }

  auto drop_type_member_table_if_exists (db_t db) 
  {
    return execute_query(db, 
      "drop table if exists t_type_member") ;
  }

  auto create_type_member_table_if_not_exists (db_t db)
  {
    return execute_query(db, 
      "create table if not exists t_type_member ("
      "id number PRIMARY KEY,                    "
      "name text not null,                       "
      "type integer not null,                    "
      "parent integer not null)                  ") ;
  }

  auto insert_ref_values_in_type_table (db_t db) 
  {
    return execute_query(db, 
      "insert into t_type (name)         " 
      "values ('int'), ('byte'), ('long')") ;
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
}

// select tk1.id, tk2.id from t_token as tk1 , t_token as tk2 where tk1."type"= 12 and tk2."type" = 4 group by tk1.id having min(tk2.id - tk1.id) >= 0;

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