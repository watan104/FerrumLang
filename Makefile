CC = gcc
AS = nasm
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -Iinclude
LDFLAGS = -lm
DEBUG_FLAGS = -g -O0
RELEASE_FLAGS = -O3

# Hedefler
TARGET = ferrumc
TEST_TARGETS = test_lexer test_parser

# Kaynak dosyalar
SRC_DIR = src
BOOTSTRAP_DIR = bootstrap
RUNTIME_DIR = src/runtime
TEST_DIR = tests/unit

SRCS = $(wildcard $(SRC_DIR)/compiler/*.c)
BOOTSTRAP_SRCS = $(BOOTSTRAP_DIR)/minimal_libc.c $(BOOTSTRAP_DIR)/start.s
RUNTIME_SRCS = $(wildcard $(RUNTIME_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)

OBJS = $(SRCS:.c=.o) $(BOOTSTRAP_SRCS:.c=.o) $(BOOTSTRAP_SRCS:.s=.o) $(RUNTIME_SRCS:.c=.o)
TEST_OBJS = $(filter-out $(SRC_DIR)/compiler/main.o, $(OBJS)) $(TEST_SRCS:.c=.o)

# Varsayılan hedef
all: $(TARGET)

# Ana derleyici
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $^ -o $@ $(LDFLAGS)

# Testler
test: $(TEST_TARGETS)
	./test_lexer
	./test_parser

test_lexer: $(filter-out $(SRC_DIR)/compiler/main.o, $(OBJS)) $(TEST_DIR)/test_lexer.o
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -o $@ $(LDFLAGS)

test_parser: $(filter-out $(SRC_DIR)/compiler/main.o, $(OBJS)) $(TEST_DIR)/test_parser.o
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -o $@ $(LDFLAGS)

# Derleme kuralları
%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

%.o: %.s
	$(AS) -f elf64 $< -o $@

# Temizleme
clean:
	rm -f $(TARGET) $(TEST_TARGETS) $(OBJS) $(TEST_OBJS)

.PHONY: all test clean