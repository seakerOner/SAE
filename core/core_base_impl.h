#ifndef CORE_BASE_IMPLEMENTATION
#define CORE_BASE_IMPLEMENTATION

#include "./core_base.h"

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
      if (x + i >= _str_n || str[x + i] != comp[i]) {
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
