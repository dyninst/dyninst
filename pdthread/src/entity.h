#ifndef __libthread_entity_h__
#define __libthread_entity_h__

#ifndef __in_thrtabentries__
#error You should not include this file directly
#endif

class entity {
  public:
    virtual item_t gettype() { return item_t_unknown; }
};

#endif
