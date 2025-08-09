# Compiler
CXX := gcc

# Compiler flags
CXXFLAGS := -Os -s -DNDEBUG -ffunction-sections -fdata-sections -flto 
СXXFLAGS += -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables
CXXFLAGS += -Wl,--gc-sections -Wl,--strip-all -Wall -Wextra -std=c++23

# Linker flags (includes -lstdc++ which you might need)
LDFLAGS := -lstdc++ -lm -lpthread

# Source files (change this to match your source files)
SRCS := main.cpp

# Object files
OBJS := $(SRCS:.cpp=.o)

# Executable name
TARGET := hssc

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: all
	sudo cp $(TARGET) /usr/bin
	hssc

uninstall:
	sudo rm -f /usr/bin/$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
