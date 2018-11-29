CXXFLAGS=-std=c++17 -Wall -Wextra -fsanitize=address,undefined
CXXFLAGS=-ggdb -Og

cbt_test: cbt_test.cpp main.o
cbt_test.cpp: cbt.hpp

