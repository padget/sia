#include <sia.hpp>
#include <fstream>
#include <iostream>
#include <map>

using properties_t = std::map<std::string, std::string> ;

auto parse_properties_file (
  std::string const & filename) 
{
  auto file = std::ifstream(filename, std::ios::in) ;
  properties_t properties ;
  
  if (file.is_open())
  {
    auto const & delimiter = "=" ;
    std::string line ;

    while (std::getline(file, line))
    {
      auto delim_position = line.find(delimiter)                     ;
      auto key            = line.substr(0, delim_position)           ;
      auto value          = line.substr(delim_position + 1, line.size()) ;

      if (!key.empty() && !value.empty())
      {
        properties[key] = value ;
      } 
      else 
      {
        sia::log::warn(std::string("the configuration key ") + key + " has no value !") ;
      }
    }
  }
  else 
  {
    sia::log::error(std::string("the file ") + filename + " can't be opened !") ;
  }

  return properties ;
}

int main(int argc, char** argv) 
{
  using namespace sia::db ;

  if (argc < 3)
  {
    sia::log::error("a configuration file must set") ; 
    return EXIT_FAILURE ;
  }

  if (std::string(argv[1]) == "--config")
  {
    auto filename   = argv[2] ;
    auto properties = parse_properties_file(filename) ;    
    auto db         = open_database("sia.config.db") ;

    if (is_db_open(db))
    {
      begin_transaction(db) ;
      drop_table(db, "sia_configuration") ;
      create_table(db, 
        "sia_configuration", 
        column("key", "text", "primary key"), 
        column("value", "text", "not null")) ;
      end_transaction(db) ;

      begin_transaction(db) ;

      for (auto property : properties)
        ddl(db, 
          "insert into sia_configuration (key, value) "
          "values (" + sia::quote(property.first) + ", " + 
          sia::quote(property.second) + ");") ; 

      end_transaction(db) ;

      return EXIT_SUCCESS ;   
    }
    else 
    {
      sia::log::error("can't open database lol2.sia.db") ;
    }
  }
  else 
  {
    sia::log::error("--config parameter expected in first position") ;
    return EXIT_FAILURE ;
  }
}
