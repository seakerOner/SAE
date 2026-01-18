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

  SAE_EVENT_MOUSE_MOVE_X,
  SAE_EVENT_MOUSE_MOVE_Y,
  SAE_EVENT_MOUSE_MOVE_X_ROT,
  SAE_EVENT_MOUSE_MOVE_Y_ROT,

  SAE_EVENT_MOUSE_BUTTON_UP,
  SAE_EVENT_MOUSE_BUTTON_DOWN,
  SAE_EVENT_MOUSE_WHEEL,
  SAE_EVENT_MOUSE_WHEEL_HI_RES,

  SAE_EVENT_GAMEPAD_LX_AXIS,
  SAE_EVENT_GAMEPAD_LY_AXIS,
  SAE_EVENT_GAMEPAD_RX_AXIS,
  SAE_EVENT_GAMEPAD_RY_AXIS,

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
      u8 trigger_pressure;
    } keypad;
    struct {
      union {
        struct {
          i32 x, y;
        } move;
        i8 wheel;
      };
    } mouse;
    struct {
      union {
        u8 x, y;
      };
    } gamepad_axis;
  };
} SAE_Event;

typedef struct SAE_EventSystem_t {
  ChannelSpmc *chan_queue;
  SenderSpmc *dispatcher;
  union {
    int epoll_linux_fd;
    void *epoll_win64_handle;
    void *epoll_apple_handle;
  };
} SAE_EventSystem;

SAE_EventSystem sae_get_event_system(void);

ReceiverSpmc *sae_event_system_get_queue(SAE_EventSystem *event_sys);
void sae_event_system_rmv_queue(ReceiverSpmc *queue);

int sae_event_system_add_inputdevice(SAE_EventSystem *event_sys,
                                     InputDevice *device);

int sae_event_system_add_inputdevice_list(SAE_EventSystem *event_sys,
                                          InputDeviceList *device_list,
                                          u8 peri_types_flags);

int sae_event_system_rmv_inputdevice(SAE_EventSystem *event_sys,
                                     InputDevice *device);

int sae_event_system_rmv_inputdevice_all(SAE_EventSystem *event_sys,
                                         InputDeviceList *device_list);

void sae_free_event_system(SAE_EventSystem event_sys);

void sae_event_system_execute(SAE_EventSystem *event_sys);

// TODO: IMPLEMENT sae_get_mice_screen_coords()
void sae_get_mice_screen_coords(i32 *x, i32 *y);

#endif
