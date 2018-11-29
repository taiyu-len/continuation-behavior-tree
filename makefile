CXXFLAGS=-std=c++17 -Wall -Wextra -fsanitize=address,undefined
CXXFLAGS+=-ggdb
cbt_test: cbt_test.cpp cbt.hpp main.o

