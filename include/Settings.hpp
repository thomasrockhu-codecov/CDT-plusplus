/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2020 Adam Getchell
 ******************************************************************************/

/// @file Settings.hpp
/// @brief Global integer and precision settings
/// @author Adam Getchell
/// @details Global project settings on integer types and MPFR precision

#ifndef INCLUDE_SETTINGS_HPP_
#define INCLUDE_SETTINGS_HPP_

#include <CGAL/GMP/Gmpzf_type.h>

#include <cstdint>

/// Results are converted to a CGAL multi-precision floating point number.
/// Gmpzf itself is based on GMP (https://gmplib.org), as is MPFR.
using Gmpzf = CGAL::Gmpzf;

/// Sets the type of integer to use throughout the project.
/// These are the base values read into the program or used in calculations.
/// Casts to unsigned types are still necessary for certain library functions
/// to work.

#if __linux
using Int_precision = int;
#else
using Int_precision = std::int_fast32_t;
#endif

#ifdef _WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

// #define CGAL_LINKED_WITH_TBB

/// Correctly declare global constants
/// See Jonathan Boccara's C++ Pitfalls, January 2021

/// Sets the precision for <a href="http://www.mpfr.org">MPFR</a>.
static inline Int_precision constinit const PRECISION  = 256;

/// Default foliated triangulation spacings
static inline double constinit const INITIAL_RADIUS    = 1.0;
static inline double constinit const FOLIATION_SPACING = 1.0;

/// Sets epsilon values for floating point comparisons
static inline double constinit const TOLERANCE         = 0.01;

/// Depends on INITIAL_RADIUS and RADIAL_FACTOR
[[maybe_unused]] static inline Int_precision constinit const GV_BOUNDING_BOX_SIZE =
    100;

/// Aligns data for ease of access on 64-bit CPUs at the expense of padding
static inline int constinit const ALIGNMENT_64_BIT = 64;

/// Except when we only need 32 bits
static inline int constinit const ALIGNMENT_32_BIT = 32;

#endif  // INCLUDE_SETTINGS_HPP_
