EXE = void.exe

CC = g++
CFLAGS = -Ideps/include -Wall -std=c++11 -O2
LDFLAGS = -Ldeps/lib -langelscript -lraylib -lopengl32 -lgdi32 -lwinmm

BUILD_DIR = build

SRCS = $(wildcard src/*.cpp) $(wildcard deps/src/*.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/$(EXE) run

$(BUILD_DIR)/$(EXE): $(OBJS)
	$(CC) $(OBJS) -o $(BUILD_DIR)/$(EXE) $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@if not exist $(BUILD_DIR)\src mkdir $(BUILD_DIR)\src
	@if not exist $(BUILD_DIR)\deps mkdir $(BUILD_DIR)\deps
	@if not exist $(BUILD_DIR)\deps\src mkdir $(BUILD_DIR)\deps\src

	$(CC) $(CFLAGS) -c $< -o $@

run: $(BUILD_DIR)/$(EXE)
	$(BUILD_DIR)/$(EXE)

.PHONY: clean
clean:
	rmdir /s $(BUILD_DIR)
