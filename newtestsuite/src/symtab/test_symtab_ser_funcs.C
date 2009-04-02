/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#include "symtab_comp.h"
#include "test_lib.h"

#include "Symtab.h"
#include "Symbol.h"
#include "Function.h"
#include "Variable.h"
#include "Module.h"
#include "Region.h"
#include "Type.h"
#include "Archive.h"
#include "Serialization.h"

using namespace Dyninst;
using namespace SymtabAPI;

extern SerializerBase *nonpublic_make_bin_symtab_serializer(Symtab *, std::string);
extern SerializerBase *nonpublic_make_bin_symtab_deserializer(Symtab *, std::string);
extern void nonpublic_free_bin_symtab_serializer(SerializerBase *);

class test_symtab_ser_funcs_Mutator : public SymtabMutator {
	std::vector<relocationEntry> relocations;
	std::vector<ExceptionBlock *> exceptions;
	std::vector<Type *> *stdtypes;
	std::vector<Type *> *builtintypes;
	std::vector<Region *> regions;
	std::vector<SymtabAPI::Module *> modules;
	std::vector<Function *> functions;
	std::vector<Variable *> variables;
	std::vector<Symbol *> symbols;

	void parse() throw(LocErr);

	void symbol_report(Symbol &s1, Symbol &s2)
	{
		fprintf(stderr, "%s[%d]:  NONEQUAL symbols:\n", FILE__, __LINE__);
		fprintf(stderr, "\t%s--%s\n", s1.getModuleName().c_str(), s2.getModuleName().c_str());
		fprintf(stderr, "\t%s--%s\n", s1.getMangledName().c_str(), s2.getMangledName().c_str());
		fprintf(stderr, "\t%s--%s\n", s1.getPrettyName().c_str(), s2.getPrettyName().c_str());
		fprintf(stderr, "\t%s--%s\n", s1.getTypedName().c_str(), s2.getTypedName().c_str());
		fprintf(stderr, "\t%d--%d\n", s1.getType(), s2.getType());
		fprintf(stderr, "\t%d--%d\n", s1.getLinkage(), s2.getLinkage());
		fprintf(stderr, "\t%d--%d\n", s1.getVisibility(), s2.getVisibility());
		fprintf(stderr, "\t%d--%d\n", s1.isInDynSymtab(), s2.isInDynSymtab());
		fprintf(stderr, "\t%d--%d\n", s1.isAbsolute(), s2.isAbsolute());
		fprintf(stderr, "\t%d--%d\n", s1.isFunction(), s2.isFunction());
		fprintf(stderr, "\t%d--%d\n", s1.isVariable(), s2.isVariable());
		fprintf(stderr, "\t%p--%p\n", s1.getAddr(), s2.getAddr());
		fprintf(stderr, "\t%d--%d\n", s1.getSize(), s2.getSize());
		fprintf(stderr, "\t%d--%d\n", s1.tag(), s2.tag());
		Region *r1 = s1.getSec();
		Region *r2 = s2.getSec();
		if (r1 && !r2) fprintf(stderr, "%s[%d]:  region discrep\n", FILE__, __LINE__);
		if (!r1 && r2) fprintf(stderr, "%s[%d]:  region discrep\n", FILE__, __LINE__);
		if (r1)
		{
			fprintf(stderr, "\t%p--%p\n", r1->getDiskOffset(), r2->getDiskOffset());
		}
	}

	template <class C>
		bool serialize_test(Symtab *st, C &control) throw (LocErr)
		{
			fprintf(stderr, "%s[%d]: welcome to serialize test\n",
					FILE__, __LINE__);

			Tempfile tf;
		std::string file(tf.getName());

		SerializerBase *sb_serializer_ptr;
		sb_serializer_ptr = nonpublic_make_bin_symtab_serializer(st, file);
		assert(sb_serializer_ptr);
		SerializerBase &sb_serializer = (SerializerBase &) *sb_serializer_ptr;

		control.serialize(&sb_serializer, NULL);

		nonpublic_free_bin_symtab_serializer(sb_serializer_ptr);

		fflush(NULL);

		C deserialize_result;
		SerializerBase *sb_deserializer_ptr;
		sb_deserializer_ptr = nonpublic_make_bin_symtab_deserializer(st, file);
		assert(sb_deserializer_ptr);
		SerializerBase &sb_deserializer = (SerializerBase &) *sb_deserializer_ptr;

		fprintf(stderr, "\n\n%s[%d]: about to deserialize\n\n",
				FILE__, __LINE__);

		deserialize_result.serialize(&sb_deserializer, NULL);

		nonpublic_free_bin_symtab_serializer(sb_deserializer_ptr);

		//  First check whether operator== (which must exist) returns equivalence

		if (!(deserialize_result == control))
		{
			if (typeid(C) == typeid(Dyninst::SymtabAPI::Symbol))
				symbol_report(deserialize_result, control);
			EFAIL("deserialize failed\n");
		}

#if 0
		//  Next (since we can't trust operator==() 100%, do a raw mem compare

		if (memcmp(&control, &deserialize_result, sizeof(C)))
			EFAIL("deserialize and operator== failed\n");
#endif

		fprintf(stderr, "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);
	}

	template <class T>
	void serialize_test_vector(Symtab *st, std::vector<T> &vec)
	{
		for (unsigned int i = 0; i < vec.size(); ++i)
		{
			T &t = vec[i];
			serialize_test(st, t);
		}
	}

	template <class T>
	void serialize_test_vector(Symtab *st, std::vector<T *> &vec)
	{
		for (unsigned int i = 0; i < vec.size(); ++i)
		{
			T *t = vec[i];
			if (!t)
				EFAIL("null array elem\n");
			serialize_test(st, *t);
		}
	}

	public:

	test_symtab_ser_funcs_Mutator() { };
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_symtab_ser_funcs_factory()
{
	return new test_symtab_ser_funcs_Mutator();
}

void test_symtab_ser_funcs_Mutator::parse() throw (LocErr)
{
	bool result = symtab->getFuncBindingTable(relocations);

	if (!result || !relocations.size() )
		EFAIL("relocations");

#if 0
	//  need to make this a c++ test to get exceptions
	result = symtab->getAllExceptions(exceptions);

	if (!result || !exceptions.size() )
		EFAIL("exceptions");
#endif

	result = symtab->getAllRegions(regions);

	if (!result || !regions.size() )
		EFAIL("regions");

	result = symtab->getAllModules(modules);

	if (!result || !modules.size() )
		EFAIL("modules");

	result = symtab->getAllVariables(variables);

	if (!result || !variables.size() )
		EFAIL("variables");

	result = symtab->getAllFunctions(functions);

	if (!result || !functions.size() )
		EFAIL("functions");

	result = symtab->getAllDefinedSymbols(symbols);

	if (!result || !symbols.size() )
		EFAIL("symbols");

	stdtypes = symtab->getAllstdTypes();

	if (!stdtypes)
		EFAIL("stdtypes");

	builtintypes = symtab->getAllbuiltInTypes();

	if (!builtintypes)
	{
		EFAIL("builtintypes");
	}
}

test_results_t test_symtab_ser_funcs_Mutator::executeTest()
{
	try 
	{
		parse();

		serialize_test(symtab, *symbols[0]);
#if 0
		serialize_test_vector(symtab, symbols);
		serialize_test_vector(symtab, functions);
		serialize_test_vector(symtab, variables);
		serialize_test_vector(symtab, exceptions);
		serialize_test_vector(symtab, relocations);
		serialize_test_vector(symtab, regions);
		serialize_test_vector(symtab, modules);

		serialize_test(symtab, *symtab);
#endif
	}
	REPORT_EFAIL;

	return PASSED;
}
