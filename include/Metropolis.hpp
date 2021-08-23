/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2015 Adam Getchell
 ******************************************************************************/

/// @file Metropolis.hpp
/// @brief Perform Metropolis-Hastings algorithm on Delaunay Triangulations
/// @author Adam Getchell
/// @details Performs the Metropolis-Hastings algorithm on the foliated Delaunay
/// triangulations. For details see:
/// M. Creutz, and B. Freedman. “A Statistical Approach to Quantum Mechanics.”
/// Annals of Physics 132 (1981): 427–62.
/// http://thy.phy.bnl.gov/~creutz/mypubs/pub044.pdf
///
/// @bug Accepted moves != attempted moves
/// @todo Atomic integral types for safe multithreading
/// @todo Implement concurrency
/// @todo Change A1 to count successful moves, total moves gets dragged down by
/// (6,2) attempts

#ifndef INCLUDE_METROPOLIS_HPP_
#define INCLUDE_METROPOLIS_HPP_

// CDT headers
#include "Move_strategy.hpp"
#include "S3Action.hpp"

using Gmpzf = CGAL::Gmpzf;

/// @brief Metropolis-Hastings algorithm strategy
/// @details The Metropolis-Hastings algorithm is a Markov Chain Monte Carlo
/// method. The probability of making an ergodic (Pachner) move is:
///
/// \f[P_{ergodic move}=a_{1}a_{2}\f]
/// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
/// \f[a_2=e^{\Delta S}\f]
///
/// @tparam ManifoldType The type of Manifold on which to apply the algorithm
template <typename ManifoldType>
class MoveStrategy<Strategies::METROPOLIS, ManifoldType>  // NOLINT
{
  /// @brief The length of the timelike edges
  long double m_Alpha{};

  /// @brief \f$K=\frac{1}{8\pi G_{N}}\f$.
  long double m_K{};

  /// @brief \f$\lambda=\frac{\Lambda}{8\pi G_{N}}\f$ where \f$\Lambda\f$ is
  /// the cosmological constant
  long double m_Lambda{};

  /// @brief The number of move passes executed by the algorithm
  /// @details Each move pass makes a number of attempts equal to the number of
  /// simplices in the triangulation.
  Int_precision m_passes{1};

  /// @brief The number of passes before a checkpoint
  /// @details Each checkpoint writes a file containing the current
  /// triangulation.
  Int_precision m_checkpoint{1};

  /// @brief The current geometry of the manifold
  Geometry<ManifoldType::dimension> m_geometry;

  /// @brief The number of moves the algorithm tried
  /// @details This equals accepted moves + rejected moves.
  move_tracker::MoveTracker<ManifoldType> m_trial_moves;

  /// @brief The number of moves accepted by the algorithm
  move_tracker::MoveTracker<ManifoldType> m_accepted_moves;

  /// @brief The number of moves rejected by the algorithm
  move_tracker::MoveTracker<ManifoldType> m_rejected_moves;

  /// @brief The number of moves that were attempted by a MoveCommand.
  /// @details This should equal accepted moves.
  move_tracker::MoveTracker<ManifoldType> m_attempted_moves;

  /// @brief The number of moves that succeeded in the MoveCommand
  move_tracker::MoveTracker<ManifoldType> m_succeeded_moves;

  /// @brief The number of moves that a MoveCommand failed to make due to an
  /// error
  move_tracker::MoveTracker<ManifoldType> m_failed_moves;

 public:
  /// @brief Default ctor
  MoveStrategy() = default;

  /// @brief Metropolis function object constructor
  /// @details Setup of runtime job parameters.
  /// @param Alpha \f$\alpha\f$ is the timelike edge length.
  /// @param K \f$k=\frac{1}{8\pi G_{Newton}}\f$
  /// @param Lambda \f$\lambda=k*\Lambda\f$ where \f$\Lambda\f$ is the
  /// Cosmological constant.
  /// @param passes Number of passes of ergodic moves on triangulation.
  /// @param checkpoint Print/write output for every n=checkpoint passes.
  [[maybe_unused]] MoveStrategy(long double Alpha, long double K,
                                long double Lambda, Int_precision passes,
                                Int_precision checkpoint)
      : m_Alpha(Alpha)
      , m_K(K)
      , m_Lambda(Lambda)
      , m_passes(passes)
      , m_checkpoint{checkpoint}
  {
#ifndef NDEBUG
    spdlog::info("{} called.\n", __PRETTY_FUNCTION__);
#endif
  }

  /// @return The length of the timelike edge
  [[nodiscard]] auto Alpha() const noexcept { return m_Alpha; }

  /// @return The normalized Newton's constant
  [[nodiscard]] auto K() const noexcept { return m_K; }

  /// @return The normalized Cosmological constant
  [[nodiscard]] auto Lambda() const noexcept { return m_Lambda; }

  /// @return The number of passes to make
  [[nodiscard]] auto passes() const noexcept { return m_passes; }

  /// @return The number of passes before writing a checkpoint file
  [[nodiscard]] auto checkpoint() const noexcept { return m_checkpoint; }

  /// @return The container of trial moves
  auto get_trial() const { return m_trial_moves; }

  /// @return The container of accepted moves
  auto get_accepted() const { return m_accepted_moves; }

  /// @return The container of rejected moves
  auto get_rejected() const { return m_rejected_moves; }

  /// @return The container of attempted moves
  auto get_attempted() const { return m_attempted_moves; }

  /// @return The container of successful moves
  auto get_succeeded() const { return m_succeeded_moves; }

  /// @return The container of failed moves
  auto get_failed() const { return m_failed_moves; }

  /// @brief Calculate A1
  /// @details Calculate the probability of making a move divided by the
  /// probability of its reverse, that is:
  /// \f[a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f]
  ///
  /// @param move The type of move
  /// @return \f$a_1=\frac{move[i]}{\sum\limits_{i}move[i]}\f$
  auto CalculateA1(move_tracker::move_type move) const noexcept
  {
    auto all_moves = m_attempted_moves.total();
    auto this_move = m_attempted_moves[move];
    // Set precision for initialization and assignment functions
    mpfr_set_default_prec(PRECISION);

    // Initialize for MPFR
    mpfr_t r1, r2, a1;                            // NOLINT
    mpfr_inits2(PRECISION, r1, r2, a1, nullptr);  // NOLINT

    mpfr_init_set_ui(r1, this_move, MPFR_RNDD);  // r1 = this_move NOLINT
    mpfr_init_set_ui(r2, all_moves, MPFR_RNDD);  // r2 = total_moves NOLINT

    // The result
    mpfr_div(a1, r1, r2, MPFR_RNDD);  // a1 = r1/r2 NOLINT

    // std::cout << "A1 is " << mpfr_out_str(stdout, 10, 0, a1, MPFR_RNDD)

    // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
    auto result = mpfr_get_d(a1, MPFR_RNDD);  // NOLINT

    // Free memory
    mpfr_clears(r1, r2, a1, nullptr);  // NOLINT

#ifndef NDEBUG
    fmt::print("total_attempted_moves() = {}\n", all_moves);
    fmt::print("A1 is {}\n", result);
#endif

    return result;
  }  // CalculateA1()

  /// @brief Calculate A2
  /// @details Calculate \f$a_2=e^{\Delta S}\f$
  /// @tparam dimension The dimensionality of the triangulation
  /// @param move The type of move
  /// @return \f$a_2=e^{-\Delta S}\f$
  template <int dimension>
  auto CalculateA2(move_tracker::move_type move) const noexcept
  {
    if (dimension == 3)
    {
      auto currentS3Action =
          S3_bulk_action(m_geometry.N1_TL, m_geometry.N3_31_13,
                         m_geometry.N3_22, m_Alpha, m_K, m_Lambda);
      auto newS3Action = static_cast<Gmpzf>(0);
      switch (move)
      {
        case move_tracker::move_type::TWO_THREE:
          // A (2,3) move adds a timelike edge
          // and a (2,2) simplex
          newS3Action =
              S3_bulk_action(m_geometry.N1_TL + 1, m_geometry.N3_31_13,
                             m_geometry.N3_22 + 1, m_Alpha, m_K, m_Lambda);
          break;
        case move_tracker::move_type::THREE_TWO:
          // A (3,2) move removes a timelike edge
          // and a (2,2) simplex
          newS3Action =
              S3_bulk_action(m_geometry.N1_TL - 1, m_geometry.N3_31_13,
                             m_geometry.N3_22 - 1, m_Alpha, m_K, m_Lambda);
          break;
        case move_tracker::move_type::TWO_SIX:
          // A (2,6) move adds 2 timelike edges and
          // 2 (1,3) and 2 (3,1) simplices
          newS3Action =
              S3_bulk_action(m_geometry.N1_TL + 2, m_geometry.N3_31_13 + 4,
                             m_geometry.N3_22, m_Alpha, m_K, m_Lambda);
          break;
        case move_tracker::move_type::SIX_TWO:
          // A (6,2) move removes 2 timelike edges and
          // 2 (1,3) and 2 (3,1) simplices
          newS3Action =
              S3_bulk_action(m_geometry.N1_TL - 2, m_geometry.N3_31_13,
                             m_geometry.N3_22 - 4, m_Alpha, m_K, m_Lambda);
          break;
        case move_tracker::move_type::FOUR_FOUR:
          // A (4,4) move changes nothing with respect to the action,
          // and e^0==1
#ifndef NDEBUG
          fmt::print("A2 is 1\n");
#endif
          return static_cast<double>(1);
        default:
          break;
      }

      auto exponent        = currentS3Action - newS3Action;
      auto exponent_double = Gmpzf_to_double(exponent);

      // if exponent > 0 then e^exponent >=1 so according to Metropolis
      // algorithm return A2=1
      if (exponent >= 0) { return static_cast<double>(1); }

      // Set precision for initialization and assignment functions
      mpfr_set_default_prec(PRECISION);

      // Initialize for MPFR
      mpfr_t r1, a2;                            // NOLINT
      mpfr_inits2(PRECISION, r1, a2, nullptr);  // NOLINT

      // Set input parameters and constants to mpfr_t equivalents
      mpfr_init_set_d(r1, exponent_double,  // NOLINT
                      MPFR_RNDD);           // r1 = exponent

      // e^exponent
      mpfr_exp(a2, r1, MPFR_RNDD);  // NOLINT

      // Convert mpfr_t total to Gmpzf result by using Gmpzf(double d)
      auto result = mpfr_get_d(a2, MPFR_RNDD);  // NOLINT

      // Free memory
      mpfr_clears(r1, a2, nullptr);  // NOLINT

#ifndef NDEBUG
      fmt::print("A2 is {}\n", result);
#endif

      return result;
    }
  }  // CalculateA2()

  /// @brief Attempt a move of the selected type
  /// @details This function implements the core of the Metropolis-Hastings
  /// algorithm by generating a random number and comparing with the results of
  /// CalculateA1 and CalculateA2. If the move is accepted, this function
  /// calls make_move(). If not, it updates **attempted_moves_**.
  /// @param move The type of move
  /// @return True if the move is accepted
  auto try_move(move_tracker::move_type move) -> bool
  {
    // Record the trial move
    m_trial_moves[move_tracker::as_integer(move)]++;

    // Calculate probability
    auto a1 = CalculateA1(move);

    // Make move if random number < probability
    auto a2 = CalculateA2<3>(move);

    const auto trial_value = generate_probability();
    // Convert to Gmpzf because trial_value will be set to 0 when
    // comparing with a1 and a2!
    const auto trial = static_cast<double>(trial_value);

#ifndef NDEBUG
    spdlog::info("{} called.\n", __PRETTY_FUNCTION__);
    spdlog::info("Trying move.\n");
    spdlog::info("Move type = {}\n", move_tracker::as_integer(move));
    spdlog::info("Trial_value = {}\n", trial_value);
    spdlog::info("Trial = {}\n", trial);
    spdlog::info("A1 = {}\n", a1);
    spdlog::info("A2 = {}\n", a2);
    spdlog::info("A1*A2 = {}\n", a1 * a2);
    spdlog::info("{}\n",
                 (trial <= a1 * a2) ? "Move accepted." : "Move rejected.");
#endif

    if (trial <= a1 * a2)
    {
      m_accepted_moves[move_tracker::as_integer(move)]++;
      return true;
    }

    m_rejected_moves[move_tracker::as_integer(move)]++;
    return false;

  }  // try_move()

  /// @brief Initialize the Metropolis algorithm
  /// @param t_manifold Manifold on which to operate
  /// @return A manifold with a move of each type completed
  [[nodiscard]] auto initialize(ManifoldType t_manifold)
      -> std::optional<MoveCommand<ManifoldType>>
  try
  {
    MoveCommand command(t_manifold);
    fmt::print("Making initial moves ...\n");

    command.enqueue(move_tracker::move_type::TWO_THREE);
    m_trial_moves.two_three_moves()++;
    m_accepted_moves.two_three_moves()++;

    command.enqueue(move_tracker::move_type::THREE_TWO);
    m_trial_moves.three_two_moves()++;
    m_accepted_moves.three_two_moves()++;

    command.enqueue(move_tracker::move_type::TWO_SIX);
    m_trial_moves.two_six_moves()++;
    m_accepted_moves.two_six_moves()++;

    command.enqueue(move_tracker::move_type::SIX_TWO);
    m_trial_moves.six_two_moves()++;
    m_accepted_moves.six_two_moves()++;

    command.enqueue(move_tracker::move_type::FOUR_FOUR);
    m_trial_moves.four_four_moves()++;
    m_accepted_moves.four_four_moves()++;

    // Execute the moves
    command.execute();

    // Update attempted, succeeded, and failed moves
    m_attempted_moves += command.get_attempted();
    m_succeeded_moves += command.get_succeeded();
    m_failed_moves += command.get_failed();

    // print initial results
    auto initial_results = command.get_results();
    initial_results.print();
    initial_results.print_details();

    return command;
  }
  catch (std::runtime_error const& RuntimeError)
  {
    fmt::print("{}\n", RuntimeError.what());
    fmt::print("Metropolis initialization failed ... exiting.\n");
    return std::nullopt;
  }

  /// @brief Call operator
  /// @details This makes the Metropolis class into a function object. Setup of
  /// the runtime job parameters is handled by the constructor. This () operator
  /// conducts all of the algorithmic work for Metropolis-Hastings on the
  /// manifold.
  /// @param t_manifold Manifold on which to operate
  /// @return The manifold upon which the passes have been completed
  auto operator()(ManifoldType const& t_manifold) -> ManifoldType
  {
#ifndef NDEBUG
    spdlog::info("{} called.\n", __PRETTY_FUNCTION__);
#endif

    fmt::print(
        "Starting Metropolis-Hastings algorithm in {}+1 dimensions ...\n",
        ManifoldType::dimension - 1);

    auto command = initialize(t_manifold).value_or(MoveCommand(t_manifold));

    fmt::print("Making random moves ...\n");
    print_results();
    // Loop through m_passes
    for (auto pass_number = 1; pass_number <= m_passes; ++pass_number)
    {
      fmt::print("=== Pass {} ===\n", pass_number);
      auto total_simplices_this_pass = command.get_const_results().N3();
      // Attempt a random move per simplex
      for (auto move_attempt = 0; move_attempt < total_simplices_this_pass;
           ++move_attempt)
      {
        // Pick a move to attempt
        auto move = move_tracker::generate_random_move_3();

        if (try_move(move)) { command.enqueue(move); }
      }  // Ends loop through CurrentTotalSimplices

      // Do the moves
      command.execute();

      // Update attempted and failed moves
      this->m_attempted_moves += command.get_attempted();
      this->m_succeeded_moves += command.get_succeeded();
      this->m_failed_moves += command.get_failed();

      // Do stuff on checkpoint
      if ((pass_number % m_checkpoint) == 0)
      {
        fmt::print("=== Pass {} ===\n", pass_number);
        fmt::print("Writing to file.\n");
        print_results();
        auto result = command.get_results();
        write_file(result, topology_type::SPHERICAL, ManifoldType::dimension,
                   result.N3(), result.max_time(), INITIAL_RADIUS,
                   FOLIATION_SPACING);
      }
    }  // Ends loop through m_passes

    // output results
    fmt::print("=== Run results ===\n");
    print_results();
    return command.get_results();
  }  // operator()

  /// @brief Display results of run
  void print_results()
  {
    if (ManifoldType::dimension == 3)
    {
      fmt::print("=== Move Results ===\n");
      fmt::print(
          "There were {} trial moves with {} accepted moves and {} rejected "
          "moves.\n",
          m_trial_moves.total(), m_accepted_moves.total(),
          m_rejected_moves.total());
      fmt::print(
          "There were {} attempted moves with {} successful moves and {} "
          "failed moves.\n",
          m_attempted_moves.total(), m_succeeded_moves.total(),
          m_failed_moves.total());
      fmt::print(
          "(2,3) moves: {} trial ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_trial_moves.two_three_moves(), m_accepted_moves.two_three_moves(),
          m_rejected_moves.two_three_moves(),
          m_attempted_moves.two_three_moves(),
          m_succeeded_moves.two_three_moves(),
          m_failed_moves.two_three_moves());

      fmt::print(
          "(3,2) moves: {} trial ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_trial_moves.three_two_moves(), m_accepted_moves.three_two_moves(),
          m_rejected_moves.three_two_moves(),
          m_attempted_moves.three_two_moves(),
          m_succeeded_moves.three_two_moves(),
          m_failed_moves.three_two_moves());

      fmt::print(
          "(2,6) moves: {} trial ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_trial_moves.two_six_moves(), m_accepted_moves.two_six_moves(),
          m_rejected_moves.two_six_moves(), m_attempted_moves.two_six_moves(),
          m_succeeded_moves.two_six_moves(), m_failed_moves.two_six_moves());

      fmt::print(
          "(6,2) moves: {} trial ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_trial_moves.six_two_moves(), m_accepted_moves.six_two_moves(),
          m_rejected_moves.six_two_moves(), m_attempted_moves.six_two_moves(),
          m_succeeded_moves.six_two_moves(), m_failed_moves.six_two_moves());

      fmt::print(
          "(4,4) moves: {} trial ({} accepted and {} rejected) with {} "
          "attempted ({} successful and {} failed).\n",
          m_trial_moves.four_four_moves(), m_accepted_moves.four_four_moves(),
          m_rejected_moves.four_four_moves(),
          m_attempted_moves.four_four_moves(),
          m_succeeded_moves.four_four_moves(),
          m_failed_moves.four_four_moves());
    }
  }  // print_results
};   // Metropolis

using Metropolis3 = MoveStrategy<Strategies::METROPOLIS, manifolds::Manifold3>;
using Metropolis4 = MoveStrategy<Strategies::METROPOLIS, manifolds::Manifold4>;

#endif  // INCLUDE_METROPOLIS_HPP_
