#ifndef _TC_LOCK_H_
#define _TC_LOCK_H_
typedef struct {
  int mutex ;
  int tid ;
} tc_lock_t ;

#define DECLARE_TC_LOCK(l) 	   tc_lock_t l={0 ,-1}
#define EXTERN_DECLARE_TC_LOCK(l)  extern tc_lock_t l

int tc_lock_init(tc_lock_t* ) ;
int tc_lock_lock(tc_lock_t* ) ;
int tc_lock_trylock(tc_lock_t*) ;
int tc_lock_unlock(tc_lock_t* ) ;
int tc_lock_destroy(tc_lock_t* ) ;

#endif /* _TC_LOCK_H_ */
