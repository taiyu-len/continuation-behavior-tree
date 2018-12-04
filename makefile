-include targets.mk    # Userdefined Targets.
-include buildflags.mk # Userdefined Build Flags
all: $(target) compile_commands.json

SOURCES != find src     -regextype awk -regex '.*\.(c|cc|cpp)'
HEADERS != find include -regextype awk -regex '.*\.(h|hh|hpp)'
OBJECTS := $(addsuffix .o,$(basename $(SOURCES)))
DEPENDS := $(addsuffix .d,$(basename $(SOURCES)))
COMPDB  := $(addsuffix .compdb,$(SOURCES) $(HEADERS))
PATTERN := %
override LDFLAGS += -L $(realpath .)
override CPPFLAGS += -Iinclude -Isrc

.LIBPATTERNS:=
.PHONY: all clean mostlyclean
.SECONDARY: $(static:%=-l%) $(shared:%=-l%) $(shared) $(static)
.SECONDARY: $(COMPDB) $(OBJECTS)
.DEFAULT_GOAL:=all

# Generate Dependencies
ifneq ($(MAKECMDGOALS:clean%=clean),clean)
-include $(DEPENDS)
%.d: %.c;   @$(CC)  $(CPPFLAGS) $< -MM -MT $*.o -MT $@ > $@
%.d: %.cc;  @$(CXX) $(CPPFLAGS) $< -MM -MT $*.o -MT $@ > $@
%.d: %.cpp; @$(CXX) $(CPPFLAGS) $< -MM -MT $*.o -MT $@ > $@
endif

# Generate compile_commands.json
-include compdb.mk
compile_commands.json: $(COMPDB)

# Depend on buildflags
$(OBJECTS) $(COMPDB): buildflags.mk

# Build Instructions
.SECONDEXPANSION:
$(target): $$(filter src/$$@/%,$(OBJECTS))
	$(LINK.o) $^ $(LDLIBS) -o $@

test: export LD_LIBRARY_PATH=$(CURDIR)
test: $(addprefix test_, $(tested))
test_%: %
	./$* $(TEST_FLAGS)

# Shared Library Dependency chain
-l%: lib%.so;
$(shared:%=lib%.so): lib%.so: $$*;
$(shared): override CXXFLAGS += -fPIC
$(shared): $$(filter src/$$@/%,$(OBJECTS))
	$(LINK.o) -shared -Wl,-soname,lib$@.so $^ $(LDLIBS) -o lib$@.so

# Static Library Dependency chain
-l%: lib%.a;
$(static:%=lib%.a) : lib%.a : $$*;
$(static): $$(filter src/$$@/%,$(OBJECTS))
	$(AR) $(ARFLAGS) lib$@.a $?

# Profiling support.
profile-generate profile-use: % :
	@$(MAKE) clean
	@$(MAKE) all CXXFLAGS=-f$*

# static analysis
clang-check clang-tidy: % : $(SOURCES)
	@$* $^

clean-depends:
	rm -f $(DEPENDS)
clean-compdb:
	rm -f $(COMPDB)
	@rm -f compile_commands.json
clean-objects:
	rm -f $(OBJECTS)
clean: clean-depends clean-compdb clean-objects
	rm -f $(target)
	rm -f $(shared:%=lib%.so) $(static:%=lib%.a)
