
TO_CORE = ../../..

IFLAGS += -I $(TO_CORE)/stackwalk/h
IFLAGS += -I $(TO_CORE)/dynutil/h
IFLAGS +=  -I $(TO_CORE)/proccontrol/h

SWK_LIBDIR = $(TO_CORE)/stackwalk/$(PLATFORM)
PARSE_LIBDIR = $(TO_CORE)/parseAPI/$(PLATFORM)
COMMON_LIBDIR = $(TO_CORE)/common/$(PLATFORM)
INSTRUCTION_LIBDIR = $(TO_CORE)/instructionAPI/$(PLATFORM)
PROCCONTROL_LIBDIR = $(TO_CORE)/proccontrol/$(PLATFORM)
SYMTAB_LIBDIR = $(TO_CORE)/symtabAPI/$(PLATFORM)

LDFLAGS += -L$(SWK_LIBDIR) -Wl,-rpath,$(SWK_LIBDIR)
LDFLAGS += -lstackwalk 
LDFLAGS += -L$(PARSE_LIBDIR) -Wl,-rpath,$(PARSE_LIBDIR)
LDFLAGS += -lparseAPI
LDFLAGS += -L$(COMMON_LIBDIR) -Wl,-rpath,$(COMMON_LIBDIR)
LDFLAGS += -lcommon 
LDFLAGS += -L$(INSTRUCTION_LIBDIR) -Wl,-rpath,$(INSTRUCTION_LIBDIR)
LDFLAGS += -linstructionAPI
LDFLAGS += -L$(PROCCONTROL_LIBDIR) -Wl,-rpath,$(PROCCONTROL_LIBDIR)
LDFLAGS += -lpcontrol
LDFLAGS += -L$(SYMTAB_LIBDIR) -Wl,-rpath,$(SYMTAB_LIBDIR)
LDFLAGS += -lsymtabAPI

stack_sampler: stack_sampler.o
	g++ $(LDFLAGS) stack_sampler.o -o stack_sampler

stack_sampler.o: stack_sampler.C
	g++ $(IFLAGS) -c -g stack_sampler.C

clean:
	rm stack_sampler stack_sampler.o

test_basic: test_basic.C
	g++ test_basic.C -o test_basic $(LDFLAGS) $(IFLAGS) -DWITHOUT_SYMTAB -lpthread
