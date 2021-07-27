#include "../include/lib_lib_uring.hpp"
#include "../include/Cqe.hpp"
#include "../include/Sqe.hpp"

namespace adl {
struct Sqe *Uring::create_Sqe() {
  if (auto sqe = get_sqe())
    return new Sqe(sqe);
  return nullptr;
}

struct Cqe *Uring::wait_Cqe() {
  if (auto cqe = wait())
    return new Cqe(cqe);
  return nullptr;
}

struct Cqe *Uring::non_block_wait_Cqe() {
  /* expection? errno? */
  if (auto cqe = non_block_wait())
    return new Cqe(cqe);
  return nullptr;
}

} // namespace adl