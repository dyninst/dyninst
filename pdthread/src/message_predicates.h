#ifndef __libthread_mailbox_predicates_h__
#define __libthread_mailbox_predicates_h__

#include "message.h"
#include "predicate.h"

class matches_tag_pred : public predicate<message*> {
  private:
    tag_t to_match;
  public:
    matches_tag_pred(tag_t tag) {
        to_match = tag;
    }

    virtual bool satisfied_by(message* m) { 
        return m && m->type() == to_match;
    }

    virtual ~matches_tag_pred() { }
};

class yank_tag_pred : public predicate<message*> {
  private:
    tag_t yanked_tag;
  public:
    tag_t get_tag() {
        return yanked_tag;
    }
    
    virtual bool satisfied_by(message* m) {
        yanked_tag = m->type();
        return true;
    }

    virtual ~yank_tag_pred() { }
};

class is_internal_msg_pred : public predicate<message*> {
  public:
    is_internal_msg_pred();
    
    virtual bool satisfied_by(message* m) { 
        return m && m->type() >= MSG_TAG_USER;
    }

    virtual ~is_internal_msg_pred() { }
};

class matches_sender_pred : public predicate<message*> {
  private:
    thread_t to_match;
  public:
    matches_sender_pred(thread_t sender) {
        to_match = sender;
    }

    virtual bool satisfied_by(message* m) { 
        return m && m->from() == to_match;
    }

    virtual ~matches_sender_pred() { }
};


class sender_matches_predicate {
  private:
    thread_t tid_to_match;
  public:
    sender_matches_predicate(thread_t sender) :
        tid_to_match(sender) { }
    
    inline bool satisfied_by(message* m) {
        return m && m->from() == tid_to_match;
    }
};

class tag_matches_predicate {
  private:
    tag_t tag_to_match;
  public:
    tag_matches_predicate(thread_t sender) :
        tag_to_match(sender) { }
    
    inline bool satisfied_by(message* m) {
        return m && m->type() == tag_to_match;
    }
};

class sender_and_tag_matches_predicate {
  private:
    thread_t tid_to_match;
    tag_t tag_to_match;
  public:
    sender_and_tag_matches_predicate(thread_t sender, tag_t tag) :
        tid_to_match(sender),
        tag_to_match(tag) { }
    
    inline bool satisfied_by(message* m) {
        return m && m->from() == tid_to_match && m->type() == tag_to_match;
    }
};

class yank_tag_predicate {
  private:
    tag_t* yanked_tag;
  public:
    yank_tag_predicate(tag_t* t) : yanked_tag(t) { }
    
    tag_t get_tag() {
        return *yanked_tag;
    }
    
    inline bool satisfied_by(message* m) {
        *yanked_tag = m->type();
        return true;
    }
};

class matches_tag_predicate {
  private:
    tag_t to_match;
  public:
    matches_tag_predicate(tag_t tag) {
        to_match = tag;
    }

    bool satisfied_by(message* m) { 
        return m && m->type() == to_match;
    }
};

class yank_sender_predicate {
  private:
    thread_t* yanked_sender;
  public:
    yank_sender_predicate(thread_t* t) : yanked_sender(t) { }
    
    thread_t get_sender() {
        return* yanked_sender;
    }
    
    inline bool satisfied_by(message* m) {
        *yanked_sender = m->from();
        return true;
    }
};

template<class P1,class P2, class Domain>
class predicate_and {
  private:
    P1* pred1;
    P2* pred2;
  public:
    predicate_and(P1* p1, P2* p2)
        : pred1(p1), pred2(p2) { }
    inline bool satisfied_by(Domain element) {
        return pred1->satisfied_by(element) &&
            pred2->satisfied_by(element);
    }

    inline P1* get_first() {
        return pred1;
    }

    inline P2* get_second() {
        return pred2;
    }
};

#endif
