BUILD_MODE ?= DEBUG

TARGET = 441final
CC = gcc
SRC = $(wildcard src/*.c)
OBJS = #icon.o
INCLUDE_PATHS = -I"C:\mingw64\mingw64\include"
LLIB_PATHS = -L"C:\mingw64\mingw64\lib"
LLIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -lglu32 -lcglm

CFLAGS += -Wall -std=c99 -Wno-missing-braces -Wfatal-errors

ifeq ($(BUILD_MODE),RELEASE)
	CFLAGS += -s -O1 -Wl,-subsystem,windows
else
	CFLAGS += -g -O0
endif

all:
	$(info BUILDING $(BUILD_MODE)...)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) $(INCLUDE_PATHS) $(LLIB_PATHS) $(LLIBS) -o build/$(TARGET).exe

run:
	./build/$(TARGET)
