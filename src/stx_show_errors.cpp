#include <sia.hpp>
#include <sstream>

using namespace sia::db ;

using id_t = unsigned long long ;

struct type_errors 
{
  std::string expected_type ;
  id_t        token_id      ;
} ;

struct type_errors_mapper 
{
  type_errors operator() (row_t const & row) const
  {
    return type_errors {
      .expected_type = row.at("expected_type") ,
      .token_id      = std::stoull(row.at("token_id"))
    } ;
  }
} ;

auto select_all_type_errors(db_t db)
{
  std::string query = "select expected_type, token_id from stx_type_error" ;

  return select(db, query, type_errors_mapper()) ;
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

std::string build_token_by_id_query (
  id_t id) 
{
  std::stringstream ss ;
  ss << "select * from tkn_token where id="
     << id << ";\n" ;
  return ss.str() ;
} 

sia::token::token select_token_by_id (
  db_t db, 
  id_t token_id)
{
  auto && query = build_token_by_id_query(token_id) ;
  return select(db, query, token_mapper()).front() ;
} 

struct file_line
{
  std::string filename ;
  std::string line     ; 
  limit_t     num      ;
  limit_t     length   ;
} ;

struct file_line_mapper 
{
  file_line operator() (row_t const & row) const 
  {
    return file_line {
      .filename = row.at("filename")         ,
      .line     = row.at("line")             ,
      .num      = std::stoull(row.at("num")) ,
      .length   = std::stoull(row.at("length")) 
    } ;
  } 
} ;

std::string build_file_line_by_token_query (
  sia::token::token const & tkn)
{
  std::stringstream ss ;
  ss << "select * from tkn_file_lines where filename="
     << sia::quote(tkn.filename) << "and num="
     << tkn.line << ";\n" ;
  
  return ss.str() ;
} 

file_line select_file_line_by_token (
  db_t                      db, 
  sia::token::token const & tkn)
{
  auto && query = build_file_line_by_token_query(tkn) ;
  return select(db, query, file_line_mapper()).front() ;
}

std::string prepare_error_to_show (
  sia::token::token const & tkn, 
  file_line const & line, 
  type_errors const & terror)
{
  std::string indent = std::string("|") + std::string(4, ' ') ;
  
  std::stringstream ss ;
  ss << "syntax error at " << line.filename << ":" << tkn.line << ":" << tkn.column << "\n\n" ;
  ss << indent << std::string(line.length, ' ') << "\n" ;
  ss << indent << line.line << "\n" ;
  ss << indent << std::string(tkn.column-1, ' ') << "^ : expected a " << terror.expected_type << "\n" ;

  return ss.str() ;
} 

void show_type_errors (db_t db)
{
  auto && type_errors = select_all_type_errors(db) ;
  
  for (auto const & error : type_errors) 
  {
    auto && tkn = select_token_by_id(db, error.token_id) ;
    auto && fl = select_file_line_by_token(db, tkn) ; 
    sia::log::error(prepare_error_to_show(tkn, fl, error)) ;
  }
}

void show_function_errors (db_t db) 
{

}

void show_case_function_errors (db_t db)
{

}

int main (int argc, char** argv) 
{
  sia::script::launching_of(argv[0]) ;
  auto db = open_database("lol2.sia.db") ;

  if (is_db_open(db)) 
  {
    show_type_errors(db)          ;
    show_function_errors(db)      ;
    show_case_function_errors(db) ;
  }

  sia::script::stop_of(argv[0]) ;
  return EXIT_SUCCESS ;
}