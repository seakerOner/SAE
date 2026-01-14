#ifndef CORE_SYS_INPUT_IMPLEMENTATION
#define CORE_SYS_INPUT_IMPLEMENTATION

#include "./core_base.h"
#include "./core_sys_input.h"
#include <stdio.h>

#if defined(__linux__)

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <unistd.h>
#define __SAE_LINUX_DEVICES_AVAILABLE_PATH__ "/proc/bus/input/devices"
#define __SAE_LINUX_DEVICES_EVENT_PATH_BASE__ "/dev/input/"
#define __SAE_LINUX_DEVICES_EVENT_PATH_MAX_SIZE 20
#define __SAE_LINUX_DEVICES_TMP_BUF_SIZE 4096

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

#define test_bit(bit, array, max_words)                                        \
  (((bit) / 64 < (max_words)) ? ((array[(bit) / 64] >> ((bit) % 64)) & 1) : 0)
#define __SAE_PERI_MAX_ABS_WORDS 8

// internal function used by `sae_get_available_peripherals_list`
int __sae_try_set_event_path(PeripheralDevice *peri) {
  int idx = ascii_contains_ret_idx(peri->inner_peripheral.linux_p.handler,
                                   peri->inner_peripheral.linux_p.handler_len,
                                   (ascii *)"event", 5);
  if (idx == -1) {
    return 0;
  }

  ascii *event_str = peri->inner_peripheral.linux_p.handler + (idx + 5);

  ascii event_num[8] = {0};

  for (int x = 0; x < 3; x += 1) {
    if ((event_str + x))
      if (ascii_is_number(*(event_str + x)))
        event_num[x] = *(event_str + x);
  }
  snprintf((char *)peri->inner_peripheral.linux_p.event_path,
           sizeof(peri->inner_peripheral.linux_p.event_path), "%s%s%.*s",
           __SAE_LINUX_DEVICES_EVENT_PATH_BASE__, "event", 3, event_num);
  return 1;
}

// Get a list of available peripherals on device
PeripheralDeviceList sae_get_available_peripherals_list() {
#if defined(__linux__)
  int list_fd = open(__SAE_LINUX_DEVICES_AVAILABLE_PATH__, O_RDONLY);
  if (list_fd == -1) {
    SAE_ERROR_ARGS("[FATAL] Could not get list of available peripherals on "
                   "device. \n[FATAL] Couldn't open path: %.*s",
                   30, __SAE_LINUX_DEVICES_AVAILABLE_PATH__)
  }

  // cannot use fstat for files stats under `/proc/ `because they are
  // pseudo-files (interfaces to kernel data)
  //
  // We read the entire file with prefixed buffer sizes, there is no info on
  // lenght of the file until you fully read it
  usize used = 0;
  usize buf_cap = __SAE_LINUX_DEVICES_TMP_BUF_SIZE;
  ascii *buf = malloc(sizeof(ascii) * buf_cap);
  SAE_CHECK_ALLOC(buf, "temporary buffer")

  int done = 0;
  while (!done) {
    if (used == buf_cap) {
      buf_cap += __SAE_LINUX_DEVICES_TMP_BUF_SIZE;
      void *new = realloc(buf, sizeof(ascii) * buf_cap);
      SAE_CHECK_ALLOC_AND(new, "temporary buffer", free(buf));
      buf = new;
    }

    int bytes_read = read(list_fd, buf + used, buf_cap - used);
    if (bytes_read < 0) {
      free(buf);
      SAE_ERROR_ARGS(
          "[FATAL] Something went wrong reading the list of available "
          "peripherals on device  \n"
          "[FATAL] List path: %.*s \n",
          30, __SAE_LINUX_DEVICES_AVAILABLE_PATH__)
    }
    if (bytes_read == 0)
      break;

    used += bytes_read;
  }
  buf[used] = '\0';

  ascii *lines_buf = &buf[0];
  Deque deque_lines = deque_new(sizeof(AsciiLine), 1000);

  while (*lines_buf != '\0') {
    ascii tmp_line[300];
    memset(tmp_line, 0, sizeof(tmp_line));
    ascii breakpoint = '\n';
    int res = __get_line(lines_buf, breakpoint, 300, tmp_line);

    if (res > 0) {
      lines_buf += res;

      AsciiLine line;
      line.len = res;
      line.line = malloc(sizeof(ascii) * line.len);
      SAE_CHECK_ALLOC_AND(line.line, "ascii line", SAE_GOTO(abort_cleanup))

      memcpy(line.line, tmp_line, line.len);
      deque_push_back(&deque_lines, &line);

    } else if (res == 0) {
      break;
    } else if (res == -1) {
      free(buf);
      SAE_ERROR_ARGS(
          "[FATAL] Something went wrong reading the list of available "
          "peripherals on device (No %c to parse file ?) \n"
          "[FATAL] List path: %.*s",
          breakpoint, 30, __SAE_LINUX_DEVICES_AVAILABLE_PATH__);
      SAE_FAILURE
    }
  }

  //  TODO: [LINUX][PERIPHERAL] : make parsing linux devices more robust
  PeripheralDeviceList list;
  // would be nice to know the expected number of peripherals and use it here
  list.cap = 99;
  list.items = calloc(list.cap, sizeof(PeripheralDevice));
  list.num_items = 0;
  SAE_CHECK_ALLOC_AND(list.items, "Peripheral Device List",
                      SAE_GOTO(abort_cleanup);)

  // CONTROL FLOW:
  // - Goto's are used to centralize control flow without adding extra functions
  // - This loop will exit through goto's (See below `for` loop) if all devices
  // are parsed
  // - If no more lines are in the Deque it will goto `exit` label
  // - The While loop if it encounters a Malloc error it will goto
  // `devices_abort_path`label and consequently to `abort_cleanup`label
  // - If capacity is reached the loop will finish normally and goes to `exit`
  // label
  for (usize x = 0; x < list.cap; x += 1) {
    PeripheralDevice *peri = &list.items[x];
    memset(&peri->inner_peripheral.linux_p, 0,
           sizeof(peri->inner_peripheral.linux_p));

    peri->id = x;
    peri->type = SAE_PERIPHERAL_T_UNKNOWN;

    while (1) {
      AsciiLine line;
      int res = deque_pop_front(&deque_lines, &line);
      if (res != 0)
        goto exit;

      // identifying devices and then breaking to next device
      // undentified devices are considered [SAE_PERIPHERAL_T_UNKNOWN]

      if (line.len == 1 && line.line[0] == '\n') {
        free(line.line);
        list.num_items += 1;

        u64 ev_bits =
            peri->inner_peripheral.linux_p.ev
                ? strtoul((char *)peri->inner_peripheral.linux_p.ev, NULL, 16)
                : 0;
        u64 abs_bits =
            peri->inner_peripheral.linux_p.abs
                ? strtoul((char *)peri->inner_peripheral.linux_p.abs, NULL, 16)
                : 0;
        u64 rel_bits[__SAE_PERI_MAX_ABS_WORDS] = {0};

        {
          if (peri->inner_peripheral.linux_p.rel) {
            char tmp[512];
            strncpy(tmp, (char *)peri->inner_peripheral.linux_p.rel,
                    sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';

            int count = 0;
            char *p = tmp;

            while (*p && count < __SAE_PERI_MAX_ABS_WORDS) {
              rel_bits[count++] = strtoul(p, &p, 16);
              while (*p == ' ')
                p++;
            }
          }
        }

        bool has_key = ev_bits & (1ul << EV_KEY);
        bool has_rel = ev_bits & (1ul << EV_REL);
        bool has_abs = ev_bits & (1ul << EV_ABS);

        bool has_abs_x = abs_bits & (1ul << ABS_X);
        bool has_abs_rx = abs_bits & (1ul << ABS_RX);
        bool has_abs_y = abs_bits & (1ul << ABS_Y);
        bool has_abs_ry = abs_bits & (1ul << ABS_RY);

        bool has_rel_x = test_bit(REL_X, rel_bits, __SAE_PERI_MAX_ABS_WORDS);
        bool has_rel_y = test_bit(REL_Y, rel_bits, __SAE_PERI_MAX_ABS_WORDS);

        if (has_rel && has_rel_x && has_rel_y) { // mouse
          peri->type = SAE_PERIPHERAL_T_MOUSE;
          __sae_try_set_event_path(peri);

        } else if (has_key && has_abs) { // possible gamepad

          // gamepad for sure
          if (has_abs_x && has_abs_y && has_abs_rx && has_abs_ry) {
            peri->type = SAE_PERIPHERAL_T_GAMEPAD;
            int gamepad_has_event = __sae_try_set_event_path(peri);

            // fallback to legacy input if no eventX found
            if (!gamepad_has_event) {
              int contains =
                  ascii_contains(peri->inner_peripheral.linux_p.handler,
                                 peri->inner_peripheral.linux_p.handler_len,
                                 (ascii *)"js0", 3);
              if (contains)
                snprintf((char *)peri->inner_peripheral.linux_p.event_path,
                         sizeof(peri->inner_peripheral.linux_p.event_path),
                         "%s%s", __SAE_LINUX_DEVICES_EVENT_PATH_BASE__, "js0");
            }
          }
        } else if (has_key && !has_abs) { // normal keyboard
          peri->type = SAE_PERIPHERAL_T_KEYBOARD;
          __sae_try_set_event_path(peri);
        }
        if (peri->type == SAE_PERIPHERAL_T_UNKNOWN) {
          __sae_try_set_event_path(peri);
        }

        break;
      }

      // parse lines
      if (ascii_starts_with(line.line, (ascii *)"N: Name=", 8)) {
        usize n = line.len - 11;
        peri->inner_peripheral.linux_p.name =
            malloc(sizeof(ascii) * (n + 1)); // 1 -> null terminator
        SAE_CHECK_ALLOC_AND(peri->inner_peripheral.linux_p.name, "Device Name",
                            SAE_GOTO(devices_abort_path))
        memcpy(peri->inner_peripheral.linux_p.name, line.line + 9,
               sizeof(ascii) * n);
        peri->inner_peripheral.linux_p.name[n] = '\0';
        peri->inner_peripheral.linux_p.name_len = n;

      } else if (ascii_starts_with(line.line, (ascii *)"H: Handlers=", 12)) {
        usize n = line.len - 13;
        peri->inner_peripheral.linux_p.handler =
            malloc(sizeof(ascii) * (n + 1)); // 1 -> null terminator
        SAE_CHECK_ALLOC_AND(peri->inner_peripheral.linux_p.handler,
                            "Device Handler", SAE_GOTO(devices_abort_path))
        memcpy(peri->inner_peripheral.linux_p.handler, line.line + 12,
               sizeof(ascii) * n);
        peri->inner_peripheral.linux_p.handler[n] = '\0';
        peri->inner_peripheral.linux_p.handler_len = n;

      } else if (ascii_starts_with(line.line, (ascii *)"P: Phys=", 8)) {
        usize n = line.len - 9;
        peri->inner_peripheral.linux_p.phys =
            malloc(sizeof(ascii) * (n + 1)); // 1 -> null terminator
        SAE_CHECK_ALLOC_AND(peri->inner_peripheral.linux_p.phys,
                            "Physical Device identification",
                            SAE_GOTO(devices_abort_path))
        memcpy(peri->inner_peripheral.linux_p.phys, line.line + 8,
               sizeof(ascii) * n);
        peri->inner_peripheral.linux_p.phys[n] = '\0';
        peri->inner_peripheral.linux_p.phys_len = n;

      } else if (ascii_starts_with(line.line, (ascii *)"B: EV=", 6)) {
        usize n = line.len - 6;
        peri->inner_peripheral.linux_p.ev =
            malloc(sizeof(ascii) * (n + 1)); // 1 -> null terminator
        SAE_CHECK_ALLOC_AND(peri->inner_peripheral.linux_p.ev, "Device EV",
                            SAE_GOTO(devices_abort_path))
        memcpy(peri->inner_peripheral.linux_p.ev, line.line + 6,
               sizeof(ascii) * n);
        peri->inner_peripheral.linux_p.ev[n] = '\0';
        peri->inner_peripheral.linux_p.ev_len = n;
      } else if (ascii_starts_with(line.line, (ascii *)"B: ABS=", 7)) {
        usize n = line.len - 7;
        peri->inner_peripheral.linux_p.abs =
            malloc(sizeof(ascii) * (n + 1)); // 1-> null terminator
        SAE_CHECK_ALLOC_AND(peri->inner_peripheral.linux_p.abs, "Device ABS",
                            SAE_GOTO(devices_abort_path))
        memcpy(peri->inner_peripheral.linux_p.abs, line.line + 7,
               sizeof(ascii) * n);
        peri->inner_peripheral.linux_p.abs[n] = '\0';
        peri->inner_peripheral.linux_p.abs_len = n;
      } else if (ascii_starts_with(line.line, (ascii *)"B: REL=", 7)) {
        usize n = line.len - 7;
        peri->inner_peripheral.linux_p.rel =
            malloc(sizeof(ascii) * (n + 1)); // 1 -> null terminator
        SAE_CHECK_ALLOC_AND(peri->inner_peripheral.linux_p.rel, "Device REL",
                            SAE_GOTO(devices_abort_path))
        memcpy(peri->inner_peripheral.linux_p.rel, line.line + 7,
               sizeof(ascii) * n);
        peri->inner_peripheral.linux_p.rel[n] = '\0';
        peri->inner_peripheral.linux_p.rel_len = n;
      }

      free(line.line);
      continue;

    devices_abort_path:
      free(line.line);
      sae_free_available_peripherals_list(list);
      goto abort_cleanup;
    }
  }
  goto exit;

// `abort_cleanup` is the cleanup of allocations in case of errors
abort_cleanup:
  close(list_fd);
  if (buf)
    free(buf);
  while (1) {
    AsciiLine _line;
    int _res = deque_pop_front(&deque_lines, &_line);
    if (_res != 0)
      break;
    free(_line.line);
  }

  deque_free(&deque_lines);
  SAE_FAILURE

// main exit point of the function.
// `exit` is the happy path of the function if no errors are found.
exit:
  close(list_fd);
  if (buf)
    free(buf);
  while (1) {
    AsciiLine _line;
    int _res = deque_pop_front(&deque_lines, &_line);
    if (_res != 0)
      break;
    free(_line.line);
  }

  deque_free(&deque_lines);
  return list;

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)

#else
#error "Unsupported operating system... :/"
#endif
}
void sae_free_available_peripherals_list(PeripheralDeviceList Plist) {
#if defined(__linux__)
  for (usize x = 0; x < Plist.num_items; x += 1) {
    if (Plist.items[x].inner_peripheral.linux_p.name)
      free(Plist.items[x].inner_peripheral.linux_p.name);
    if (Plist.items[x].inner_peripheral.linux_p.handler)
      free(Plist.items[x].inner_peripheral.linux_p.handler);
    if (Plist.items[x].inner_peripheral.linux_p.ev)
      free(Plist.items[x].inner_peripheral.linux_p.ev);
    if (Plist.items[x].inner_peripheral.linux_p.rel)
      free(Plist.items[x].inner_peripheral.linux_p.rel);
    if (Plist.items[x].inner_peripheral.linux_p.abs)
      free(Plist.items[x].inner_peripheral.linux_p.abs);
    if (Plist.items[x].inner_peripheral.linux_p.phys)
      free(Plist.items[x].inner_peripheral.linux_p.phys);
  }
  free(Plist.items);

// TODO: [WINDOWS][PERIPHERAL]: Make pheripheral device list
#elif defined(_WIN64)
  // TODO: [APPLE][PERIPHERAL]: Make pheripheral device list
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif
}

InputDeviceList
sae_peripheralslist_to_inputdeviceslist(PeripheralDeviceList *peri_list) {
  InputDeviceList inputlist;
  inputlist.devices = linkedlist_new(sizeof(InputDevice));

  for (usize x = 0; x < peri_list->num_items; x += 1) {
    PeripheralDevice *peri = &peri_list->items[x];

    if (peri->type == SAE_PERIPHERAL_T_UNKNOWN)
      continue;

    InputDevice input;
    input.id = peri->id;

    switch (peri->type) {
    case SAE_PERIPHERAL_T_MOUSE:
    case SAE_PERIPHERAL_T_KEYBOARD:
    case SAE_PERIPHERAL_T_GAMEPAD:
      input.type = peri->type;

#if defined(__linux__)
      int fd = open((char *)peri->inner_peripheral.linux_p.event_path,
                    O_RDONLY | O_NONBLOCK);
      if (fd < 0) {
        SAE_ERROR_ARGS(
            "[FAILURE] Could not create a file descriptor for Input Device "
            "from Peripheral Device\n[FAILURE] Invalid path: %.s",
            peri->inner_peripheral.linux_p.event_path)
      }
      input.linux_fd = fd;

      ll_append(&inputlist.devices, &input);

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif
      break;

    default:
      SAE_ERROR("[ANOMALY] Foreign Type found when turning a Peripheral Device "
                "into a Input Device\n[ANOMALY] Type must be a valid\n[TIP] If "
                "you added a new peripheral type add it to the switch inside "
                "this function"
                "SAE_PERIPHERAL_T_xxx")
      break;
    }
  }

  return inputlist;
}

int sae_free_input_device(InputDeviceList input_list, usize id) {
  const LlNode *node = ll_iterable(&input_list.devices);
  for (usize x = 0; x < ll_len(&input_list.devices); x += 1) {
    InputDevice *device = (InputDevice *)node->elem;
    if (device->id == id) {
      int res = ll_remove(&input_list.devices, node);
      if (res == 0)
        return 1;
      else
        return -1;
    }
    node = ll_next(node);
  }

  return 0;
}

void sae_free_input_devices_list(InputDeviceList input_list) {
  const LlNode *node = ll_iterable(&input_list.devices);
  for (usize x = 0; x < ll_len(&input_list.devices); x += 1) {
    InputDevice *device = node->elem;
    close(device->linux_fd);

    node = ll_next(node);
  }

  ll_free(&input_list.devices);
}
#endif
