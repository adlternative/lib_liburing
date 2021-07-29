#include "../include/lib_lib_uring.hpp"
#include "../include/AsyncIORequest.hpp"
namespace adl {
int Uring::submitOne(const std::shared_ptr<adl::AsyncIORequest> &request) {
  /* lock_guard? */
  auto sqe = get_sqe();
  if (!sqe)
    return -1;
  *sqe = request->get_sqe();
  return io_uring_submit(&ring);
}

std::vector<adl::AsyncIORequest *> Uring::wait(size_t minRequests,
                                               size_t maxRequests) {
  std::vector<adl::AsyncIORequest *> result;
  size_t count = 0;

  while (count < maxRequests) {
    struct io_uring_cqe *cqe = nullptr;

    /* 成功PEEK到 */
    if (!io_uring_peek_cqe(&ring, &cqe) && cqe) {
      count++;
      adl::AsyncIORequest *request =
          reinterpret_cast<adl::AsyncIORequest *>(io_uring_cqe_get_data(cqe));
      // int save_errno = errno;
      assert(request);
      // request->set_errno(save_errno);
      request->set_result(cqe->res);

      io_uring_cqe_seen(&ring, cqe);
      result.push_back(request);
    } else if (count < minRequests) {
      /* 需不需要错误处理？ */
      /* 否则 WAIT */
      io_uring_wait_cqe(&ring, &cqe);
    } else {
      break;
    }
  }

  return std::move(result);
}
} // namespace adl