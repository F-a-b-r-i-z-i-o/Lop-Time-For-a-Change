# ---- Compilers ----
CXX := g++
CC  := gcc

# ---- Flags ----
CPPFLAGS := -Iinclude -Iexternal
CXXFLAGS := -O3 -g -std=c++17 -Wall -Wextra -Wno-deprecated -ftree-vectorize
CFLAGS   := -O3 -g -Wall -Wextra
LDLIBS   := -lm

# ---- Target ----
TARGET := Solver

# ---- Sources / Objects ----
OBJS := main.o \
        src/LOP.o \
        src/PSolution.o \
        src/random.o \
        src/Tools.o \
        external/SFMT.o

# ---- Default ----
all: $(TARGET)

# ---- Link ----
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

# ---- Compile rules ----
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# ---- Explicit deps ----
main.o: main.cpp \
	include/Tools.hpp include/ParameterSetting.h \
	include/LOP.h include/PSolution.h include/random.h

src/LOP.o: src/LOP.cpp include/LOP.h include/Tools.hpp

src/Tools.o: src/Tools.cpp include/Tools.hpp include/LOP.h

src/PSolution.o: src/PSolution.cpp include/PSolution.h include/random.h src/PSolution_util.cpp

src/random.o: src/random.cpp include/random.h external/SFMT.h

external/SFMT.o: external/SFMT.c external/SFMT.h \
	external/SFMT-common.h external/SFMT-params.h external/SFMT-params19937.h

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
