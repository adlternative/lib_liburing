#ifndef ADL_ASYNCIOREQUEST_H__
#define ADL_ASYNCIOREQUEST_H__

#include "./lib_lib_uring.hpp"
#include <cstring>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <memory>
#include <strings.h>
namespace adl {

/* share_from_this? */
class AsyncIORequest : public std::enable_shared_from_this<AsyncIORequest> {
public:
  enum state {
    INIT,
    READY,
    SUBMITTED,
    COMPLETEED,
    FINISH,
    CLOSED,
    ERROR,
  };
  AsyncIORequest() : stat_(INIT), result_(0), errno_(0) {}
  AsyncIORequest(AsyncIORequest &&rhs) noexcept
      : stat_(rhs.stat_), result_(rhs.result_), errno_(rhs.errno_),
        sqe_(std::move(rhs.sqe_)) {
    set_finish_callback(default_finish_callback);
  }
  AsyncIORequest &operator=(AsyncIORequest &&rhs) noexcept {
    if (this != &rhs) {
      stat_ = std::exchange(rhs.stat_, INIT);
      result_ = std::exchange(rhs.result_, 0);
      errno_ = std::exchange(rhs.errno_, 0);
      sqe_ = std::exchange(rhs.sqe_, {});
    }
    return *this;
  }

  AsyncIORequest(const AsyncIORequest &) = delete;
  AsyncIORequest &operator=(const AsyncIORequest &) = delete;
  virtual ~AsyncIORequest() = default;

  /**
   * @brief 设置 submit_queue_entry 的 flag 比如 IOSQE_IO_LINK
   */
  void add_flag(unsigned flags) { sqe_.flags |= flags; }
  void set_flags(unsigned flags) { sqe_.flags = flags; }

  const io_uring_sqe &get_sqe() const { return sqe_; }

  void set_result(int result) { result_ = result; }
  int get_result() { return result_; }

  // void set_errno(int err) { errno_ = err; }
  int get_errno() { return -result_; }

  void set_stat(state stat) { stat_ = stat; }
  state get_stat() { return stat_; }

  virtual void *complete_call_back() { return nullptr; }
  void *finish_callback() { return finish_cb_(); }
  void set_finish_callback(std::function<void *()> &&fn) {
    finish_cb_ = std::move(fn);
  }
  void set_finish_callback(const std::function<void *()> &fn) {
    finish_cb_ = fn;
  }
  static void *default_finish_callback() { return nullptr; }
  // void set_complete_call_back(std::function<void *()> fn) { complete_cb = fn;
  // }
protected:
  state stat_;
  int result_;
  int errno_;
  struct io_uring_sqe sqe_ {}; /* 这里初始化为 0 */
  std::function<void *()> finish_cb_;
  // std::function<void *()> complete_cb;
};

class AsyncReadRequest : public AsyncIORequest {
public:
  AsyncReadRequest(int fd, void *buffer, unsigned nbytes, off_t offset)
      : AsyncIORequest(), request_size_(nbytes), offset_(offset), fd_(fd),
        cur_(buffer), buffer_(buffer) {
    stat_ = READY;
    io_uring_prep_read(&sqe_, fd_, buffer_, request_size_, offset_);
    io_uring_sqe_set_data(&sqe_, this);
  }
  AsyncReadRequest(AsyncReadRequest &&rhs) noexcept
      : AsyncIORequest(std::move(rhs)), request_size_(rhs.request_size_),
        offset_(rhs.offset_), fd_(rhs.fd_), cur_(rhs.cur_) {}
  AsyncReadRequest &operator=(AsyncReadRequest &&rhs) noexcept {
    if (this != &rhs) {
      *this = std::move(rhs);
      request_size_ = std::exchange(rhs.request_size_, 0);
      offset_ = std::exchange(rhs.offset_, 0);
      fd_ = std::exchange(rhs.fd_, 0);
      cur_ = rhs.cur_;
    }
    return *this;
  }

  virtual void *complete_call_back() override {
    if (result_ < 0) {
      /* 读出错 */
      spdlog::error("Err: {}\n", strerror(-result_));
      stat_ = ERROR;
    } else if (result_ == 0) {
      /* 读完 */
      spdlog::info("fd Close: {}\n", strerror(-result_));
      stat_ = CLOSED; /* 任务ok 删除任务？*/
      /* close fd? */
    } else {
      spdlog::info("Read result:{}", result_);

      /* 没读完则自重启 */
      if (request_size_ == result_) {
        cur_ = (char *)cur_ + result_;
        request_size_ -= result_;
        offset_ += result_;
        stat_ = FINISH;
      } else if (request_size_ > result_) {
        spdlog::info("Need read again!");
        cur_ = (char *)cur_ + result_;
        request_size_ -= result_;
        offset_ += result_;
        ::explicit_bzero(&sqe_, sizeof(sqe_));
        io_uring_prep_read(&sqe_, fd_, cur_, request_size_, offset_);
        io_uring_sqe_set_data(&sqe_, this);
        stat_ = READY;
      } else {
        /* bug? */
        spdlog::error("something wrong? reques_size_={}, result_={}",
                      request_size_, result_);
        stat_ = ERROR;
      }
    }
    return nullptr;
  }

  void *get_buffer() { return buffer_; }
  int get_processed_size() { return (char *)cur_ - (char *)buffer_; }

private:
  unsigned request_size_; /* 当前请求大小 */
  off_t offset_;          /* 文件偏移量 */
  void *cur_;             /* 当前读到哪里了 */
  void *buffer_;          /* 原始传递的 buffer 起始地址 */
  int fd_;                /* 对应的文件描述符 */
};

class AsyncWriteRequest : public AsyncIORequest {
public:
  AsyncWriteRequest(int fd, void *buffer, unsigned nbytes, off_t offset)
      : AsyncIORequest(), request_size_(nbytes), offset_(offset), fd_(fd),
        cur_(buffer), buffer_(buffer) {
    stat_ = READY;
    io_uring_prep_write(&sqe_, fd_, buffer_, request_size_, offset_);
    io_uring_sqe_set_data(&sqe_, this);
  }
  AsyncWriteRequest(AsyncWriteRequest &&rhs) noexcept
      : AsyncIORequest(std::move(rhs)), request_size_(rhs.request_size_),
        offset_(rhs.offset_), fd_(rhs.fd_), cur_(rhs.cur_) {}
  AsyncWriteRequest &operator=(AsyncWriteRequest &&rhs) noexcept {
    if (this != &rhs) {
      *this = std::move(rhs);
      request_size_ = std::exchange(rhs.request_size_, 0);
      offset_ = std::exchange(rhs.offset_, 0);
      fd_ = std::exchange(rhs.fd_, 0);
      cur_ = rhs.cur_;
    }
    return *this;
  }

  virtual void *complete_call_back() override {
    if (result_ < 0) {
      spdlog::error("complete_call_back error: {}\n", strerror(-result_));
      stat_ = ERROR;
    } else {

      if (request_size_ == result_) {
        cur_ = (char *)cur_ + result_;
        request_size_ -= result_;
        offset_ += result_;
        stat_ = FINISH; /* 任务ok 删除任务？*/
      } else if (request_size_ > result_) {
        /* 没发完则自重启 */
        cur_ = (char *)cur_ + result_;
        request_size_ -= result_;
        offset_ += result_;
        ::explicit_bzero(&sqe_, sizeof(sqe_));
        io_uring_prep_write(&sqe_, fd_, cur_, request_size_, offset_);
        io_uring_sqe_set_data(&sqe_, this);
        stat_ = READY;
      } else {
        /* bug? */
        spdlog::error("Something wrong? reques_size_={}, result_={}",
                      request_size_, result_);
        stat_ = ERROR;
      }
    }
    return nullptr;
  }
  void *get_buffer() { return buffer_; }
  int get_processed_size() { return (char *)cur_ - (char *)buffer_; }

  unsigned request_size_; /* 当前请求大小 */
  off_t offset_;          /* 文件偏移量 */
  void *cur_;             /* 当前读到哪里了 */
  void *buffer_;          /* 原始传递的 buffer 起始地址 */
  int fd_;                /* 对应的文件描述符 */
};

} // namespace adl

#endif // ADL_ASYNCIOREQUEST_H__