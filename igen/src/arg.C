// arg.h
// Put in by Ariel Tamches; moved from main.C into its own .h/.C file combo
// for clarity.

#include "arg.h"
#include "Options.h"

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

bool arg::tag_bundle_send_one(ofstream &, const pdstring,
                              const pdstring, const pdstring) const 
{
  abort();
  return false;
}
