TARGET_EXEC ?= myprogram
TARGET_TEST ?= test-lab

BUILD_DIR ?= build
TEST_DIR ?= tests
SRC_DIR ?= src
EXE_DIR ?= app

SRCS := $(shell find $(SRC_DIR) -name "*.c")
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TEST_SRCS := $(shell find $(TEST_DIR) -name "*.c")
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJS:.o=.d)

EXE_SRCS := $(shell find $(EXE_DIR) -name "*.c")
EXE_OBJS := $(EXE_SRCS:%=$(BUILD_DIR)/%.o)
EXE_DEPS := $(EXE_OBJS:.o=.d)

CFLAGS ?= -Wall -Wextra -MMD -MP -I$(SRC_DIR)
DEBUG ?= -g
SANITIZE ?= -fno-omit-frame-pointer -fsanitize=address

# Link against readline and ncurses to resolve undefined references
LDFLAGS ?= -lreadline -lncurses

# Default to building without debug flags
all: $(TARGET_EXEC) $(TARGET_TEST)

# Build with debug flags and address sanitizer
debug: CFLAGS += $(SANITIZE)
debug: CFLAGS += $(DEBUG)
debug: $(TARGET_EXEC) $(TARGET_TEST)

# Build main program (ensuring app/main.c is linked correctly)
$(TARGET_EXEC): $(OBJS) $(EXE_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(EXE_OBJS) -o $@ $(LDFLAGS)

# Build test executable
$(TARGET_TEST): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(TEST_OBJS) -o $@ $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests
check: $(TARGET_TEST)
	ASAN_OPTIONS=detect_leaks=1 ./$<

.PHONY: clean
clean:
	$(RM) -rf $(BUILD_DIR) $(TARGET_EXEC) $(TARGET_TEST)

# Install required dependencies for GNU Readline (for Codespaces)
.PHONY: install-deps
install-deps:
	sudo apt-get update -y
	sudo apt-get install -y libreadline-dev libncurses-dev

# Include dependency files for incremental builds
-include $(DEPS) $(TEST_DEPS) $(EXE_DEPS)
