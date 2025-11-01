# Directories
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
BIN_DIR = bin

# Compiler
CC = gcc
CFLAGS = -O2 $(shell pkg-config --cflags libpipewire-0.3) $(addprefix -I, $(shell find $(INC_DIR) -type d))
LDFLAGS = $(shell pkg-config --libs libpipewire-0.3) -lm

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Detect which .cpp files contain main()
MAINS := $(shell grep -l "int[[:space:]]\+main" $(SRC_DIR)/*.c 2>/dev/null)
MAIN_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(MAINS))
UTIL_OBJS := $(filter-out $(MAIN_OBJS), $(OBJS))

# Binaries: one per main file
BINS := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(MAINS))

# Default target
all: $(BINS)
	@echo "Cleaning object directory..."
	@rm -rf $(OBJ_DIR)

# Each binary links its own .o and all utility .o files (no other mains)
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(UTIL_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure dirs exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Ensure dirs exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
