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
#include "Type.h"
#include "Module.h"
#include <iostream>

using namespace Dyninst;
using namespace SymtabAPI;

class test_type_info_Mutator : public SymtabMutator {
   std::vector<Type *> *std_types;
   std::vector<Type *> *builtin_types;
   test_results_t verify_basic_type_lists();
   bool verify_type(Type *t);
   bool verify_type_enum(typeEnum *t, std::vector<std::pair<std::string, int> > * = NULL);
   bool verify_type_pointer(typePointer *t, std::string * = NULL);
   bool verify_type_function(typeFunction *t);
   bool verify_type_subrange(typeSubrange *t);
   bool verify_type_array(typeArray *t, int * = NULL, int * = NULL, std::string * = NULL);
   bool verify_type_struct(typeStruct *t, 
		   std::vector<std::pair<std::string, std::string> > * = NULL, 
		   std::vector<std::pair<std::string, std::string> > * = NULL);
   bool verify_type_union(typeUnion *t, 
		   std::vector<std::pair<std::string, std::string> > * = NULL, 
		   std::vector<std::pair<std::string, std::string> > * = NULL);
   bool verify_type_scalar(typeScalar *t);
   bool verify_type_typedef(typeTypedef *t, std::string * = NULL);
   bool verify_field(Field *f);
   bool verify_field_list(fieldListType *t, 
		   std::vector<std::pair<std::string, std::string> > * = NULL, 
		   std::vector<std::pair<std::string, std::string> > * = NULL);

   bool got_type_enum;
   bool got_type_pointer;
   bool got_type_function;
   bool got_type_subrange;
   bool got_type_array;
   bool got_type_struct;
   bool got_type_union;
   bool got_type_scalar;
   bool got_type_typedef;

	public:
   test_type_info_Mutator() : 
	   std_types(NULL), 
	   builtin_types(NULL),
	   got_type_enum(false),
	   got_type_pointer(false),
	   got_type_function(false),
	   got_type_subrange(false),
	   got_type_array(false),
	   got_type_struct(false),
	   got_type_union(false),
	   got_type_scalar(false),
	   got_type_typedef(false)
   { }

   bool specific_type_tests();

   bool got_all_types()
   {
	   if (!got_type_enum)
	   {
		   fprintf(stderr, "%s[%d]:  enum was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_pointer)
	   {
		   fprintf(stderr, "%s[%d]:  pointer was missed\n", FILE__, __LINE__);
		   return false;
	   }

#if 0
	   //  I think typeFunction is c++ only
	   if (!got_type_function)
	   {
		   fprintf(stderr, "%s[%d]:  function was missed\n", FILE__, __LINE__);
		   return false;
	   }
#endif

	   if (!got_type_subrange)
	   {
		   fprintf(stderr, "%s[%d]:  subrange was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_array)
	   {
		   fprintf(stderr, "%s[%d]:  array was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_struct)
	   {
		   fprintf(stderr, "%s[%d]:  struct was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_union)
	   {
		   fprintf(stderr, "%s[%d]:  union was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_scalar)
	   {
		   fprintf(stderr, "%s[%d]:  scalar was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_typedef)
	   {
		   fprintf(stderr, "%s[%d]:  typedef was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   return true;
   }
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_type_info_factory()
{
   return new test_type_info_Mutator();
}

bool test_type_info_Mutator::verify_type_enum(typeEnum *t, std::vector<std::pair<std::string, int> >*vals)
{
	got_type_enum = true;
	std::string &tn = t->getName();
	//std::cerr << "verify_type_enum for " << tn << std::endl;

	std::vector<std::pair<std::string, int> > &constants = t->getConstants();

	if (!constants.size())
	{
		fprintf(stderr, "%s[%d]: empty enum %s\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	for (unsigned int i = 0; i < constants.size(); ++i)
	{
		if (constants[i].first.length() == 0)
		{
			fprintf(stderr, "%s[%d]:  enum %s has unnamed element\n", 
					FILE__, __LINE__, tn.c_str());
			return false;
		}
	}

	if (vals)
	{
		std::vector<std::pair<std::string, int> > &expected_vals = *vals;

		if (expected_vals.size() != constants.size())
		{
			fprintf(stderr, "%s[%d]:  differing sizes for values: %d vs %d\n", 
					FILE__, __LINE__, expected_vals.size(), constants.size());
			return false;
		}

		for (unsigned int i = 0; i < expected_vals.size(); ++i)
		{
			std::string &tag1 = expected_vals[i].first;
			std::string &tag2 = constants[i].first;
			int &val1 = expected_vals[i].second;
			int &val2 = constants[i].second;

			if (tag1 != tag2)
			{
				fprintf(stderr, "%s[%d]:  enum elems[%d] differ: %s != %s\n", 
						FILE__, __LINE__, i, tag1.c_str(), tag2.c_str());
				return false;
			}

			if (val1 != val2)
			{
				fprintf(stderr, "%s[%d]:  enum elems[%d] differ: %d != %d\n", 
						FILE__, __LINE__, i, val1, val2);
				return false;
			}
		}
	}

	return true;
}

bool test_type_info_Mutator::verify_type_pointer(typePointer *t, std::string *exp_base)
{
	got_type_pointer = true;
	std::string &tn = t->getName();
	Type *c = t->getConstituentType();
	if (!c)
	{
		fprintf(stderr, "%s[%d]:  NULL constituent type for type %s!\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (exp_base)
	{
		if (c->getName() != *exp_base)
		{
			fprintf(stderr, "%s[%d]:  unexpected base type %s (not %s) for type %s\n",
					FILE__, __LINE__, c->getName().c_str(), exp_base->c_str(), tn.c_str());
			return false;
		}
	}
	return true;
}

bool test_type_info_Mutator::verify_type_function(typeFunction *t)
{
	got_type_function = true;
	std::string &tn = t->getName();
	Type *retType = t->getReturnType();

	if (!retType)
	{
		fprintf(stderr, "%s[%d]:  func type %s has no return type\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	std::vector<Type *> &params = t->getParams();

	//  It is not an error to have zero params

	for (unsigned int i = 0; i < params.size(); ++i)
	{
		if (params[i] == NULL)
		{
			fprintf(stderr, "%s[%d]:  got NULL param type\n", FILE__, __LINE__);
			return false;
		}
	}

	return true;
}

bool test_type_info_Mutator::verify_type_subrange(typeSubrange *t)
{
	got_type_subrange = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_type_subrange for " << tn << std::endl;

	if (t->getLow() > t->getHigh())
	{
		fprintf(stderr, "%s[%d]:  bad range [%d--%d] for type %s!\n", 
				FILE__, __LINE__, t->getLow(), t->getHigh(), tn.c_str());
		return false;
	}

	return true;
}

bool test_type_info_Mutator::verify_type_array(typeArray *t, int *exp_low, int *exp_hi, 
		std::string *base_type_name)
{
	got_type_array = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_type_array for " << tn << std::endl;

	if (t->getLow() > t->getHigh())
	{
		fprintf(stderr, "%s[%d]:  bad ranges [%lu--%lu] for type %s!\n", 
				FILE__, __LINE__, t->getLow(), t->getHigh(), tn.c_str());
		return false;
	}

	Type *b = t->getBaseType();
	if (!b)
	{
		fprintf(stderr, "%s[%d]:  NULL base type for type %s!\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (exp_low)
	{
		if (*exp_low != t->getLow())
		{
			fprintf(stderr, "%s[%d]:  unexpected lowbound %d (not %d) for type %s!\n", 
					FILE__, __LINE__, t->getLow(), *exp_low, tn.c_str());
			return false;
		}
	}

	if (exp_hi)
	{
		if (*exp_hi != t->getHigh())
		{
			fprintf(stderr, "%s[%d]:  unexpected hibound %d (not %d) for type %s!\n", 
					FILE__, __LINE__, t->getHigh(), *exp_hi, tn.c_str());
			return false;
		}
	}

	if (base_type_name)
	{
		if (*base_type_name != b->getName())
		{
			fprintf(stderr, "%s[%d]:  unexpected basetype %s (not %s) for type %s!\n", 
					FILE__, __LINE__, b->getName().c_str(), base_type_name->c_str(), tn.c_str());
			return false;
		}
	}

	return true;
}

bool test_type_info_Mutator::verify_field(Field *f)
{
	if (!f)
	{
		fprintf(stderr, "%s[%d]:  NULL field\n", FILE__, __LINE__);
		return false;
	}

	if (0 == f->getName().length())
	{
		fprintf(stderr, "%s[%d]:  unnamed field\n", FILE__, __LINE__);
		return false;
	}

	Type *ft = f->getType();
	if (NULL == ft)
	{
		fprintf(stderr, "%s[%d]:  field %s has NULL type\n", FILE__, __LINE__, f->getName().c_str());
		return false;
	}

#if 0
	if (0 == f->getOffset())
	{
		//  this is probably ok
		fprintf(stderr, "%s[%d]: field %s has zero offset\n", FILE__, __LINE__, f->getName().c_str());
		return false;
	}

	if (visUnknown == f->getVisibility())
	{
		//  this is probably ok
		fprintf(stderr, "%s[%d]: field %s has unknown visibility\n", FILE__, __LINE__, f->getName().c_str());
		return false;
	}
#endif

	return true;
}

bool test_type_info_Mutator::verify_field_list(fieldListType *t, 
		std::vector<std::pair<std::string, std::string> > *comps, 
		std::vector<std::pair<std::string, std::string> > *efields)
{
	std::string &tn = t->getName();

	//std::cerr << "verify_field_list for " << tn << std::endl;

	std::vector<Field *> *components = t->getComponents();

	if (comps && !components)
	{
		fprintf(stderr, "%s[%d]:  no components for type %s\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (components)
	{
		for (unsigned int i = 0; i < components->size(); ++i)
		{
			Field *f = (*components)[i];
			if (!verify_field(f))
			{
				fprintf(stderr, "%s[%d]:  verify field failed for type %s\n", 
						FILE__, __LINE__, tn.c_str());
				return false;
			}
		}
	}

	std::vector<Field *> *fields = t->getFields();

	if (efields && !fields)
	{
		fprintf(stderr, "%s[%d]:  no fields for type %s\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (fields)
	{
		for (unsigned int i = 0; i < fields->size(); ++i)
		{
			Field *f = (*fields)[i];
			if (!verify_field(f))
			{
				fprintf(stderr, "%s[%d]:  verify field failed for type %s\n", 
						FILE__, __LINE__, tn.c_str());
				return false;
			}
		}

		if (efields)
		{
			std::vector<std::pair<std::string, std::string> > &expected_fields = *efields;

			if (efields->size() != fields->size())
			{
				fprintf(stderr, "%s[%d]:  WARNING:  differing sizes for expected fields\n", 
						FILE__, __LINE__);
				fprintf(stderr, "%s[%d]:  got %d, expected %d\n", FILE__, __LINE__, 
						fields->size(), efields->size());
			}

			bool err = false;
			for (unsigned int i = 0; i < fields->size(); ++i)
			{
				Field *f1 = (*fields)[i];
				std::string fieldname = f1->getName();
				std::string fieldtypename = f1->getType() ? f1->getType()->getName() : "";

				std::string expected_fieldname = 
					(expected_fields.size() > i) ? expected_fields[i].first 
					: std::string("range_error");
				std::string expected_fieldtypename = 
					(expected_fields.size() > i) ? expected_fields[i].second 
					: std::string("range_error");
				if (fieldtypename != expected_fieldname)
				{
					fprintf(stderr, "%s[%d]:  Field type %s not expected %s\n", FILE__,
							__LINE__, fieldtypename.c_str(), expected_fieldname.c_str());
					err = true;
				}

				if (fieldname != expected_fieldtypename)
				{
					fprintf(stderr, "%s[%d]:  Field type '%s' not expected '%s'\n", FILE__,
							__LINE__, fieldname.c_str(), expected_fieldtypename.c_str());
					err = true;
				}
			}

			if (err) 
				return false;
		}
	}

	return true;
}

bool test_type_info_Mutator::verify_type_struct(typeStruct *t, 
		std::vector<std::pair<std::string, std::string> > *ecomps, 
		std::vector<std::pair<std::string, std::string> > *efields)
{
	got_type_struct = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_struct for " << tn << std::endl;

	if (!verify_field_list(t, ecomps, efields))
	{
		fprintf(stderr, "%s[%d]:  verify struct %s failing\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	return true;
}

bool test_type_info_Mutator::verify_type_union(typeUnion *t, 
		std::vector<std::pair<std::string, std::string> > *ecomps, 
		std::vector<std::pair<std::string, std::string> > *efields)
{
	got_type_union = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_union for " << tn << std::endl;

	if (!verify_field_list(t, ecomps, efields))
	{
		fprintf(stderr, "%s[%d]:  verify union %s failing\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	return true;
}

bool test_type_info_Mutator::verify_type_scalar(typeScalar *t)
{
	got_type_scalar = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_scalar for " << tn << std::endl;

	//  uh... nothing to do here....  (maybe check sizes??)

	return true;
}

bool test_type_info_Mutator::verify_type_typedef(typeTypedef *t, std::string *tn_constituent)
{
	got_type_typedef = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_typedef for " << tn << std::endl;
	
	Type *c = t->getConstituentType();
	if (!c)
	{
		fprintf(stderr, "%s[%d]:  NULL constituent type for type %s!\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (tn_constituent)
	{
		if (c->getName() != *tn_constituent)
		{
			fprintf(stderr, "%s[%d]:  unexpected constituent type '%s' (not %s) for type %s!\n", 
					FILE__, __LINE__, c->getName().c_str(), tn_constituent->c_str(), tn.c_str());
			return false;
		}
	}

	return true;
}

bool test_type_info_Mutator::verify_type(Type *t)
{
	assert(t);
	std::string & tn = t->getName();

	//std::cerr << "considering type " << tn << std::endl;

	if (!t->getID())
	{
		fprintf(stderr, "%s[%d]:  type %s with zero id\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if ( 0 == tn.length())
	{
		fprintf(stderr, "%s[%d]:  unnamed type\n", FILE__, __LINE__);
		return false;
	}

	dataClass dc = t->getDataClass();

	if (dc == dataUnknownType)
	{
		fprintf(stderr, "%s[%d]:  type %s has bad data class\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (dc == dataNullType)
	{
		fprintf(stderr, "%s[%d]:  type %s has bad data class\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (t->getEnumType())
		return verify_type_enum(t->getEnumType());
	else if (t->getPointerType())
		return verify_type_pointer(t->getPointerType());
	else if (t->getFunctionType())
		return verify_type_function(t->getFunctionType());
	else if (t->getSubrangeType())
		return verify_type_subrange(t->getSubrangeType());
	else if (t->getArrayType())
		return verify_type_array(t->getArrayType());
	else if (t->getStructType())
		return verify_type_struct(t->getStructType());
	else if (t->getUnionType())
		return verify_type_union(t->getUnionType());
	else if (t->getScalarType())
		return verify_type_scalar(t->getScalarType());
	else if (t->getTypedefType())
		return verify_type_typedef(t->getTypedefType());
	else if (t->getCommonType())
	{
		// common blocks are fortran only
		// we don't test that here yet
		fprintf(stderr, "%s[%d]:  weird, got common type\n", FILE__, __LINE__);
		return true;
		//return verify_type_common(t->getCommonType());
	}
	else if (t->getRefType())
	{
		//  references are c++ only
		// we don't test that here yet
		fprintf(stderr, "%s[%d]:  weird, got reference type\n", FILE__, __LINE__);
		return true;
		//return verify_type_ref(t->getRefType());
	}
	else
	{
		fprintf(stderr, "%s[%d]: uknown type type for %s!\n", FILE__, __LINE__, tn.c_str());
	}
	return false;
}

bool test_type_info_Mutator::specific_type_tests()
{
	Type *t = NULL;

	std::string tname = "enum1";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		fprintf(stderr, "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeEnum *te = t->getEnumType();
	if (!te)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::vector<std::pair<std::string, int> > expected_vals;
	expected_vals.push_back(std::pair<std::string, int>(std::string("ef1_1"), 20));
	expected_vals.push_back(std::pair<std::string, int>(std::string("ef1_2"), 40));
	expected_vals.push_back(std::pair<std::string, int>(std::string("ef1_3"), 60));
	expected_vals.push_back(std::pair<std::string, int>(std::string("ef1_4"), 80));
	if (!verify_type_enum(te, &expected_vals)) 
		return false;

	tname = "my_union";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		fprintf(stderr, "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeUnion *tu = t->getUnionType();
	if (!tu)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::vector<std::pair<std::string, std::string> > expected_union_fields;
	expected_union_fields.push_back(std::pair<std::string, std::string>("char *", "my_str"));
	expected_union_fields.push_back(std::pair<std::string, std::string>("int", "my_int"));

	if (!verify_type_union(tu, NULL, &expected_union_fields)) 
		return false;

	tname = "mystruct";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		fprintf(stderr, "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeStruct *ts = t->getStructType();
	if (!ts)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::vector<std::pair<std::string, std::string> > expected_struct_fields;
	expected_struct_fields.push_back(std::pair<std::string, std::string>("int", "elem1"));
	expected_struct_fields.push_back(std::pair<std::string, std::string>("long int", "elem2"));
	expected_struct_fields.push_back(std::pair<std::string, std::string>("char", "elem3"));
	expected_struct_fields.push_back(std::pair<std::string, std::string>("float", "elem4"));

	if (!verify_type_struct(ts, NULL, &expected_struct_fields)) 
		return false;

	tname = "int_alias_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		fprintf(stderr, "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeTypedef *ttd = t->getTypedefType();
	if (!ttd)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::string expected_constituent_typename("int");
	if (!verify_type_typedef(ttd, &expected_constituent_typename)) 
		return false;

	tname = "int_array_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		fprintf(stderr, "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeTypedef *tt = t->getTypedefType();
	if (!tt)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	if (!verify_type_typedef(tt, NULL)) 
		return false;

	Type *tc = tt->getConstituentType();
	if (!tc)
	{
		fprintf(stderr, "%s[%d]:  %s: no constituent type\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeArray *ta = tc->getArrayType();
	if (!ta)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::string expected_array_base = "int";
	int expected_low = 0;
	int expected_hi = 255;
	if (!verify_type_array(ta, &expected_low, &expected_hi, &expected_array_base)) 
		return false;

	tname = "my_intptr_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		fprintf(stderr, "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	tt = t->getTypedefType();
	if (!tt)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	if (!verify_type_typedef(tt, NULL)) 
		return false;

	tc = tt->getConstituentType();
	if (!tc)
	{
		fprintf(stderr, "%s[%d]:  %s: no constituent type\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typePointer *tp = tc->getPointerType();
	if (!tp)
	{
		fprintf(stderr, "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::string expected_pointer_base = "int";
	if (!verify_type_pointer(tp, &expected_pointer_base)) 
		return false;

	return true;
}

test_results_t test_type_info_Mutator::verify_basic_type_lists()
{
   std_types = symtab->getAllstdTypes();
   builtin_types = symtab->getAllbuiltInTypes();

   if (!std_types || !std_types->size() )
   {
      logerror("[%s:%u] - Unable to find std types\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (!builtin_types || !builtin_types->size() )
   {
      logerror("[%s:%u] - Unable to find std types\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   for (unsigned int i = 0; i < std_types->size(); ++i)
   {
	   Type *t = (*std_types)[i];
	   if (!t)
	   {
		   fprintf(stderr, "%s[%d]:  NULL type returned to user\n", FILE__, __LINE__);
		   return FAILED;
	   }

	   if (!verify_type(t))
	   {
		   fprintf(stderr, "%s[%d]:  failing due to bad type\n", FILE__, __LINE__);
		   return FAILED;
	   }
   }

   for (unsigned int i = 0; i < builtin_types->size(); ++i)
   {
	   Type *t = (*builtin_types)[i];
	   if (!t)
	   {
		   fprintf(stderr, "%s[%d]:  NULL type returned to user\n", FILE__, __LINE__);
		   return FAILED;
	   }

	   if (!verify_type(t))
	   {
		   fprintf(stderr, "%s[%d]:  failing due to bad type\n", FILE__, __LINE__);
		   return FAILED;
	   }
   }

   std::vector<SymtabAPI::Module *> mods;
   bool result = symtab->getAllModules(mods);

   if (!result || !mods.size() )
   {
	   logerror("%s[%d]: Unable to getAllModules\n", FILE__, __LINE__);
	   return FAILED;
   }

   for (unsigned int i = 0; i < mods.size(); ++i)
   {
	   std::vector<Type *> *modtypes = mods[i]->getAllTypes();

	   if (!modtypes)
	   {
		   //  we try to look at all modules that have types
		   //  but not all do
		   //  Only fail if the module is one of ours

		   if (  mods[i]->fileName() == std::string("mutatee_util.c")
		      ||(mods[i]->fileName() == std::string("solo_mutatee_boilerplate.c"))
		      ||(mods[i]->fileName() == std::string("mutatee_driver.c")))
		   {
			   fprintf(stderr, "%s[%d]:  module %s has no types\n", FILE__, __LINE__, 
					   mods[i]->fileName().c_str());

			   return FAILED;
		   }
		   else
			   continue;
	   }

	   //fprintf(stderr, "%s[%d]:  examining types in module %s\n", FILE__, __LINE__,
	   //		   mods[i]->fileName().c_str());

	   for (unsigned int j = 0; j < modtypes->size(); ++j)
	   {
		   Type *t = (*modtypes)[j];
		   if (!t)
		   {
			   fprintf(stderr, "%s[%d]:  NULL type returned to user\n", FILE__, __LINE__);
			   return FAILED;
		   }

		   if (!verify_type(t))
		   {
			   fprintf(stderr, "%s[%d]:  failing due to bad type\n", FILE__, __LINE__);
			   return FAILED;
		   }
	   }
   }

   if (!got_all_types())
   {
	   fprintf(stderr, "%s[%d]:  did not test all types...  failing\n", FILE__, __LINE__);
	   return FAILED;
   }

   if (!specific_type_tests())
   {
	   fprintf(stderr, "%s[%d]:  specific type test failed... \n", FILE__, __LINE__);
	   return FAILED;
   }

   return PASSED;
}

test_results_t test_type_info_Mutator::executeTest()
{


	test_results_t ret = verify_basic_type_lists();
   return ret;
}

