// arg.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _ARG_H_
#define _ARG_H_

#include "common/h/String.h"
#include <fstream.h>

class arg {
public:
  // gen_variable()
  arg(const string *type, const unsigned star_count, const bool is_const,
      const string *name, const bool is_ref);
  arg() { }
  ~arg() { }
  bool operator== (const arg &other) const { return (type_ == other.type()); }

  bool is_void() const { return (type_ == "void");}
  string gen_bundler_name(bool send_routine) const;
  void gen_bundler(bool send_routine,
                   ofstream &outStream, const string &obj_name,
		   const string &data_name) const;

  const string &pointers() const { return pointers_; }
  const string &base_type() const { return type_;}
  string type(const bool use_const=false, const bool use_ref=false) const;
  const string &name() const { return name_; }
  bool is_const() const { return constant_;}
  bool tag_bundle_send(ofstream &out_stream, const string bundle_value, 
		       const string tag_value, const string return_value) const;
  unsigned stars() const { return stars_;}
  bool is_ref() const { return is_ref_;}
  string deref(const bool local) const;

private:
  string pointers_;
  string type_;
  string name_;
  bool constant_;
  unsigned stars_;
  bool is_ref_;

  bool tag_bundle_send_one(ofstream &out_stream, const string bundle_value, 
                           const string tag_value, const string return_value) const;
  bool tag_bundle_send_many(ofstream &out_stream, const string bundle_value, 
                            const string tag_value, const string return_value) const;
};

#endif
