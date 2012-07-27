/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: ParameterDict.C,v 1.3 2008/10/20 20:35:43 legendre Exp $
#include "ParameterDict.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Parameter -
Parameter::Parameter()
{
   data1 = 0;
}
Parameter::~Parameter()
{
}

// Error functions, in proper operation these will be overidden in a subclass
void Parameter::setString(const char *str)
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

ParamString::ParamString(const char *str)
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

void ParamString::setString(const char *str)
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
