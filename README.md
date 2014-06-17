cli-stats
=========

This repository contains a few command line utilities that I use on a day-to-day basis.
Everything is implemented in C++, since these CLI are used to deal with large ascii files containing millions of entries.
Memory usage is kept to a minimum by dumping everything to the standard output whenever possible.

### Compilation

    # discrete_distribution.cpp
    g++ -o3 -W -Wall -Wextra -pedantic -std=c++0x discrete_distribution.cpp -lboost_program_options -larmadillo -o discrete_distribution
    # binned_discrete_distribution.cpp
    g++ -o3 -W -Wall -Wextra -pedantic -std=c++0x binned_discrete_distribution.cpp -lboost_program_options -larmadillo -o binned_discrete_distribution

### Installation 

    mkdir ~/bin
    cp discrete_distribution ~/bin/discrete_distribution
    cp binned_discrete_distribution ~/bin/binned_discrete_distribution

### Usage

For the following raw data file

    # file.dat
    0  127  0.65
    1  52   0.84
    2  16   0.51
    3  1    0.54
    4  17   0.87
    5  43   0.67
    6  13   0.64
    7  24   0.38
    8  53   0.78
    9  9    0.75
    10  40  0.88

One could obtain the discrete distribution of the value in column 1 (column are numbered from 0 to n-1) with

    discrete_distribution -i file.dat -c 1 --are_int

or the binned distribution of the values in column 2 with 5 bins spanning the range [0.5, 1.0] using
    
    binned_discrete_distribution -i file.dat -l 0.5 -u 1.0 -b 5 -c 2

Please use the `--help` flag to see the list of available commands.