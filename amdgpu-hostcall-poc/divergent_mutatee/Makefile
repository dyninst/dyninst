HIPCC   = /opt/rocm-7.0.2/bin/hipcc
BUNDLER = /opt/rocm-7.0.2/lib/llvm/bin/clang-offload-bundler
# EXACT same target-id + COV as vectoradd_mutatee / hostcall_lib so the code object
# ABI (ver 4, flags 0xE30) matches and cross-object linking is valid.
ARCH    = gfx908:sramecc+:xnack-
COV     = 6
HIPFLAGS = --offload-arch=$(ARCH) -mcode-object-version=$(COV)

all: divergent divergent.co

# Runnable mutatee (sanity-check the divergent kernel yields C=A+B). ROCR_VISIBLE_DEVICES=1.
divergent: divergent.cpp
	$(HIPCC) $(HIPFLAGS) $< -o $@

# Code object dyninst instruments.
divergent.co: divergent.cpp
	$(HIPCC) --genco $(HIPFLAGS) $< -o divergent_genco.hsaco
	$(BUNDLER) --type=o --targets=hipv4-amdgcn-amd-amdhsa--$(ARCH) \
	  --input=divergent_genco.hsaco --output=$@ --unbundle

run: divergent
	ROCR_VISIBLE_DEVICES=1 ./divergent

clean:
	rm -f divergent divergent.co divergent_genco.hsaco divergent.inst.co
