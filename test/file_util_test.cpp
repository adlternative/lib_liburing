#include "../include/file_util.hpp"
#include <gtest/gtest.h>

TEST(file_util, Fd) {
  try {
    adl::Fd fd(open("/dev/null", O_RDWR));
    EXPECT_EQ(write(fd, "hello world", 11), 11);
    fd = open("/tmp/a.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    EXPECT_EQ(write(fd, "hello world", 11), 11);
    fd = open("/tmp/a.txt", O_RDONLY);
    char buffer[11];
    EXPECT_EQ(read(fd, buffer, 11), 11);
    EXPECT_EQ(strncmp(buffer, "hello world", 11), 0);
    EXPECT_STREQ(buffer, "hello world");

  } catch (std::system_error &e) {
    FAIL() << "Failed to open /dev/null: " << e.what();
  }
}