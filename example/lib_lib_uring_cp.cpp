#ifndef ADL_LIB_LIB_URING_CP_H__
#define ADL_LIB_LIB_URING_CP_H__

#include "../include/AsyncIORequest.hpp"
#include "../include/file_util.hpp"
#include "../include/lib_lib_uring.hpp"
#include <fcntl.h>
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <memory>
#define SPDLOG_FMT_EXTERNAL
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>
#include <unistd.h>

DEFINE_int32(queue_depth, 32, "queue depth");
DEFINE_int64(block_size, 1U << 14, "queue depth");

#define FLAGS(x) FLAGS_##x

int copy_file(const char *in_file_name, const char *out_file_name) {
  try {
    int ret;
    auto uring = std::make_unique<adl::Uring>(FLAGS(queue_depth));

    auto infd = std::make_unique<adl::Fd>(open(in_file_name, O_RDONLY));
    auto outfd = std::make_unique<adl::Fd>(
        open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644));

    off_t total_size = infd->get_file_size();
    off_t read_offset = 0;

    off_t write_left = total_size, read_left = total_size;

    /* 将输出文件的大小分成多个块进行分配并分配给读取任务，
     * 注意这样可能分配的内存会比较大，但是这样可以保证读取任务的效率。*/
    while (read_left) {
      /* 限制每次的提交块最大 BS 16K */
      off_t block_size = std::min(read_left, (long)FLAGS(block_size));
      /* 准备数据和缓冲区加入到 uring的请求队列中，
       * 还没真的 prep_read! */
      /* 为每一个请求分配一个 BUFFER */
      auto buffer = new char[block_size];
      auto read_request = std::make_shared<adl::AsyncReadRequest>(
          *infd, buffer, block_size, read_offset);

      read_request->set_finish_callback(
          [=, &outfd, &write_left, &uring]() -> void * {
            auto write_request = std::make_shared<adl::AsyncWriteRequest>(
                *outfd, read_request->get_buffer(),
                read_request->get_processed_size(), read_offset);
            write_request->set_finish_callback(
                [=, &write_left, &uring]() -> void * {
                  spdlog::info("Write_request finish callback begin!");
                  write_left -= write_request->get_processed_size();
                  free(write_request->get_buffer());
                  return nullptr;
                });
            uring->push_back(write_request);
            return nullptr;
          });
      uring->push_back(read_request);

      read_left -= block_size;
      read_offset += block_size;
    }

    while (write_left) {

      /* 提交任务 */
      uring->submit();

      /* 等待完成 */
      if (auto complicate_queue_ptr = uring->wait(1, SIZE_MAX)) {
        auto &&complicate_queue = *complicate_queue_ptr;

        for (auto &&request_ptr : complicate_queue) {
          request_ptr->complete_call_back();
          switch (request_ptr->get_stat()) {
          case adl::AsyncIORequest::READY:
            uring->ioRequestsQueue.push_back(request_ptr);
            break;
          case adl::AsyncIORequest::FINISH:
            request_ptr->finish_callback();
            break;
          case adl::AsyncIORequest::ERROR: {
            if (request_ptr->get_errno() == EINTR) {
              /* TEMP DONOTHING */
              continue;
            } else {
              // fallthrough
            }
          }
          case adl::AsyncIORequest::CLOSED:
            infd.reset();
            break;
          default:
            // /* bug? */
            throw std::runtime_error(fmt::format(
                "Unkown request_ptr->get_stat() {}", request_ptr->get_stat()));
            break;
          }
        }
      }
    }
  } catch (std::system_error &e) {
    spdlog::critical("system_error: {}", e.what());
    return -1;
  } catch (std::runtime_error &e) {
    spdlog::critical("runtime_error: {}", e.what());
    return -1;
  } catch (std::exception &e) {
    spdlog::critical("exception: {}", e.what());
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  off_t file_size;
  int ret;

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  spdlog::set_level(spdlog::level::debug);
  // spdlog::set_pattern("*** [%H:%M:%S %z] %v ***");
  if (argc < 3) {
    spdlog::error("Usage: {} <infile> <outfile>\n", argv[0]);
    exit(1);
  }
  return copy_file(argv[1], argv[2]);
}

#endif // ADL_LIB_LIB_URING_CP_H__