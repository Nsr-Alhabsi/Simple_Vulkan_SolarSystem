# --- COMPILER AND FLAGS ---
CXX      := g++
CXXFLAGS := -std=c++17 -O3 -Wall
LDFLAGS  := -lglfw3 -lvulkan-1 -lgdi32 -luser32 -lshell32 -mconsole

# --- DIRECTORIES ---
SRC_DIR     := src
BUILD_DIR   := build
BIN_NAME    := MyVulkanProject.exe

# --- DIRECTORIES ---
SRC_DIR := src
# Explicitly list the subfolders you created
SOURCES := $(wildcard $(SRC_DIR)/*.cpp) \
           $(wildcard $(SRC_DIR)/core/*.cpp) \
           $(wildcard $(SRC_DIR)/models/*.cpp) \
           $(wildcard $(SRC_DIR)/ADDONS/*.cpp)

# This converts src/core/file.cpp to build/core/file.o
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# --- DEBUG TARGET ---
debug:
	@echo "Current Directory: $(CURDIR)"
	@echo "Looking in: $(SRC_DIR)"
	@echo "Sources found: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"

# --- TARGETS ---
all: $(BUILD_DIR)/$(BIN_NAME)

$(BUILD_DIR)/$(BIN_NAME): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	@echo "Launching..."
	./$(BUILD_DIR)/$(BIN_NAME)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean debug