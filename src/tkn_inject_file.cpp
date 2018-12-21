#include <sia.hpp>
#include <fstream>
#include <sstream>
#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/combine.hpp>

using lines_t = std::list<std::string> ;
using values_t = std::list<std::string> ;

lines_t read_chunk (auto & file, auto max_size) 
{
  lines_t lines = std::list<std::string>() ;
  auto index = 0ull ;
  auto line  = std::string() ;

  while (index < max_size && std::getline(file, line)) 
  { 
    lines.push_back(line) ;
    index++ ;
  }
  
  return lines ;
}

auto prepare_line_to_be_injected (
  std::string   const & filename, 
  size_t        const & current_line_num)
{ 
  return [=] (auto const & line_with_index) mutable {
    std::string line  ; 
    size_t      index ;
    boost::tie(line, index) = line_with_index ;
    return ( boost::format("('%s', '%s', %d, %d)") % filename % line % (current_line_num + index) % line.size()).str() ;
  } ;
}

std::string prepare_lines_to_be_injected (
  lines_t     const & lines, 
  std::string const & filename, 
  size_t      const & current_line_num)
{
  using namespace boost::range ;
  std::vector<size_t> indexes(lines.size()) ;
  generate(indexes, [index = 0] () mutable {return index++ ;}) ;
 
  values_t values ;
  transform(
    combine(lines, indexes), 
    std::back_inserter(values), 
    prepare_line_to_be_injected(filename, current_line_num)) ;
  
  return (boost::format(
    "insert into tkn_file_lines    "
    "(filename, line, num, length) " 
    "values %s;\n                  ") 
    % boost::join(values, ",")).str() ;
}

auto inject_file (
  sia::db::db_t       db,
  std::string const & filename)
{
  using namespace sia::db ;

  auto file             = std::ifstream(filename, std::ios::in) ;
  auto current_line_num = 1ull ;
  auto const max_size   = sia::config::get_conf_ull("inject_file.chunk.size") ;

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
