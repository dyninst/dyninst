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

// $Id: ParameterDict.h,v 1.2 2008/10/20 20:35:44 legendre Exp $
#ifndef PARAMETERDICT_H
#define PARAMETERDICT_H

#include <map>
#include <string>
#include "test_lib_dll.h"


class TESTLIB_DLL_EXPORT Parameter {
   public:
      Parameter();
      virtual ~Parameter();
      virtual char *getString();
      virtual void setString(const char *str);

      virtual int getInt();
      virtual void setInt(int num);

      virtual void *getPtr();
      virtual void setPtr(void *ptr);
      
      int data1;
};

class TESTLIB_DLL_EXPORT ParamString : public Parameter {
   private:
     char *data;

   public:
      ParamString();
      ~ParamString();
      ParamString(const char *str);
      char *getString();
      void setString(const char *str);
};

class TESTLIB_DLL_EXPORT ParamInt : public Parameter {
   private:
      int data;

   public:
      ParamInt(int num);
      int getInt();
      void setInt(int num);
};

class TESTLIB_DLL_EXPORT ParamPtr : public Parameter {
   private:
      void *data;

   public:
      ParamPtr();
      ParamPtr(void *ptr);
      void *getPtr();
      void setPtr(void *ptr);
};

typedef std::map<std::string, Parameter*> ParameterDict;

#endif /* PARAMETERDICT_H */
