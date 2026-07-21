// loadtrace.cpp — diagnostic LD_PRELOAD interposer.
// Logs HIP's HSA code-object loading path so we know where to hook the real
// instrumentation preload. Intercepts the HSA executable/loader entry points,
// logs their args, then chains to the real implementation via RTLD_NEXT.
#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdint>

#define REAL(name) static decltype(&name) real_##name = \
    (decltype(&name))dlsym(RTLD_NEXT, #name); if (!real_##name) { \
    fprintf(stderr, "[trace] dlsym miss %s\n", #name); }

extern "C" {

hsa_status_t hsa_executable_create_alt(hsa_profile_t p,
        hsa_default_float_rounding_mode_t r, const char* opt, hsa_executable_t* e) {
    REAL(hsa_executable_create_alt);
    hsa_status_t s = real_hsa_executable_create_alt(p, r, opt, e);
    fprintf(stderr, "[trace] hsa_executable_create_alt -> exe=%lu status=%d opt='%s'\n",
            e ? e->handle : 0, s, opt ? opt : "");
    return s;
}

hsa_status_t hsa_executable_create(hsa_profile_t p,
        hsa_executable_state_t st, const char* opt, hsa_executable_t* e) {
    REAL(hsa_executable_create);
    hsa_status_t s = real_hsa_executable_create(p, st, opt, e);
    fprintf(stderr, "[trace] hsa_executable_create -> exe=%lu status=%d\n", e?e->handle:0, s);
    return s;
}

hsa_status_t hsa_code_object_reader_create_from_memory(const void* data, size_t size,
        hsa_code_object_reader_t* r) {
    REAL(hsa_code_object_reader_create_from_memory);
    hsa_status_t s = real_hsa_code_object_reader_create_from_memory(data, size, r);
    fprintf(stderr, "[trace] co_reader_create_from_memory data=%p size=%zu -> reader=%lu status=%d\n",
            data, size, r ? r->handle : 0, s);
    return s;
}

hsa_status_t hsa_code_object_reader_create_from_file(hsa_file_t fd,
        hsa_code_object_reader_t* r) {
    REAL(hsa_code_object_reader_create_from_file);
    hsa_status_t s = real_hsa_code_object_reader_create_from_file(fd, r);
    fprintf(stderr, "[trace] co_reader_create_from_file fd=%d -> reader=%lu status=%d\n",
            fd, r ? r->handle : 0, s);
    return s;
}

hsa_status_t hsa_executable_load_agent_code_object(hsa_executable_t e, hsa_agent_t a,
        hsa_code_object_reader_t r, const char* opt, hsa_loaded_code_object_t* lco) {
    REAL(hsa_executable_load_agent_code_object);
    hsa_status_t s = real_hsa_executable_load_agent_code_object(e, a, r, opt, lco);
    fprintf(stderr, "[trace] load_agent_code_object exe=%lu agent=%lu reader=%lu opt='%s' -> status=%d\n",
            e.handle, a.handle, r.handle, opt ? opt : "", s);
    return s;
}

hsa_status_t hsa_executable_load_program_code_object(hsa_executable_t e,
        hsa_code_object_reader_t r, const char* opt, hsa_loaded_code_object_t* lco) {
    REAL(hsa_executable_load_program_code_object);
    hsa_status_t s = real_hsa_executable_load_program_code_object(e, r, opt, lco);
    fprintf(stderr, "[trace] load_PROGRAM_code_object exe=%lu reader=%lu -> status=%d\n",
            e.handle, r.handle, s);
    return s;
}

hsa_status_t hsa_executable_freeze(hsa_executable_t e, const char* opt) {
    REAL(hsa_executable_freeze);
    hsa_status_t s = real_hsa_executable_freeze(e, opt);
    fprintf(stderr, "[trace] hsa_executable_freeze exe=%lu -> status=%d\n", e.handle, s);
    return s;
}

} // extern "C"
