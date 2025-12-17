CXX      := g++
CXXFLAGS := -O2 -std=c++17 -Wall -Wextra -Iinclude

TARGET := main
SRCDIR := src

SRC := main.cpp $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(notdir $(SRC:.cpp=.o))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o
