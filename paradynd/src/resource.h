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

#ifndef RESOURCE_HDR
#define RESOURCE_HDR

/*
 * $Log: resource.h,v $
 * Revision 1.8  1997/01/15 00:29:51  tamches
 * added uses of dictionary find() method.  Added some const.
 *
 * Revision 1.7  1996/09/26 18:59:13  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.6  1996/08/16 21:19:47  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/06/01 00:03:29  tamches
 * const and refs added in appropriate place to enhance speed and
 * compile-time error checking.
 *
 * Revision 1.4  1996/03/01 22:35:58  mjrg
 * Added a type to resources.
 * Changes to the MDL to handle the resource hierarchy better.
 *
 * Revision 1.3  1995/11/29 18:45:25  krisna
 * added inlines for compiler. added templates
 *
 * Revision 1.2  1995/05/18 10:41:51  markc
 * Cache global ids supplied by paradyn
 * have a canonical form for the resource list
 *
 * Revision 1.1  1994/11/01  16:58:10  markc
 * Prototypes
 *
 */

#include "dyninstRPC.xdr.h"

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
extern resource *processResource;
extern resource *moduleRoot;
extern resource *syncRoot;

class resource {
public:
  enum index { machine, procedure, process, sync_object };

  inline resource();
  inline resource(const string& abstraction, const string& self_name,
		  timeStamp creation,
		  void *handle, bool suppressed, resource *parent, 
		  const vector<string>& v_names,
		  unsigned type);

  const vector<string>& names() const { return names_; }

  // A temporary kludge until the mdl is here alone
  bool is_top() const { return (names_.size() == 1); }

  const string &top() const { return names_[0]; }
  const string &full_name() const { return flat_name_; }
  const string &part_name() const { return part_name_; }

  bool suppressed() const { return suppressed_; }
  void suppress(bool set_to) { suppressed_ = set_to; }

  const string &abstraction() const { return abstraction_; }
  void *handle() const { return handle_; }
  unsigned id() const { return id_; }
  unsigned type() const { return type_; }

  resource *parent() const { return parent_; }

  static void make_canonical(const vector< vector<string> >& focus,
			     vector< vector<string> >& ret);
  inline static resource *findResource(const string& name);
  inline static resource *findResource(unsigned& name);
  inline static resource *findResource(vector<string>& name);

  inline bool isResourceDescendent(resource *is_a_parent);

  static bool foc_to_strings(vector< vector<string> >& string_foc,
			     const vector<u_int>& ids,
			     bool print_err_msg);
  static resource *newResource(resource *parent, void *handle,
			       const string &abstraction,
			       const string &name, timeStamp creation,
			       const string &unique, 
			       unsigned type);
  static resource *newResource(resource *parent, const string& name, unsigned id,
			       unsigned type);
  inline void set_id(unsigned id);
  static u_int num_outstanding_creates; 


private:
  vector<string> names_;        // name of resource 
  string flat_name_;
  string abstraction_;          // abstraction name (wouldn't an abstraction-id be
                                // more efficient?)
  timeStamp creation_;          // when did it get created
  void *handle_;                // resource specific data 
  bool suppressed_;
  resource *parent_;
  string part_name_;
  unsigned id_;
  unsigned type_;  // the mdl type of this resource
  
  static dictionary_hash<string, resource*> allResources;
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
: creation_(0), handle_(NULL), suppressed_(true), parent_(NULL) { }

inline resource::resource(const string& abstraction, const string& self_name,
                   timeStamp creat,
		   void *hand, bool supp, resource *par,
		   const vector<string>& v_names, unsigned type)
: names_(v_names), flat_name_(par->full_name() + "/" + self_name),
  abstraction_(abstraction),
  creation_(creat), handle_(hand), suppressed_(supp), parent_(par),
  part_name_(self_name), type_(type) { }

inline resource *resource::findResource(unsigned& name) {
   // why is the param call-by-reference?
   resource *result;
   if (!res_dict.find(name, result))
      return NULL;
   else
      return result;
}

inline resource *resource::findResource(const string &name) {
  if (!name.length()) assert(0);

  resource *result;
  if (!allResources.find(name, result))
     return NULL;
  else
     return result;
}

inline resource *resource::findResource(vector<string>& name) {
  unsigned n_size = name.size();
  if (!n_size) assert(0);
  string flat(string("/")+name[0]);

  for (unsigned u=1; u<n_size; u++)
    flat += string("/") + name[u];

  resource *result;
  if (!allResources.find(flat, result)) {
    cout << "Cannot find " << flat << endl;
    return (NULL);
  }
  else
     return result;
}

inline void resource::set_id(unsigned id) {
  id_ = id;
  res_dict[id] = this;
}

#endif
