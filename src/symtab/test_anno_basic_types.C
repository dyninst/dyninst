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

#include "Annotatable.h"

using namespace Dyninst;
//using namespace SymtabAPI;

class test_anno_basic_types_Mutator : public SymtabMutator 
{
   public:
      test_anno_basic_types_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_anno_basic_types_factory()
{
   return new test_anno_basic_types_Mutator();
}

class TestClass
{
   public:
      TestClass() {}
      
      int somestuff1;
      long somestuff2;
      float somestuff3;
      char *somestuff4;
};

class TestClassSparse : public TestClass, public AnnotatableSparse
{
   public:
      TestClassSparse() {}
      ~TestClassSparse() {}
};

class TestClassDense : public TestClass, public AnnotatableDense
{
   public:
   TestClassDense() {}
   ~TestClassDense() {}
};

template <class TC, class T>
void remove_anno(TC &tcs, const char *anno_prefix_to_use = NULL) THROW_SPEC (LocErr)
{
	std::string an(typeid(T).name());

	if (anno_prefix_to_use)
	{
		std::string prefix(anno_prefix_to_use);
		an = prefix + an;
	}

   AnnotationClass<T> my_ac(an);

   if (!tcs.removeAnnotation(my_ac))
      EFAIL("failed to remove annotation here");

   //  try to get the annotation now.
   //  if its still there, then we have a failure
   
   T *out = NULL;

   if (tcs.getAnnotation(out, my_ac))
      EFAIL("failed to get annotation here");
}

template <class TC, class T>
void verify_anno(TC &tcs, const T &test_val, 
		const char *anno_prefix_to_use = NULL) THROW_SPEC (LocErr)
{
	std::string an(typeid(T).name());

	if (anno_prefix_to_use)
	{
		std::string prefix(anno_prefix_to_use);
		an = prefix + an;
	}

   AnnotationClass<T> my_ac(an);

   T *out = NULL;

   if (!tcs.getAnnotation(out, my_ac))
      EFAIL("failed to get annotation here");

   if (!out)
      EFAIL("failed to get annotation here");

   if ((*out) != test_val)
      EFAIL("failed to get annotation here");
}

template <class TC, class T>
void add_get_and_verify_anno(TC &tcs, const T &test_val, 
		const char *anno_prefix_to_use = NULL) THROW_SPEC(LocErr)
{

   //  A very simple function that adds an annotation of type T to the given class
   //  then just verifies that it can retrieve the annotation and then checks that
   //  the value of the annotation is the same as was provided.

	std::string an(typeid(T).name());

	if (anno_prefix_to_use)
	{
		std::string prefix(anno_prefix_to_use);
		an = prefix + an;
	}

   AnnotationClass<T> my_ac(an);

   if (!tcs.addAnnotation(&test_val, my_ac))
      EFAIL("failed to add annotation here");

   T *out = NULL;

   if (!tcs.getAnnotation(out, my_ac))
      EFAIL("failed to get annotation here");

   if (!out)
      EFAIL("failed to get annotation here");

   if ((*out) != test_val)
      EFAIL("failed to get annotation here");

   //else 
   //{
   //   cerr <<  an << ":" << (*out) << " == " << test_val << endl;
   //}

}

template <class TC, class T>
void add_verify_dispatch(TC &tcs, const T &test_val, bool do_add, 
      const char *anno_prefix_to_use = NULL) THROW_SPEC (LocErr)
{
   if (do_add)
   {
      add_get_and_verify_anno(tcs, test_val, anno_prefix_to_use);
   }
   else
   {
      verify_anno(tcs, test_val, anno_prefix_to_use);
   }
}

template <class T>
void test_for_annotatable() THROW_SPEC (LocErr)
{
   T tc;
   bool do_add = false;

   do {
      //  First pass (do_add = true) adds and verifies annotation
      //  Second pass (do_add = false) just verifies existing annotation
      //  ...  ie, makes sure nothing got unexpectedly clobbered

      do_add = !do_add;

      add_verify_dispatch<T, int>(tc, -5000, do_add);
      add_verify_dispatch<T, unsigned int>(tc, 5001, do_add);
      add_verify_dispatch<T, char>(tc, -1*'c', do_add);
      add_verify_dispatch<T, unsigned char>(tc, 'd', do_add);
      add_verify_dispatch<T, short>(tc, -24, do_add);
      add_verify_dispatch<T, unsigned short>(tc, 50, do_add);
      add_verify_dispatch<T, long>(tc, -500000, do_add);
      add_verify_dispatch<T, unsigned long>(tc, 500001, do_add);
      add_verify_dispatch<T, float>(tc, -5e5, do_add);
      add_verify_dispatch<T, double>(tc, -5e50, do_add);

      //  Add more annotations of the same set of types, but under 
      //  different annotation names -- unspecified annotation names
      //  are later derived from the typename for this test

      add_verify_dispatch<T, int>(tc, -6000, do_add, "auxname1");
      add_verify_dispatch<T, unsigned int>(tc, 6001, do_add,"auxname2");
      add_verify_dispatch<T, char>(tc, -1*'e', do_add,"auxname3");
      add_verify_dispatch<T, unsigned char>(tc, 'f', do_add,"auxname4");
      add_verify_dispatch<T, short>(tc, -34, do_add,"auxname5");
      add_verify_dispatch<T, unsigned short>(tc, 60, do_add,"auxname6");
      add_verify_dispatch<T, long>(tc, -600000, do_add,"auxname7");
      add_verify_dispatch<T, unsigned long>(tc, 600001, do_add,"auxname8");
      add_verify_dispatch<T, float>(tc, -6e5, do_add,"auxname9");
      add_verify_dispatch<T, double>(tc, -6e50, do_add,"auxname10");

	  if (!do_add)
	  {
		  //  remove first set
		  remove_anno<T, int>(tc);
		  remove_anno<T, unsigned int>(tc);
		  remove_anno<T, char>(tc);
		  remove_anno<T, unsigned char>(tc);
		  remove_anno<T, short>(tc);
		  remove_anno<T, unsigned short>(tc);
		  remove_anno<T, long>(tc);
		  remove_anno<T, unsigned long>(tc);
		  remove_anno<T, float>(tc);
		  remove_anno<T, double>(tc);

		  //  verify that second set remains
		  add_verify_dispatch<T, int>(tc, -6000, do_add, "auxname1");
		  add_verify_dispatch<T, unsigned int>(tc, 6001, do_add,"auxname2");
		  add_verify_dispatch<T, char>(tc, -1*'e', do_add,"auxname3");
		  add_verify_dispatch<T, unsigned char>(tc, 'f', do_add,"auxname4");
		  add_verify_dispatch<T, short>(tc, -34, do_add,"auxname5");
		  add_verify_dispatch<T, unsigned short>(tc, 60, do_add,"auxname6");
		  add_verify_dispatch<T, long>(tc, -600000, do_add,"auxname7");
		  add_verify_dispatch<T, unsigned long>(tc, 600001, do_add,"auxname8");
		  add_verify_dispatch<T, float>(tc, -6e5, do_add,"auxname9");
		  add_verify_dispatch<T, double>(tc, -6e50, do_add,"auxname10");
	  }
   } while (do_add);
}

test_results_t test_anno_basic_types_Mutator::executeTest()
{

   //  Sparse annotation class should not add any size to child classes
   if (sizeof(TestClass) != sizeof(TestClassSparse))
   {
      fprintf(stderr, "%s[%d]:  ERROR, size creep in sparse annotation class\n", 
            FILE__, __LINE__);
      fprintf(stderr, "sizeof(TestClass) = %lu, sizeof(TestClassSparse) = %lu\n", 
            sizeof(TestClass), sizeof(TestClassSparse));
      return FAILED;
   }

   //  Dense annotation class should add size of one pointer
   if ((sizeof(TestClass) + sizeof(void *)) != sizeof(TestClassDense))
   {
      fprintf(stderr, "%s[%d]:  ERROR, size creep in dense annotation class\n", 
            FILE__, __LINE__);
      fprintf(stderr, "sizeof(TestClass) + sizeof(void *)= %lu, sizeof(TestClassDense) = %lu\n", 
            sizeof(TestClass) + sizeof(void *), sizeof(TestClassDense));
      return FAILED;
   }

   try 
   {
     test_for_annotatable<TestClassSparse>();
     test_for_annotatable<TestClassDense>();
   } REPORT_EFAIL;

   return PASSED;

}

