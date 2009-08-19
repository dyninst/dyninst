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
#include "Annotatable.h"
#include "Serialization.h"

#if 0
#include "Symtab.h"
#include "Symbol.h"
#include "Function.h"
#include "Variable.h"
#include "Module.h"
#include "Region.h"
#include "Type.h"
#include "Archive.h"
#endif

using namespace Dyninst;
using namespace SymtabAPI;

bool debugPrint = false;
#ifndef dprintf
#define dprintf if (debugPrint) fprintf
#endif

class AnnotateeBase : public Serializable
{
	public:

		int somestuff1;
		long somestuff2;
		Address somestuff3;
		float somestuff4;
		std::string somestuff5;

		AnnotateeBase() {}
		~AnnotateeBase() {}

		void print()
		{
			fprintf(stderr, "%s[%d]:  AnnotateeBase[%d, %lu, %p, %f, %s]\n", FILE__, __LINE__, somestuff1, somestuff2, (void *) somestuff3, somestuff4, somestuff5.c_str());
		}
		bool operator==(const AnnotateeBase &src)
		{
			if (somestuff1 != src.somestuff1) {
				fprintf(stderr, "%s[%d]:  %d != %d\n", FILE__, __LINE__, 
						somestuff1, src.somestuff1);
				return false;
			}
			if (somestuff2 != src.somestuff2) {
				fprintf(stderr, "%s[%d]:  %lu != %lu\n", FILE__, __LINE__, 
						somestuff2, src.somestuff2);
				return false;
			}
			if (somestuff3 != src.somestuff3) {
				fprintf(stderr, "%s[%d]:  %p != %p\n", FILE__, __LINE__, 
						somestuff3, src.somestuff3);
				return false;
			}
			if (somestuff4 != src.somestuff4) {
				fprintf(stderr, "%s[%d]:  %f != %f\n", FILE__, __LINE__, 
						somestuff4, src.somestuff4);
				return false;
			}
			if (somestuff5 != src.somestuff5) {
				fprintf(stderr, "%s[%d]:  %s != %s\n", FILE__, __LINE__, 
						somestuff5.c_str(), src.somestuff5.c_str());
				return false;
			}
			return true;
		}

		bool operator!=(const AnnotateeBase &src)
		{
			return (!(*this == src));
		}


		void serialize_impl(SerializerBase *sb, const char *) THROW_SPEC(SerializerError)
		{
			fprintf(stderr, "%s[%d]:  welcome to AnnotateeBase::serialize_impl: %s\n", 
					FILE__, __LINE__, sb->isInput() ? "deserialize" : "serialize");
			ifxml_start_element(sb, "AnnotateeBase");
			gtranslate(sb, somestuff1, "somestuff1");
			gtranslate(sb, somestuff2, "somestuff2");
			gtranslate(sb, somestuff3, "somestuff3");
			gtranslate(sb, somestuff4, "somestuff4");
			gtranslate(sb, somestuff5, "somestuff5");
			ifxml_end_element(sb, "AnnotateeBase");
			fprintf(stderr, "%s[%d]:  leaving to AnnotateeBase::serialize_impl\n", FILE__, __LINE__);
		}
};

#define INIT_OR_CHECK(VAR, VAL, SELECT) \
	(SELECT ? (NULL != &(VAR = VAL)) : !(VAR == VAL))

#define INIT true
#define CHECK false

int init_or_check_values(AnnotateeBase &ab, bool init_or_check)
{
	if (!INIT_OR_CHECK(ab.somestuff1, 5, init_or_check)) return 1;
	if (!INIT_OR_CHECK(ab.somestuff2, 55, init_or_check)) return 2;
	if (!INIT_OR_CHECK(ab.somestuff3, (Address)0x5, init_or_check)) return 3;
	if (!INIT_OR_CHECK(ab.somestuff4, 5.5, init_or_check)) return 4;
	if (!INIT_OR_CHECK(ab.somestuff5, std::string("5"), init_or_check)) return 5;

	return 0;
}
class AnnotateeSparse : public AnnotateeBase, public AnnotatableSparse 
{
	public:
		AnnotateeSparse() : AnnotateeBase() {}
		~AnnotateeSparse() {}
};

class AnnotateeDense : public AnnotateeBase, public AnnotatableDense 
{
	public:
		AnnotateeDense() : AnnotateeBase() {}
		~AnnotateeDense() {}
};

class MyAnnotationClass : public Serializable
{
	int val;
	public:

	MyAnnotationClass() {}
	MyAnnotationClass(int x) : val(x) {}
	~MyAnnotationClass() {}

	void serialize_impl(SerializerBase *sb, const char *) THROW_SPEC(SerializerError)
	{
		fprintf(stderr, "%s[%d]:  welcome to MyAnnotationClass::serialize_impl\n", FILE__, __LINE__);

		ifxml_start_element(sb, "MyAnnotationClass");

		gtranslate(sb, val, "val");
		ifxml_end_element(sb, "MyAnnotationClass");
		fprintf(stderr, "%s[%d]:  leaving to MyAnnotationClass::serialize_impl\n", FILE__, __LINE__);
	}

	bool operator==(MyAnnotationClass &src)
	{
		return src.val == val;
	}

	bool operator!=(MyAnnotationClass &src)
	{
		return !(*this == src);
	}
};

template <class A, class C>
void serialize_test1(A &annotatee, C &control) THROW_SPEC(LocErr)
{
	//  serialize_test1:  add annotation to class A (annotatee), serialize A.
	//  Then deserialize A and verify that (1) serialization worked for A proper,
	//  and (2) that added annotation was also properly serialized
	//
	//  This should work for both cases where C (the annotation class) is derived
	//  from Serializable and when it is a basic type
	if (dynamic_cast<AnnotatableSparse *>(&annotatee))
		fprintf(stderr, "%s[%d]:  annotatee is sparse\n", FILE__, __LINE__);
	else if (dynamic_cast<AnnotatableDense *>(&annotatee))
		fprintf(stderr, "%s[%d]:  annotatee is dense\n", FILE__, __LINE__);
	else fprintf(stderr, "%s[%d]:  ERROR:  annotatee is not annotatable\n", FILE__, __LINE__);

	dprintf(stderr, "%s[%d]: welcome to serialize test for annotating type %s with %s\n",
			FILE__, __LINE__, typeid(A).name(), typeid(C).name());
	Tempfile tf;
	std::string file(tf.getName());

	SerContext<A> scs(&annotatee);
	std::string sername = std::string("SerializerBin") + std::string(typeid(A).name());
	SerializerBin *serializer = new SerializerBin(&scs, sername.c_str(), 
			file, sd_serialize, true);

	if (!serializer)
		EFAIL("alloc serializer failed\n");


	//  Add the annotation
	std::string an(typeid(C).name());
	AnnotationClass<C> my_ac(an.c_str());

	if (!annotatee.addAnnotation(&control, my_ac))
		EFAIL("failed to add annotation");

	if (NULL == my_ac.getSerializeFunc())
	{
		EFAIL("annotation class has no serialize func");
	}
	//  Do the serialization
	//Serializable &a_serial = (Serializable &) annotatee;
	//a_serial.serialize(serializer, NULL);
	annotatee.serialize(serializer, NULL);

	fflush(NULL);
	dprintf(stderr, "%s[%d]:  after serialize\n", FILE__, __LINE__);


	A deserialize_result;
	Serializable &c_serial = (Serializable &) deserialize_result;
	SerializerBin *deserializer = NULL;
	SerContext<A> scs_d(&deserialize_result);
	try 
	{
		std::string sername = std::string("DeserializerBin") + std::string(typeid(A).name());
		deserializer = new SerializerBin(&scs_d, 
				sername.c_str(), 
				file, sd_deserialize, true);
	}
	catch (const SerializerError &serr)
	{
		fprintf(stderr, "%s[%d]:  serializer ctor threw exception:\n", FILE__, __LINE__);
		fprintf(stderr, "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		EFAIL("deserialize failed\n");
	}

	if (!deserializer)
		EFAIL("alloc serializer failed\n");

	//  Do the deserialize
	try 
	{
		c_serial.serialize(deserializer, NULL);
	}
	catch (const SerializerError &serr)
	{
		fprintf(stderr, "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
		fprintf(stderr, "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		EFAIL("deserialize failed\n");
	}

	if (annotatee != deserialize_result)
	{
		fprintf(stderr, "%s[%d]:  deserialize result is incorrect\n", FILE__, __LINE__);
		deserialize_result.print();
	}

	//  operator== will check basic class values, but we also need to verify annotations
	C *des_annotation;
	if (!deserialize_result.getAnnotation(des_annotation, my_ac))
		EFAIL("could not find annotation in deserialized result");

	if (*des_annotation != control)
		EFAIL("deserialized annotation did not match provided value");
}

class test_ser_anno_Mutator : public SymtabMutator {

#if 0
	template <class C>
		void serialize_test(Symtab *st, C &control, void (*report)(const C &, const C &) ) THROW_SPEC (LocErr)
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

			fprintf(stderr, "%s[%d]:  deserialize for %s failing\n", 
					FILE__, __LINE__, typeid(C).name());
			EFAIL("deserialize failed\n");
		}

#if 0
		//  Next (since we can't trust operator==() 100%, do a raw mem compare

		if (memcmp(&control, &deserialize_result, sizeof(C)))
			EFAIL("deserialize and operator== failed\n");
#endif

		dprintf(stderr, "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);
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

	test_ser_anno_Mutator() { };
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_ser_anno_factory()
{
	return new test_ser_anno_Mutator();
}

test_results_t test_ser_anno_Mutator::executeTest()
{

	try 
	{
		AnnotateeSparse as;
		if ( 0 != init_or_check_values(as, INIT)) 
		{
			fprintf(stderr, "%s[%d]:  weird, failed to init annotatee here\n", FILE__, __LINE__);
			return FAILED;
		}
		as.print();
		MyAnnotationClass mac(7);
		serialize_test1(as, mac);

		AnnotateeDense ad;
		if ( 0 != init_or_check_values(ad, INIT)) 
		{
			fprintf(stderr, "%s[%d]:  weird, failed to init annotatee here\n", FILE__, __LINE__);
			return FAILED;
		}
		ad.print();
		MyAnnotationClass mac2(8);
		serialize_test1(ad, mac2);
#if 0
		serialize_test(symtab, *variables[0], &variable_report);
		serialize_test(symtab, *functions[0], &function_report);
		serialize_test(symtab, *symbols[0], &symbol_report);
		serialize_test(symtab, *modules[0], &module_report);
#if !defined (os_aix_test) && !defined (os_windows)
		serialize_test(symtab, relocations[0], &relocation_report);
#endif
		serialize_test(symtab, *regions[0], &region_report);
		serialize_test(symtab, *type_enum, &type_enum_report);
		serialize_test(symtab, *type_pointer, &type_pointer_report);
		serialize_test(symtab, *type_struct, &type_struct_report);
		serialize_test(symtab, *type_union, &type_union_report);
		serialize_test(symtab, *type_array, &type_array_report);
#endif
	}
	REPORT_EFAIL;

	return PASSED;
}
