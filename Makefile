# Customizable options
CC=clang++

CPP_FLAGS := -std=c++17 -stdlib=libc++ -g
EXEC_NAME := SourDo

BUILD_PATH := bin
OBJ_PATH := bin-int
PCH_HEADER := src/sdpch.h

# End of customizable options
CPP_SOURCE_FILES := $(shell find src -name *.cpp)
CPP_HEADER_FILES := $(shell find src -name *.h)
CPP_OBJECT_FILES := $(patsubst src/%.cpp, $(OBJ_PATH)/obj/%.o, $(CPP_SOURCE_FILES))
PCH_OUTPUT := $(patsubst src/%.h, $(OBJ_PATH)/pch/%.pch, $(PCH_HEADER))

$(PCH_OUTPUT): $(OBJ_PATH)/pch/%.pch : $(PCH_HEADER)
	mkdir -p $(dir $@) && \
	$(CC) $(CPP_FLAGS) $(patsubst $(OBJ_PATH)/pch/%.pch, src/%.h, $@) -emit-pch -o $@

$(CPP_OBJECT_FILES): $(OBJ_PATH)/obj/%.o : src/%.cpp $(PCH_OUTPUT) $(CPP_HEADER_FILES)
	mkdir -p $(dir $@) && \
	$(CC) $(CPP_FLAGS) -include-pch $(PCH_OUTPUT) -c $(patsubst $(OBJ_PATH)/obj/%.o, src/%.cpp, $@) -o $@

.PHONY: build_files clean
build_files: $(CPP_OBJECT_FILES)
	mkdir -p $(BUILD_PATH) && \
	$(CC) $(CPP_FLAGS) -o $(BUILD_PATH)/$(EXEC_NAME) $(CPP_OBJECT_FILES)

clean:
	rm -r $(OBJ_PATH)/obj && \
	rm -r $(BUILD_PATH)
