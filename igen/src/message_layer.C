// message_layer.C

#include "message_layer.h"
#include "Options.h"

message_layer::message_layer(const pdstring nm, const medium md, const pdstring bp,
			     const pdstring brt, const pdstring mdp, const pdstring mo,
			     const pdstring mop, const AS as, const pdstring bfail,
			     const pdstring bok, const pdstring dir_f, const pdstring pack_f,
			     const pdstring unpack_f, const pdstring free_f,
			     const pdstring rpc_par, const pdstring tag_type,
				 const pdstring send_msg,
			     const bool r_used, const pdstring skip_msg,
			     const pdstring recv_msg, const pdstring incs,
			     const bool do_serial, 
			     const pdstring enc, const pdstring dec,
			     const bool do_skip)
: name_(nm), med_(md), bundler_prefix_(bp), bundler_return_type_(brt),
  marshall_data_ptr_(mdp), marshall_obj_(mo), marshall_obj_ptr_(mop), address_space_(as),
  bundle_fail_(bfail), bundle_ok_(bok), dir_field_(dir_f), pack_const_(pack_f), 
  unpack_const_(unpack_f), free_const_(free_f), rpc_parent_(rpc_par),
  tag_type_(tag_type),
  send_message_(send_msg), records_used_(r_used), skip_message_(skip_msg),
  recv_message_(recv_msg), incs_(incs), serial_(do_serial), 
  encode_(enc), decode_(dec), skip_(do_skip)
{
  
}

pdstring message_layer::send_tag(const pdstring &obj_name, const pdstring &tag_s) const {
   return Options::ml->bundler_prefix() + // e.g. "P_xdr"
          "send(" +
          obj_name +
          ", " + tag_s + ")";
}

pdstring message_layer::recv_tag(const pdstring &obj_name, const pdstring &tag_s) const {
   return Options::ml->bundler_prefix() + // e.g. "P_xdr"
          "recv(" +
          obj_name +
          ", " + tag_s + ")";
}
