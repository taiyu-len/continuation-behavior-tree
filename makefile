CXXFLAGS=-std=c++17 -Wall -Wextra -fsanitize=address,undefined -Wno-comment
cbt_test: cbt_test.cpp cbt.hpp main.o

