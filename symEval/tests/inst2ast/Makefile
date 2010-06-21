
CC = g++ -g
DYNINST_CFLAGS = -I$(DYNINST_ROOT)/dyninst/dynutil/h -I$(DYNINST_ROOT)/include -I$(DYNINST_ROOT)/dyninst/symEval/src \
-I$(DYNINST_ROOT)/dyninst/symEval/h -I$(DYNINST_ROOT)/dyninst



LIB_FLAGS = -L$(DYNINST_ROOT)/$(PLATFORM)/lib -L$(DYNINST_ROOT)/dyninst/symEval/$(PLATFORM)

XTARGET = inst2ast

all: $(XTARGET)

$(XTARGET): $(XTARGET).o
	$(CC) $(XTARGET).o $(LIB_FLAGS) -ldyninstAPI -lsymEval -lcommon -liberty -o $(XTARGET)

$(XTARGET).o: $(XTARGET).C
	$(CC) -c $(CFLAGS) $(DYNINST_CFLAGS) $(XTARGET).C

clean: 
	rm -f $(XTARGET) $(XTARGET).o

