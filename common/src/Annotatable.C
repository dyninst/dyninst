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

// $Id: Annotatable.C,v 1.1 2007/09/17 15:13:57 tugrul Exp $

#include "common/h/Annotatable.h"
#include "common/h/String.h"
#include "common/h/Dictionary.h"

template<class T>
Annotatable<T>::Annotatable() {
  if(annotationTypes == NULL) {
    annotationTypes = new dictionary_hash<pdstring, int>(pdstring::hash);
    number = 0;
  }
  if(metadataTypes == NULL) {
    metadataTypes = new dictionary_hash<pdstring, int>(pdstring::hash);
    metadataNum = 0;
  }
}

template<class T>
Annotatable<T>::~Annotatable() {
  unsigned i,j;
  for(i=0; i<annotations.size(); i++) {
    vector<Annotation*>* list = annotations.at(i);
    // vectorSet<Annotation*>* list = annotations[i];
    for(j=0; j<list->size(); j++) {
      delete (*list).at(j);
      // delete (*list)[j];
    }
    delete list;
  }
}

template<class T>
int Annotatable<T>::createAnnotationType(char* name) {
  pdstring n(name);
  int num = getAnnotationType(name);
  if(num != -1) {
    return num;
  }
  annotationTypes->set(n,number);
  number++;
  return number-1;
}

template<class T>
int Annotatable<T>::getAnnotationType(char* name) {
  pdstring str(name);
  if(! annotationTypes->defines(str)) {
    return -1;
  }
  int n = annotationTypes->get(str);
  return n;
}

template<class T>
int Annotatable<T>::createMetadata(char* name) {
  pdstring n(name);
  int num = getMetadata(name);
  if(num != -1) {
    return num;
  }
  metadataTypes->set(n,metadataNum);
  metadataNum++;
  return metadataNum-1;
}

template<class T>
int Annotatable<T>::getMetadata(char* name) {
  pdstring str(name);
  if(! metadataTypes->defines(str)) {
    return -1;
  }
  int n = metadataTypes->get(str);
  return n;
}

template<class T>
void Annotatable<T>::setAnnotation(int type, Annotation* annotation) {
  if(type<number) {
    while((unsigned)type >= annotations.size()) {
      annotations.push_back(new vector<Annotation*>());
      // annotations+= new vectorSet<BPatch_annotation*>();
      // annotations.push_back(new BPatch_Vector<BPatch_annotation*>());
    }
    (annotations.at(type))->push_back(annotation);
    // (*annotations[type])+= annotation;
    // (annotations[type])->push_back(annotation);
  }
  return;
}

template<class T>
Annotation* Annotatable<T>::getAnnotation(int type, int index) {
  if((unsigned)type<annotations.size()) {
    vector<Annotation*>* v = annotations.at(type);
    // vectorSet<BPatch_annotation*>* v = annotations[type];
    if(v != NULL && v->size() > (unsigned)index) {
      return (*v)[index];
    }
  }
  return NULL;
}

template<class T>
void* Annotatable<T>::getAnnotation(char* name, int index) {
  int annotType = getAnnotationType(name);
  if(annotType == -1) {
    annotType = createAnnotationType(name);
    return NULL;
  }
  Annotation* annot = getAnnotation(annotType, index);
  if(annot == NULL)
    return NULL;
  return annot->getItem();
}
