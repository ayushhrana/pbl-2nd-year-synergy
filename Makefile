CXX = g++
FLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
SRC = $(wildcard src/*/*.cpp)

ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32
    OUT = pbl_project.exe
else
    LIBS =
    OUT = pbl_project
endif

all: $(OUT)

$(OUT): $(SRC) src/main.cpp
	$(CXX) $(FLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f pbl_project pbl_project.exe
