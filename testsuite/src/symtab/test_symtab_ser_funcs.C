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

class test_symtab_ser_funcs_Mutator : public SymtabMutator {
#if !defined(SERIALIZATION_DISABLED)
	std::vector<relocationEntry> relocations;
	std::vector<ExceptionBlock *> exceptions;
	std::vector<Type *> *stdtypes;
	std::vector<Type *> *builtintypes;
	std::vector<Region *> regions;
	std::vector<SymtabAPI::Module *> modules;
	std::vector<SymtabAPI::Function *> functions;
	std::vector<SymtabAPI::Variable *> variables;
	std::vector<Symbol *> symbols;

	typeEnum *type_enum;
	typePointer *type_pointer;
	typeTypedef *type_typedef;
	typeStruct *type_struct;
	typeUnion *type_union;
	typeArray *type_array;
	typeRef *type_ref;
	typeCommon *type_common;
	typeFunction *type_function;

	void parse() THROW_SPEC (LocErr);

	static void location_list_report(const std::vector<VariableLocation> *l1, 
			const std::vector<VariableLocation> *l2)
	{
		if (l1 && !l2) logerror( "%s[%d]:  loc list discrep\n", FILE__, __LINE__);
		if (!l1 && l2) logerror( "%s[%d]:  loc list discrep\n", FILE__, __LINE__);
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
					logerror( "\t\t[%d, %d, %d, %l, %lu, %lu]\n", 
							loc1->stClass, loc1->refClass, loc1->reg, loc1->frameOffset, 
							loc1->hiPC, loc1->lowPC);
				}
				else
				{
					logerror( "\t\t[no location at this index]\n");
				}

				if (loc2)
				{
					logerror( "\t  ==  [%d, %d, %d, %l, %lu, %lu]\n", 
							loc2->stClass, loc2->refClass, loc2->reg, loc2->frameOffset, 
							loc2->hiPC, loc2->lowPC);
				}
				else
				{
					logerror( "\t  ==  [no location at this index]\n");
				}
			}
		}
	}

	static void namelist_report(const std::vector<std::string> &v1, 
			const std::vector<std::string> &v2, const char *label)
	{
		assert(label);

		logerror( "%s[%d]:  namelist '%s':\n", FILE__, __LINE__, label);

		int maxlen = v1.size() > v2.size() ? v1.size() : v2.size();

		for (unsigned int i = 0; i < maxlen; ++i)
		{
			const std::string &s1 = (i < v1.size()) ? v1[i] : std::string("no_string");
			const std::string &s2 = (i < v2.size()) ? v2[i] : std::string("no_string");
			logerror( "\t%s -- %s\n", s1.c_str(),  s2.c_str());
		}
	}

	static void aggregate_report(const Aggregate &a1, const Aggregate &a2)
	{
		logerror( "%s[%d]:  Aggregate:\n", FILE__, __LINE__);

		SymtabAPI::Module * m1 = a1.getModule();
		SymtabAPI::Module * m2 = a2.getModule();

		if (m1 && !m2)  logerror( "%s[%d]:  module discrep\n", FILE__, __LINE__);
		if (!m1 && m2)  logerror( "%s[%d]:  module discrep\n", FILE__, __LINE__);
		if (m1)
		{
			logerror( "%s[%d]:  moduleName:  %s -- %s\n", FILE__, __LINE__, 
					m1->fullName().c_str(), m2->fullName().c_str());
			logerror( "%s[%d]:  moduleOffset:  %p -- %p\n", FILE__, __LINE__, 
					m1->addr(), m2->addr());
		}

		const std::vector<std::string> & mn1 = const_cast<Aggregate &>(a1).getAllMangledNames();
		const std::vector<std::string> & mn2 = const_cast<Aggregate &>(a2).getAllMangledNames();
		const std::vector<std::string> & pn1 = const_cast<Aggregate &>(a1).getAllPrettyNames();
		const std::vector<std::string> & pn2 = const_cast<Aggregate &>(a2).getAllPrettyNames();
		const std::vector<std::string> & tn1 = const_cast<Aggregate &>(a1).getAllTypedNames();
		const std::vector<std::string> & tn2 = const_cast<Aggregate &>(a2).getAllTypedNames();

		namelist_report(mn1, mn2, "mangled");
		namelist_report(pn1, pn2, "pretty");
		namelist_report(tn1, tn2, "typed");

		std::vector<Symbol *> syms1;
		std::vector<Symbol *> syms2;

		bool r1 = a1.getSymbols(syms1);
		bool r2 = a2.getSymbols(syms1);

		if (r1 != r2) logerror( "%s[%d]:  getSymbols discrep\n", FILE__, __LINE__);

		int maxsym = syms1.size() > syms2.size() ? syms1.size() : syms2.size();

		logerror( "%s[%d]:  Symbol List:\n", FILE__, __LINE__);
		if (syms1.size() != syms2.size())
		{
			logerror( "%s[%d]:  size discrep:  [%ld -- %ld]\n", FILE__, __LINE__, syms1.size(), syms2.size());
		}

		for (unsigned int i = 0; i < maxsym; ++i)
		{
			Symbol *s1 = (i < syms1.size()) ? syms1[i] : NULL;
			Symbol *s2 = (i < syms2.size()) ? syms2[i] : NULL;

			logerror( "\t[%s-%p] -- [%s-%p]\n", 
					s1 ? s1->getName().c_str() : "no_sym",
					s1 ? s1->getOffset() : 0xdeadbeef,
					s2 ? s2->getName().c_str() : "no_sym",
					s2 ? s2->getOffset() : 0xdeadbeef);

		}
	}

	static void localvar_report(const localVar &lv1, const localVar &lv2)
	{
		logerror( "%s[%d]:  welcome to localVar report\n", FILE__, __LINE__);
	}

	static void relocation_report(const relocationEntry &r1, const relocationEntry &r2)
	{
		logerror( "%s[%d]:  Relcoation\n", FILE__, __LINE__);
		cerr << "     " << r1 << endl;
		cerr << "     " << r2 << endl;
	}

	static void type_report( const Type & ct1, const Type &ct2)
	{
		Type &t1 = const_cast<Type &>(ct1);
		Type &t2 = const_cast<Type &>(ct2);
		logerror( "%s[%d]:  Type report:\n", FILE__, __LINE__);
		logerror( "\t id: %d -- %d\n", t1.getID(), t2.getID());
		logerror( "\t size: %d -- %d\n", t1.getSize(), t2.getSize());
		logerror( "\t name: %s -- %s\n", t1.getName().c_str(), t2.getName().c_str());
		logerror( "\t dataclass: %d -- %d\n", (int)t1.getDataClass(), (int)t2.getDataClass());
	}

	static void derived_type_report( const derivedType & ct1, const derivedType &ct2)
	{
		type_report(ct1, ct2);

		derivedType &t1 = const_cast<derivedType &>(ct1);
		derivedType &t2 = const_cast<derivedType &>(ct2);

		Type *st1 = t1.getConstituentType();
		Type *st2 = t2.getConstituentType();

		if (st1 && !st2)
		{
			logerror( "%s[%d]:  inconsistent constituent types\n", FILE__, __LINE__);
			return;
		}

		if (!st1 && st2)
		{
			logerror( "%s[%d]:  inconsistent constituent types\n", FILE__, __LINE__);
			return;
		}

		if (!st1) 
			return;

		logerror( "%s[%d]:  derived from '%s' -- '%s'\n", FILE__, __LINE__, 
				st1->getName().c_str(), st2->getName().c_str());
	}

	static void type_pointer_report( const typePointer & ct1, const typePointer &ct2)
	{
		derived_type_report(ct1, ct2);
	}

	static void field_report(Field * f1, Field * &f2)
	{
		//Field *f1 = const_cast<Field *>(ct1);
		//Field *f2 = const_cast<Field *>(ct2);
		if (f1)
		{
			Type *t = f1->getType();
			std::string tname = t ? t->getName() : std::string("no_type");
			logerror( "[%s %s %u %d %s]", tname.c_str(), f1->getName().c_str(), 
					f1->getOffset(), f1->getSize(), visibility2Str(f1->getVisibility()));
		}
		if (f2)
		{
			Type *t = f2->getType();
			std::string tname = t ? t->getName() : std::string("no_type");
			logerror( "--[%s %s %u %d %s]", tname.c_str(), f2->getName().c_str(), 
					f2->getOffset(), f2->getSize(), visibility2Str(f2->getVisibility()));
		}
		logerror( "\n");
	}

	static void field_list_type_report( const fieldListType & ct1, const fieldListType &ct2)
	{
		type_report(ct1, ct2);

		fieldListType &t1 = const_cast<fieldListType &>(ct1);
		fieldListType &t2 = const_cast<fieldListType &>(ct2);

		std::vector<Field *> *c1 = t1.getComponents();
		std::vector<Field *> *c2 = t2.getComponents();

		if (c1 && !c2)
		{
			logerror( "%s[%d]: component discrep\n", FILE__, __LINE__);
		}

		if (!c1 && c2)
		{
			logerror( "%s[%d]: component discrep\n", FILE__, __LINE__);
		}

		if (c1)
		{

			unsigned int m = (c1->size() > c2->size()) ? c1->size() : c2->size();

			logerror( "%s[%d]:  components:\n", FILE__, __LINE__);

			for (unsigned int i = 0; i < m; ++i)
			{
				Field *f1 = (c1->size() > i) ? (*c1)[i] : NULL;
				Field *f2 = (c2->size() > i) ? (*c2)[i] : NULL;
				field_report(f1, f2);
			}
		}

		std::vector<Field *> *ff1 = t1.getFields();
		std::vector<Field *> *ff2 = t2.getFields();

		if (ff1 && !ff2)
		{
			logerror( "%s[%d]: field discrep\n", FILE__, __LINE__);
		}

		if (!ff1 && ff2)
		{
			logerror( "%s[%d]: field discrep\n", FILE__, __LINE__);
		}

		if (ff1)
		{
			unsigned int m = (ff1->size() > ff2->size()) ? ff1->size() : ff2->size();

			logerror( "%s[%d]:  fields:\n", FILE__, __LINE__);
			for (unsigned int i = 0; i < m; ++i)
			{
				Field *f1 = (ff1->size() > i) ? (*ff1)[i] : NULL;
				Field *f2 = (ff2->size() > i) ? (*ff2)[i] : NULL;
				field_report(f1, f2);
			}
		}
	}

	static void type_struct_report( const typeStruct & ct1, const typeStruct &ct2)
	{
		field_list_type_report(ct1, ct2);
	}

	static void type_union_report( const typeUnion & ct1, const typeUnion &ct2)
	{
		field_list_type_report(ct1, ct2);
	}

	static void ranged_type_report( const rangedType & ct1, const rangedType &ct2)
	{
		type_report(ct1, ct2);

		rangedType &t1 = const_cast<rangedType &>(ct1);
		rangedType &t2 = const_cast<rangedType &>(ct2);

		logerror( "%s[%d]:  ranged (high):  %lu -- %lu\n", FILE__, __LINE__, 
				t1.getHigh(), t2.getHigh());
		logerror( "%s[%d]:  ranged (low):  %lu -- %lu\n", FILE__, __LINE__, 
				t1.getLow(), t2.getLow());
	}

	static void type_array_report( const typeArray & ct1, const typeArray &ct2)
	{
		ranged_type_report(ct1, ct2);
		typeArray &t1 = const_cast<typeArray &>(ct1);
		typeArray &t2 = const_cast<typeArray &>(ct2);
		Type *st1 = t1.getBaseType();
		Type *st2 = t2.getBaseType();
		std::string tname1 = st1 ? st1->getName() : std::string("no_base_type");
		std::string tname2 = st2 ? st2->getName() : std::string("no_base_type");
		logerror( "%s[%d]:  array subtype: %s -- %s\n", FILE__, __LINE__, 
				tname1.c_str(), tname2.c_str());
	}

	static void type_enum_report( const typeEnum & ct1, const typeEnum &ct2)
	{
		type_report(ct1, ct2);

		typeEnum &t1 = const_cast<typeEnum &>(ct1);
		typeEnum &t2 = const_cast<typeEnum &>(ct2);

		std::vector<std::pair<std::string, int> > &consts1 = t1.getConstants();
		std::vector<std::pair<std::string, int> > &consts2 = t2.getConstants();
		int max_len = consts1.size() > consts2.size() ? consts1.size() : consts2.size();

		for (unsigned int i = 0; i < max_len; ++i)
		{
			if (i < consts1.size())
			{
				std::pair<std::string, int>  &c = consts1[i];
				logerror( "\t const %d: %s=%d:", i, c.first.c_str(), c.second);
			}
			else
			{
				logerror( "\t const %d: no-entry:", i);
			}

			if (i < consts2.size())
			{
				std::pair<std::string, int>  &c = consts2[i];
				logerror( "\t:%s=%d", c.first.c_str(), c.second);
			}
			else
			{
				logerror( "\t:no-entry", i);
			}
		}

	}

	static void region_report(const Region &r1, const Region &r2)
	{
		logerror( "%s[%d]:  NONEQUAL Regions\n", FILE__, __LINE__);
		logerror( "\t name: %s -- %s\n", 
				r1.getRegionName().c_str(), r2.getRegionName().c_str());
		logerror( "\t number: %u -- %u\n", 
				r1.getRegionNumber(), r2.getRegionNumber());
		logerror( "\t type: %u -- %u\n", 
				r1.getRegionType(), r2.getRegionType());
		logerror( "\t perms: %u -- %u\n", 
				r1.getRegionPermissions(), r2.getRegionPermissions());
		logerror( "\t disk offset: %p -- %p\n", 
				r1.getDiskOffset(), r2.getDiskOffset());
		logerror( "\t disk size: %lu -- %lu\n", 
				r1.getDiskSize(), r2.getDiskSize());
		logerror( "\t mem offset: %p -- %p\n", 
				r1.getMemOffset(), r2.getMemOffset());
		logerror( "\t isText: %s -- %s\n", 
				r1.isText() ? "true" : "false",
				r2.isText() ? "true" : "false");
		logerror( "\t isData: %s -- %s\n", 
				r1.isData() ? "true" : "false",
				r2.isData() ? "true" : "false");
		logerror( "\t isBSS: %s -- %s\n", 
				r1.isBSS() ? "true" : "false",
				r2.isBSS() ? "true" : "false");
		logerror( "\t isLoadable: %s -- %s\n", 
				r1.isLoadable() ? "true" : "false",
				r2.isLoadable() ? "true" : "false");
		logerror( "\t isDirty: %s -- %s\n", 
				r1.isDirty() ? "true" : "false",
				r2.isDirty() ? "true" : "false");
	}

	static void function_report(const Function &f1, const Function &f2)
	{
		logerror( "%s[%d]:  NONEQUAL functions\n", FILE__, __LINE__);

		Type *t1 = f1.getReturnType();
		Type *t2 = f2.getReturnType();

		if (t1 && !t2) logerror( "%s[%d]:  ret type discrep\n", FILE__, __LINE__);
		if (!t1 && t2) logerror( "%s[%d]:  ret type discrep\n", FILE__, __LINE__);

		if (t1)
		{
			logerror( "\t%d--%d\n", t1->getID(), t2->getID());
		}
		
		logerror( "\t%d--%d\n", f1.getFramePtrRegnum(), f2.getFramePtrRegnum());

		std::vector<VariableLocation> &l1 = const_cast<Function &>(f1).getFramePtr();
		std::vector<VariableLocation> &l2 = const_cast<Function &>(f2).getFramePtr();

		location_list_report(&l1, &l2);

		const Aggregate &a1 = (const Aggregate &) f1;
		const Aggregate &a2 = (const Aggregate &) f2;
		aggregate_report(a1, a2);
	}

	static void module_report(const SymtabAPI::Module &m1, const SymtabAPI::Module &m2)
	{
		logerror( "%s[%d]:  welcome to module report\n", FILE__, __LINE__);
	}

	static void variable_report(const Variable &v1, const Variable &v2)
	{
		logerror( "%s[%d]:  NONEQUAL Variables\n", FILE__, __LINE__);

		Type *t1 = const_cast<Variable &>(v1).getType();
		Type *t2 = const_cast<Variable &>(v2).getType();
		if (t1 && !t2) logerror( "%s[%d]:  type discrep\n", FILE__, __LINE__);
		if (!t1 && t2) logerror( "%s[%d]:  type discrep\n", FILE__, __LINE__);
		if (t1)
		{
			logerror( "\t%d--%d\n", t1->getID(), t2->getID());
		}

		const Aggregate &a1 = (const Aggregate &) v1;
		const Aggregate &a2 = (const Aggregate &) v2;
		aggregate_report(a1, a2);
	}

	static void symbol_report(const Symbol &s1, const Symbol &s2)
	{
		logerror( "%s[%d]:  NONEQUAL symbols:\n", FILE__, __LINE__);
		logerror( "\t%s--%s\n", s1.getModuleName().c_str(), s2.getModuleName().c_str());
		logerror( "\t%s--%s\n", s1.getMangledName().c_str(), s2.getMangledName().c_str());
		logerror( "\t%s--%s\n", s1.getPrettyName().c_str(), s2.getPrettyName().c_str());
		logerror( "\t%s--%s\n", s1.getTypedName().c_str(), s2.getTypedName().c_str());
		logerror( "\t%d--%d\n", s1.getType(), s2.getType());
		logerror( "\t%d--%d\n", s1.getLinkage(), s2.getLinkage());
		logerror( "\t%d--%d\n", s1.getVisibility(), s2.getVisibility());
		logerror( "\t%d--%d\n", s1.isInDynSymtab(), s2.isInDynSymtab());
		logerror( "\t%d--%d\n", s1.isAbsolute(), s2.isAbsolute());
		logerror( "\t%d--%d\n", s1.isFunction(), s2.isFunction());
		logerror( "\t%d--%d\n", s1.isVariable(), s2.isVariable());
		logerror( "\t%p--%p\n", s1.getAddr(), s2.getAddr());
		logerror( "\t%d--%d\n", s1.getSize(), s2.getSize());
		logerror( "\t%d--%d\n", s1.tag(), s2.tag());
		Region *r1 = s1.getSec();
		Region *r2 = s2.getSec();
		if (r1 && !r2) 
		{
			logerror( "%s[%d]:  region discrep\n", FILE__, __LINE__);
			return;
		}
		if (!r1 && r2) 
		{
			logerror( "%s[%d]:  region discrep\n", FILE__, __LINE__);
			return;
		}
#if 0
		for (unsigned long i = 0; i < regions.size(); ++i)
		{
			serialize_test(symtab, *(symbols[i]), &symbol_report);
		}
#endif
		if (r1)
		{
			logerror( "\t%p--%p\n", r1->getDiskOffset(), r2->getDiskOffset());
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
	void serialize_test(Symtab *st, C &control, void (*report)(const C &, const C &) ) THROW_SPEC (LocErr)
	{
		logerror( "%s[%d]: welcome to serialize test for type %s\n",
				FILE__, __LINE__, typeid(C).name());

		Tempfile tf;
		std::string file(tf.getName());

		SerializerBase* sb_serializer_ptr;
#if 1

		sb_serializer_ptr = nonpublic_make_bin_symtab_serializer(st, file);
#else
		SerializerBin<Symtab> *sb_serializer_ptr;
		sb_serializer_ptr = new SerializerBin<Symtab>(st, "SerializerBin2", file, sd_serialize, true);
		Symtab *test_st = sb_serializer_ptr->getScope();
		assert(test_st == st);

		sb_serializer_ptr = nonpublic_make_bin_serializer<Symtab>(st, file);
#endif
		assert(sb_serializer_ptr);

#if 0
		Dyninst::ScopedSerializerBase<Dyninst::SymtabAPI::Symtab> *ssb = dynamic_cast<Dyninst::ScopedSerializerBase<Dyninst::SymtabAPI::Symtab> *>(sb_serializer_ptr);
		if (!ssb)
			EFAIL("bad c++ inheritance hierarchy");
		logerror( "%s[%d]:  ssb name = %s\n", FILE__, __LINE__, typeid(ssb).name());
		Dyninst::SerializerBase *sb_serializer2 =  (Dyninst::SerializerBase *) sb_serializer_ptr;
#endif

		Dyninst::SerializerBase &sb_serializer = * (Dyninst::SerializerBase *) sb_serializer_ptr;

		logerror( "%s[%d]:  before serialize: &sb_serializer = %p\n", FILE__, __LINE__, &sb_serializer);

		Serializable *sable = &control;
		try 
		{
			sable->serialize(&sb_serializer);
		}
		catch (const SerializerError &serr)
		{
			logerror( "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
			logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
			EFAIL("serialize failed\n");
		}

		logerror( "%s[%d]:  after serialize\n", FILE__, __LINE__);
		fflush(NULL);

#if 1
		nonpublic_free_bin_symtab_serializer(sb_serializer_ptr);

#else
		SerializerBin<Symtab> *sbin = dynamic_cast<SerializerBin<Symtab> *>(sb_serializer_ptr);
		if (sbin)
		{
			delete(sbin);
		}
		else
			EFAIL("delete serializer");

		nonpublic_free_bin_serializer<Symtab>(sb_serializer_ptr);
#endif


		C deserialize_result;
		SerializerBase *sb_deserializer_ptr;
#if 1
		sb_deserializer_ptr = nonpublic_make_bin_symtab_deserializer(st, file);

#else
		SerializerBin<Symtab> *sb_deserializer_ptr;
		sb_deserializer_ptr = new SerializerBin<Symtab>(st, "DeserializerBin", file, sd_deserialize, true);
		test_st = sb_deserializer_ptr->getScope();
		assert(test_st == st);

		//sb_deserializer_ptr = nonpublic_make_bin_deserializer<Symtab>(&deserialize_result, file);
		sb_deserializer_ptr = nonpublic_make_bin_deserializer<Symtab>(st, file);
#endif
		assert(sb_deserializer_ptr);
#if 0
		ssb = dynamic_cast<ScopedSerializerBase<Symtab> *>(sb_deserializer_ptr);
		if (!ssb)
			EFAIL("bad c++ inheritance hierarchy");
#endif
		SerializerBase &sb_deserializer = (SerializerBase &) *sb_deserializer_ptr;

		logerror( "\n\n%s[%d]: about to deserialize: ---- %s\n\n",
				FILE__, __LINE__, typeid(C).name());

		try
		{
			Serializable *des_result_ptr= &deserialize_result;
			des_result_ptr->serialize(sb_deserializer_ptr, NULL);
			//deserialize_result.serialize(sb_deserializer_ptr, NULL);
		}
		catch (const SerializerError &serr)
		{
			logerror( "%s[%d]:  deserializer function threw exception:\n", FILE__, __LINE__);
			logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
			EFAIL("serialize failed\n");
		}

#if 1

		nonpublic_free_bin_symtab_serializer(sb_deserializer_ptr);
#else
		SerializerBin<Symtab> *dbin = dynamic_cast<SerializerBin<Symtab> *>(sb_deserializer_ptr);
		if (dbin)
		{
			delete(dbin);
		}
		else
			EFAIL("delete deserializer");

		nonpublic_free_bin_serializer<Symtab>(sb_deserializer_ptr);
#endif

		//  First check whether operator== (which must exist) returns equivalence

		if (!(deserialize_result == control))
		{
			if (report)
				(*report)(deserialize_result, control);

			logerror( "%s[%d]:  deserialize for %s failing\n", 
					FILE__, __LINE__, typeid(C).name());
			EFAIL("deserialize failed\n");
		}
#if 0
		//  Next (since we can't trust operator==() 100%, do a raw mem compare

		if (memcmp(&control, &deserialize_result, sizeof(C)))
			EFAIL("deserialize and operator== failed\n");
#endif

		logerror( "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);
	}

	template <class T>
		void serialize_test_vector(Symtab *st, std::vector<T> &vec, 
				void (*report)(const T &, const T &) )
		{
			for (unsigned int i = 0; i < vec.size(); ++i)
			{
				T &t = vec[i];
				serialize_test(st, t, report);
			}
		}

	template <class T>
		void serialize_test_vector(Symtab *st, std::vector<T *> &vec, 
				void (*report)(const T &, const T &) )
		{
			for (unsigned int i = 0; i < vec.size(); ++i)
			{
				T *t = vec[i];
				if (!t)
					EFAIL("null array elem\n");
				serialize_test(st, *t, report);
			}
		}
#endif
	public:

	test_symtab_ser_funcs_Mutator() { };
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_symtab_ser_funcs_factory()
{
	return new test_symtab_ser_funcs_Mutator();
}

#if !defined(SERIALIZATION_DISABLED)
void test_symtab_ser_funcs_Mutator::parse() THROW_SPEC (LocErr)
{
	bool result = symtab->getFuncBindingTable(relocations);

#if !defined(os_aix_test) && !defined (os_windows_test)
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

	Type *t = NULL;

	std::string tname = "sf_enum1";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (type_enum = t->getEnumType()))
	{
		EFAIL(tname.c_str());
	}

	tname = "sf_my_union";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (type_union = t->getUnionType()))
	{
		EFAIL(tname.c_str());
	}

	tname = "sf_mystruct";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (type_struct = t->getStructType()))
	{
		EFAIL(tname.c_str());
	}
	
	tname = "sf_int_alias_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (type_typedef = t->getTypedefType()))
	{
		EFAIL(tname.c_str());
	}

	typeTypedef *tt  = NULL;

	tname = "sf_int_array_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (tt = t->getTypedefType()))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (t = tt->getConstituentType()))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (type_array = t->getArrayType()))
	{
		EFAIL(tname.c_str());
	}

	tname = "sf_my_intptr_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (tt = t->getTypedefType()))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (t = tt->getConstituentType()))
	{
		EFAIL(tname.c_str());
	}

	if (NULL == (type_pointer = t->getPointerType()))
	{
		EFAIL(tname.c_str());
	}

	//  don't handle c++ or fortran yet...
	type_ref = NULL;
	type_common = NULL;
	type_function = NULL;
}
#endif

test_results_t test_symtab_ser_funcs_Mutator::executeTest()
{
//#if !defined (os_linux_test)
	return SKIPPED;
//#endif

#if !defined(SERIALIZATION_DISABLED)
	try 
	{
		parse();

		serialize_test(symtab, *symbols[0], &symbol_report);
		serialize_test(symtab, *regions[0], &region_report);
		for (unsigned long i = 0; i < symbols.size(); ++i)
		{
			if (NULL == symbols[i]->getRegion()) 
			{
				fprintf(stderr, "%s[%d]:  SKIPPING symbol %s with no region\n", FILE__, __LINE__, symbols[i]->getPrettyName().c_str());
				continue;
			}
			serialize_test(symtab, *(symbols[i]), &symbol_report);
		}
		serialize_test(symtab, *variables[0], &variable_report);
		serialize_test(symtab, *functions[0], &function_report);
		serialize_test(symtab, *modules[0], &module_report);
#if !defined (os_aix_test) && !defined (os_windows)
		serialize_test(symtab, relocations[0], &relocation_report);
#endif
		serialize_test(symtab, *type_enum, &type_enum_report);
		serialize_test(symtab, *type_pointer, &type_pointer_report);
		serialize_test(symtab, *type_struct, &type_struct_report);
		serialize_test(symtab, *type_union, &type_union_report);
		serialize_test(symtab, *type_array, &type_array_report);
		//  type_subrange??
		//  type_scalar??
		//  type_common_block??
#if 0
	typeEnum *type_enum;
	typePointer *type_pointer;
	typeTypedef *type_typedef;
	typeStruct *type_struct;
	typeUnion *type_union;
	typeArray *type_array;
#endif

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
#endif
}
