compile_commands.json:
	@echo "["       > $@
	@sed  '$$d' $^ >> $@
	@echo "  }"    >> $@
	@echo "]"      >> $@
%.compdb: %
	@echo "  {"                                            > $@
	@echo "    \"command\":\"$(COMPDB_COMMAND) -c $*\", " >> $@
	@echo "    \"directory\":\"$(CURDIR)\","              >> $@
	@echo "    \"file\":\"$*\""                           >> $@
	@echo "  },"                                          >> $@
%.c.compdb:   COMPDB_COMMAND=$(CC)  $(CFLAGS)   $(CPPFLAGS)
%.h.compdb:   COMPDB_COMMAND=$(CC)  $(CFLAGS)   $(CPPFLAGS)
%.cc.compdb:  COMPDB_COMMAND=$(CXX) $(CXXFLAGS) $(CPPFLAGS)
%.hh.compdb:  COMPDB_COMMAND=$(CXX) $(CXXFLAGS) $(CPPFLAGS)
%.cpp.compdb: COMPDB_COMMAND=$(CXX) $(CXXFLAGS) $(CPPFLAGS)
%.hpp.compdb: COMPDB_COMMAND=$(CXX) $(CXXFLAGS) $(CPPFLAGS)
