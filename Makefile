# Directories
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = bin
BIN_DIR = bin

# Compiler
CC = g++
CFLAGS = -O2 -std=c++17 $(shell pkg-config --cflags libpipewire-0.3) -I$(INC_DIR)
LDFLAGS = $(shell pkg-config --libs libpipewire-0.3)

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Detect which .cpp files contain main()
MAINS := $(shell grep -l "int[[:space:]]\+main" $(SRC_DIR)/*.cpp 2>/dev/null)
MAIN_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(MAINS))
UTIL_OBJS := $(filter-out $(MAIN_OBJS), $(OBJS))

# Binaries: one per main file
BINS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%, $(MAINS))

# Default target
all: $(BINS)

# Each binary links its own .o and all utility .o files (no other mains)
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(UTIL_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure dirs exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
