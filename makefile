CC = gcc

DEBUG = 0

ifeq ($(DEBUG), 0)
FLAGS = -Wall -Wextra
else
FLAGS = -Wall -Wextra -g -DDEBUG
endif

BUILD = ./build/

exec: main.c
	$(CC) $(FLAGS) main.c -o $(BUILD)exec

run:
	make
	@echo "Compiled!!"
	@echo " "
	$(BUILD)exec

debug:
	@echo "Compiling with debug symbols..."
	make DEBUG=1
	@echo "Debug executable at ./build/..."

clean:
	rm -f $(BUILD)exec

