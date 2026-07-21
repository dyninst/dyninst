#define _GNU_SOURCE
// hostcall_preload.cpp — LD_PRELOAD replacement for the dedicated launcher.
//
// Instead of a standalone process that hand-loads code objects, this shim injects
// itself into a REAL HIP application and hooks the HSA loader path that HIP uses
// internally. On ROCm 7.0.2 HIP loads each device code object as:
//     hsa_executable_create_alt
//     hsa_code_object_reader_create_from_memory(data, size)   <- the co bytes
//     hsa_executable_load_agent_code_object(exe, agent, reader)
//     hsa_executable_freeze(exe)
// (one executable PER code object; verified with probe/loadtrace.so).
//
// We interpose those entry points and, for the target code object:
//   1. from_memory: substitute the app's co bytes with a pre-instrumented co
//      (offline Dyninst rewrite: hc_open@entry / hc_write@sites / hc_close@exit).
//   2. load_agent_code_object: augment the SAME executable once — define the shared
//      `mailbox` global and load the hostcall library into it — so freeze resolves
//      the inserted .dyninst.hc_* relocations + the mailbox extern (cross-object link).
//   3. a CPU service thread (started lazily) answers the GPU->CPU hostcalls.
//
// This is the launcher's load+link+service flow, driven by HIP's own load sequence,
// so ordinary kernel launches (any/all kernels) run instrumented transparently.
//
// v1 scope: OFFLINE-instrumented co (env-provided), MAILBOX-ONLY instrumentation
// (no inserted-call args, no kernarg growth — HIP sizes kernarg from the KD and will
// not fill an appended per-wave slot; that path needs a global-define and is v2).
//
// Config (env):
//   HOSTCALL_ORIG_CO   path to the ORIGINAL app code object to match (content+size)
//   HOSTCALL_INST_CO   path to the pre-instrumented replacement code object
//   HOSTCALL_LIB       path to the hostcall library (…aliased.elf)
//   HOSTCALL_VERBOSE   if set, log every hook
//
// Build: see launcher/Makefile target `hostcall_preload.so`.

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include <dlfcn.h>
#include <atomic>
#include <map>
#include <set>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>

#include "hostcalls.h"   // shared mailbox ABI

// ------------------------------------------------------------------ real syms
#define BIND(name) \
    static decltype(&name) real_##name = (decltype(&name))dlsym(RTLD_NEXT, #name)

static bool g_verbose = getenv("HOSTCALL_VERBOSE") != nullptr;
#define LOG(...) do { if (g_verbose) fprintf(stderr, __VA_ARGS__); } while (0)

// ------------------------------------------------------------------ globals
static std::mutex               g_mtx;
static std::set<uint64_t>       g_inst_readers;    // reader handles we substituted
static std::set<uint64_t>       g_augmented_exes;  // executables already given lib+mailbox
static HostcallMailbox*         g_mbox = nullptr;   // one shared mailbox (host-coherent)
static void*                    g_pw_buf = nullptr; // global per-wave buffer (defined as g_pw_base)
static size_t                   g_pw_bytes = 0;     // its size (HOSTCALL_PW_BYTES; 0 => disabled)
static std::thread              g_svc;
static std::atomic<bool>        g_run{true};
static std::atomic<bool>        g_svc_started{false};
// One code-object substitution: match the app's original co, swap in the instrumented
// bundle. A manifest holds several so a multi-code-object app (exe + HIP .so's, each its
// own fatbin/executable) is fully covered.
struct Sub { std::vector<uint8_t> orig, inst, bundle; std::string tag; };

// These are accessed from __hipRegisterFatBinary, which a HIP shared library fires
// during DSO init — possibly BEFORE this .so's own static constructors run. A file-scope
// `static std::vector` would be populated by the early call and then RE-CONSTRUCTED
// (emptied) when its own ctor finally runs (static-init-order fiasco, observed: 2 subs on
// the first registration, 0 on the second). Function-local statics construct on first use
// and are never reset, so they survive the early call.
static std::vector<Sub>&     g_subs()   { static std::vector<Sub> v; return v; }
static std::vector<uint8_t>& g_lib_co() { static std::vector<uint8_t> v; return v; }

// HIP's fat-binary wrapper: __hipRegisterFatBinary(data) receives a pointer to this;
// `binary` points at a __CLANG_OFFLOAD_BUNDLE__ that HIP parses for kernel metadata
// (incl. private_segment_size) AND loads via HSA. We swap `binary` for our bundle so
// HIP's cached metadata matches the instrumented KD (the note-synced co).
struct FatBinaryWrapper { unsigned magic; unsigned version; void* binary; void* dummy; };

// ------------------------------------------------------------------ file slurp
static bool slurp(const char* path, std::vector<uint8_t>& out) {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "[preload] cannot open %s: %s\n", path, strerror(errno)); return false; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    out.resize(n);
    size_t got = fread(out.data(), 1, n, f);
    fclose(f);
    if ((long)got != n) { fprintf(stderr, "[preload] short read %s\n", path); return false; }
    return true;
}

static bool add_sub(const char* orig, const char* inst, const char* bundle) {
    Sub s; s.tag = orig;
    if (!slurp(orig, s.orig) || !slurp(inst, s.inst) || !slurp(bundle, s.bundle)) return false;
    fprintf(stderr, "[preload] sub[%zu]: %s (%zu B) -> inst %zu B / bundle %zu B\n",
            g_subs().size(), orig, s.orig.size(), s.inst.size(), s.bundle.size());
    g_subs().push_back(std::move(s));
    return true;
}
static bool load_config() {
    const char* lib = getenv("HOSTCALL_LIB");
    if (!lib || !slurp(lib, g_lib_co())) {
        fprintf(stderr, "[preload] set HOSTCALL_LIB\n"); return false;
    }
    // Multi-code-object: HOSTCALL_MANIFEST is a file of "orig_co inst_co bundle" lines.
    if (const char* mf = getenv("HOSTCALL_MANIFEST")) {
        FILE* f = fopen(mf, "r"); if (!f) { perror(mf); return false; }
        char a[1024], b[1024], c[1024];
        while (fscanf(f, "%1023s %1023s %1023s", a, b, c) == 3) add_sub(a, b, c);
        fclose(f);
    } else {
        const char* orig = getenv("HOSTCALL_ORIG_CO");
        const char* inst = getenv("HOSTCALL_INST_CO");
        const char* bundle = getenv("HOSTCALL_BUNDLE");
        if (!orig || !inst || !bundle) {
            fprintf(stderr, "[preload] set HOSTCALL_ORIG_CO/INST_CO/BUNDLE (or HOSTCALL_MANIFEST)\n");
            return false;
        }
        add_sub(orig, inst, bundle);
    }
    fprintf(stderr, "[preload] %zu substitution(s); lib=%s (%zu B)\n", g_subs().size(), lib, g_lib_co().size());
    return !g_subs().empty();
}
// LAZY, once: a HIP shared library's __hipRegisterFatBinary can fire before this .so's
// static initializers run (init order across DSOs is not guaranteed), so we must NOT
// gate on a `static bool = load_config()` — that leaves g_cfg_ok=false for the earliest
// registration (the .so's), silently skipping its substitution. Initialize on first use.
static std::once_flag g_cfg_once;
static bool           g_cfg_ok_v = false;
static bool cfg() { std::call_once(g_cfg_once, []{ g_cfg_ok_v = load_config(); }); return g_cfg_ok_v; }

// ------------------------------------------------------------------ bundle parse
// Locate the AMDGPU device code object inside a __CLANG_OFFLOAD_BUNDLE__: returns its
// (ptr,size) and the total bundle byte span. Format: magic[24], u64 count, then per
// entry { u64 offset, u64 size, u64 triple_len, char[triple_len] }, then payloads.
static bool bundle_device_co(const void* b, const uint8_t*& co, uint64_t& co_sz, uint64_t& total) {
    const uint8_t* p = (const uint8_t*)b;
    if (memcmp(p, "__CLANG_OFFLOAD_BUNDLE__", 24) != 0) return false;
    uint64_t n; memcpy(&n, p + 24, 8);
    const uint8_t* e = p + 32;
    co = nullptr; total = 0;
    for (uint64_t i = 0; i < n; i++) {
        uint64_t off, sz, tl; memcpy(&off,e,8); memcpy(&sz,e+8,8); memcpy(&tl,e+16,8);
        const char* triple = (const char*)(e + 24);
        if (off + sz > total) total = off + sz;
        if (tl >= 6 && memmem(triple, tl, "amdgcn", 6)) { co = p + off; co_sz = sz; }
        e += 24 + tl;
    }
    return co != nullptr;
}

// ------------------------------------------------------------------ HSA discovery
struct AgentSearch { hsa_agent_t agent; bool found; };
static hsa_status_t find_cpu(hsa_agent_t a, void* d) {
    hsa_device_type_t t; hsa_agent_get_info(a, HSA_AGENT_INFO_DEVICE, &t);
    if (t == HSA_DEVICE_TYPE_CPU) { auto* s=(AgentSearch*)d; s->agent=a; s->found=true; return HSA_STATUS_INFO_BREAK; }
    return HSA_STATUS_SUCCESS;
}
struct PoolSearch { hsa_amd_memory_pool_t pool; bool found; };
static hsa_status_t find_fine_grained(hsa_amd_memory_pool_t pool, void* d) {
    hsa_amd_segment_t seg;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &seg);
    if (seg != HSA_AMD_SEGMENT_GLOBAL) return HSA_STATUS_SUCCESS;
    uint32_t flags;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_FINE_GRAINED) {
        auto* s=(PoolSearch*)d; s->pool=pool; s->found=true; return HSA_STATUS_INFO_BREAK;
    }
    return HSA_STATUS_SUCCESS;
}

// ------------------------------------------------------------------ service loop
// Scan the ring for request-ready slots and service them OUT OF ORDER (any slot, any
// order) — this is what lets a resident wave make progress regardless of FIFO order,
// breaking the single-cell ticket-lock deadlock at high wave counts.
static void service_one(HostcallSlot* s, std::map<std::string, FILE*>& files,
                        FILE*& primary, long& opens, long& writes, long& closes) {
    switch (s->opcode) {
    case HC_OP_FOPEN: {
        std::string name(s->path);
        FILE*& f = files[name];
        if (!f) { f = fopen(s->path, s->mode); if (!primary) primary = f;
                  LOG("[preload] fopen('%s','%s') -> %p\n", s->path, s->mode, (void*)f); }
        opens++; s->handle=(int64_t)(uintptr_t)f; s->retval = f?0:-1; break; }
    case HC_OP_FWRITE: {
        FILE* f=(FILE*)(uintptr_t)s->handle; int n=f?(int)fwrite(s->data,1,s->size,f):-1;
        if (f) fflush(f); writes++; s->retval=n; break; }
    case HC_OP_FREAD: {
        FILE* f=(FILE*)(uintptr_t)s->handle; s->retval=f?(int)fread(s->data,1,s->size,f):-1; break; }
    case HC_OP_FCLOSE: {
        FILE* f=(FILE*)(uintptr_t)s->handle; if (f) fflush(f); else if (primary) fflush(primary);
        closes++; s->retval=0; break; }
    case HC_OP_WRITE_ID:
        if (primary) { fprintf(primary,"[gpu] site %d\n", s->arg); fflush(primary); }
        writes++; s->retval=0; break;
    default: s->retval=-1; break;
    }
}
static void service_loop(HostcallQueue* q) {
    std::map<std::string, FILE*> files;
    FILE* primary = nullptr;
    long opens=0, writes=0, closes=0;
    while (g_run.load(std::memory_order_acquire)) {
        bool any = false;
        for (uint32_t i = 0; i < HC_NSLOTS; i++) {
            HostcallSlot* s = &q->slots[i];
            if (__atomic_load_n(&s->status, __ATOMIC_ACQUIRE) != 1) continue;
            any = true;
            service_one(s, files, primary, opens, writes, closes);
            __atomic_thread_fence(__ATOMIC_RELEASE);
            __atomic_store_n(&s->status, 2, __ATOMIC_RELEASE);   // done -> release the wave
        }
        if (!any) std::this_thread::yield();
    }
    for (auto& kv : files) if (kv.second) fclose(kv.second);
    fprintf(stderr, "[preload] serviced %ld fopen / %ld fwrite / %ld fclose (%zu files)\n",
            opens, writes, closes, files.size());
}

// ------------------------------------------------------------------ lazy init
// Allocate the shared mailbox in host-coherent fine-grained memory, grant the GPU
// agent access, and start the service thread. Done once, on first instrumented load.
static bool ensure_runtime(hsa_agent_t gpu) {
    if (g_svc_started.load()) return g_mbox != nullptr;

    AgentSearch cpu{}; hsa_iterate_agents(find_cpu, &cpu);
    if (!cpu.found) { fprintf(stderr, "[preload] no CPU agent\n"); return false; }
    PoolSearch fg{}; hsa_amd_agent_iterate_memory_pools(cpu.agent, find_fine_grained, &fg);
    if (!fg.found) { fprintf(stderr, "[preload] no fine-grained pool\n"); return false; }

    if (hsa_amd_memory_pool_allocate(fg.pool, sizeof(HostcallMailbox), 0, (void**)&g_mbox)
            != HSA_STATUS_SUCCESS) { fprintf(stderr, "[preload] mailbox alloc failed\n"); return false; }
    hsa_amd_agents_allow_access(1, &gpu, nullptr, g_mbox);
    memset(g_mbox, 0, sizeof(*g_mbox));
    for (uint32_t i = 0; i < HC_NSLOTS; i++) g_mbox->slots[i].turn = i;  // ring generation gates

    // Optional global per-wave buffer (the LD_PRELOAD substitute for the kernarg-based
    // dyninst per-wave variable): allocate once, GPU-accessible, defined as `g_pw_base`.
    if (const char* e = getenv("HOSTCALL_PW_BYTES")) {
        g_pw_bytes = strtoul(e, nullptr, 0);
        if (g_pw_bytes) {
            if (hsa_amd_memory_pool_allocate(fg.pool, g_pw_bytes, 0, &g_pw_buf) != HSA_STATUS_SUCCESS) {
                fprintf(stderr, "[preload] per-wave buffer alloc failed\n"); g_pw_buf = nullptr;
            } else {
                hsa_amd_agents_allow_access(1, &gpu, nullptr, g_pw_buf);
                memset(g_pw_buf, 0, g_pw_bytes);
                fprintf(stderr, "[preload] per-wave buffer g_pw_base @ %p (%zu B)\n", g_pw_buf, g_pw_bytes);
            }
        }
    }

    g_run.store(true);
    g_svc = std::thread(service_loop, g_mbox);
    g_svc_started.store(true);
    fprintf(stderr, "[preload] mailbox @ %p; service thread started\n", (void*)g_mbox);
    return true;
}

// Define `mailbox` and load the hostcall library into `exe`, once per executable.
// Uses the REAL loader entry points (not our hooks) so we don't recurse/re-match.
static void augment_executable(hsa_executable_t exe, hsa_agent_t agent) {
    BIND(hsa_executable_agent_global_variable_define);
    BIND(hsa_code_object_reader_create_from_memory);
    BIND(hsa_executable_load_agent_code_object);

    hsa_status_t s = real_hsa_executable_agent_global_variable_define(exe, agent, "mailbox", g_mbox);
    if (s != HSA_STATUS_SUCCESS) fprintf(stderr, "[preload] define(mailbox) status=%d\n", s);

    // Define the per-wave buffer symbol if the co references it (per-wave instrumentation).
    if (g_pw_buf) {
        hsa_status_t ps = real_hsa_executable_agent_global_variable_define(exe, agent, "g_pw_base", g_pw_buf);
        if (ps == HSA_STATUS_SUCCESS) fprintf(stderr, "[preload] defined g_pw_base -> %p\n", g_pw_buf);
        // (a co that doesn't reference g_pw_base just ignores the definition)
    }

    hsa_code_object_reader_t r{};
    s = real_hsa_code_object_reader_create_from_memory(g_lib_co().data(), g_lib_co().size(), &r);
    if (s != HSA_STATUS_SUCCESS) { fprintf(stderr, "[preload] lib reader status=%d\n", s); return; }
    s = real_hsa_executable_load_agent_code_object(exe, agent, r, "", nullptr);
    if (s != HSA_STATUS_SUCCESS) fprintf(stderr, "[preload] load(lib) status=%d\n", s);
    else fprintf(stderr, "[preload] augmented exe=%lu: mailbox defined + hostcall lib loaded\n", exe.handle);
}

// ------------------------------------------------------------------ hooks
// Substitute the app's fat binary at registration so HIP parses OUR (note-synced)
// instrumented co for its cached kernel metadata (private_segment_size etc.) AND loads
// it. Matches by the device co embedded in the incoming bundle == the original app co.
extern "C" void** __hipRegisterFatBinary(void* data) {
    static auto real = (void**(*)(void*))dlsym(RTLD_NEXT, "__hipRegisterFatBinary");
    if (cfg() && data) {
        auto* w = (FatBinaryWrapper*)data;
        const uint8_t* co; uint64_t co_sz, total;
        if (w->binary && bundle_device_co(w->binary, co, co_sz, total)) {
            for (auto& s : g_subs()) {
                if (co_sz == s.orig.size() && memcmp(co, s.orig.data(), co_sz) == 0) {
                    FatBinaryWrapper* nw = new FatBinaryWrapper(*w);
                    nw->binary = s.bundle.data();
                    fprintf(stderr, "[preload] __hipRegisterFatBinary: SUBSTITUTED %s "
                            "(device co %lu B -> instrumented bundle %zu B)\n",
                            s.tag.c_str(), co_sz, s.bundle.size());
                    return real(nw);
                }
            }
        }
    }
    return real(data);
}

// Detector (no substitution): the substituted fatbin makes HIP hand us the instrumented
// co bytes here — mark that reader so the load hook augments its executable.
extern "C"
hsa_status_t hsa_code_object_reader_create_from_memory(const void* data, size_t size,
                                                       hsa_code_object_reader_t* reader) {
    BIND(hsa_code_object_reader_create_from_memory);
    hsa_status_t s = real_hsa_code_object_reader_create_from_memory(data, size, reader);
    bool matched = false;
    if (s == HSA_STATUS_SUCCESS && cfg()) {
        for (auto& sub : g_subs()) {
            if (size == sub.inst.size() && memcmp(data, sub.inst.data(), size) == 0) {
                std::lock_guard<std::mutex> lk(g_mtx);
                g_inst_readers.insert(reader->handle);
                fprintf(stderr, "[preload] detected instrumented co load %s (%zu B); reader=%lu\n",
                        sub.tag.c_str(), size, reader->handle);
                matched = true; break;
            }
        }
    }
    if (!matched) LOG("[preload] passthrough co_reader size=%zu\n", size);
    return s;
}

extern "C"
hsa_status_t hsa_executable_load_agent_code_object(hsa_executable_t exe, hsa_agent_t agent,
                                                   hsa_code_object_reader_t reader,
                                                   const char* options,
                                                   hsa_loaded_code_object_t* lco) {
    BIND(hsa_executable_load_agent_code_object);

    bool is_inst;
    { std::lock_guard<std::mutex> lk(g_mtx); is_inst = g_inst_readers.count(reader.handle) > 0; }

    if (is_inst) {
        // Before loading the (instrumented) app co, make the executable self-contained:
        // start the runtime + define mailbox + load the hostcall lib into THIS exe once.
        if (ensure_runtime(agent)) {
            std::lock_guard<std::mutex> lk(g_mtx);
            if (g_augmented_exes.insert(exe.handle).second)
                augment_executable(exe, agent);
        }
    }
    hsa_status_t s = real_hsa_executable_load_agent_code_object(exe, agent, reader, options, lco);
    if (is_inst) LOG("[preload] loaded instrumented app co into exe=%lu status=%d\n", exe.handle, s);
    return s;
}

extern "C"
hsa_status_t hsa_executable_freeze(hsa_executable_t exe, const char* options) {
    BIND(hsa_executable_freeze);
    hsa_status_t s = real_hsa_executable_freeze(exe, options);
    { std::lock_guard<std::mutex> lk(g_mtx);
      if (g_augmented_exes.count(exe.handle))
          fprintf(stderr, "[preload] froze augmented exe=%lu status=%d (cross-object relocs resolved)\n",
                  exe.handle, s); }
    return s;
}

// ------------------------------------------------------------------ teardown
__attribute__((destructor)) static void fini() {
    if (g_svc_started.load()) {
        g_run.store(false, std::memory_order_release);
        if (g_svc.joinable()) g_svc.join();
    }
}
