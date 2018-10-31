#ifndef __include_sia_hpp__
#  define __include_sia_hpp__

#  include <sqlite3.h>
#  include <sstream>
#  include <string>
#  include <iostream>



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

  auto log (std::string const & lvl, std::string const & message) 
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
      log("fatal", message) ;
    }
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
    char* error_message_buffer = nullptr ;
    sqlite3_exec(
      db, query.data(), nullptr, 
      nullptr, &error_message_buffer) ;
    sqlite3_free(error_message_buffer) ;
    sia::log::debug(std::string("execution de la requete : ") + query) ;
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
    return execute_query(db, "begin transaction") ;
  }

  auto end_transaction(db_t db) 
  {
    return execute_query(db, "commit") ;
  }

  auto open_database (const std::string& db_name) 
  {
    db_t db ;
    sqlite3_open(db_name.c_str(), &db) ;
    return db ;
  }

  auto close_database(db_t db) 
  {
    return sqlite3_close(db) ;
  }
}


namespace sia::script 
{
  void launching_of (std::string const & app_name)
  {
    sia::log::info(app_name + " lauching...") ;
  }
}

#endif