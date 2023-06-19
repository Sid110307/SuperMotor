BIN_DIR := bin
SOURCE_DIR := src

SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECT_FILES := $(patsubst $(SOURCE_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SOURCE_FILES))

CFLAGS=-std=c++20 -pedantic -Wall -Wextra
LIBS=-lSDL2 -lSDL2_image -lSDL2_ttf

.PHONY: all clean

all: clean
	mkdir -p $(BIN_DIR)
	g++ $(CFLAGS) $(SOURCE_FILES) $(LIBS) -o $(BIN_DIR)/superMotor

clean:
	rm -rf $(BIN_DIR)
