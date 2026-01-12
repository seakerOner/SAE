#ifndef CORE_BASE_H
#define CORE_BASE_H

// Base implementation of engine CORE
//
// This file contains code used internally by the engine across the code base
// and definitions of implementations of SEAKCUTILS
//
//
// `CORE_BASE_IMPLEMENTATION` should be the first definition in the engine

#include "./seakcutils/arenas/arena.h"
#include "./seakcutils/data_structures/deque.h"
#include "./seakcutils/strings/mystrings.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNUSED
#define UNUSED(v) (void)v
#endif

// get a 'line' from a buffer until `breakpoint` is reached and copies to 'out'
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

// returns '0' if EOF (\0)
// returns on success 'N' as number of ascii in the line
// returns -1 if '__n' ascii is reached and breakpoint was not found
int __get_line(const ascii *buf, const ascii breakpoint, const size_t __n,
               ascii *out);

/* FIX: [LINUX][PERIPHERAL] : Sometimes lines break on a random place of the
 * line, seems to be rare. Maybe somehow there is a '\n' on the middle of the
 * line ?? (RELATED TO /proc/bus/input/devices on linux)*/

typedef struct AsciiLine_t {
  ascii *line;
  usize len;
} AsciiLine;

int ascii_starts_with(const ascii *str, const ascii *prefix, const usize __n);
int ascii_contains(const ascii *str, const usize _str_n, const ascii *comp,
                   const usize _comp_n);
int ascii_is_number(const ascii c);

// tmp
#define CORE_BASE_IMPLEMENTATION

#if defined(CORE_BASE_IMPLEMENTATION)

// SEAKCUTILS IMPLEMENTATION
#define MYSTRINGS_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#define DEQUE_IMPLEMENTATION

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

#define SAE_FAILURE                                                            \
  { exit(EXIT_FAILURE); }

#else

// DO NOTHING
#define SAE_ASSERT(expr)
#define SAE_ASSERT_MSG(expr, msg, ...)
#define SAE_ERROR_ARGS(msg, ...)
#define SAE_ERROR(msg)
#define SAE_FAILURE

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

int __get_line(const ascii *buf, const ascii breakpoint, const size_t __n,
               ascii *out) {
  if (!buf)
    return -1;

  for (size_t x = 0; x < __n; x += 1) {
    if (buf[x] == breakpoint) {
      memcpy(out, buf, sizeof(ascii) * (x + 1));
      return x + 1;
    } else if (buf[x] == '\0')
      return 0;
  }

  return -1;
}

int ascii_starts_with(const ascii *str, const ascii *prefix, const usize __n) {
  if (!str || !prefix)
    return -1;
  for (usize x = 0; x < __n; x += 1)
    if (str[x] != prefix[x])
      return 0;

  return 1;
}

int ascii_contains(const ascii *str, const usize _str_n, const ascii *comp,
                   const usize _comp_n) {
  if (!str || !comp)
    return -1;
  usize acum = 0;

  for (usize x = 0; x < _str_n; x += 1)
    for (usize i = 0; i < _comp_n; i += 1)
      if (str[x + i] != comp[i]) {
        acum = 0;
        break;
      } else {
        acum += 1;
        if (acum == _comp_n)
          return 1;
      }

  return 0;
}

int ascii_contains_ret_idx(const ascii *str, const usize _str_n,
                           const ascii *comp, const usize _comp_n) {
  if (!str || !comp)
    return -1;
  usize acum = 0;

  for (usize x = 0; x < _str_n; x += 1) {
    int word_start_idx = (int)x;
    for (usize i = 0; i < _comp_n; i += 1)
      if (x + i >= _str_n || str[x + i] != comp[i]) {
        acum = 0;
        break;
      } else {
        acum += 1;
        if (acum == _comp_n)
          return word_start_idx;
      }
  }

  return -1;
}

int ascii_is_number(const ascii c) {
  u8 num_0_ascii = 0x30;
  u8 num_9_ascii = 0x39;
  if (c >= num_0_ascii && c <= num_9_ascii)
    return 1;
  else
    return 0;
}

#endif
