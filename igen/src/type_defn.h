// type_defn.h
// Moved here from parse.h, into its own [.hC] file combination,
// for clarity

#ifndef _TYPE_DEFN_H_
#define _TYPE_DEFN_H_

#include "common/h/String.h"
#include <fstream.h>
#include "arg.h"
#include "common/h/Vector.h"

class type_defn {
public:
  friend void recursive_dump_kids(const type_defn *from, ofstream &output);

  typedef enum { TYPE_SCALAR, TYPE_COMPLEX } type_type;

  type_defn(string stl_name, string element_name, const unsigned star_count,
            bool in_lib);
  type_defn(const string &name, bool is_class, bool is_abstract,
            bool is_derived, const string &parent, 
            const type_type type, vector<arg*> *arglist = NULL, 
            const bool can_point=false, const bool in_lib=false, 
	    const string &ignore="", const string &bundle_name="");
  type_defn(const string &name, bool is_class, type_type type, 
	    const bool can_point=false, const bool in_lib=false);
  ~type_defn() { }

  string gen_bundler_name_stl(bool send_routine) const;
  string gen_bundler_name(bool send_routine) const;
  string gen_bundler_call(const string &obj_name, const string &data_name,
                          const unsigned) const;
  string gen_bundler_call(bool send_routine,
                          const string &obj_name, const string &data_name,
                          const unsigned pointer_count) const;
  bool gen_bundler_body(bool send_routine,
                        const string &bundler_prefix, const string &class_prefix,
                        ofstream &out_stream) const;
  bool gen_bundler_sig(bool print_extern,
                       bool for_definition, // false --> just print the prototype
                       bool send_routine, // false --> receive routine
                       const string &class_prefix,
                       const string &bundler_prefix, ofstream &out_stream) const;
  bool gen_bundler_ptr(const string &class_prefix,
		       ofstream &out_c, ofstream &out_h) const;
  bool gen_class(const string bundler_prefix, ofstream &out_stream);

  string unqual_id() const { return (unqual_name_ + "_id");}
  string qual_id() const;
  const string &name() const { return name_;}
  const string &bundle_name() const { return bundle_name_;}
  type_type my_type() const { return my_type_;}
  bool is_in_library() const { return in_lib_;}
  bool operator== (const type_defn &other) const { return (other.name() == name_); }
  bool is_same_type(vector<arg*> *arglist) const;
  string dump_args(const string data_name, const string sep) const;
  void dump_type() const;
  const string &unqual_name() const { return unqual_name_;}
  bool is_stl() const { return is_stl_;}
  const string &prefix() const { return prefix_;}
  bool assign_to(const string prefix, const vector<arg*> &alist, ofstream &out_stream) const;
  bool pointer_used() const { return pointer_used_;}
  void set_pointer_used() { pointer_used_ = true;}
  bool can_point() const { return can_point_;}
  const vector<arg*> &copy_args() const { return (arglist_);}
  unsigned numFields() const { return arglist_.size(); }

  string ignore() const { return ignore_; }
  bool is_class() const { return is_class_; }
  bool is_abstract() const { return is_abstract_; }
  bool is_derived() const { return is_derived_; }
  string parent() const { return parent_; }
  void add_kid(const string kid_name);
  bool has_kids() const { return (kids_.size() > 0); }


private:
  type_type my_type_;
  string name_;
  string bundle_name_;
  bool in_lib_;
  vector<arg*> arglist_;
  string unqual_name_;
  bool is_stl_;
  string prefix_;
  bool pointer_used_;
  bool can_point_;
  arg *stl_arg_;
  string ignore_;
  bool is_class_;
  bool is_abstract_;
  bool is_derived_;
  string parent_;
  vector<string> kids_;

  bool gen_bundler_ptr_struct(const string class_prefix,
		       ofstream &out_c, ofstream &out_h) const;
  bool gen_bundler_ptr_class(const string class_prefix,
		       ofstream &out_c, ofstream &out_h) const;

  bool gen_bundler_body_class(bool send_routine,
			      const string &bundler_prefix,
			      const string &class_prefix, 
			      ofstream &out_stream) const;
  bool gen_bundler_body_struct_or_class(bool send_routine,
                                        const string &bundler_prefix,
                                        const string &class_prefix,
                                        ofstream &out_stream) const;
};

#endif
