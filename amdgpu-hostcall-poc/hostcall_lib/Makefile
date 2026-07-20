HIPCC   = /opt/rocm-7.0.2/bin/hipcc
BUNDLER = /opt/rocm-7.0.2/lib/llvm/bin/clang-offload-bundler
OBJ2YAML = /usr/bin/obj2yaml
YAML2OBJ = /usr/bin/yaml2obj
# Explicit full target-id + COV6 => ABI ver 4, flags 0xE30 (identical to the
# vectoradd mutatee). Must match any mutatee this library is linked into.
ARCH    = gfx908:sramecc+:xnack-
COV     = 6
HIPFLAGS = --offload-arch=$(ARCH) -mcode-object-version=$(COV) \
           -Xoffload-linker --unresolved-symbols=ignore-all

# Final artifact dyninst links against: raw code object + .dyninst.* OBJECT
# aliases (for cross-object call resolution) + .dyninst.<fn>.<key> ABS symbols
# (per-function VGPR/SGPR/scratch usage, for bumping the caller kernel descriptor).
all: hostcall_lib.aliased.elf

# Standalone device code object (mailbox stays an undefined extern).
hostcall_lib.hsaco: hostcall_lib.cpp hostcalls.h
	$(HIPCC) --genco $(HIPFLAGS) hostcall_lib.cpp -o $@

# Assembly carries the compiler's post-regalloc per-function resource usage
# (.set .L<fn>.num_vgpr, ...) that add_object_aliases.py re-exports as symbols.
hostcall_lib.s: hostcall_lib.cpp hostcalls.h
	$(HIPCC) -S --cuda-device-only $(HIPFLAGS) hostcall_lib.cpp -o $@

# Unbundle the genco output to a raw ELF code object.
hostcall_lib.elf: hostcall_lib.hsaco
	$(BUNDLER) --type=o --targets=hipv4-amdgcn-amd-amdhsa--$(ARCH) \
	  --input=hostcall_lib.hsaco --output=$@ --unbundle

# Inject OBJECT aliases + register-usage symbols; e_flags preserved by the script.
hostcall_lib.aliased.elf: hostcall_lib.elf hostcall_lib.s ../add_object_aliases.py
	python3 ../add_object_aliases.py hostcall_lib.elf $@ \
	  --asm hostcall_lib.s --tables symtab --obj2yaml $(OBJ2YAML) --yaml2obj $(YAML2OBJ)

clean:
	rm -f hostcall_lib.hsaco hostcall_lib.s hostcall_lib.elf hostcall_lib.aliased.elf
