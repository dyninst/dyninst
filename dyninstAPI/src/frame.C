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

// $Id: frame.C,v 1.3 2005/02/22 22:30:47 tlmiller Exp $

#include <stdio.h>
#include <iostream>
#include "process.h"
#include "dyn_thread.h"
#include "dyn_lwp.h"
#include "function.h"
#include "func-reloc.h"
#include "instPoint.h"
#include "trampTemplate.h"
#include "miniTrampHandle.h"
#include "frame.h"

#if defined( DONT_LEAK_FRAME_MEMORY )
#if defined( arch_ia64 )

#include <libunwind.h>
#include <libunwind-ptrace.h>

Frame & Frame::operator = ( const Frame & rhs ) {
	memmove( this, & rhs, sizeof( Frame ) );
	
	if( this->unwindCursor_ != NULL ) {
   		this->unwindCursor_ = malloc( sizeof( unw_cursor_t ) );
   		assert( this->unwindCursor_ != NULL );
   		memmove( this->unwindCursor_, rhs.unwindCursor_, sizeof( unw_cursor_t ) );
		}
		
	return * this;
	} /* end operator =() */
	
#else

Frame & Frame::operator = ( const Frame & rhs ) {
	memmove( this, & rhs, sizeof( Frame ) );
	
	return * this;
	} /* end operator =() */
	
#endif

/* Invoke operator = to make the right thing happen. */
Frame( const Frame & f ) {
	* this = f;
	} /* end copy constructor */

Frame::~Frame() {
	if( unwindCursor_ != NULL ) {
		// /* DEBUG */ fprintf( stderr, "%s[%d]: freeing %p.\n", __FILE__, __LINE__, unwindCursor_ );
		free( unwindCursor_ );
		}
	} /* end ~Frame() */
#endif

Frame::Frame() : 
  frameType_(FRAME_unset), 
  uppermost_(false), 
  pc_(0), 
  fp_(0), 
  sp_(0),
  pid_(0), 
  proc_(NULL), 
  thread_(NULL), 
  lwp_(NULL), 
  range_(0), 
  unwindCursor_(NULL),
  pcAddr_(0)
  {}


Frame::Frame(Address pc, Address fp, Address sp,
	     unsigned pid, process *proc, 
	     dyn_thread *thread, dyn_lwp *lwp, 
	     bool uppermost,
	     Address pcAddr,
	     void * unwindCursor) :
  frameType_(FRAME_unset),
  uppermost_(uppermost),
  pc_(pc), fp_(fp), sp_(sp),
  pid_(pid), proc_(proc), thread_(thread), lwp_(lwp), 
  range_(0), unwindCursor_( unwindCursor ), pcAddr_(pcAddr)
  {};


codeRange *Frame::getRange() {
  if (!range_) {
    // First time... so get it and cache
    if (!getProc())
      return NULL;
    range_ = getProc()->findCodeRangeByAddress(getPC());
  }
  return range_;
}

void Frame::setRange(codeRange *range) {
  assert(!range_);
  range_ = range;
}

bool Frame::isLastFrame() const
{
   if (fp_ == 0) return true;
   if (pc_ == 0) return true;
   return false;
}

ostream & operator << ( ostream & s, Frame & f ) {
	codeRange * range = f.getRange();
	s << "PC: 0x" << std::hex << f.getPC() << " ";
	if( range ) {
		int_function * func_ptr = range->is_function();
		trampTemplate * basetramp_ptr = range->is_basetramp();
		miniTrampHandle * minitramp_ptr = range->is_minitramp();
		relocatedFuncInfo * reloc_ptr = range->is_relocated_func();
		
		if( func_ptr ) {
			s << "(" << func_ptr->prettyName().c_str() << ") ";
			}
		if( basetramp_ptr ) {
			s << "(basetramp from '" << basetramp_ptr->location->pointFunc()->prettyName().c_str() << "' ";
			}
		if( minitramp_ptr ) {
			s << "(minitramp from '" << minitramp_ptr->baseTramp->location->pointFunc()->prettyName().c_str() << "' ";
			}
		if( reloc_ptr ) {
			s << "(" << reloc_ptr->func()->prettyName().c_str() << " [RELOCATED]) ";
			}
		}
   
	s << "FP: 0x" << std::hex << f.getFP() << " PID: " << std::dec << f.getPID() << " "; 
	if( f.getThread() ) {
   		s << "TID: " << f.getThread()->get_tid() << " ";
   		}
   	if( f.getLWP() ) {
   		s << "LWP: " << f.getLWP()->get_lwp_id() << " ";
   		}
	
	return s;
	}

