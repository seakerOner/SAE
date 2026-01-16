#ifndef CORE_EVENTS_H
#define CORE_EVENTS_H
#include "seakcutils/channels/channels.h"
#include "seakcutils/channels/spmc.h"

#include "./core_base.h"
#include "./core_sys_input.h"

#include "../sae_input_list_names.h"

#if defined(__linux__)

#include "sys/epoll.h"

#elif defined(_WIN64)
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unsupported operating system... :/"
#endif

typedef enum SAE_EventType_t {
  SAE_EVENT_KEY_DOWN,
  SAE_EVENT_KEY_UP,
  SAE_EVENT_MOUSE_MOVE,
  SAE_EVENT_MOUSE_BUTTON_UP,
  SAE_EVENT_MOUSE_BUTTON_DOWN,
  SAE_EVENT_MOUSE_WHEEL,
  SAE_EVENT_GAMEPAD_AXIS,
  SAE_EVENT_GAMEPAD_BUTTON_UP,
  SAE_EVENT_GAMEPAD_BUTTON_DOWN,
  SAE_EVENT_DEVICE_ADDED,
  SAE_EVENT_DEVICE_REMOVED
} SAE_EventType;

typedef struct SAE_Event_t {
  SAE_EventType type;
  SAE_TimeStamp timestamp;
  u32 device_id;

  union {
    struct {
      SAE_Key key;
    } keypad;
    struct {
      i32 dx, dy;
    } mouse_move;
    struct {
      u8 axis;
      float32 value;
    } gamepad_axis;
  };
} SAE_Event;

typedef union SAE_EventSystem_t {
  struct {
    int epoll_fd;
    ChannelSpmc *chan_queue;
    SenderSpmc *dispatcher;
  } linux_epoll;

  struct { // placeholder
    void *epoll_handle;
  } windows_epoll;

  struct { // placeholder
    void *epoll_handle;
  } apple_epoll;
} SAE_EventSystem;

SAE_EventSystem sae_get_event_system(void);

ReceiverSpmc *sae_event_system_get_queue(SAE_EventSystem *event_sys);
void sae_event_system_rmv_queue(ReceiverSpmc *queue);

int sae_event_system_add_inputdevice(SAE_EventSystem *event_sys,
                                     InputDevice *device);

int sae_event_system_add_inputdevice_list(SAE_EventSystem *event_sys,
                                          InputDeviceList *device_list);

int sae_event_system_rmv_inputdevice(SAE_EventSystem *event_sys,
                                     InputDevice *device);

int sae_event_system_rmv_inputdevice_all(SAE_EventSystem *event_sys,
                                         InputDeviceList *device_list);

void sae_free_event_system(SAE_EventSystem event_sys);

void sae_event_system_execute(SAE_EventSystem *event_sys);

#endif
