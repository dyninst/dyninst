INCLUDE_DEST = ../../include
CP = cp
CP_DIR = cp -r

TO_INC = h
PUBLIC_H = Annotatable.h \
           dyntypes.h \
           util.h \
           Serialization.h \
	  dyn_detail

world:
	@echo Nothing to be done for build

install: $(patsubst %,$(INCLUDE_DEST)/%,$(PUBLIC_H))

$(INCLUDE_DEST)/%.h: $(TO_INC)/%.h
	$(CP) $^ $(INCLUDE_DEST)
$(INCLUDE_DEST)/dyn_detail: $(TO_INC)/dyn_detail
	$(CP_DIR) $^ $(INCLUDE_DEST)


clean distclean:
