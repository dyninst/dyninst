// message_layer.C

#include "message_layer.h"
#include "Options.h"

message_layer::message_layer(const string nm, const medium md, const string bp,
			     const string brt, const string mdp, const string mo,
			     const string mop, const AS as, const string bfail,
			     const string bok, const string dir_f, const string pack_f,
			     const string unpack_f, const string free_f,
			     const string rpc_par, const string tag_type,
				 const string send_msg,
			     const bool r_used, const string skip_msg,
			     const string recv_msg, const string incs,
			     const bool do_serial, 
			     const string enc, const string dec,
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

string message_layer::send_tag(const string &obj_name, const string &tag_s) const {
   return Options::ml->bundler_prefix() + // e.g. "P_xdr"
          "send(" +
          obj_name +
          ", " + tag_s + ")";
}

string message_layer::recv_tag(const string &obj_name, const string &tag_s) const {
   return Options::ml->bundler_prefix() + // e.g. "P_xdr"
          "recv(" +
          obj_name +
          ", " + tag_s + ")";
}
