#include "../include/AsyncIORequest.hpp"
#include "../include/lib_lib_uring.hpp"
#include <fcntl.h>
#include <fmt/core.h>
#include <fmt/printf.h>
#include <iostream>
#include <memory>
#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define QD 32
#define BS (16 * 1024)

static int get_file_size(int fd, off_t *size) {
  struct stat st;

  if (fstat(fd, &st) < 0)
    return -1;
  if (S_ISREG(st.st_mode)) {
    *size = st.st_size;
    return 0;
  } else if (S_ISBLK(st.st_mode)) {
    unsigned long long bytes;

    if (ioctl(fd, BLKGETSIZE64, &bytes) != 0)
      return -1;

    *size = bytes;
    return 0;
  }
  return -1;
}

int copy_file(const char *in_file_name, const char *out_file_name) {
  int ret;
  off_t total_size;
  off_t read_offset = 0;
  int infd, outfd;
  std::unique_ptr<adl::Uring> uring = std::make_unique<adl::Uring>(QD);

  infd = open(in_file_name, O_RDONLY);
  if (infd < 0) {
    spdlog::error("open {}: {}", in_file_name, strerror(errno));
    return -1;
  }

  outfd = open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (outfd < 0) {
    spdlog::error("open {}: {}", out_file_name, strerror(errno));
    return -1;
  }

  if (get_file_size(infd, &total_size)) {
    spdlog::error("{} get_file_size error: {}", in_file_name, strerror(errno));
    return -1;
  } else {
    spdlog::info("{} size:{}", in_file_name, total_size);
  }

  off_t write_left = total_size, read_left = total_size;

  /* 将输出文件的大小分成多个块进行分配并分配给读取任务，
   * 注意这样可能分配的内存会比较大，但是这样可以保证读取任务的效率。*/
  while (read_left) {
    /* 限制每次的提交块最大 BS 16K */
    off_t block_size = std::min(read_left, (long)BS);
    /* 准备数据和缓冲区加入到 uring的请求队列中，
     * 还没真的 prep_read! */
    /* 为每一个请求分配一个 BUFFER */
    auto buffer = new char[block_size];
    auto read_request = std::make_shared<adl::AsyncReadRequest>(
        infd, buffer, block_size, read_offset);

    read_request->set_finish_callback([=, &write_left, &uring]() -> void * {
      auto write_request = std::make_shared<adl::AsyncWriteRequest>(
          outfd, read_request->get_buffer(), read_request->get_processed_size(),
          read_offset);
      write_request->set_finish_callback([=, &write_left, &uring]() -> void * {
        spdlog::info("Write_request finish callback begin!");
        spdlog::debug("processed_size={}", write_request->get_processed_size());
        write_left -= write_request->get_processed_size();
        free(write_request->get_buffer());
        return nullptr;
      });
      uring->ioRequestsQueue.push_back(write_request);
      return nullptr;
    });
    uring->ioRequestsQueue.push_back(read_request);

    read_left -= block_size;
    read_offset += block_size;
  }
  while (write_left) {
    /* 提交任务 */
    if (uring->submit() < 0) {
      spdlog::critical("uring submit failed: {}\n", strerror(-ret), -ret);
      return -1;
    }

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
          close(infd);
          break;
        default:
          /* bug? */
          close(infd);
          close(outfd);
          spdlog::critical("Unkown request_ptr->get_stat() {}",
                           request_ptr->get_stat());
          break;
        }
      }
    } else {
      spdlog::error("Something wrong when uring wait?");
      close(infd);
      close(outfd);
      spdlog::critical("Bye!");
      return -1;
    }
  }
  close(outfd);

  return 0;
}

int main(int argc, char *argv[]) {
  off_t file_size;
  int ret;

  spdlog::set_level(spdlog::level::debug);
  // spdlog::set_pattern("*** [%H:%M:%S %z] %v ***");
  if (argc < 3) {
    spdlog::error("Usage: {} <infile> <outfile>\n", argv[0]);
    exit(1);
  }
  return copy_file(argv[1], argv[2]);
}
