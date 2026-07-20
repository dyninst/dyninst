HIP_PATH?= $(wildcard /opt/rocm-7.0.2/hip)
ifeq (,$(HIP_PATH))
	HIP_PATH=../../..
endif

HIPCC=$(HIP_PATH)/bin/hipcc

TARGET=hcc

SOURCES = device_only.cpp #vectorsub_kernel.cpp 
OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE=device_only.exe

.PHONY: test


all: $(EXECUTABLE) #test

CXXFLAGS =-g# --offload-compress

CXX=$(HIPCC)


$(EXECUTABLE): $(OBJECTS) 
	$(HIPCC) $(OBJECTS) -o $@


test: $(EXECUTABLE)
	$(EXECUTABLE)


clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
	rm .*.swp


