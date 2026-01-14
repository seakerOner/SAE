#define SAE_RELEASE 0
#define SAE_DEBUG 1
#define SAE_TRACER 1

#include "core/core_base.h"
#include "core/core_base_impl.h"

#include "./core/core_sys_input.h"
#include "./core/core_sys_input_impl.h"

#include "./core/core_events.h"
#include "./core/core_events_impl.h"

#include <stdio.h>

int main() {
  PeripheralDeviceList peri_list = sae_get_available_peripherals_list();

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
    printf("[REL]       :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.rel);
    printf("[ABS]       :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.abs);
    printf("[HANDLER]   :%s\n",
           peri_list.items[x].inner_peripheral.linux_p.handler);
    printf("[EVENTPATH]:%s\n",
           peri_list.items[x].inner_peripheral.linux_p.event_path);
  }
  InputDeviceList input_list =
      sae_peripheralslist_to_inputdeviceslist(&peri_list);

  SAE_EventSystem ev_sys = sae_get_event_system();

  sae_event_system_add_inputdevice_list(&ev_sys, &input_list);

  sae_free_input_devices_list(input_list);
  sae_free_available_peripherals_list(peri_list);
  return 0;
}
