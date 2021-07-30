#include "../include/AsyncIORequest.hpp"
#include "../include/lib_lib_uring.hpp"
#include <iostream>
#define SPDLOG_FMT_EXTERNAL
#include <fcntl.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int get_file_size(int fd, off_t *size) {
  struct stat st;

  if (fstat(fd, &st) < 0)
    return -1;
  if (S_ISREG(st.st_mode)) {
    *size = st.st_size;
    return 0;
  } else if (S_ISBLK(st.st_mode)) {
    unsigned long long bytes;

    if (ioctl(fd, BLKGETSIZE64, &bytes) != 0)
      return -1;

    *size = bytes;
    return 0;
  }
  return -1;
}

#define QD 32
#define BS (16 * 1024)

static int infd, outfd;

struct io_data {
  int read;
  off_t first_offset, offset;
  size_t first_len;
  struct iovec iov;
};

static void queue_prepped(struct io_uring *ring, struct io_data *data) {
  struct io_uring_sqe *sqe;

  sqe = io_uring_get_sqe(ring);
  assert(sqe);

  if (data->read)
    io_uring_prep_readv(sqe, infd, &data->iov, 1, data->offset);
  else
    io_uring_prep_writev(sqe, outfd, &data->iov, 1, data->offset);

  io_uring_sqe_set_data(sqe, data);
}

static void queue_read(std::unique_ptr<adl::Uring> &uring, off_t block_size,
                       off_t offset) {
  auto read_request = std::make_shared<adl::AsyncReadRequest>(block_size);
  char *buf = new char[block_size];
  read_request->prep_pread(infd, &buf, block_size, offset);
  /* add to uring request list */
  uring->ioRequestsQueue.push_back(read_request);
}

static void queue_write(std::unique_ptr<adl::Uring> &uring, const char *buffer,
                        off_t block_size, off_t offset) {
  auto read_request = std::make_shared<adl::AsyncWriteRequest>(block_size);
  read_request->prep_pwrite(outfd, buffer, block_size, offset);
  /* add to uring request list */
  uring->ioRequestsQueue.push_back(read_request);
}

int copy_file(std::unique_ptr<adl::Uring> &uring, off_t read_left) {
  unsigned long reads, writes;
  struct io_uring_cqe *cqe;
  off_t write_left, offset;
  int ret;

  write_left = read_left;
  writes = reads = offset = 0;

  while (read_left) {
    /* 限制每次的提交块最大 BS 16K */
    off_t block_size = std::min(read_left, (long)BS);
    /* 准备数据和缓冲区，还没真的 prep_read! */
    queue_read(uring, block_size, offset);
    read_left -= block_size;
    offset += block_size;
  }
  /* 提交任务 */
  ret = uring->submit();
  if (ret < 0) {
    spdlog::error("uring submit failed \n", strerror(-ret), -ret);
  }
  /* 等待完成 */
  auto complicate_queue_ptr = uring->wait();
  if (complicate_queue_ptr) {
    auto &&complicate_queue = *complicate_queue_ptr;

    for (auto &&request_ptr : complicate_queue) {
      // std::dynamic_pointer_cast(request);

      /* 没读完 CONTINUE*/
      if (request_ptr->result_ == -1) {
        spdlog::error("Err {}\n", strerror(request_ptr->result_));
      }
      /* 读完*/
      else if (request_ptr->result_ == 0) {
      } else if (request_ptr->result_ < request_ptr->request_size_) {
      }
    }
  }

  while (write_left) {
    int had_reads, got_comp;

    /* Queue up as many reads as we can */
  }

  return 0;
}

int main(int argc, char *argv[]) {
  off_t read_left;
  int ret;

  if (argc < 3) {
    printf("Usage: %s <infile> <outfile>\n", argv[0]);
    return 1;
  }

  infd = open(argv[1], O_RDONLY);
  if (infd < 0) {
    spdlog::error(argv[0], "open: ", argv[1], ": ", strerror(errno));
    return 1;
  }

  outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (outfd < 0) {
    spdlog::error(argv[0], "open: ", argv[2], ": ", strerror(errno));
    return 1;
  }

  std::unique_ptr<adl::Uring> uring = std::make_unique<adl::Uring>(QD);

  if (get_file_size(infd, &read_left))
    return 1;

  ret = copy_file(uring, read_left);

  close(infd);
  close(outfd);
  return ret;
}
