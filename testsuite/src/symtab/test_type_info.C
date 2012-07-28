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
#include "Type.h"
#include "Module.h"
#include <iostream>

using namespace Dyninst;
using namespace SymtabAPI;

class test_type_info_Mutator : public SymtabMutator {
   std::vector<Type *> *std_types;
   std::vector<Type *> *builtin_types;
   test_results_t verify_basic_type_lists();
   std::string execname;
   bool verify_type(Type *t);
   bool verify_type_enum(typeEnum *t, std::vector<std::pair<std::string, int> > * = NULL);
   bool verify_type_pointer(typePointer *t, std::string * = NULL);
   bool verify_type_function(typeFunction *t);
   bool verify_type_subrange(typeSubrange *t);
   bool verify_type_array(typeArray *t, int * = NULL, int * = NULL, std::string * = NULL);
   bool verify_type_struct(typeStruct *t, 
             std::vector<std::pair<std::string, std::string> > * = NULL, 
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

   supportedLanguages lang;
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
	   got_type_typedef(false),
	   lang(lang_Unknown)
   { }

   bool specific_type_tests();

   bool got_all_types()
   {
	   if (!got_type_enum)
	   {
		   logerror( "%s[%d]:  enum was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_pointer)
	   {
		   logerror( "%s[%d]:  pointer was missed\n", FILE__, __LINE__);
		   return false;
	   }

#if 0
	   //  I think typeFunction is c++ only
	   if (!got_type_function)
	   {
		   logerror( "%s[%d]:  function was missed\n", FILE__, __LINE__);
		   return false;
	   }
#endif

	   if (!got_type_subrange)
	   {
		   //  solaris CC does not appear to produce these
#if !defined(os_aix_test) && !defined(os_windows_test)
		   logerror( "%s[%d]:  subrange was missed\n", FILE__, __LINE__);
		   return false;
#endif
	   }

	   if (!got_type_array)
	   {
		   logerror( "%s[%d]:  array was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_struct)
	   {
		   logerror( "%s[%d]:  struct was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_union)
	   {
		   logerror( "%s[%d]:  union was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_scalar)
	   {
		   logerror( "%s[%d]:  scalar was missed\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!got_type_typedef)
	   {
#if !defined(os_windows_test)
		   logerror( "%s[%d]:  typedef was missed\n", FILE__, __LINE__);
		   return false;
#endif
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
		logerror( "%s[%d]: empty enum %s\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	for (unsigned int i = 0; i < constants.size(); ++i)
	{
		if (constants[i].first.length() == 0)
		{
			logerror( "%s[%d]:  enum %s has unnamed element\n", 
					FILE__, __LINE__, tn.c_str());
			return false;
		}
	}

	if (vals)
	{
		std::vector<std::pair<std::string, int> > &expected_vals = *vals;

		if (expected_vals.size() != constants.size())
		{
			logerror( "%s[%d]:  differing sizes for values: %d vs %d\n", 
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
				logerror( "%s[%d]:  enum elems[%d] differ: %s != %s\n", 
						FILE__, __LINE__, i, tag1.c_str(), tag2.c_str());
				return false;
			}

			if (val1 != val2)
			{
				logerror( "%s[%d]:  enum elems[%d] differ: %d != %d\n", 
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
		logerror( "%s[%d]:  NULL constituent type for type %s!\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (exp_base)
	{
		if (c->getName() != *exp_base)
		{
			logerror( "%s[%d]:  unexpected base type %s (not %s) for type %s\n",
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
		logerror( "%s[%d]:  func type %s has no return type\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	std::vector<Type *> &params = t->getParams();

	//  It is not an error to have zero params

	for (unsigned int i = 0; i < params.size(); ++i)
	{
		if (params[i] == NULL)
		{
			logerror( "%s[%d]:  got NULL param type\n", FILE__, __LINE__);
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
		logerror( "%s[%d]:  bad range [%d--%d] for type %s!\n", 
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
		//  special case (encountered w/ sun compilers -- if low bound is zero and
		//  highbound is -1, the array is not specified with a proper range, so
		//  ignore
		if (! (t->getLow() == 0L && t->getHigh() == -1L))
		{
			logerror( "%s[%d]:  bad ranges [%lu--%lu] for type %s!\n", 
					FILE__, __LINE__, t->getLow(), t->getHigh(), tn.c_str());
			return false;
		}
	}

	Type *b = t->getBaseType();
	if (!b)
	{
		logerror( "%s[%d]:  NULL base type for type %s!\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (exp_low)
	{
		if (*exp_low != t->getLow())
		{
			logerror( "%s[%d]:  unexpected lowbound %d (not %d) for type %s!\n", 
					FILE__, __LINE__, t->getLow(), *exp_low, tn.c_str());
			return false;
		}
	}

	if (exp_hi)
	{
		if (*exp_hi != t->getHigh())
		{
			logerror( "%s[%d]:  unexpected hibound %d (not %d) for type %s!\n", 
					FILE__, __LINE__, t->getHigh(), *exp_hi, tn.c_str());
			return false;
		}
	}

	if (base_type_name)
	{
		if (*base_type_name != b->getName())
		{
			logerror( "%s[%d]:  unexpected basetype %s (not %s) for type %s!\n", 
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
		logerror( "%s[%d]:  NULL field\n", FILE__, __LINE__);
		return false;
	}

	if (0 == f->getName().length())
	{
		logerror( "%s[%d]:  unnamed field\n", FILE__, __LINE__);
		return false;
	}

	Type *ft = f->getType();
	if (NULL == ft)
	{
		logerror( "%s[%d]:  field %s has NULL type\n", FILE__, __LINE__, f->getName().c_str());
		return false;
	}

#if 0
	if (0 == f->getOffset())
	{
		//  this is probably ok
		logerror( "%s[%d]: field %s has zero offset\n", FILE__, __LINE__, f->getName().c_str());
		return false;
	}

	if (visUnknown == f->getVisibility())
	{
		//  this is probably ok
		logerror( "%s[%d]: field %s has unknown visibility\n", FILE__, __LINE__, f->getName().c_str());
		return false;
	}
#endif

	return true;
}

bool test_type_info_Mutator::verify_field_list(fieldListType *t, 
		std::vector<std::pair<std::string, std::string> > *comps, 
      std::vector<std::pair<std::string, std::string> > *efields,
      std::vector<std::pair<std::string, std::string> > *afields)
{
	std::string &tn = t->getName();

	//std::cerr << "verify_field_list for " << tn << std::endl;

	std::vector<Field *> *components = t->getComponents();

	if (comps && !components)
	{
		logerror( "%s[%d]:  no components for type %s\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (components)
	{
		for (unsigned int i = 0; i < components->size(); ++i)
		{
			Field *f = (*components)[i];
			if (!verify_field(f))
			{
				logerror( "%s[%d]:  verify field failed for type %s\n", 
						FILE__, __LINE__, tn.c_str());
				return false;
			}
		}
	}

	std::vector<Field *> *fields = t->getFields();

	if (efields && !fields)
	{
		logerror( "%s[%d]:  no fields for type %s\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (fields)
	{
		for (unsigned int i = 0; i < fields->size(); ++i)
		{
			Field *f = (*fields)[i];
			if (!verify_field(f))
			{
				logerror( "%s[%d]:  verify field failed for type %s\n", 
						FILE__, __LINE__, tn.c_str());
				return false;
			}
		}

		if (efields)
		{
			std::vector<std::pair<std::string, std::string> > &expected_fields = *efields;

			//  We would be prudent here to use module language info to check
			//  whether this is c or c++, since that affects how many fields we have
			//  (c++ may have fields that represent functions, ctors, etc)
			//
			//  But alas, our language determination is still a bit inconsistent
			//  and can't really be treated as reliable
			//

			//if (lang = lang_CPlusPlus)
			//{
				//  C++ field lists may contain function definitions as well
				// as fields
			//	if (efields->size() < fields->size())
			//	{
			//		logerror( "%s[%d]:  bad sizes for expected fields\n", 
			//				FILE__, __LINE__);
			//		logerror( "%s[%d]:  got %d, expected %d\n", FILE__, __LINE__, 
			//				fields->size(), efields->size());
			//	}
			//}
			//else
			//{
			//	if (efields->size() != fields->size())
			//	{
			//		logerror( "%s[%d]:  WARNING:  differing sizes for expected fields\n", 
			//				FILE__, __LINE__);
			//		logerror( "%s[%d]:  got %d, expected %d\n", FILE__, __LINE__, 
			//				fields->size(), efields->size());
			//	}
			//}

			if (efields->size() > fields->size())
			{
				logerror( "%s[%d]:  bad sizes for expected fields for type %s\n", 
						FILE__, __LINE__, tn.c_str());
				logerror( "%s[%d]:  got %d, expected %d\n", FILE__, __LINE__, 
						fields->size(), efields->size());
				return false;
			}

			bool err = false;
			for (unsigned int i = 0; i < expected_fields.size(); ++i)
			{
				if (fields->size() <= i)
				{
					logerror( "%s[%d]:  cannot get %dth elem of %d size vec\n", 
							FILE__, __LINE__, i, fields->size());
					break;
				}

				Field *f1 = (*fields)[i];
				std::string fieldname = f1->getName();
				std::string fieldtypename = f1->getType() ? f1->getType()->getName() : "";
				Type *ft = f1->getType();

				//if (lang == lang_CPlusPlus && ft->getFunctionType())
				//{
				//	logerror( "%s[%d]:  skipping field %s\n", FILE__, __LINE__, 
				//			fieldname.c_str());
				//	continue;
				//}

				std::string expected_fieldname = 
					(expected_fields.size() > i) ? expected_fields[i].second 
					: std::string("range_error");
				std::string expected_fieldtypename = 
					(expected_fields.size() > i) ? expected_fields[i].first 
					: std::string("range_error");
            std::string alternate_fieldname = 
               (afields && afields->size() > i) ? (*afields)[i].second : "range_error";
            std::string alternate_fieldtypename = 
               (afields && afields->size() > i) ? (*afields)[i].first : "range_error";

				if (fieldtypename != expected_fieldtypename &&
                fieldtypename != alternate_fieldtypename)
				{
					logerror( "%s[%d]:  Field type '%s', not expected '%s'\n", FILE__,
							__LINE__, fieldtypename.c_str(), expected_fieldname.c_str());
					err = true;
				}

				if (fieldname != expected_fieldname &&
                fieldname != alternate_fieldname)
				{
					logerror( "%s[%d]:  Field type '%s' not expected '%s'\n", FILE__,
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
      std::vector<std::pair<std::string, std::string> > *efields,
      std::vector<std::pair<std::string, std::string> > *afields)
{
	got_type_struct = true;
	std::string &tn = t->getName();

	//std::cerr << "verify_struct for " << tn << std::endl;

	if (!verify_field_list(t, ecomps, efields, afields))
	{
		logerror( "%s[%d]:  verify struct %s failing\n", FILE__, __LINE__, tn.c_str());
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
		logerror( "%s[%d]:  verify union %s failing\n", FILE__, __LINE__, tn.c_str());
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
		logerror( "%s[%d]:  NULL constituent type for type %s!\n", 
				FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (tn_constituent)
	{
		if (c->getName() != *tn_constituent)
		{
			logerror( "%s[%d]:  unexpected constituent type '%s' (not %s) for type %s!\n", 
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
		logerror( "%s[%d]:  type %s with zero id\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if ( 0 == tn.length())
	{
		logerror( "%s[%d]:  unnamed %s type\n", FILE__, __LINE__, 
				dataClass2Str(t->getDataClass()));
		//return false;
	}

	dataClass dc = t->getDataClass();

	if (dc == dataUnknownType)
	{
		logerror( "%s[%d]:  type %s has bad data class\n", FILE__, __LINE__, tn.c_str());
		return false;
	}

	if (dc == dataNullType)
	{
		logerror( "%s[%d]:  type %s has bad data class\n", FILE__, __LINE__, tn.c_str());
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
		logerror( "%s[%d]:  weird, got common type\n", FILE__, __LINE__);
		return true;
		//return verify_type_common(t->getCommonType());
	}
	else if (t->getRefType())
	{
		//  references are c++ only
		// we don't test that here yet
		logerror( "%s[%d]:  weird, got reference type\n", FILE__, __LINE__);
		return true;
		//return verify_type_ref(t->getRefType());
	}
	else
	{
		logerror( "%s[%d]: uknown type type for %s!\n", FILE__, __LINE__, tn.c_str());
	}
	return false;
}

bool test_type_info_Mutator::specific_type_tests()
{
	Type *t = NULL;

	std::string tname = "enum1";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		logerror( "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeEnum *te = t->getEnumType();
	if (!te)
	{
		logerror( "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
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
		logerror( "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeUnion *tu = t->getUnionType();
	if (!tu)
	{
		logerror( "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::vector<std::pair<std::string, std::string> > expected_union_fields;
	expected_union_fields.push_back(std::pair<std::string, std::string>("float", "my_float"));
	expected_union_fields.push_back(std::pair<std::string, std::string>("int", "my_int"));

	if (!verify_type_union(tu, NULL, &expected_union_fields)) {
		logerror( "%s[%d]:  could not verify union\n", FILE__, __LINE__);
		return false;
   }

	tname = "mystruct";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		logerror( "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeStruct *ts = t->getStructType();
	if (!ts)
	{
		logerror( "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::vector<std::pair<std::string, std::string> > expected_struct_fields;
	std::vector<std::pair<std::string, std::string> > alternate_struct_fields;
	expected_struct_fields.push_back(std::pair<std::string, std::string>("int", "elem1"));
	alternate_struct_fields.push_back(std::pair<std::string, std::string>("int", "elem1"));
   
	expected_struct_fields.push_back(std::pair<std::string, std::string>("double", "elem2"));
	alternate_struct_fields.push_back(std::pair<std::string, std::string>("double", "elem2"));
	expected_struct_fields.push_back(std::pair<std::string, std::string>("char", "elem3"));
	alternate_struct_fields.push_back(std::pair<std::string, std::string>("signed char", "elem3"));
	expected_struct_fields.push_back(std::pair<std::string, std::string>("float", "elem4"));
	alternate_struct_fields.push_back(std::pair<std::string, std::string>("float", "elem4"));

	if (!verify_type_struct(ts, NULL, &expected_struct_fields, &alternate_struct_fields)) {
      logerror( "[%s:%u] - Could not verify struct\n");
		return false;
   }
#if !defined(os_windows_test)
	tname = "int_alias_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		logerror( "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	typeTypedef *ttd = t->getTypedefType();
	if (!ttd)
	{
		logerror( "%s[%d]:  %s: unexpected variety\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	std::string expected_constituent_typename("int");
	if (!verify_type_typedef(ttd, &expected_constituent_typename)) 
		return false;

	tname = "int_array_t";
	if (!symtab->findType(t, tname) || (NULL == t))
	{
		logerror( "%s[%d]:  could not find type %s\n", FILE__, __LINE__, tname.c_str());
		return false;
	}

	Type *tc = NULL;
	typeTypedef *tt = t->getTypedefType();
	if (!tt)
	{
		//  Caveat:  Solaris and gnu compilers differ here in how they emit the stab
		//  for the typedef array...  while it would be nice to have a consistent representation
		//  but it would involve creating "fake" placeholder typedefs...  or just
		//  modifying the test to be OK with either scenario....
		tc = t->getArrayType();
		if (NULL == tc)
		{
			logerror( "%s[%d]:  %s: unexpected variety %s\n", 
					FILE__, __LINE__, tname.c_str(), t->specificType().c_str());
			return false;
		}
	}
	else
	{
		if (!verify_type_typedef(tt, NULL)) 
			return false;

		tc = tt->getConstituentType();
	}
	if (!tc)
	{
		logerror( "%s[%d]:  %s: no constituent type\n", FILE__, __LINE__, tname.c_str());
		return false;
	}
	//logerror( "%s[%d]:  typedef %s constituent typename: %s:%s/id %d, typedef id = %d\n", FILE__, __LINE__, tt->getName().c_str(), tc->specificType().c_str(), tc->getName().c_str(), tc->getID(), tt->getID());

	typeArray *ta = tc->getArrayType();
	if (!ta)
	{
		logerror( "%s[%d]:  %s: unexpected variety: %s--%s\n", FILE__, __LINE__, tname.c_str(), tc->specificType().c_str(), tc->getName().c_str());
		typeTypedef *ttd = tc->getTypedefType();
		if (ttd)
		{
			Type *ttd_c = ttd->getConstituentType();
			logerror( "%s[%d]:  typedef constituent %s--%s\n", FILE__, __LINE__, ttd_c->getName().c_str(), ttd_c->specificType().c_str());
					}
		return false;
	}

	std::string expected_array_base = "int";
	int expected_low = 0;
	int expected_hi = 255;
	if (!verify_type_array(ta, &expected_low, &expected_hi, &expected_array_base)) 
	{
		logerror( "%s[%d]: failed to verify typeArray\n", FILE__, __LINE__);
		return false;
	}

	if (std::string::npos == execname.find("CC")) 
	{
		tname = "my_intptr_t";
		if (!symtab->findType(t, tname) || (NULL == t))
		{
			logerror( "%s[%d]:  could not find type %s\n", 
					FILE__, __LINE__, tname.c_str());
			return false;
		}

		tt = t->getTypedefType();
		if (!tt)
		{
			logerror( "%s[%d]:  %s: unexpected variety\n", 
					FILE__, __LINE__, tname.c_str());
			return false;
		}

		if (!verify_type_typedef(tt, NULL)) 
			return false;

		tc = tt->getConstituentType();
		if (!tc)
		{
			logerror( "%s[%d]:  %s: no constituent type\n", 
					FILE__, __LINE__, tname.c_str());
			return false;
		}

		typePointer *tp = tc->getPointerType();
		if (!tp)
		{
			logerror( "%s[%d]:  %s: unexpected variety: %s\n", 
					FILE__, __LINE__, tname.c_str(), dataClass2Str(tc->getDataClass()));
			return false;
		}

		std::string expected_pointer_base = "int";
		if (!verify_type_pointer(tp, &expected_pointer_base)) 
			return false;
	}
	else
	{
		logerror("%s[%d]:  skipped function pointer type verifiction for sun CC compiler\n", 
				FILE__, __LINE__);
	}
#endif
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
		   logerror( "%s[%d]:  NULL type returned to user\n", FILE__, __LINE__);
		   return FAILED;
	   }

	   if (!verify_type(t))
	   {
		   logerror( "%s[%d]:  failing due to bad type\n", FILE__, __LINE__);
		   return FAILED;
	   }
   }

   for (unsigned int i = 0; i < builtin_types->size(); ++i)
   {
	   Type *t = (*builtin_types)[i];
	   if (!t)
	   {
		   logerror( "%s[%d]:  NULL type returned to user\n", FILE__, __LINE__);
		   return FAILED;
	   }

	   if (!verify_type(t))
	   {
		   logerror( "%s[%d]:  failing due to bad type\n", FILE__, __LINE__);
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
			   logerror( "%s[%d]:  module %s has no types\n", FILE__, __LINE__, 
					   mods[i]->fileName().c_str());

			   return FAILED;
		   }
		   else
			   continue;
	   }

	   //logerror( "%s[%d]:  examining types in module %s\n", FILE__, __LINE__,
	   //		   mods[i]->fileName().c_str());

	   for (unsigned int j = 0; j < modtypes->size(); ++j)
	   {
		   Type *t = (*modtypes)[j];
		   if (!t)
		   {
			   logerror( "%s[%d]:  NULL type returned to user\n", FILE__, __LINE__);
			   return FAILED;
		   }

		   if (!verify_type(t))
		   {
			   logerror( "%s[%d]:  failing due to bad type\n", FILE__, __LINE__);
			   return FAILED;
		   }
	   }
   }


   if (!specific_type_tests())
   {
	   logerror( "%s[%d]:  specific type test failed... \n", FILE__, __LINE__);
	   return FAILED;
   }

   if (!got_all_types())
   {
	   logerror( "%s[%d]:  did not test all types...  failing\n", FILE__, __LINE__);
	   return FAILED;
   }

   return PASSED;
}

test_results_t test_type_info_Mutator::executeTest()
{

	if (createmode == DESERIALIZE)
		return SKIPPED;
#if defined (os_linux_test) && defined (arch_x86_test)
	if ((createmode == DESERIALIZE) && (compiler == std::string("g++")))
		return SKIPPED;
#endif
#if defined (os_aix_test) 
	if (createmode == DESERIALIZE)
		return SKIPPED;
#endif

	SymtabAPI::Module *mod = NULL;
	std::vector<SymtabAPI::Module *> mods;

	execname = symtab->name();
	
	if (!symtab->getAllModules(mods))
	{
		logerror( "%s[%d]:  failed to get all modules\n", FILE__, __LINE__);
		return FAILED;
	}

	for (unsigned int i = 0; i < mods.size(); ++i)
	{
		std::string mname = mods[i]->fileName();
		//logerror( "%s[%d]:  considering module %s\n", FILE__, __LINE__, mname.c_str());
		if (!strncmp("solo_mutatee", mname.c_str(), strlen("solo_mutatee")) ||	
		    !strncmp("test_type_info_mutatee", mname.c_str(), strlen("test_type_info_mutatee")))
		{
		   if (mod)
		      logerror( "%s[%d]:  FIXME\n", FILE__, __LINE__);
		   mod = mods[i];
		}
	}

	if (!mod)
	{
		logerror( "%s[%d]:  failed to find module\n", FILE__, __LINE__);
		return FAILED;
	}

	lang = mod->language();
	//logerror( "%s[%d]:  lang = %s\n", FILE__, __LINE__, supportedLanguages2Str(lang));
	test_results_t ret = verify_basic_type_lists();
   return ret;
}

