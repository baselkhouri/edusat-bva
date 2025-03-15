# Compiler
CXX := g++

# Common warnings/flags
IGNORED_WARNINGS := -Wno-unused-variable -Wno-unused-but-set-variable -Wno-reorder-ctor
CXXFLAGS := -Wall -std=c++17 -I src $(IGNORED_WARNINGS)

# Define flags for debug vs. release
DEBUG_FLAGS := -g -O0          # Debug: produce symbols, no optimizations
RELEASE_FLAGS := -O3 -DNDEBUG  # Release: high optimization, disable asserts

# You can set BUILD_TYPE on the command line, e.g.:
#    make BUILD_TYPE=release
BUILD_TYPE ?= debug

# Set build directory and extra flags depending on BUILD_TYPE
ifeq ($(BUILD_TYPE),debug)
  BUILD_DIR := build/debug
  CXXFLAGS += $(DEBUG_FLAGS)
else ifeq ($(BUILD_TYPE),release)
  BUILD_DIR := build/release
  CXXFLAGS += $(RELEASE_FLAGS)
else
  # fallback to debug if not recognized
  $(warning Unrecognized BUILD_TYPE=$(BUILD_TYPE). Using debug mode.)
  BUILD_DIR := build/debug
  CXXFLAGS += $(DEBUG_FLAGS)
endif

# Final binary
BIN := $(BUILD_DIR)/edusat

# Source/object lists
SRC_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Default rule: build the chosen configuration
.PHONY: all
all: $(BIN)
	@$(MAKE) clean_objects

# Build the final executable
$(BIN): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build each .o from corresponding .cpp
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Provide shorthand targets for debug and release
.PHONY: debug release
debug:
	@$(MAKE) all BUILD_TYPE=debug

release:
	@$(MAKE) all BUILD_TYPE=release

# Clean everything
.PHONY: clean
clean:
	rm -rf build

# Remove only the object files from current BUILD_TYPE folder
.PHONY: clean_objects
clean_objects:
	rm -f $(OBJ_FILES)

# Run the executable
.PHONY: run
run: $(BIN)
	./$(BIN)
