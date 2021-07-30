#ifndef ADL_ASYNCIOREQUEST_H__
#define ADL_ASYNCIOREQUEST_H__

#include "./lib_lib_uring.hpp"
#include <liburing.h>
#include <liburing/io_uring.h>
#include <memory>
namespace adl {

/* share_from_this? */
class AsyncIORequest : public std::enable_shared_from_this<AsyncIORequest> {
public:
  enum state {
    INIT,
    READY,
    SUBMITTED,
    COMPLETEED,
  };
  AsyncIORequest() : stat_(INIT), result_(0), errno_(0) {}
  AsyncIORequest(AsyncIORequest &&rhs)
      : stat_(rhs.stat_), result_(rhs.result_), errno_(rhs.errno_) {}
  AsyncIORequest &operator=(AsyncIORequest &&rhs) {
    stat_ = rhs.stat_;
    result_ = rhs.result_;
    errno_ = rhs.errno_;
    sqe_ = std::move(rhs.sqe_);
    return *this;
  }

  AsyncIORequest(const AsyncIORequest &) = delete;
  AsyncIORequest &operator=(const AsyncIORequest &) = delete;
  virtual ~AsyncIORequest() = default;

  virtual void prep_pwritev(int fd, const iovec *iov, int iovcnt, off_t start) {
    stat_ = READY;
    io_uring_prep_writev(&sqe_, fd, iov, iovcnt, start);
    io_uring_sqe_set_data(&sqe_, this);
  }
  virtual void prep_preadv(int fd, const iovec *iov, int iovcnt, off_t start) {
    stat_ = READY;
    io_uring_prep_readv(&sqe_, fd, iov, iovcnt, start);
    io_uring_sqe_set_data(&sqe_, this);
  }
  virtual void prep_pwrite(int fd, const void *buf, unsigned nbytes,
                           off_t offset) {
    stat_ = READY;
    io_uring_prep_write(&sqe_, fd, buf, nbytes, offset);
    io_uring_sqe_set_data(&sqe_, this);
  }
  virtual void prep_pread(int fd, void *buf, unsigned nbytes, off_t offset) {
    stat_ = READY;
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

  void set_stat(state stat) { stat_ = stat; }

private:
  state stat_;
  int result_;
  int errno_;
  struct io_uring_sqe sqe_ {}; /* 这里初始化为 0 */
};

class AsyncReadRequest : public AsyncIORequest {
public:
  AsyncReadRequest(int request_size = 0)
      : request_size_(request_size), offset_(0), AsyncIORequest() {}

  virtual void prep_pread(int fd, void *buffer, unsigned nbytes,
                          off_t offset) override {
    AsyncIORequest::prep_pread(fd, buffer, nbytes, offset);
    offset_ = offset;
    buffer_ = buffer;
  }

  off_t request_size_;
  off_t offset_;
  void *buffer_;
};

class AsyncWriteRequest : public AsyncIORequest {
public:
  AsyncWriteRequest(int request_size = 0)
      : request_size_(request_size), offset_(0), AsyncIORequest() {}

  virtual void prep_pwrite(int fd, const void *buffer, unsigned nbytes,
                           off_t offset) override {
    AsyncIORequest::prep_pwrite(fd, buffer, nbytes, offset);
    offset_ = offset;
    buffer_ = buffer;
  }

  off_t request_size_;
  off_t offset_;
  const void *buffer_;
};

} // namespace adl

#endif // ADL_ASYNCIOREQUEST_H__