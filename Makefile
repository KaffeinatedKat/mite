SRC_ROOT := src
HEADER_ROOT := include
SRC := $(shell fd -e cpp . $(SRC_ROOT))
HEADERS := $(shell fd -e hpp . $(HEADER_ROOT))
OBJ := $(SRC:%.cpp=%.o)
WARNINGS := -Wall -Wextra -Wpedantic -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn -Wsuggest-attribute=cold -Walloca -Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wlarger-than=4KiB -Wpointer-arith
OUT ?= mite
CXXFLAGS ?= -std=c++11 -pipe
INCLUDE := -Iinclude
LIB :=

all: $(OUT) compile_flags.txt

.DEFAULT_GOAL = debug

debug: CXX = g++
debug: CXXFLAGS += -Og -ggdb3
debug: all

analyze: CXX = g++
analyze: CXXFLAGS += -fanalyzer
analyze: debug

release: CXX = clang++
release: CXXFLAGS += -O2 -flto=thin
release: all

minimal: CXXFLAGS += -DNO_LSP
minimal: release

format: $(SRC) $(HEADERS)
	clang-format -i $(SRC) $(HEADERS)

debugger: debug
	gdb $(OUT)

$(OUT): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIB)

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $(INCLUDE) $(CXXFLAGS) $<

compile_flags.txt: Makefile
	rm -f compile_flags.txt
	for flag in $(WARNINGS) $(CXXFLAGS) $(INCLUDE); do \
		echo $$flag >> $@; \
	done

clean:
	rm -rf $(OUT)
	rm -rf compile_flags.txt 
	rm -rf src/*.o
