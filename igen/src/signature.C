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

// signature.C

#include "signature.h"
#include "type_defn.h"
#include "Options.h"
#include "common/h/Dictionary.h"

signature::signature(pdvector<arg*> *alist, const pdstring rf_name) : is_const_(false), stars_(0) {
  assert(alist);
  for (unsigned i=0; i<alist->size(); i++) 
    args += (*alist)[i];

  switch (args.size()) {
  case 0: type_ = "void"; break;
  case 1:
    type_ = args[0]->base_type();
    stars_ = args[0]->stars();
    is_const_ = args[0]->is_const();
    break;
  default:
    const type_defn *td = NULL;
    bool match = false;
    for (dictionary_hash_iter<pdstring, type_defn*> tdi=Options::all_types.begin();
         tdi != Options::all_types.end() && !match; tdi++) {
       td = tdi.currval();
       match = td->is_same_type(alist);
    }
    
    if (match) {
      type_ = td->name();
      for (unsigned q=0; q<args.size(); ++q)
	delete args[q];
      args.resize(0);
      args = td->copy_args();
      // get names here
    } else {
      type_ = Options::allocate_type(pdstring("T_") + rf_name, false, false,
				     false,
                     false,
                     "",
				     type_defn::TYPE_COMPLEX,
				     true, false, &args);
    }
    break;
  }
}

pdstring signature::type(const bool use_c) const {
  pdstring ret;
  if (use_c && is_const_)
    ret = "const ";
  ret += type_;
  ret += Options::make_ptrs(stars_);
  return ret;
}

pdstring signature::gen_bundler_call(bool send_routine,
                                   const pdstring &obj_nm, const pdstring &data_nm) const {

  //return " signature \n";
  return ((Options::all_types[base_type()])->gen_bundler_call(send_routine,
                                                              obj_nm,
							      data_nm, stars_));
}

bool signature::tag_bundle_send(ofstream &out_stream, const pdstring &return_value,
				const pdstring &req_tag) const
{

  if (Options::ml->address_space() == message_layer::AS_many)
    return (tag_bundle_send_many(out_stream, return_value, req_tag));
  else
    return (tag_bundle_send_one(out_stream, return_value, req_tag));
}

bool signature::tag_bundle_send_many(ofstream &out_stream, const pdstring &return_value,
				     const pdstring &) const
{

   // set direction encode
   out_stream << "   " << Options::set_dir_encode() << ";\n";

   // bundle individual args
   out_stream << "   if (!" << Options::ml->send_tag("net_obj()", "tag");
   out_stream << std::flush;

   for (unsigned i=0; i<args.size(); i++) {
      out_stream << " ||" << endl;
      out_stream << "       !";

      args[i]->gen_bundler(true, // sending
                           out_stream, "net_obj()", ""); // "" was formerly "&"
      out_stream << std::flush;
   }
  
   out_stream << ") ";
   out_stream << std::flush;
   out_stream << Options::error_state(true, 6, "igen_encode_err", return_value);
   out_stream << std::flush;

   // send message
   out_stream << "   if (!" << Options::ml->send_message() << ") ";
   out_stream << Options::error_state(true, 6, "igen_send_err", return_value);

   out_stream << std::flush;
   
   return true;
}

bool signature::tag_bundle_send_one(ofstream &out_stream, const pdstring return_value,
				    const pdstring req_tag) const
{
    if( args.size() > 0 )
    {
        out_stream << type(true) << "* send_buffer = new " << type(true) << ";\n";
        if( args.size() == 1 )
        {
            out_stream << "*send_buffer = " << args[0]->name() << ";\n";
        }
        else
        {
            (Options::all_types[type()])->assign_to("send_buffer->", 
                                                        args, 
                                                        out_stream);
        }
    }
    else
    {
        out_stream << "char* send_buffer = new char;\n";
    }

  pdstring sb = "send_buffer);\n";

  out_stream << Options::ml->bundler_return_type() << " res = "
    << Options::ml->send_message() << "(net_obj(), " << req_tag
      << ", " << sb;

  out_stream << "if (res == " << Options::ml->bundle_fail() << ") ";
  out_stream << Options::error_state(true, 6, "igen_send_err", return_value);

  return true;
}
//---------------------------------------------------------------------------------------------
bool signature::tag_bundle_send_mrnet(ofstream &out_stream, const pdstring &return_value,
				const pdstring &req_tag,const pdstring &fnc_name) const
{
  if (Options::ml->address_space() == message_layer::AS_many)
    return (tag_bundle_send_many_mrnet(out_stream, return_value, req_tag, fnc_name));
  else
    return (tag_bundle_send_one_mrnet(out_stream, return_value, req_tag, fnc_name));
}

bool signature::tag_bundle_send_many_mrnet(ofstream &out_stream, const pdstring &return_value,
				     const pdstring &rq_tag,const pdstring &fnc_name) const
{

  pdstring argument_list;
  pdstring format_str;
  pdstring base_send = "\n//signature 1\n\tint sret = stream->send( tag,";
  pdstring vec_calc = "\n";
  pdstring vec_calc_format = "\n";
  pdstring vec_calc_finish = "\n";
  pdstring clean_up;
  pdstring temp;
  pdstring struc_name = "";
  pdstring received_field;
  if(args.size() == 0)
    {
      format_str = "";
      argument_list = "0";
    }
  else
    {
      // bundle individual args

      for (unsigned i=0; i < args.size(); i++) 
	{
	  pdstring convert = "";
	  pdstring cast_for_convert = "";
	  pdstring data_name;
	  pdstring data_type;
	  int level = 0;
	  
	  data_type = args[i]->gen_bundler_type(&data_name);

	  out_stream << "\t//type = "<< data_type << "  name = " << data_name <<"\n"<< endl;

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
	      
	      vec_calc +="\n\tfor(unsigned int i = 0 ; i < "+data_name+".size(); i++){\n\n//Signature pdvector\n\n";


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

		      vec_calc_format +="\t"+ temp+"* array_"+data_name+" = new "+ temp +"["+data_name+".size()+1];\n//signature 1\n";

		      clean_up +="\n\tdelete array_"+data_name+";\n";

		      vec_calc +="\tarray_"+data_name+"[i] = "+cast_for_convert+data_name+"[i]"+convert+";\n\t}\n\n";
		    }
		  else // this is a byteArray
		    {
		      
		      //format_str += " char** unsigned* ";
		      format_str += "%a"+getFormatType("char");//+"a"+getFormatType("unsigned");

		      argument_list += "\n\t\t\t\t array_"+data_name+", array_"+data_name+"_length\n\t\t\t\t";

		      vec_calc_format +="\tchar** array_"+data_name+" = new "+ temp +"["+data_name+".size()];\n//signature 2\n";
		      vec_calc_format +="\tunsigned* array_"+data_name+"_length = new "+ temp +"["+data_name+".size()];\n//signature 3\n";

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
	  if( i < args.size()-1)
	    {
	      //format_str += " ";
	      argument_list += ", ";
	    }
	}
      out_stream << vec_calc_format << endl << std::flush;
      out_stream << vec_calc << endl << std::flush;
      out_stream << vec_calc_finish << endl << std::flush;
    }



   // send message

   out_stream << base_send << "\"" << format_str << "\"," << argument_list << ");" << endl;
   out_stream << std::flush;

   out_stream << clean_up << endl << std::flush;

   out_stream << "\tint fret = -1;" << endl;
   out_stream << "\tif( sret != -1 ) {" << endl;
   out_stream << "\t  fret = stream->flush();\n\t}" << endl;

   out_stream << "   if( (sret == -1) || (fret == -1) ) ";
   out_stream << Options::error_state(true, 6, "igen_send_err", return_value);

   out_stream << std::flush;
   
   return true;
}
bool signature::tag_bundle_send_one_mrnet(ofstream &out_stream, const pdstring return_value,
				    const pdstring req_tag,const pdstring &fnc_name) const
{

  out_stream <<"-- send_one_mrnet "<<endl<<std::flush;

    if( args.size() > 0 )
    {
        out_stream << type(true) << "* send_buffer = new " << type(true) << ";\n";
        if( args.size() == 1 )
        {
            out_stream << "*send_buffer = " << args[0]->name() << ";\n";
        }
        else
        {
            (Options::all_types[type()])->assign_to("send_buffer->", 
                                                        args, 
                                                        out_stream);
        }
    }
    else
    {
        out_stream << "char* send_buffer = new char;\n";
    }

  pdstring sb = "send_buffer);\n";

  out_stream << Options::ml->bundler_return_type() << " res = "
    << Options::ml->send_message() << "(net_obj(), " << req_tag
      << ", " << sb;

  out_stream << "if (res == " << Options::ml->bundle_fail() << ") ";
  out_stream << Options::error_state(true, 6, "igen_send_err", return_value);

  return true;
}
//---------------------------------------------------------------------------------------------
pdstring signature::dump_args(const pdstring data_name, const pdstring sep) const {
  pdstring ret;
  switch (args.size()) {
  case 0:
    // out_stream << "void";
    return "";
  case 1:
    return data_name;
  default:
    for (unsigned i=0; i<args.size(); i++) {
      ret += data_name + sep + args[i]->name();
      if (i < (args.size() - 1))
	ret += ", ";
    }
    return ret;
  }
}

bool signature::gen_sig(ofstream &out_stream, bool is_syncCall) const {
  if (Options::ml->name() != "mrnet")
    {

      switch (args.size())
	{
	case 0:
	  //out_stream << "void";
	  out_stream << "";
	  break;
	case 1:
	default:
	  for (unsigned i=0; i<args.size(); i++)
	    {
	      out_stream << args[i]->type(true, true) << " " << args[i]->name();
	      if (i < (args.size()-1))
		out_stream << ", ";
	    }
	  break;
	}
    }
  else
    {
     out_stream << "Stream *stream";
      switch (args.size())
	{
	case 0:
	  break;
	case 1:
	default:
	  out_stream << ", ";
	  for (unsigned i=0; i<args.size(); i++)
	    {
	      out_stream << args[i]->type(true, true) << " " << args[i]->name();
	      if (i < (args.size()-1))
		out_stream << ", ";
	    }
	  break;
	}
    }
  return true;
}

bool signature::arg_struct(ofstream &out_stream) const {
  for (unsigned i=0; i<args.size(); i++)
    out_stream << args[i]->type() << " " << args[i]->name() << ";\n";
  return true;
}

pdstring signature::getFormatType(pdstring old_type) const
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
  else 
    if(old_type == "short")
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


