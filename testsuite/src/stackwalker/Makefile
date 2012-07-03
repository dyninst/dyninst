
IFLAGS += -I../../../../include
LDFLAGS += -L../../../../$(PLATFORM)/lib -lstackwalk -lsymtabAPI -lparseAPI -lcommon -lpcontrol -lpthread

stack_sampler: stack_sampler.o
	g++ $(LDFLAGS) stack_sampler.o -o stack_sampler

stack_sampler.o: stack_sampler.C
	g++ $(IFLAGS) -c -g stack_sampler.C

clean:
	rm stack_sampler stack_sampler.o
