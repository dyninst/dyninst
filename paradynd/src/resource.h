/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: resource.h,v

#ifndef RESOURCE_HDR
#define RESOURCE_HDR

#include "common/h/Time.h"
#include "common/h/Dictionary.h"
#include "pdutil/h/resource.h"

class resource;

/*
 * Control information arriving about a resource Classes
 * resource		- enable notification of children of this resource
 */
bool enableResourceCreationNotification(resource*);

class resource;

extern resource *rootResource;
extern resource *machineRoot;
extern resource *machineResource;
// extern resource *processResource;
extern resource *moduleRoot;
extern resource *syncRoot;

const pdstring slashStr = "/";
extern resource *memoryRoot;
extern resource *memoryResource;


class resource {
public:
  enum index { machine, procedure, sync_object, memory };

  inline resource();
  inline resource(const pdstring& abstraction, const pdstring& self_name,
		  timeStamp creation,
		  void *handle, bool suppressed, resource *parent, 
          ResourceType type,
		  unsigned int mdlType);

  const pdstring &name() const { return name_; }

  const pdvector<pdstring> names() const {
    if (parent() == NULL) {
      pdvector<pdstring> ret;
      return ret;
    }
    unsigned level = 1;
    resource *p;
    for (p = parent(); p && p->parent(); p = p->parent())
      level++;
    pdvector<pdstring> ret(level);
    if (level == 0)
      return ret;
    ret[--level] = name();
    for (p = parent(); p->parent(); p = p->parent())
      ret[--level] = p->name();
    assert(level==0);
    return ret;
  }

  bool is_top() const { return (parent() == NULL); }

  const pdstring full_name() const { 
    if (!parent())
      return name();
    pdstring full_name_ = slashStr + name();
    for (resource *p = parent(); p && p->parent(); p = p->parent())
      full_name_ = slashStr + p->part_name() + full_name_;
    return full_name_;
  }
  const pdstring &part_name() const { return name_; }

  bool suppressed() const { return suppressed_; }
  void suppress(bool set_to) { suppressed_ = set_to; }

  const pdstring &abstraction() const { return abstraction_; }
  void *handle() const { return handle_; }
  unsigned id() const { return id_; }
  unsigned int mdlType() const { return mdlType_; }
  ResourceType type( void ) const { return type_; }

  resource *parent() const { return parent_; }

  static void make_canonical(const pdvector< pdvector<pdstring> >& focus,
			     pdvector< pdvector<pdstring> >& ret);
  inline static resource *findResource(const pdstring& name);
  inline static resource *findResource(unsigned name);
  inline static resource *findResource(pdvector<pdstring>& name);

  inline bool isResourceDescendent(resource *is_a_parent);

  static bool foc_to_strings(pdvector< pdvector<pdstring> >& string_foc,
			     const pdvector<u_int>& ids,
			     bool print_err_msg);
  static resource *newResource(resource *parent, void *handle,
			       const pdstring &abstraction,
			       const pdstring &name, timeStamp creation,
			       const pdstring &unique, 
                   ResourceType type,
			       unsigned int mdlType,
			       bool send_now);
  static void send_now();

  static resource *newResource_ncb(resource *parent, void *handle,
			       const pdstring &abstraction,
			       const pdstring &name, timeStamp creation,
			       const pdstring &unique, 
                   ResourceType type,
			       unsigned int mdlType);
  static resource *newResource(resource *parent,
                                const pdstring& name,
                                unsigned int id,
                                ResourceType type,
			                    unsigned int mdlType);
  inline void set_id(unsigned id);

private:
  pdstring name_;                 // name of resource
  pdstring abstraction_;          // abstraction name (wouldn't an abstraction-id be
                                // more efficient?)
  timeStamp creation_;          // when did it get created
  void *handle_;                // resource specific data 
  bool suppressed_;
  resource *parent_;
  unsigned id_;
  ResourceType type_;         // the type of this resource (func, module, ...)
  unsigned mdlType_;            // the mdl type of this resource

  static dictionary_hash<pdstring, resource*> allResources;
  static dictionary_hash<unsigned, resource*> res_dict;
};

inline bool resource::isResourceDescendent(resource *is_a_parent) {
  resource *current = this;
  while (current) {
    if (current == is_a_parent)
      return true;
    else 
      current = current->parent();
  }
  return false;
}

inline resource::resource()
: creation_(timeStamp::ts1970()),
  handle_(NULL),
  suppressed_(true),
  parent_(NULL)
{ }

inline resource::resource(const pdstring& abstraction,
                            const pdstring& self_name,
                            timeStamp creat,
		                    void *hand,
                            bool supp,
                            resource *par,
                            ResourceType type,
		                    unsigned int mdlType)
: name_(self_name),
  abstraction_(abstraction),
  creation_(creat), handle_(hand), suppressed_(supp), parent_(par),
  type_(type),
  mdlType_(mdlType)
{ }

inline resource *resource::findResource(unsigned name) {
   // why is the param call-by-reference?
   resource *result;
   if (!res_dict.find(name, result))
     return NULL;
   else
     return result;
}

inline resource *resource::findResource(const pdstring &name) {
  if (!name.length()) assert(0);

  resource *result;
  if (!allResources.find(name, result))
     return NULL;
  else
     return result;
}

inline resource *resource::findResource(pdvector<pdstring>& name) {
  unsigned n_size = name.size();
  if (!n_size) assert(0);
  pdstring flat(slashStr+name[0]);

  for (unsigned u=1; u<n_size; u++)
    flat += slashStr + name[u];

  resource *result;
  if (!allResources.find(flat, result)) {
    cerr << "Cannot find " << flat.c_str() << endl;
    return (NULL);
  }
  else
     return result;
}

inline void resource::set_id(unsigned new_id) {
  if (res_dict.defines(id_))
    res_dict.undef(id_);
  id_ = new_id;
  res_dict[new_id] = this;
}

#endif
