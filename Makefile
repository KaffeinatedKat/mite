SRC_ROOT := src
HEADER_ROOT := include
SRC := $(shell fd -e cpp . $(SRC_ROOT))
HEADERS := $(shell fd -e hpp . $(HEADER_ROOT))
OBJ := $(SRC:%.cpp=%.o)
WARNINGS := -Wall -Wextra -Wpedantic -Walloca -Wfloat-equal -Wlarger-than=4KiB -Wpointer-arith
OUT ?= mite
CXXFLAGS += -std=c++20 -pipe
INCLUDE := -Iinclude
LIB :=

all: $(OUT) compile_flags.txt

.DEFAULT_GOAL = debug

debug: CXX = g++
debug: CXXFLAGS += -Og -ggdb3 -DDEBUG -ltree-sitter -ltree-sitter-cpp
debug: all

analyze: CXX = g++
analyze: CXXFLAGS += -fanalyzer
analyze: debug

release: CXX = clang++
release: CXXFLAGS += -O2 -flto=thin -DNO_TS
release: clean
release: all

minimal: CXXFLAGS += -DNO_LSP -DNO_TS
minimal: release

tree-sitter: CXX = clang++
tree-sitter: CXXFLAGS += -O2 -flto=thin -ltree-sitter -ltree-sitter-cpp
tree-sitter: clean
tree-sitter: all

format: $(SRC) $(HEADERS)
	clang-format -i $(SRC) $(HEADERS)

debugger: debug
	gdb $(OUT)

$(OUT): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIB)

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $(INCLUDE) $(WARNINGS) $(CXXFLAGS) $<

compile_flags.txt: Makefile
	rm -f compile_flags.txt
	for flag in $(INCLUDE) $(WARNINGS) $(CXXFLAGS); do \
		echo $$flag >> $@; \
	done

clean:
	rm -rf $(OUT)
	rm -rf compile_flags.txt 
	rm -rf src/*.o
