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

// type_defn.h
// Moved here from parse.h, into its own [.hC] file combination,
// for clarity

#ifndef _TYPE_DEFN_H_
#define _TYPE_DEFN_H_

#include "common/h/String.h"
#include <fstream>
#include "arg.h"
#include "common/h/Vector.h"

using std::ofstream;

class type_defn {
public:
  friend void recursive_dump_kids(const type_defn *from,
                                  ofstream &output);

  typedef enum { TYPE_SCALAR, TYPE_COMPLEX } type_type;

  type_defn(pdstring stl_name, pdstring element_name, const unsigned star_count,
            bool in_lib);
  type_defn(const pdstring &name, bool is_class, bool is_abstract,
            bool is_derived,
            bool is_virtual,
            const pdstring &parent, 
            const type_type type, pdvector<arg*> *arglist = NULL, 
            const bool can_point=false, const bool in_lib=false, 
	    const pdstring &ignore="", const pdstring &bundle_name="");
  type_defn(const pdstring &name, bool is_class, type_type type, 
	    const bool can_point=false, const bool in_lib=false);
  ~type_defn() { }

  pdstring gen_bundler_name_stl(bool send_routine) const;
  pdstring gen_bundler_name(bool send_routine) const;
  pdstring gen_bundler_call(const pdstring &obj_name, const pdstring &data_name,
                          const unsigned) const;
  pdstring gen_bundler_call(bool send_routine,
			    const pdstring &obj_name,
			    const pdstring &data_name,
			    const unsigned pointer_count) const;


  bool gen_bundler_call_mrnet(ofstream &out_stream,
			      bool send_routine,
			      const pdstring &pointer,
			      const pdstring &obj,
			      const pdstring &data,
			      const pdstring &ret_field,
			      const unsigned pointer_count) const;
    
  bool gen_bundler_body(bool send_routine,
                        const pdstring &bundler_prefix,
                        const pdstring &class_prefix,
                        ofstream &out_stream) const;
  bool gen_bundler_sig(bool print_extern,
                       bool for_definition, // false --> just print the prototype
                       bool send_routine, // false --> receive routine
                       const pdstring &class_prefix,
                       const pdstring &bundler_prefix,
                       ofstream &out_stream) const;
  //------------------------------------------------------------

  bool gen_bundler_body_mrnet_noVec(bool send_routine, pdstring type_to_do,
				    pdstring dot_data_name,
				    pdstring * vec_calc,
				    pdstring * vec_calc_format,
				    pdstring * vec_calc_finish,
				    pdstring * format_str,
				    pdstring * argument_list,
				    pdstring * clean_up,
				    pdstring passed_structure,
				    pdstring received_field,
				    int * level,
				    ofstream &out_stream) const;

  bool gen_bundler_body_mrnet(bool send_routine, pdstring type_to_do,
			      pdstring dot_data_name,
			      pdstring * vec_calc,
			      pdstring * vec_calc_format,
			      pdstring * vec_calc_finish,
			      pdstring * format_str,
			      pdstring * argument_list,
			      pdstring * clean_up,
			      pdstring passed_structure,
			      pdstring received_field,
			      int * level,
			      ofstream &out_stream) const;

  pdstring getFormatType(pdstring old_type)const;

  //------------------------------------------------------


 bool gen_bundler_ptr(const pdstring &class_prefix,
                       ofstream &out_c, ofstream &out_h) const;
  bool gen_class(const pdstring bundler_prefix, ofstream &out_stream);

  pdstring unqual_id() const { return (unqual_name_ + "_id");}
  pdstring qual_id() const;
  const pdstring &name() const { return name_;}
  const pdstring &bundle_name() const { return bundle_name_;}
  type_type my_type() const { return my_type_;}
  bool is_in_library() const { return in_lib_;}
  bool operator== (const type_defn &other) const { return (other.name() == name_); }
  bool is_same_type(pdvector<arg*> *arglist) const;
  pdstring dump_args(const pdstring data_name, const pdstring sep) const;
  void dump_type() const;
  const pdstring &unqual_name() const { return unqual_name_;}
  bool is_stl() const { return is_stl_;}
  const pdstring &prefix() const { return prefix_;}
  bool assign_to(const pdstring prefix, const pdvector<arg*> &alist,
                 ofstream &out_stream) const;
  bool pointer_used() const { return pointer_used_;}
  void set_pointer_used() { pointer_used_ = true;}
  bool can_point() const { return can_point_;}
  const pdvector<arg*> &copy_args() const { return (arglist_);}
  unsigned numFields() const { return arglist_.size(); }

  pdstring ignore() const { return ignore_; }
  bool is_class() const { return is_class_; }
  bool is_abstract() const { return is_abstract_; }
  bool is_derived() const { return is_derived_; }
  bool is_virtual() const { return is_virtual_; }
  pdstring parent() const { return parent_; }
  void add_kid(const pdstring kid_name);
  bool has_kids() const { return (kids_.size() > 0); }


private:
  type_type my_type_;
  pdstring name_;
  pdstring bundle_name_;
  bool in_lib_;
  pdvector<arg*> arglist_;
  pdstring unqual_name_;
  bool is_stl_;
  pdstring prefix_;
  bool pointer_used_;
  bool can_point_;
  arg *stl_arg_;
  pdstring ignore_;
  bool is_class_;
  bool is_abstract_;
  bool is_derived_;
  bool is_virtual_;
  pdstring parent_;
  pdvector<pdstring> kids_;

  bool gen_bundler_ptr_struct(const pdstring class_prefix,
                              ofstream &out_c,
                              ofstream &out_h) const;
  bool gen_bundler_ptr_class(const pdstring class_prefix,
                             ofstream &out_c, ofstream &out_h) const;

  bool gen_bundler_body_class(bool send_routine,
			      const pdstring &bundler_prefix,
			      const pdstring &class_prefix, 
			      ofstream &out_stream) const;
  bool gen_bundler_body_struct_or_class(bool send_routine,
                                        const pdstring &bundler_prefix,
                                        const pdstring &class_prefix,
                                        ofstream &out_stream) const;
};

#endif
