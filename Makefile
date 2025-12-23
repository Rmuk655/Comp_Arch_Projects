# Native compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
TARGET = riscv_sim.exe

# All source files
SRC_FILES = $(wildcard *.cpp)

# Native build source: exclude wasm_bindings.cpp
NATIVE_SRC = $(filter-out Wasm_bindings.cpp, $(SRC_FILES))
NATIVE_OBJ = $(patsubst %.cpp,%.o,$(NATIVE_SRC))

# WebAssembly build source: exclude main.cpp
WEB_SRC = $(filter-out main.cpp TestRunner.cpp, $(SRC_FILES))

# Default target - native build
build: clean $(TARGET)

# Link native executable
$(TARGET): $(NATIVE_OBJ)
	echo "Linking native executable: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile native object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
ifeq ($(OS),Windows_NT)
	powershell -Command "Remove-Item *.o, *.exe, *.wasm -Force -ErrorAction SilentlyContinue"
else
	rm -f *.o *.exe *.wasm
endif

# Native test and helpers
test: $(TARGET)
	./$(TARGET) -mode RV64IM -t

assemble: $(TARGET)
	./$(TARGET) -mode RV64IM -a Test/complexMul.s

deassemble: $(TARGET)
	./$(TARGET) -mode RB64IM -d Test/complexMul.b

simulate: $(TARGET)
	./$(TARGET) -mode RV64IM -s

# WebAssembly build settings
WEB_TARGET = riscv_web.js
EMXX = em++
EMXXFLAGS = -std=c++17 -O2 \
	-s ASSERTIONS=1 \
	-s WASM=1 \
	-s MODULARIZE=1 \
	-s EXPORT_NAME="createModule" \
	-s EXPORT_ES6=1 \
	-s ENVIRONMENT='web' \
	-s NO_EXIT_RUNTIME=1 \
	--no-entry \
	-s "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS']" \
	-s FORCE_FILESYSTEM=1 \
	--bind

websim:
	@echo "Compiling to WebAssembly: $(WEB_TARGET)"
	$(EMXX) $(EMXXFLAGS) $(WEB_SRC) -o $(WEB_TARGET)

serve:
	python3 serve_wasm.py


.PHONY: build clean test assemble deassemble simulate websim serve