#define SAE_DEBUG 1
#define SAE_TRACER 1

#define CORE_BASE_IMPLEMENTATION
//#include "core/core_base.h"
#define CORE_SYS_INPUT_IMPLEMENTATION
#include "./core/core_sys_input.h"
#include <stdio.h>

int main() {
  PeripheralDeviceList list = sae_get_available_peripherals_list();

  for (usize x = 0; x < list.num_items; x += 1) {
    // if (!list.items[x].inner_peripheral.linux_p.name)
    //   continue;
    printf("\n===========================\n");
    printf("[ID]        :%ld\n", x);
    switch (list.items[x].type) {
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
    printf("[NAME]      :%s\n", list.items[x].inner_peripheral.linux_p.name);
    printf("[PHYS]      :%s\n", list.items[x].inner_peripheral.linux_p.phys);
    printf("[EV]        :%s", list.items[x].inner_peripheral.linux_p.ev);
    printf("[REL]       :%s\n", list.items[x].inner_peripheral.linux_p.rel);
    printf("[ABS]       :%s\n", list.items[x].inner_peripheral.linux_p.abs);
    printf("[HANDLER]   :%s\n", list.items[x].inner_peripheral.linux_p.handler);
    printf("[EVENTPATH]:%s\n",
           list.items[x].inner_peripheral.linux_p.event_path);
  }

  sae_free_available_peripherals_list(list);
  return 0;
}
