override CXXFLAGS+=-std=c++17
LINK.o   = $(LINK.cc)
CXX=g++

DEBUG?=1
ifeq ($(DEBUG), 1)
  override CXXFLAGS += -Og -g -fsanitize=address,undefined
else
  override CXXFLAGS += -O3 # -flto
  override CPPFLAGS += -DNDEBUG # -DDOCTEST_CONFIG_DISABLE
endif

cbt: -ldoctest
