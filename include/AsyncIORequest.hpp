#ifndef ADL_ASYNCIOREQUEST_H__
#define ADL_ASYNCIOREQUEST_H__

#include "./lib_lib_uring.hpp"
#include <liburing.h>
#include <liburing/io_uring.h>
namespace adl {

/* share_from_this? */
class AsyncIORequest {
public:
  enum state {
    INIT,
    READY,
    SUBMITTED,
    COMMITTED,
  };
  AsyncIORequest() : stat(INIT), result_(0), errno_(0) {}
  AsyncIORequest(AsyncIORequest &&rhs) {}
  // bool operator==(const AsyncIORequest &rhs) const { return !(*this == rhs);
  // } bool operator!=(const AsyncIORequest &rhs) const { return !(*this ==
  // rhs); }

  AsyncIORequest &operator=(AsyncIORequest &&rhs) {
    stat = rhs.stat;
    result_ = rhs.result_;
    errno_ = rhs.errno_;
    sqe_ = std::move(rhs.sqe_);
    return *this;
  }

  AsyncIORequest(const AsyncIORequest &) = delete;
  AsyncIORequest &operator=(const AsyncIORequest &) = delete;
  ~AsyncIORequest() = default;

  void prep_pwritev(int fd, const iovec *iov, int iovcnt, off_t start) {
    stat = READY;
    io_uring_prep_writev(&sqe_, fd, iov, iovcnt, start);
    io_uring_sqe_set_data(&sqe_, this);
  }
  void prep_preadv(int fd, const iovec *iov, int iovcnt, off_t start) {
    stat = READY;
    io_uring_prep_writev(&sqe_, fd, iov, iovcnt, start);
    io_uring_sqe_set_data(&sqe_, this);
  }
  void prep_pwrite(int fd, const void *buf, unsigned nbytes, off_t offset) {
    stat = READY;
    io_uring_prep_write(&sqe_, fd, buf, nbytes, offset);
    io_uring_sqe_set_data(&sqe_, this);
  }
  void prep_pread(int fd, void *buf, unsigned nbytes, off_t offset) {
    stat = READY;
    io_uring_prep_read(&sqe_, fd, buf, nbytes, offset);
    io_uring_sqe_set_data(&sqe_, this);
  }

  /**
   * @brief 设置 submit_queue_entry 的 flag
   */
  void add_flag(unsigned flags) { sqe_.flags |= flags; }
  void set_flags(unsigned flags) { sqe_.flags = flags; }

  const io_uring_sqe &get_sqe() const { return sqe_; }

  void set_result(int result) { result_ = result; }
  int get_result() { return result_; }

  void set_errno(int err) { errno_ = err; }
  int get_errno() { return errno_; }

private:
  state stat;
  int result_;
  int errno_;
  struct io_uring_sqe sqe_ {}; /* 这里初始化 */
};
} // namespace adl
#endif // ADL_ASYNCIOREQUEST_H__