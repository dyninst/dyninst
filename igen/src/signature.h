// signature.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include "common/h/Vector.h"
#include "common/h/String.h"
#include <fstream.h>
#include "arg.h"

class signature {
public:
  signature(vector<arg*> *alist, const string rf_name);
  ~signature() { }

  string type(const bool use_bool=false) const;
  string base_type() const { return type_;}
  void type(const string t, const unsigned star);
  bool gen_sig(ofstream &out_stream) const;
  bool tag_bundle_send(ofstream &out_stream, const string &return_value,
		       const string &req_tag) const;
  bool tag_bundle_send_many(ofstream &out_stream, const string &return_value,
			    const string &req_tag) const;
  bool tag_bundle_send_one(ofstream &out_stream, const string return_value,
			   const string req_tag) const;
  bool arg_struct(ofstream &out_stream) const;
  string dump_args(const string message, const string sep) const;
  string gen_bundler_call(bool send_routine,
                          const string &obj_name, const string &data_name) const;

private:
  vector<arg*> args;
  string type_;
  bool is_const_;
  unsigned stars_;
};

#endif
