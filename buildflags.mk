CXXFLAGS:=-std=c++17 -Wall -Wextra
CPPFLAGS:=-Iinclude
LINK.o   = $(LINK.cc)
CXX=g++
DEBUG?=1

ifeq ($(DEBUG), 1)
  CXXFLAGS += -Og -g -fsanitize=address,undefined
  CXXFLAGS += -foptimize-sibling-calls
else
  CXXFLAGS += -O3 # -flto
  CPPFLAGS += -DNDEBUG # -DDOCTEST_CONFIG_DISABLE
endif

cbt: -ldoctest
