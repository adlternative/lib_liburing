#include "../include/lib_lib_uring.hpp"
#include "../include/AsyncIORequest.hpp"

namespace adl {
int Uring::submit() {
  return submitMany(ioRequestsQueue, ring_size_ - submitted_size_);
}

int Uring::submitOne(const std::shared_ptr<adl::AsyncIORequest> &request) {
  /* lock_guard? */
  int ret = 0;
  auto sqe = get_sqe();
  if (!sqe)
    return 0;
  *sqe = request->get_sqe();
  ioRequestsSet.insert(request);

  ret = io_uring_submit(&ring);
  if (ret == -1)
    return ret;
  /* expect(1 == ret) */
  if (ret == 1)
    request->set_stat(adl::AsyncIORequest::SUBMITTED);
  return ret;
}

int Uring::submitMany(
    std::deque<std::shared_ptr<adl::AsyncIORequest>> &requests, int submit_nr) {
  int ret = 0;
  int count = 0;

  for (auto &&request : requests) {
    /* 获取 io_uring 提交队列 */
    io_uring_sqe *sqe = get_sqe();
    /* 拿不到则说明用满了 */
    if (!sqe)
      break;
    /* 拷贝请求数据到真实的 SQE 中 */
    *sqe = request->get_sqe();
    /* 插入到请求集合中，不然智能指针将死去 */
    ioRequestsSet.insert(request);
    /* 达到请求最大数字则也退出 */
    if (++count == submit_nr)
      break;
  }
  /* 成功提交的请求数量 */
  ret = io_uring_submit(&ring);
  if (ret == -1)
    return ret;
  /* expect(count == ret) */
  /* 设置状态 */
  for (auto iter = requests.begin(); iter != requests.begin() + ret; iter++)
    (*iter)->set_stat(adl::AsyncIORequest::SUBMITTED);
  /* 从队列中去除这些请求 */
  requests.erase(requests.begin(), requests.begin() + ret);
  return ret;
}

std::optional<std::vector<std::shared_ptr<adl::AsyncIORequest>>>
Uring::wait(size_t minRequests, size_t maxRequests) {
  std::vector<std::shared_ptr<adl::AsyncIORequest>> result;
  size_t count = 0;
  io_uring_cqe *cqe = nullptr;
  int ret = 0;

  while (count < maxRequests) {
    cqe = nullptr;
    /* 没有拿到 到达最低阀值，可以不等待退出 */
    if ((ret = io_uring_peek_cqe(&ring, &cqe))) {
      if ((ret == -EAGAIN || ret == -EWOULDBLOCK)) {
        spdlog::info("io_uring_wait_cqe failed: {} errno={} \n", strerror(-ret),
                     -ret);
        if (count > minRequests)
          break;
        else if ((ret = io_uring_wait_cqe(&ring, &cqe))) {
          spdlog::error("io_uring_wait_cqe failed: {} errno={} \n",
                        strerror(-ret), -ret);
          goto err;
        }
      } else {
        spdlog::error("io_uring_peek_cqe failed: {} errno={} \n",
                      strerror(-ret), -ret);
        goto err;
      }
    }
    /* 成功PEEK到 */
    count++;
    adl::AsyncIORequest *request =
        reinterpret_cast<adl::AsyncIORequest *>(io_uring_cqe_get_data(cqe));
    // int save_errno = errno;
    assert(request);
    // request->set_errno(save_errno);
    request->set_result(cqe->res);
    request->set_stat(adl::AsyncIORequest::COMPLETEED);
    auto &&ptr = request->shared_from_this();
    ioRequestsSet.erase(ptr);
    result.push_back(ptr);

    io_uring_cqe_seen(&ring, cqe);
    goto ok;
  }
err:
  return std::nullopt;
ok:
  return {std::move(result)};
}



} // namespace adl