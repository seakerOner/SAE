#ifndef CORE_EVENTS_IMPLEMENTATION
#define CORE_EVENTS_IMPLEMENTATION

#include "./core_base.h"
#include "./core_events.h"
#include "./core_sys_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)

#include <errno.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <unistd.h>

#define SAE_LINUX_MAX_EPOLL_EVENTS 64

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

ReceiverSpmc *sae_event_system_get_queue(SAE_EventSystem *event_sys) {
  ReceiverSpmc *queue = spmc_get_receiver(event_sys->linux_epoll.chan_queue);
  SAE_CHECK_ALLOC(queue, "Event System Queue Receiver")

  return queue;
}

void sae_event_system_rmv_queue(ReceiverSpmc *queue) {
  spmc_close_receiver(queue);
  free(queue);
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

// must be set on a isolated thread
void sae_event_system_execute(SAE_EventSystem *event_sys) {
#if defined(__linux__)
  int epoll_fd = event_sys->linux_epoll.epoll_fd;
  SenderSpmc *dispatcher = event_sys->linux_epoll.dispatcher;
  ChannelSpmc *chan_queue = event_sys->linux_epoll.chan_queue;

  struct epoll_event events[SAE_LINUX_MAX_EPOLL_EVENTS];
  while (spmc_is_closed(chan_queue) == OPEN) {
    int res =
        epoll_pwait(epoll_fd, events, SAE_LINUX_MAX_EPOLL_EVENTS, -1, NULL);

    if (res < 0) {
      SAE_ERROR_ARGS(
          "[FATAL] An Error ocurred on Linux epoll EventSystem\n[FATAL] "
          "System message: %s",
          strerror(errno))

    } else if (res > 0) { // N file descriptors ready to be read
      for (int x = 0; x < res; x += 1) {
        InputDevice *i_device = (InputDevice *)events[x].data.ptr;
        struct input_event iev;
        int b_read = 0;
        do {
          b_read += read(i_device->linux_fd, &iev, sizeof(struct input_event));
        } while (b_read != sizeof(struct input_event));

        // TODO: [LINUX][EVENTSYSTEM] since its in the stack and the channel
        // copies the reference.. yeah FIX ME
        SAE_Event event;

        event.device_id = i_device->id;
        event.timestamp.seconds = iev.time.tv_sec;
        event.timestamp.microseconds = iev.time.tv_usec;

        switch (iev.type) {
          // key event (Could be a key from a keyboard, mouse, gamepad
        case EV_KEY:

          // event type
          switch (iev.value) {
          case 0:
            if (iev.code >= 0x110 && iev.code <= 0x117)
              event.type = SAE_EVENT_MOUSE_BUTTON_UP;
            else if (iev.code >= 0x130 && iev.code <= 0x13e)
              event.type = SAE_EVENT_GAMEPAD_BUTTON_UP;
            else
              event.type = SAE_EVENT_KEY_UP;
            break;
          case 1:
            if (iev.code >= 0x110 && iev.code <= 0x117)
              event.type = SAE_EVENT_MOUSE_BUTTON_DOWN;
            else if (iev.code >= 0x130 && iev.code <= 0x13e)
              event.type = SAE_EVENT_GAMEPAD_BUTTON_DOWN;
            else
              event.type = SAE_EVENT_KEY_DOWN;
            break;
          }

          switch (iev.code) {
            // mouse key event
          case BTN_LEFT:
            event.keypad.key = SAE_BTN_LEFT;
            break;
          case BTN_RIGHT:
            event.keypad.key = SAE_BTN_RIGHT;
            break;
          case BTN_MIDDLE:
            event.keypad.key = SAE_BTN_MIDDLE;
            break;
          case BTN_SIDE:
            event.keypad.key = SAE_BTN_SIDE;
            break;
          case BTN_EXTRA:
            event.keypad.key = SAE_BTN_EXTRA;
            break;
          case BTN_FORWARD:
            event.keypad.key = SAE_BTN_FORWARD;
            break;
          case BTN_BACK:
            event.keypad.key = SAE_BTN_BACK;
            break;
          case BTN_TASK:
            event.keypad.key = SAE_BTN_TASK;
            break;

            // gamepad key event
          case BTN_SOUTH:
            event.keypad.key = SAE_BTN_SOUTH;
            break;
          case BTN_EAST:
            event.keypad.key = SAE_BTN_EAST;
            break;
          case BTN_C:
            event.keypad.key = SAE_BTN_C;
            break;
          case BTN_NORTH:
            event.keypad.key = SAE_BTN_NORTH;
            break;
          case BTN_WEST:
            event.keypad.key = SAE_BTN_WEST;
            break;
          case BTN_Z:
            event.keypad.key = SAE_BTN_Z;
            break;
          case BTN_TL:
            event.keypad.key = SAE_BTN_TL;
            break;
          case BTN_TR:
            event.keypad.key = SAE_BTN_TR;
            break;
          case BTN_TL2:
            event.keypad.key = SAE_BTN_TL2;
            break;
          case BTN_TR2:
            event.keypad.key = SAE_BTN_TR2;
            break;
          case BTN_SELECT:
            event.keypad.key = SAE_BTN_SELECT;
            break;
          case BTN_START:
            event.keypad.key = SAE_BTN_START;
            break;
          case BTN_MODE:
            event.keypad.key = SAE_BTN_MODE;
            break;
          case BTN_THUMBL:
            event.keypad.key = SAE_BTN_THUMBL;
            break;
          case BTN_THUMBR:
            event.keypad.key = SAE_BTN_THUMBR;
            break;

            // keyboard keys
          case KEY_ESC:
            event.keypad.key = SAE_KEY_ESC;
            break;
          case KEY_1:
            event.keypad.key = SAE_KEY_1;
            break;
          case KEY_2:
            event.keypad.key = SAE_KEY_2;
            break;
          case KEY_3:
            event.keypad.key = SAE_KEY_3;
            break;
          case KEY_4:
            event.keypad.key = SAE_KEY_4;
            break;
          case KEY_5:
            event.keypad.key = SAE_KEY_5;
            break;
          case KEY_6:
            event.keypad.key = SAE_KEY_6;
            break;
          case KEY_7:
            event.keypad.key = SAE_KEY_7;
            break;
          case KEY_8:
            event.keypad.key = SAE_KEY_8;
            break;
          case KEY_9:
            event.keypad.key = SAE_KEY_9;
            break;
          case KEY_0:
            event.keypad.key = SAE_KEY_0;
            break;
          case KEY_MINUS:
            event.keypad.key = SAE_KEY_MINUS;
            break;
          case KEY_EQUAL:
            event.keypad.key = SAE_KEY_EQUAL;
            break;
          case KEY_BACKSPACE:
            event.keypad.key = SAE_KEY_BACKSPACE;
            break;
          case KEY_TAB:
            event.keypad.key = SAE_KEY_TAB;
            break;
          case KEY_Q:
            event.keypad.key = SAE_KEY_Q;
            break;
          case KEY_W:
            event.keypad.key = SAE_KEY_W;
            break;
          case KEY_E:
            event.keypad.key = SAE_KEY_E;
            break;
          case KEY_R:
            event.keypad.key = SAE_KEY_R;
            break;
          case KEY_T:
            event.keypad.key = SAE_KEY_T;
            break;
          case KEY_Y:
            event.keypad.key = SAE_KEY_Y;
            break;
          case KEY_U:
            event.keypad.key = SAE_KEY_U;
            break;
          case KEY_I:
            event.keypad.key = SAE_KEY_I;
            break;
          case KEY_O:
            event.keypad.key = SAE_KEY_O;
            break;
          case KEY_P:
            event.keypad.key = SAE_KEY_P;
            break;
          case KEY_LEFTBRACE:
            event.keypad.key = SAE_KEY_LEFTBRACE;
            break;
          case KEY_RIGHTBRACE:
            event.keypad.key = SAE_KEY_RIGHTBRACE;
            break;
          case KEY_ENTER:
            event.keypad.key = SAE_KEY_ENTER;
            break;
          case KEY_LEFTCTRL:
            event.keypad.key = SAE_KEY_LEFTCTRL;
            break;
          case KEY_A:
            event.keypad.key = SAE_KEY_A;
            break;
          case KEY_S:
            event.keypad.key = SAE_KEY_S;
            break;
          case KEY_D:
            event.keypad.key = SAE_KEY_D;
            break;
          case KEY_F:
            event.keypad.key = SAE_KEY_F;
            break;
          case KEY_G:
            event.keypad.key = SAE_KEY_G;
            break;
          case KEY_H:
            event.keypad.key = SAE_KEY_H;
            break;
          case KEY_J:
            event.keypad.key = SAE_KEY_J;
            break;
          case KEY_K:
            event.keypad.key = SAE_KEY_K;
            break;
          case KEY_L:
            event.keypad.key = SAE_KEY_L;
            break;
          case KEY_SEMICOLON:
            event.keypad.key = SAE_KEY_SEMICOLON;
            break;
          case KEY_APOSTROPHE:
            event.keypad.key = SAE_KEY_APOSTROPHE;
            break;
          case KEY_GRAVE:
            event.keypad.key = SAE_KEY_GRAVE;
            break;
          case KEY_LEFTSHIFT:
            event.keypad.key = SAE_KEY_LEFTSHIFT;
            break;
          case KEY_BACKSLASH:
            event.keypad.key = SAE_KEY_BACKSLASH;
            break;
          case KEY_Z:
            event.keypad.key = SAE_KEY_Z;
            break;
          case KEY_X:
            event.keypad.key = SAE_KEY_X;
            break;
          case KEY_C:
            event.keypad.key = SAE_KEY_C;
            break;
          case KEY_V:
            event.keypad.key = SAE_KEY_V;
            break;
          case KEY_B:
            event.keypad.key = SAE_KEY_B;
            break;
          case KEY_N:
            event.keypad.key = SAE_KEY_N;
            break;
          case KEY_M:
            event.keypad.key = SAE_KEY_M;
            break;
          case KEY_COMMA:
            event.keypad.key = SAE_KEY_COMMA;
            break;
          case KEY_DOT:
            event.keypad.key = SAE_KEY_DOT;
            break;
          case KEY_SLASH:
            event.keypad.key = SAE_KEY_SLASH;
            break;
          case KEY_RIGHTSHIFT:
            event.keypad.key = SAE_KEY_RIGHTSHIFT;
            break;
          case KEY_KPASTERISK:
            event.keypad.key = SAE_KEY_KPASTERISK;
            break;
          case KEY_LEFTALT:
            event.keypad.key = SAE_KEY_LEFTALT;
            break;
          case KEY_SPACE:
            event.keypad.key = SAE_KEY_SPACE;
            break;
          case KEY_CAPSLOCK:
            event.keypad.key = SAE_KEY_CAPSLOCK;
            break;
          case KEY_F1:
            event.keypad.key = SAE_KEY_F1;
            break;
          case KEY_F2:
            event.keypad.key = SAE_KEY_F2;
            break;
          case KEY_F3:
            event.keypad.key = SAE_KEY_F3;
            break;
          case KEY_F4:
            event.keypad.key = SAE_KEY_F4;
            break;
          case KEY_F5:
            event.keypad.key = SAE_KEY_F5;
            break;
          case KEY_F6:
            event.keypad.key = SAE_KEY_F6;
            break;
          case KEY_F7:
            event.keypad.key = SAE_KEY_F7;
            break;
          case KEY_F8:
            event.keypad.key = SAE_KEY_F8;
            break;
          case KEY_F9:
            event.keypad.key = SAE_KEY_F9;
            break;
          case KEY_F10:
            event.keypad.key = SAE_KEY_F10;
            break;
          case KEY_NUMLOCK:
            event.keypad.key = SAE_KEY_NUMLOCK;
            break;
          case KEY_SCROLLLOCK:
            event.keypad.key = SAE_KEY_SCROLLLOCK;
            break;
          case KEY_KP7:
            event.keypad.key = SAE_KEY_KP7;
            break;
          case KEY_KP8:
            event.keypad.key = SAE_KEY_KP8;
            break;
          case KEY_KP9:
            event.keypad.key = SAE_KEY_KP9;
            break;
          case KEY_KPMINUS:
            event.keypad.key = SAE_KEY_KPMINUS;
            break;
          case KEY_KP4:
            event.keypad.key = SAE_KEY_KP4;
            break;
          case KEY_KP5:
            event.keypad.key = SAE_KEY_KP5;
            break;
          case KEY_KP6:
            event.keypad.key = SAE_KEY_KP6;
            break;
          case KEY_KPPLUS:
            event.keypad.key = SAE_KEY_KPPLUS;
            break;
          case KEY_KP1:
            event.keypad.key = SAE_KEY_KP1;
            break;
          case KEY_KP2:
            event.keypad.key = SAE_KEY_KP2;
            break;
          case KEY_KP3:
            event.keypad.key = SAE_KEY_KP3;
            break;
          case KEY_KP0:
            event.keypad.key = SAE_KEY_KP0;
            break;
          case KEY_KPDOT:
            event.keypad.key = SAE_KEY_KPDOT;
            break;
          case KEY_F11:
            event.keypad.key = SAE_KEY_F11;
            break;
          case KEY_F12:
            event.keypad.key = SAE_KEY_F12;
            break;
          case KEY_RIGHTCTRL:
            event.keypad.key = SAE_KEY_RIGHTCTRL;
            break;
          case KEY_RIGHTALT:
            event.keypad.key = SAE_KEY_RIGHTALT;
            break;
          case KEY_HOME:
            event.keypad.key = SAE_KEY_HOME;
            break;
          case KEY_UP:
            event.keypad.key = SAE_KEY_UP;
            break;
          case KEY_PAGEUP:
            event.keypad.key = SAE_KEY_PAGEUP;
            break;
          case KEY_LEFT:
            event.keypad.key = SAE_KEY_LEFT;
            break;
          case KEY_RIGHT:
            event.keypad.key = SAE_KEY_RIGHT;
            break;
          case KEY_END:
            event.keypad.key = SAE_KEY_END;
            break;
          case KEY_DOWN:
            event.keypad.key = SAE_KEY_DOWN;
            break;
          case KEY_PAGEDOWN:
            event.keypad.key = SAE_KEY_PAGEDOWN;
            break;
          case KEY_INSERT:
            event.keypad.key = SAE_KEY_INSERT;
            break;
          case KEY_DELETE:
            event.keypad.key = SAE_KEY_DELETE;
            break;
          case KEY_MUTE:
            event.keypad.key = SAE_KEY_MUTE;
            break;
          case KEY_VOLUMEDOWN:
            event.keypad.key = SAE_KEY_VOLUMEDOWN;
            break;
          case KEY_VOLUMEUP:
            event.keypad.key = SAE_KEY_VOLUMEUP;
            break;
          case KEY_POWER:
            event.keypad.key = SAE_KEY_POWER;
            break;
          case KEY_PAUSE:
            event.keypad.key = SAE_KEY_PAUSE;
            break;
          }
          break;

          // GAMEPAD
        case EV_ABS:
          printf("EVENT FROM GAMEPAD\n");
          break;

          // MOUSE
        case EV_REL:

          printf("EVENT FROM MOUSE\n");
          break;

        default:
          printf("OTHER EVENT HAPPENED\n");
          break;
        }

        // Send SAE_Event to the queue
        switch (spmc_send(dispatcher, &event)) {
        case CHANNEL_OK:
          break;

          // These 2 will only happen on a race condition:
          //
          // - Since this loop stops when the channel is closed and the
          // EventSystem on shutdown closes the dispatcher and the channel
          // itself, this case will happen if the user frees the EventSystem and
          // there is a cicle in this loop still yet to finish.
          // - If so the last cicle will try to send the event to a closed
          // channel or a closed dispatcher.
          // - User must remove all InputDevices from the Event System before
          // freeing the InputDevices
        case CHANNEL_ERR_NULL:
          SAE_ERROR("[WARNING] The Event System Dispatcher is NULL\n[TIP] You "
                    "should remove all associated InputDevices from the Event "
                    "System before closing the queue")
          break;
        case CHANNEL_ERR_CLOSED:
          SAE_ERROR(
              "[WARNING] The Event System Queue Channel is CLOSED\n[TIP] You "
              "should remove all associated InputDevices from the Event "
              "System before freeing the Event System)")
          break;
        }
      }

    } else if (res == 0) { // no file descriptors ready
      cpu_relax();
    }
  }
#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif
}

#endif
