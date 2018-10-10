/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell
///
/// Simplicial Manifold data structures
///
/// @file  Manifold.hpp
/// @brief Data structures for manifolds
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MANIFOLD_HPP
#define CDT_PLUSPLUS_MANIFOLD_HPP

#include <Geometry.hpp>
#include <S3Triangulation.hpp>
#include <functional>
#include <stddef.h>

/// Manifold class template
/// @tparam dimension Dimensionality of manifold
template <std::size_t dimension>
class Manifold;

/// 3D Manifold
template <>
class Manifold<3>
{
 public:
  /// @brief Default ctor
  Manifold() = default;

  Manifold(std::size_t desired_simplices, std::size_t desired_timeslices)
      : universe{make_triangulation(desired_simplices, desired_timeslices)}
      , geometry{make_geometry(getUniverse())}
  {}

  /// @brief Construct a Geometry of useful data from a triangulation
  /// @tparam Manifold Reference type of triangulation
  /// @param manifold Reference to triangulation
  /// @return Geometry of triangulation
  template <typename Manifold>
  Geometry3 make_geometry(Manifold& manifold)
  {
#ifndef NDEBUG
    std::cout << "make_geometry() invoked ...\n";
#endif

    Geometry3 geom{manifold};
    return geom;
  }

  /// @brief Obtain a reference to the triangulation pointer
  /// Note: would prefer observer_ptr<T>, GotW91 suggests a T*
  /// @return A read-only reference to the triangulation pointer
  const std::unique_ptr<Delaunay3>& getUniverse() const { return universe; }

  /// @brief Obtain a reference to the Geometry
  /// @return A read-only reference to the Geometry
  const Geometry3& getGeometry() const { return geometry; }

 private:
  std::unique_ptr<Delaunay3> universe;

  Geometry3 geometry;
};

using Manifold3 = Manifold<3>;

#endif  // CDT_PLUSPLUS_MANIFOLD_HPP
