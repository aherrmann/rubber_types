CXX = c++
# CXX = g++
# CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

SRC = simple.cpp \
      merge_concepts.cpp \
      drawable.cpp

EXE = $(SRC:.cpp=)

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^


.PHONY: all
all: $(EXE)


.PHONY: clean
clean:
	rm -f $(EXE)
