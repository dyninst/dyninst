/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: ParameterDict.C,v 1.1 2005/09/29 20:40:04 bpellin Exp $
#include "ParameterDict.h"
#include <stdio.h>
#include <string.h>

// Parameter -
Parameter::Parameter()
{
   data1 = 0;
}
Parameter::~Parameter()
{
}

// Error functions, in proper operation these will be overidden in a subclass
void Parameter::setString(char *str)
{
   fprintf(stderr, "Warning: Setting a string for a non-string Parameter, ignored\n");
}

char *Parameter::getString()
{
   fprintf(stderr, "Warning: Getting a string for an non-string Parameter, ignored\n");
   return NULL;
}

int Parameter::getInt()
{
   fprintf(stderr, "Warning: Getting an int for a non-int Parameter, ignored\n");
   return -1;
}

void Parameter::setInt(int num)
{
   fprintf(stderr, "Warning: Setting an int for a non-int Parameter, ignored\n");
}

void *Parameter::getPtr()
{
   fprintf(stderr, "Warning: Getting a ptr for a non-ptr Parameter, ignored\n");
   return NULL;
}

void Parameter::setPtr(void *ptr)
{
   fprintf(stderr, "Warning: Setting an ptr for a non-ptr Parameter, ignored\n");
}
// End Error Functions

// String Functions
ParamString::ParamString()
{
   data = NULL;
}

ParamString::ParamString(char *str)
{
   if ( str != NULL )
   {
      data = strdup(str);
   }
   else
   {
      data = NULL;
   }

}

ParamString::~ParamString()
{
   if (data != NULL )
   {
      free(data);
   }
}

void ParamString::setString(char *str)
{
   // Free old string
   if ( data != NULL )
   {
      free(data);
   }

   if ( str != NULL )
   {
      data = strdup(str);
   }
   else
   {
      data = NULL;
   }
}

char* ParamString::getString()
{
   return data;
}

// End String Functions
// Integer functions
ParamInt::ParamInt(int num)
{
   data = num;
}

int ParamInt::getInt()
{
   return data;
}

void ParamInt::setInt(int num)
{
   data = num;
}

// End Integer functions

// Pointer functions
ParamPtr::ParamPtr()
{
   data = NULL;
}

ParamPtr::ParamPtr(void *ptr)
{
   data = ptr;
}

void *ParamPtr::getPtr()
{
   return data;
}

void ParamPtr::setPtr(void *ptr)
{
   data = ptr;
}
