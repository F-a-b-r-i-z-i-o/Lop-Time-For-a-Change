# ---- Compilers & flags ----
CXX      := g++
CC       := gcc

CXXFLAGS := -g -O3 -Wno-deprecated -ftree-vectorize -std=c++17 -Wall -Wextra \
            -Iinclude -Iexternal
CFLAGS   := -g -O3 -Wall -Wextra -Iexternal

TARGET   := Solver

SRCS_CPP := main.cpp \
            src/LOP.cpp \
            src/PSolution.cpp \
            src/random.cpp \
            src/Tools.cpp

SRCS_C   := external/SFMT.c

OBJS_CPP := $(SRCS_CPP:.cpp=.o)
OBJS_C   := $(SRCS_C:.c=.o)
OBJS     := $(OBJS_CPP) $(OBJS_C)

# ---- Default target ----
all: $(TARGET)

# ---- Link step ----
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lm

# ---- Explicit dependencies ----
src/main.o: main.cpp \
            include/Tools.hpp include/ParameterSetting.h \
            include/LOP.h include/PSolution.h include/random.h

src/LOP.o: src/LOP.cpp include/LOP.h include/Tools.hpp

src/Tools.o: src/Tools.cpp include/Tools.hpp include/LOP.h

src/PSolution.o: src/PSolution.cpp include/PSolution.h \
                 src/PSolution_util.cpp include/random.h

src/random.o: src/random.cpp include/random.h external/SFMT.h

external/SFMT.o: external/SFMT.c external/SFMT.h \
                 external/SFMT-common.h external/SFMT-params.h \
                 external/SFMT-params19937.h

# ---- Generic compilation rules ----
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---- Cleaning ----
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
