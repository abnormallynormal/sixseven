// Entry point for the search test suite.

#include "test_harness.h"

void run_soundness_tests(int depth);
void run_mate_tests();
void run_equivalence_tests(int depth);
void run_node_count_tests(int depth);
void print_node_count_summary();
void run_move_ordering_tests(int depth);
void print_move_ordering_summary();
void run_quiescence_tests();

int main()
{
  std::cout << "================ Chess Engine Search Test Suite ================\n";

  std::cout << "[main] starting A\n" << std::flush;
  run_soundness_tests(5);

  std::cout << "[main] starting B\n" << std::flush;
  run_mate_tests();

  std::cout << "[main] starting C\n" << std::flush;
  run_equivalence_tests(4);

  std::cout << "[main] starting D\n" << std::flush;
  run_node_count_tests(5);

  std::cout << "[main] starting E\n" << std::flush;
  run_move_ordering_tests(5);

  std::cout << "[main] starting F\n" << std::flush;
  run_quiescence_tests();

  // (G) repetition / 50-move draw — engine does not currently detect these, skip
  std::cout << "\n=== Repetition / 50-move Tests (G) ===\n";
  std::cout << "  SKIPPED: engine does not track repetition or fifty-move draws.\n";

  // Final summary
  print_node_count_summary();
  print_move_ordering_summary();
  print_summary();

  return g_stats().failed == 0 ? 0 : 1;
}
