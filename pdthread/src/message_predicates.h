/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef __libthread_mailbox_predicates_h__
#define __libthread_mailbox_predicates_h__

#include "../h/thread.h"
#include "message.h"
#include "predicate.h"

namespace pdthr
{

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


class sender_matches_predicate : public predicate<message*> {
  private:
    thread_t tid_to_match;
  public:
    sender_matches_predicate(thread_t sender) :
        tid_to_match(sender) { }
    virtual ~sender_matches_predicate( void ) { }
    
    virtual bool satisfied_by(message* m) {
        return m && m->from() == tid_to_match;
    }
};

class tag_matches_predicate : public predicate<message*> {
  private:
    tag_t tag_to_match;
  public:
    tag_matches_predicate(thread_t sender) :
        tag_to_match(sender) { }
    virtual ~tag_matches_predicate( void ) { }
    
    virtual bool satisfied_by(message* m) {
        return m && m->type() == tag_to_match;
    }
};

class sender_and_tag_matches_predicate : public predicate<message*> {
  private:
    thread_t tid_to_match;
    tag_t tag_to_match;
  public:
    sender_and_tag_matches_predicate(thread_t sender, tag_t tag) :
        tid_to_match(sender),
        tag_to_match(tag) { }
    virtual ~sender_and_tag_matches_predicate( void ) { }
    
    virtual bool satisfied_by(message* m) {
        return m && m->from() == tid_to_match && m->type() == tag_to_match;
    }
};

class yank_tag_predicate : public predicate<message*> {
  private:
    tag_t* yanked_tag;
  public:
    yank_tag_predicate(tag_t* t) : yanked_tag(t) { }
    virtual ~yank_tag_predicate( void ) {}
    
    tag_t get_tag() {
        return *yanked_tag;
    }
    
    virtual bool satisfied_by(message* m) {
        *yanked_tag = m->type();
        return true;
    }
};

class matches_tag_predicate : public predicate<message*> {
  private:
    tag_t to_match;
  public:
    matches_tag_predicate(tag_t tag) {
        to_match = tag;
    }
    virtual ~matches_tag_predicate( void ) { }

    virtual bool satisfied_by(message* m) { 
        return m && m->type() == to_match;
    }
};

class yank_sender_predicate : public predicate<message*> {
  private:
    thread_t* yanked_sender;
  public:
    yank_sender_predicate(thread_t* t) : yanked_sender(t) { }
    virtual ~yank_sender_predicate( void ) { }
    
    thread_t get_sender() {
        return* yanked_sender;
    }
    
    virtual bool satisfied_by(message* m) {
        *yanked_sender = m->from();
        return true;
    }
};

#if READY
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
#endif // READY

class match_message_pred : public predicate<message*> {
  private:
    bool is_tag_specified;
    bool is_sender_specified;
    thread_t sender;
    tag_t tag;

	bool tag_matches( message* msg )
		{
			assert( is_tag_specified );
			return ((msg->type() == tag) ||
					(tag == MSG_TAG_THREAD) && (msg->type() >= MSG_TAG_USER) );
		}

  public:
    thread_t actual_sender;
    tag_t actual_type;
    
    match_message_pred(thread_t _sender, tag_t _tag) : 
            is_tag_specified( _tag != MSG_TAG_UNSPEC ),
            is_sender_specified( _sender != THR_TID_UNSPEC ),
            sender(_sender), 
            tag(_tag) { }
    virtual ~match_message_pred( void ) {}
    
    virtual bool satisfied_by(message* element) {
        assert(element);

        actual_sender = element->from();
        actual_type = element->type();
        
		if( is_tag_specified && is_sender_specified )
		{
			// both sender and tag were specified - must match both
            return tag_matches(element) && (element->from() == sender);
        }
		else if (is_tag_specified)
		{
			// tag was specified but not sender - must match tag
            return tag_matches(element);
        }
		else if (is_sender_specified)
		{
			// sender was specified but not tag - must match sender
            return element->from() == sender;
		}
		else
		{
			// neither tag nor sender was specified - everything matches
			assert( !is_tag_specified && !is_sender_specified );
            return true;
        }
    }
};

} // namespace pdthr

#endif

