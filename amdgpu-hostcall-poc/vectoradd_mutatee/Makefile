HIPCC   = /opt/rocm-7.0.2/bin/hipcc
BUNDLER = /opt/rocm-7.0.2/lib/llvm/bin/clang-offload-bundler
# EXPLICIT full target-id (not generic gfx908 — that drifts) + COV6.
# Both the runnable exe and the genco code object => ABI ver 4, flags 0xE30,
# identical to hostcall_lib so cross-object linking is valid.
ARCH    = gfx908:sramecc+:xnack-
COV     = 6
HIPFLAGS = --offload-arch=$(ARCH) -mcode-object-version=$(COV)

all: vectoradd vectoradd.co

# Runnable mutatee (for sanity-checking the kernel). Run with ROCR_VISIBLE_DEVICES=1
# so the MI100 is the only visible agent (device 0 here is gfx900).
vectoradd: vectoradd.cpp
	$(HIPCC) $(HIPFLAGS) $< -o $@

# Device code object dyninst operates on — built via --genco (same path as
# hostcall_lib) so features match. --genco emits a bundle; unbundle to raw ELF.
vectoradd.co: vectoradd.cpp
	$(HIPCC) --genco $(HIPFLAGS) $< -o vectoradd_genco.hsaco
	$(BUNDLER) --type=o --targets=hipv4-amdgcn-amd-amdhsa--$(ARCH) \
	  --input=vectoradd_genco.hsaco --output=$@ --unbundle

run: vectoradd
	ROCR_VISIBLE_DEVICES=1 ./vectoradd

clean:
	rm -f vectoradd vectoradd.co vectoradd_genco.hsaco fatbin.bin vadd_* vectoradd_defcov
