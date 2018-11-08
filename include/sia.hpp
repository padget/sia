#ifndef __include_sia_hpp__
#  define __include_sia_hpp__

#  include <sqlite3.h>
#  include <sstream>
#  include <fstream> 
#  include <string>
#  include <iostream>
#  include <map>
#  include <list> 

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
  using select_callback_t = 
    int(void *, int, char **, char **) ;
  
  auto execute_query (
    db_t               db, 
    const std::string& query) 
  {
    sia::log::debug(std::string("execution de la requete : ") + query) ;
    char* error_message_buffer = nullptr ;
    auto result = sqlite3_exec(
      db, query.data(), nullptr, 
      nullptr, &error_message_buffer) ;
    if (result != SQLITE_OK || error_message_buffer != nullptr) 
    {
      sia::log::error(error_message_buffer) ;
      sqlite3_free(error_message_buffer) ;
    }
    else
    {
      sia::log::debug("requete executee avec succes") ;
    } 

    return result ; 
  }

  auto execute_select_query (
    db_t               db, 
    const std::string& query, 
    select_callback_t  callback, 
    void*              data_buffer = nullptr) 
  {
    char* error_message_buffer = nullptr ;
    sqlite3_exec(
      db, query.data(), callback, 
      data_buffer, &error_message_buffer) ;

    if (error_message_buffer != nullptr) 
    {
      std::cout << error_message_buffer << std::endl ; 
    }

    sqlite3_free(error_message_buffer) ;
  }

  auto begin_transaction(db_t db) 
  {
    sia::log::debug("begin transaction") ;
    return execute_query(db, "begin transaction") ;
  }

  auto end_transaction(db_t db) 
  {
    sia::log::debug("end transaction") ;
    return execute_query(db, "commit") ;
  }

  auto rollback_transaction(db_t db) 
  {
    sia::log::debug("rollback transaction") ;
    return execute_query(db, "rollback") ;
  }

  auto execute_transactional(db_t db, auto && executor)
  {
    begin_transaction(db) ;
    
    if (executor(db) == SQLITE_OK) 
    {
      end_transaction(db) ;
      return SQLITE_OK ;
    }
    else 
    {
      rollback_transaction(db) ;
      return SQLITE_ABORT ;
    }
  }

  auto execute_transactional_query (
    db_t                db, 
    std::string const & query) 
  {
    return execute_transactional(db, [&query] (db_t db) {
      return execute_query(db, query) ; 
    }) ; 
  }

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

  auto execute_sql_file (db_t db, std::string const & filename) 
  {
    sia::log::info("execution fichier " + filename) ;
    using isbuff_t = std::istreambuf_iterator<char> ;
    
    std::fstream sql_stream(filename.data(), std::ios::in) ;
    auto && sql = sql_stream.is_open() ? 
      std::string(isbuff_t(sql_stream), isbuff_t()) :
      std::string() ;

    if (sql.empty()) 
    {
      sia::log::error("le fichier " + filename + " n'existe pas ou bien est vide") ;
      return SQLITE_ABORT ; 
    }

    return execute_query(db, sql) ;
  }

  auto execute_transactional_sql_file (
    db_t                db, 
    std::string const & filename) 
  {
    return execute_transactional(db, [&filename] (db_t db) {
      return execute_sql_file(db, filename) ;
    }) ;
  }

  auto execute_sql_files (
    db_t                db, 
    std::string const & filename, 
    auto const & ...    filenames) 
  {
    auto result = execute_sql_file(db, filename) ;

    if (result == SQLITE_OK) 
    {
      if constexpr (sizeof...(filenames) > 0)
      {
        return execute_sql_files(db, filenames...) ;
      } 
    } 

    return result ; 
  }
  
  auto execute_transactional_sql_files (
    db_t                db, 
    std::string const & filename, 
    auto const & ...    filenames) 
  {
    return execute_transactional(db, [&filename, &filenames...] (db_t db) {
      return execute_sql_files(db, filename, filenames...) ;
    }) ;
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
    std::string const & query) 
  {
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
    return nb_impacted_lines ;
  }

  struct count_mapper
  {
    auto operator() (row_t && row) const
    {
      return std::stoi(row.at("c")) ;
    }
  } ;

  auto count (db_t db, std::string const & table)
  {
    auto count_query = std::string("select count(*) as c from ") + table ;
    return select(db, count_query, count_mapper()).front() ;
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

#endif