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
#include "Serialization.h"
#include <iostream>

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

class test_ser_anno_Mutator : public SymtabMutator {

	public:

	test_ser_anno_Mutator() { };
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_ser_anno_factory()
{
	return new test_ser_anno_Mutator();
}

#if !defined(SERIALIZATION_DISABLED)
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
			logerror( "%s[%d]:  AnnotateeBase[%d, %lu, %p, %f, %s]\n", 
					FILE__, __LINE__, somestuff1, somestuff2, (void *) somestuff3, 
					somestuff4, somestuff5.c_str());
		}

		bool operator==(const AnnotateeBase &src)
		{
			if (somestuff1 != src.somestuff1) 
			{
				logerror( "%s[%d]:  %d != %d\n", FILE__, __LINE__, 
						somestuff1, src.somestuff1);
				return false;
			}
			if (somestuff2 != src.somestuff2) 
			{
				logerror( "%s[%d]:  %lu != %lu\n", FILE__, __LINE__, 
						somestuff2, src.somestuff2);
				return false;
			}
			if (somestuff3 != src.somestuff3) 
			{
				logerror( "%s[%d]:  %p != %p\n", FILE__, __LINE__, 
						somestuff3, src.somestuff3);
				return false;
			}
			if (somestuff4 != src.somestuff4) 
			{
				logerror( "%s[%d]:  %f != %f\n", FILE__, __LINE__, 
						somestuff4, src.somestuff4);
				return false;
			}
			if (somestuff5 != src.somestuff5) 
			{
				logerror( "%s[%d]:  %s != %s\n", FILE__, __LINE__, 
						somestuff5.c_str(), src.somestuff5.c_str());
				return false;
			}
			return true;
		}

		bool operator!=(const AnnotateeBase &src)
		{
			return (!(*this == src));
		}


		Serializable *serialize_impl(SerializerBase *sb, const char *) THROW_SPEC(SerializerError)
		{
			logerror("%s[%d]:  welcome to AnnotateeBase::serialize_impl: %s\n", 
					FILE__, __LINE__, is_input(sb) ? "deserialize" : "serialize");
			//ifxml_start_element(sb, "AnnotateeBase");
			gtranslate(sb, somestuff1, "somestuff1");
			gtranslate(sb, somestuff2, "somestuff2");
			gtranslate(sb, somestuff3, "somestuff3");
			gtranslate(sb, somestuff4, "somestuff4");
			gtranslate(sb, somestuff5, "somestuff5");
			//ifxml_end_element(sb, "AnnotateeBase");
			logerror("%s[%d]:  leaving to AnnotateeBase::serialize_impl\n", 
					FILE__, __LINE__);
			return NULL;
		}
};

#define INIT_OR_CHECK(VAR, VAL, SELECT) \
	(SELECT ? (NULL != &(VAR = VAL)) : (VAR == VAL))

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

#if 0
template <class T>
class AnnotateeContainer : public Serializable, public AnnotatableSparse
{
	std::vector<T> elements;
	public:
	AnnotateeContainer() {}
	void addElement(T it) {elements.push_back(it);}

	void serialize_impl(SerializerBase *sb, const char *) THROW_SPEC(SerializerError)
	{
		logerror("%s[%d]:  welcome to MyAnnotationClass::serialize_impl: val = %d, id = %d\n", 
				FILE__, __LINE__, val, getID());
		gtranslate(sb, elements, "elements");
	}

	bool operator==(AnnotateeContainer &src)
	{
		if (elements.size() != src.elements.size())
		{
			logerror( "%s[%d]:  size mismatch: %d--%d\n", 
					FILE__, __LINE__, elements.size(), src.elements.size());
			return false;
		}

		for (unsigned int i = 0; i < elements.size(); ++i)
			if (elements[i] != src.elements[i]) 
				return false;

		return true;
	}

	bool operator!=(AnnotateeContainer &src)
	{
		return !(*this == src);
	}
};

template <class T, class A>
void test4(AnnotateeContainer<T> &cont, std::vector<T> &elems, std::vector<A> &annos)
{
	assert(elems.size() == annos.size());
	for (unsigned int i = 0; i < annos.size(); ++i)
	{
		cont.addElement(elems[i]);
	}
}
#endif

class MyAnnotationClass : public Serializable
{
	int val;
	public:

	MyAnnotationClass() : val(666){}
	MyAnnotationClass(int x) : val(x) {}
	~MyAnnotationClass() {}


	Serializable *serialize_impl(SerializerBase *sb, const char *) THROW_SPEC(SerializerError)
	{
		logerror("%s[%d]:  welcome to MyAnnotationClass::serialize_impl: val = %d, id = %d\n", 
				FILE__, __LINE__, val, getID());

		ifxml_start_element(sb, "MyAnnotationClass");
		gtranslate(sb, val, "val");
		ifxml_end_element(sb, "MyAnnotationClass");

		logerror("%s[%d]:  leaving to MyAnnotationClass::serialize_impl, val = %d\n", 
				FILE__, __LINE__, val);
		return NULL;
	}

	bool operator==(MyAnnotationClass &src)
	{
		return src.val == val;
	}

	bool operator!=(MyAnnotationClass &src)
	{
		return !(*this == src);
	}
	friend std::ostream &operator<<(std::ostream &os, const MyAnnotationClass &mac);
	void print()
	{
		logerror( "%s[%d]:  MyAnnotateeClass: val = %d\n", FILE__, __LINE__, val);
	}
};

std::ostream &operator<<(std::ostream &os, const MyAnnotationClass &mac)
{
	return os << mac.val;
}

template <class T>
class MyAnnotationContainer : public AnnotationContainer<T>
{
	std::vector<T> vec;
	bool addItem_impl(T i) {vec.push_back(i); return true;}

	public:

	MyAnnotationContainer() : AnnotationContainer<T>() 
	{
		//logerror( "%s[%d]:  MyAnnotationContainer ctor: %p\n", FILE__, __LINE__, this);
	}

	bool operator==(MyAnnotationContainer &src)
	{
		if (vec.size() != src.vec.size())
		{
			logerror( "%s[%d]:  size mismatch: %d--%d\n", 
					FILE__, __LINE__, vec.size(), src.vec.size());
			return false;
		}

		for (unsigned int i = 0; i < vec.size(); ++i)
			if (vec[i] != src.vec[i]) 
				return false;

#if 0
		logerror( "%s[%d]:  annotation container elem0 is %d\n", 
				FILE__, __LINE__, vec.size() ? vec[0] : -1);
#endif

		return true;
	}

	bool operator!=(MyAnnotationContainer &src)
	{
		return !(*this == src);
	}

	void print()
	{
		logerror( "%s[%d]:  MyAnnotateeContainer<%s>(size %d): [ ", 
				FILE__, __LINE__, typeid(T).name(), vec.size());
		for (unsigned int i = 0; i < vec.size(); ++i)
			std::cerr << vec[i] << " ";
		logerror( "]\n");
	}

	Serializable *ac_serialize_impl(SerializerBase *sb, const char *a) THROW_SPEC(SerializerError)
	{
		//logerror( "%s[%d]:  welcome to MyAnnotationContainer::ac_serialize_impl %s: vec.size() = %d, this = %p\n", FILE__, __LINE__, sb_is_input(sb) ? "deserialize" : "serialize", vec.size(), this);

		gtranslate(sb, vec, "MyAnnotationContainer");

		//logerror( "%s[%d]:  leaving MyAnnotationContainer::ac_serialize_impl, vec.size() = %d\n", FILE__, __LINE__, vec.size());
		return NULL;
	}
};

class MyAnnotationContainer2 : public AnnotationContainer<MyAnnotationClass *>
{
	std::vector<MyAnnotationClass *> vec;
	bool addItem_impl(MyAnnotationClass *i) {vec.push_back(i); return true;}

	public:

	MyAnnotationContainer2() : AnnotationContainer<MyAnnotationClass *>() 
	{
		logerror( "%s[%d]:  MyAnnotationContainer2 ctor: %p\n", FILE__, __LINE__, this);
	}

	bool operator==(MyAnnotationContainer2 &src)
	{
		if (vec.size() != src.vec.size())
		{
			logerror( "%s[%d]:  size mismatch: %d--%d\n", 
					FILE__, __LINE__, vec.size(), src.vec.size());
			return false;
		}

		for (unsigned int i = 0; i < vec.size(); ++i)
			if ( (*vec[i]) != (*src.vec[i])) 
				return false;

#if 0
		logerror( "%s[%d]:  annotation container elem0 is %d\n", 
				FILE__, __LINE__, vec.size() ? (*vec[0]) : -1);
#endif

		return true;
	}

	bool operator!=(MyAnnotationContainer2 &src)
	{
		return !(*this == src);
	}

	void print()
	{
		logerror( "%s[%d]:  MyAnnotateeContainer2(size %d): [ ", 
				FILE__, __LINE__, vec.size());
		for (unsigned int i = 0; i < vec.size(); ++i)
			std::cerr << *vec[i] << " ";
		logerror( "]\n");
	}

	Serializable *ac_serialize_impl(SerializerBase *sb, const char *a) THROW_SPEC(SerializerError)
	{
		logerror( "%s[%d]:  welcome to MyAnnotationContainer2::ac_serialize_impl %s: vec.size() = %d, this = %p\n", FILE__, __LINE__, sb_is_input(sb) ? "deserialize" : "serialize", vec.size(), this);

		gtranslate(sb, vec, "MyAnnotationContainer2");

		logerror( "%s[%d]:  leaving MyAnnotationContainer2::ac_serialize_impl, vec.size() = %d\n", FILE__, __LINE__, vec.size());
		return NULL;
	}
};


void unlinkSerfile(SerContextBase *scb)
{
	if (!scb) return;
	std::string fname = scb->getSerFileName();
	if (fname.length())
	{
		logerror("%s[%d]:  unlinking %s\n", FILE__, __LINE__, fname.c_str());
		unlink(fname.c_str());
	}
}
template <class A, class C>
void serialize_test1(A &annotatee, C &control) THROW_SPEC(LocErr)
{
	SerContext<A> *scs = NULL;
	try {
	//  serialize_test1:  add annotation to class A (annotatee), serialize A.
	//  Then deserialize A and verify that (1) serialization worked for A proper,
	//  and (2) that added annotation was also properly serialized
	//
	//  This should work for both cases where C (the annotation class) is derived
	//  from Serializable and when it is a basic type

	if (dynamic_cast<AnnotatableSparse *>(&annotatee))
		logerror("%s[%d]:  annotatee is sparse\n", FILE__, __LINE__);
	else if (dynamic_cast<AnnotatableDense *>(&annotatee))
		logerror("%s[%d]:  annotatee is dense\n", FILE__, __LINE__);
	else {
		EFAIL("ERROR:  annotatee is not annotatable\n");
	}

	logerror( "%s[%d]: welcome to serialize test for annotating type %s with %s\n",
			FILE__, __LINE__, typeid(A).name(), typeid(C).name());

	Tempfile tf;
	std::string file(tf.getName());

	//  Add the annotation
	std::string an = std::string(typeid(C).name()) + std::string(typeid(A).name());

	AnnotationClass<C> *my_acp = new AnnotationClass<C>(an);
	AnnotationClass<C> &my_ac = *my_acp;

	if (!annotatee.addAnnotation(&control, my_ac))
	{
		EFAIL("failed to add annotation");
	}

	if (NULL == my_ac.getSerializeFunc())
		EFAIL("annotation class has no serialize func");

	//  Do the serialization
	scs = new SerContext<A>(&annotatee, file);
	scs->enableSerDes(true);

	if (!annotatee.serialize(file, scs, ser_bin))
	{
		unlinkSerfile(scs);
		EFAIL("serialize failed");
	}

	//  It is necessary to flush the file buffers to make sure the cache file is
	//  fully synchronized before we do deserialize.
	fflush(NULL);

	logerror( "%s[%d]:  after serialize\n", FILE__, __LINE__);

	A deserialize_result;
	Serializable &c_serial = (Serializable &) deserialize_result;

	//  Do the deserialize
	try 
	{
		c_serial.deserialize(file, scs);
	}
	catch (const SerializerError &serr)
	{
		logerror( "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
		logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		unlinkSerfile(scs);
		EFAIL("deserialize failed");
	}

	if (annotatee != deserialize_result)
	{
		deserialize_result.print();
		unlinkSerfile(scs);
		EFAIL("deserialize result is incorrect");
	}

	//  checking equality is fine, but let's also check absolute values
	int res = 0;
	if ( 0 != (res = init_or_check_values(deserialize_result, CHECK))) 
	{
		deserialize_result.print();
		logerror( "%s[%d]:  res = %d\n", FILE__, __LINE__);
		unlinkSerfile(scs);
		EFAIL("deserialized annotatee has bad values");
	}

	//  operator== will check basic class values, but we also need to verify annotations
	C *des_annotation;
	if (!deserialize_result.getAnnotation(des_annotation, my_ac))
	{
		unlinkSerfile(scs);
		EFAIL("could not find annotation in deserialized result");
	}

	if (*des_annotation != control)
	{
		logerror( "%s[%d]:  annotations (id = %d) did not match after deserialize\n", FILE__, __LINE__, my_ac.getID());
		logerror( "%s[%d]:  des_annotation = %p, des_obj = %p, control = %p\n", FILE__, __LINE__, des_annotation, &deserialize_result, &control);

		des_annotation->print();
		control.print();
		unlinkSerfile(scs);
		EFAIL("deserialized annotation did not match provided value");
	}
	}
	catch (const SerializerError &serr)
	{
		logerror( "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
		logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		//EFAIL("deserialize failed\n");
	}
#if 0
	catch (...)
	{
		logerror( "%s[%d]:  caught unknown exception\n", FILE__, __LINE__);
	}
#endif
	unlinkSerfile(scs);
}

template <class A, class C>
void serialize_test2(A &annotatee, C &control) THROW_SPEC(LocErr)
{
	//  serialize_test2:  First serialize A, then add an annotation to A.  
	//  Then deserialize A and verify that (1) serialization worked for A proper,
	//  and (2) that added annotation was also properly serialized
	//
	//  This should work for both cases where C (the annotation class) is derived
	//  from Serializable and when it is a basic type

	SerContext<A> *scs = NULL;

	AnnotatableDense *ad = NULL;
	if (dynamic_cast<AnnotatableSparse *>(&annotatee))
		logerror("%s[%d]:  annotatee is sparse\n", FILE__, __LINE__);
	else if (NULL != ( ad = dynamic_cast<AnnotatableDense *>(&annotatee)))
		logerror("%s[%d]:  \nannotatee is dense: %p\n", FILE__, __LINE__, ad);
	else {
		EFAIL("ERROR:  annotatee is not annotatable\n");
	}

	logerror( "%s[%d]: welcome to serialize test for annotating type %s with %s\n",
			FILE__, __LINE__, typeid(A).name(), typeid(C).name());

	Tempfile tf;
	std::string file(tf.getName());

	//  Do the serialization
	//annotatee.serialize(serializer, NULL);
	
	scs = new SerContext<A>(&annotatee, file);
	scs->enableSerDes(true);
	if (!annotatee.serialize(file, scs, ser_bin))
	{
		unlinkSerfile(scs);
		EFAIL("serialize failed");
	}

	//  Add the annotation
	std::string an = std::string(typeid(C).name()) + std::string(typeid(A).name());
	AnnotationClass<C> *my_acp = new AnnotationClass<C>(an);
	AnnotationClass<C> &my_ac = *my_acp;

	if (NULL == my_ac.getSerializeFunc())
	{
		unlinkSerfile(scs);
		EFAIL("annotation class has no serialize func");
	}

	if (!annotatee.addAnnotation(&control, my_ac))
	{
		unlinkSerfile(scs);
		EFAIL("failed to add annotation");
	}

	//  It is necessary to flush the file buffers to make sure the cache file is
	//  fully synchronized before we do deserialize.
	if (0 !=fflush(NULL))
	{
		logerror( "%s[%d]:  fflush failed: %s\n", FILE__, __LINE__, strerror(errno));
	}

	logerror( "%s[%d]:  after serialize\n", FILE__, __LINE__);


	A deserialize_result;
	Serializable &c_serial = (Serializable &) deserialize_result;


	//  Do the deserialize
	try 
	{
		//c_serial.serialize(deserializer, NULL);
		if (!c_serial.deserialize(file, scs))
		{
			unlinkSerfile(scs);
			EFAIL("deserialize failed");
		}
	}
	catch (const SerializerError &serr)
	{
		logerror( "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
		logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		unlinkSerfile(scs);
		EFAIL("deserialize failed\n");
	}

	if (annotatee != deserialize_result)
	{
		deserialize_result.print();
		unlinkSerfile(scs);
		EFAIL("deserialize result is incorrect");
	}

	//  checking equality is fine, but let's also check absolute values
	if ( 0 != init_or_check_values(deserialize_result, CHECK)) 
	{
		deserialize_result.print();
		unlinkSerfile(scs);
		EFAIL("deserialized annotatee has bad values");
	}

	//  operator== will check basic class values, but we also need to verify annotations
	C *des_annotation;
	if (!deserialize_result.getAnnotation(des_annotation, my_ac))
	{
		unlinkSerfile(scs);
		EFAIL("could not find annotation in deserialized result");
	}

	if (*des_annotation != control)
	{
		des_annotation->print();
		control.print();
		unlinkSerfile(scs);
		EFAIL("deserialized annotation did not match provided value");
	}
	unlinkSerfile(scs);
}


template <class A, class C, class I>
void serialize_test3(A &annotatee, C &container, I item) THROW_SPEC(LocErr)
{
	//  serialize_test3:  Add annotation to A, then serialize A.  
	//  Then add item to annotation container and verify that (1) serialization 
	//  worked for A proper,
	//  and (2) that added annotation was also properly serialized
	//  and (3) that added item was also properly serialized

	SerContext<A> *scs = NULL;
	AnnotatableDense *ad = NULL;
	if (dynamic_cast<AnnotatableSparse *>(&annotatee))
		logerror("%s[%d]:  annotatee is sparse\n", FILE__, __LINE__);
	else if (NULL != ( ad = dynamic_cast<AnnotatableDense *>(&annotatee)))
		logerror("%s[%d]:  \nannotatee is dense: %p\n", FILE__, __LINE__, ad);
	else {
		EFAIL("ERROR:  annotatee is not annotatable\n");
	}

	logerror( "%s[%d]: welcome to serialize test for annotating type %s with %s\n",
			FILE__, __LINE__, typeid(A).name(), typeid(C).name());

	Tempfile tf;
	std::string file(tf.getName());

	
	//  Add the annotation
	std::string an = std::string(typeid(C).name()) + std::string(typeid(A).name()) + std::string(typeid(I).name());
	AnnotationClass<C> *my_acp = new AnnotationClass<C>(an);
	AnnotationClass<C> &my_ac = *my_acp;

	if (NULL == my_ac.getSerializeFunc())
		EFAIL("annotation class has no serialize func");

	if (!annotatee.addAnnotation(&container, my_ac))
		EFAIL("failed to add annotation");

	//  Do the serialization
	//annotatee.serialize(serializer, NULL);
	scs = new SerContext<A>(&annotatee, file);
	scs->enableSerDes(true);
	if (!annotatee.serialize(file, scs, ser_bin))
	{
		unlinkSerfile(scs);
		EFAIL("serialize failed");
	}

	//  Add the item to the container post-serialize
	if (!container.addItem(item))
	{
		unlinkSerfile(scs);
		EFAIL("failed to add item to container here");
	}

	//  It is necessary to flush the file buffers to make sure the cache file is
	//  fully synchronized before we do deserialize.
	if (0 !=fflush(NULL))

	logerror( "%s[%d]:  after serialize\n", FILE__, __LINE__);


	A deserialize_result;
	Serializable &c_serial = (Serializable &) deserialize_result;


	//  Do the deserialize
	try 
	{
		//c_serial.serialize(deserializer, NULL);
		if (!c_serial.deserialize(file, scs))
		{
			logerror( "%s[%d]:  deserialize failed\n", FILE__, __LINE__);
		}
	}
	catch (const SerializerError &serr)
	{
		logerror( "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
		logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		unlinkSerfile(scs);
		EFAIL("deserialize failed\n");
	}

	if (annotatee != deserialize_result)
	{
		logerror( "%s[%d]:  deserialize result is incorrect\n", FILE__, __LINE__);
		deserialize_result.print();
	}

	//  checking equality is fine, but let's also check absolute values
	if ( 0 != init_or_check_values(deserialize_result, CHECK)) 
	{
		deserialize_result.print();
		unlinkSerfile(scs);
		EFAIL("deserialized annotatee has bad values\n");
	}

	//  operator== will check basic class values, but we also need to verify annotations
	C *des_annotation;
	if (!deserialize_result.getAnnotation(des_annotation, my_ac))
	{
		unlinkSerfile(scs);
		EFAIL("could not find annotation in deserialized result");
	}

	if (*des_annotation != container)
	{
		des_annotation->print();
		container.print();
		unlinkSerfile(scs);
		EFAIL("deserialized annotation did not match provided value");
	}

	unlinkSerfile(scs);
}
#endif

test_results_t test_ser_anno_Mutator::executeTest()
{
	return SKIPPED;
#if !defined(SERIALIZATION_DISABLED)
	//  set environment variable enabling serialization
	errno = 0;

	try 
	{
		AnnotateeSparse as1;
		Serializable *s = dynamic_cast<Serializable *>(&as1);
		if (!s)
		{
			logerror( "%s[%d]:  as1 not serializable\n", FILE__, __LINE__);
			return FAILED;
		}
		AnnotatableSparse *ss = dynamic_cast<AnnotatableSparse *>(s);
		if (!ss)
		{
			logerror( "%s[%d]:  as1 not annotatableSparse\n", FILE__, __LINE__);
			return FAILED;
		}
		if ( 0 != init_or_check_values(as1, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as1.print();
		MyAnnotationClass mac(77);
		serialize_test1(as1, mac);

		AnnotateeDense ad1;
		if ( 0 != init_or_check_values(ad1, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad1.print();
		MyAnnotationClass mac2(8);
		serialize_test1(ad1, mac2);

		AnnotateeSparse *as2 = new AnnotateeSparse();
		if ( 0 != init_or_check_values(*as2, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as2->print();
		MyAnnotationClass mac3(11);
		serialize_test2(*as2, mac3);


		AnnotateeDense ad2;
		if ( 0 != init_or_check_values(ad2, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad2.print();
		MyAnnotationClass mac4(10);
		serialize_test2(ad2, mac4);

		//  same thing with annotation containers
		//  First check that empty containers work...
		AnnotateeSparse as3;
		if ( 0 != init_or_check_values(as3, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as3.print();
		MyAnnotationContainer<int> cont1;
		serialize_test1(as1, cont1);

		AnnotateeDense ad3;
		if ( 0 != init_or_check_values(ad3, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad3.print();
		MyAnnotationContainer<int> cont2;
		serialize_test1(ad3, cont2);

		AnnotateeSparse as4;
		if ( 0 != init_or_check_values(as4, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as4.print();
		MyAnnotationContainer<int> cont3;
		serialize_test2(as4, cont3);

		AnnotateeDense ad4;
		if ( 0 != init_or_check_values(ad4, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad4.print();
		MyAnnotationContainer<int> cont4;
		serialize_test2(ad4, cont4);

		//  Slightly less trivial case:  the annotation containers actually contain stuff
		AnnotateeSparse as5;
		if ( 0 != init_or_check_values(as5, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as5.print();
		MyAnnotationContainer<int> cont5;
		cont5.addItem(5);
		serialize_test1(as5, cont5);

		AnnotateeDense ad5;
		if ( 0 != init_or_check_values(ad5, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad5.print();
		MyAnnotationContainer<int> cont6;
		cont6.addItem(6);
		serialize_test1(ad5, cont6);

		AnnotateeSparse as6;
		if ( 0 != init_or_check_values(as6, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as6.print();
		MyAnnotationContainer<int> cont7;
		cont7.addItem(7);
		serialize_test2(as6, cont7);

		AnnotateeDense ad6;
		if ( 0 != init_or_check_values(ad6, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad6.print();
		MyAnnotationContainer<int> cont8;
		cont8.addItem(8);
		serialize_test2(ad6, cont8);

		//  now test serializing additions to containers

		//  first use int
		AnnotateeSparse as7;
		if ( 0 != init_or_check_values(as7, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as7.print();
		MyAnnotationContainer<int> cont9;
		serialize_test3(as7, cont9, 9);

		AnnotateeDense ad7;
		if ( 0 != init_or_check_values(ad7, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad7.print();
		MyAnnotationContainer<int> cont10;
		serialize_test3(ad7, cont10, 10);

		//  then use descendant of Serializable
		AnnotateeSparse as8;
		if ( 0 != init_or_check_values(as8, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		as8.print();
		MyAnnotationContainer<MyAnnotationClass> cont11;
		MyAnnotationClass mac11(11);
		serialize_test3(as8, cont11, mac11);

		AnnotateeDense ad8;
		if ( 0 != init_or_check_values(ad8, INIT)) 
		{
			logerror( "%s[%d]:  weird, failed to init annotatee here\n", 
					FILE__, __LINE__);
			return FAILED;
		}
		ad8.print();
		MyAnnotationContainer<MyAnnotationClass> cont12;
		MyAnnotationClass mac12(12);
		serialize_test3(ad8, cont12, mac12);

		//  then test container of pointers to Serializable
	}
	catch (const LocErr &err) {
		logerror( "%s[%d]:  FAILED\n", FILE__, __LINE__);
		err.print(stderr);
		return FAILED;
	}
	catch (const SerializerError &serr)
	{
		logerror( "%s[%d]:  serializer function threw exception:\n", FILE__, __LINE__);
		logerror( "\tfrom %s[%d]: %s\n", serr.file().c_str(), serr.line(), serr.what());
		EFAIL("deserialize failed");
	}
	catch (...)
	{
		logerror( "%s[%d]:  caught unknown exception\n", FILE__, __LINE__);
		return FAILED;
	}
		
	return PASSED;
#endif
}
