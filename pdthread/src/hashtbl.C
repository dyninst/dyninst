#ifndef __libthread_hashtbl_C__
#define __libthread_hashtbl_C__

#define HASHTBL_SIZE 512

#ifdef HASH_DEBUG
#include <stdio.h>
#endif

#include "hash.h"

/* Basic fixed-size hash table class, protected by a monitor.  Some important
   restrictions: 
    * if key_t is a pointer type, then keys will be hashed based on
      their address, not the contents of said address.  To hash on contents, 
      use the put(key_t,len, value) and get(key_t,len) methods.
    * value_t must be a pointer type.
 */

template<class key_t, class value_t>
struct pair {
    key_t key;
    value_t value;
    pair<key_t,value_t>* next;
    pair<key_t,value_t>* cleanup_next;
};

template<class key_t, class value_t, class monitor_t>
class hashtbl {
  private:
    pair<key_t,value_t>** entries;
    pair<key_t,value_t>* cleanup_list;
    monitor_t monitor;
    int (*compare_func)(const key_t*,const key_t*);

    inline unsigned long 
    hashval(key_t key) { return hash((ub1*)&key, sizeof(key)) % HASHTBL_SIZE; }

    inline unsigned long 
    hashval(key_t *keyp, unsigned len) { return hash((ub1*)keyp, len) %
                                             HASHTBL_SIZE; }

    inline pair<key_t,value_t>* 
    find(pair<key_t,value_t>* start_here, key_t key) {
        while(start_here) {
            if(!start_here || start_here->key == key) break;
            start_here = start_here->next;
        }
        return start_here;
    }

  public:
    hashtbl() {
        entries = new pair<key_t,value_t>* [HASHTBL_SIZE];
        for(int i = 0; i < HASHTBL_SIZE; i++)
            entries[i] = NULL;
        compare_func = NULL;
        cleanup_list = NULL;
    }

    hashtbl(int (*comparator)(const key_t*,const key_t*)) {
        entries = new pair<key_t,value_t>* [HASHTBL_SIZE];
        for(int i = 0; i < HASHTBL_SIZE; i++)
            entries[i] = NULL;
        compare_func = comparator;
        cleanup_list = NULL;
    }
    
    ~hashtbl() {
        pair<key_t,value_t> *bucket_to_delete = cleanup_list,
            *next_bucket = NULL;
        
        while(bucket_to_delete) {
            next_bucket = bucket_to_delete->cleanup_next;
            delete bucket_to_delete;
            bucket_to_delete = next_bucket;
        }
        
        delete [] entries;
    }

    void put(key_t key, value_t value) {
        monitor.lock();
        unsigned long index = hashval(key);
        pair<key_t,value_t> *start_bucket, *result_bucket;
        start_bucket = entries[index];
        result_bucket = NULL;
#ifdef HASH_DEBUG
        fprintf(stderr, "key %d hashes to %d\n", (unsigned)key, index);
#endif
        result_bucket = find(start_bucket, key);
            
        if(!result_bucket) {
            result_bucket = new pair<key_t,value_t>;
            result_bucket->cleanup_next = cleanup_list;
            result_bucket->next = entries[index];
            result_bucket->key = key;
            result_bucket->value = value;
            cleanup_list = result_bucket;
            entries[index] = result_bucket;
        } else {
            result_bucket->value = value;
        }
        monitor.unlock();
    }

    void put(key_t key, value_t value, unsigned len) {
        monitor.lock();
        unsigned long index = hashval(key, len);
        pair<key_t,value_t> *start_bucket, *result_bucket;
        start_bucket = entries[index];
        result_bucket = NULL;
#ifdef HASH_DEBUG
        fprintf(stderr, "key %d hashes to %d\n", (unsigned)key, index);
#endif
        result_bucket = find(start_bucket, key);
            
        if(!result_bucket) {
            result_bucket = new pair<key_t,value_t>;
            result_bucket->cleanup_next = cleanup_list;
            result_bucket->next = entries[index];
            result_bucket->key = key;
            result_bucket->value = value;
            cleanup_list = result_bucket;
            entries[index] = result_bucket;
        } else {
            result_bucket->value = value;
        }
        monitor.unlock();
    }    

    value_t get(key_t key) {
        monitor.lock();
        
        unsigned long index = hashval(key);
        value_t retval;
        pair<key_t,value_t> *start_bucket, *result_bucket;
        start_bucket = entries[index];
        result_bucket = NULL;
#ifdef HASH_DEBUG
        fprintf(stderr, "key %d hashes to %d\n", (unsigned)key, index);
#endif
        result_bucket = find(start_bucket, key);

        if(result_bucket)
            retval = result_bucket->value;
        else
            retval = (value_t)NULL;
        
        monitor.unlock();
        return retval;
    }

    value_t get(key_t *key, unsigned len) {
        monitor.lock();
        
        unsigned long index = hashval(key, len);
        value_t retval;
        pair<key_t,value_t> *start_bucket, *result_bucket;
        start_bucket = entries[index];
        result_bucket = NULL;
#ifdef HASH_DEBUG
        fprintf(stderr, "key %d hashes to %d\n", (unsigned)key, index);
#endif
        result_bucket = find(start_bucket, key);

        if(result_bucket)
            retval = result_bucket->value;
        else
            retval = NULL;
        
        monitor.unlock();
        return retval;
    }

    template<class Command>
    void map_keys(Command* cmd) {
        monitor.lock();
        for (int i = 0; i < HASHTBL_SIZE; i++)
            if(entries[i] != NULL) cmd->exec((entries[i])->key);
        monitor.unlock();
    }

    template<class Command>
    void map_vals(Command* cmd) {
        monitor.lock();
        for (int i = 0; i < HASHTBL_SIZE; i++)
            if(entries[i] != NULL) cmd->exec((entries[i])->value);        
        monitor.unlock();
    }    
};

#endif

