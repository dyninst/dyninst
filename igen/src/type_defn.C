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
#include "arg.h"
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

bool type_defn::gen_bundler_call_mrnet(ofstream &out_stream,
				       bool send_routine,
				       const pdstring &pointer,
				       const pdstring &obj,
				       const pdstring &data,
				       const pdstring &ret_field,
				       const unsigned /*pointer_count*/) const
{
  pdstring argument_list;
  pdstring format_str;
  pdstring base_unpack = "\tint uret = Stream::unpack(recv_buffer,";
  pdstring vec_calc = "\n";
  pdstring vec_calc_format = "\n";
  pdstring vec_calc_finish = "\n";
  pdstring clean_up;
  pdstring temp;
  pdstring struc_name = "";
  pdstring received_field;  
  // bundle individual args

  pdstring convert = "";
  pdstring cast_for_convert = "";
  pdstring data_name;
  pdstring data_type;
  int level = 0;
  
  data_type = data;
  data_name = obj;
  
  out_stream << "\t//unpack type = "<< data_type << "  name = " <<pointer << data_name << endl;

  out_stream << std::flush;  
  temp = "";
  

  if(data_type.find("<") < data_type.length())
    {
      unsigned int start = data_type.find("<")+1;
      unsigned int end = data_type.find(">");
      temp = data_type.substr(start,end-start);
      
      pdstring original_type = temp;

      if(temp == "pdstring")
	{
	  temp = "char*";
	}
      else if(temp == "bool")
	{
	  cast_for_convert = "(const char)";
	}
      
      vec_calc_format += "\tunsigned vector_"+data_name+"_length = 0;\n//1\n";

      //vec_calc +="\tvector_"+data_name+"_length = "+data_name+".size();\n";
      
      format_str +="%"+ getFormatType("unsigned");
      
      argument_list += "vector_"+data_name+"_length, \n//300\n";
      
      vec_calc +="\tfor(unsigned int i = 0 ; i < vector_"+data_name+"_length; i++)\n\t{\n//FOR 1\n";
      //vec_calc +="\t"+original_type+" *"+data_name+"_temp = new "+original_type+";\n//400\n";
      level++;
      received_field = data_name+"_temp";

      //-------------------------------------
      
      if(temp.find(":") < temp.length() )
	{	      
	  if(!send_routine)
	    {
	      vec_calc +="\t"+temp+" *"+data_name+"_temp = new "+temp+";\n//400a\n";
	    }
	  struc_name = pointer+data_name;
	  for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
	    {
	      type_defn *td = Options::vec_types[u1];
	      
	      if(td->gen_bundler_body_mrnet(send_routine,temp,struc_name,&vec_calc,
					    &vec_calc_format,&vec_calc_finish,&format_str,
					    &argument_list,&clean_up,temp,received_field,&level,out_stream)== true)
		break;	      
	    }
	  vec_calc+="\t("+pointer+data_name+").push_back(*"+received_field+");\n//PB 1\n";
	  vec_calc +="\t}\n\n//401\n";
	}
      
      else
	{
	  
	  if(temp != "byteArray")
	    {
	      format_str += "%a"+getFormatType(original_type);//+getFormatType("unsigned");
	      argument_list += "\n\t\t\t\t array_"+data_name+", array_"+data_name+"_length\n\n//301\n\t\t\t\t";

	      if(send_routine)
		{
		  vec_calc_format +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"["+data_name+".size()+1];\n//A1\n";
		  clean_up +="\n\tdelete array_"+data_name+";\n";
		  vec_calc +="\tarray_"+data_name+"[i] = "+cast_for_convert+data_name+"[i]"+convert+";\n\t}\n\n//402\n";
		}
	      else
		{
		  if(temp == "bool")
		    {//
		      vec_calc_format +="\tchar* array_"+data_name+";\n//200\n";
		      vec_calc += "\t"+received_field+"->"+pointer+data_name+" = (bool)array_"+data_name+"[i];\n//403\n";
		    }
		  else
		    {
		      vec_calc_format += "\t"+temp+"* array_"+data_name+";\n//201\n";
		      vec_calc += "\t("+pointer+data_name+").push_back( array_"+data_name+"[i]);\n\t}\n//404\n";
		    }
		  vec_calc_format +=";\n\tu_int array_"+data_name+"_length;\n//202\n";
		}
	    }
	  else // this is a byteArray
	    {
	      
	      format_str += "%a"+getFormatType("char");//+"a"+getFormatType("unsigned");
	      
	      argument_list += "\n\t\t\t\t array_"+data_name+", array_"+data_name+"_length\n\n//302\n\t\t\t\t";
	      
	      if(send_routine)
		{
		  vec_calc_format +="\tchar** array_"+data_name+" = new "+ temp +"["+data_name+".size()];\n//A2\n";
		}
	      else
		{
		  vec_calc_format +="\tchar** array_"+data_name+";\n//203\n";
		}
	      if(send_routine)
		{
		  vec_calc_format +="\tunsigned* array_"+data_name+"_length = new "+ temp +"["+data_name+".size()];\n//A3\n";
		  clean_up +="\n\tdelete [] array_"+data_name+";\n";
		  clean_up +="\n\tdelete [] array_"+data_name+"_length;\n";
		}
	      else
		{
		  vec_calc_format +="\tunsigned* array_"+data_name+"_length;\n//204\n";
		}
	      
	      vec_calc +="\tarray_"+data_name+"[i] = (char*)"+data_name+"[i].getArray();\n\t}\n\n//405\n";
	      vec_calc +="\tarray_"+data_name+"_length[i] = "+data_name+"[i].length();\n\t}\n\n//406\n";
	      
	    }
	}
      
      //----------------------------
      
    }
  else
    {
      pdstring original_type = data_type;
      
      if(data_type == "pdstring")
	{
	  data_type = "char* ";
	  if(send_routine)
	    {
	      cast_for_convert = "(char*)";
	      convert = ".c_str()";
	    }
	}
      else if(data_type == "bool")
	{
	  vec_calc_format +="\tchar "+data_name+"_temp;\n//205\n";
	}
      
      temp = data_type;	    
      
      if(temp.find(":") < temp.length() )
	{
	  if(send_routine)
	    {
	      struc_name = data_name;
	    }
	  else
	    {
	      struc_name = pointer+data_name;
	    }

	  for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
	    {
	      type_defn *td = Options::vec_types[u1];
	      
	      if(td->gen_bundler_body_mrnet_noVec(send_routine,temp,struc_name,
						  &vec_calc,&vec_calc_format,&vec_calc_finish,
						  &format_str,&argument_list,&clean_up,temp,received_field,&level,out_stream)== true)
		break;
	      
	      
	    }
	}
      else
	{
	  
	  if(temp != "byteArray")
	    {
	      format_str +="%"+ getFormatType(original_type);
	      
	      if(data_type == "bool")
		{
		  argument_list += data_name+"_temp\n//303\n";
		  if(send_routine)
		    {
		      vec_calc += "\t"+data_name+" = (bool)"+data_name+"_temp;\n\n//407\n";
		    }
		  else
		    {
		      vec_calc += "\t"+pointer+data_name+" = (bool)"+data_name+"_temp;\n\n//408\n";
		    }
		}
	      else if((original_type == "pdstring") && (!send_routine))
		{

		  vec_calc_format +="\tchar* "+data_name+"_temp;\n//206\n";
		  argument_list += data_name+"_temp\n//304\n";
		  vec_calc += "\t"+pointer+data_name+" = "+data_name+"_temp;\n\n//409\n";
		}
	      else
		{
		  if(send_routine)
		    {
		      argument_list += cast_for_convert+data_name+convert;
		    }
		  else
		    {
		      argument_list += cast_for_convert+pointer+data_name+convert+"\n//305\n";
		    }
		}
	    }
	  else // this is a byteArray
	    {
	      format_str += "%a"+getFormatType("char");//+getFormatType("unsigned");
	      if(send_routine)
		{	      
		  argument_list += "\n\t\t\t\t(char*)"+data_name+".getArray(), "+data_name+".length()\n\n//306\n\t\t\t\t";
		}
	      else
		{
		  vec_calc_format += "\tchar*"+data_name+"_tempBA;\n//207\n";
		  vec_calc_format += "\tunsigned "+data_name+"_tempBA_length;\n//208\n";
		  argument_list += "\n\t\t\t\t"+data_name+"_tempBA, "+data_name+"_tempBA_length\n\n//307\n\t\t\t\t";
		  vec_calc_finish += data_name+" = byteArray((void*)("+data_name+"_tempBA, "+data_name+"_tempBA_length));\n";
		}
	    }
	  
	}
    }
  
  if(vec_calc_format.length() > 1)
    out_stream << vec_calc_format << endl << std::flush;

  out_stream << base_unpack << "\"" << format_str << "\"," << argument_list << ");" << endl;
  out_stream << std::flush;
  out_stream << "\tif( uret != 1 ) \n\t";
  out_stream << Options::error_state(true, 6, "igen_encode_err", ret_field); 
  out_stream << std::flush;
  

  if(vec_calc.length() > 1)
    out_stream << vec_calc << endl << std::flush;
  if(vec_calc_finish.length() > 1)
    out_stream << vec_calc_finish << endl << std::flush;


  //out_stream << clean_up << endl << std::flush;
  
  
  return true;
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


bool type_defn::gen_class(const pdstring, ofstream &out_stream) 
{
   // We no longer emit fwd declarations; it causes problems if it's templated.
  if ( (numFields() == 0) && (!is_abstract() || is_stl()) ) 
    {
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

  if (is_class())
    {
      out_stream << "public:\n";
      
      if (Options::ml->name() != "mrnet")
	{
	  if (has_kids())
	    out_stream << "virtual ";
	  
	  out_stream << Options::ml->bundler_return_type() << " marshall "
		     << "(" << Options::ml->marshall_obj()
		     << " " << Options::ml->marshall_obj_ptr() << " obj);\n";
	  
	}
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
    for (unsigned int j=0; j<arglist_.size(); j++) {
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


//---------------------------------------------------------------------------------------

bool type_defn::gen_bundler_body_mrnet(bool send_routine,pdstring type_to_do,
				       pdstring struc_arg,
				       pdstring * vec_calc,
				       pdstring * vec_calc_format,
				       pdstring * vec_calc_finish,
				       pdstring * format_str,
				       pdstring * argument_list,
				       pdstring * clean_up,
				       pdstring passed_structure,
				       pdstring received_field,
				       int * level,
				       ofstream &out_stream) const
{

  if(type_to_do != name_)
    return false;

  pdstring temp;

  unsigned first_st = 0;
  pdstring struc_ptr = "";
  if(struc_arg[0] == '*')
    {
    first_st++;
    struc_ptr = "*";
    }
  pdstring temp_struc_arg = struc_arg.substr(first_st,struc_arg.length());

  //out_stream << "body send_routine = "<< send_routine << endl;



  (*level)++;

  pdstring level_str = "i";

  for(int i = 0; i < *level ; i++)
    level_str += "i";

  for (unsigned i=0; i<arglist_.size(); i++)
    {
      pdstring data_name;
      pdstring data_type;

      pdstring convert = "";
      pdstring cast_for_convert = "";

      data_type =  arglist_[i]->type();
      data_name = arglist_[i]->name();

      out_stream << "\t//type = "<< data_type << "  name = " << data_name << endl;

      pdstring original_type = "";

      temp = "";
      
      if(data_type.find("<") < data_type.length())
	{
 	  unsigned int start = data_type.find("<")+1;
	  unsigned int end = data_type.find(">");
	  temp = data_type.substr(start,end-start);
	  original_type = temp;

	  pdstring length_fnc = ".size()";
	  pdstring array_fnc = "";
 	  
	  if(original_type == "pdstring")
	    {
	      if(send_routine)
		{
		  cast_for_convert = "(char*)";
		  convert = ".c_str()";
		}
	      temp = "char*";
	    }
	  else if(original_type == "byteArray")
	    {
	      temp = "char*";
	      cast_for_convert = "(char*)";
	      convert = ".getArray()";
	      pdstring length_fnc = ".length()";
	    }
	  else if(original_type == "bool")
	    {
	      cast_for_convert = "(char)";
	    }
	  else
	    {
	      cast_for_convert = "("+original_type+")";
	    }
	  
	  *format_str += "%"+getFormatType("unsigned");
	  
	  *argument_list += "vector_"+data_name+"_length, \n//308\n";

	  if(send_routine)
	    {

	      *vec_calc +="\tvector_"+data_name+"_length = (("+passed_structure+")"+struc_arg+"[i])."+data_name+".size();\n//410\n";

    
	      *vec_calc_format += "\tunsigned vector_"+data_name+"_length = 0;\n//2\n";

	      *vec_calc +="\n\tfor(unsigned int "+level_str+ " = 0 ; "+level_str+" < (("+
		passed_structure+")"+struc_arg+"[i])."+data_name+".size(); "+level_str+"++){\n";	  
	    }
	  else
	    {

	      //   fix
	      *vec_calc_format += "\tunsigned vector_"+data_name+"_length = 0;\n//111\n";

	      *vec_calc +="\tfor(unsigned int "+level_str+" = 0 ; "+level_str+" < vector_"+data_name+"_length; "+level_str+"++)\n\t{\n//FOR 2\n";
	      if(original_type == "pdstring")
		{
		  *vec_calc +="\tpdstring *"+data_name+"_temp = new pdstring(array_"+data_name+"["+level_str+"]);\n//C1\n";
		}
	      else
		{
		  *vec_calc +="\t"+original_type+" *"+data_name+"_temp = new "+original_type+";\n//411\n";
		}
	    }
	  
	  //-------------------------------------
	  
	  if(temp.find(":") < temp.length() )
	    {
	      for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
		{
		  type_defn *td = Options::vec_types[u1];

		  pdstring pass_name = struc_arg+"."+data_name;

		  if(td->gen_bundler_body_mrnet(send_routine,temp,pass_name,vec_calc,vec_calc_format,vec_calc_finish,
						format_str,argument_list,clean_up,temp,received_field,level,out_stream)== true)
		    {
		      break;
		    }
		}
	    }
	  
	  else
	    {

	      *format_str += "%a"+getFormatType(original_type);

	      switch(*level)
		{
		case 0:
		  if(send_routine)
		    {
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", "+struc_arg+length_fnc+"\n\t\t\t\t" ;
		    }
		  else
		    {
		      *vec_calc_format += "\tu_int array_"+data_name+"_length;\n\n//209\n";
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", array_"+data_name+"_length\n\n//309\n\t\t\t\t" ;
		    }
		  break;
		case 1:
		case 2:
		  if(send_routine)
		    {
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", vector_"+data_name+length_fnc+"\n\n//310\n\t\t\t\t" ;
		      *argument_list += ",\n//312\n";
		    }
		  else
		    {
		      //*vec_calc_format += "\tu_int array_"+data_name+"_length;\n\n//210\n";
		      //*argument_list += "\n\t\t\t\tarray_"+data_name+", array_"+data_name+"_length\n\n//311\n\t\t\t\t" ;
		    }
		  break;
		default:
		  ;
		}
	      if(send_routine)
		{
		  if(data_type.find("<") < data_type.length())
		    {
		      *vec_calc_format +="\t"+data_type+" vector_"+data_name+";\n//211\n";
		      *clean_up += "\n\tdelete &vector_"+data_name+";\n\n";
		    }
		  else
		    {
		      *vec_calc_format +="\t"+data_type+" *vector_"+data_name+" = new "+data_type+";\n//212\n";
		      *clean_up += "\n\tdelete vector_"+data_name+";\n\n";
		    }
		}
	      else
		{
		  *vec_calc_format +="\t"+data_type+" vector_"+data_name+ ";\n//213\n";
		}


	      if(send_routine)
		{
		  //*vec_calc_format +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"["+struc_arg+".size()];\n//A99\n";
		  *clean_up += "\n\tdelete [] array_"+data_name+";\n\n";
		}
	      else
		{

		  if(temp == "bool")
		    {
		      *vec_calc_format +="\tchar* array_"+data_name+";\n//214\n";
		    }
		  else
		    {
		      *vec_calc_format +="\t"+ temp+"* array_"+data_name+";\n//215\n";
		      *vec_calc_format +="\tunsigned array_"+data_name+"_length;\n//215a\n";
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", array_"+data_name+"_length,\n\n//313\n\t\t\t\t" ;
		    }
		  
		  //*vec_calc_format += "\tu_int array_"+data_name+"_length;\n"; $$$$
		  //*vec_calc_format +="\t"+ temp+"* vector_"+data_name+"_length;\n";
		}
	    }

	  //----------------------------
	  switch(*level)
	    {
	    case 0:
	      if(send_routine)
		{
		  *vec_calc +="\tarray_"+data_name+"[i] = "+struc_arg+"[i]."+data_name+";\n\t}\n\n//412\n";
		}
	      else
		{
		  //*vec_calc+="\t"+struc_arg+"."+data_name+".push_back(*"+*clean_up+");\n";
		  if(original_type == "bool")
		    {
		      *vec_calc += "\t"+received_field+"->"+data_name+" = array_"+data_name+"[i];\n\t}\n\n//413\n";
		    }
		  else
		    {
		      *vec_calc += "\t"+received_field+"->"+data_name+" = (bool)array_"+data_name+"[i];\n\t}\n\n//414\n";
		    }

		}
	      break;
	    case 1:
	    case 2:
	      if(send_routine)
		{
		  *vec_calc_format += "\n\tunsigned* "+struc_arg+"_"+data_name+"_vector= new unsigned["+struc_arg+".size()];\n//A100\n";
		  *clean_up += "\n\tdelete []"+struc_arg+"_"+data_name+"_vector;\n";
		  *argument_list += "\n\t\t\t\t"+struc_arg+"_"+data_name+"_vector, ("+struc_arg+").size()\n\n//314\n\t\t\t\t" ;
		  *vec_calc += "\n\t"+temp_struc_arg+"_"+data_name+"_vector["+level_str+"] = (("+passed_structure+struc_ptr+")"+temp_struc_arg+"["+level_str+"])."+data_name+".size();\n\n//415\n";
		  *vec_calc +="\tvector_"+data_name+".push_back((("+passed_structure+struc_ptr+")"+temp_struc_arg+"[i])."+data_name+"["+level_str+"]);\n\t}\n\n//PB 2 send\n";
		}
	      else
		{

		  //*vec_calc_format += "\n\tunsigned* "+temp_struc_arg+"_"+data_name+"_vector;\n//A100a\n";
		  *vec_calc_format += "\n\tunsigned* "+data_name+"_vector;\n//216\n";
		  *vec_calc_format += "\n\tunsigned "+data_name+"_vector_size;\n//217\n";
		  *argument_list += "\n\t\t\t\t"+data_name+"_vector, "+data_name+"_vector_size\n//AL 1\n\t\t\t\t" ;

		  *vec_calc +="\tvector_"+data_name+".push_back(*"+data_name+"_temp);\n\t}\n\n//PB 2 receive\n";
		}

	      *format_str += "%a"+getFormatType("unsigned");//+getFormatType("unsigned");

	      //*vec_calc_finish +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"[vector_"+data_name+".size()];\n\n//type_defn 1\n";

	      if(send_routine)
		{
		  *vec_calc_finish +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"[vector_"+data_name+".size()];\n\n//type_defn 1\n";
		  *clean_up += "\n\tdelete [] array_"+data_name+";\n";
		  //}
		  *vec_calc_finish +="\tfor(unsigned int  j = 0 ; j < vector_"+data_name+".size(); j++)\n\t{\n//FOR 3\n";	  

		  *vec_calc_finish += "\n\tarray_"+data_name+"[j] = "+cast_for_convert+"vector_"+data_name+"[j]"+convert+";\n\n\t}\n\n";	  
		}

	      break;
	    default:
	      ;
	    }
	}
      else
	{
	  temp = data_type;
	  if(data_type == "pdstring")
	    {
	      if(send_routine)
		{
		  cast_for_convert = "(char*)";
		  convert = ".c_str()";
		}
	      temp = "char*";
	    }
	  else if(data_type == "bool")
	    {
	      cast_for_convert = "(char)";
	    }


 	  if(temp.find(":") < temp.length() )
	    {
	      pdstring pass_name;

	      pass_name = struc_arg+"."+data_name;

	      for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
		{
		  type_defn *td = Options::vec_types[u1];
		  if(td->gen_bundler_body_mrnet(send_routine,temp,pass_name,vec_calc,vec_calc_format,vec_calc_finish,
						format_str,argument_list,clean_up,temp,received_field,level,out_stream)== true)
		    break;	      
		}
	    }
	  else
	    {
	      if(data_type != "byteArray")
		{
		  
		  *format_str += "%a"+getFormatType(temp);//+getFormatType("unsigned");
		  if(send_routine)
		    {
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", ("+struc_arg+").size()\n\n//315\n\t\t\t\t" ;
		      *vec_calc +="\tarray_"+data_name+"[i] = "+cast_for_convert+"("+struc_arg+"[i])."+data_name+convert+";\n\n";
		    }
		  else
		    {
		      *vec_calc_format += "\tu_int array_"+data_name+"_length;\n//218\n";
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", array_"+data_name+"_length\n\n//316\n\t\t\t\t" ;
		      if(temp == "bool")
			{
			  *vec_calc += "\t"+received_field+"->"+data_name+" = (bool)array_"+data_name+"[i];\n//415b\n";
			}
		      else
			{
			  *vec_calc += "\t"+received_field+"->"+data_name+" = array_"+data_name+"[i];\n//416 "+level_str+"\n";
			}

		    }

		  if(send_routine)
		    {
		     *vec_calc_format +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"[("+struc_arg+").size()];\n//A5\n";
		    }
		  else
		    {
		      if(temp == "bool")
			{
			  *vec_calc_format +="\tchar* array_"+data_name+";\n//219\n";
			}
		      else
			{
			  *vec_calc_format +="\t"+ temp+"* array_"+data_name+";\n//220\n";
			}
		    }
		  if(send_routine)
		    {
		      *clean_up += "\n\tdelete [] array_"+data_name+";\n\n";
		    }
		}
	      else
		{//this is a byteArray

		  *format_str += "%a"+getFormatType("char");//+getFormatType("unsigned");
		  *format_str += "%a"+getFormatType("unsigned");//+getFormatType("unsigned");


		  if(send_routine)
		    {
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", sizeof( array_"+data_name+" )\n\n//317\n\t\t\t\t";
		      *argument_list += ", \n//318\n";
		      *argument_list += "\n\t\t\t\t"+data_name+"_length, ("+struc_arg+").size()\n\n//319\n\t\t\t\t" ;

		      *vec_calc_format +="\tchar** vector_"+data_name+" = new char*[("+struc_arg+").size()];\n//A101\n";
		      *vec_calc_format +="\tunsigned *"+data_name+"_length = new unsigned[("+struc_arg+").size()];\n//A110\n";
		      *vec_calc +="\tvector_"+data_name+"[i] = (char*)("+struc_arg+"[i])."+data_name+".getArray();\n\n";
		      *vec_calc +="\t"+data_name+"_length[i] = ("+struc_arg+"[i])."+data_name+".length();\n\n";


		      *vec_calc_finish +="\tchar* array_"+data_name+" = new char[sizeof(vector_"+data_name+")];\n//B1\n";
		      *vec_calc_finish +="\tchar* array_"+data_name+"_ptr = array_"+data_name+";\n//B2\n";

		      //*vec_calc_finish +="\tchar* array_"+data_name+" = temp_"+data_name+";\n";

		      *vec_calc_finish +="\tfor(unsigned int  j = 0 ; j < "+struc_arg+".size(); j++)\n\t{\n\n";	  

		      *vec_calc_finish +="\tfor(unsigned int  k = 0 ; k < "+data_name+"_length[j]; k++)\n\t{\n";	  
		      *vec_calc_finish += "\n\t*array_"+data_name+"_ptr= "+cast_for_convert+"vector_"+
			data_name+"[j][k]"+convert+";\n\n";

		      *vec_calc_finish += "\n\tarray_"+data_name+"_ptr++;\n\n\t}\n\t}\n";

		      *clean_up += "\n\tdelete [] array_"+data_name+";\n\n";
		      *clean_up += "\n\tdelete [] "+data_name+"_length;\n\n";
		      *clean_up += "\n\tdelete [] vector_"+data_name+";\n\n";
		    }
		  else
		    {
		      *vec_calc_format += "\tchar* array_"+data_name+";\n//221\n";
		      *vec_calc_format += "\tunsigned array_"+data_name+"_size;\n//222\n";
		      *argument_list += "\n\t\t\t\tarray_"+data_name+", array_"+data_name+"_size\n\n//320\n\t\t\t\t,";

		      *vec_calc_format += "\tunsigned array_"+data_name+"_length_size;\n//223\n";
		      *argument_list += "\n\t\t\t\t"+data_name+"_length, array_"+data_name+"_length_size\n\n//321\n\t\t\t\t" ;

		      *vec_calc_format +="\tchar** vector_"+data_name+";\n//224\n";
		      *vec_calc_format +="\tunsigned* "+data_name+"_length;\n//225\n";


		      *vec_calc +="\tbyteArray *tempByteArray"+data_name+" = new byteArray((void*)array_"+data_name+", "+data_name+"_length[i]);\n";

		      *vec_calc +="\t"+received_field+"->"+data_name+" = *tempByteArray"+data_name+";\n";

		      *vec_calc +="\tarray_"+data_name+" += "+data_name+"_length[i];\n";
		      

		    }
		}
	    }

	}
      if( i < arglist_.size() - 1 )
	{
	  //*format_str += " ";
	  *argument_list += ", \n//322\n";
	}
    }


  return true;
}
bool type_defn::gen_bundler_body_mrnet_noVec(bool send_routine,pdstring type_to_do,
				       pdstring struc_arg,
				       pdstring * vec_calc,
				       pdstring * vec_calc_format,
				       pdstring * vec_calc_finish,
				       pdstring * format_str,
				       pdstring * argument_list,
				       pdstring * clean_up,
				       pdstring passed_structure,
				       pdstring received_field,
				       int * level,
				       ofstream &out_stream) const
{

  if(type_to_do != name_)
    return false;

  //out_stream << "body send_routine = "<< send_routine << endl;


  pdstring temp;

  (*level)++;

  pdstring level_str = "";

  for(int i = 0; i < *level ; i++)
    level_str += "i";

  for (unsigned i=0; i<arglist_.size(); i++)
    {
      pdstring data_name;
      pdstring data_type;

      data_type =  arglist_[i]->type();
      data_name = arglist_[i]->name();

      out_stream << "\t//type = "<< data_type << "  name = " << data_name << endl;


      temp = "";
      
      if(data_type.find("<") < data_type.length())
	{
 	  unsigned int start = data_type.find("<")+1;
	  unsigned int end = data_type.find(">");
	  temp = data_type.substr(start,end-start);
	  
 	  pdstring original_type = temp;
	  if(temp == "pdstring")
	    temp = "char*";

      	  *vec_calc_format += "\tunsigned vector_"+data_name+"_length = 0;\n//3\n";

	  *format_str += "%"+getFormatType("unsigned");
	  
	  *argument_list += "vector_"+data_name+"_length, \n//323\n";
	  if(send_routine)
	    {
	      *vec_calc +="\tvector_"+data_name+"_length = ("+struc_arg+")."+data_name+".size();\n//alpha\n";
	      *vec_calc +="\tfor(unsigned int "+level_str+ " = 0 ; "+level_str+" < ("+struc_arg+")."+data_name+".size(); "+level_str+"++){\n\t\n//FOR 4\n";
	    }
	  else
	    {
	      *vec_calc +="\tfor(unsigned int i = 0 ; i < vector_"+data_name+"_length; i++)\n\t{\n//FOR 5\n";

	      if(original_type == "pdstring")
		{
		  *vec_calc +="\tpdstring *"+data_name+"_temp = new pdstring (array_"+data_name+"[i]);\n//C2\n";
		}
	      else
		{
		  *vec_calc +="\t"+temp+" *"+data_name+"_temp = new "+temp+";\n//420\n";
		}

	      received_field = data_name+"_temp";
	    }
	  
	  //-------------------------------------
	  
	  if(temp.find(":") < temp.length() )
	    {
	      for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
		{
		  type_defn *td = Options::vec_types[u1];

		  pdstring pass_name;

		  pass_name = struc_arg+"."+data_name;
		  
		  if(td->gen_bundler_body_mrnet(send_routine,temp,pass_name,vec_calc,vec_calc_format,vec_calc_finish,
						format_str,argument_list,clean_up,temp,received_field,level,out_stream)== true)
		    break;	  
		}
	    }
	  
	  else
	    {

	      *format_str += "%a"+getFormatType(original_type);//+getFormatType("unsigned");
	      if(send_routine)
		{
		  *argument_list += "\n\t\t\t\tarray_"+data_name+", ("+struc_arg+").size()\n\n//324\n\t\t\t\t" ;
		}
	      else
		{
		  *vec_calc_format +="\tu_int array_"+data_name+"_length;\n//226\n";
		  *argument_list += "\n\t\t\t\tarray_"+data_name+", array_"+data_name+ "_length\n\n//325\n\t\t\t\t" ;
		}
	  
	      if(send_routine)
		{
		  *vec_calc_format +="\t"+ temp+"* array_"+struc_arg+"_"+data_name+" = new "+ temp +"["+data_name+".size()+1];\n//A6\n";
		  *clean_up += "\tdelete [] array_"+struc_arg+"_"+data_name+";\n\n";
		}
	      else
		{
		  if(temp == "bool")
		    {
		      *vec_calc += "\t"+received_field+"->"+data_name+" = (bool)array_"+data_name+"[i];\n//421\n";
		      *vec_calc_format +="\tchar* array_"+struc_arg+"_"+data_name+";\n//227\n";
		    }
		  else
		    {
		      *vec_calc_format +="\t"+ temp+"* array_"+data_name+";\n//228\n";
		    }
		}
	      //*vec_calc_format +=";\n//229\n";
	    }

	  //----------------------------
	  if(!send_routine)
	    {
	      *vec_calc+="\t("+struc_arg+")."+data_name+".push_back(*"+received_field+");\n//PB 3\n";
	    }
	  *vec_calc +="\t}\n\n//423\n";

	}
      else
	{  
	  temp = data_type;
 	  pdstring original_type = temp;

	  if(temp == "pdstring")
	    {

	      temp = "char*";
	    }

	  if(!send_routine)
	    {
	    if(data_type == "bool")
	      {
		*vec_calc_format += "\n\tchar "+data_name+"_temp;\n//230\n";
		*argument_list += data_name+"_temp\n//326\n";
		*vec_calc += "\t("+struc_arg+")."+data_name+" = (bool)"+data_name+"_temp;\n\n//424\n";
	      }

	      else if((original_type == "pdstring") && (!send_routine))
		{

		  *vec_calc_format +="\tchar* "+data_name+"_temp;\n//231\n";
		  *argument_list += data_name+"_temp\n//327\n";
		  *vec_calc += "\t("+struc_arg+")."+data_name+" = "+data_name+"_temp;\n\n//425\n";
		}
	    else
	      {
		*argument_list += "("+struc_arg+")."+data_name+"\n//328 "+level_str+"\n";
	      }
	    }
	  else
	    {
	      if(data_type == "bool")
		{
		  *vec_calc_format += "\n\tchar "+data_name+"_temp;\n//230b\n";
		  *vec_calc_format += "\t"+data_name+"_temp = (char)("+struc_arg+")."+data_name+";\n\n//230c\n";
		  *argument_list += data_name+"_temp\n//326b\n";
		}
	      else
		{
		  *argument_list += "("+struc_arg+")."+data_name+"\n//329\n";
		}
	    }
	      
	  *format_str += "%"+getFormatType(data_type);

 
	  if(temp.find(":") < temp.length() )
	    {
	      pdstring pass_name;

	      pass_name = struc_arg+"."+data_name;

	      for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
		{
		  type_defn *td = Options::vec_types[u1];
		  if(td->gen_bundler_body_mrnet(send_routine,temp,pass_name,vec_calc,vec_calc_format,vec_calc_finish,
						format_str,argument_list,clean_up,temp,received_field,level,out_stream)== true)
		    break;	      
		}
	    }
	}
      if( i < arglist_.size() - 1 )
	{
	  *argument_list += ", \n//330\n";
	}
    }


  return true;
}

pdstring type_defn::getFormatType(pdstring old_type) const
{

  if((old_type == "unsigned short")||
   (old_type == "u_short"))
    {
      switch(sizeof(unsigned short))
	{
	case 1:
	  return "uc ";
	case 2:
	  return "uhd ";
	case 4:
	  return "ud ";
	case 8:
	  return "uld ";
	default:
	  ;
	}
    }
  else if(old_type == "short")
    {
      switch(sizeof(short))
	{
	case 1:
	  return "c ";
	case 2:
	  return "hd ";
	case 4:
	  return "d ";
	case 8:
	  return "ld ";
	default:
	  ;
	}
    }
  else if((old_type == "unsigned")||
	  (old_type == "unsigned int")||
	  (old_type == "u_int"))
    {
      switch(sizeof(unsigned int))
	{
	case 1:
	  return "uc ";
	case 2:
	  return "uhd ";
	case 4:
	  return "ud ";
	case 8:
	  return "uld ";
	default:
	  ;
	}
    }
  else if( old_type == "int")
    {
      switch(sizeof(int))
	{
	case 1:
	  return "c ";
	case 2:
	  return "hd ";
	case 4:
	  return "d ";
	case 8:
	  return "ld ";
	default:
	  ;
	}
    }
  else if((old_type == "unsigned long")||
	  (old_type == "u_long"))
    {
      switch(sizeof(long))
	{
	case 2:
	  return "uhd ";
	case 4:
	  return "ud ";
	case 8:
	  return "uld ";
	default:
	  ;
	}
    }
  else if(old_type == "long")
    {
      switch(sizeof(long))
	{
	case 2:
	  return "hd ";
	case 4:
	  return "d ";
	case 8:
	  return "ld ";
	default:
	  ;
	}
    }
  else if((old_type == "longlong_t")||
	  (old_type == "long long"))
    {
      switch(sizeof(long long))
	{
	case 2:
	  return "hd ";
	case 4:
	  return "d ";
	case 8:
	  return "ld ";
	default:
	  ;
	}
    }
  else if((old_type == "u_longlong")||
	  (old_type == "unsigned long long"))
    {
      switch(sizeof(long long))
	{
	case 2:
	  return "uhd ";
	case 4:
	  return "ud ";
	case 8:
	  return "uld ";
	default:
	  ;
	}
    }
  else if(old_type == "char")
    {
      return "c ";
    }
  else if((old_type == "u_char")||
	  (old_type == "unsigned char"))
    {
      return "uc ";
    }
  else if((old_type == "pdstring")||
	  (old_type == "char*"))
    {
      return "s ";
    }
  else if(old_type == "uint32_t")
    {
      return "ud ";
    }
  else if(old_type == "uint64_t")
    {
      return "uld ";
    }
  else if(old_type == "int32_t")
    {
      return "d ";
    }
  else if(old_type == "int64_t")
    {
      return "ld ";
    }
  else if(old_type == "float")
    {
      return "f ";
    }
  else if( old_type == "double")
    {
      return "lf ";
    }
  else if( old_type == "bool")
    {
      return "c ";
    }
  return "ERROR ("+ old_type +")";

}

