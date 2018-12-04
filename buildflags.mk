override CXXFLAGS+=-std=c++17 -Wall -Wextra
override CPPFLAGS+=-Iinclude
LINK.o   = $(LINK.cc)
CXX=g++
DEBUG?=1


ifeq ($(DEBUG), 1)
  override CXXFLAGS += -Og -g -fsanitize=address,undefined
  override CXXFLAGS += -foptimize-sibling-calls
else
  override CXXFLAGS += -O3 # -flto
  override CPPFLAGS += -DNDEBUG # -DDOCTEST_CONFIG_DISABLE
endif

cbt: -ldoctest
