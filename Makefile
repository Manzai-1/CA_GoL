# Compiler and flags
CC = emcc
CFLAGS = -O2 -Wall -Wextra
EMFLAGS = -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 \
	-s EXPORTED_FUNCTIONS='["_main", "_js_pan", "_js_zoom", "_js_get_min_speed", "_js_get_max_speed", "_js_set_speed", "_js_toggle_pause", "_js_get_paused", "_js_get_generations"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
	--shell-file web/shell.html

# Paths
SRC = casdl.c
BUILD_DIR = build
OUTPUT = $(BUILD_DIR)/index.html

# Default target
all: $(OUTPUT)

$(OUTPUT): $(SRC) config.h web/shell.html | $(BUILD_DIR)
	$(CC) $(SRC) -o $(OUTPUT) $(CFLAGS) $(EMFLAGS)
	cp web/styles.css $(BUILD_DIR)/
	cp web/controls.js $(BUILD_DIR)/

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

serve: all
	cd $(BUILD_DIR) && python3 -m http.server 8000

.PHONY: all clean serve