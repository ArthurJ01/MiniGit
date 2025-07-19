# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -I./src/headers

# Directories
BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj

# Collect all .cpp source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Generate .o file paths in OBJ_DIR
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Source and object files
OUT = $(BIN_DIR)/minigit

# Default
all: $(OUT)

$(OUT): $(OBJS) | $(BIN_DIR)
	@echo -e "\033[1;32mLinking...\033[0m"
	@echo "executable created: $@.exe"
	@$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo -e "\033[1;32mCompiling... \033[0m "
	@echo -e "$< -> $@"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Cleanup
clean:
	@echo "Cleaning up..."
	@rm -rf $(BIN_DIR) $(OBJ_DIR)
	@echo -e "\033[1;32mCleaning done. \033[0m "

.PHONY: all clean