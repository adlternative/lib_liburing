#ifndef __LIB_LIB_URING_H__
#define __LIB_LIB_URING_H__

#define SPDLOG_FMT_EXTERNAL
#include <cassert>
#include <liburing.h>
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
  explicit Uring(unsigned int entries, unsigned flags = 0) {
    if (io_uring_queue_init(entries, &ring, flags)) {
      int save_errno = errno;
      throw std::system_error(save_errno, std::system_category(), "Uring");
    }
  }

  void clear() { io_uring_queue_exit(&ring); }
  ~Uring() { clear(); }

  /*****************submit*****************************/

  int submitOne(const std::shared_ptr<adl::AsyncIORequest> &request);

  void submit_and_wait(unsigned wait_nr) {
    int ret = io_uring_submit_and_wait(&ring, wait_nr);
    if (ret < 0) {
      int save_errno = errno;
      throw std::system_error(save_errno, std::system_category(),
                              "Uring::submit");
    }
  }

  /*****************sqe***************************/

  struct io_uring_sqe *get_sqe() {
    return io_uring_get_sqe(&ring);
  }

  // struct AsyncIORequest *create_AsyncIORequest();

  /*****************cqe***************************/

  /* 内核给 complicate_queue.push_back()
   * 用户则是给 complicate_queue.pop_front()
   * 用户用完 cqe 之后 head++
   */

  void cq_advance_one_with_cqe(struct io_uring_cqe *cqe) {
    io_uring_cqe_seen(&ring, cqe);
  }
  void cq_advance(unsigned nr) { io_uring_cq_advance(&ring, nr); }

  /*****************wait***************************/

  std::vector<adl::AsyncIORequest *> wait(size_t minRequests,
                                          size_t maxRequests);

  /**
   * @brief 等到一个 cqe 完成
   *
   * @return struct io_uring_cqe*
   */
  struct io_uring_cqe *wait() {
    struct io_uring_cqe *cqe_ptr = nullptr;
    int ret = io_uring_wait_cqe(&ring, &cqe_ptr);
    if (ret < 0) {
      int save_errno = errno;
      throw std::system_error(save_errno, std::system_category(),
                              "Uring::wait");
    }
    return cqe_ptr;
  }

  /**
   * @brief 非阻塞,返回 cqe 如果 EAGAIN
   *
   * @return struct io_uring_cqe*
   */
  struct io_uring_cqe *non_block_wait() {
    struct io_uring_cqe *cqe_ptr = nullptr;
    int ret = io_uring_peek_cqe(&ring, &cqe_ptr);
    if (ret < 0) {
      int save_errno = errno;
      if (save_errno == EAGAIN || save_errno == EWOULDBLOCK)
        return nullptr;
      throw std::system_error(save_errno, std::system_category(),
                              "Uring::non_block_wait");
    }
    // spdlog::debug("{}", ret);
    return cqe_ptr;
  }

  int wait_cqe_timeout(struct io_uring_cqe **cqe_ptr,
                       struct __kernel_timespec *ts);

  int wait_cqe_nr(struct io_uring_cqe **cqe_ptr, unsigned wait_nr);

  /**********register*******/
  int register_buffers(const struct iovec *iovecs, unsigned nr_iovecs);
  int unregister_buffers(struct io_uring *ring);
  int register_files(const int *files, unsigned nr_files);
  int unregister_files(struct io_uring *ring);
  int register_files_update(unsigned off, int *files, unsigned nr_files);
  int register_eventfd(int fd);
  int register_eventfd_async(int fd);
  int unregister_eventfd(struct io_uring *ring);
  int register_probe(struct io_uring_probe *p, unsigned nr);
  int register_personality(struct io_uring *ring);
  int unregister_personality(int id);
  int register_restrictions(struct io_uring *ring,
                            struct io_uring_restriction *res,
                            unsigned int nr_res);

private:
  struct io_uring ring;
};

} // namespace adl
#endif // __LIB_LIB_URING_H__