// Compute a unbinned distribution from a set of raw data.
// A column may be specified by arguments
// Compilation [requires the dev boost package]: 
// g++ -o3 -W -Wall -Wextra -pedantic -std=c++0x discrete_distribution.cpp -lboost_program_options -o discrete_distribution
#include <cstdlib> 
#include <iostream>
#include <iomanip>
#include <fstream> 
#include <map> 
#include <set> 
// option parser
#include <boost/program_options.hpp>
/* ALIASES */
namespace po = boost::program_options;
// prototypes
void print_line(unsigned int _lenght, std::ostream& _os);

int main(int argc, char const *argv[]) {
  /****************************************************************/
  /*  Parse arguments with boost::program_options                 */
  /****************************************************************/
  // variables declaration
  std::string input_path;
  unsigned int columnn_idx=0;
  unsigned int precision=8;
  double tolerance=1e-10;

  // boost::po declarations
  po::options_description description(" Options");
  description.add_options()
  ("input_path,i",boost::program_options::value<std::string>(&input_path)->default_value(""),
    "Path to the edge list file.")
  ("columnn_idx,c",boost::program_options::value<unsigned int>(&columnn_idx),
    "Column index of the raw data (starting from column 0).")
  ("precision,p",boost::program_options::value<unsigned int>(&precision),
    "Precision of the output.")
  ("are_int" ,"Key values are integer.")
  ("ignore_null","Ignore null entries (with given tolerance).")
  ("tolerance,t",boost::program_options::value<double>(&tolerance),
    "Tolerance on null entries.")
  ("help,h" ,"Produce this help message.")
  ;
  po::variables_map var_map;
  po::store(po::parse_command_line(argc,argv,description), var_map);
  po::notify(var_map);
  if (argc==1 || var_map.count("help")>0) {
    print_line(80,std::clog);
    std::clog << "Discrete distribution from raw data.\n" << " Author: J.-G. Young.\n";
    print_line(80,std::clog);
    std::clog << description;
    print_line(80,std::clog);
    return EXIT_SUCCESS;
  }
  bool ignore_null = false;
  if (var_map.count("ignore_null")>0) ignore_null = true;

  /****************************************************************/
  /*  Read data and compute description as we go.                 */
  /****************************************************************/
  if (var_map.count("are_int")>0) {
    std::map<unsigned int,double> discrete_distribution;
    std::set<double> seen;
    std::ifstream file(input_path.c_str(),std::ios::in);
    double normalization = 0;
    // read file
    std::string line_buffer; 
    double data_buffer;
    while( getline(file,line_buffer) ) {
      std::stringstream line_buffer_stream(line_buffer);
      for (unsigned int i = 0;  i <= columnn_idx; ++i) line_buffer_stream >> data_buffer;
      // data_buffer contains the relevant data
      if (!ignore_null || std::abs(data_buffer)>tolerance) {
        auto ret_value = seen.insert(data_buffer);
        if (ret_value.second == true) {
          discrete_distribution[(unsigned int)data_buffer]=1;
        }
        else {
          discrete_distribution[(unsigned int)data_buffer]+=1;
        }
        ++normalization;
      }
    }
    for (auto it = seen.begin(); it != seen.end(); ++it) {
      std::cout << std::left <<  std::setprecision(100) << std::setw(16) << *it << "\t" <<  std::setprecision(precision) << std::setw(precision*2) << discrete_distribution[*it] / normalization << "\n";
    }
  }
  else {
    std::map<double,double> discrete_distribution;
    std::set<double> seen;
    std::ifstream file(input_path.c_str(),std::ios::in);
    double normalization = 0;
    // read file
    std::string line_buffer; 
    double data_buffer;
    while( getline(file,line_buffer) ) {
      std::stringstream line_buffer_stream(line_buffer);
      for (unsigned int i = 0;  i <= columnn_idx; ++i) line_buffer_stream >> data_buffer;
      // data_buffer contains the relevant data
      if (!ignore_null || std::abs(data_buffer)>tolerance) {
        auto ret_value = seen.insert(data_buffer);
        if (ret_value.second == true) {
          discrete_distribution[data_buffer]=1;
        }
        else {
          discrete_distribution[data_buffer]+=1;
        }
        ++normalization;
      }
    }
    for (auto it = seen.begin(); it != seen.end(); ++it) {
      std::cout << std::fixed << std::left << std::setprecision(precision) << std::setw(precision*2) <<  *it << discrete_distribution[*it] / normalization << "\n";
    }
  }
  return EXIT_SUCCESS;
}


void print_line(unsigned int _lenght, std::ostream& _os) {
/* Print a line of _lengt stars.
 */
  for (unsigned int i = 0; i < _lenght; ++i) std::clog << "*";
  _os << "\n";
  return;
}
