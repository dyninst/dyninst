#if !defined( MICROBENCH_H )
#define MICROBENCH_H 1

typedef enum {
    MB_EXIT=MRN::FIRST_APPL_TAG,
    MB_ROUNDTRIP_LATENCY,
    MB_RED_THROUGHPUT
} Protocol;

#endif /* MICROBENCH_H */
