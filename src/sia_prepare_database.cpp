#include <sia.hpp>

int main (int argc, char** argv) 
{
  using namespace sia::db ;
  
  auto db = open_database("lol2.sia.db") ;

  if (is_db_open(db))
  {
    begin_transaction(db) ;
    drop_table(db, "tkn_file_lines") ;
    create_table(db, "tkn_file_lines", 
      column("filename", "text", "not null"), 
      column("line", "text", "not null"), 
      column("num", "integer", "not null"), 
      column("length", "integer", "not null")) ;
    drop_table(db, "tkn_token") ;
    create_table(db, "tkn_token", 
      column("id", "integer", "primary key"), 
      column("filename", "text", "not null"), 
      column("line", "integer", "not null"), 
      column("column", "integer", "not null"), 
      column("value", "text", "not null"), 
      column("type", "text", "not null")) ;
    drop_table(db, "stx_type_member") ;
    create_table(db, "stx_type_member", 
      column("id", "integer", "primary key"), 
      column("name", "text", "not null"), 
      column("type", "text", "not null"), 
      column("parent", "text", "not null")) ;
    drop_table(db, "stx_type") ;
    create_table(db, "stx_type", 
      column("id", "integer", "primary key"), 
      column("name", "text", "not null"), 
      column("nb_members", "integer", "not null")) ;
    ddl(db, 
      "drop view if exists stx_types_boundaries;"
      "create view if not exists                "
      "stx_types_boundaries                     "
      "(\"begin\", \"end\", \"distance\")       " 
      "as                                       "
      "	select                                  "
      "		tk1.id as \"begin\",                  "  
      "		tk2.id as \"end\",                    "
      "		min(tk2.id - tk1.id)                  "
      "	from                                    "
      "		tkn_token as tk1,                     " 
      "		tkn_token as tk2                      "
      "	where                                   "
      "		tk1.\"type\" = 'type'                 "
      "	and tk2.\"type\" = 'rbracket'           "
      "	and tk2.id - tk1.id > 0                 "
      "	group by tk1.id;                        ") ;
    drop_table(db, "stx_type_error") ;
    create_table(db, "stx_type_error", 
      column("id", "integer", "primary key"), 
      column("expected_type", "text", "not null"), 
      column("token_id", "integer", "not null")) ;
    drop_table(db, "tkn_not_treated_token") ;
    create_table(db, "tkn_treated_token_interval", 
      column("begin_id", "integer", "not null"), 
      column("end_id", "integer", "not null")) ;
    end_transaction(db) ;
    ddl(db, 
      "insert into                  "
      "  stx_type (name, nb_members)"
      "values                       "
      "  ('int', 0),                "
      "  ('byte', 0),               "
      "  ('long', 0);               ") ;
    close_database(db) ;
    
  
  }
}
