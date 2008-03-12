INCLUDE_DEST = ../../include
CP = cp

TO_INC = h
PUBLIC_H = Annotatable.h \
           dyntypes.h \
           util.h

world:
	@echo Nothing to be done for build

install: $(patsubst %,$(INCLUDE_DEST)/%,$(PUBLIC_H))

$(INCLUDE_DEST)/%.h: $(TO_INC)/%.h
	$(CP) $^ $(INCLUDE_DEST)


clean distclean:
