CXX = c++
# CXX = g++
# CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

SRC = 01_greeter_manual.cpp \
      02_opener_and_greeter_manual.cpp \
      03_greeter_automated.cpp \
      04_opener_and_greeter_automated.cpp

EXE = $(SRC:.cpp=)

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^


.PHONY: all
all: $(EXE)


.PHONY: clean
clean:
	rm -f $(EXE)
