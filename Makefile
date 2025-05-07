# STL Viewer Makefile

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lGL -lGLEW -lglfw -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/stl_viewer

# Source files
SRCS = $(SRC_DIR)/stl_viewer.cpp
HEADERS = $(SRC_DIR)/stl_viewer.hpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Debug build settings
ifeq ($(DEBUG),1)
    CXXFLAGS += -g -DDEBUG
    BUILD_DIR := $(BUILD_DIR)/debug
else
    CXXFLAGS += -O2
    BUILD_DIR := $(BUILD_DIR)/release
endif

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Link target executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install target (adjust the install path as needed)
.PHONY: install
install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/

# Debug build
.PHONY: debug
debug:
	$(MAKE) DEBUG=1

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all      - Build the STL viewer (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  debug    - Build with debug information"
	@echo "  install  - Install the STL viewer"
	@echo "  help     - Display this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build release version"
	@echo "  make debug        # Build debug version"
	@echo "  make clean        # Clean build artifacts"
	@echo "  sudo make install # Install the program"