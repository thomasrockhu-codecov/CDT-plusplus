/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2020 Adam Getchell

/// @file Move_command.hpp
/// @brief Do ergodic moves using the Command pattern
/// @author Adam Getchell

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include "Apply_move.hpp"
#include "Ergodic_moves_3.hpp"

template <typename ManifoldType,
          typename ExpectedType = tl::expected<ManifoldType, std::string_view>,
          typename FunctionType = function_ref<ExpectedType(ManifoldType&)>>
class MoveCommand
{
  /// @brief The Manifold on which to make the move
  ManifoldType m_manifold;

  /// @brief The queue of moves to make
  std::deque<FunctionType> m_moves;

 public:
  /// @brief Default dtor
  //  ~MoveCommand() = default;

  /// @brief No default ctor
  MoveCommand() = delete;

  //  /// @brief Default copy ctor
  //  MoveCommand(MoveCommand const& other) = default;
  //
  //  /// @brief Default copy assignment
  //  auto operator=(MoveCommand const& other) -> MoveCommand& = default;
  //
  //  /// @brief Default move ctor
  //  MoveCommand(MoveCommand&& other) noexcept = default;
  //
  //  /// @brief Default move assignment
  //  auto operator=(MoveCommand&& other) noexcept -> MoveCommand& = default;

  /// @brief MoveCommand ctor
  /// @param t_manifold The manifold to perform moves upon
  explicit MoveCommand(ManifoldType t_manifold)
      : m_manifold{std::move(t_manifold)}
  {}

  /// @return A read-only reference to the manifold
  auto get_manifold() const -> ManifoldType const&
  {
    return std::cref(m_manifold);
  }

  /// @return The results of the moves invoked by MoveCommand
  [[nodiscard]] auto get_results() -> ManifoldType& { return m_manifold; }

  /// @brief Execute a single move on the manifold
  /// @param t_move The move to do on the manifold
  void move(FunctionType&& t_move)
  {
#ifndef NDEBUG
    fmt::print("Before manifold move:\n");
    m_manifold.print_details();
#endif

    auto result = apply_move(m_manifold, std::forward<FunctionType>(t_move));
    if (result)
    {
      result->update();

#ifndef NDEBUG
      fmt::print("After manifold move:\n");
      m_manifold.print_details();
#endif

      swap(result.value(), m_manifold);
    }
    else
    {
      fmt::print(result.error());
    }

  }  // move

  /// @brief Push a Pachner move onto the move queue
  /// @param t_move The move to do on the manifold
  void enqueue(FunctionType t_move) { m_moves.push_front(std::move(t_move)); }

  auto size() const { return m_moves.size(); }

  /// Execute all moves in the queue on the manifold
  void execute()
  {
    while (m_moves.size() > 0)
    {
#ifndef NDEBUG
      fmt::print("Before manifold move:\n");
      m_manifold.print_details();
#endif

      auto move   = m_moves.back();
      auto result = apply_move(m_manifold, move);
      m_moves.pop_back();
      if (result)
      {
        result->update();
#ifndef NDEBUG
        fmt::print("After manifold move:\n");
        m_manifold.print_details();
#endif

        swap(result.value(), m_manifold);
      }
      else
      {
        fmt::print(result.error());
      }
    }
#ifndef NDEBUG
    fmt::print("After moves:\n");
    //    print_manifold_details(m_manifold);
    m_manifold.print_details();
#endif
  }  // execute

  // Functionality for later, perhaps using a Memento
  //    virtual void undo();
  //    virtual void redo();
};

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
