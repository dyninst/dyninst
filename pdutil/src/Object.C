#include "util/h/Object.h"

bool AObject::needs_function_binding() const {
    return false;
}

bool AObject::get_func_binding_table(vector<relocationEntry> &) const {
    return false;
}

bool AObject::get_func_binding_table_ptr(const vector<relocationEntry> *&) const {
    return false;
}
