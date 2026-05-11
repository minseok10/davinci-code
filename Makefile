CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
TARGET := davinci-code
SRCS := main.cpp menus.cpp game.cpp turn.cpp
OBJS := $(SRCS:.cpp=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
