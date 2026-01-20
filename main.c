#include <string.h>
#define SAE_RELEASE 0
#define SAE_DEBUG 1
#define SAE_TRACER 1

#include <stdio.h>

#define RGFW_IMPLEMENTATION
#define RGFW_DEBUG 1
//#define RGFW_EXPORT
//#define RGFW_IMPORT
#include "./core/RGFW-1.8.1/RGFW.h"
#include <GL/gl.h>

int main() {
  RGFW_window *win = RGFW_createWindow("name", 100, 100, 500, 500, (u64)0);
  RGFW_event event;

  RGFW_window_setExitKey(win, RGFW_escape);
  RGFW_window_setIcon(win, NULL, 3, 3, RGFW_formatRGBA8);

  while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
    while (RGFW_window_checkEvent(win, &event)) {
      if (event.type == RGFW_quit)
        break;
    }
  }

  RGFW_window_close(win);

  return 0;
}
