#include <sia.hpp>
#include <fstream>
#include <sstream>

auto read_chunk (auto & file, auto max_size) 
{
  auto lines = std::list<std::string>() ;
  auto index = 0ull ;
  auto line  = std::string() ;

  while (index < max_size && std::getline(file, line)) 
  { 
    lines.push_back(line) ;
    index++ ;
  }
  
  return lines ;
}

auto prepare_lines_to_be_injected (
  auto && lines, 
  std::string const & filename, 
  auto const & current_line_num)
{
  std::stringstream ss ;
  auto index = 0u ;
  ss << "insert into tkn_file_lines (filename, line, num, length) values " ;
  
  for (auto && line : lines)
  {
    ss << "(" 
       << sia::quote(filename) << ", " 
       << sia::quote(line) << ", "
       << (current_line_num + index) << ", " 
       << line.size() << ")" ;

    if (index <= lines.size() - 2)
    {
      ss << ", " ;
    }

    index++ ;
  }

  ss << ";" ;
  return ss.str() ;
}

auto inject_file (
  sia::db::db_t       db,
  std::string const & filename)
{
  using namespace sia::db ;

  auto file             = std::ifstream(filename, std::ios::in) ;
  auto current_line_num = 1ull ;
  auto const max_size   = 1000ull ;

  decltype(read_chunk(file, max_size)) lines ;

  if (file.is_open())
  {
     while (!(lines = std::move(read_chunk(file, max_size))).empty()) 
    {
      begin_transaction(db) ;
      ddl(db, prepare_lines_to_be_injected(lines, filename, current_line_num)) ;
      end_transaction(db) ;
      current_line_num += lines.size() ; 
    }
  }
}

int main (int argc, char** argv)
{
  using namespace sia::db ;
  sia::script::launching_of(argv[0]) ;
  auto db = open_database("lol2.sia.db") ;

  if (is_db_open(db)) 
  {
    inject_file(db, "lol2.sia") ;
    sia::script::stop_of(argv[0]) ;
    return EXIT_SUCCESS ;
  } 
  else 
  {
    sia::script::interruption_of(argv[0]) ;
    return EXIT_FAILURE ;
  }
}