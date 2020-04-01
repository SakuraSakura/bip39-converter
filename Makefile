TARGET  = seed_conv
TARGET_TEST = mytest
SRC_DIR = ./src/
TEST_DIR = ./test/
INCLUDES = -I./includes/
LIBS = -lcrypto

CXX = g++
CXXFLAGS += $(INCLUDES)
CXXFLAGS += $(LIBS)
CXXFLAGS += -std=c++11 -Wall -Wshadow -O2

SRC      = $(wildcard $(SRC_DIR)*.cpp)
SRC_TEST = $(wildcard $(TEST_DIR)*.cpp)
OBJ      = $(SRC:%.cpp=%.o)
OBJ_TEST = $(SRC_TEST:%.cpp=%.o)
OBJ_TEST += $(filter-out $(SRC_DIR)main.o, $(OBJ))

DEP      = $(SRC:%.cpp=%.d)
DEP      += $(SRC_TEST:%.cpp=%.d)

.PHONY: all main test clean

all: main test

main: $(TARGET)

test: $(TARGET_TEST)

$(TARGET): $(OBJ)
	$(CXX) $^ $(CXXFLAGS) -o $@

$(TARGET_TEST): $(OBJ_TEST)
	$(CXX) $^ $(CXXFLAGS) -o $@

-include $(DEP)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -MMD -c -o $@

clean:
	rm -f $(TARGET) $(TARGET_TEST) $(OBJ) $(DEP)
