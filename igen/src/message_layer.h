// message_layer.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _MESSAGE_LAYER_H_
#define _MESSAGE_LAYER_H_

#include "common/h/String.h"

class message_layer {
public:
  typedef enum { AS_one, AS_many, AS_none } AS;
  typedef enum { Med_xdr, Med_pvm, Med_thread, Med_other, Med_none, Med_rpc } medium;

  message_layer() { med_ = Med_none; }
  message_layer(const string) { }
  message_layer(const string nm, const medium md, const string bp, const string brt,
		const string mdp, const string mo, const string mop, const AS as,
		const string bfail, const string bok, const string dir_f,
		const string pack_f, const string unpack_f, const string free_c,
		const string rpc_par, const string tag_type,
		const string send_msg, const bool r_used,
		const string skip_msg, const string r_msg, const string incs,
		const bool do_serial, const string enc, const string dec,
		const bool do_skip);
  ~message_layer() { }
  bool operator== (const message_layer &other) const { return (name_ == other.name());}

  const string &name() const { return name_;}
  medium med() const { return med_;}
  const string &bundler_prefix() const { return bundler_prefix_;}
  const string &bundler_return_type() const { return bundler_return_type_;}
  const string &marshall_data_ptr() const { return marshall_data_ptr_;}
  const string &marshall_obj() const { return marshall_obj_;}  
  const string &marshall_obj_ptr() const { return marshall_obj_ptr_;}
  AS address_space() const { return address_space_;}
  const string &bundle_fail() const { return bundle_fail_;}
  const string &bundle_ok() const { return bundle_ok_;}
  const string &dir_field() const { return dir_field_;}
  const string &pack_const() const { return pack_const_;}
  const string &unpack_const() const { return unpack_const_;}
  const string &free_const() const { return free_const_;}
  const string &rpc_parent() const { return rpc_parent_;}
  const string &tag_type() const { return tag_type_;}
  const string &send_message() const { return send_message_;}
  string send_tag(const string &obj_name, const string &tag) const;
  string recv_tag(const string &obj_name, const string &tag) const;
  bool records_used() const { return records_used_;}
  const string &skip_message() const { return skip_message_;}
  const string &recv_message() const { return recv_message_;}
  const string &includes() const { return incs_;}
  bool serial() const { return serial_;}
  const string &set_dir_decode() const { return decode_;}
  const string &set_dir_encode() const { return encode_;}
  bool skip() const { return skip_;}

private:
  string name_;
  medium med_;
  string bundler_prefix_;
  string bundler_return_type_;
  string dir_is_free_;
  string marshall_data_ptr_;
  string marshall_obj_;
  string marshall_obj_ptr_;
  AS address_space_;
  string bundle_fail_;
  string bundle_ok_;
  string dir_field_;
  string pack_const_;
  string unpack_const_;
  string free_const_;
  string rpc_parent_;
  string tag_type_;
  string send_message_;
  bool records_used_;
  string skip_message_;
  string recv_message_;
  string incs_;
  bool serial_;
  string encode_;
  string decode_;
  bool skip_;
};

#endif
