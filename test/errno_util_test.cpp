
#include "../include/errno_util.hpp"
#include <gtest/gtest.h>

TEST(errorToString, errnoToString) {
  EXPECT_STREQ(adl::errorToString(EINTR), "EINTR");
  EXPECT_STREQ(adl::errorToString(EWOULDBLOCK), "EAGAIN");
  EXPECT_STREQ(adl::errorToString(211), "<INVALID errno>");
}