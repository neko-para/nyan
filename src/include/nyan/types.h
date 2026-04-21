#pragma once

#ifdef __cplusplus
#define __NYAN_EXTERNC_BEGIN__ extern "C" {
#define __NYAN_EXTERNC_END__ }
#else
#define __NYAN_EXTERNC_BEGIN__
#define __NYAN_EXTERNC_END__
#endif

#ifdef __BUILDING_NYAN__

#define __NYAN_BEGIN__ namespace nyan {
#define __NYAN_END__ }

#define __NYAN_SYSCALL_BEGIN__ namespace nyan::syscall {
#define __NYAN_SYSCALL_END__ }
#define __NYAN_SYSCALL__(name) name

#else

#define __NYAN_BEGIN__
#define __NYAN_END__

#define __NYAN_SYSCALL_BEGIN__ __NYAN_EXTERNC_BEGIN__
#define __NYAN_SYSCALL_END__ __NYAN_EXTERNC_END__
#define __NYAN_SYSCALL__(name) sys_##name

#endif
