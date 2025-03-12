# Compiler
CXX := g++
# We decided to ignore some warnings to not change the code.
# The existence of 'wrong' code might give us hints where to add new features.
# The reorder-ctor was ignored because its frustrating to fix the order of the initializers.
IGNORED_WARNINGS := -Wno-unused-variable -Wno-unused-but-set-variable -Wno-reorder-ctor
CXXFLAGS := -Wall -std=c++17 -I src $(IGNORED_WARNINGS)

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN := $(BUILD_DIR)/edusat

# Source and object files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Default target
all: $(BIN)
	@$(MAKE) clean_objects

# Build executable
$(BIN): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# Clean object files after binary is created
clean_objects:
	rm -f $(OBJ_FILES)

# Run executable
run: $(BIN)
	./$(BIN)

.PHONY: all clean run
