// tvFocus.C
// Ariel Tamches

/*
 * $Log: tvFocus.C,v $
 * Revision 1.4  1996/01/11 01:51:37  tamches
 * completely revamped/debugged how short focus names are calculated
 *
 * Revision 1.3  1995/12/22 22:39:41  tamches
 * added visiLibId
 *
 * Revision 1.2  1995/11/08 21:48:13  tamches
 * moved implementation of constructor to .C file
 *
 * Revision 1.1  1995/11/04 00:46:40  tamches
 * First version of new table visi
 *
 */

#include "Vector.h"
#include "tvFocus.h"

tvFocus::tvFocus(unsigned iVisiLibId,
		 const string &iLongName, XFontStruct *nameFontStruct) :
                                                   longName(iLongName) {
   visiLibId = iVisiLibId;

   longNamePixWidth = XTextWidth(nameFontStruct, longName.string_of(),
				 longName.length());
   
   // Calculate the short name as follows:
   // Split the name up into its components.  Assuming there are 4 resource
   // hierarchies, there will be 4 components.
   // For each component, strip off everything upto and including the last '/'
   // Then re-concatenate the components back together (with comma delimiters)
   // to obtain the short name
   
   // Step 0: make an exception for "Whole Program"
   if (0==strcmp(longName.string_of(), "Whole Program")) {
      shortName = longName;
   }
   else {
      // Step 1: split up into components; 1 per resource hierarchy
      vector<string> components;

      const char *ptr = longName.string_of();
      while (*ptr != '\0') {
         // begin a new component; collect upto & including the first seen comma
         char buffer[200];
         char *bufferPtr = &buffer[0];
         do {
            *bufferPtr++ = *ptr++;
         } while (*ptr != ',' && *ptr != '\0');

         if (*ptr == ',')
            *bufferPtr++ = *ptr++;

         *bufferPtr = '\0';

         components += string(buffer);
      }

      // Step 2: for each component, strip off all upto and including
      //         the last '/'
      for (unsigned componentlcv=0; componentlcv < components.size(); componentlcv++) {
         const string &oldComponentString = components[componentlcv];

         char *ptr = strrchr(oldComponentString.string_of(), '/');
         if (ptr == NULL)
            cerr << "tableVisi: could not find / in component " << oldComponentString << endl;
         else if (ptr+1 == '\0')
            cerr << "tableVisi: there was nothing after / in component " << oldComponentString << endl;
         else
            components[componentlcv] = string(ptr+1);
      }

      // Step 3: combine the components
      string theShortName;
      for (unsigned componentlcv=0; componentlcv < components.size(); componentlcv++)
         theShortName += components[componentlcv];

      // Step 4: pull it all together:
      this->shortName = theShortName;
   }
   
   shortNamePixWidth = XTextWidth(nameFontStruct, shortName.string_of(),
				  shortName.length());
}
