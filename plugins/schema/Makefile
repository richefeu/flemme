CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -O2 -Wall

LIB := libflemme-schema.a
OBJ := Schema.o

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^
	@echo "→ $(LIB) créée"

$(OBJ): Schema.cpp Schema.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
