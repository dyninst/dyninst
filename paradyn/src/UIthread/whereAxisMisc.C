// whereAxisMisc.C
// non-member functions otherwise related to whereAxis.C & where4tree.C

#include <ctype.h>
#include "tclclean.h"
#include "tkclean.h"

#include "String.h"

#include "minmax.h"

//#ifndef PARADYN
//// The where axis test program has the proper -I settings
//#include "DMinclude.h"
//#else
//#include "paradyn/src/DMthread/DMinclude.h"
//#endif

//#include "graphicalPath.h"
//#include "whereAxis.h"
//#include "whereAxisMisc.h"

// Stuff only for the where axis test program;
#ifndef PARADYN
#include <fstream.h>
string readUntilQuote(ifstream &is) {
   char buffer[256];
   char *ptr = &buffer[0];
   
   while (true) {
      char c;
      is.get(c);
      if (!is || is.eof())
         return string(); // empty
      if (c=='"')
         break;
      if (c==')') {
         cerr << "readUntilQuote: found ) before \"" << endl;
         exit(5);
      }
      *ptr++ = c;
   }

   *ptr = '\0'; // finish off the string
   return string(buffer);
}

string readUntilSpace(ifstream &is) {
   char buffer[256];
   char *ptr = &buffer[0];
   
   while (true) {
      char c;
      is.get(c);
      if (!is || is.eof())
         return string(); // empty
      if (isspace(c))
         break;
      if (c==')') {
         is.putback(c);
         break;
      }
      *ptr++ = c;
   }

   *ptr = '\0'; // finish off the string
   return string(buffer);
}

string readItem(ifstream &is) {
   char c;
   is >> c;
   if (!is || is.eof())
      return (char *)NULL;

   if (c== ')') {
      is.putback(c);
      return string();
   }

   if (c=='"')
      return readUntilQuote(is);
   else {
      is.putback(c);
      return readUntilSpace(is);
   }
}
#endif
