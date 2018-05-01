CXX=g++
CXX_FLAGS=-Wall -I../src -ldpsaudio -L../build/lib/

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:%.cpp=%.o)

default: all

all: $(OBJS)
	@echo Use LD_LIBRARY_PATH=../build/lib/

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -o $@ $<

.PHONY: clean
clean:
	rm *.o