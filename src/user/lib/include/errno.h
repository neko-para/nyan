#pragma once

#include <nyan/errno.h>
#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

// Operation not permitted
#define EPERM SYS_EPERM

// No such file or directory
#define ENOENT SYS_ENOENT

// No such process
#define ESRCH SYS_ESRCH

// Interrupted system call
#define EINTR SYS_EINTR

// Input/output error
#define EIO SYS_EIO

// No such device or address
#define ENXIO SYS_ENXIO

// Argument list too long
#define E2BIG SYS_E2BIG

// Exec format error
#define ENOEXEC SYS_ENOEXEC

// Bad file descriptor
#define EBADF SYS_EBADF

// No child processes
#define ECHILD SYS_ECHILD

// Resource temporarily unavailable
#define EAGAIN SYS_EAGAIN

// Cannot allocate memory
#define ENOMEM SYS_ENOMEM

// Permission denied
#define EACCES SYS_EACCES

// Bad address
#define EFAULT SYS_EFAULT

// Block device required
#define ENOTBLK SYS_ENOTBLK

// Device or resource busy
#define EBUSY SYS_EBUSY

// File exists
#define EEXIST SYS_EEXIST

// Invalid cross-device link
#define EXDEV SYS_EXDEV

// No such device
#define ENODEV SYS_ENODEV

// Not a directory
#define ENOTDIR SYS_ENOTDIR

// Is a directory
#define EISDIR SYS_EISDIR

// Invalid argument
#define EINVAL SYS_EINVAL

// Too many open files in system
#define ENFILE SYS_ENFILE

// Too many open files
#define EMFILE SYS_EMFILE

// Inappropriate ioctl for device
#define ENOTTY SYS_ENOTTY

// Text file busy
#define ETXTBSY SYS_ETXTBSY

// File too large
#define EFBIG SYS_EFBIG

// No space left on device
#define ENOSPC SYS_ENOSPC

// Illegal seek
#define ESPIPE SYS_ESPIPE

// Read-only file system
#define EROFS SYS_EROFS

// Too many links
#define EMLINK SYS_EMLINK

// Broken pipe
#define EPIPE SYS_EPIPE

// Numerical argument out of domain
#define EDOM SYS_EDOM

// Numerical result out of range
#define ERANGE SYS_ERANGE

// Resource deadlock avoided
#define EDEADLK SYS_EDEADLK

// File name too long
#define ENAMETOOLONG SYS_ENAMETOOLONG

// No locks available
#define ENOLCK SYS_ENOLCK

// Function not implemented
#define ENOSYS SYS_ENOSYS

// Directory not empty
#define ENOTEMPTY SYS_ENOTEMPTY

// Too many levels of symbolic links
#define ELOOP SYS_ELOOP

// Resource temporarily unavailable
#define EWOULDBLOCK SYS_EWOULDBLOCK

// No message of desired type
#define ENOMSG SYS_ENOMSG

// Identifier removed
#define EIDRM SYS_EIDRM

// Channel number out of range
#define ECHRNG SYS_ECHRNG

// Level 2 not synchronized
#define EL2NSYNC SYS_EL2NSYNC

// Level 3 halted
#define EL3HLT SYS_EL3HLT

// Level 3 reset
#define EL3RST SYS_EL3RST

// Link number out of range
#define ELNRNG SYS_ELNRNG

// Protocol driver not attached
#define EUNATCH SYS_EUNATCH

// No CSI structure available
#define ENOCSI SYS_ENOCSI

// Level 2 halted
#define EL2HLT SYS_EL2HLT

// Invalid exchange
#define EBADE SYS_EBADE

// Invalid request descriptor
#define EBADR SYS_EBADR

// Exchange full
#define EXFULL SYS_EXFULL

// No anode
#define ENOANO SYS_ENOANO

// Invalid request code
#define EBADRQC SYS_EBADRQC

// Invalid slot
#define EBADSLT SYS_EBADSLT

// Resource deadlock avoided
#define EDEADLOCK SYS_EDEADLOCK

// Bad font file format
#define EBFONT SYS_EBFONT

// Device not a stream
#define ENOSTR SYS_ENOSTR

// No data available
#define ENODATA SYS_ENODATA

// Timer expired
#define ETIME SYS_ETIME

// Out of streams resources
#define ENOSR SYS_ENOSR

// Machine is not on the network
#define ENONET SYS_ENONET

// Package not installed
#define ENOPKG SYS_ENOPKG

// Object is remote
#define EREMOTE SYS_EREMOTE

// Link has been severed
#define ENOLINK SYS_ENOLINK

// Advertise error
#define EADV SYS_EADV

// Srmount error
#define ESRMNT SYS_ESRMNT

// Communication error on send
#define ECOMM SYS_ECOMM

// Protocol error
#define EPROTO SYS_EPROTO

// Multihop attempted
#define EMULTIHOP SYS_EMULTIHOP

// RFS specific error
#define EDOTDOT SYS_EDOTDOT

// Bad message
#define EBADMSG SYS_EBADMSG

// Value too large for defined data type
#define EOVERFLOW SYS_EOVERFLOW

// Name not unique on network
#define ENOTUNIQ SYS_ENOTUNIQ

// File descriptor in bad state
#define EBADFD SYS_EBADFD

// Remote address changed
#define EREMCHG SYS_EREMCHG

// Can not access a needed shared library
#define ELIBACC SYS_ELIBACC

// Accessing a corrupted shared library
#define ELIBBAD SYS_ELIBBAD

// .lib section in a.out corrupted
#define ELIBSCN SYS_ELIBSCN

// Attempting to link in too many shared libraries
#define ELIBMAX SYS_ELIBMAX

// Cannot exec a shared library directly
#define ELIBEXEC SYS_ELIBEXEC

// Invalid or incomplete multibyte or wide character
#define EILSEQ SYS_EILSEQ

// Interrupted system call should be restarted
#define ERESTART SYS_ERESTART

// Streams pipe error
#define ESTRPIPE SYS_ESTRPIPE

// Too many users
#define EUSERS SYS_EUSERS

// Socket operation on non-socket
#define ENOTSOCK SYS_ENOTSOCK

// Destination address required
#define EDESTADDRREQ SYS_EDESTADDRREQ

// Message too long
#define EMSGSIZE SYS_EMSGSIZE

// Protocol wrong type for socket
#define EPROTOTYPE SYS_EPROTOTYPE

// Protocol not available
#define ENOPROTOOPT SYS_ENOPROTOOPT

// Protocol not supported
#define EPROTONOSUPPORT SYS_EPROTONOSUPPORT

// Socket type not supported
#define ESOCKTNOSUPPORT SYS_ESOCKTNOSUPPORT

// Operation not supported
#define EOPNOTSUPP SYS_EOPNOTSUPP

// Protocol family not supported
#define EPFNOSUPPORT SYS_EPFNOSUPPORT

// Address family not supported by protocol
#define EAFNOSUPPORT SYS_EAFNOSUPPORT

// Address already in use
#define EADDRINUSE SYS_EADDRINUSE

// Cannot assign requested address
#define EADDRNOTAVAIL SYS_EADDRNOTAVAIL

// Network is down
#define ENETDOWN SYS_ENETDOWN

// Network is unreachable
#define ENETUNREACH SYS_ENETUNREACH

// Network dropped connection on reset
#define ENETRESET SYS_ENETRESET

// Software caused connection abort
#define ECONNABORTED SYS_ECONNABORTED

// Connection reset by peer
#define ECONNRESET SYS_ECONNRESET

// No buffer space available
#define ENOBUFS SYS_ENOBUFS

// Transport endpoint is already connected
#define EISCONN SYS_EISCONN

// Transport endpoint is not connected
#define ENOTCONN SYS_ENOTCONN

// Cannot send after transport endpoint shutdown
#define ESHUTDOWN SYS_ESHUTDOWN

// Too many references: cannot splice
#define ETOOMANYREFS SYS_ETOOMANYREFS

// Connection timed out
#define ETIMEDOUT SYS_ETIMEDOUT

// Connection refused
#define ECONNREFUSED SYS_ECONNREFUSED

// Host is down
#define EHOSTDOWN SYS_EHOSTDOWN

// No route to host
#define EHOSTUNREACH SYS_EHOSTUNREACH

// Operation already in progress
#define EALREADY SYS_EALREADY

// Operation now in progress
#define EINPROGRESS SYS_EINPROGRESS

// Stale file handle
#define ESTALE SYS_ESTALE

// Structure needs cleaning
#define EUCLEAN SYS_EUCLEAN

// Not a XENIX named type file
#define ENOTNAM SYS_ENOTNAM

// No XENIX semaphores available
#define ENAVAIL SYS_ENAVAIL

// Is a named type file
#define EISNAM SYS_EISNAM

// Remote I/O error
#define EREMOTEIO SYS_EREMOTEIO

// Disk quota exceeded
#define EDQUOT SYS_EDQUOT

// No medium found
#define ENOMEDIUM SYS_ENOMEDIUM

// Wrong medium type
#define EMEDIUMTYPE SYS_EMEDIUMTYPE

// Operation canceled
#define ECANCELED SYS_ECANCELED

// Required key not available
#define ENOKEY SYS_ENOKEY

// Key has expired
#define EKEYEXPIRED SYS_EKEYEXPIRED

// Key has been revoked
#define EKEYREVOKED SYS_EKEYREVOKED

// Key was rejected by service
#define EKEYREJECTED SYS_EKEYREJECTED

// Owner died
#define EOWNERDEAD SYS_EOWNERDEAD

// State not recoverable
#define ENOTRECOVERABLE SYS_ENOTRECOVERABLE

// Operation not possible due to RF-kill
#define ERFKILL SYS_ERFKILL

// Memory page has hardware error
#define EHWPOISON SYS_EHWPOISON

// Operation not supported
#define ENOTSUP SYS_ENOTSUP

extern int __errno;

#define errno (__errno)
