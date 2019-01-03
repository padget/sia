# coding=utf-8

import sqlite3

def drop_table(cursor, table_name):
  query = f'drop table if exists {table_name}'
  conn.execute(query)

def create_table(cursor, table_name, columns):
  columns_str = ', '.join([' '.join(column) for (column) in columns])
  query       = f'create table if not exists {table_name} ({columns_str})'
  conn.execute(query)

if __name__ == '__main__':
  print("début péparation base de données")

  filename    = 'lol2.sia'
  db_filename = '{}.db'.format(filename)
  conn = sqlite3.connect(db_filename)
  cursor = conn.cursor()

  drop_table(cursor, 'tkn_file_lines')
  create_table(cursor, 'tkn_file_lines', 
    [("filename", "text", "not null"), 
         ("line", "text", "not null"), 
         ("num", "integer", "not null"), 
         ("length", "integer", "not null")])

  drop_table(cursor, "tkn_token") 
  create_table(cursor, "tkn_token", 
    [("id", "integer", "primary key"), 
    ("filename", "text", "not null"), 
    ("line", "integer", "not null"), 
    ("column", "integer", "not null"), 
    ("value", "text", "not null"), 
    ("type", "text", "not null")]) 
  
  drop_table(cursor, "stx_type_member") 
  create_table(cursor, "stx_type_member", 
    [("id", "integer", "primary key"), 
    ("name", "text", "not null"), 
    ("type", "text", "not null"), 
    ("parent", "text", "not null")]) 
  
  drop_table(cursor, "stx_type") 
  create_table(cursor, "stx_type", 
    [("id", "integer", "primary key"), 
    ("name", "text", "not null"), 
    ("nb_members", "integer", "not null")]) 
  
  drop_table(cursor, "stx_function") 
  create_table(cursor, "stx_function", 
    [("id", "integer", "primary key"), 
    ("name", "text", "not null"), 
    ("type", "text", "not null")]) 
  
  drop_table(cursor, "stx_function_param") 
  create_table(cursor, "stx_function_param", 
    [("id", "integer", "primary key"), 
    ("name", "text", "not null"), 
    ("type", "text", "not null"), 
    ("parent", "text", "not null")]) 

  drop_table(cursor, "stx_function_call") 
  create_table(cursor, "stx_function_call", 
    [("id", "integer", "primary key"),
    ("alias", "text", "not null"), 
    ("fname", "text", "not null"), 
    ("parent", "text", "not null")]) 

  drop_table(cursor, "stx_function_arg") 
  create_table(cursor, "stx_function_arg", 
    [("id", "integer", "primary key"), 
    ("value", "text", "not null"), 
    ("parent", "text", "not null")]) 

  drop_table(cursor, "stx_case_function") 
  create_table(cursor, "stx_case_function", 
    [("id", "integer", "primary key"), 
    ("name", "text", "not null")]) 
  
  drop_table(cursor, "stx_case_function_call") 
  create_table(cursor, "stx_case_function_call", 
    [("id", "integer", "primary key"),
    ("alias", "text", "not null"), 
    ("fname", "text", "not null"), 
    ("parent", "text", "not null")]) 

  drop_table(cursor, "stx_case_function_arg") 
  create_table(cursor, "stx_case_function_arg", 
    [("id", "integer", "primary key"), 
    ("value", "text", "not null"), 
    ("parent", "text", "not null")]) 

  drop_table(cursor, "stx_types_boundaries") 
  create_table(cursor, "stx_types_boundaries", 
    [("begin", "integer", "not null"), 
    ("end", "integer", "not null")]) 
  
  drop_table(cursor, "stx_functions_boundaries") 
  create_table(cursor, "stx_functions_boundaries", 
    [("begin", "integer", "not null"), 
    ("end", "integer", "not null")]) 

  drop_table(cursor, "stx_function_case_boundaries") 
  create_table(cursor, "stx_function_case_boundaries", 
    [("begin", "integer", "not null"), 
    ("end", "integer", "not null")]) 
  
  drop_table(cursor, "stx_type_error") 
  create_table(cursor, "stx_type_error", 
    [("id", "integer", "primary key"), 
    ("expected_type", "text", "not null"), 
    ("token_id", "integer", "not null")]) 
  
  drop_table(cursor, "stx_function_error") 
  create_table(cursor, "stx_function_error", 
    [("id", "integer", "primary key"), 
    ("expected_type", "text", "not null"), 
    ("token_id", "integer", "not null")]) 
  
  drop_table(cursor, "stx_case_function_error") 
  create_table(cursor, "stx_case_function_error", 
    [("id", "integer", "primary key"), 
    ("expected_type", "text", "not null"), 
    ("token_id", "integer", "not null")]) 

  drop_table(cursor, "tkn_not_treated_token") 
  create_table(cursor, "tkn_treated_token_interval", 
    [("begin_id", "integer", "not null"), 
    ("end_id", "integer", "not null")]) 

  conn.executescript(
    '''insert into                  
        stx_type (name, nb_members)
      values                       
        ('int', 0),                
        ('byte', 0),               
        ('long', 0);               ''')
    
  conn.commit()
  conn.close()
  
  print('fin préparation base de données')