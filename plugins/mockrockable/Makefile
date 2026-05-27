CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -O2 -Wall

LIB := libflemme-mockrockable.a
OBJ := MockRockable.o

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^
	@echo "→ $(LIB) créée"

$(OBJ): MockRockable.cpp MockRockable.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
