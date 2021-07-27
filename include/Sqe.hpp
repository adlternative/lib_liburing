#ifndef __SQE_H__
#define __SQE_H__
#include "./lib_lib_uring.hpp"
#include <liburing.h>
namespace adl {

class Sqe {
public:
  /* Sqe 没有 sqe 的所有权 */
  Sqe(io_uring_sqe *sqe) : sqe_(sqe) {}
  ~Sqe() {}

  // static Sqe *create_impl(Uring &u) {
  // }

  void prep_pwritev() {}
  void prep_preadv() {}
  void prep_pwrite(int fd, const void *buf, unsigned nbytes, off_t offset) {
    io_uring_prep_write(sqe_, fd, buf, nbytes, offset);
  }
  void prep_pread(int fd, void *buf, unsigned nbytes, off_t offset) {
    io_uring_prep_read(sqe_, fd, buf, nbytes, offset);
  }
  void set_data() {}

  /**
   * @brief 设置 submit_queue_entry 的 flag
   *
   */
  void add_flag(unsigned flags) { sqe_->flags |= flags; }
  void set_flags(unsigned flags) { sqe_->flags = flags; }

private:
  struct io_uring_sqe *sqe_;
};
} // namespace adl
#endif // __SQE_H__