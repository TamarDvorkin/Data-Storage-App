# CMD args
ARGS= 
MODE=GD
#NAME=beesnas
NAME = refw
# -Werror: Turn any warning into a compilation error.
# -pedantic: Turns on lots of compiler warning flags, specifically (-Waddress, -Wcomment, -Wformat, -Wbool-compare, -Wuninitialized, -Wunknown-pragmas, -Wunused-value, -Wunused-value …)
# -Wextra: Enables extra flags not enabled by -Wall, such as -Wsign-compare (C only), -Wtype-limits, -Wuninitialized …
# -pedantic: Issue all warning required by ISO C and ISO C++ standard, it issues warning whenever there are compiler extensions non compliant to ISO C or C++ standard.
# -pthread:- Compile against Posix threads shared library (Unix Dependency)

# Compilation
CC = g++
#CFLAGS = -std=c++14 -Werror -pedantic-errors -Wall -Wextra -pthread 
#CFLAGS - to make comment to error in compilation
CFLAGS_DEBUG = $(CFLAGS) -g
CFLAGS_RELEASE = $(CFLAGS) -DNDEBUG -O3
PIC_FLAGS = -fPIC
DEPFLAGS = -MMD -MP  
IPATH = -Iinclude
OS_FLAG = -shared
LIB_INCLUTION_FLAG = -Wl,-rpath=
LINKER_FLAG = -ldl -rdynamic

# Directories
SRC_DIR := ./src
OBJ_DIR := ./.object
TEST_DIR := ./test
BIN_DIR := ./bin
INC_DIR := ./include
DEBUG_DIR := $(OBJ_DIR)/debug
RELEASE_DIR := $(OBJ_DIR)/release
BIN_DEBUG_DIR := ./bin/debug
BIN_RELEASE_DIR := ./bin/release

# Files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES := $(wildcard $(TEST_DIR)/*_test.cpp)
TEST_BY_NAME := $(TEST_DIR)/$(NAME)_test.cpp
LIB_DEBUG := lib_debug_beesnas.so
LIB_RELEASE := lib_release__beesnas.so

# Targets
LIB_DEBUG_NAME := $(DEBUG_DIR)/$(LIB_DEBUG)
LIB_RELEASE_NAME := $(RELEASE_DIR)/$(LIB_RELEASE)
OBJ_FILES_DEBUG := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(SRC_FILES))
OBJ_FILES_RELEASE := $(patsubst $(SRC_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(SRC_FILES))
DEPS_FILES_DEBUG := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_DIR)/%.d, $(SRC_FILES))
DEPS_FILES_RELEASE := $(patsubst $(SRC_DIR)/%.cpp, $(RELEASE_DIR)/%.d, $(SRC_FILES))
BINARY := BeesNas

.PHONY :all
all: $(TEST_BY_NAME) $(LIB_DEBUG_NAME) $(LIB_RELEASE_NAME)
ifeq ($(MODE), GD)
	mkdir -p  $(BIN_DEBUG_DIR)
	$(CC) $(LIB_INCLUTION_FLAG)$(DEBUG_DIR) $(CFLAGS_DEBUG) -o $(BIN_DEBUG_DIR)/$(BINARY) $(TEST_BY_NAME) $(LIB_DEBUG_NAME) $(IPATH) $(LINKER_FLAG)
else
	mkdir -p  $(BIN_RELEASE_DIR)
	$(CC) $(LIB_INCLUTION_FLAG)$(RELEASE_DIR) $(CFLAGS_RELEASE) -o $(BIN_DEBUG_DIR)/$(BINARY) $(TEST_BY_NAME) $(LIB_RELEASE_NAME) $(IPATH) $(LINKER_FLAG)
endif

#-----------------------------------------------------
.PHONY :debug
debug:$(LIB_DEBUG_NAME)

$(LIB_DEBUG_NAME): $(OBJ_FILES_DEBUG)
	$(CC) $(OS_FLAG) -o $@ $(OBJ_FILES_DEBUG)

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p  $(DEBUG_DIR)
	$(CC) -o $@ -c $< $(CFLAGS_DEBUG) $(PIC_FLAGS) $(DEPFLAGS) $(IPATH) 

#-----------------------------------------------------
.PHONY :release
release: $(LIB_RELEASE_NAME)

$(LIB_RELEASE_NAME): $(OBJ_FILES_RELEASE)
	$(CC) $(OS_FLAG) -o $@ $(OBJ_FILES_RELEASE)

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p  $(RELEASE_DIR)
	$(CC) -o $@ -c $< $(CFLAGS_RELEASE) $(PIC_FLAGS) $(DEPFLAGS) $(IPATH) 

#-----------------------------------------------------
.PHONY :clean
clean:
	rm ./log/beeznas.log

-include $(DEPS_FILES_DEBUG)
-include $(DEPS_FILES_DEBUG)













