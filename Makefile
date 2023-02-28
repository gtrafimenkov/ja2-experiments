CLANG_FORMATTER ?= clang-format-13

format:
	find ja2 \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
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
COMPILE_FLAGS = -c -Wall --std=c17 -DFORCE_ASSERTS_ON -I./ja2lib

TARGET_ARCH    ?=
ifeq "$(TARGET_ARCH)" ""
BUILD_DIR      := tmp/default
else
BUILD_DIR      := tmp/$(TARGET_ARCH)
endif
# MAIN_SOURCES := $(shell grep -l "// build:linux" -r ja2lib)
MAIN_SOURCES := $(shell find ja2lib -name '*.c')
MAIN_OBJS0   := $(filter %.o, $(MAIN_SOURCES:.c=.o) $(MAIN_SOURCES:.cc=.o) $(MAIN_SOURCES:.cpp=.o))
MAIN_OBJS    := $(addprefix $(BUILD_DIR)/,$(MAIN_OBJS0))

LIBS         := -lpthread
# LIBS         += -lgtest

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC)  $(CFLAG) $(COMPILE_FLAGS) $(COVERAGE_FLAGS) -o $@ $<

tester-linux: $(MAIN_OBJS)
	$(CXX) $(CFLAG) $(COVERAGE_FLAGS) -o tester-linux \
		$(MAIN_OBJS) \
		$(LIBS)

test-linux: tester-linux
	cp ./tester-linux ../ja2-installed
	cd ../ja2-installed && ./tester-linux

###################################################################
#
###################################################################
