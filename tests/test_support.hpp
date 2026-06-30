#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

#define BSE_EXPECT_TRUE(expr)                                                        \
  do {                                                                               \
    if (!(expr)) {                                                                   \
      std::cerr << __FILE__ << ":" << __LINE__ << " expectation failed: " #expr     \
                << "\n";                                                            \
      std::abort();                                                                  \
    }                                                                                \
  } while (false)

#define BSE_EXPECT_EQ(lhs, rhs)                                                       \
  do {                                                                               \
    const auto bse_lhs_value = (lhs);                                                 \
    const auto bse_rhs_value = (rhs);                                                 \
    if (!(bse_lhs_value == bse_rhs_value)) {                                          \
      std::cerr << __FILE__ << ":" << __LINE__ << " equality failed: " #lhs          \
                << " != " #rhs << "\n";                                             \
      std::abort();                                                                  \
    }                                                                                \
  } while (false)
