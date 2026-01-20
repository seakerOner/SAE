#include <string.h>
#define SAE_RELEASE 0
#define SAE_DEBUG 1
#define SAE_TRACER 1

// includes
#include "../sae_input_list_names.h"

#include "../core/core_base.h"
#include "../core/core_base_impl.h"

#include "../core/core_sys_input.h"
#include "../core/core_sys_input_impl.h"

#include "../core/core_events.h"
#include "../core/core_events_impl.h"

#include "../core/seakcutils/arenas/r_arena.h"
#include "../core/seakcutils/channels/mpmc.h"
#include "../core/seakcutils/job_system/jobsystem.h"
#include "../core/seakcutils/threadpool/threadpool.h"

#include <stdio.h>

#define JOYSTICK_MIDDLE 128

// function used to execute event_system on a thread
void set_events(void *event_sys) {
  SAE_EventSystem *ev_sys = (SAE_EventSystem *)event_sys;

  // execute event_system on a isolated thread, it does polling and file reads
  sae_event_system_execute(ev_sys);
}

void print_ev_keypress(SAE_EventType ev_t) {
  switch (ev_t) {
  case SAE_EVENT_KEY_UP:
    printf("[KEYUP]\n");
    break;
  case SAE_EVENT_KEY_DOWN:
    printf("[KEYDOWN]\n");
    break;
  case SAE_EVENT_KEY_DOWN_REPEAT:
    printf("[KEYDOWN][REPEATED]\n");
    break;
  case SAE_EVENT_MOUSE_BUTTON_UP:
    printf("[MOUSEBUTTONUP]\n");
    break;
  case SAE_EVENT_MOUSE_BUTTON_DOWN:
    printf("[MOUSEBUTTONDOWN]\n");
    break;
  case SAE_EVENT_GAMEPAD_BUTTON_UP:
    printf("[GAMEPADBUTTONUP]\n");
    break;
  case SAE_EVENT_GAMEPAD_BUTTON_DOWN:
    printf("[GAMEPADBUTTONDOWN]\n");
    break;
  default:
    break;
  }
}

int main() {
  // create a threadpool
  ThreadPool *pool = threadpool_init_for_scheduler(4);

  // create job scheduler with the new threadpool
  job_scheduler_spawn(pool);

  // get all available pheripheral's logic interfaces
  PeripheralDeviceList peri_list = sae_get_available_peripherals_list();

  // make pheripherals into "input devices", they are simply file descriptors of
  // the pheripherals interfaces
  //
  // last argument is a bitflag for the type of pheripherals you want to
  // transform
  InputDeviceList input_list = sae_peripheralslist_to_inputdeviceslist(
      &peri_list, SAE_PERIPHERAL_T_ALL_KNOWN);

  // create a SAE_EventSystem instance
  SAE_EventSystem ev_sys = sae_get_event_system();

  // subscribe inputdevices to eventsystem
  sae_event_system_add_inputdevice_list(&ev_sys, &input_list,
                                        SAE_PERIPHERAL_T_ALL_KNOWN);

  // get a channel receiver where you will get the SAE events
  ReceiverSpmc *ev_queue = sae_event_system_get_queue(&ev_sys);

  // create a job for the scheduler with a function ptr executing the event
  // system
  JobHandle *event_sys_for_thread = job_spawn(set_events, &ev_sys);

  // submit the job to the job scheduler
  job_wait(event_sys_for_thread);

  bool break_outof_queue = FALSE;

  u8 gamepad_lx = JOYSTICK_MIDDLE, gamepad_ly = JOYSTICK_MIDDLE;
  u8 gamepad_rx = JOYSTICK_MIDDLE, gamepad_ry = JOYSTICK_MIDDLE;

  // this section is very self explanatory
  //
  // PS: Events include a timestamp!
  while (TRUE) {
    SAE_Event ev = {0};

    while (spmc_try_recv(ev_queue, &ev) == CHANNEL_OK) {

      switch (ev.type) {
      case SAE_EVENT_GAMEPAD_LX_AXIS:
        gamepad_lx = ev.gamepad_axis.x;
        printf("GAMEPAD LEFT  AXIS: [X]: %d | [Y]: %d\n", gamepad_lx,
               gamepad_ly);
        printf("GAMEPAD RIGHT AXIS: [X]: %d | [Y]: %d\n", gamepad_rx,
               gamepad_ry);
        break;
      case SAE_EVENT_GAMEPAD_LY_AXIS:
        gamepad_ly = ev.gamepad_axis.y;
        printf("GAMEPAD LEFT  AXIS: [X]: %d | [Y]: %d\n", gamepad_lx,
               gamepad_ly);
        printf("GAMEPAD RIGHT AXIS: [X]: %d | [Y]: %d\n", gamepad_rx,
               gamepad_ry);
        break;
      case SAE_EVENT_GAMEPAD_RX_AXIS:
        gamepad_rx = ev.gamepad_axis.x;
        printf("GAMEPAD LEFT  AXIS: [X]: %d | [Y]: %d\n", gamepad_lx,
               gamepad_ly);
        printf("GAMEPAD RIGHT AXIS: [X]: %d | [Y]: %d\n", gamepad_rx,
               gamepad_ry);
        break;
      case SAE_EVENT_GAMEPAD_RY_AXIS:
        gamepad_ry = ev.gamepad_axis.y;
        printf("GAMEPAD LEFT  AXIS: [X]: %d | [Y]: %d\n", gamepad_lx,
               gamepad_ly);
        printf("GAMEPAD RIGHT AXIS: [X]: %d | [Y]: %d\n", gamepad_rx,
               gamepad_ry);
        break;

      case SAE_EVENT_MOUSE_MOVE_X:
        printf("[MOUSE][Y] %d\n", ev.mouse.move.x);
        break;
      case SAE_EVENT_MOUSE_MOVE_Y:
        printf("[MOUSE][X AXIS] %d\n", ev.mouse.move.y);
        break;
      case SAE_EVENT_MOUSE_WHEEL:
        printf("[MWHEEL] %d\n", ev.mouse.wheel);
        break;
      default:
        break;
      }

      switch (ev.keypad.key) {

      case SAE_KEY_H:
        printf("[KEY] H\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_KEY_J:
        printf("[KEY] J\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_KEY_K:
        printf("[KEY] K\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_KEY_L:
        printf("[KEY] L\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_KEY_LEFTCTRL:
        printf("[KEY] LEFTCTRL\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_BTN_NORTH:
        printf("[KEY] GAMEPAD TRIANGLE\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_BTN_DPAD_DOWN:
        printf("[KEY] GAMEPAD DOWN KEY\n");
        print_ev_keypress(ev.type);
        break;
      case SAE_BTN_TL2:
        printf("[KEY] GAMEPAD LOWER LEFT TRIGGER\n[PRESSURE] %d\n",
               ev.keypad.trigger_pressure);
        print_ev_keypress(ev.type);
        break;
      case SAE_KEY_P:
        printf("BREAKING LOOP!!!!\n");
        break_outof_queue = 1;
        break;

        // "UNKNOWN KEYBOARD KEY"
      default:
        break;
      }
      fflush(stdout);
    }

    if (break_outof_queue)
      break;
  }

  // FOR A CLEAN SHUTDOWN:
  // - User must remove all InputDevices from the Event System before
  // freeing the InputDevices

  // unsubcribe inputdevices from the event system
  sae_event_system_rmv_inputdevice_all(&ev_sys, &input_list);
  // close receiver and free channel (the said "queue")
  sae_event_system_rmv_queue(ev_queue);
  // free inputsdevices list
  sae_free_input_devices_list(input_list);
  // free pheripherals list
  sae_free_available_peripherals_list(peri_list);
  // free event system
  sae_free_event_system(ev_sys);
  return 0;
}
