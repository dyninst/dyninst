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

// type_defn.C

#include "type_defn.h"
#include "Options.h"

pdstring type_defn::qual_id() const { return (Options::type_prefix()+unqual_id());}

bool type_defn::assign_to(const pdstring prefix, const pdvector<arg*> &alist,
			  ofstream &out_stream) const
{
  assert (alist.size() == arglist_.size());
  for (unsigned i=0; i<alist.size(); i++) 
    out_stream << prefix << arglist_[i]->name() << " = " << alist[i]->name() << ";\n";
  return true;
}

bool type_defn::is_same_type(pdvector<arg*> *arglist) const {
  assert(arglist);
  if (arglist_.size() != arglist->size())
    return false;

  for (unsigned i=0; i<arglist_.size(); i++) {
    if (arglist_[i]->type() != (*arglist)[i]->type())
      return false;
  }
  return true;
}

pdstring type_defn::gen_bundler_name_stl(bool send_routine) const {
  return (prefix_ +
  	  Options::ml->bundler_prefix() + (send_routine ? "send_" : "recv_") +
  	  bundle_name_);
}

pdstring type_defn::gen_bundler_name(bool send_routine) const {
  return Options::ml->bundler_prefix() + // e.g. "P_xdr_"
         (send_routine ? "send" : "recv");
}

pdstring type_defn::gen_bundler_call(bool send_routine,
                                   const pdstring &obj, const pdstring &data,
				   const unsigned /*pointer_count*/) const
{
  return gen_bundler_name(send_routine) + "(" + obj + ", " + data + ")";
}

pdstring type_defn::dump_args(const pdstring data_name, const pdstring sep) const {
  pdstring ret("");

  if (my_type_ == type_defn::TYPE_SCALAR) {
    if (name_ != "void")
      ret = data_name;
  } else if (is_stl()) {
    ret = data_name;
  } else {
    switch (arglist_.size()) {
    case 0:
      break;
    case 1:
      ret = data_name;
      break;
    default:
      for (unsigned i=0; i<arglist_.size(); i++) {
	ret += pdstring(data_name + sep + arglist_[i]->name());
	if (i < (arglist_.size() - 1))
	  ret += pdstring(", ");
      }
      break;
    }
  }
  return ret;
}

type_defn::type_defn(pdstring stl_name, pdstring element_name, const unsigned ct,
		     bool in_lib)
: my_type_(type_defn::TYPE_COMPLEX), 
  in_lib_(in_lib), is_stl_(true),
  pointer_used_(false), can_point_(true), is_class_(false), is_derived_(false) 
{
  pdstring ptrs = Options::make_ptrs(ct);
  pdstring waste="dummy";
  name_ = stl_name + "<" + element_name + ptrs + ">";
  stl_arg_ = new arg(&element_name, ct, false, &waste, false);
  if (Options::all_types.defines(name_)) {
    cerr << name_ << " is already defined\n";
    exit(-1);
  }
  prefix_ = Options::type_class() + "_";
  unqual_name_ = name_;
  bundle_name_ = "stl";
}

static pdstring process_ignore(const pdstring txt) {
  if (txt.length() < 17) return "";
  char *buffer = strdup(txt.c_str());
  char *temp = buffer;

  temp[strlen(temp) - 8] = (char) 0;
  temp += 8;
  pdstring s = temp;
  free(buffer);
  return s;
}

type_defn::type_defn(const pdstring &name, bool is_cl, bool is_abs,
		     bool is_der,
             bool is_virt,
             const pdstring &par, 
		     const type_type type, pdvector<arg*> *arglist, 
		     const bool can_point, const bool in_lib,
		     const pdstring &ignore, const pdstring &bundle_name)
: my_type_(type), bundle_name_(bundle_name), in_lib_(in_lib),
  is_stl_(false), pointer_used_(false), can_point_(can_point),
  ignore_(process_ignore(ignore)), is_class_(is_cl), 
  is_abstract_(is_abs), is_derived_(is_der),
  is_virtual_(is_virt)
{
  stl_arg_ = NULL;
  if (Options::all_types.defines(name)) {
    cerr << name << " is already defined\n";
    exit(-1);
  }

  if (in_lib || Options::ml->AS_one || Options::shortnames()) {
    name_ = name;
    prefix_ = "";
  } else {
    name_ = Options::type_prefix() + name;
    prefix_ = Options::type_prefix();
  }
  unqual_name_ = name;

  if (!bundle_name_.length())
    bundle_name_ = unqual_name_;
  
  if (arglist) {
    for (unsigned i=0; i< arglist->size(); i++)
      arglist_ += (*arglist)[i];
  }
  if (is_derived_) {
    parent_ = Options::type_prefix() + par;
    if (!Options::all_types.defines(parent_)) {
      cerr << "Parent " << par << " not defined\n";
      exit(0);
    } else {
      type_defn *p = Options::all_types[parent_];
      assert(p);
      p->add_kid(name_);
    }
  }
}

type_defn::type_defn(const pdstring &name, bool is_class, type_type type, 
		     const bool can_point, const bool in_lib) :
   my_type_(type), in_lib_(in_lib), is_stl_(false), pointer_used_(false),
   can_point_(can_point), is_class_(is_class), 
   is_abstract_(false), is_derived_(false)
{
   stl_arg_ = NULL;
   if (Options::all_types.defines(name)) {
      cerr << name << " is already defined\n";
      exit(-1);
   }

   if (in_lib || Options::ml->AS_one || Options::shortnames()) {
     name_ = name;
     prefix_ = "";
   } else {
     name_ = Options::type_prefix() + name;
     prefix_ = Options::type_prefix();
   }
   unqual_name_ = name;

   if (!bundle_name_.length())
      bundle_name_ = unqual_name_;
}

void type_defn::add_kid(const pdstring kid_name) { kids_ += kid_name; }

/* Convert a type name with qualified names to a names with unqualified names.
   E.g. vector<T_dyninstRPC::mdl_expr*> to vector<mdl_expr*>
   This is needed to compile the igen output with Visual C++, which
   does not accept the qualified names (in the example above, it complains
   that T_dyninstRPC is undefined).
*/
pdstring unqual_type(const pdstring &type) { // also referenced by interface_spec.C
  pdstring ret;
  const char *t = type.c_str();
  char *p;

  while (1) {
    p = strstr(t, Options::type_prefix().c_str());
    if (!p) {
      ret += t;
      return ret;
    }
    ret += pdstring(t, p-t);
    t = p + Options::type_prefix().length();
  }
}


bool type_defn::gen_class(const pdstring, ofstream &out_stream) {
   // We no longer emit fwd declarations; it causes problems if it's templated.
   if ( (numFields() == 0) && (!is_abstract() || is_stl()) ) {
      //cout << "Not emitting anything for " << unqual_name() << " since no fields"
      //     << " and not abstract class" << endl;
      return true;
   }
 
   if (is_class())
      out_stream << "class "; 
   else
      out_stream << "struct ";
   out_stream << unqual_name();
   
   if (is_derived()) 
   {
      out_stream << " :";
      if( is_virtual() )
      {
         out_stream << " virtual";
      }
      out_stream << " public " << Options::qual_to_unqual(parent()) << " ";
   }
   out_stream << "{ \n";
   if (is_class()) {
      out_stream << " public: \n ";
      out_stream << unqual_name() << "();\n";
   }
  
   for (unsigned i=0; i<arglist_.size(); i++)
     if (Options::ml->address_space() == message_layer::AS_one )
       out_stream << unqual_type(arglist_[i]->type(true)) << " " << arglist_[i]->name() << ";\n";
     else
       out_stream << unqual_type(arglist_[i]->type(false)) << " " << arglist_[i]->name() << ";\n"; // false --> don't use const, causes havoc for bundler recv routines

   if (ignore_.length())
      out_stream << "\n" << ignore_ << endl;

   if (is_class()) {
      out_stream << "public:\n";
      if (has_kids())
	out_stream << "virtual ";
      out_stream << Options::ml->bundler_return_type() << " marshall "
                 << "(" << Options::ml->marshall_obj()
                 << " " << Options::ml->marshall_obj_ptr() << " obj);\n";
      if (has_kids())
	out_stream << "virtual ";
      if( ! Options::shortnames() )
	out_stream << "class_ids getId() const { return " << unqual_id() << ";}\n";
      else
	out_stream << Options::type_prefix() << "class_ids getId() const { return " 
		   << Options::type_prefix() << unqual_id() << ";}\n";
   }

   out_stream << "};" << endl;

   if (!is_class()) 
      out_stream << "typedef struct " << unqual_name() << " " << unqual_name()
                 << ";" << endl;
   return true;
}

bool type_defn::gen_bundler_ptr(const pdstring &class_prefix,
				ofstream &out_c, ofstream &out_h) const 
{
  bool do_it = false;
  if (pointer_used())
    do_it = true;
  else if (is_class()) {
    const type_defn *curr_defn = this;
    while (curr_defn->is_derived()) {
      if (!Options::all_types.defines(curr_defn->parent())) {
	cerr << "Parent " << curr_defn->parent() << " should be defined\n";
	assert(0);
      }
      curr_defn = Options::all_types[curr_defn->parent()];
      if (curr_defn->pointer_used()) {
	do_it = true;
	break;
      }
    }
  }

  if (!do_it)
    return true;

  if (is_class()) 
    return (gen_bundler_ptr_class(class_prefix, out_c, out_h));
  else
    return (gen_bundler_ptr_struct(class_prefix, out_c, out_h));
}

bool type_defn::gen_bundler_ptr_struct(const pdstring /*class_prefix*/, 
				       ofstream &out_c, ofstream &out_h) const 
{
  // generate bundler send(XDR*, <type>*) declaration
  out_h << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(true) << "(" << Options::ml->marshall_obj()
	<< Options::ml->marshall_obj_ptr() << ", " << unqual_name() 
	<< Options::ml->marshall_data_ptr() << ");\n";

  // generate bundler recv(XDR*, <type>*&) declaration
  out_h << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(false) << "(" << Options::ml->marshall_obj()
	<< Options::ml->marshall_obj_ptr() << ", " << unqual_name() 
	<< Options::ml->marshall_data_ptr() << "&);\n";

  // generate bundler send(XDR*, <type>*) implementation
  out_c << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(true) << "(" << Options::ml->marshall_obj()
	<< " " << Options::ml->marshall_obj_ptr() << "obj, " << name() 
	<< " " << Options::ml->marshall_data_ptr() << "data) {\n";
  
  out_c << "  assert(obj);\n";
  out_c << "  assert(" << Options::obj_ptr() << " == " 
	<< Options::ml->pack_const() << ");\n";
  out_c << "  bool is_null(false);\n";
  out_c << "  if (!data) is_null=true;\n";
  out_c << "  if (!" 
	<< Options::all_types["bool"]->gen_bundler_call(true, "obj", "is_null", 0)
	<< ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  if (!data) return " << Options::ml->bundle_ok() << ";\n";
  out_c << "  return " << gen_bundler_call(true, "obj", "*data", 0) << ";\n";
  out_c << "}\n";

  // generate bundler recv(XDR*, <type>*&) implementation
  out_c << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(false) << "(" << Options::ml->marshall_obj()
	<< " " << Options::ml->marshall_obj_ptr() << "obj, " << name() 
	<< " " << Options::ml->marshall_data_ptr() << "&data) {\n";
  out_c << "  assert(obj);\n";
  out_c << "  assert(" << Options::obj_ptr() << " == " 
	<< Options::ml->unpack_const() << ");\n";
  out_c << "  bool is_null(false);\n";
  out_c << "  if (!" 
	<< Options::all_types["bool"]->gen_bundler_call(false, "obj", "is_null", 0)
	<< ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  if (is_null) { data = NULL; return " 
	<< Options::ml->bundle_ok() << ";}\n";
  out_c << "  data = new " << name() << ";\n";
  out_c << "  if (!data) return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  return " << gen_bundler_call(false, "obj", "*data", 0) << ";\n";
  out_c << "}\n";

  return true;
}

bool type_defn::gen_bundler_ptr_class(const pdstring class_prefix, 
       				      ofstream &out_c, ofstream &out_h) const 
{
  // generate bundler send(XDR*, <type>*) declaration
  out_h << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(true) << "(" 
	<< Options::ml->marshall_obj() << " " << Options::ml->marshall_obj_ptr() 
	<< "obj, " << class_prefix << unqual_name() << " " 
	<< Options::ml->marshall_data_ptr() << "data);\n";

  // generate bundler recv(XDR*, <type>*&) declaration 
  out_h << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(false) << "(" 
	<< Options::ml->marshall_obj() << " " << Options::ml->marshall_obj_ptr() 
	<< "obj, " << class_prefix << unqual_name() << " " 
	<< Options::ml->marshall_data_ptr() << "&data);\n";

  // generate bundler send(XDR*, <type>*) implementation
  out_c << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(true) << "("
	<< Options::ml->marshall_obj() << " " << Options::ml->marshall_obj_ptr()
	<< "obj, " << class_prefix << unqual_name() << " " 
	<< Options::ml->marshall_data_ptr() << "data) {\n";
  out_c << "  assert(obj);\n";
  out_c << "  assert(" << Options::obj_ptr() << " == " << Options::ml->pack_const()
    << ");\n";
  out_c << "  bool is_null = (data == NULL);\n";
  out_c << "  unsigned tag;\n";
  out_c << "  if (!data) tag = " << qual_id() << ";\n";
  out_c << "  else tag = (unsigned) data->getId();\n";
  out_c << "  if (!" << Options::all_types["bool"]->gen_bundler_call(true, "obj", 
								     "is_null", 0)
	<< ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  if (!" << Options::all_types["u_int"]->gen_bundler_call(true, "obj",
								      "tag", 0)
	<< ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  if (!data) return " << Options::ml->bundle_ok() << ";\n";
  out_c << "  return data->marshall(obj);\n";
  out_c << "}\n";

  // generate bundler recv(XDR*, <type>*&) implementation
  out_c << Options::ml->bundler_return_type() << " "
	<< gen_bundler_name(false) << "("
	<< Options::ml->marshall_obj() << " " << Options::ml->marshall_obj_ptr()
	<< "obj, " << class_prefix << unqual_name() << " " 
	<< Options::ml->marshall_data_ptr() << "&data) {\n";
  out_c << "  assert(obj);\n";
  out_c << "  assert(" << Options::obj_ptr() << " == " << Options::ml->unpack_const()
    << ");\n";
  out_c << "  bool is_null = (data == NULL);\n";
  out_c << "  unsigned tag;\n";
  out_c << "  if (!" << Options::all_types["bool"]->gen_bundler_call(false, "obj", 
								     "is_null", 0)
	<< ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  if (!" << Options::all_types["u_int"]->gen_bundler_call(false, "obj",
								      "tag", 0)
	<< ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  if (is_null) { data = NULL; return " << Options::ml->bundle_ok() << ";}\n";
  out_c << "  switch (tag) {\n";
  if (!is_abstract()) {
    out_c << "    case " << qual_id() << ":\n";
    out_c << "      data = new " << name() << ";\n";
    out_c << "      break;\n";
  }
  
  recursive_dump_kids(this, out_c);

  out_c << "    default: \n      return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  };\n";
  out_c << "  if (!data) return " << Options::ml->bundle_fail() << ";\n";
  out_c << "  return (data->marshall(obj));\n";
  out_c << "}\n";

  // generate <type>::marshall(XDR*) implementation
  out_c << Options::ml->bundler_return_type() << " " << name() 
	<< "::marshall(" << Options::ml->marshall_obj()
	<< " " << Options::ml->marshall_obj_ptr();

  if ( !is_abstract() ) {
    out_c << "obj) {\n";
    out_c << "  if (" << Options::obj_ptr() << " == " << Options::ml->pack_const()
	  << ") {\n";
    for (unsigned i=0; i<arglist_.size(); i++) {
      out_c << "    if (!";
      arglist_[i]->gen_bundler(true, out_c, "obj", "");
      out_c << ")\n";
      out_c << "      return " << Options::ml->bundle_fail() << ";\n";
    }
    const type_defn *curr_defn = this;
    while (curr_defn->is_derived()) {
      if (!Options::all_types.defines(curr_defn->parent())) {
	cerr << "Parent " << curr_defn->parent() << " should be defined\n";
	assert(0);
      }
      curr_defn = Options::all_types[curr_defn->parent()];
      assert(curr_defn);
      for (unsigned ui=0; ui<curr_defn->arglist_.size(); ui++) {
	out_c << "    if (!";
	arglist_[ui]->gen_bundler(true, out_c, "obj", "");
	out_c << ")\n";
	out_c << "      return " << Options::ml->bundle_fail() << ";\n";
      }
    }
    out_c << "  } else {\n";
    for (int j=0; j<arglist_.size(); j++) {
      out_c << "    if (!";
      arglist_[j]->gen_bundler(false, out_c, "obj", "");
      out_c << ")\n";
      out_c << "      return " << Options::ml->bundle_fail() << ";\n";
    }
    curr_defn = this;
    while (curr_defn->is_derived()) {
      if (!Options::all_types.defines(curr_defn->parent())) {
	cerr << "Parent " << curr_defn->parent() << " should be defined\n";
	assert(0);
      }
      curr_defn = Options::all_types[curr_defn->parent()];
      assert(curr_defn);
      for (unsigned ui=0; ui<curr_defn->arglist_.size(); ui++) {
	out_c << "    if (!";
	arglist_[ui]->gen_bundler(false, out_c, "obj", "");
	out_c << ")\n";
	out_c << "      return " << Options::ml->bundle_fail() << ";\n";
      }
    }
    out_c << "  }\n";
  } else {
    out_c << ") {\n";
  }
  out_c << "  return " << Options::ml->bundle_ok() << " ;\n";
  out_c << "}\n";
  return true;
}

bool type_defn::gen_bundler_body(bool send_routine,
                                 const pdstring &bundler_prefix,
                                 const pdstring &class_prefix,
				 ofstream &out_stream) const
{
   // structs and classes now emit the same bundler code:
   return gen_bundler_body_struct_or_class(send_routine,
                                           bundler_prefix, class_prefix, out_stream);
}
				 
bool type_defn::gen_bundler_body_struct_or_class(bool send_routine,
                                                 const pdstring &bundler_prefix,
                                                 const pdstring &class_prefix,
                                                 ofstream &out_stream) const
{
   gen_bundler_sig(false, // print_extern
                   true, // definition, not just declaration
                   send_routine, // send routine
                   class_prefix, bundler_prefix, out_stream);

   out_stream << "   assert(obj);\n";
   if (send_routine)
      out_stream << "   assert(obj->x_op == XDR_ENCODE);" << endl;
   else
      out_stream << "   assert(obj->x_op == XDR_DECODE);" << endl;

   for (unsigned i=0; i<arglist_.size(); i++) {
      out_stream << "   if (!";
      arglist_[i]->gen_bundler(send_routine, out_stream, "obj", "data.");
      out_stream << ")\n";
      out_stream << "      return " << Options::ml->bundle_fail() << ";\n";
   }
  
   out_stream << "   return " << Options::ml->bundle_ok() << ";\n}\n";

   return true;
}

bool type_defn::gen_bundler_sig(bool /* print_extern */,
                                bool for_definition, // false --> just prototype
                                bool send_routine, // false --> receive routine
                                const pdstring &class_prefix,
				const pdstring &bundler_prefix,
				ofstream &out_stream) const
{
   out_stream << Options::ml->bundler_return_type()  // typically "bool"
              << ' '
              << bundler_prefix // e.g. "P_xdr_"
              << (send_routine ? "send" : "recv")
              << "("
              << Options::ml->marshall_obj() // e.g. "XDR"
              << Options::ml->marshall_obj_ptr() // e.g. "*"
              << (for_definition ? " obj" : "")
              << ", "
	      << (send_routine ? "const " : "")
     	      << (Options::shortnames() ? "" : class_prefix)
              << unqual_name() // class/struct/builtin-type name
     //<< (send_routine ? " " : " *")
              << (for_definition ? " &data" : "&")
              << ")";
   if (!for_definition)
      // just prototype --> semicolon needed
      out_stream << ";";
   else
      // actual function definition --> open braces needed
      out_stream << " {";

   if (!send_routine)
      out_stream << "\n // space for data (2nd param) will be allocated but uninitialized";
   
   out_stream << endl;

   return true;
}

void type_defn::dump_type() const {
  cerr << "Type " << name_ << "  in_lib=" << in_lib_ << endl;
  for (unsigned i=0; i<arglist_.size(); i++)
    cerr << "   " << arglist_[i]->type() << "    "  << arglist_[i]->name() << endl;
}

