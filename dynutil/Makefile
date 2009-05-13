# pick up INCLUDE_DEST
include ../make.config


CP = cp
CP_DIR = cp -r

TO_INC = h
PUBLIC_H = Annotatable.h \
           dyntypes.h \
           util.h \
           Serialization.h \
	   Graph.h \
           Edge.h \
           Node.h \
	   dyn_detail 

world:
	@echo Nothing to be done for build

install: $(patsubst %,$(INCLUDE_DEST)/%,$(PUBLIC_H))

$(INCLUDE_DEST)/%.h: $(TO_INC)/%.h
	mkdir -p $(INCLUDE_DEST)
	$(CP) $^ $@
$(INCLUDE_DEST)/dyn_detail: $(TO_INC)/dyn_detail
	mkdir -p $(INCLUDE_DEST)
	$(CP_DIR) $^ $@


clean distclean:
