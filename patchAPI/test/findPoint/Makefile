DYNINST_ROOT = /afs/cs.wisc.edu/p/paradyn/development/wenbin/dyninst
INC_DIR =  -I$(DYNINST_ROOT)/include -I$(DYNINST_ROOT)/dyninst/patchAPI/h -I$(DYNINST_ROOT)/dyninst -I$(DYNINST_ROOT)/dyninst/dyninstAPI/src -I$(DYNINST_ROOT)/dyninst/patchAPI/src -I$(DYNINST_ROOT)/dyninst/patchAPI/src/dyninst -I$(DYNINST_ROOT)/dyninst/dyninstAPI/src/Relocation

LIB_DIR = -L$(DYNINST_ROOT)/$(PLATFORM)/lib 
LIB     = -ldyninstAPI -lsymtabAPI -linstructionAPI -lcommon -lpcontrol -lparseAPI -lpatchAPI
CC  = g++ 
cc  = gcc
CXXFLAG = -Wall -g -Dos_linux -Darch_x86_64 -Darch_64bit -Dx86_64_unknown_linux2_4  -Dcap_ptrace -Dcap_stripped_binaries -Dcap_async_events -Dcap_threads -Dcap_dynamic_heap -Dcap_relocation -Dcap_dwarf -Dcap_32_64 -Dcap_liveness -Dcap_fixpoint_gen -Dcap_noaddr_gen -Dcap_mutatee_traps -Dcap_binary_rewriter -Dcap_registers -Dcap_instruction_api -Dcap_instruction_replacement -Dcap_tramp_liveness -Dbug_syscall_changepc_rewind -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
CFLAG = -Wall -fPIC

PLATFORM = x86_64-unknown-linux2.4

#VPATH += $(DYNINST_ROOT)/dyninst/patchAPI/h
VPATH += $(DYNINST_ROOT)/dyninst/common/h
VPATH += $(DYNINST_ROOT)/dyninst/common
VPATH += $(DYNINST_ROOT)/dyninst/

CA_DEP = main.C

all: test.exe 

test.exe: main.C $(DYNINST_ROOT)/$(PLATFORM)/lib/libdyninstAPI.so
	$(CC) -o $@ $(LIB_DIR) $(INC_DIR) $(CXXFLAG)  $< $(LIB)

clean:
	rm -f test.exe
