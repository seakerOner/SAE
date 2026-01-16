#ifndef CORE_BASE_H
#define CORE_BASE_H

// Base implementation of engine CORE
//
// This file contains code used internally by the engine across the code base
// and definitions of implementations of SEAKCUTILS
//
//
// `CORE_BASE_IMPLEMENTATION` should be the first definition in the engine

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SEAKCUTILS IMPLEMENTATION
//
#include "./seakcutils/strings/mystrings.h"
#ifndef MYSTRINGS_IMPLEMENTATION
#define MYSTRINGS_IMPLEMENTATION
#endif

#include "./seakcutils/arenas/arena.h"
#ifndef ARENA_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#endif

#include "./seakcutils/arenas/r_arena.h"
#ifndef REGION_ARENA_IMPLEMENTATION
#define REGION_ARENA_IMPLEMENTATION
#endif

#include "./seakcutils/data_structures/deque.h"
#ifndef DEQUE_IMPLEMENTATION
#define DEQUE_IMPLEMENTATION
#endif

#include "./seakcutils/data_structures/linkedlist.h"
#ifndef LINKEDLIST_IMPLEMENTATION
#define LINKEDLIST_IMPLEMENTATION
#endif

#include "./seakcutils/channels/channels.h"
#ifndef CHANNEL_BASICS_IMPLEMENTATION
#define CHANNEL_BASICS_IMPLEMENTATION
#endif

#include "./seakcutils/channels/spmc.h"
#ifndef SPMC_IMPLEMENTATION
#define SPMC_IMPLEMENTATION
#endif

#include "./seakcutils/channels/mpmc.h"
#ifndef MPMC_IMPLEMENTATION
#define MPMC_IMPLEMENTATION
#endif

#include "./seakcutils/threadpool/threadpool.h"
#ifndef THREADPOOL_IMPLEMENTATION
#define THREADPOOL_IMPLEMENTATION
#endif

#include "./seakcutils/job_system/jobsystem.h"
#ifndef JOBSYSTEM_IMPLEMENTATION
#define JOBSYSTEM_IMPLEMENTATION
#endif

#ifndef UNUSED
#define UNUSED(v) (void)v
#endif

typedef uint8_t bool;
#define TRUE 1
#define FALSE 0
typedef size_t usize;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float float32;
typedef double float64;
typedef uintptr_t uintptr;

typedef struct SAE_TimeStamp_t {
  u64 seconds;
  u64 microseconds;
} SAE_TimeStamp;

// get a 'line' from a buffer until `breakpoint` is reached and copies to 'out'
// returns '0' if EOF (\0)
// returns on success 'N' as number of ascii in the line
// returns -1 if '__n' ascii is reached and breakpoint was not found
int __get_line(const ascii *buf, const ascii breakpoint, const size_t __n,
               ascii *out);

/* FIX: [LINUX][PERIPHERAL] : Sometimes lines break on a random place of the
 * line, seems to be rare. Maybe somehow there is a '\n' on the middle of the
 * line ?? (RELATED TO /proc/bus/input/devices)*/

typedef struct AsciiLine_t {
  ascii *line;
  usize len;
} AsciiLine;

int ascii_starts_with(const ascii *str, const ascii *prefix, const usize __n);
int ascii_contains(const ascii *str, const usize _str_n, const ascii *comp,
                   const usize _comp_n);
int ascii_is_number(const ascii c);
int ascii_contains_ret_idx(const ascii *str, const usize _str_n,
                           const ascii *comp, const usize _comp_n);

#if defined(SAE_RELEASE) && SAE_RELEASE == 0
#define SAE_FAILURE                                                            \
  { exit(EXIT_FAILURE); }
#else
#define SAE_FAILURE
#endif

#if defined(SAE_DEBUG) && SAE_DEBUG == 1
#define SAE_ASSERT(expr)                                                       \
  { assert(expr); }

#define SAE_ASSERT_MSG(expr, msg, ...)                                         \
  {                                                                            \
    if (!expr) {                                                               \
      fprintf(stderr, msg, __VA_ARGS__);                                       \
      SAE_TRACE                                                                \
      SAE_FAILURE                                                              \
    }                                                                          \
  }

#define SAE_ERROR_ARGS(msg, ...)                                               \
  {                                                                            \
    fprintf(stderr, msg, __VA_ARGS__);                                         \
    SAE_TRACE                                                                  \
    SAE_FAILURE                                                                \
  }

#define SAE_ERROR(msg)                                                         \
  {                                                                            \
    fprintf(stderr, msg);                                                      \
    SAE_TRACE                                                                  \
    SAE_FAILURE                                                                \
  }

#else

// DO NOTHING
#define SAE_ASSERT(expr)
#define SAE_ASSERT_MSG(expr, msg, ...)
#define SAE_ERROR_ARGS(msg, ...)
#define SAE_ERROR(msg)

#endif

#if defined(SAE_TRACER) && SAE_TRACER == 1
#define SAE_TRACE                                                              \
  {                                                                            \
    fprintf(stderr,                                                            \
            "\n[TRACE] File: %s \n[TRACE] Line:%d \n[TRACE] Function:%s \n",   \
            __FILE__, __LINE__, __func__);                                     \
  }

#else
// do nothing
#define SAE_TRACE
#endif

#define SAE_CHECK_ALLOC(ptr, name)                                             \
  {                                                                            \
    if (!ptr) {                                                                \
      SAE_ERROR_ARGS("[FATAL] Memory allocation failed for %s. \n", name)      \
    }                                                                          \
  }

#define SAE_GOTO(label) goto label

#define SAE_CHECK_ALLOC_AND(ptr, name, closure)                                \
  {                                                                            \
    if (!ptr) {                                                                \
      closure;                                                                 \
      SAE_ERROR_ARGS("[FATAL] Memory allocation failed for %s. \n", name)      \
    }                                                                          \
  }

#endif
