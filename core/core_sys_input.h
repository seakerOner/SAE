#ifndef CORE_SYS_INPUT_H
#define CORE_SYS_INPUT_H
/*==================================================*/
/*      General / Platform Dependent includes       */
/*==================================================*/
#include "./core_base.h"
#include "./seakcutils/data_structures/deque.h"
#include "./seakcutils/data_structures/linkedlist.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/*==================================================*/
/* API / Types                                      */
/*==================================================*/
// header types section

typedef enum OS_Used_t { SAE_OS_LINUX, SAE_OS_WIN64, SAE_OS_APPLE } OS_Used;

// Doesnt include type UNK
#define SAE_PERIPHERAL_T_ALL_KNOWN 0x07
#define SAE_PERIPHERAL_T_ALL 0x0F

typedef enum PeripheralType_t {
  SAE_PERIPHERAL_T_KEYBOARD = 0x01,
  SAE_PERIPHERAL_T_MOUSE = 0x02,
  SAE_PERIPHERAL_T_GAMEPAD = 0x04,
  SAE_PERIPHERAL_T_UNKNOWN = 0x08,
} PeripheralType;

typedef struct _LinuxPeripheralDevice_t {
  ascii *name;
  usize name_len;
  ascii *phys;
  usize phys_len;
  ascii *handler;
  usize handler_len;
  ascii *ev;
  usize ev_len;
  ascii *rel; // for mouse
  usize rel_len;
  ascii *abs; // for gamepads
  usize abs_len;
  ascii event_path[__SAE_LINUX_DEVICES_EVENT_PATH_MAX_SIZE];
} _LinuxPeripheralDevice;

typedef struct _Win64PeripheralDevice_t {
} _Win64PeripheralDevice;

typedef struct _ApplePeripheralDevice_t {
} _ApplePeripheralDevice;

typedef struct PeripheralDevice_t {
  usize id;
  PeripheralType type;
  union { // OS is the descriminator for the union
    _LinuxPeripheralDevice linux_p;
    _Win64PeripheralDevice wind64_p;
    _ApplePeripheralDevice apple_p;
  } inner_peripheral;
} PeripheralDevice;

typedef struct PeripheralDeviceList_t {
  PeripheralDevice *items;
  usize num_items;
  usize cap;
} PeripheralDeviceList;

typedef struct InputDevice_t {
  usize id;
  PeripheralType type;
  union { // OS is the descriminator for the union
    int linux_fd;
    void *windows_handle;
    void *mac_handle;
  };
} InputDevice;

typedef struct InputDeviceList_t {
  LinkedList devices; // LinkedList<T> where T = InputDevice
} InputDeviceList;

// header api section

// Get a list of available peripherals on system
PeripheralDeviceList sae_get_available_peripherals_list();
void sae_free_available_peripherals_list(PeripheralDeviceList);

// Turn a list of Peripherals into a list of Input Devices.
//
// Input Devices are file descriptors of the peripherals, they
// are ready to be consumed by the Event System
InputDeviceList
sae_peripheralslist_to_inputdeviceslist(PeripheralDeviceList *peri_list,
                                        u8 peri_type_flags);

int sae_free_input_device(InputDeviceList input_list, usize id);

void sae_free_input_devices_list(InputDeviceList input_list);

// internal functions
int __sae_try_set_event_path(PeripheralDevice *peri);
#endif
