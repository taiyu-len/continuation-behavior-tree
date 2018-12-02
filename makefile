CXXFLAGS:=-std=c++17 -Wall -Wextra
CPPFLAGS:=-Iinclude
CXX=g++
DEBUG?=1
ifeq ($(DEBUG), 1)
  CXXFLAGS += -Og -ggdb -fsanitize=address,undefined
  CXXFLAGS += -foptimize-sibling-calls
else
  CXXFLAGS += -O3 -flto
  CPPFLAGS += -DNDEBUG # -DDOCTEST_CONFIG_DISABLE
endif

# Test objects
sources:=main.cpp tailcall_test.cpp
# core objects
sources+=behavior.cpp continuation.cpp status.cpp
# node objects
sources+=composites.cpp decorators.cpp spawn.cpp
sources:=$(addprefix src/cbt/,$(sources))
objects:=$(addsuffix .o,$(basename $(sources)))
depends:=$(addsuffix .d,$(basename $(sources)))

tests: $(objects)
	$(LINK.cc) $^ $(LDLIBS) -o $@
test: tests
	./$^

ifeq ($(MAKECMDGOALS),clean)
else
-include $(depends)
%.d: %.cpp; @$(CXX) $(CPPFLAGS) $< -MM -MT $*.o -MT $@ > $@
endif

clean:
	@rm -f tests $(objects) $(depends)
