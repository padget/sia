#ifndef __include_sia_hpp__
#  define __include_sia_hpp__

#  include <sqlite3.h>
#  include <sstream>
#  include <fstream> 
#  include <string>
#  include <iostream>
#  include <map>
#  include <list> 
#  include <chrono>

namespace sia  
{
  auto const endl = '\n' ; 

  auto quote (const auto & str) 
  {
    auto ss = std::stringstream() ;
    ss << '\'' ;
    ss << str  ;
    ss << '\'' ;
    return ss.str() ;
  }
}

namespace sia::log 
{
  auto skip_debug_value = false ;
  auto skip_info_value = false ;
  auto skip_warn_value = false ;
  auto skip_error_value = false ;
  auto skip_fatal_value = false ;   

  auto skip_debug () 
  {
    skip_debug_value = true ;
  }

  auto skip_info () 
  {
    skip_debug() ;
    skip_info_value = true ;
  }

  auto skip_warn ()
  {
    skip_info() ;
    skip_warn_value = true ;
  }

  auto skip_error ()
  {
    skip_warn() ;
    skip_error_value = true ;
  }

  auto skip_fatal ()
  {
    skip_error() ;
    skip_error_value = true ;
  }

  auto log (std::string const & lvl, 
            std::string const & message) 
  {
    std::cout << "[" << lvl << "] : " << message << endl ; 
  }

  auto debug (std::string const & message) 
  {
    if (!skip_debug_value) 
    {
      log("debug", message) ;
    }
  }

  auto info (std::string const & message) 
  {
    if (!skip_info_value) 
    {
      log("info", message) ;
    }
  }

  auto warn (std::string const & message) 
  {
    if (!skip_warn_value) 
    {
      log("warn", message) ;
    }
  }

  auto error (std::string const & message) 
  {
    if (!skip_error_value)
    {
      log("error", message) ;
    }
  }

  auto fatal (std::string const & message) 
  {
    if (!skip_fatal_value)
    {
      log("fatal", std::string("/!\\") + message + std::string("/!\\")) ;
    }
  }

  auto important (std::string const & message) 
  {
    log("important", message) ;
  }
}

namespace sia::token 
{


  struct token 
  {
    using id_t = unsigned long long ;

    id_t        id       ;
    std::string filename ;
    int         line     ; 
    int         column   ; 
    std::string value    ; 
    std::string type     ;
  } ; 

  auto create(const std::string& filename, 
              int line, int column, 
              const std::string& value, 
              std::string type) 
  {
    return (token) 
    {
      .filename = filename,
      .line     = line, 
      .column   = column, 
      .value    = value, 
      .type     = type
    } ;
  }
}

namespace sia::db 
{
  using db_t = sqlite3*;
  using select_callback_t = int(void *, int, char **, char **) ;
  
  
  auto open_database (std::string const & db_name) 
  {
    db_t db ;
    sqlite3_open(db_name.c_str(), &db) ;
    return db ;
  }

  auto is_db_open(db_t db) 
  {
    return db != nullptr ;
  }

  auto close_database(db_t db) 
  {
    return sqlite3_close(db) ;
  }

  std::map<std::string, std::string> 
  to_map (
    int    nb_columns, 
    char** values, 
    char** columns)
  {
    std::map<std::string, std::string> results ;

    for (int i = 0 ; i < nb_columns ; ++i) 
    {
      results[columns[i]] = values[i] ;
    }

    return results ;
  }

  using row_t    = std::map<std::string, std::string> ;

  auto store_select_result_callback (
    auto const & mapper)
  {
    using mapper_t = std::decay_t<decltype(mapper)> ;
    using item_t   = std::decay_t<decltype(mapper(std::declval<row_t&&>()))> ;
    
    return [] (
      void *  results, 
      int     nb_columns, 
      char ** values, 
      char ** columns) 
    {
      constexpr mapper_t mapper ;

      auto && result = to_map(nb_columns, values, columns) ;
      auto && obj    = mapper(std::move(result)) ;

      static_cast<std::list<item_t>*>(results)->push_back(std::move(obj)) ;
      return 0 ;
    } ;
  }

  auto log_on_error (
    bool    test, 
    auto && error_message_buffer, 
    auto && query) 
  {
    if (test) 
    {
      sia::log::error(
        std::string() + 
        "an error during query : '" + 
        query.c_str() + "' : " + 
        error_message_buffer) ;    
    }
  }

  auto select (
    db_t                db, 
    std::string const & query, 
    auto &&             mapper)  
  {   
    using item_t = std::decay_t<decltype(mapper(std::declval<row_t&&>()))> ;

    std::list<item_t> results ;
    
    if (!is_db_open(db)) 
    {
      return results ;
    }

    char* error = nullptr ;
    auto rc = sqlite3_exec(
      db, query.c_str(), 
      store_select_result_callback(mapper), 
      static_cast<void*>(&results), 
      &error) ;
    
    log_on_error(rc != SQLITE_OK, error, query) ;
    sqlite3_free(error) ;
    return results ;
  }

  using limit_t = std::size_t ;

  auto select (
    db_t db, 
    std::string const & query,
    limit_t limit, 
    limit_t offset, 
    auto && mapper)
  {
    auto limit_clause   = std::string(" limit ") + std::to_string(limit) ;
    auto offset_clause = std::string(" offset ") + std::to_string(offset) ;  
    return select(db, query + limit_clause + offset_clause, mapper) ;
  }

  auto ddl (
    db_t db, 
    std::string const & query, 
    bool print_time = false) 
  {
    auto t1 = std::chrono::high_resolution_clock::now() ;
    auto nb_impacted_lines = 0u ;
    
    if (!is_db_open(db)) 
    {
      return nb_impacted_lines ;
    }

    char* error = nullptr ;
    auto rc = sqlite3_exec(
      db, query.c_str(), 
      nullptr, nullptr, 
      &error) ;
    
    nb_impacted_lines = rc == SQLITE_OK ? 
      sqlite3_changes(db) : 0u ;
    
    log_on_error(rc != SQLITE_OK, error, query) ;
    sqlite3_free(error) ;
    auto t2 = std::chrono::high_resolution_clock::now() ;
    
    if (print_time) 
      sia::log::debug(std::string("query time duration : ") + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()) + " s" ) ;
    return nb_impacted_lines ;
  }

  struct count_mapper
  {
    auto operator() (row_t && row) const
    {
      return std::stoi(row.at("c")) ;
    }
  } ;

  auto count (
    db_t                db, 
    std::string const & table)
  {
    auto count_query = std::string("select count(ROWID) as c from ") + table ;
    return select(db, count_query, count_mapper()).front() ;
  }

  auto equal (
    std::string const & colname, 
    std::string const & value)
  {
    return colname + " = " + value ;
  }

  auto count (
    db_t                db, 
    std::string const & table, 
    std::string const & condition)
  {
    auto count_query = 
      std::string("select count(*) as c from ") + table 
      + " where " + condition ;
    return select(db, count_query, count_mapper()).front() ;
  }

  auto column (
    std::string const & column_name, 
    std::string const & column_type, 
    std::string const & column_options)
  {
    std::stringstream ss ;
    ss << column_name << " " 
       << column_type << " "
       << column_options ;
    return ss.str() ;
  }

  auto columns (
    auto const & column,
    auto const & ... other) 
  {
    std::stringstream ss ;
    ss << column ;

    if constexpr (sizeof...(other) >= 1) 
    {
      ss << ", \n" ;
      ss << columns (other...) ;
    }

    return ss.str() ;
  }

  auto create_table (
    db_t                db,
    std::string const & table_name, 
    auto const & ...    column) 
  {
    std::stringstream ss ;
    ss << "create table if not exists " << table_name 
       <<  "( " << columns(column...) << ");" ; 
    sia::log::debug(ss.str()) ;
    return ddl(db, ss.str()) ;
  }

  auto drop_table(
    db_t                db, 
    std::string const & table_name)
  {
    std::stringstream ss ;
    ss << "drop table if exists " << table_name ;
    sia::log::debug(ss.str()) ;
    return ddl(db, ss.str()) ;
  }

  
  auto begin_transaction(db_t db) 
  {
    sia::log::debug("begin transaction") ;
    return ddl(db, "begin transaction") ;
  }

  auto end_transaction(db_t db) 
  {
    sia::log::debug("end transaction") ;
    return ddl(db, "commit") ;
  }

  auto rollback_transaction(db_t db) 
  {
    sia::log::debug("rollback transaction") ;
    return ddl(db, "rollback") ;
  }
}


namespace sia::script 
{
  void launching_of (std::string const & app_name)
  {
    sia::log::important(app_name + " launching...") ;
  }

  void interruption_of (std::string const & app_name) 
  {
    sia::log::important(app_name + " interrupted") ;
  }

  void stop_of (std::string const & app_name) 
  {
    sia::log::important(app_name + " quitted") ;
  }
}

namespace sia::config
{
  struct value_config_mapper
  {
    auto operator()(sia::db::row_t const & row) const
    {
      return row.at("value") ;
    }
  } ;

  auto get_conf (
    std::string const & key)
  {
    using namespace sia::db ;
    auto db = open_database("sia.config.db") ;
    
    if (is_db_open(db))
    {
      auto values = select(db, 
        std::string("select value from sia_configuration where key = ") + sia::quote(key), 
        value_config_mapper()) ;
      close_database(db) ;

      if (values.size() == 1)
      {
        return values.front() ;
      }
      else 
      {
        sia::log::error(std::string("can't find a value in sia.config.db for the key ") + key) ;
        return std::string() ;
      }
    }
    else 
    {
      sia::log::error("can't open the sia.config.db") ;
      return std::string() ;
    }
  }

  auto get_conf_ull (
    std::string const & key) 
  {
    return std::stoull(get_conf(key)) ;
  }
}

#endif
