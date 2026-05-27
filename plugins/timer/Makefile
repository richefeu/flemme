CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -O2 -Wall

LIB := libflemme-timer.a
OBJ := FlemmeTimer.o

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^
	@echo "→ $(LIB) créée"

$(OBJ): FlemmeTimer.cpp FlemmeTimer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
