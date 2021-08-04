#ifndef ADL_ERRNO_UTIL_H__
#define ADL_ERRNO_UTIL_H__

#include <cerrno>

namespace adl {
enum ERROR {
  EPERM_ = EPERM,               /* Operation not permitted */
  ENOENT_ = ENOENT,             /* No such file or directory */
  ESRCH_ = ESRCH,               /* No such process */
  EINTR_ = EINTR,               /* Interrupted system call */
  EIO_ = EIO,                   /* I/O error */
  ENXIO_ = ENXIO,               /* No such device or address */
  E2BIG_ = E2BIG,               /* Argument list too long */
  ENOEXEC_ = ENOEXEC,           /* Exec format error */
  EBADF_ = EBADF,               /* Bad file number */
  ECHILD_ = ECHILD,             /* No child processes */
  EAGAIN_ = EAGAIN,             /* Try again */
  ENOMEM_ = ENOMEM,             /* Out of memory */
  EACCES_ = EACCES,             /* Permission denied */
  EFAULT_ = EFAULT,             /* Bad address */
  ENOTBLK_ = ENOTBLK,           /* Block device required */
  EBUSY_ = EBUSY,               /* Device or resource busy */
  EEXIST_ = EEXIST,             /* File exists */
  EXDEV_ = EXDEV,               /* Cross-device link */
  ENODEV_ = ENODEV,             /* No such device */
  ENOTDIR_ = ENOTDIR,           /* Not a directory */
  EISDIR_ = EISDIR,             /* Is a directory */
  EINVAL_ = EINVAL,             /* Invalid argument */
  ENFILE_ = ENFILE,             /* File table overflow */
  EMFILE_ = EMFILE,             /* Too many open files */
  ENOTTY_ = ENOTTY,             /* Not a typewriter */
  ETXTBSY_ = ETXTBSY,           /* Text file busy */
  EFBIG_ = EFBIG,               /* File too large */
  ENOSPC_ = ENOSPC,             /* No space left on device */
  ESPIPE_ = ESPIPE,             /* Illegal seek */
  EROFS_ = EROFS,               /* Read-only file system */
  EMLINK_ = EMLINK,             /* Too many links */
  EPIPE_ = EPIPE,               /* Broken pipe */
  EDOM_ = EDOM,                 /* Math argument out of domain of func */
  ERANGE_ = ERANGE,             /* Math result not representable */
  EDEADLK_ = EDEADLK,           /* Resource deadlock would occur */
  ENAMETOOLONG_ = ENAMETOOLONG, /* File name too long */
  ENOLCK_ = ENOLCK,             /* No record locks available */

  /*
   * This error code is special: arch syscall entry code will return
   * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
   * failures of syscalls that really do exist distinguishable from
   * failures due to attempts to use a nonexistent syscall, syscall
   * implementations should refrain from returning -ENOSYS.
   */
  ENOSYS_ = ENOSYS, /* Invalid system call number */

  ENOTEMPTY_ = ENOTEMPTY,     /* Directory not empty */
  ELOOP_ = ELOOP,             /* Too many symbolic links encountered */
  EWOULDBLOCK_ = EWOULDBLOCK, /* Operation would block */
  ENOMSG_ = ENOMSG,           /* No message of desired type */
  EIDRM_ = EIDRM,             /* Identifier removed */
  ECHRNG_ = ECHRNG,           /* Channel number out of range */
  EL2NSYNC_ = EL2NSYNC,       /* Level 2 not synchronized */
  EL3HLT_ = EL3HLT,           /* Level 3 halted */
  EL3RST_ = EL3RST,           /* Level 3 reset */
  ELNRNG_ = ELNRNG,           /* Link number out of range */
  EUNATCH_ = EUNATCH,         /* Protocol driver not attached */
  ENOCSI_ = ENOCSI,           /* No CSI structure available */
  EL2HLT_ = EL2HLT,           /* Level 2 halted */
  EBADE_ = EBADE,             /* Invalid exchange */
  EBADR_ = EBADR,             /* Invalid request descriptor */
  EXFULL_ = EXFULL,           /* Exchange full */
  ENOANO_ = ENOANO,           /* No anode */
  EBADRQC_ = EBADRQC,         /* Invalid request code */
  EBADSLT_ = EBADSLT,         /* Invalid slot */

  EDEADLOCK_ = EDEADLOCK,

  EBFONT_ = EBFONT,       /* Bad font file format */
  ENOSTR_ = ENOSTR,       /* Device not a stream */
  ENODATA_ = ENODATA,     /* No data available */
  ETIME_ = ETIME,         /* Timer expired */
  ENOSR_ = ENOSR,         /* Out of streams resources */
  ENONET_ = ENONET,       /* Machine is not on the network */
  ENOPKG_ = ENOPKG,       /* Package not installed */
  EREMOTE_ = EREMOTE,     /* Object is remote */
  ENOLINK_ = ENOLINK,     /* Link has been severed */
  EADV_ = EADV,           /* Advertise error */
  ESRMNT_ = ESRMNT,       /* Srmount error */
  ECOMM_ = ECOMM,         /* Communication error on send */
  EPROTO_ = EPROTO,       /* Protocol error */
  EMULTIHOP_ = EMULTIHOP, /* Multihop attempted */
  EDOTDOT_ = EDOTDOT,     /* RFS specific error */
  EBADMSG_ = EBADMSG,     /* Not a data message */
  EOVERFLOW_ = EOVERFLOW, /* Value too large for defined data type */
  ENOTUNIQ_ = ENOTUNIQ,   /* Name not unique on network */
  EBADFD_ = EBADFD,       /* File descriptor in bad state */
  EREMCHG_ = EREMCHG,     /* Remote address changed */
  ELIBACC_ = ELIBACC,     /* Can not access a needed shared library */
  ELIBBAD_ = ELIBBAD,     /* Accessing a corrupted shared library */
  ELIBSCN_ = ELIBSCN,     /* .lib section in a.out corrupted */
  ELIBMAX_ = ELIBMAX,
  /* Attempting to link in too many shared libraries */
  ELIBEXEC_ = ELIBEXEC, /* Cannot exec a shared library directly */
  EILSEQ_ = EILSEQ,     /* Illegal byte sequence */
  ERESTART_ = ERESTART, /* Interrupted system call should be restarted */
  ESTRPIPE_ = ESTRPIPE, /* Streams pipe error */
  EUSERS_ = EUSERS,     /* Too many users */
  ENOTSOCK_ = ENOTSOCK, /* Socket operation on non-socket */
  EDESTADDRREQ_ = EDESTADDRREQ,       /* Destination address required */
  EMSGSIZE_ = EMSGSIZE,               /* Message too long */
  EPROTOTYPE_ = EPROTOTYPE,           /* Protocol wrong type for socket */
  ENOPROTOOPT_ = ENOPROTOOPT,         /* Protocol not available */
  EPROTONOSUPPORT_ = EPROTONOSUPPORT, /* Protocol not supported */
  ESOCKTNOSUPPORT_ = ESOCKTNOSUPPORT, /* Socket type not supported */
  EOPNOTSUPP_ =
      EOPNOTSUPP, /* Operation not supported on transport endpoint   */
  EPFNOSUPPORT_ = EPFNOSUPPORT,   /* Protocol family not supported */
  EAFNOSUPPORT_ = EAFNOSUPPORT,   /* Address family not supported by protocol */
  EADDRINUSE_ = EADDRINUSE,       /* Address already in use */
  EADDRNOTAVAIL_ = EADDRNOTAVAIL, /* Cannot assign requested address */
  ENETDOWN_ = ENETDOWN,           /* Network is down */
  ENETUNREACH_ = ENETUNREACH,     /* Network is unreachable */
  ENETRESET_ = ENETRESET, /* Network dropped connection because of reset */
  ECONNABORTED_ = ECONNABORTED, /* Software caused connection abort */
  ECONNRESET_ = ECONNRESET,     /* Connection reset by peer */
  ENOBUFS_ = ENOBUFS,           /* No buffer space available */
  EISCONN_ = EISCONN,           /* Transport endpoint is already connected */
  ENOTCONN_ = ENOTCONN,         /* Transport endpoint is not connected */
  ESHUTDOWN_ = ESHUTDOWN, /* Cannot send after transport endpoint shutdown */
  ETOOMANYREFS_ = ETOOMANYREFS, /* Too many references: cannot splice */
  ETIMEDOUT_ = ETIMEDOUT,       /* Connection timed out */
  ECONNREFUSED_ = ECONNREFUSED, /* Connection refused */
  EHOSTDOWN_ = EHOSTDOWN,       /* Host is down */
  EHOSTUNREACH_ = EHOSTUNREACH, /* No route to host */
  EALREADY_ = EALREADY,         /* Operation already in progress */
  EINPROGRESS_ = EINPROGRESS,   /* Operation now in progress */
  ESTALE_ = ESTALE,             /* Stale file handle */
  EUCLEAN_ = EUCLEAN,           /* Structure needs cleaning */
  ENOTNAM_ = ENOTNAM,           /* Not a XENIX named type file */
  ENAVAIL_ = ENAVAIL,           /* No XENIX semaphores available */
  EISNAM_ = EISNAM,             /* Is a named type file */
  EREMOTEIO_ = EREMOTEIO,       /* Remote I/O error */
  EDQUOT_ = EDQUOT,             /* Quota exceeded */

  ENOMEDIUM_ = ENOMEDIUM,       /* No medium found */
  EMEDIUMTYPE_ = EMEDIUMTYPE,   /* Wrong medium type */
  ECANCELED_ = ECANCELED,       /* Operation Canceled */
  ENOKEY_ = ENOKEY,             /* Required key not available */
  EKEYEXPIRED_ = EKEYEXPIRED,   /* Key has expired */
  EKEYREVOKED_ = EKEYREVOKED,   /* Key has been revoked */
  EKEYREJECTED_ = EKEYREJECTED, /* Key was rejected by service */

  /* for robust mutexes */
  EOWNERDEAD_ = EOWNERDEAD,           /* Owner died */
  ENOTRECOVERABLE_ = ENOTRECOVERABLE, /* State not recoverable */
  ERFKILL_ = ERFKILL,     /* Operation not possible due to RF-kill */
  EHWPOISON_ = EHWPOISON, /* Memory page has hardware error */
};

const char *errorToString(unsigned char op);

#ifndef FAILURE_RETRY
#define FAILURE_RETRY(expression)                                              \
  ({                                                                           \
    __typeof(expression) __result;                                             \
    do {                                                                       \
      __result = (expression);                                                 \
    } while (__result == -1 && errno == EINTR);                                \
    __result;                                                                  \
  })
#endif

} // namespace adl

#endif // ADL_ERRNO_UTIL_H__