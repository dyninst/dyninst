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

#ifndef _BPatch_collections_h_
#define _BPatch_collections_h_

#include "BPatch_type.h"     //type and localVar
#include "common/h/String.h"
#include "common/h/Dictionary.h"
#include "common/h/List.h"


/*
 * This class contains a collection of local variables.
 * Each function will have one of these objects associated with it.
 * This object will store all the local variables within this function.
 * Note: This class is unaware of scope.
 */
class BPatch_localVarCollection{
  
  dictionary_hash<pdstring, BPatch_localVar *> localVariablesByName;

public:
  BPatch_localVarCollection(): localVariablesByName(pdstring::hash){};
  ~BPatch_localVarCollection();

  void addLocalVar(BPatch_localVar * var);
  BPatch_localVar * findLocalVar(const char *name);
  BPatch_Vector<BPatch_localVar *> *getAllVars();  
};
  


/*
 * Due to DWARF weirdness, this can be shared between multiple BPatch_modules.
 * So we reference-count to make life easier.
 */
class BPatch_typeCollection {
    friend class BPatch_image;
    friend class BPatch_module;

    dictionary_hash<pdstring, BPatch_type *> typesByName;
    dictionary_hash<pdstring, BPatch_type *> globalVarsByName;
    dictionary_hash<int, BPatch_type *> typesByID;

    ~BPatch_typeCollection();

    unsigned refcount;
    BPatch_typeCollection();

    // DWARF:
    /* Cache type collections on a per-image basis.  (Since
       BPatch_functions are solitons, we don't have to cache them.) */
    static dictionary_hash< pdstring, BPatch_typeCollection * > fileToTypesMap;

    // DWARF...
    bool dwarfParsed_;

public:
    static BPatch_typeCollection *getGlobalTypeCollection();
    static BPatch_typeCollection *getModTypeCollection(BPatch_module *mod);
    static void freeTypeCollection(BPatch_typeCollection *tc);

    // DWARF...
    bool dwarfParsed() { return dwarfParsed_; }
    void setDwarfParsed() { dwarfParsed_ = true; }

    BPatch_type	*findType(const char *name);
    BPatch_type	*findType(const int & ID);
    BPatch_type *findTypeLocal(const char *name);
    BPatch_type *findTypeLocal(const int &ID);
    void	addType(BPatch_type *type);
    void        addGlobalVariable(const char *name, BPatch_type *type)
      {globalVarsByName[name] = type;}

    /* Some debug formats allow forward references.  Rather than
       fill in forward in a second pass, generate placeholder
       types, and fill them in as we go.  Because we require
       One True Pointer for each type (in parseStab.C), when
       updating a type, return that One True Pointer. */
    BPatch_type * findOrCreateType( const int & ID );
    BPatch_type * addOrUpdateType( BPatch_type * type );

    BPatch_type *findVariableType(const char *name);
    
    void clearNumberedTypes();
};

/*
 * This class defines the collection for the built-in Types
 * gnu ( and AIX??) use negative numbers to define other types
 * in terms of these built-in types.
 * This collection is global and built in the BPatch_image constructor.
 * This means that only one collection of built-in types is made
 * per image.  jdd 4/21/99
 *
 */

class BPatch_builtInTypeCollection {
   
    dictionary_hash<pdstring, BPatch_type *> builtInTypesByName;
    dictionary_hash<int, BPatch_type *> builtInTypesByID;
public:

    BPatch_builtInTypeCollection();
    ~BPatch_builtInTypeCollection();

    BPatch_type	*findBuiltInType(const char *name);
    BPatch_type	*findBuiltInType(const int & ID);
    void	addBuiltInType(BPatch_type *type);
   
};


#endif /* _BPatch_collections_h_ */



