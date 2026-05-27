CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -O2 -Wall

LIB := libflemme-stats.a
OBJ := Stats.o

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^
	@echo "→ $(LIB) créée"

$(OBJ): Stats.cpp Stats.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
