// remote_func.C

#include "remote_func.h"
#include "Options.h"

pdstring remote_func::request_tag(bool unqual) const {
  return((unqual ? pdstring("") : Options::type_prefix()) + name() + "_REQ");}

pdstring remote_func::response_tag(bool unqual) const {
  return((unqual ? pdstring("") : Options::type_prefix()) + name() + "_RESP");}

remote_func::remote_func(const pdstring name, pdvector<arg*> *arglist, const call_type &ct,
			 const bool &is_v, const arg &return_arg, const bool do_free)
: name_(name), function_type_(ct), is_virtual_(is_v),
  call_sig_(arglist, name_), return_arg_(return_arg), do_free_(do_free)
{
}

// If I am generating code for the server, I don't handle async upcalls since
// these are calls that the server makes, not receives
// And the opposite applies for the client
bool remote_func::handle_request(const pdstring &spaces, ofstream &out_stream, 
				 const bool srvr, bool special) const {
   if (is_srvr_call()) {
      if (srvr) return false;
   } else {
      if (!srvr) return false;
   }

   out_stream << spaces << "case " << request_tag() << ": {" << endl;

   const pdstring spacesp2 = spaces + "  ";
   const pdstring spacesp4 = spacesp2 + "  ";
   const pdstring spacesp6 = spacesp4 + "  ";

   pdstring vrbleToReadInto;

   bool void_type = (call_sig_.type() == "void");

   if (Options::ml->address_space() == message_layer::AS_many) {
      out_stream << spacesp2 << "if (buffer) {" << endl;

      if (!void_type) {
         vrbleToReadInto = pdstring("*(") + call_sig_.type() + "*)buffer";
         out_stream << spacesp4 << "if ("
                    << call_sig_.gen_bundler_call(false, // receive
                                                  "net_obj()", vrbleToReadInto) << ") "
                    << endl;
         out_stream << spacesp6;
      }
      else
         out_stream << spacesp4;
         
      // Don't make the method call if just buffering
      out_stream << "break;" << endl;
      out_stream << spacesp2 << "}" << endl;
      out_stream << spacesp2 << "else {" << endl;

      if (!void_type) {
	out_stream << spacesp4 << "uint64_t stack_buffer[sizeof(" << call_sig_.type() << ") / sizeof(uint64_t) + (sizeof(" << call_sig_.type() << ") % sizeof(uint64_t) > 0 ? 1 : 0)];" << endl;
	out_stream << spacesp4 << call_sig_.type() << " *buffer = ("
                   << call_sig_.type() << " *)stack_buffer;" << endl;
	vrbleToReadInto = "(*buffer)"; //parentheses ARE needed!!
	out_stream << spacesp4 << "if ("
		   << call_sig_.gen_bundler_call(false, // receive
						 "net_obj()", vrbleToReadInto)
		   << ") "
		   << " {" << endl;
      }

      if (void_type)
	out_stream << spacesp4;
      else
	out_stream << spacesp6;
   }

   if (Options::ml->address_space() == message_layer::AS_one) {
     if (special) {
       out_stream << spacesp2 
            << "if (buffer != NULL) { *buffer = msgBuf; break; }" 
            << endl;
     }
     out_stream << spacesp2
                << call_sig_.type() 
                << "* typedMsgBuf = (" 
                << call_sig_.type() 
                << "*)msgBuf;\n";
     out_stream << spacesp2;
   }

   // Now make the method call

   if (!is_async_call()) {
     if (return_arg_.base_type() != "void")
     {
       out_stream << return_arg_.type(true) 
                << "* ret = new " << return_arg_.type(true)
                << ";\n";
       out_stream << spacesp2 << "(*ret)=";
     }
     else
     {
       // we allocate something so the buffer for our return 
       // message is non-NULL
       out_stream << "char* ret = new char;\n";
     }
   }
   out_stream << name() << "(";
   
   if (Options::ml->address_space() == message_layer::AS_one) {
      if( call_sig_.num_args() > 0 )
      {
            if( call_sig_.num_args() == 1 )
            {
                out_stream << "*typedMsgBuf";
            }
            else
            {
                out_stream << call_sig_.dump_args("typedMsgBuf", "->");
            }
      }
      out_stream << ");\n";
      out_stream << spacesp2 << "delete typedMsgBuf;\n";
   } else {
      out_stream << call_sig_.dump_args(vrbleToReadInto, ".");
      out_stream << ");\n";
   }

   // reply
   if (Options::ml->address_space() == message_layer::AS_many) {
      if (!is_async_call()) {
         out_stream << spaces << "unsigned ret_tag = " << response_tag() << ";\n";
         return_arg_.tag_bundle_send(out_stream, "ret", "ret_tag", 
                                     Options::type_prefix() + "error");
      }
   } else {
      if (!is_async_call()) {
         out_stream << spacesp2
                    << "val = msg_send(getRequestingThread(), "
                    << response_tag()
                    << ", ret);\n";
      }
   }

   // only attempt to free the return value, the user should free the 
   // call_sig args only if there is a return arg
   if ((Options::ml->address_space() != message_layer::AS_one) && do_free() &&
       (return_arg_.base_type() != "void")) {
      out_stream << "delete ret;\n";
   }

   if (Options::ml->address_space() == message_layer::AS_many) {
     if (!void_type) {
       pdstring type = call_sig_.type();
       if( (type != "int") && (type != "double") ) {
         if( type.prefixed_by(Options::type_prefix()) ) {
	   //need to remove the type_prefix from type for destructor call
	   int prefix_length = Options::type_prefix().length();
	   type = type.substr(prefix_length, type.length() - prefix_length);
         }
         if( type.suffixed_by("*") ) {
	   //need to remove * from type and call destructor on non-NULL deref'ed pointer
	   type = type.substr(0, type.length()-1);
	   out_stream << spacesp6 << "if( *buffer != NULL )\n";
	   out_stream << spacesp6 << "  (*buffer)->~" << type << "();\n";
         } else
	   out_stream << spacesp6 << "buffer->~" << type << "();\n";
       }
       out_stream << spacesp6 << "break;" << endl;
     }
     out_stream << (void_type ? spacesp2 : spacesp4) << "}" << endl;
     if (!void_type) {
       // It's potentially dangerous to emit destruct(buffer) in the else case,
       // because the very fact that we got to the else means that the P_xdr_recv()
       // at best partially-completed, leaving buffer in a partially-constructed
       // state.  If we could guarantee that P_xdr_recv() always leaves the object
       // in a valid state, then we could give it a try.  I think that'd be tough.
       // Of course I recognize that by taking the easy way out, there'll be a memory
       // leak in the error case.
       //out_stream << spacesp4 << "else" << endl;
       //out_stream << spacesp6 << "destruct(buffer);" << endl;
      
       out_stream << spacesp2 << "}" << endl;
     }
      
     // Code that hasn't done a "break" before now has fallen through to this
     // error code:
     if (!void_type)
       out_stream << Options::error_state(false, // no braces around this code
					  spaces.length() + 2, "igen_decode_err",
					  Options::type_prefix() + "error");

     out_stream << spacesp2 << "break;" << endl;
     out_stream << spaces << "}" << endl;
   } else { // message_layer::AS_one
     out_stream << spaces << "}\n";
     out_stream << spaces << "break;\n";
   }

   return true;
}

bool remote_func::free_async(const pdstring &spaces,
                             ofstream &out_stream, const bool srvr) const {
   if (is_srvr_call()) {
      if (srvr) return false;
   } else {
      if (!srvr) return false;
   }
   if (!is_async_call()) return false;
   out_stream << spaces << "case " << request_tag() << ": {" << endl;
   if (call_sig_.type() != "void") {
      out_stream << spaces << "   "
                 << call_sig_.type() << " *message = (" << call_sig_.type()
                 << "*)(item->data_ptr);" << endl;
   }

   out_stream << spaces << "   " << name() << "(";
   out_stream << call_sig_.dump_args("(*message)", "."); // "->" doesn't work when 1 arg
   // out_stream << (Options::all_types[call_sig_.base_type()])->dump_args("(*message)", ".");
   out_stream << ");" << endl;

   if (call_sig_.type() != "void") {
     pdstring type = call_sig_.type();
     if( (type != "int") && (type != "double") ) {
       if( type.prefixed_by(Options::type_prefix()) ) {
         //need to remove the type_prefix from type for destructor call
         int prefix_length = Options::type_prefix().length();
         type = type.substr(prefix_length, type.length() - prefix_length);
       }
       if( type.suffixed_by("*") ) {
         //need to remove * from type and call destructor on non-NULL deref'ed pointer
         type = type.substr(0, type.length()-1);
         out_stream << spaces << "   if( *message != NULL )\n";
         out_stream << spaces << "     (*message)->~" << type << "();\n";
       } else
         out_stream << spaces << "   message->~" << type << "();\n";
     }
     out_stream << spaces << "   free(message);" << endl;

//        out_stream << spaces << "   delete (" << call_sig_.type()
//                   << "*) (item->data_ptr);" << endl;
   }

   out_stream << spaces << "   break;\n";
   out_stream << spaces << "}" << endl;

   return true;
}

// If I am generating code for the server, I don't handle async upcalls since
// these are calls that the server makes, not receives
// And the opposite applies for the client
bool remote_func::save_async_request(const pdstring &spaces,
                                     ofstream &out_stream, const bool srvr) const {
   if (is_srvr_call()) {
      if (srvr) return false;
   } else {
      if (!srvr) return false;
   }
   if (!is_async_call()) return false;

   out_stream << spaces;
   out_stream << "case " << request_tag() << ": {" << endl;

   const pdstring spaces_p3 = spaces + "   "; // p3 stands for 'plus 3 more spaces'
   
   if (call_sig_.type() != "void") {
     //out_stream << spaces_p3 << call_sig_.type()
     //		<< " *message = new " << call_sig_.type()
     //	<< ";" << endl;

     out_stream << spaces_p3 << "void *message = malloc(sizeof("
		<< call_sig_.type() << "));" << endl;

     //const pdstring readIntoVrble = "*message";
     const pdstring readIntoVrble = pdstring("*(") + call_sig_.type() + "*)message";
     out_stream << spaces_p3 << "if (!"
		<< call_sig_.gen_bundler_call(false, "net_obj()", 
					      readIntoVrble) << ") ";

     out_stream << Options::error_state(true, 6 + spaces.length(),
					"igen_decode_err", "false");
   }
   out_stream << spaces_p3
              << Options::type_prefix()
              << "buf_struct *buffer = new "
              << Options::type_prefix() << "buf_struct;\n";
   out_stream << spaces << "   assert(buffer);\n";
   out_stream << spaces << "   buffer->data_type = " << request_tag() << ";\n";

   if (call_sig_.type() != "void")
      //out_stream << spaces << "   buffer->data_ptr = (void*) message" << ";\n";
      out_stream << spaces << "   buffer->data_ptr = message" << ";\n";
   else
      out_stream << spaces << "   buffer->data_ptr = NULL;\n";

   out_stream << spaces << "   async_buffer += buffer;\n";
   out_stream << spaces << "   break;\n";
   out_stream << spaces << "}" << endl;

   return true;
}

bool remote_func::gen_async_struct(ofstream &) const { return true; }

bool remote_func::gen_signature(ofstream &out_stream, const bool &hdr,
				const bool server) const
{
  if (hdr)
    if ((server && !is_srvr_call()) ||
        (!server && is_srvr_call()))
      if (is_virtual())
         out_stream << " virtual ";

  out_stream << return_arg_.type(true, true) << " ";

  if (!hdr)
    out_stream << Options::current_interface->gen_class_prefix(is_srvr_call());

  out_stream << name() << "(";
  call_sig_.gen_sig(out_stream);
  out_stream << ")" << (hdr ? ";\n" : "");
  return true;
}

bool remote_func::gen_stub(ofstream &out_srvr, ofstream &out_clnt) const 
{
  switch (function_type_) {
  case async_upcall:
    gen_stub_helper(out_srvr, out_clnt, true);
    break;
  case async_call:
  case sync_call:
    gen_stub_helper(out_srvr, out_clnt, false);
    break;
  default:
    abort();
  }
  return true;
}

bool remote_func::gen_stub_helper(ofstream &out_srvr, ofstream &out_clnt,
				  const bool srvr) const 
{
  if (Options::ml->address_space() == message_layer::AS_one) 
    return (gen_stub_helper_one(out_srvr, out_clnt, srvr));
  else
    return (gen_stub_helper_many(out_srvr, out_clnt, srvr));
}

bool remote_func::gen_stub_helper_many(ofstream &out_srvr, ofstream &out_clnt,
				       const bool srvr) const
{
   ofstream& out_str = (srvr ? out_srvr : out_clnt);

   gen_signature(out_str, false, srvr);
   out_str << " {" << endl;
   if (!is_async_call() && !is_void()) 
      out_str << return_arg_.type() << " ret_arg;\n";

   out_str << "   if (get_err_state() != igen_no_err) {" << endl
           << "      IGEN_ERR_ASSERT;" << endl
           << "      return " << return_value() << ";" << endl
           << "   }" << endl;
  
   if (srvr) {
      out_srvr << "   if (!getVersionVerifyDone()) {" << endl;
      out_srvr << "      if (!verify_protocol()) ";
      out_srvr << Options::error_state(true,
                                       9, "igen_proto_err", return_value()) << endl;
      out_srvr << "   }" << endl;
   }

   out_str << "   " << Options::ml->tag_type() << " tag = " << request_tag() << ";"
           << endl;
   call_sig_.tag_bundle_send(out_str, return_value(),
                             request_tag());

   if (!is_async_call()) {
      out_str << "   if (!awaitResponse(" << response_tag() << ")) "
              << Options::error_state(true, 6, "igen_read_err", return_value());

      // set direction decode 
      if (!is_void()) {
         out_str << Options::set_dir_decode() << ";\n";

         // decode something
         out_str << "   if (!"
                 << (Options::all_types[return_arg_.base_type()])->gen_bundler_call(false, // false --> receiving
                                                                                    "net_obj()",
                                                                                    "ret_arg",
                                                                                    return_arg_.stars())
                 << ") ";
         out_str << Options::error_state(true, 6, "igen_decode_err", return_value());
      }

      out_str << "   return " << return_value() << ";\n";
   }

   out_str << "}" << endl;

   return true;
}

bool remote_func::gen_stub_helper_one(ofstream &out_srvr, ofstream &out_clnt,
				      const bool srvr) const
{
  ofstream& out_str = (srvr ? out_srvr : out_clnt);

  gen_signature(out_str, false, srvr);
  out_str << "{\n";
  if (!is_async_call() && !is_void()) 
    out_str << return_arg_.type() << " ret_arg;\n";

  out_str << "if (get_err_state() != igen_no_err) {\n"
    << "IGEN_ERR_ASSERT;\nreturn " << return_value() << ";\n}\n";
  
  call_sig_.tag_bundle_send(out_str, return_value(),
			    request_tag());

  if (!is_async_call()) {

	out_str << "thread_t tid = THR_TID_UNSPEC;\n";
    out_str << Options::ml->tag_type() << " tag = " << response_tag() << ";\n";
    if( !is_void() )
    {
        out_str << return_arg_.type();
    }
    else
    {
        out_str << "char";
    }
    out_str << "* rval = NULL;\n";
    out_str << "if(" << Options::ml->recv_message()
            << "(&tid, &tag, (void**)&rval) == " 
            << Options::ml->bundle_fail() 
            << ")\n";
    out_str << Options::error_state(true, 6, "igen_read_err", return_value());
    out_str << "if (tag != " << response_tag() << ") ";
    out_str << Options::error_state(true, 6, "igen_read_err", return_value());

    if( !is_void() )
    {
        out_str << return_value() << " = *rval;\n";
    }
    out_str << "delete rval;\n";
    out_str << "return " << return_value() << ";\n";
  }

  out_str << "}" << endl;
  return true;
}

