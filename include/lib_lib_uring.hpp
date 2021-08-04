#ifndef __LIB_LIB_URING_H__
#define __LIB_LIB_URING_H__

#define SPDLOG_FMT_EXTERNAL
#include <cassert>
#include <deque>
#include <liburing.h>
#include <optional>
#include <queue>
#include <set>
#include <spdlog/spdlog.h>
#include <system_error>

namespace adl {
class AsyncIORequest;
class Uring {
public:
  /*****************basic*****************************/
  /**
   * @brief Construct a new lib uring wrap object
   *
   * @param entries 队列深度
   * @param flags IORING_SETUP_IOPOLL | IORING_SETUP_SQPOLL ...
   */
  explicit Uring(unsigned int ring_size, unsigned flags = 0)
      : ring_size_(ring_size), exited(false) {
    if (io_uring_queue_init(ring_size, &ring, flags)) {
      errno_ = errno;
      throw std::system_error(errno_, std::system_category(), "Uring");
    }
  }
  /**
   * @brief 表示 Uring 的生命周期结束 一般来说 Uring 应该是一个线程内的单例？
   *
   */
  void exit() {
    exited = true;
    io_uring_queue_exit(&ring);
  }
  /**
   * @brief Destroy the Uring object
   *
   */
  ~Uring() {
    if (!exited) {
      exited = true;
      exit();
    }
  }

  io_uring_sqe *get_sqe() { return io_uring_get_sqe(&ring); }

  /*****************submit*****************************/

  int submit();

  /*****************wait***************************/

  std::optional<std::vector<std::shared_ptr<adl::AsyncIORequest>>>
  wait(size_t minRequests, size_t maxRequests);

  int wait_cqe_timeout(struct io_uring_cqe **cqe_ptr,
                       struct __kernel_timespec *ts);

  int wait_cqe_nr(struct io_uring_cqe **cqe_ptr, unsigned wait_nr);

  /**********register*******/
  int register_buffers(const struct iovec *iovecs, unsigned nr_iovecs);
  int unregister_buffers();
  int register_files(const int *files, unsigned nr_files);
  int unregister_files();
  int register_files_update(unsigned off, int *files, unsigned nr_files);
  int register_eventfd(int fd);
  int register_eventfd_async(int fd);
  int unregister_eventfd();
  int register_probe(struct io_uring_probe *p, unsigned nr);
  int register_personality();
  int unregister_personality(int id);
  int register_restrictions(struct io_uring_restriction *res,
                            unsigned int nr_res);

  void push_back(const std::shared_ptr<adl::AsyncIORequest> &request) {
    ioRequestsQueue.push_back(request);
  }

  void push_back(std::shared_ptr<adl::AsyncIORequest> &&request) {
    ioRequestsQueue.push_back(std::move(request));
  }

  /* deque ? vec ? list ? */
  std::deque<std::shared_ptr<adl::AsyncIORequest>>
      ioRequestsQueue; /* 请求任务队列 */
  std::set<std::shared_ptr<adl::AsyncIORequest>>
      ioRequestsSet; /* 目前来说是用来在提交任务之后保存任务的智能指针 */

private:
  std::atomic<int> exited; /* 退出状态 */
  struct io_uring ring;    /* io_uring */
  int ring_size_;          /* 初始设置的环大小 */
  int errno_;              /* 保存 errno? */
};

} // namespace adl
#endif // __LIB_LIB_URING_H__