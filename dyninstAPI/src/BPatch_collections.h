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

#ifndef _BPatch_collections_h_
#define _BPatch_collections_h_

#include <string>
#include "BPatch_type.h"     //type and localVar
#include <unordered_map>
#include "common/h/util.h"


class BPatch_localVarCollection{
  
  std::unordered_map<std::string, BPatch_localVar *> localVariablesByName;

public:
  BPatch_localVarCollection() {}
  ~BPatch_localVarCollection();

  void addLocalVar(BPatch_localVar * var);
  BPatch_localVar * findLocalVar(const char *name);
  BPatch_Vector<BPatch_localVar *> *getAllVars();  
};
  


class BPatch_typeCollection {
    friend class BPatch_image;
    friend class BPatch_module;

    std::unordered_map<std::string, BPatch_type *> typesByName;
    std::unordered_map<std::string, BPatch_type *> globalVarsByName;
    std::unordered_map<int, BPatch_type *> typesByID;

    ~BPatch_typeCollection();

    unsigned refcount;
    BPatch_typeCollection();

    static std::unordered_map< std::string, BPatch_typeCollection * > fileToTypesMap;

    bool dwarfParsed_;

public:
    static BPatch_typeCollection *getGlobalTypeCollection();
    static BPatch_typeCollection *getModTypeCollection(BPatch_module *mod);
    static void freeTypeCollection(BPatch_typeCollection *tc);

    bool dwarfParsed() { return dwarfParsed_; }
    void setDwarfParsed() { dwarfParsed_ = true; }

    BPatch_type	*findType(const char *name);
    BPatch_type	*findType(const int & ID);
    BPatch_type *findTypeLocal(const char *name);
    BPatch_type *findTypeLocal(const int &ID);
    void	addType(BPatch_type *type);
    void        addGlobalVariable(const char *name, BPatch_type *type)
      {globalVarsByName[name] = type;}

    BPatch_type * findOrCreateType( const int & ID );
    BPatch_type * addOrUpdateType( BPatch_type * type );

    BPatch_type *findVariableType(const char *name);
    
    void clearNumberedTypes();
};

class BPatch_builtInTypeCollection {
   
    std::unordered_map<std::string, BPatch_type *> builtInTypesByName;
    std::unordered_map<int, BPatch_type *> builtInTypesByID;
public:

    BPatch_builtInTypeCollection();
    ~BPatch_builtInTypeCollection();

    BPatch_type	*findBuiltInType(const char *name);
    BPatch_type	*findBuiltInType(const int & ID);
    void	addBuiltInType(BPatch_type *type);
   
};


#endif /* _BPatch_collections_h_ */



