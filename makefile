SDK_PATH := $(shell xcrun --sdk macosx --show-sdk-path)
CFLAGS += -isysroot $(SDK_PATH)
FRAMEWORKS = -framework Metal -framework Foundation -framework QuartzCore
TARGET = metal_compute
all:
	g++ -std=c++17 -I./metal-cpp/ metal_compute.cpp -o $(TARGET) $(CFLAGS) $(FRAMEWORKS) 
clean:
	rm $(TARGET)
