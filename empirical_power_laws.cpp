/** Take a long list of integer and produce various graphical representations of the data.
  * Compilation [requires the dev boost package and GSL headers]: 
  *     g++ -o3 -W -Wall -Wextra -pedantic -std=c++0x empirical_power_laws.cpp -lboost_program_options  -lgsl -lgslcblas -o empirical_power_laws
  *  
  */
// Streams
#include <iostream>
#include <iomanip>
#include <fstream>
// C and STL Libraries
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <set>
#include <iterator>
// GSL
#include "gsl/gsl_histogram.h"
#include "gsl/gsl_errno.h"
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

  // boost::po declarations
  po::options_description description(" Options");
  description.add_options()
  ("input_path,i",boost::program_options::value<std::string>(&input_path),
    "Path to the edge list file.")
  ("columnn_idx,c",boost::program_options::value<unsigned int>(&columnn_idx),
    "Column index of the raw data (starting from column 0).")
  ("precision,p",boost::program_options::value<unsigned int>(&precision),
    "Precision of the output.")
  ("logarithmic_bins,L" ,"Produce logarithmically binned histogram.")
  ("ccdf,C","Produce the un-binned empirical CCDF.")
  ("help,h" ,"Produce this help message.")
  ;
  po::variables_map var_map;
  po::store(po::parse_command_line(argc,argv,description), var_map);
  po::notify(var_map);
  if (argc==1 || var_map.count("help")>0) {
    print_line(80,std::clog);
    std::clog << "Empirical power law graphical representation.\n" << " Author: J.-G. Young.\n";
    print_line(80,std::clog);
    std::clog << description;
    print_line(80,std::clog);
    return EXIT_SUCCESS;
  }
  // Test parameters
  if (var_map.count("logarithmic_bins") > 0 && var_map.count("ccdf") > 0) {
    std::clog << "Both --ccdf and --logarithmic_bins flags are present. Select only one.\n";
    return EXIT_FAILURE;
  }
  assert(var_map.count("logarithmic_bins") == 0 || var_map.count("ccdf") == 0);

  if (var_map.count("input_path") == 0) {
    std::clog << "No input file provided present. Use the -i or --input_path= flag.\n";
    return EXIT_FAILURE;
  }
  assert(var_map.count("input_path") > 0);
  std::ifstream file(input_path.c_str(), std::ios::in);
  if (!file.is_open()) {
    std::clog << "Cannot open " << input_path << "\n";
    return EXIT_FAILURE;
  }

  /****************************************************************/
  /* Read data and build output                                   */
  /****************************************************************/

  /* Build set of existing elements and return to first line */
  std::string line_buffer;
  unsigned int data_buffer;
  std::set<unsigned int> element_set;
  while(getline(file, line_buffer)) {
    std::stringstream lbs(line_buffer);
    lbs >> data_buffer;
    element_set.insert(data_buffer);
  }
  file.clear();
  file.seekg(0, std::ios::beg);

  /* Mode specific computations */

  if (var_map.count("logarithmic_bins") > 0) {
    // LOGARITHMIC MODE
    std::clog << "Logarithmically binned mode selected.\n";

    /* Declare histogram */
    unsigned int max_element = *std::prev(element_set.end());
    double max_bin = pow(10, ceil(log10(max_element)));
    unsigned int max_exponent = (unsigned int) log10(max_bin);
    unsigned int number_of_bins = 10 * max_exponent - 1;

    std::clog << "\tmax_bin = " << max_bin << "\n";
    std::clog << "\tmax_exponent = " << max_exponent << "\n";
    std::clog << "\tnumber_of_bins = " << number_of_bins << "\n";

    gsl_histogram * h = gsl_histogram_alloc(number_of_bins);
    double *range = new double[number_of_bins + 1];
    {
      for (unsigned int exponent = 0; exponent < max_exponent; ++exponent) {
        for (unsigned int bin = 0; bin < 10; ++bin) {
          range[exponent*10 + bin] = pow(10, exponent + bin * 0.1);
        }
      }
      range[number_of_bins] = pow(10, max_exponent);
    }
    gsl_histogram_set_ranges(h, range, number_of_bins + 1);

    /* Fill the histogram */
    while(getline(file, line_buffer)) {
      std::stringstream lbs(line_buffer);
      lbs >> data_buffer;
      gsl_histogram_increment(h, (double) data_buffer);
    }
    double norm = gsl_histogram_sum(h);

    /* Output to STDOUT and free memory */
    for (unsigned int idx = 0; idx < number_of_bins; ++idx) {
      std::cout << std::fixed << std::left << std::setprecision(precision) << std::setw(precision*2)
                << range[idx] << "\t" << range[idx + 1] << "\t" << gsl_histogram_get (h, idx) / norm << "\n";
    }

    delete[] range;
    gsl_histogram_free(h);
  }
  else {
    // Create the distribution
    std::clog << "Computing the empirical distribution.\n";
    /* Declare histogram */
    unsigned int nb_of_unique_elements = element_set.size();
    gsl_histogram * h = gsl_histogram_alloc(nb_of_unique_elements);
    double *range = new double[nb_of_unique_elements + 1];
    { // limit the scope of idx explicitly
      unsigned int idx = 0;
      for (auto it = element_set.begin(); it != element_set.end(); ++idx, ++it) {
        range[idx] = *it - 0.1;
      }
      range[idx] = range[idx - 1] + 0.2; 
    }
    gsl_histogram_set_ranges(h, range, nb_of_unique_elements + 1);

    /* Fill the histogram */
    while(getline(file, line_buffer)) {
      std::stringstream lbs(line_buffer);
      lbs >> data_buffer;
      gsl_histogram_increment(h, (double) data_buffer);
    }
    double norm = gsl_histogram_sum(h);

    if (var_map.count("ccdf") > 0) {
      // CCDF MODE
      std::clog << "CCDF mode selected.\n";
      double accu = 0;
      for (unsigned int idx = 0; idx < nb_of_unique_elements; ++idx) {
        accu += gsl_histogram_get (h, idx) / norm;
        std::cout << std::fixed << std::left << std::setprecision(precision) << std::setw(precision*2) 
                  << (int) round(range[idx] + 0.1) << "\t" << 1 - accu << "\n";
      }
    }
    else {
      // NORMAL MODE
      std::clog << "Normal mode selected.\n";
      /* Output to STDOUT and free memory */
      for (unsigned int idx = 0; idx < nb_of_unique_elements; ++idx) {
        std::cout << std::fixed << std::left << std::setprecision(precision) << std::setw(precision*2) 
                  << (int) round(range[idx] + 0.1) << "\t" << gsl_histogram_get (h, idx) / norm << "\n";
      }
    }
    delete[] range;
    gsl_histogram_free(h);
  }

  element_set.clear();
  file.close();
  return EXIT_SUCCESS;
}


void print_line(unsigned int _lenght, std::ostream& _os) {
/* Print a line of _lengt stars.
 */
  for (unsigned int i = 0; i < _lenght; ++i) std::clog << "*";
  _os << "\n";
  return;
}

