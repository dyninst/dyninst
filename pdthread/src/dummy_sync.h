#ifndef SEEN_DUMMY_SYNC
#define SEEN_DUMMY_SYNC

class dummy_sync {
public:
  inline void lock() {}
  inline void unlock() {}
  inline void register_cond(unsigned cond_num) {}
  inline void signal(unsigned cond_num) {}
  inline void wait(unsigned cond_num) {}
};

#endif
