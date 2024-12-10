# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Executable names
MAIN_EXEC = main.out
PART_EXEC = part.out
STORE_EXEC = store.out

# Source files
MAIN_SRC = main.cpp
PART_SRC = part.cpp
STORE_SRC = store.cpp
LOGGER_SRC = logger.cpp

# Object files
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
PART_OBJ = $(PART_SRC:.cpp=.o)
STORE_OBJ = $(STORE_SRC:.cpp=.o)
LOGGER_OBJ = $(LOGGER_SRC:.cpp=.o)

# Default target
all: $(MAIN_EXEC) $(PART_EXEC) $(STORE_EXEC)

# Main executable
$(MAIN_EXEC): $(MAIN_OBJ) $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Part executable
$(PART_EXEC): $(PART_OBJ) $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Store executable
$(STORE_EXEC): $(STORE_OBJ) $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run main executable
run: all
	@read input && ./$(MAIN_EXEC) $$input
	rm -f $(MAIN_OBJ) $(PART_OBJ) $(STORE_OBJ) $(LOGGER_OBJ) $(MAIN_EXEC) $(PART_EXEC) $(STORE_EXEC)
	find . -type p -delete

