# Project Name (executable)
PROJECT = gcflobdd
# Compiler
CC = g++
SRC_DIR = src
OBJ_DIR = obj

# Run Options       
COMMANDLINE_OPTIONS = #/dev/ttyS0

# Compiler options during compilation
COMPILE_OPTIONS = -O3 -std=c++2a -w

#Header include directories
HEADERS = -I$(SRC_DIR)/gcflobdd -I$(SRC_DIR)/utils/ -I$(SRC_DIR)/ops -I$(SRC_DIR)/grammar -I$(SRC_DIR)/hardware_benchmarks -I$(SRC_DIR)/visualization
#Libraries for linking
LIBS = -I/usr/include/graphviz -L/usr/lib -lgvc -lcgraph #-fsanitize=address,undefined -fno-omit-frame-pointer

# Dependency options
DEPENDENCY_OPTIONS = -MM

#-- Do not edit below this line --

# Subdirs to search for additional source files
SOURCE_FILES += $(shell ls $(SRC_DIR)/gcflobdd/*.cpp)
SOURCE_FILES += $(shell ls $(SRC_DIR)/utils/*.cpp)
SOURCE_FILES += $(shell ls $(SRC_DIR)/ops/*.cpp)
SOURCE_FILES += $(shell ls $(SRC_DIR)/grammar/*.cpp)
SOURCE_FILES += $(shell ls $(SRC_DIR)/hardware_benchmarks/*.cpp)
SOURCE_FILES += $(shell ls $(SRC_DIR)/visualization/*.cpp)
SOURCE_FILES += $(shell ls $(SRC_DIR)/*.cpp)
# SOURCE_FILES += $(shell find . -maxdepth 1 -mindepth 1 -name \*.cpp -a -not -name main.cpp)

# Create an object file of every cpp file
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCE_FILES))

# Make $(PROJECT) the default target
all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $(PROJECT) $(OBJECTS) $(LIBS) -stdlib=libc++

# Compile every cpp file to an object
# %.cpp
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $^ $(HEADERS)

# Build & Run Project
run: $(PROJECT)
	./$(PROJECT) $(COMMANDLINE_OPTIONS)

# Clean & Debug
.PHONY: makefile-debug
makefile-debug:

.PHONY: clean
clean:
	rm -f $(PROJECT) $(OBJECTS)

.PHONY: depclean
depclean:
	rm -f $(DEPENDENCIES)

clean-all: clean depclean

