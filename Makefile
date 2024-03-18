
#cacheSim: cacheSim.cpp
#	g++ -o cacheSim cacheSim.cpp

#.PHONY: clean
#clean:
#	rm -f *.o
#	rm -f cacheSim



CXX = g++
CXXFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG
CXXLINK = $(CXX)
OBJS = cacheSim.o
RM = rm -f
# Creating the  executable
cacheSim: $(OBJS)
	$(CXXLINK) -o cacheSim $(OBJS)
# Creating the object files
cacheSim.o: cacheSim.cpp helper.cpp
# Cleaning old files before new make
.PHONY: clean
clean:
	rm -f *.o
	rm -f cacheSim
