CLANG_FORMATTER ?= clang-format-13

.PHONY: format format-modified tester-linux-bin tester-bin run-linux-tester run-tester

format:
	find . \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
		| xargs $(CLANG_FORMATTER) -i --style=file

format-modified:
	git status --porcelain | egrep -e '[.](c|cc|cpp|h)$$' | awk '{print $$2}' \
		| xargs $(CLANG_FORMATTER) -i --style=file

###################################################################
# linux build
###################################################################

CC = gcc
CXX	= g++
CFLAG = -fPIC
# COMPILE_FLAGS = -c -Wall -Werror -DFORCE_ASSERTS_ON -I./ja2lib
# COMPILE_FLAGS = -c -Wall --std=c17 -DFORCE_ASSERTS_ON -I./ja2lib
COMPILE_FLAGS = --std=gnu17 -DFORCE_ASSERTS_ON -I./ja2lib

TARGET_ARCH    ?=
ifeq "$(TARGET_ARCH)" ""
BUILD_DIR      := tmp/default
else
BUILD_DIR      := tmp/$(TARGET_ARCH)
endif
# JA2LIB_SOURCES := $(shell grep -l "// build:linux" -r ja2lib)
JA2LIB_SOURCES := $(shell find ja2lib -name '*.c')
JA2LIB_OBJS0   := $(filter %.o, $(JA2LIB_SOURCES:.c=.o) $(JA2LIB_SOURCES:.cc=.o) $(JA2LIB_SOURCES:.cpp=.o))
JA2LIB_OBJS    := $(addprefix $(BUILD_DIR)/,$(JA2LIB_OBJS0))

TESTER_PLATFORM_SOURCES := $(shell find tester -name '*.c' -o -name '*.cc' -o -name '*.cpp')
TESTER_PLATFORM_OBJS0   := $(filter %.o, $(TESTER_PLATFORM_SOURCES:.c=.o) $(TESTER_PLATFORM_SOURCES:.cc=.o) $(TESTER_PLATFORM_SOURCES:.cpp=.o))
TESTER_PLATFORM_OBJS    := $(addprefix $(BUILD_DIR)/,$(TESTER_PLATFORM_OBJS0))

DUMMY_PLATFORM_SOURCES := $(shell find platform-dummy -name '*.c')
DUMMY_PLATFORM_OBJS0   := $(filter %.o, $(DUMMY_PLATFORM_SOURCES:.c=.o) $(DUMMY_PLATFORM_SOURCES:.cc=.o) $(DUMMY_PLATFORM_SOURCES:.cpp=.o))
DUMMY_PLATFORM_OBJS    := $(addprefix $(BUILD_DIR)/,$(DUMMY_PLATFORM_OBJS0))

LINUX_PLATFORM_SOURCES := $(shell find platform-linux -name '*.c')
LINUX_PLATFORM_OBJS0   := $(filter %.o, $(LINUX_PLATFORM_SOURCES:.c=.o) $(LINUX_PLATFORM_SOURCES:.cc=.o) $(LINUX_PLATFORM_SOURCES:.cpp=.o))
LINUX_PLATFORM_OBJS    := $(addprefix $(BUILD_DIR)/,$(LINUX_PLATFORM_OBJS0))

LIBS         := -lpthread
# LIBS         += -lgtest

$(BUILD_DIR)/%.o: %.c
	@echo .. compiling $<
	@mkdir -p $$(dirname $@)
	@$(CC) $(CFLAG) -c $(COMPILE_FLAGS) -o $@ $<

libs: $(BUILD_DIR)/ja2lib.a $(BUILD_DIR)/dummy-platform.a

$(BUILD_DIR)/ja2lib.a: $(JA2LIB_OBJS)
	@echo .. building $@
	@ar rcs $@ $^

$(BUILD_DIR)/dummy-platform.a: $(DUMMY_PLATFORM_OBJS)
	@echo .. building $@
	@ar rcs $@ $^

$(BUILD_DIR)/linux-platform.a: $(LINUX_PLATFORM_OBJS)
	@echo .. building $@
	@ar rcs $@ $^

linux-tester-bin: $(BUILD_DIR)/linux-tester

$(BUILD_DIR)/linux-tester: linux-tester/main.c $(BUILD_DIR)/linux-platform.a $(BUILD_DIR)/ja2lib.a
	@echo .. building $@
	@$(CC) $(CFLAG) $(COMPILE_FLAGS) $^ -o $@

tester-bin: $(BUILD_DIR)/tester

$(BUILD_DIR)/tester: $(TESTER_PLATFORM_OBJS) $(BUILD_DIR)/ja2lib.a
	@echo .. building $@
	@$(CXX) $(CFLAG) $(COMPILE_FLAGS) $^ -o $@ -lgtest_main -lgtest -lpthread

run-tester: $(BUILD_DIR)/tester
	./$(BUILD_DIR)/tester

###################################################################
#
###################################################################

install-build-dependencies-deb:
	sudo apt install clang-format-13 libgtest-dev
