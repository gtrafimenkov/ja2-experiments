CLANG_FORMATTER ?= clang-format-13

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
COMPILE_FLAGS = -c --std=c17 -DFORCE_ASSERTS_ON -I./ja2lib

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

LIBS         := -lpthread
# LIBS         += -lgtest

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC)  $(CFLAG) $(COMPILE_FLAGS) $(COVERAGE_FLAGS) -o $@ $<

ja2lib.a: $(JA2LIB_OBJS)
	ar rcs ja2lib.a $(JA2LIB_OBJS)

tester-linux: $(JA2LIB_OBJS)
	$(CXX) $(CFLAG) $(COVERAGE_FLAGS) -o tester-linux \
		$(JA2LIB_OBJS) \
		$(LIBS)

test-linux: tester-linux
	cp ./tester-linux ../ja2-installed
	cd ../ja2-installed && ./tester-linux

###################################################################
#
###################################################################
