CXXFLAGS:=-std=c++17 -Wall -Wextra
CPPFLAGS:=-Iinclude
CXX=g++
DEBUG?=1
ifeq ($(DEBUG), 1)
  CXXFLAGS += -Og -ggdb -fsanitize=address,undefined
else
  CXXFLAGS += -Os
  CPPFLAGS += -DNDEBUG # -DDOCTEST_CONFIG_DISABLE
endif

sources:=main.cpp
sources+=behavior.cpp continuation.cpp spawn.cpp
sources+=composites.cpp decorators.cpp status.cpp
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

clean: mostlyclean
	@rm -f tests $(objects) $(depends)
