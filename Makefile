CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I.
TARGET = TestRunner
SRCDIR = Tests
SOURCES = $(SRCDIR)/TestRunner.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	del $(TARGET).exe

test: $(TARGET)
	./$(TARGET)

.PHONY: clean test