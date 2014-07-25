// Compute a binned distribution from a set of raw data.
// A column may be specified by arguments
// Compilation [requires the dev boost package]: 
// g++ -o3 -W -Wall -Wextra -pedantic -std=c++0x binned_discrete_distribution.cpp -lboost_program_options -o binned_discrete_distribution

#include <cstdlib> 
#include <cmath> 
#include <iostream>
#include <iomanip>
#include <fstream> 
#include <map> 
#include <set> 
#include <vector>
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
  unsigned int column_idx=0;
  unsigned int row_idx=0;
  unsigned int precision=8;
  unsigned int number_of_bins=10;
  double lower_bound=0;
  double upper_bound=1;
  double tolerance=1e-10;

  // boost::po declarations
  po::options_description description(" Options");
  description.add_options()
  ("input_path,i",boost::program_options::value<std::string>(&input_path)->default_value(""),
    "Path to the edge list file.")
  ("column_idx,c",boost::program_options::value<unsigned int>(&column_idx),
    "Column index of the raw data (starting from column 0).")
  ("row_idx,r",boost::program_options::value<unsigned int>(&row_idx),
    "Row index of the raw data (starting from column 0). Conflicts and override column_idx mode, which is the default behavior ")
  ("precision,p",boost::program_options::value<unsigned int>(&precision),
    "Precision of the output.")
  ("number_of_bins,b",boost::program_options::value<unsigned int>(&number_of_bins),
    "Number of bins.")
  ("lower_bound,l",boost::program_options::value<double>(&lower_bound),
    "Lower bound of the bins.")
  ("upper_bound,u",boost::program_options::value<double>(&upper_bound),
    "Upper bound of the bins.")
  ("ignore_null","Ignore null entries.")
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
  if (var_map.count("upper_bound")==0 || var_map.count("lower_bound")==0) {
    std::clog << "ERR: [Missing parameter] Set lower and upper bound of the bins with -l and -u flags.\n";
    return EXIT_FAILURE;
  }
  bool ignore_null = false;
  if (var_map.count("ignore_null")>0) ignore_null = true;

  /****************************************************************/
  /*  Read data and compute distribution as we go.                */
  /****************************************************************/
  std::vector<double> bin_limits(number_of_bins+1);
  std::vector<double> bins(number_of_bins+1);
  for (unsigned int i = 0 ; i < number_of_bins+1; ++i) {
    bin_limits[i]= lower_bound + ( (double) i / ((double) number_of_bins) ) * ( upper_bound-lower_bound ) ; 
  }

  std::ifstream file(input_path.c_str(),std::ios::in);
  double normalization = 0;
  // read file
  std::string line_buffer; 
  double data_buffer;
  if (var_map.count("row_idx")==0) {
    // column mode
    while( getline(file,line_buffer) ) {
      std::stringstream line_buffer_stream(line_buffer);
      for (unsigned int i = 0;  i <= column_idx; ++i) line_buffer_stream >> data_buffer;
      // data_buffer contains the relevant data
      if (!ignore_null || std::abs(data_buffer)>tolerance) {
        unsigned int bin_idx = 0;
        for (; bin_limits[bin_idx] <= data_buffer; ++bin_idx) {};
        // data is between limits of bin_idx-1 and bin_idx, thus in bins @ bin_idx-1 (due to the lower bound being the first bin limit)
        bins[bin_idx-1] += 1.0;
        normalization += 1.0;
      }
    }
  }
  else {
    // row mode
    unsigned int row_count = 0;
    while(row_count<=row_idx)  {
      getline(file,line_buffer);
      ++row_count;
    }
    std::stringstream line_buffer_stream(line_buffer);
    while (line_buffer_stream >> data_buffer) {
      if (!ignore_null || std::abs(data_buffer)>tolerance) {
        unsigned int bin_idx = 0;
        for (; bin_limits[bin_idx] <= data_buffer; ++bin_idx) {};
        // data is between limits of bin_idx-1 and bin_idx, thus in bins @ bin_idx-1 (due to the lower bound being the first bin limit)
        bins[bin_idx-1] += 1.0;
        normalization += 1.0;
      }
    }
  }

  // binning
  for (unsigned int i = 0 ; i < number_of_bins; ++i) {
    std::cout  << std::left <<  std::setprecision(precision) << std::setw(16)  << (bin_limits[i+1]+bin_limits[i])/2 << "\t" 
               <<  std::setprecision(precision) << std::setw(precision*2) << bins[i] / normalization << "\n";
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
