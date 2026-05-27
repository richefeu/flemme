CXX      := g++
AR       := ar
CXXFLAGS := -std=c++17 -O2 -Wall

LIB := libflemme-plotsvg.a
OBJ := PlotSVG.o

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^
	@echo "→ $(LIB) créée"

$(OBJ): PlotSVG.cpp PlotSVG.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
