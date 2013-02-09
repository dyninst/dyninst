# pick up INCLUDE_DEST
TO_CORE = ..

include ../make.config

LOCAL_INCLUDE_DEST = $(INCLUDE_DEST)
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
		dyn_regs.h \
		stl_utils.h \
		SymReader.h \
		DynAST.h \
		ProcReader.h \
	   IBSTree.h \
      entryIDs.h \
		VariableLocation.h \
		Buffer.h

#INSTALL_TARGETS = $(pathsubst %, $(INCLUDE_DEST)/%,$(PUBLIC_H))
INSTALL_TARGETS = $(addprefix $(LOCAL_INCLUDE_DEST)/, $(PUBLIC_H))
world:
	@echo Nothing to be done for build

#install: $(patsubst %,$(INCLUDE_DEST)/%,$(PUBLIC_H))
install: $(INSTALL_TARGETS)
#	echo INSTALL_TARGETS = $(INSTALL_TARGETS);

$(LOCAL_INCLUDE_DEST)/%.h: $(TO_INC)/%.h
	$(INSTALL) $^ $@

clean distclean:
