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

bool debugPrint = false;
#ifndef dprintf
#define dprintf if (debugPrint) fprintf
#endif

class test_symtab_ser_funcs_Mutator : public SymtabMutator {
	std::vector<relocationEntry> relocations;
	std::vector<ExceptionBlock *> exceptions;
	std::vector<Type *> *stdtypes;
	std::vector<Type *> *builtintypes;
	std::vector<Region *> regions;
	std::vector<SymtabAPI::Module *> modules;
	std::vector<SymtabAPI::Function *> functions;
	std::vector<SymtabAPI::Variable *> variables;
	std::vector<Symbol *> symbols;

	void parse() THROW_SPEC (LocErr);

	static void location_list_report(const std::vector<VariableLocation> *l1, const std::vector<VariableLocation> *l2)
	{
		if (l1 && !l2) fprintf(stderr, "%s[%d]:  loc list discrep\n", FILE__, __LINE__);
		if (!l1 && l2) fprintf(stderr, "%s[%d]:  loc list discrep\n", FILE__, __LINE__);
		if (l1)
		{
			int max_length = l1->size();

			if (l2->size() > max_length) 
				max_length = l2->size();

			for (unsigned int i = 0; i < max_length; ++i)
			{
				const VariableLocation *loc1 = (i < l1->size()) ? (& (*l1)[i]) : NULL;
				const VariableLocation *loc2 = (i < l2->size()) ? (& (*l2)[i]) : NULL;

				if (loc1)
				{
					fprintf(stderr, "\t\t[%d, %d, %d, %l, %lu, %lu]\n", 
							loc1->stClass, loc1->refClass, loc1->reg, loc1->frameOffset, 
							loc1->hiPC, loc1->lowPC);
				}
				else
				{
					fprintf(stderr, "\t\t[no location at this index]\n");
				}

				if (loc2)
				{
					fprintf(stderr, "\t  ==  [%d, %d, %d, %l, %lu, %lu]\n", 
							loc2->stClass, loc2->refClass, loc2->reg, loc2->frameOffset, 
							loc2->hiPC, loc2->lowPC);
				}
				else
				{
					fprintf(stderr, "\t  ==  [no location at this index]\n");
				}
			}
		}
	}

	static void aggregate_report(const Aggregate &a1, const Aggregate &a2)
	{
		fprintf(stderr, "%s[%d]:  welcome to Aggregate report\n", FILE__, __LINE__);
	}

	static void localvar_report(const localVar &lv1, const localVar &lv2)
	{
		fprintf(stderr, "%s[%d]:  welcome to localVar report\n", FILE__, __LINE__);
	}

	static void relocation_report(const relocationEntry &r1, const relocationEntry &r2)
	{
		fprintf(stderr, "%s[%d]:  welcome to relcoation report\n", FILE__, __LINE__);
	}

	static void region_report(const Region &r1, const Region &r2)
	{
		fprintf(stderr, "%s[%d]:  welcome to region report\n", FILE__, __LINE__);
	}

	static void function_report(const Function &f1, const Function &f2)
	{
		fprintf(stderr, "%s[%d]:  NONEQUAL functions\n", FILE__, __LINE__);

		Type *t1 = f1.getReturnType();
		Type *t2 = f2.getReturnType();
		if (t1 && !t2) fprintf(stderr, "%s[%d]:  ret type discrep\n", FILE__, __LINE__);
		if (!t1 && t2) fprintf(stderr, "%s[%d]:  ret type discrep\n", FILE__, __LINE__);
		if (t1)
		{
			fprintf(stderr, "\t%d--%d\n", t1->getID(), t2->getID());
		}
		
		fprintf(stderr, "\t%d--%d\n", f1.getFramePtrRegnum(), f2.getFramePtrRegnum());

		std::vector<VariableLocation> *l1 = const_cast<Function &>(f1).getFramePtr();
		std::vector<VariableLocation> *l2 = const_cast<Function &>(f2).getFramePtr();

		location_list_report(l1, l2);

		const Aggregate &a1 = (const Aggregate &) f1;
		const Aggregate &a2 = (const Aggregate &) f2;
		aggregate_report(a1, a2);
	}

	static void module_report(const SymtabAPI::Module &m1, const SymtabAPI::Module &m2)
	{
		fprintf(stderr, "%s[%d]:  welcome to module report\n", FILE__, __LINE__);
	}

	static void variable_report(const Variable &v1, const Variable &v2)
	{
		fprintf(stderr, "%s[%d]:  NONEQUAL Variables\n", FILE__, __LINE__);

		Type *t1 = const_cast<Variable &>(v1).getType();
		Type *t2 = const_cast<Variable &>(v2).getType();
		if (t1 && !t2) fprintf(stderr, "%s[%d]:  type discrep\n", FILE__, __LINE__);
		if (!t1 && t2) fprintf(stderr, "%s[%d]:  type discrep\n", FILE__, __LINE__);
		if (t1)
		{
			fprintf(stderr, "\t%d--%d\n", t1->getID(), t2->getID());
		}

		const Aggregate &a1 = (const Aggregate &) v1;
		const Aggregate &a2 = (const Aggregate &) v2;
		aggregate_report(a1, a2);
	}

	static void symbol_report(const Symbol &s1, const Symbol &s2)
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

	typedef void (*symrep_t)(const Symbol &, const Symbol &);
	typedef void (*varrep_t)(const Variable &, const Variable &);
	typedef void (*funcrep_t)(const Function &, const Function &);
	typedef void (*modrep_t)(const SymtabAPI::Module &, const SymtabAPI::Module &);
	typedef void (*regrep_t)(const Region &, const Region &);
	typedef void (*relrep_t)(const relocationEntry &, const relocationEntry &);
	typedef void (*lvarrep_t)(const localVar &, const localVar &);

	template <class C>
		bool serialize_test(Symtab *st, C &control, void (*report)(const C &, const C &) ) THROW_SPEC (LocErr)
		{
			dprintf(stderr, "%s[%d]: welcome to serialize test for type %s\n",
					FILE__, __LINE__, typeid(C).name());

			Tempfile tf;
		std::string file(tf.getName());

		SerializerBase *sb_serializer_ptr;
		sb_serializer_ptr = nonpublic_make_bin_symtab_serializer(st, file);
		assert(sb_serializer_ptr);
		SerializerBase &sb_serializer = (SerializerBase &) *sb_serializer_ptr;

		dprintf(stderr, "%s[%d]:  before serialize\n", FILE__, __LINE__);
		
		Serializable *sable = &control;
		try 
		{
			sable->serialize(sb_serializer_ptr, NULL);
		}
		catch (const SerializerError &serr)
		{
			fprintf(stderr, "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
			fprintf(stderr, "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
			EFAIL("serialize failed\n");
		}

		dprintf(stderr, "%s[%d]:  after serialize\n", FILE__, __LINE__);
		fflush(NULL);

		nonpublic_free_bin_symtab_serializer(sb_serializer_ptr);


		C deserialize_result;
		SerializerBase *sb_deserializer_ptr;
		sb_deserializer_ptr = nonpublic_make_bin_symtab_deserializer(st, file);
		assert(sb_deserializer_ptr);
		SerializerBase &sb_deserializer = (SerializerBase &) *sb_deserializer_ptr;

		dprintf(stderr, "\n\n%s[%d]: about to deserialize: ---- %s\n\n",
				FILE__, __LINE__, typeid(C).name());

		try
		{
			deserialize_result.serialize(sb_deserializer_ptr, NULL);
		}
		catch (const SerializerError &serr)
		{
			fprintf(stderr, "%s[%d]:  deserializer function threw exception:\n", FILE__, __LINE__);
			fprintf(stderr, "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
			EFAIL("serialize failed\n");
		}

		nonpublic_free_bin_symtab_serializer(sb_deserializer_ptr);

		//  First check whether operator== (which must exist) returns equivalence

		if (!(deserialize_result == control))
		{
			if (report)
				(*report)(deserialize_result, control);

			EFAIL("deserialize failed\n");
		}

#if 0
		//  Next (since we can't trust operator==() 100%, do a raw mem compare

		if (memcmp(&control, &deserialize_result, sizeof(C)))
			EFAIL("deserialize and operator== failed\n");
#endif

		dprintf(stderr, "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);
	//  FIXME:  need to catch serializer errors here
	}

	template <class T>
	void serialize_test_vector(Symtab *st, std::vector<T> &vec, void (*report)(const T &, const T &) )
	{
		for (unsigned int i = 0; i < vec.size(); ++i)
		{
			T &t = vec[i];
			serialize_test(st, t, report);
		}
	}

	template <class T>
	void serialize_test_vector(Symtab *st, std::vector<T *> &vec, void (*report)(const T &, const T &) )
	{
		for (unsigned int i = 0; i < vec.size(); ++i)
		{
			T *t = vec[i];
			if (!t)
				EFAIL("null array elem\n");
			serialize_test(st, *t, report);
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

void test_symtab_ser_funcs_Mutator::parse() THROW_SPEC (LocErr)
{
	bool result = symtab->getFuncBindingTable(relocations);

#if !defined(os_aix_test)
	if (!result || !relocations.size() )
		EFAIL("relocations");
#endif

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

#if 0
		symrep_t sr = &symbol_report;
		varrep_t vr = &variable_report;
		modrep_t mr = &module_report;
		funcrep_t mr = &function_report;
		lvarrep_t mr = &localvar_report;
		regrep_t mr = &region_report;
		relgrep_t mr = &relocation_report;
#endif

		dprintf(stderr, "%s[%d]:  about to serialize test for variable:\n", FILE__, __LINE__);
		cerr << *variables[0] <<endl;
		serialize_test(symtab, *variables[0], &variable_report);
		serialize_test(symtab, *functions[0], &function_report);
		serialize_test(symtab, *symbols[0], &symbol_report);
		serialize_test(symtab, *modules[0], &module_report);
#if !defined (os_aix_test)
		serialize_test(symtab, relocations[0], &relocation_report);
#endif
		serialize_test(symtab, *regions[0], &region_report);
#if 0
		serialize_test_vector(symtab, symbols);
		serialize_test_vector(symtab, functions);
		serialize_test_vector(symtab, variables);
		serialize_test_vector(symtab, exceptions);
#if !defined (os_aix_test)
		serialize_test_vector(symtab, relocations);
#endif
		serialize_test_vector(symtab, regions);
		serialize_test_vector(symtab, modules);

		serialize_test(symtab, *symtab);
#endif
	}
	REPORT_EFAIL;

	return PASSED;
}
