#include "../include/lib_lib_uring.hpp"
#include "../include/AsyncIORequest.hpp"
#include "../include/errno_util.hpp"
namespace adl {

/**
 * @brief 尽可能多的提交请求
 *
 * @return int 成功提交的请求数量 | 失败返回-1
 */
int Uring::submit() {
  int ret = 0;
  if (ioRequestsQueue.empty()) {
    return 0;
  }
  for (auto &&request : ioRequestsQueue) {
    if (exited)
      return -1;
    /* 获取 io_uring 提交队列 */
    if (io_uring_sqe *sqe = io_uring_get_sqe(&ring)) {
      /* 拿不到则说明用满了 */
      /* 拷贝请求数据到真实的 SQE 中 */
      *sqe = request->get_sqe();
      /* 插入到请求集合中，不然智能指针将死去 */
      ioRequestsSet.insert(request);
    }
  }
  if ((ret = io_uring_submit(&ring)) < 0) {
    errno_ = errno;
    throw std::system_error(errno_, std::system_category(), "io_uring_submit");
  }
  /* 给这些成功提交的请求设置状态 */
  for (auto iter = ioRequestsQueue.begin();
       iter != ioRequestsQueue.begin() + ret; iter++)
    (*iter)->set_stat(adl::AsyncIORequest::SUBMITTED);
  /* 从队列中去除这些请求 */
  ioRequestsQueue.erase(ioRequestsQueue.begin(), ioRequestsQueue.begin() + ret);
  return ret;
}

/**
 * @brief 主动等待任务完成
 *
 * @param minRequests 等待完成的最小数量
 * @param maxRequests 等待完成的最大数量
 * @return std::optional<std::vector<std::shared_ptr<adl::AsyncIORequest>>>
 */
std::optional<std::vector<std::shared_ptr<adl::AsyncIORequest>>>
Uring::wait(size_t minRequests, size_t maxRequests) {
  std::vector<std::shared_ptr<adl::AsyncIORequest>> result;
  size_t count = 0;
  io_uring_cqe *cqe = nullptr;
  int ret = 0;
  adl::AsyncIORequest *request = nullptr;
  std::shared_ptr<adl::AsyncIORequest> request_ptr;

  while (count < maxRequests) {
    cqe = nullptr;

    if (exited)
      return std::nullopt;
    if ((ret = (io_uring_peek_cqe(&ring, &cqe))) < 0) {
      /* 没有拿到  */
      if ((ret != -EAGAIN && ret != -EWOULDBLOCK)) {
        errno_ = errno;
        throw std::system_error(errno_, std::system_category(),
                                "io_uring_peek_cqe");
      } else if (count >= minRequests) {
        /*  到达最低阀值，可以不等待退出 */
        break;
        /*  否则再多等一会儿 */
      } else if ((ret = io_uring_wait_cqe(&ring, &cqe)) < 0) {
        errno_ = errno;
        throw std::system_error(errno_, std::system_category(),
                                "io_uring_wait_cqe");
      }
    }
    /* 成功PEEK到 */
    count++;
    request =
        reinterpret_cast<adl::AsyncIORequest *>(io_uring_cqe_get_data(cqe));
    assert(request);
    request->set_result(cqe->res);
    request->set_stat(adl::AsyncIORequest::COMPLETEED);
    request_ptr = request->shared_from_this();
    /* 从暂存请求的集合中删除 */
    ioRequestsSet.erase(request_ptr);
    /* 添加到结果列表中 */
    result.push_back(request_ptr);
    /* 表示我们已经消费了该CQE了 */
    io_uring_cqe_seen(&ring, cqe);
  }
  return {result};
}

} // namespace adl