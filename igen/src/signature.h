// signature.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include "common/h/Vector.h"
#include "common/h/String.h"
#include <fstream>
#include "arg.h"

using std::ofstream;

class signature {
public:
  signature(pdvector<arg*> *alist, const pdstring rf_name);
  ~signature() { }

  pdstring type(const bool use_bool=false) const;
  pdstring base_type() const { return type_;}
  void type(const pdstring t, const unsigned star);
  bool gen_sig(ofstream &out_stream) const;
  bool tag_bundle_send(ofstream &out_stream, const pdstring &return_value,
		       const pdstring &req_tag) const;
  bool tag_bundle_send_many(ofstream &out_stream,
                            const pdstring &return_value,
                            const pdstring &req_tag) const;
  bool tag_bundle_send_one(ofstream &out_stream,
                           const pdstring return_value,
                           const pdstring req_tag) const;
  bool arg_struct(ofstream &out_stream) const;
  pdstring dump_args(const pdstring message, const pdstring sep) const;
  pdstring gen_bundler_call(bool send_routine, const pdstring &obj_name,
                            const pdstring &data_name) const;
  unsigned int num_args( void ) const   { return args.size(); } 

private:
  pdvector<arg*> args;
  pdstring type_;
  bool is_const_;
  unsigned stars_;
};

#endif
