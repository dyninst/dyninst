#ifndef __libthread_entity_h__
#define __libthread_entity_h__

class entity {
  public:
    virtual item_t gettype() { return item_t_unknown; }
};

#endif
