/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// tvFocus.C
// Ariel Tamches

// $Id: tvFocus.C,v 1.8 2002/05/13 19:54:08 mjbrim Exp $

#include "common/h/Vector.h"
#include "tvFocus.h"

tvFocus::tvFocus(unsigned iVisiLibId,
		 const string &iLongName, Tk_Font nameFont) :
                                                   longName(iLongName) {
   visiLibId = iVisiLibId;

   longNamePixWidth = Tk_TextWidth(nameFont, longName.c_str(),
				 longName.length());
   
   // Calculate the short name as follows:
   // Split the name up into its components.  Assuming there are 4 resource
   // hierarchies, there will be 4 components.
   // For each component, strip off everything upto and including the last '/'
   // Then re-concatenate the components back together (with comma delimiters)
   // to obtain the short name
   
   // Step 0: make an exception for "Whole Program"
   if (0==strcmp(longName.c_str(), "Whole Program")) {
      shortName = longName;
   }
   else {
      // Step 1: split up into components; 1 per resource hierarchy
      vector<string> components;
		unsigned componentlcv;

      const char *ptr = longName.c_str();
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
      for (componentlcv=0; componentlcv < components.size(); componentlcv++) {
         const string &oldComponentString = components[componentlcv];

         char *ptr = strrchr(oldComponentString.c_str(), '/');
         if (ptr == NULL)
            cerr << "tableVisi: could not find / in component " << oldComponentString << endl;
         else if (ptr+1 == '\0')
            cerr << "tableVisi: there was nothing after / in component " << oldComponentString << endl;
         else
            components[componentlcv] = string(ptr+1);
      }

      // Step 3: combine the components
      string theShortName;
      for (componentlcv=0; componentlcv < components.size(); componentlcv++)
         theShortName += components[componentlcv];

      // Step 4: pull it all together:
      this->shortName = theShortName;
   }
   
   shortNamePixWidth = Tk_TextWidth(nameFont, shortName.c_str(),
				  shortName.length());
}
