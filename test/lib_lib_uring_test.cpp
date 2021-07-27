#include "../include/Cqe.hpp"
#include "../include/Sqe.hpp"
#include "../include/lib_lib_uring.hpp"
#include <gtest/gtest.h>
#include <iostream>
#define SPDLOG_FMT_EXTERNAL
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
TEST(Uring, constructor) {
  try {
    adl::Uring uring(8, IORING_SETUP_SQPOLL);
    for (int i = 0; i < 10; i++) {
      auto sqe = uring.get_sqe();
      if (i < 8)
        ASSERT_NE(sqe, nullptr);
      else
        ASSERT_EQ(sqe, nullptr);
    }
  } catch (std::system_error &e) {
    spdlog::error("Error: {}", e.what());
  }
}

TEST(Sqe, create_impl) {
  int fd;

  try {
    adl::Uring uring(8, IORING_SETUP_SQPOLL);
    auto sqe = uring.create_Sqe();
    ASSERT_NE(sqe, nullptr);

    fd = open("./a.txt", O_RDWR, 0644);
    // spdlog::info("fd: {}", fd);
    ASSERT_GT(fd, 0);

    char buffer[4096];
    sqe->prep_pread(fd, buffer, 4096, 0);
    uring.submit();
    if (auto cqe = uring.wait_Cqe()) {
      EXPECT_EQ(cqe->get_result(), 5);
      EXPECT_STREQ(buffer, "func\n");
    }

  } catch (std::system_error &e) {
    spdlog::error("Error: {}", e.what());
  }
  close(fd);
}