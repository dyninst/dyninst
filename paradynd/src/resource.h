
#ifndef RESOURCE_HDR
#define RESOURCE_HDR

/*
 * $Log: resource.h,v $
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
  inline resource(string& abstraction, string& self_name, timeStamp creation,
		  void *handle, bool suppressed, resource *parent, 
		  vector<string>& v_names,
		  unsigned type);

  vector<string>& names() { return names_; }

  // A temporary kludge until the mdl is here alone
  bool is_top() const { return (names_.size() == 1); }

  string top() const { return names_[0]; }
  string full_name() const { return flat_name_; }
  string part_name() const { return part_name_; }
  bool suppressed() const { return suppressed_; }
  void suppress(bool set_to) { suppressed_ = set_to; }
  string abstraction() const { return abstraction_; }
  void *handle() const { return handle_; }
  resource *parent() const { return parent_; }
  unsigned id() const { return id_; }
  unsigned type() const { return type_; }

  static void make_canonical(const vector< vector<string> >& focus,
			     vector< vector<string> >& ret);
  inline static resource *findResource(string& name);
  inline static resource *findResource(unsigned& name);
  inline static resource *findResource(vector<string>& name);

  inline bool isResourceDescendent(resource *is_a_parent);

  static bool foc_to_strings(vector< vector<string> >& string_foc, vector<u_int>& ids);
  static resource *newResource(resource *parent, void *handle, string abstraction,
			       string name, timeStamp creation, string unique, 
			       unsigned type);
  static resource *newResource(resource *parent, string& name, unsigned id,
			       unsigned type);
  inline void set_id(unsigned id);

private:
  vector<string> names_;        // name of resource 
  string flat_name_;
  string abstraction_;          // abstraction name 
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

inline resource::resource(string& abstraction, string& self_name, timeStamp creat,
		   void *hand, bool supp, resource *par,
		   vector<string>& v_names, unsigned type)
: names_(v_names), flat_name_(par->full_name() + "/" + self_name),
  abstraction_(abstraction),
  creation_(creat), handle_(hand), suppressed_(supp), parent_(par),
  part_name_(self_name), type_(type) { }

inline resource *resource::findResource(unsigned& name) {
  if (!res_dict.defines(name)) return NULL;
  return (res_dict[name]);
}

inline resource *resource::findResource(string &name) {
  if (!name.length()) assert(0);

  if (allResources.defines(name))
    return (allResources[name]);
  else
    return (NULL);
}

inline resource *resource::findResource(vector<string>& name) {
  unsigned n_size = name.size();
  if (!n_size) assert(0);
  string flat(string("/")+name[0]);

  for (unsigned u=1; u<n_size; u++)
    flat += string("/") + name[u];

  if (allResources.defines(flat))
    return (allResources[flat]);
  else {
    cout << "Cannot find " << flat << endl;
    return (NULL);
  }
}

inline void resource::set_id(unsigned id) {
  id_ = id;
  res_dict[id] = this;
}

#endif
