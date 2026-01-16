#include "sae_input_list_names.h"
#define SAE_RELEASE 0
#define SAE_DEBUG 1
#define SAE_TRACER 1
// #include "core/seakcutils/channels/channels.h"

#include "core/core_base.h"
#include "core/core_base_impl.h"

#include "./core/core_sys_input.h"
#include "./core/core_sys_input_impl.h"

#include "./core/core_events.h"
#include "./core/core_events_impl.h"

#include "core/seakcutils/arenas/r_arena.h"
#include "core/seakcutils/channels/mpmc.h"
#include "core/seakcutils/job_system/jobsystem.h"
#include "core/seakcutils/threadpool/threadpool.h"

#include <stdio.h>

void set_events(void *event_sys) {
  SAE_EventSystem *ev_sys = (SAE_EventSystem *)event_sys;
  sae_event_system_execute(ev_sys);
}

void print_peri_list(PeripheralDeviceList peri_list) {
  for (usize x = 0; x < peri_list.num_items; x += 1) {
    printf("\n===========================\n");
    printf("[ID]        :%ld\n", x);
    switch (peri_list.items[x].type) {
    case SAE_PERIPHERAL_T_UNKNOWN:
      printf("[TYPE]      :SAE_PERIPHERAL_T_UNKNOWN\n");
      break;
    case SAE_PERIPHERAL_T_KEYBOARD:
      printf("[TYPE]      :SAE_PERIPHERAL_T_KEYBOARD\n");
      break;
    case SAE_PERIPHERAL_T_MOUSE:
      printf("[TYPE]      :SAE_PERIPHERAL_T_MOUSE\n");
      break;
    case SAE_PERIPHERAL_T_GAMEPAD:
      printf("[TYPE]      :SAE_PERIPHERAL_T_GAMEPAD\n");
      break;
    }
    printf("[NAME]      :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.name);
    printf("[PHYS]      :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.phys);
    printf("[EV]        :%s", peri_list.items[x].inner_peripheral.linux_p.ev);
    printf("[REL] :%s\n", peri_list.items[x].inner_peripheral.linux_p.rel);
    printf("[ABS]       :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.abs);
    printf("[HANDLER]   :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.handler);
    printf("[EVENTPATH]:%s\n",
           peri_list.items[x].inner_peripheral.linux_p.event_path);
  }
}

int main() {
  ThreadPool *pool = threadpool_init_for_scheduler(4);
  job_scheduler_spawn(pool);

  PeripheralDeviceList peri_list = sae_get_available_peripherals_list();
  // print_peri_list(peri_list);

  InputDeviceList input_list =
      sae_peripheralslist_to_inputdeviceslist(&peri_list);

  SAE_EventSystem ev_sys = sae_get_event_system();

  ReceiverSpmc *ev_queue = sae_event_system_get_queue(&ev_sys);

  sae_event_system_add_inputdevice_list(&ev_sys, &input_list);

  JobHandle *event_sys_for_thread = job_spawn(set_events, &ev_sys);
  job_wait(event_sys_for_thread);

  int break_outof_queue = 0;
  while (1) {
    SAE_Event ev;

    while (spmc_try_recv(ev_queue, &ev) == CHANNEL_OK) {

      switch (ev.type) {
      case SAE_EVENT_KEY_UP:
        printf("\n[KEYUP]\n");
        break;
      case SAE_EVENT_KEY_DOWN:
        printf("\n[KEYDOWN]\n");
        break;
      case SAE_EVENT_MOUSE_BUTTON_UP:
        printf("\n[MOUSEBUTTONUP]\n");
        break;
      case SAE_EVENT_MOUSE_BUTTON_DOWN:
        printf("\n[MOUSEBUTTONDOWN]\n");
        break;
      case SAE_EVENT_GAMEPAD_BUTTON_UP:
        printf("\n[GAMEPADBUTTONUP]\n");
        break;
      case SAE_EVENT_GAMEPAD_BUTTON_DOWN:
        printf("\n[GAMEPADBUTTONDOWN]\n");
        break;
      default:
        break;
      }

      switch (ev.keypad.key) {
      case SAE_KEY_H:
        printf("[TIMESTAMP] %ld:%ld\n", ev.timestamp.seconds,
               ev.timestamp.microseconds);
        printf("[KEYPRESSED] H\n");
        break;
      case SAE_KEY_J:
        printf("[TIMESTAMP] %ld:%ld\n", ev.timestamp.seconds,
               ev.timestamp.microseconds);
        printf("[KEYPRESSED] J\n");
        break;
      case SAE_KEY_K:
        printf("[TIMESTAMP] %ld:%ld\n", ev.timestamp.seconds,
               ev.timestamp.microseconds);
        printf("[KEYPRESSED] K\n");
        break;
      case SAE_KEY_L:
        printf("[TIMESTAMP] %ld:%ld\n", ev.timestamp.seconds,
               ev.timestamp.microseconds);
        printf("[KEYPRESSED] L\n");
        break;
      case SAE_KEY_LEFTCTRL:
        printf("[TIMESTAMP] %ld:%ld\n", ev.timestamp.seconds,
               ev.timestamp.microseconds);
        printf("[KEYPRESSED] LEFTCTRL\n");
        break;
      case SAE_BTN_NORTH:
        printf("[TIMESTAMP] %ld:%ld\n", ev.timestamp.seconds,
               ev.timestamp.microseconds);
        printf("[KEYPRESSED] GAMEPAD TRIANGLE\n");
        break;
      case SAE_KEY_P:
        printf("BREAKING LOOP!!!!\n");
        break_outof_queue = 1;
        sae_event_system_rmv_queue(ev_queue);
        break;

        // "UNKNOWN KEYBOARD KEY"
      default:
        break;
      }
    }

    if (break_outof_queue)
      break;
  }

  sae_event_system_rmv_inputdevice_all(&ev_sys, &input_list);
  sae_free_input_devices_list(input_list);
  sae_free_available_peripherals_list(peri_list);
  sae_free_event_system(ev_sys);
  return 0;
}
