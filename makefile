MAIN_DIR=youtube_looper
BUILD_DIR=build
FILE ?= driver

ARGS ?= "assets/mirror_tune.mp4"

INCLUDE = -I/Library/Frameworks/GStreamer.framework/Headers -I/opt/homebrew/include
LIBS = -F/Library/Frameworks \
        -framework GStreamer \
        -Wl,-rpath,/Library/Frameworks \
        -L/opt/homebrew/lib \
        -Wl,-rpath,/opt/homebrew/lib \
        -lraylib \
        -lavcodec -lavformat -lavutil -lswscale
# Load .env if present and export its variables to recipes
ifneq (,$(wildcard .env))
  include .env
  export $(shell sed -n 's/^\([A-Za-z_][A-Za-z0-9_]*\)=.*/\1/p' .env)
endif

default: build link run

build:
	clang -c $(MAIN_DIR)/driver/$(FILE).c -o $(BUILD_DIR)/$(FILE).o $(INCLUDE)
	clang -c $(MAIN_DIR)/actions.c -o $(BUILD_DIR)/actions.o $(INCLUDE)
	clang -c $(MAIN_DIR)/state.c -o $(BUILD_DIR)/state.o $(INCLUDE)
	clang -c $(MAIN_DIR)/utils.c -o $(BUILD_DIR)/utils.o $(INCLUDE)
	clang -c $(MAIN_DIR)/assets.c -o $(BUILD_DIR)/assets.o $(INCLUDE)
	clang -c $(MAIN_DIR)/gui_manager.c -o $(BUILD_DIR)/gui_manager.o $(INCLUDE)
	clang -c $(MAIN_DIR)/gui_objects.c -o $(BUILD_DIR)/gui_objects.o $(INCLUDE)

link:
	clang $(BUILD_DIR)/$(FILE).o  $(BUILD_DIR)/assets.o $(BUILD_DIR)/actions.o $(BUILD_DIR)/state.o $(BUILD_DIR)/utils.o \
	-o $(BUILD_DIR)/$(FILE) $(LIBS)

link_so: build
	clang -shared -undefined dynamic_lookup $(BUILD_DIR)/$(FILE).o $(BUILD_DIR)/gui_manager.o $(BUILD_DIR)/gui_objects.o $(BUILD_DIR)/assets.o $(BUILD_DIR)/state.o $(BUILD_DIR)/utils.o \
	-o $(BUILD_DIR)/$(FILE).so $(INCLUDE) $(LIBS)

	clang -shared -undefined dynamic_lookup $(MAIN_DIR)/actions.c \
	-o $(BUILD_DIR)/actions.so $(INCLUDE) $(LIBS)


run: 
	./$(BUILD_DIR)/$(FILE) $(ARGS)

driver: build link_so
	# .venv/bin/activate
	python3 youtube_looper/driver/driver.py $(ARGS)

test: build link
	truncate -s 0 leaks # clear the file
	leaks -list -nostacks -nosources --atExit -- ./$(FILE) $(ARGS) >> leaks
