CC = gcc
CFLAGS = -g -Wall -Wpedantic
SRC_DIR = ./src
BUILD_DIR = ./build

SOURCES = main lexer error parser ast stringt interpreter

OBJECTS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(SOURCES)))
EXEC = $(BUILD_DIR)/lamb

# Link exec
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)