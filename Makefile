# Glyph Programming Language Interpreter Makefile
# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
DEBUGFLAGS := -std=c++17 -Wall -Wextra -pedantic -g -DDEBUG

# Directories
BUILD_DIR := build
SRC_DIR := glyph

# Target executable
TARGET := glyph
DEBUG_TARGET := glyph_debug

# Source files
SOURCES := glyph.cpp
OBJECTS := $(BUILD_DIR)/glyph.o

# Default target
.PHONY: all
all: $(TARGET)

# Build release version
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build debug version
.PHONY: debug
debug: CXXFLAGS = $(DEBUGFLAGS)
debug: $(BUILD_DIR)/glyph.o
	$(CXX) $(DEBUGFLAGS) $(BUILD_DIR)/glyph.o -o $(DEBUG_TARGET)
	@echo "Debug build complete: $(DEBUG_TARGET)"

# Run the interpreter
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Run tests
.PHONY: test
test: $(TARGET)
	@echo "Running test programs..."
	@echo "_" | ./$(TARGET)
	@echo "(+__)" | ./$(TARGET)
	@echo "(^(+__)(+__))" | ./$(TARGET)

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(DEBUG_TARGET)
	@echo "Clean complete"

# Install to system (optional - requires sudo)
.PHONY: install
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

# Uninstall from system
.PHONY: uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled from /usr/local/bin/$(TARGET)"

# Show help
.PHONY: help
help:
	@echo "Glyph Interpreter Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make          Build release version"
	@echo "  make debug    Build debug version with symbols"
	@echo "  make run      Build and run the interpreter"
	@echo "  make test     Build and run test programs"
	@echo "  make clean    Remove build artifacts"
	@echo "  make install  Install to /usr/local/bin (requires sudo)"
	@echo "  make uninstall Remove from /usr/local/bin"
	@echo "  make help     Show this help message"

# Rebuild everything
.PHONY: rebuild
rebuild: clean all