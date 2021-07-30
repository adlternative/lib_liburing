#include "../include/AsyncIORequest.hpp"
#include "../include/lib_lib_uring.hpp"
#include <gtest/gtest.h>
#include <iostream>
#define SPDLOG_FMT_EXTERNAL
#include <fcntl.h>
#include <memory>
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

TEST(Uring, AsyncIORequest) {
  char buffer[4096];
  int fd = open("./a.txt", O_RDWR, 0644);
  ASSERT_GT(fd, 0);
  spdlog::set_level(spdlog::level::debug);

  try {
    adl::Uring uring(8, IORING_SETUP_SQPOLL);
    auto read_request = std::make_shared<adl::AsyncIORequest>();
    read_request->prep_pread(fd, buffer, 4096, 0);
    uring.submitOne(read_request);
    auto result_vec_pointer = uring.wait(1, 1);
    if (result_vec_pointer) {
      EXPECT_EQ((*result_vec_pointer).size(), 1);
      EXPECT_EQ((*result_vec_pointer)[0]->get_result(), 5);
      EXPECT_STREQ(buffer, "func\n");
      close(fd);
    }
  } catch (std::system_error &e) {
    spdlog::error("Error: {}", e.what());
  } catch (...) {
    close(fd);
    throw;
  }
}