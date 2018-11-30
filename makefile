CXXFLAGS=-std=c++17 -Wall -Wextra -O3
CXXFLAGS+=-fsanitize=address,undefined
CXXFLAGS+=-ggdb

cbt_test: cbt_test.o cbt.o main.o
	$(LINK.cc) $^ $(LDLIBS) -o $@

cbt.o: cbt.hpp
cbt_test.o: cbt_test.cpp cbt.hpp
