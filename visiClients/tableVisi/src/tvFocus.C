// tvFocus.C
// Ariel Tamches

/*
 * $Log: tvFocus.C,v $
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

#include "tvFocus.h"

tvFocus::tvFocus(unsigned iVisiLibId,
		 const string &iLongName, XFontStruct *nameFontStruct) :
                                                   longName(iLongName) {
   visiLibId = iVisiLibId;

   longNamePixWidth = XTextWidth(nameFontStruct, longName.string_of(),
				 longName.length());
      
   // Calculate the short name now, by starting at the end of name, working
   // backwards, and looking for a "/"
   const char *ptr = strrchr(longName.string_of(), '/');
   if (ptr == NULL)
      shortName = longName;
   else if (ptr + 1 == '\0')
      shortName = longName;
   else
      shortName = ptr + 1;

   shortNamePixWidth = XTextWidth(nameFontStruct, shortName.string_of(),
				  shortName.length());
}

