
#include <common/h/headers.h>

#include "BG_AuxvReader.h"
#include "parseauxv.h"
#include "auxvtypes.h"


// taken from LinuxKludges.C.
// TODO: put this in some common place?  It's used by at least 2 platforms.
char * P_cplus_demangle( const char * symbol, bool nativeCompiler, bool includeTypes ) 
{
  int opts = 0;
  opts |= includeTypes ? DMGL_PARAMS | DMGL_ANSI : 0;
  opts |= nativeCompiler ? DMGL_ARM : 0;
  
  char * demangled = cplus_demangle( const_cast< char *>(symbol), opts);
  if( demangled == NULL ) { return NULL; }
  
  if( ! includeTypes ) {
    /* de-demangling never increases the length */   
    char * dedemangled = strdup( demangled );   
    assert( dedemangled != NULL );
    dedemangle( demangled, dedemangled );
    assert( dedemangled != NULL );
    
    free( demangled );
    return dedemangled;
  }
  
  return demangled;
} /* end P_cplus_demangle() */



bool AuxvParser::readAuxvInfo() {
  BG_AuxvReader reader(pid);
  
  while (reader.has_next()) {
    auxv_element elt = reader.next();
    if (elt.type == AT_BASE) {
      interpreter_base = static_cast<Address>(elt.value);
      return reader.good();
    }
  }

  if (!reader.good()) {
    return false;
  }

  return true; // not found, but we tried.  Must be static.
}


