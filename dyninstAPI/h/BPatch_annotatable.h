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

// $Id: BPatch_annotatable.h,v 1.3 2007/07/20 22:28:15 tugrul Exp $

#ifndef BPATCH_ANNOTATABLE_
#define BPATCH_ANNOTATABLE_

#include "BPatch_Vector.h"

template <class K, class V>
class dictionary_hash;

class pdstring;

class BPatch_annotation {
  void* item;
 public:
  BPatch_annotation(void* it) : item(it) {}
  void setItem(void* newItem) { item=newItem; }
  void* getItem() {return item; }
};

template <class T>
class BPatch_annotatable
{
  static int number;
  BPatch_Vector<BPatch_Vector<BPatch_annotation*>* > annotations;
  static dictionary_hash<pdstring, int>* annotationTypes;// = *(new dictionary_hash<pdstring, int>(pdstring::hash));
  static dictionary_hash<pdstring, int>* metadataTypes; //(pdstring::hash);
  static int metadataNum;
 public:
  BPatch_annotatable();
  virtual ~BPatch_annotatable();

  int createAnnotationType(char* name);
  int getAnnotationType(char* name);
  int createMetadata(char* name);
  int getMetadata(char* name);
  void setAnnotation(int type, BPatch_annotation* annotation);
  BPatch_annotation* getAnnotation(int type, int index=0);
  void* getAnnotation(char* name, int index=0);
};

#endif
