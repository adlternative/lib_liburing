#ifndef ADL_FILE_UTIL_H__
#define ADL_FILE_UTIL_H__

#include <fcntl.h>
#include <linux/fs.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>
#include <utility>

namespace adl {
/**
 * @brief 文件描述符类
 *
 */
struct Fd {

  explicit Fd(int fd) : fd_(fd) {
    if (fd_ == -1)
      throw std::system_error(errno, std::system_category(), "Fd::Fd");
  }

  /* 不可拷贝 */
  Fd(const Fd &) = delete;
  Fd &operator=(const Fd &) = delete;
  /* 可移动 */
  Fd(Fd &&rhs) noexcept { fd_ = std::exchange(rhs.fd_, -1); }
  Fd &operator=(Fd &&rhs) noexcept {
    if (this != &rhs)
      fd_ = std::exchange(rhs.fd_, -1);
    return *this;
  }
  Fd &operator=(int fd) {
    if (fd_ != -1)
      close();
    fd_ = fd;
    return *this;
  }
  /* 析构会自动 close() */
  ~Fd() {
    if (fd_ != -1)
      close();
  }
  operator int() { return fd_; }
  int get_fd() { return fd_; }

  off_t get_file_size() {
    struct stat st;

    if (fd_ == -1)
      throw std::runtime_error("Fd::get_file_size: fd_ == -1");

    if (fstat(fd_, &st) < 0)
      throw std::system_error(errno, std::system_category(), "fstat");

    if (S_ISREG(st.st_mode)) {
      return st.st_size;
    } else if (S_ISBLK(st.st_mode)) {
      unsigned long long bytes;

      if (ioctl(fd_, BLKGETSIZE64, &bytes) != 0)
        throw std::system_error(errno, std::system_category(), "ioctl");
      return bytes;
    } else {
      throw std::runtime_error(
          "Temporarily unable to process other types of files");
    }
    return 0;
  }

protected:
  void close() {
    if (fd_ != -1)
      ::close(fd_);
  }
  int fd_ = -1;
};

} // namespace adl
#endif // ADL_FILE_UTIL_H__