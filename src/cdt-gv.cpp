/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// A program that visualizes spacetimes
///
/// Inspired by:
/// http://doc.cgal.org/latest/Geomview/Geomview_2gv_terrain_8cpp-example.html
///
/// \done Load files generated by cdt.cpp
/// \done Invoke Geomview
/// \done Use <a href="https://github.com/docopt/docopt.cpp">docopt</a>
/// for a beautiful command line interface.
/// \todo Parse file and read in points only
/// \todo Colorize/improve output

/// @file cdt-gv.cpp
/// @brief A pipeline from CDT++ to <a href="http://www.geomview.org">Geomview
/// </a>, a visualizer
/// @author Adam Getchell

// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/IO/Geomview_stream.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/IO/Triangulation_geomview_ostream_3.h>

// C++ headers
#include <iostream>
#include <fstream>
#include <map>
#include <string>

// Docopt
#include "docopt/docopt.h"

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Delaunay = CGAL::Delaunay_triangulation_3<K>;
using Gt3 = CGAL::Projection_traits_xy_3<K>;
using Point3 = Gt3::Point;

/// Help message parsed by docopt into options
static const char USAGE[] {
R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2015 Adam Getchell

A program that loads d-dimensional triangulated spacetimes
with a defined causal structure generated by cdt.cpp into a GeomView
pipeline for visualization.

Note that the standard output of CDT++ includes cell neighbors, and should
be truncated to just include points.

Usage:./cdt-gv --file FILE

Example:
./cdt --file points.dat
./cdt --f points.dat

Options:
  -h --help             Show this message
  --version             Show program version
  -f --file FILENAME    The file to load into GeomView
)"
};

/// @brief The main path of the cdt-gv program
///
/// @param[in,out]  argc  Argument count = 1 + number of arguments
/// @param[in,out]  argv  Argument vector (array) to be passed to docopt
/// @returns        Integer value 0 if successful, 1 on failure
int main(int argc, char* const argv[]) {
  // docopt option parser
  std::map<std::string, docopt::value> args
    = docopt::docopt(USAGE,
                     { argv + 1, argv + argc},
                     true,           // print help message automatically
                     "cdt-gv 1.0");  // Version

  // Debugging docopt values
  // for (auto const& arg : args) {
  //   std::cout << arg.first << " " << arg.second << std::endl;
  // }

  // Parse docopt::values in args map
  auto file = args["--file"].asString();

  // Test
  std::cout << "File to be loaded is " << file << std::endl;

  CGAL::Geomview_stream gv(CGAL::Bbox_3(-100, -100, -100, 600, 600, 600));
  gv.set_line_width(4);
  gv.set_bg_color(CGAL::Color(0, 200, 200));

  Delaunay D;
  std::ifstream iFile(file, std::ios::in);
  Point3 p;

  // Insert points from file into Delaunay triangulation
  while (iFile >> p) {
    D.insert(p);
  }

  std::cout << "Drawing 3D Delaunay triangulation in wired mode." << std::endl;
  gv.set_wired(true);
  gv << D;

  std::cout << "Enter a key to finish" << std::endl;
  char ch;
  std::cin >> ch;

  return 0;
}
