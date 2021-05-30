BUILD_MODE ?= DEBUG

TARGET = 441final
CC = gcc
SRC = $(wildcard src/*.c)
OBJS := $(SRC:%.c=%.o)
# OBJS = #icon.o
INCLUDE_PATHS = -I"C:\mingw64\mingw64\include"
LLIB_PATHS = -L"C:\mingw64\mingw64\lib"
LLIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -lglu32 -lcglm

CFLAGS += -Wall -std=c99 -Wno-missing-braces -Wfatal-errors -MD

ifeq ($(BUILD_MODE),RELEASE)
	CFLAGS += -s -O1 -Wl,-subsystem,windows
else
	CFLAGS += -g -O0
endif

$(TARGET).exe : $(OBJS)
	$(info BUILDING $(BUILD_MODE)...)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(INCLUDE_PATHS) $(LLIB_PATHS) $(LLIBS)

$(OBJS): src/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE_PATHS) $(LLIB_PATHS) $(LLIBS) -o $@

run:
	./$(TARGET)

clean:
	del $(TARGET).exe src\*.o src\*.d

-include $(OBJS:.o=.d)