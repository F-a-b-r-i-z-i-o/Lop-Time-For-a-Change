CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O3

TARGET := calcolate
SRCS := calcolate.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
