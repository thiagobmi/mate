CC = gcc
CFLAGS = -Wall -Wextra -g -O3
LDFLAGS = -lm -lncurses -lffi

SRC_DIR = src
BUILD_DIR = build
SRCS = $(addprefix $(SRC_DIR)/, main.c node.c token.c dictionary.c tokenizer.c parser.c evaluator.c eval.c test.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = mate

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rmdir $(BUILD_DIR) 2>/dev/null || true

.PHONY: all clean
