HIPCC    = /opt/rocm-7.0.2/bin/hipcc
BUNDLER  = /opt/rocm-7.0.2/lib/llvm/bin/clang-offload-bundler
OBJ2YAML = /usr/bin/obj2yaml
YAML2OBJ = /usr/bin/yaml2obj
# Match the hostcall_lib / vectoradd mutatee build contract exactly (ABI ver 4,
# flags 0xE30). Every co-loaded object must agree on arch + COV + features.
ARCH     = gfx908:sramecc+:xnack-
COV      = 6
# NOTE: we used to pass -fno-optimize-sibling-calls here. A probe like
# `return gpu_fopen(...)` tail-calls by default (s_setpc straight into the callee, no
# s_swappc), and older ParseAPI followed that jump into the callee's body and corrupted
# its CFG (segfault at parse). Upstream #2326 (AMDGPU indirect control-flow parsing) fixed
# that, so the workaround is gone: tail-called probes now parse cleanly and instrument
# end-to-end (verified: capture pw_openfile()@entry -> pw_writeln(value())@sites).
HIPFLAGS = --offload-arch=$(ARCH) -mcode-object-version=$(COV) \
           -Xoffload-linker --unresolved-symbols=ignore-all
HCDIR    = ../hostcall_lib
SRCS     = user_probe.cpp $(HCDIR)/hostcall_lib.cpp

# Final artifact dyninst links the mutatee against: the user probe AND the
# hostcall runtime in one code object, with .dyninst.* OBJECT aliases + per-
# function register-usage symbols.
all: combined.aliased.elf

# User probe + hostcall runtime -> ONE device code object. -fgpu-rdc (relocatable
# device code) makes the driver device-LINK both TUs into a single code object, so
# user_probe's call to hc_write_id is resolved internally (both end up defined).
# Without -fgpu-rdc, --genco emits one object per source and rejects a single -o.
combined.hsaco: $(SRCS) $(HCDIR)/hostcalls.h
	$(HIPCC) --genco -fgpu-rdc $(HIPFLAGS) -I$(HCDIR) $(SRCS) -o $@

# Per-function resource usage lives in the compiler assembly (.set .L<fn>.<key>).
# user_probe is NON-LEAF (it calls hc_write_id), so its usage is emitted as an
# EXPRESSION over the callee's symbols. Crucially, a STANDALONE compile of
# user_probe.cpp can't see hc_write_id and reports the worst case
# (max(N, amdgpu.max_num_vgpr)); compiling BOTH sources as ONE TU makes the callee
# visible, so the expression references hc_write_id's CONCRETE symbols
# (max(1, .Lhc_write_id.num_vgpr)). __noinline__ keeps it a real call, so the
# numbers match the -fgpu-rdc-linked object; add_object_aliases.py evaluates them.
combined.s: $(SRCS) $(HCDIR)/hostcalls.h
	cat user_probe.cpp $(HCDIR)/hostcall_lib.cpp > combined_tu.cpp
	$(HIPCC) -S --cuda-device-only $(HIPFLAGS) -I$(HCDIR) combined_tu.cpp -o $@

combined.elf: combined.hsaco
	$(BUNDLER) --type=o --targets=hipv4-amdgcn-amd-amdhsa--$(ARCH) \
	  --input=combined.hsaco --output=$@ --unbundle

combined.aliased.elf: combined.elf combined.s ../add_object_aliases.py
	python3 ../add_object_aliases.py combined.elf $@ \
	  --asm combined.s --tables symtab --obj2yaml $(OBJ2YAML) --yaml2obj $(YAML2OBJ)

clean:
	rm -f combined.hsaco combined.s combined_tu.cpp combined.elf combined.aliased.elf
