CC = gcc

DEBUG ?= 0
X11 ?= 0
WAYLAND ?= 0
OPENGL ?= 0

DEBUG_FLAG =  
X11_FLAG = 
WAYLAND_FLAG =
OPENGL_FLAG = 

# Detect X11 or Wayland (Linux only)
ifeq ($(shell echo $XDG_SESSION_TYPE), x11)
	X11 ?= 1
endif

ifeq ($(shell echo $XDG_SESSION_TYPE), wayland)
	WAYLAND ?= 1
endif

ifeq ($(DEBUG), 1)
	DEBUG_FLAG = -DDEBUG -g
endif

ifeq ($(X11), 1)
	X11_FLAG = -lX11 -lXrandr
endif

ifeq ($(WAYLAND), 1)
	WAYLAND_FLAG = -lwayland-client -lwayland-egl
endif

ifeq ($(OPENGL), 1)
	OPENGL_FLAG = -lGL
endif

BASE_FLAGS = -Wall -Wextra $(DEBUG_FLAG)

FLAGS = $(X11_FLAG) $(WAYLAND_FLAG) $(OPENGL_FLAG)

BUILD = ./build/

exec: main.c
	$(CC) $(BASE_FLAGS) main.c $(FLAGS) -o $(BUILD)exec

run:
	make
	@echo "Compiled!!"
	@echo " "
	$(BUILD)exec

runsudo:
	make
	@echo "Compiled!!"
	@echo "Running with sudo permissions!"
	@echo " "
	sudo $(BUILD)exec
	
runsudo OPENGL:
	make OPENGL=1 X11=1
	@echo "Compiled with OPENGL (X11/WAYLAND enabled by default on Linux)!!"
	@echo "Running with sudo permissions!"
	@echo " "
	sudo $(BUILD)exec

debug:
	@echo "Compiling with debug symbols..."
	make DEBUG=1
	@echo "Debug executable at ./build/exec"

clean:
	rm -f $(BUILD)exec

examples example_input_event_system:
	@echo "Compiling: input_event_system_example..."
	$(CC) $(BASE_FLAGS) ./examples/example_input_event_system.c -o $(BUILD)input_event_system_example
	@echo "Compiled!!"
	@echo "Running with sudo permissions!"
	@echo " "
	@echo "==================================="
	@echo "Running example: Input Event System"
	@echo "==================================="
	sudo $(BUILD)input_event_system_example
