CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c17 -Iinclude
BUILD_DIR=build
SRC=src/main.c src/fs.c
OBJ=$(SRC:src/%.c=$(BUILD_DIR)/%.o)
BIN=$(BUILD_DIR)/gsea

$(BIN): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c include/gsea.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run
clean:
	rm -rf $(BUILD_DIR)

run: $(BIN)
	$(BIN)
