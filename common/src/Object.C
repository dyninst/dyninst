#include "util/h/Object.h"
#include "util/h/Dictionary.h"

ostream & relocationEntry::operator<< (ostream &s) const {
    s << "target_addr_ = " << target_addr_ << endl;
    s << "rel_addr_ = " << rel_addr_ << endl;
    s << "name_ = " << name_ << endl;
    return s; 
}

ostream &operator<<(ostream &os, relocationEntry &q) {
    return q.operator<<(os);
}

bool AObject::needs_function_binding() const {
    return false;
}

bool AObject::get_func_binding_table(vector<relocationEntry> &) const {
    return false;
}

bool AObject::get_func_binding_table_ptr(const vector<relocationEntry> *&) const {
    return false;
}

/**************************************************
 *
 *  Stream based debuggering output - for regreesion testing.
 *  Dump info on state of object *this....
 *
**************************************************/

const ostream &AObject::dump_state_info(ostream &s) {

    // key and value for distc hash iter.... 
    string str;
    Symbol sym;
    dictionary_hash_iter<string, Symbol> symbols_iter(symbols_);

    s << "Debugging Info for AObject (address) : " << this << endl;

    s << " file_ = " << file_ << endl;
    s << " symbols_ = " << endl;

    // and loop over all the symbols, printing symbol name and data....
    //  or try at least....
    symbols_iter.reset();
    while (symbols_iter.next(str, sym)) {
      s << "  key = " << str << " val " << sym << endl;
    }

    s << " code_ptr_ = " << code_ptr_ << endl;
    s << " code_off_ = " << code_off_ << endl;
    s << " code_len_ = " << code_len_ << endl;
    s << " data_ptr_ = " << data_ptr_ << endl;
    s << " data_off_ = " << data_off_ << endl;
    s << " data_len_ = " << data_len_ << endl;
    return s;
}





