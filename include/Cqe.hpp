#ifndef __CQE_H__
#define __CQE_H__

#include <liburing.h>

namespace adl {
class Cqe {
public:
  Cqe(struct io_uring_cqe *cqe) : cqe_(cqe) {}
  ~Cqe() {}

  void *get_data() { return io_uring_cqe_get_data(cqe_); }
  unsigned int get_result() { return cqe_->res; }

private:
  struct io_uring_cqe *cqe_;
};
} // namespace adl

#endif // __CQE_H__