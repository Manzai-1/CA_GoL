# Compiler and flags
CC = emcc
CFLAGS = -O2 -Wall -Wextra
EMFLAGS = -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1

# Paths
SRC = casdl.c
BUILD_DIR = build
OUTPUT = $(BUILD_DIR)/index.html

# Default target
all: $(OUTPUT)

$(OUTPUT): $(SRC) config.h | $(BUILD_DIR)
	$(CC) $(SRC) -o $(OUTPUT) $(CFLAGS) $(EMFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

serve: all
	cd $(BUILD_DIR) && python3 -m http.server 8000

.PHONY: all clean serve