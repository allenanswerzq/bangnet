Q := @
# All of the directories containing code.
SRC_DIRS := $(shell find * -type d -exec bash -c "find {} -maxdepth 1 \
	\( -name '*.cc' \) | grep -q ." \; -print)

# Get all source files.
CXX_SRCS := $(shell find $(SRC_DIRS) ! -name "*_test.cc" -name "*.cc")
TEST_MAIN_SRC := $(SRC_DIRS)/test_main.cc
TEST_SRCS := $(shell find $(SRC_DIRS) -name "*_test.cc")

BUILD_DIR := build
ALL_BUILD_DIRS := $(sort $(BUILD_DIR) $(addprefix $(BUILD_DIR)/, $(SRC_DIRS)))

# The objects corresponding to the source files.
CXX_OBJS := $(addprefix $(BUILD_DIR)/, ${CXX_SRCS:.cc=.o})

# All the warning txt files. 
WARNS_TXT := warning.txt
CXX_WARNS := $(addprefix $(BUILD_DIR)/, ${CXX_SRCS:.cc=.o.$(WARNS_EXT)})

LIBRARIES += glog gflags protobuf

INCLUDE_DIRS += .
COMMON_FLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += $(COMMON_FLAGS) --std=c++11
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(PKG_CONFIG) \
		$(foreach library,$(LIBRARIES),-l$(library))

# Determine platform.
UNAME := $(shell uname -s)
ifeq ($(UNAME), Linux)
	LINUX := 1
else ifeq ($(UNAME), Darwin)
	OSX := 1
endif

# Linux
ifeq ($(LINUX), 1)
	CXX ?= /usr/bin/g++
else
	CXX ?= /usr/bin/clang
endif 

.PHONY: all clean runtest

all: elf

elf: $(CXX_OBJS)

$(ALL_BUILD_DIRS): 
	@ mkdir -p $@

# Build object files from souce cpp files.
$(BUILD_DIR)/%.o: %.cc | $(ALL_BUILD_DIRS)
	@ echo CXX $<
	$(Q)$(CXX) $< $(CXXFLAGS) -c -o $@ 2> $@.$(WARNS_TXT) \
		|| (cat $@.$(WARNS_TXT); exit 1)

clean:
	rm -rf $(ALL_BUILD_DIRS)

print-%:
	@echo $* = $($*)