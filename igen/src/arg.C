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

// arg.h
// Put in by Ariel Tamches; moved from main.C into its own .h/.C file combo
// for clarity.

#include "arg.h"
#include "Options.h"
#include "type_defn.h"

pdstring arg::deref(const bool use_ref) const {
  if (stars_)
    return pointers_;
  else if (is_ref_ && use_ref)
    return "&";
  else
    return "";
}

pdstring arg::type(const bool use_const, const bool local) const {
  if (use_const)
    return ((constant_ ? pdstring("const ") : pdstring("")) + type_ + deref(local));
  else
    return (type_ + deref(local));
}

pdstring arg::gen_bundler_name(bool send_routine) const {
  pdstring suffix;
  switch (stars_) {
  case 0: break;
  case 1: suffix = "_PTR"; break;
  default: abort();
  }
  return ((Options::all_types[base_type()])->gen_bundler_name(send_routine) + suffix);
}


void arg::gen_bundler(bool send_routine,
                      ofstream &out_stream, const pdstring &obj_name,
  		      const pdstring &data_name) const
{
   out_stream <<
      (Options::all_types[base_type()])->gen_bundler_call(send_routine,
                                                          obj_name,
                                                          data_name+name_, stars_);
}

pdstring arg::gen_bundler_type(pdstring * data_name)
{
  *data_name = name_;
   return type_;
}

arg::arg(const pdstring *type, const unsigned star_count,
	 const bool b, const pdstring *name, const bool is_ref) 
: pointers_(Options::make_ptrs(star_count)), type_(*type),
  constant_(b), stars_(star_count), is_ref_(is_ref) {

  if(is_ref_ && star_count) {
    cerr << "Cannot use pointers with a reference, good bye\n";
    exit(0);
  }

  if (name)
    name_ = *name;
  else
    name_ = Options::gen_name();

  if (!(Options::all_types[type_])->can_point() &&
      (star_count>1) &&
      (Options::ml->address_space() == message_layer::AS_many)) {
    cerr << "Sorry, pointers not handled for this type, goodbye\n";
    cerr << "Type = " << *type << "  name = " << *name << endl;
    exit(-1);
  }
  if (star_count)
    (Options::all_types[type_])->set_pointer_used();
}

bool arg::tag_bundle_send(ofstream &out_stream, const pdstring bundle_val,
			  const pdstring tag_val, const pdstring return_value) const
{
  if (Options::ml->address_space() == message_layer::AS_many)
    return (tag_bundle_send_many(out_stream, bundle_val,
				 tag_val, return_value));
  else
    return (tag_bundle_send_one(out_stream, bundle_val,
				tag_val, return_value));
}

bool arg::tag_bundle_send_many(ofstream &out_stream, const pdstring bundle_val,
			       const pdstring tag_val, const pdstring return_value) const
{

  if (Options::ml->name() != "mrnet")
    {
      // set direction encode
      out_stream << Options::set_dir_encode() << ";\n";
      
      // bundle individual args
      out_stream << "if (!" << Options::ml->send_tag("net_obj()", tag_val) << "\n";
      
      if (type_ != "void") {
	out_stream << " || !" <<
	  (Options::all_types[type_])->gen_bundler_call(true, // send routine
							"net_obj()",
							//pdstring("&")+bundle_val,
							bundle_val,
							stars_)
		   << "\n";
      }
      
      out_stream << ") ";
      out_stream << Options::error_state(true, 6, "igen_encode_err", return_value);
      
      // send message
      out_stream << "if (!" << Options::ml->send_message() << ") ";
      out_stream << Options::error_state(true, 6, "igen_send_err", return_value);
      
      return true;
    }
  else
    {
      pdstring base_send = "\n//tag_bundle_send_many\n\tint sret = stream->send( "+tag_val+",";

      pdstring format_str = "";
      pdstring argument_list = "0";

      if (type_ != "void") 
	{

	  if(type_ == "pdstring")
	    {
	      out_stream << "\tchar * pass_"+bundle_val+" = (char*)"+bundle_val+".c_str();\n";
	      format_str = getFormatType(type_);
	      argument_list = "pass_"+bundle_val ;
	    }
	  else if(type_ == "bool")
	    {
	      out_stream << "\tchar pass_"+bundle_val+" = (char)"+bundle_val+";\n";
	      format_str = getFormatType("char");
	      argument_list = "pass_"+bundle_val;
	    }
	  else
	    {
	      format_str = getFormatType(type_);
	      argument_list = bundle_val;
	    }
	}
      out_stream << base_send << "\"" << format_str << "\"," << argument_list << ");" << endl;
      out_stream << std::flush;

      out_stream << "\tint fret = -1;" << endl;
      out_stream << "\tif( sret != -1 ) {" << endl;
      out_stream << "\t  fret = stream->flush();\n\t}" << endl;

      out_stream << "   if( (sret == -1) || (fret == -1) ) ";
      out_stream << Options::error_state(true, 6, "igen_send_err", return_value);

      out_stream << std::flush;
      return true;
    }
  return false;
}

bool arg::tag_bundle_send_one(ofstream &, const pdstring,
                              const pdstring, const pdstring) const 
{
  abort();
  return false;
}

//---------------------------------------------------------------------------------------------
bool arg::tag_bundle_send_mrnet(ofstream &out_stream, const pdstring &return_value,
				const pdstring &req_tag,const pdstring &fnc_name) const
{
    return (tag_bundle_send_many_mrnet(out_stream, return_value, req_tag, fnc_name));
}

bool arg::tag_bundle_send_many_mrnet(ofstream &out_stream, const pdstring &return_value,
				     const pdstring &rq_tag,const pdstring &fnc_name) const
{

  pdstring argument_list;
  pdstring format_str;
  pdstring base_send = "\n//arg 1\n\tint sret = stream->send( tag,";
  pdstring vec_calc = "\n";
  pdstring vec_calc_format = "\n";
  pdstring vec_calc_finish = "\n";
  pdstring clean_up;
  pdstring temp;
  pdstring struc_name = "";
  pdstring received_field;
  if(type_ == "void")
    {
      format_str = "";
      argument_list = "0";
    }
  else
    {
      // bundle individual args

      pdstring convert = "";
      pdstring cast_for_convert = "";
      pdstring data_name;
      pdstring data_type;
      int level = 0;
      
      data_type = type_;
      data_name = return_value;      
      out_stream << "\t//type = "<< data_type << "  name = " << data_name <<"\n"<< endl;

      //cout << "arg" <<"\t//type = "<< data_type << "  name = " << data_name <<"\n"<< endl;
      //cout <<std::flush;
      
      temp = "";
      
      if(data_type.find("<") < data_type.length())
	{
	  unsigned int start = data_type.find("<")+1;
	  unsigned int end = data_type.find(">");
	  temp = data_type.substr(start,end-start);
	  
	  pdstring original_type = temp;
	  
	  if(temp == "pdstring")
	    {
	      convert = ".c_str()";
	      cast_for_convert = "(char*)";
	      temp = "char*";
	    }
	  else if(temp == "bool")
	    {
	      cast_for_convert = "(char)";
	    }
	  
	  vec_calc_format += "\tunsigned vector_"+data_name+"_length = 0;\n//4\n";
	  
	  vec_calc +="\tvector_"+data_name+"_length = "+data_name+".size();\n";
	  
	  format_str += "%"+getFormatType("unsigned");
	  
	  argument_list += "vector_"+data_name+"_length, ";
	  
	  vec_calc +="\n\tfor(unsigned int i = 0 ; i < "+data_name+".size(); i++){\n\n//Arg pdvector\n\n";
	  
	  
	  //-------------------------------------
	  
	  if(temp.find(":") < temp.length() )
	    {	      
	      struc_name = data_name;
	      for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
		{
		  type_defn *td = Options::vec_types[u1];
		  
		  if(td->gen_bundler_body_mrnet(true,temp,struc_name,&vec_calc,
						&vec_calc_format,&vec_calc_finish,&format_str,
						&argument_list,&clean_up,temp,received_field,&level,out_stream)== true)
		    break;
		  
		  out_stream << std::flush;
		  
		}
	      vec_calc +="\t}\n\n";
		}
	  
	  else
	    {
	      
	      if(temp != "byteArray")
		{
		  
		  //format_str += temp+"* unsigned ";
		  
		  format_str += "%a"+getFormatType(original_type);//+getFormatType("unsigned");
		  
		  argument_list += "\n\t\t\t\t array_"+data_name+", "+data_name+".size()\n\t\t\t\t";
		  
		  vec_calc_format +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"["+data_name+".size()+1];\n//arg 1\n";
		  
		  clean_up +="\n\tdelete array_"+data_name+";\n";
		  
		  vec_calc +="\tarray_"+data_name+"[i] = "+cast_for_convert+data_name+"[i]"+convert+";\n\t}\n\n";
		}
	      else // this is a byteArray
		{
		  
		  //format_str += " char** unsigned* ";
		  format_str += "%a"+getFormatType("char");//+"a"+getFormatType("unsigned");

		  argument_list += "\n\t\t\t\t array_"+data_name+", array_"+data_name+"_length\n\t\t\t\t";
		  
		  vec_calc_format +="\tchar** array_"+data_name+" = new "+ temp +"["+data_name+".size()];\n//arg 2\n";
		  vec_calc_format +="\tunsigned* array_"+data_name+"_length = new "+ temp +"["+data_name+".size()];\n//arg 3\n";
		  
		  clean_up +="\n\tdelete [] array_"+data_name+";\n";
		  clean_up +="\n\tdelete [] array_"+data_name+"_length;\n";
		  
		  vec_calc +="\tarray_"+data_name+"[i] = (char*)"+data_name+"[i].getArray();\n\t}\n\n";
		  vec_calc +="\tarray_"+data_name+"_length[i] = "+data_name+"[i].length();\n\t}\n\n";
		  
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
	      cast_for_convert = "(char*)";
	      convert = ".c_str()";
	    }
	  else if(data_type == "bool")
	    {
	      cast_for_convert = "(char)";
	    }
	  
	  temp = data_type;	    
	  
	  if(temp.find(":") < temp.length() )
	    {
	      struc_name = data_name;		  
	      for (unsigned u1=0; u1<Options::vec_types.size(); u1++)
		{
		  type_defn *td = Options::vec_types[u1];
		  
		  if(td->gen_bundler_body_mrnet_noVec(true,temp,struc_name,
						      &vec_calc,&vec_calc_format,&vec_calc_finish,
						      &format_str,&argument_list,&clean_up,temp,received_field,&level,out_stream)== true)
		    break;
		  
		  out_stream << std::flush;
		  
		}
	    }
	  else
	    {
	      
	      if(temp != "byteArray")
		{
		  //format_str += data_type;
		  
		  format_str += "%"+getFormatType(original_type);
		  
		  argument_list += cast_for_convert+data_name+convert;
		}
	      else // this is a byteArray
		{
		  
		  //format_str += " char* unsigned ";
		  
		  format_str += "%a"+getFormatType("char");//+getFormatType("unsigned");
		  
		  
		  argument_list += "\n\t\t\t\t(char*)"+data_name+".getArray(), "+data_name+".length()\n\t\t\t\t";
		}

	    }
	}
      //if( i < args.size()-1)
      //{
	  //format_str += " ";
      //  argument_list += ", ";
      //}
    }
  out_stream << vec_calc_format << endl << std::flush;
  out_stream << vec_calc << endl << std::flush;
  out_stream << vec_calc_finish << endl << std::flush;




   // send message

   out_stream << base_send << "\"" << format_str << "\"," << argument_list << ");" << endl;
   out_stream << std::flush;

   out_stream << clean_up << endl << std::flush;

   out_stream << "\tint fret = -1;" << endl;
   out_stream << "\tif( sret != -1 ) {" << endl;
   out_stream << "\t  fret = stream->flush();\n\t}" << endl;

   out_stream << "   if( (sret == -1) || (fret == -1) ) ";
   out_stream << Options::error_state(true, 6, "igen_send_err",Options::type_prefix() +"error");
   out_stream << std::flush;
   
   return true;
}
//---------------------------------------------------------------------------------------------
pdstring arg::getFormatType(pdstring old_type) const
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
  return "ERROR ("+old_type+")";

}

