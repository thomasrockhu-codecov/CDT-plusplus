#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/draw_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <spdlog/spdlog.h>

#include "Utilities.hpp"

using K       = CGAL::Exact_predicates_inexact_constructions_kernel;
using DT3     = CGAL::Delaunay_triangulation_3<K>;
using Point   = K::Point_3;
using Creator = CGAL::Creator_uniform_3<double, Point>;

auto main() -> int
try
{
  std::vector<K::Point_3>                              points;
  CGAL::Random_points_in_sphere_3<K::Point_3, Creator> gen(1.0);
  std::copy_n(gen, 50, std::back_inserter(points));

  DT3 dt3(points.begin(), points.end());

  // Write to file
  std::string const filename = "test.off";
  utilities::write_file(filename, dt3);

  // Read from file
  auto dt_in = utilities::read_file<DT3>(filename);

  // Draw triangulation
  fmt::print("Drawing {}\n", filename);
  CGAL::draw(dt_in);

  return EXIT_SUCCESS;
}
catch (...)
{
  spdlog::critical("Something went wrong ... Exiting.\n");
  return EXIT_FAILURE;
}
