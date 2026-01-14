#ifndef CORE_EVENTS_IMPLEMENTATION
#define CORE_EVENTS_IMPLEMENTATION

#include "./core_events.h"
#include "core_base.h"
#include "core_sys_input.h"
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)

#include <errno.h>
#include <linux/input.h>
#include <sys/epoll.h>

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

SAE_EventSystem sae_get_event_system(void) {
  SAE_EventSystem event_sys;

#if defined(__linux__)

  int epoll = epoll_create1(EPOLL_CLOEXEC);
  if (epoll < 0) {
    SAE_ERROR("[FATAL] Failed to create epoll instance for events system")
  }
  event_sys.linux_epoll.epoll_fd = epoll;

  ChannelSpmc *chan = channel_create_spmc(5000, sizeof(SAE_Event));
  SAE_CHECK_ALLOC(chan, "Event System Channel Queue")
  SenderSpmc *dispatcher = spmc_get_sender(chan);
  event_sys.linux_epoll.chan_queue = chan;
  event_sys.linux_epoll.dispatcher = dispatcher;

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

  return event_sys;
}

int sae_event_system_add_inputdevice(SAE_EventSystem *event_sys,
                                     InputDevice *device) {
  if (!event_sys || !device)
    return -1;

#if defined(__linux__)

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.ptr = device;

  int res = epoll_ctl(event_sys->linux_epoll.epoll_fd, EPOLL_CTL_ADD,
                      device->linux_fd, &ev);

  if (res == -1) {
    SAE_ERROR_ARGS("[ERROR] Could not add InputDevice to EventSystem\n[ERROR] "
                   "System message: %s\n[ERROR] InputDevice id: %ld",
                   strerror(errno), device->id)
  }

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

  return 1;
}

int sae_event_system_add_inputdevice_list(SAE_EventSystem *event_sys,
                                          InputDeviceList *device_list) {
  if (!event_sys || !device_list)
    return -1;

  const LlNode *node = ll_iterable(&device_list->devices);

  for (usize x = 0; x < ll_len(&device_list->devices); x += 1) {
    InputDevice *input_device = (InputDevice *)node->elem;

#if defined(__linux__)

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = input_device;

    int res = epoll_ctl(event_sys->linux_epoll.epoll_fd, EPOLL_CTL_ADD,
                        input_device->linux_fd, &ev);
    if (res == -1) {
      SAE_ERROR_ARGS(
          "[ERROR] Could not add InputDevice to EventSystem\n[ERROR] "
          "System message: %s\n[ERROR] InputDevice id: %ld",
          strerror(errno), input_device->id)
    }

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

    node = ll_next(node);
  }

  return 1;
}

int sae_event_system_rmv_inputdevice(SAE_EventSystem *event_sys,
                                     InputDevice *device) {
  if (!event_sys || !device)
    return -1;

#if defined(__linux__)
  // A NULL pointer can be provided on epoll_event argument since its ignored,
  // but to avoid bugs with older kernel versions (Before Linux 2.6.9) we
  // provide a non-NULL ptr
  struct epoll_event ev;
  int res = epoll_ctl(event_sys->linux_epoll.epoll_fd, EPOLL_CTL_DEL,
                      device->linux_fd, &ev);
  if (res == -1) {
    SAE_ERROR_ARGS(
        "[ERROR] Could not remove InputDevice from EventSystem\n[ERROR] "
        "System message: %s\n[ERROR] InputDevice id: %ld",
        strerror(errno), device->id)
  }

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

  return 1;
}

// return values:
// -1  : invalid arguments;
// 0   : all devices were removed successfully
// > 1 : number of input devices failed to remove
int sae_event_system_rmv_inputdevice_all(SAE_EventSystem *event_sys,
                                         InputDeviceList *device_list) {
  if (!event_sys || !device_list)
    return -1;

  int failed_q = 0;

  const LlNode *node = ll_iterable(&device_list->devices);
  for (usize x = 0; x < ll_len(&device_list->devices); x += 1) {
#if defined(__linux__)
    InputDevice *device = (InputDevice *)node->elem;
    struct epoll_event ev;
    int res = epoll_ctl(event_sys->linux_epoll.epoll_fd, EPOLL_CTL_DEL,
                        device->linux_fd, &ev);
    if (res == -1) {
      SAE_ERROR_ARGS(
          "[ERROR] Could not remove InputDevice from EventSystem\n[ERROR] "
          "System message: %s\n[ERROR] InputDevice id: %ld",
          strerror(errno), device->id)
      failed_q += 1;
    }

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

    node = ll_next(node);
  }

  return failed_q;
}

void sae_free_event_system(SAE_EventSystem event_sys) {
#if defined(__linux__)
  close(event_sys.linux_epoll.epoll_fd);
  spmc_close(event_sys.linux_epoll.chan_queue);
  spmc_destroy(event_sys.linux_epoll.chan_queue);
  free(event_sys.linux_epoll.dispatcher);
#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif
}

#endif
