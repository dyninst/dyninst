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

#if !defined (SERIALIZATION_PUBLIC_H)
#define SERIALIZATION_PUBLIC_H
//  Hopefully just a few definitions allowing for a public interface to 
//  serializing user-providede annotations


#define SERIALIZATION_DISABLED
#if defined(SERIALIZATION_DISABLED)

#if defined(THROW_SPEC)
#undef THROW_SPEC
#endif
#define THROW_SPEC(X)

#include <stdlib.h>

namespace Dyninst {
   class Serializable {
     public:
      virtual ~Serializable() {}
   };
   class SerializerBase {
     public:
      virtual ~SerializerBase() {}
   };

   template <class T>
   class AnnotationContainer
   {
      virtual bool deserialize_item(SerializerBase *) { return true; }
     public:
		AnnotationContainer() { }
		virtual ~AnnotationContainer() { }
		virtual bool addItem_impl(T t) = 0;
		bool addItem(T t) { return addItem_impl(t); }
		virtual const char *getElementTypename() {return NULL;}
		virtual Serializable *ac_serialize_impl(SerializerBase *, const char *) THROW_SPEC(SerializerError) { return NULL; };
   };
}

#else

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include <map>
#include <assert.h>
#include "dyntypes.h"
#include "util.h"
#include "Annotatable.h"
#include "boost/type_traits/is_fundamental.hpp"
#include "boost/type_traits/is_const.hpp"
#include "boost/type_traits/remove_cv.hpp"
#include "boost/type_traits/is_pointer.hpp"
#include "boost/type_traits/is_enum.hpp"
#include "boost/type_traits/alignment_of.hpp"
#include "boost/type_traits/type_with_alignment.hpp"
#include "boost/type_traits/remove_pointer.hpp"

#define SERIALIZE_CONTROL_ENV_VAR "DYNINST_SERIALIZE_ENABLE"
#define SERIALIZE_DISABLE "disable"
#define SERIALIZE_ONLY "serialize_but_dont_deserialize"
#define SERIALIZE_DESERIALIZE "deserialize"
#define SERIALIZE_DESERIALIZE_OR_DIE "deser_or_die"
#define DESERIALIZE_FORCE_ENV_VAR "DYNINST_DESERIALIZE_FORCE"

namespace Dyninst 
{
#if 0
//  throw_ser_err(FILE__, __LINE__,"msg") -- an attempt at "graceful" failure.  If debug flag is set
//  it will assert, otherwise it throws...  leaving the "graceful" aspect
//  to the next (hopefully top-level) exception handler.
//  UPDATE -- due to vtable recognition probs between modules (dyninst libs and
//  testsuite executables) no longer asserts.
	


#define throw_ser_err(FILE__, __LINE__,cmsg) \
   do { \
      if (serializer_debug_flag()) { \
         serialize_printf("SER_ERR: %s", cmsg); \
         throw SerializerError(__FILE__, __LINE__, std::string(cmsg)); \
      } else { \
         throw SerializerError(__FILE__, __LINE__, std::string(cmsg)); \
      } \
   } while (0)
#endif

COMMON_EXPORT bool &serializer_debug_flag();

class SerializerBase;

typedef enum {sd_serialize, sd_deserialize} iomode_t;

class SerializerError {
   //  SerializerError:  a small class that is thrown by serialization/deserialization
   //  routines.  This exists as an attempt to standardize and simplify error handling
   //  for ser-des routines that are possibly deeply nested.
   //  Here's the rub:  we don't want stray, unhandled exceptions finding their way into
   //  the larger system...  thus all entry points to serialization/deserialization need
   //  to catch this exception to render it transparent to the rest of the system.

   public:

   typedef enum {
      ser_err_unspecified,
      ser_err_no_err,
      ser_err_disabled
   } SerializerErrorType;

   private:

   std::string msg__;
   std::string file__;
   int line__;
   SerializerErrorType err__;

   public:

   COMMON_EXPORT SerializerError(const std::string &__file__, 
         const int &__line__, 
         const std::string &msg, 
         SerializerErrorType __err__ = ser_err_unspecified) :
      msg__(msg),
      file__(__file__),
      line__(__line__),
      err__(__err__)
   {}

   COMMON_EXPORT virtual ~SerializerError() THROW {}

   COMMON_EXPORT std::string file() const {return file__;}
   COMMON_EXPORT std::string msg() const {return msg__;}
   COMMON_EXPORT const char* what() const {return msg__.c_str();}
   COMMON_EXPORT int line() const {return line__;}
   COMMON_EXPORT SerializerErrorType code() const {return err__;}
};

COMMON_EXPORT void printSerErr(const SerializerError &err);

typedef enum {
	ser_bin,
	ser_xml
} ser_type_t;

class SerContextBase 
{
	friend class Serializable;
	friend class SerializerBase;
	COMMON_EXPORT static std::vector<std::pair<std::string, dyn_hash_map<std::string, short>*> > ser_control_map;
	std::string fname;
	std::string serfilename;
	static dyn_hash_map<std::string, short> *getMapForType(std::string);
	public:

	COMMON_EXPORT SerContextBase(std::string, std::string); 
	COMMON_EXPORT virtual ~SerContextBase() {}
	COMMON_EXPORT virtual void *getVoidContext() = 0;
	COMMON_EXPORT virtual const char *getRootTypename() = 0;
	COMMON_EXPORT static void enableSerialize(std::string, std::string, bool);
	COMMON_EXPORT static void enableDeserialize(std::string, std::string, bool);
	COMMON_EXPORT static void enforceDeserialize(std::string, std::string, bool);
	COMMON_EXPORT static void enableSerDes(std::string, std::string, bool);
	COMMON_EXPORT void enableSerialize(bool);
	COMMON_EXPORT void enableDeserialize(bool);
	COMMON_EXPORT void enforceDeserialize(bool);
	COMMON_EXPORT void enableSerDes(bool);

	COMMON_EXPORT bool serializeEnabled();
	COMMON_EXPORT bool deserializeEnabled();
	COMMON_EXPORT static bool deserializeEnforced(std::string, std::string);
	COMMON_EXPORT bool deserializeEnforced();

	COMMON_EXPORT std::string getSerFileName();
	COMMON_EXPORT virtual bool isContextType(Serializable *) = 0;
};

template <class T>
class SerContext : public SerContextBase
{
	T *scope;

	public:

	SerContext(T *scope_, std::string fname) : SerContextBase(std::string(typeid(T).name()), fname), scope(scope_) {}
	~SerContext() {}
	void *getVoidContext() {return (void *) scope;}
	const char *getRootTypename() {return typeid(T).name();}
	T *getScope() {return scope;}
	bool isContextType(Serializable *s) { return (NULL != dynamic_cast<T *>(s));}
};

class SerFile;
class SerDes;

class SerializerBase {
	friend class Serializable;

	public:
	COMMON_EXPORT static std::vector<SerializerBase *> active_serializers;
	//  TODO:  make these private or protected
	static bool global_disable;
	private:

	SerFile *sf;
	SerDes *sd;
	SerContextBase *scon;
	unsigned short ser_index;

	std::string serializer_name;

	typedef dyn_hash_map<std::string, SerializerBase *> subsystem_serializers_t;
	COMMON_EXPORT static dyn_hash_map<std::string, subsystem_serializers_t> all_serializers;

	dyn_hash_map<void *, AnnotatableSparse *> *sparse_annotatable_map;
	dyn_hash_map<void *, AnnotatableDense *> *dense_annotatable_map;
	void clearAnnotationMaps();
	public:
	COMMON_EXPORT void set_annotatable_sparse_map(AnnotatableSparse *, void *);
	COMMON_EXPORT void set_annotatable_dense_map(AnnotatableDense *, void *);
	COMMON_EXPORT unsigned short getIndex();
	COMMON_EXPORT static void globalDisable();
	COMMON_EXPORT static bool serializationDisabled();
	COMMON_EXPORT static void globalEnable();

	COMMON_EXPORT virtual bool isXML() = 0;
	COMMON_EXPORT virtual bool isBin ()= 0;
	COMMON_EXPORT bool isEOF();

	COMMON_EXPORT SerContextBase *getContext();
	COMMON_EXPORT bool isInput ();
	COMMON_EXPORT bool isOutput ();
	COMMON_EXPORT AnnotatableSparse *findSparseAnnotatable(void *id);
	COMMON_EXPORT AnnotatableDense *findDenseAnnotatable(void *id);

	COMMON_EXPORT static void dumpActiveBinSerializers();

	COMMON_EXPORT SerializerBase(SerContextBase *scb, std::string name_, std::string filename,                 iomode_t dir, bool verbose);
	COMMON_EXPORT SerializerBase();

	COMMON_EXPORT virtual ~SerializerBase();

	COMMON_EXPORT virtual SerDes &getSD();
	COMMON_EXPORT SerFile &getSF(); 
	COMMON_EXPORT std::string &name() {return serializer_name;}
	COMMON_EXPORT static SerializerBase *getSerializer(std::string subsystem, std::string fname);
	COMMON_EXPORT static bool addSerializer(std::string subsystem, std::string fname, SerializerBase *sb);
	COMMON_EXPORT static bool removeSerializer(unsigned short);
	COMMON_EXPORT virtual void vector_start(unsigned long &, const char * = NULL);
	COMMON_EXPORT virtual void vector_end();
	COMMON_EXPORT virtual void hash_map_start(unsigned long &size, const char *tag = NULL);
	COMMON_EXPORT virtual void hash_map_end();
	COMMON_EXPORT virtual void multimap_start(unsigned long &size, const char *tag = NULL);
	COMMON_EXPORT virtual void multimap_end();
	COMMON_EXPORT virtual void pair_start(const char *tag = NULL);
	COMMON_EXPORT virtual void pair_end();
	COMMON_EXPORT virtual void annotation_start(AnnotationClassID &a_id, void *&lparent_id, sparse_or_dense_anno_t &lsod, const char *);
	COMMON_EXPORT virtual void annotation_end();
	COMMON_EXPORT virtual void annotation_container_start(void *&id);
	COMMON_EXPORT virtual void annotation_container_end();
	COMMON_EXPORT virtual void annotation_container_item_start(void *&id);
	COMMON_EXPORT virtual void annotation_container_item_end();
	COMMON_EXPORT void translate_base(bool &v, const char *&t);
	COMMON_EXPORT void translate_base(short &v, const char *&t);
	COMMON_EXPORT void translate_base(unsigned short &v, const char *&t);
	COMMON_EXPORT void translate_base(char &v, const char *&t);
	COMMON_EXPORT void translate_base(int &v, const char *&t);
	COMMON_EXPORT void translate_base(unsigned int &v, const char *&t);
	COMMON_EXPORT void translate_base(unsigned long &v, const char *&t);
	COMMON_EXPORT void translate_base(long &v, const char *&t);
	COMMON_EXPORT void translate_base(float &v, const char *&t);
	COMMON_EXPORT void translate_base(double &v, const char *&t);
	COMMON_EXPORT void translate_base(const char * &v, int bufsize, const char *&t);
	COMMON_EXPORT void translate_base(char * &v, int bufsize, const char *&t);
	COMMON_EXPORT void translate_base(void * &v, const char *&t);
	COMMON_EXPORT void translate_base(std::string &v, const char *t);
	COMMON_EXPORT virtual void magic_check(const char *file__, unsigned int line__);

	COMMON_EXPORT virtual iomode_t iomode();

	COMMON_EXPORT void serialize_annotations(void *, std::vector<ser_rec_t> &sers, const char * = NULL);
	COMMON_EXPORT bool serialize_post_annotation(void *parent_id, void *anno, AnnotationClassBase *acb, sparse_or_dense_anno_t , const char * = NULL);
};

SerializerBase *createSerializer(SerContextBase *, std::string, std::string, ser_type_t, iomode_t, bool = false);

class AnnotatableSparse;
class AnnotatableDense;
class AnnotationContainerBase;

COMMON_EXPORT void serialize_annotatable_id(SerializerBase *sb, void *&id, const char *tag);
COMMON_EXPORT bool set_sb_annotatable_sparse_map(SerializerBase *, AnnotatableSparse *, void *);
COMMON_EXPORT bool set_sb_annotatable_dense_map(SerializerBase *, AnnotatableDense *, void *);
COMMON_EXPORT unsigned short get_serializer_index(SerializerBase *sb);
COMMON_EXPORT void annotation_start(SerializerBase *, AnnotationClassID &, void *&, sparse_or_dense_anno_t &, const char *);
COMMON_EXPORT void annotation_end(SerializerBase *);
COMMON_EXPORT AnnotatableSparse *find_sparse_annotatable(SerializerBase *, void *);
COMMON_EXPORT AnnotatableDense *find_dense_annotatable(SerializerBase *, void *);
COMMON_EXPORT bool isEOF(SerializerBase *);
COMMON_EXPORT void throw_ser_err(const char *file__, unsigned int line, const char *msg);


COMMON_EXPORT void annotation_container_start(SerializerBase *sb, void *&id);
COMMON_EXPORT void annotation_container_end(SerializerBase *sb);
COMMON_EXPORT void annotation_container_item_start(SerializerBase *, void *&);
COMMON_EXPORT void annotation_container_item_end(SerializerBase *);
COMMON_EXPORT bool deserialize_container_item(SerializerBase *, void *);
COMMON_EXPORT AnnotationContainerBase *get_container(void *);
COMMON_EXPORT bool deserialize_container_item(AnnotationContainerBase *acb, SerializerBase *sb);

template <class T>
void enableSerialize(std::string fname, bool val)
{
	serialize_printf("%s[%d]:  %s serialize for type %s\n", 
			FILE__, __LINE__, val ? "enabling" : "disabling", typeid(T).name());
	SerContextBase::enableSerialize(std::string(typeid(T).name()), fname, val);
}

template <class T>
void enableDeserialize(std::string fname, bool val)
{
	serialize_printf("%s[%d]:  %s deserialize for type %s\n", 
			FILE__, __LINE__, val ? "enabling" : "disabling", typeid(T).name());
	SerContextBase::enableDeserialize(std::string(typeid(T).name()), fname, val);
}

template <class T>
void enforceDeserialize(std::string fname, bool val)
{
	serialize_printf("%s[%d]:  %s enforced deserialize for type %s\n", 
			FILE__, __LINE__, val ? "enabling" : "disabling", typeid(T).name());
	SerContextBase::enforceDeserialize(std::string(typeid(T).name()), fname, val);
}

template <class T>
bool deserializeEnforced(std::string fname)
{
	return SerContextBase::deserializeEnforced(std::string(typeid(T).name()), fname);
}

template <class T>
void enableSerDes(std::string fname, bool val)
{
	serialize_printf("%s[%d]:  %s serialize/deserialize for type %s\n", 
			FILE__, __LINE__, val ? "enabling" : "disabling", typeid(T).name());
	SerContextBase::enableSerDes(std::string(typeid(T).name()), fname, val);
}

class Serializable {
	bool was_deserialized;
	static void clearContainersByID();

	protected:
	unsigned short active_serializer_index;

	COMMON_EXPORT Serializable() : 
		was_deserialized(false), 
		active_serializer_index((unsigned short) (-1)) {}

	COMMON_EXPORT virtual ~Serializable() 
	{
		if (active_serializer_index != (unsigned short) -1)
		{
			SerializerBase *sb = getExistingOutputSB(active_serializer_index);
			if (sb)
			{
				SerContextBase *scb = sb->getContext();
				if (scb->isContextType(this))
				{
					//  hrm...  not sure this works as intended.
					fprintf(stderr, "%s[%d]:  TOP LEVEL SERIALIZE DONE:  removing serializer\n", FILE__, __LINE__);
					sb->removeSerializer(active_serializer_index);
				}
			}
		}
	}

	COMMON_EXPORT virtual Serializable *serialize_impl(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError) = 0;

	public:

	COMMON_EXPORT unsigned short getID() {return active_serializer_index;}

	COMMON_EXPORT bool serialize(std::string filename, SerContextBase *scb, ser_type_t);

	COMMON_EXPORT bool deserialize(std::string filename, SerContextBase *scb) 
	{
		std::string sername = std::string("Deserializer");

		SerializerBase *serializer = createSerializer(scb, sername, filename, 
				ser_bin, sd_deserialize, /*verbose*/ false);

		if (!serializer) 
		{
			serialize_printf("%s[%d]:  ERROR:  failed to create deserializer for %s\n", 
					FILE__, __LINE__, filename.c_str());
			clearContainersByID();
			return false;
		}

		try
		{
			//  Do the serialization
			serialize(serializer, NULL);
		}
		catch (const SerializerError &err_)
		{
			serialize_printf("%s[%d]:  deserialize failed\n", FILE__, __LINE__);
			printSerErr(err_);
			serializer->clearAnnotationMaps();
			clearContainersByID();
			return false;
		}
		catch (...)
		{
			serialize_printf("%s[%d]:  caught unexpected exception\n", FILE__, __LINE__);
			serializer->clearAnnotationMaps();
			clearContainersByID();
			return false;
		}

		void *barrier_magic = (void *) 0xdeadbeef;
		serialize_annotatable_id(serializer, barrier_magic, NULL);

		if (barrier_magic != (void *)0xdeadbeef)
		{
			fprintf(stderr, "%s[%d]:  FIXME:  failed to read magic barrier\n", FILE__, __LINE__);
		}

		unsigned op_count = 0;
		while (1)
		{
			try
			{
				void *my_anno = NULL;
				void *parent_id = NULL;
				ser_post_op_t op = sp_add_anno;
				AnnotationClassBase *acb = NULL;
				sparse_or_dense_anno_t sod = sparse;
				AnnotationClassID a_id = 0;

				serializer_printf("%s[%d]:  reading post-serialize item %d\n", 
						FILE__, __LINE__, op_count);

				if (!ser_operation(serializer, op, NULL))
				{
					if (isEOF(serializer))
					{
						serialize_printf("%s[%d]:  got EOF\n", FILE__, __LINE__);
						serializer->clearAnnotationMaps();
						clearContainersByID();
						return true;
					}
				}
				switch (op) {
					case sp_add_anno:
						{
						annotation_start(serializer, a_id, parent_id, sod, NULL);
						acb = AnnotationClassBase::findAnnotationClass(a_id);
						if (!acb)
						{
							fprintf(stderr, "%s[%d]:  failed to find annotation type %d\n", 
									FILE__, __LINE__, a_id);
							serializer->clearAnnotationMaps();
							clearContainersByID();
							return false;
						}
						else
						{
							serializer_printf("%s[%d]:  found annotation id %d/%d\n", 
									FILE__, __LINE__, acb->getID(), a_id);
						}

						my_anno = acb->allocate();
						assert(my_anno);

						ser_func_t sf = acb->getSerializeFunc();
						if (!sf)
						{
							fprintf(stderr, "%s[%d]:  failed to find serialization function\n", 
									FILE__, __LINE__);
							serializer->clearAnnotationMaps();
							clearContainersByID();
							return false;
						}

						//  execute the serialization function for this annotation
						serializer_printf("%s[%d]:  calling serialize func for type %s\n",
								FILE__, __LINE__, acb->getTypeName());

						(*sf)(my_anno, serializer, NULL);

						serializer_printf("%s[%d]:  called serialize func for type %s\n",
								FILE__, __LINE__, acb->getTypeName());

						annotation_end(serializer);

						//  we have the (void *) annotation and the annotation type
						//  now lookup the object to which it belonged in the map of 
						//  annotatable objects
						//  and add it as an annotation.

						if (sparse == sod)
						{
							AnnotatableSparse *as = find_sparse_annotatable(serializer, parent_id);
							if (NULL == as)
							{
								fprintf(stderr, "%s[%d]:  ERROR:  cannot find anno parent id %p for anno of type %s\n",
										FILE__, __LINE__, parent_id, acb->getTypeName());
							}
							else
							{
								assert(acb);

								if (!as->addAnnotation(my_anno, acb->getID()))
								{
									fprintf(stderr, "%s[%d]:  ERROR:  failed to add annotation here\n",
											FILE__, __LINE__);
								}

								//  Need to consider case if annotatee is itself annotatable
								//  this should be tested  (should transparently end up in
								//  the annotatable-object mapping, but not sure if it does 
								//  right now)
							}
						}
						else if (dense == sod)
						{
							AnnotatableDense *ad = find_dense_annotatable(serializer, parent_id);
							if (NULL == ad)
							{
								fprintf(stderr, "%s[%d]:  ERROR:  cannot find anno parent id = %p\n",
										FILE__, __LINE__, (void *) parent_id);
							}
							else
							{
								assert(acb);
								serializer_printf("%s[%d]:  reading post annotation\n", 
										FILE__, __LINE__);
								if (!ad->addAnnotation(my_anno, acb->getID()))
								{
									fprintf(stderr, "%s[%d]:  ERROR:  failed to add annotation here\n",
											FILE__, __LINE__);
								}
							}
						}
						else
						{
							fprintf(stderr, "%s[%d]:  ERROR:  sparse/dense not set properly\n", 
									FILE__, __LINE__);
							serializer->clearAnnotationMaps();
							clearContainersByID();
							return false;
						}
						break;
						}
					case sp_rem_anno:
						{
							fprintf(stderr, "%s[%d]:  FIXME:  not implemented\n", FILE__, __LINE__);
						break;
						}
					case sp_add_cont_item:
						{
							void *parent_id = NULL;
							annotation_container_item_start(serializer, parent_id);
							if (!parent_id)
							{
								fprintf(stderr, "%s[%d]:  NULL container with id\n", 
										FILE__, __LINE__);
								serializer->clearAnnotationMaps();
								clearContainersByID();
								return false;
							}

							if (!deserialize_container_item(serializer, parent_id))
							{
								fprintf(stderr, "%s[%d]:  failed deser contitem w/parent %p\n", 
										FILE__, __LINE__, parent_id);
								serializer->clearAnnotationMaps();
								clearContainersByID();
								return false;
							}

							annotation_container_item_end(serializer);
							break;
						}
					case sp_rem_cont_item:
						{
							fprintf(stderr, "%s[%d]:  FIXME:  not implemented\n", 
									FILE__, __LINE__);
						break;
						}
					default:
						fprintf(stderr, "%s[%d]:  ERROR:  bad ser operation %d\n", 
								FILE__, __LINE__, op);
						serializer->clearAnnotationMaps();
						clearContainersByID();
						return false;
				};

				if (isEOF(serializer))
				{
					fprintf(stderr, "%s[%d]:  got EOF\n", FILE__, __LINE__);
					serializer->clearAnnotationMaps();
					clearContainersByID();
					return true;
				}

			}
			catch (const Dyninst::SerializerError &err_)
			{
				if (isEOF(serializer))
				{
					serialize_printf("%s[%d]:  got EOF\n", FILE__, __LINE__);
					serializer->clearAnnotationMaps();
					clearContainersByID();
					return true;
				}
				if (serializer_debug_flag()) 
				{
					fprintf(stderr, "%s[%d]:  deserialize caught exception\n", FILE__, __LINE__);
					printSerErr(err_);
				}
				serializer->clearAnnotationMaps();
				clearContainersByID();
				return false;
			}
			catch (...)
			{
				serialize_printf("%s[%d]:  caught unknown exception\n", FILE__, __LINE__);
				if (isEOF(serializer))
				{
					serialize_printf("%s[%d]:  got EOF\n", FILE__, __LINE__);
					serializer->clearAnnotationMaps();
					clearContainersByID();
					return true;
				}
				serializer->clearAnnotationMaps();
				clearContainersByID();
				return false;
			}
			op_count++;
		}

		serializer->clearAnnotationMaps();
		clearContainersByID();
		return true;
	}

	COMMON_EXPORT SerializerBase *lookupExistingSerializer();
	COMMON_EXPORT bool from_cache() {return was_deserialized;}

	COMMON_EXPORT virtual Serializable *serialize(SerializerBase *sb,  
			const char *tag = NULL) THROW_SPEC(SerializerError)
	{
		//  This function must be implemented in the header file so that
		//  the vtables of AnnotatableSparse and AnnotatableDense resolve properly
		//  (only matters in case of testsuite, but it's nice to have finer grain 
		//  tests that this allows)

		//  do base serialization for this class

		sb->magic_check(FILE__, __LINE__);
		Serializable *res = serialize_impl(sb, tag);
		sb->magic_check(FILE__, __LINE__);

		//  then serialize all Annotations for which a serialization function has been provided
		void *id = this;

		AnnotatableSparse *as = dynamic_cast<AnnotatableSparse *> (this);
		if (as)
		{
			id = as;
			//  since this class is annotatable, serialize its id (address)
			//  so that we can later build a map of all annotatable objects
			//  by id, and apply post-annotations to them 

			sb->magic_check(FILE__, __LINE__);
			serialize_annotatable_id(sb, id, "SparseAnnotatableObjectID");
			sb->magic_check(FILE__, __LINE__);

			if (is_input(sb))  
			{
				if (NULL != id)
				{
					if (!set_sb_annotatable_sparse_map(sb, as, id))
					{
						fprintf(stderr, "%s[%d]:  failed to set annotatable-anno mapping here\n", 
								FILE__, __LINE__);
					}
				}
				else
					fprintf(stderr, "%s[%d]:  ERROR:  id is NULL\n", FILE__, __LINE__);
			}

			if (is_output(sb))
				serialize_printf("%s[%d]:  set anno mapping for %p\n", FILE__, __LINE__, id);

			sb->magic_check(FILE__, __LINE__);
			as->serializeAnnotations(sb, tag);
			sb->magic_check(FILE__, __LINE__);
		}
		else
		{
			AnnotatableDense *ad = dynamic_cast<AnnotatableDense *> (this);
			if (ad)
			{
				//  since this class is annotatable, serialize its id (address)
				//  so that we can later build a map of all annotatable objects
				//  by id, and apply post-annotations to them 
				id = ad;
				sb->magic_check(FILE__, __LINE__);
				serialize_annotatable_id(sb, id, "DenseAnnotatableObjectID");
				sb->magic_check(FILE__, __LINE__);

				serialize_printf("%s[%d]:  %sserializing annotatable id %p, this = %p\n", 
						FILE__, __LINE__, is_output(sb) ? "": "de", id, this);

				if (is_input(sb))
				{
					if (NULL != id)
					{
						//  add id to the map of annotatable objects, mapping to "this"

						if (!set_sb_annotatable_dense_map(sb, ad, id))
						{
							fprintf(stderr, "%s[%d]:  failed to set annotatable-annotation map\n",
									FILE__, __LINE__);
						}

						serialize_printf("%s[%d]:  set dense annotatable mapping for id %p\n", 
								FILE__, __LINE__, (void *)id);
					}
					else
						fprintf(stderr, "%s[%d]:  got NULL id in deserialize\n", FILE__, __LINE__);
				}

				sb->magic_check(FILE__, __LINE__);
				ad->serializeAnnotations(sb, tag);
				sb->magic_check(FILE__, __LINE__);
			}
			else
			{
				serialize_printf("%s[%d]:  class %s is not annotatable\n", 
						FILE__, __LINE__, typeid(this).name());
			}
		}

		if (is_input(sb))
			was_deserialized = true;

		active_serializer_index = get_serializer_index(sb);

		return res;
	}
};


class AnnotationContainerBase : public Serializable
{
	friend class Serializable;
	friend COMMON_EXPORT bool deserialize_container_item(SerializerBase *, void *);
	friend COMMON_EXPORT bool deserialize_container_item(AnnotationContainerBase *, SerializerBase *);

	protected:
		static dyn_hash_map<void *, AnnotationContainerBase *> containers_by_id;
		COMMON_EXPORT static void clearContainersByID();

		COMMON_EXPORT AnnotationContainerBase() {}
		COMMON_EXPORT virtual ~AnnotationContainerBase() {}
		COMMON_EXPORT virtual bool deserialize_item(SerializerBase *) = 0;

	public:

		COMMON_EXPORT virtual Serializable *ac_serialize_impl(SerializerBase *,  
				const char * = NULL) THROW_SPEC(SerializerError) = 0;

		COMMON_EXPORT virtual Serializable *serialize_impl(SerializerBase *sb,  
				const char *tag = NULL) THROW_SPEC(SerializerError) 
		{
			void *id = this;
			annotation_container_start(sb, id);
			//  serialize the id of this container, then call the container serialize func
			//serialize_annotatable_id(sb, id, "ContainerID");
			Serializable *res = ac_serialize_impl(sb, tag);
			annotation_container_end(sb);

			if (is_input(sb))
			{
				//  for deserialize build a hash of container ids
				containers_by_id[id] = this;
			}
			annotatable_printf("%s[%d]: %s AnnotationContainer(%p) of type %s, id = %p\n",
					FILE__, __LINE__, is_input(sb) ? "deserialized" : "serialized", this,
					getElementTypename(), id);	   
			return res;
		}	

		COMMON_EXPORT virtual const char *getElementTypename() = 0; 
		COMMON_EXPORT static AnnotationContainerBase *getContainer(void *id)
		{
			dyn_hash_map<void *, AnnotationContainerBase *>::iterator iter;

			iter = containers_by_id.find(id);
			if (iter == containers_by_id.end())
			{
				fprintf(stderr, "%s[%d]:  ERROR:  cannot find id %p in container-id map\n", 
						FILE__, __LINE__, id);
				return NULL;
			}

			return iter->second;
		}

		COMMON_EXPORT static void setContainerID(AnnotationContainerBase *acb, void *id)
		{
			//  Currently see no need to first check whether we have a duplicate here
			containers_by_id[id] = acb;
		}
};


template <class T>
Serializable *cont_ser_func_wrapper(void *it, SerializerBase *sb, const char *)
{
	T *itt =  (T*) it;
	gtranslate(sb, *itt);
	return NULL;
}

#if 0
template <class T, typename IS_POINTER, typename IS_SERIALIZABLE>
class SerFuncExecutor 
{
	AnnotationClass<T> *ac;

	public:

	SerFuncExecutor(AnnotationClass<T> *ac_) : ac(ac_) {}

	Serializable *operator()(T &my_item, SerializerBase *sb, const char *tag = NULL) 
	{
		ser_func_t sf = ac->getSerializeFunc();
		assert(sf);

		serialize_printf("%s[%d]:  calling serialize func for type %s\n",
				FILE__, __LINE__, ac->getTypeName());

		Serializable *res = (*sf)(&my_item, sb, tag);
		return res;
	}
};

template <class T>
class SerFuncExecutor<T, dyn_detail::boost::true_type, dyn_detail::boost::true_type>
{
	AnnotationClass<T> *ac;

	public:

	SerFuncExecutor(AnnotationClass<T> *ac_) : ac(ac_) {}

	void operator()(T &my_item, SerializerBase *sb, const char *tag = NULL) 
	{
		ser_func_t sf = ac->getSerializeFunc();
		assert(sf);

		//  T is a pointer, and we need to alloc the object
		my_item = (T) ac->allocate();
		serialize_printf("%s[%d]:  calling serialize func for type %s\n",
				FILE__, __LINE__, ac->getTypeName());

		Serializable *sable = (*sf)(my_item, sb, tag);

		T res = dynamic_cast<T>(sable);

		if (sable && !res)
		{
			fprintf(stderr, "%s[%d]:  ERROR:  %s not Serializable ??\n", 
					FILE__, __LINE__, typeid(T).name());
		}

		if (res && (res != my_item))
		{
			fprintf(stderr, "%s[%d]:  serialize func for %s did a realloc\n", 
					FILE__, __LINE__, typeid(T).name());
			fprintf(stderr, "%s[%d]:  REMOVED DELETE\n", FILE__, __LINE__);
			delete my_item;
			my_item = res;
		}
		serialize_printf("%s[%d]:  called serialize func for type %s\n",
				FILE__, __LINE__, ac->getTypeName());
	}
};
#endif

template <class T>
class AnnotationContainer : public AnnotationContainerBase
{
#if 0
	//  We internally manage the annotation class (which includes serialize function)
	//  for the elements of this container.  When the container is added as a annotation,
	//  the element annotation class is thus not required in addition to the annotation
	//  class that the user must supply for the container overall.

	AnnotationClass<T> *ac;
#endif

		virtual bool deserialize_item(SerializerBase *sb)
		{
			T my_item;
			if (dyn_detail::boost::is_pointer<T>::value)
			{
				//  if T is a class , memset would clobber the vptr
				memset( &my_item, 0, sizeof(T));
			}

			if (!sb)
			{
				fprintf(stderr, "%s[%d]:  FIXME:  bad param\n", FILE__, __LINE__);
				return false;
			}
#if 0 
			ser_func_t sf = ac->getSerializeFunc();

			if (!sf)
			{
				fprintf(stderr, "%s[%d]:  failed to find serialization function\n", 
						FILE__, __LINE__);
				return false;
			}

			SerFuncExecutor<T, typename dyn_detail::boost::is_pointer<T>::type, 
				typename dyn_detail::boost::is_base_of<Serializable, 
				typename dyn_detail::boost::remove_pointer<T>::type>::type>  
					sfe(ac);

			sfe(my_item, sb);
#else
			gtranslate(sb, my_item);
#endif

			if (!addItem_impl(my_item))
			{
				fprintf(stderr, "%s[%d]:  failed to addItem after deserialize\n", FILE__, __LINE__);
				return false;
			}

			return true;
		}

	public:


		AnnotationContainer() 
#if 0
			: ac(NULL)
#endif
		{
#if 0
			ser_func_t sf = NULL;

			if (dyn_detail::boost::is_fundamental<T>::value)
			{
				sf = (ser_func_t) cont_ser_func_wrapper<T>;
			}

			std::string aname = std::string(typeid(*this).name()) + std::string("_elem");
			ac = new AnnotationClass<T>(aname, NULL, sf);
#endif
		}

		~AnnotationContainer()
		{/* if (ac) delete ac; */}

		//  The routine that actually adds the item and manages whatever data structure
		//  the container is using.  Must be provided by descendant class.

		virtual bool addItem_impl(T t) = 0;

		bool addItem(T t) 
		{
			if (!addItem_impl(t))
			{
				fprintf(stderr, "%s[%d]:  failed to add item of type %s to container\n", 
						FILE__, __LINE__, typeid(T).name());
				return false;
			}

			//  If a serialization has already occurred for the container, serialize this item too

			SerializerBase *sb = lookupExistingSerializer();
			if (sb)
			{
				//  T must be either a basic type or a descendant of Serializable
				ser_post_op_t op =  sp_add_cont_item;
				void *id = this;
				ser_operation(sb, op, NULL);
				annotation_container_item_start(sb, id);
				gtranslate(sb, t, NULL);
				annotation_container_item_end(sb);
			}
			return true;
		}	

		virtual const char *getElementTypename() {return typeid(T).name();}

		virtual Serializable *ac_serialize_impl(SerializerBase *, 
				const char *tag) THROW_SPEC(SerializerError) = 0;
};

template <typename T>
void translate_vector(SerializerBase *ser, std::vector<T> &vec,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned long nelem = 0UL;

	if (ser->iomode() == sd_serialize)
		nelem = (unsigned long) vec.size();

	ser->vector_start(nelem, tag);

	if (ser->iomode() == sd_deserialize) 
	{
		if (vec.size())
			throw_ser_err(FILE__, __LINE__,"nonempty vector used to create");

		//  zero size vectors are allowed
		serialize_printf("%s[%d]:  about to resize vector to %lu\n", 
				FILE__, __LINE__, nelem);

		if (nelem)
			vec.resize(nelem);
	}

	for (unsigned long i = 0; i < nelem; ++i) 
	{
		gtranslate(ser, vec[i], elem_tag);
#if 0
		if (ser->isOutput())
			gtranslate(ser, vec[i], elem_tag);
		else
		{
			T t;
			gtranslate(ser, t, elem_tag);
			vec.push_back(t);
		}
#endif
#if 0
		T t;
		if (ser->isOutput())
			t = vec[i];
		//T &t = vec[i];
		//memset(&(vec[i]), 0, sizeof(T));
		gtranslate(ser, t, elem_tag);
		if (ser->isInput())
			vec[i] = t;
#endif
	}

	ser->vector_end();
}

template <typename K, typename V>
void translate_pair(SerializerBase *ser, std::pair<K, V> &p,
		const char *tag = NULL, const char *tag2 = NULL)
{
	ser->pair_start();

	gtranslate(ser, p.first, tag);
	gtranslate(ser, p.second, tag2);

	ser->pair_end();
}

template <typename T, typename T2>
struct pair_translator {
	void operator()(SerializerBase *ser, std::pair<T, T2> &p, 
			const char *tag = NULL, const char *tag2 = NULL)
{
	ser->pair_start();

	gtranslate(ser,  p.first, tag);
	gtranslate(ser,  p.second, tag2);

	ser->pair_end();
}
};

template <class S, class K, class V>
void translate_dyn_hash_map(S *ser, dyn_hash_map<K, V> &map,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	serialize_printf("%s[%d]:  welcome to translate_dyn_hash_map<%s, %s>, size = %ld\n", 
			FILE__, __LINE__, typeid(K).name(), typeid(V).name(), map.size());

	unsigned long nelem = 0UL;
	nelem = map.size();
	ser->hash_map_start(nelem, tag);

	if (ser->iomode() == sd_deserialize)
	{
		if (map.size())
			throw_ser_err(FILE__, __LINE__,"nonempty vector used to create");

		for (unsigned long i = 0UL; i < nelem; ++i)
		{
			typename dyn_hash_map<K, V>::value_type mapentry;
			gtranslate(ser, mapentry, elem_tag);
			map.insert(mapentry);
		}
	}
	else
	{
		assert (ser->iomode() == sd_serialize);
		typename dyn_hash_map<K, V>::iterator iter = map.begin();
		while (iter != map.end())
		{
			gtranslate(ser, *iter, elem_tag);
			iter++;
		}
	}

	ser->hash_map_end();

	serialize_printf("%s[%d]:  leaving translate_dyn_hash_map<%s, %s>\n", 
			FILE__, __LINE__, typeid(K).name(), typeid(V).name());
}

template <class S, class K, class V>
void translate_map(S *ser, std::map<K, V> &map,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned long nelem = 0UL;
	nelem = map.size();
	ser->map_start(nelem, tag);

	if (ser->iomode() == sd_deserialize)
	{
		if (map.size())
			throw_ser_err(FILE__, __LINE__,"nonempty vector used to create");

		typename std::map<K, V>::iterator lastentry = map.begin();
		//  cannot do any kind of bulk allocation with maps
		for (unsigned int i = 0; i < nelem; ++i)
		{
			K a_k;
			V a_v;
			gtranslate(ser, a_k, elem_tag);
			gtranslate(ser, a_v, elem_tag);
			map[a_k] = a_v;
		}

	}
	else
	{
		assert (ser->iomode() == sd_serialize);
		typename std::map<K, V>::iterator iter = map.begin();
		while (iter != map.end())
		{
			K &a_k = const_cast<K &>(iter->first);
			V &a_v = const_cast<V &>(iter->second);
			gtranslate(ser, a_k, elem_tag);
			gtranslate(ser, a_v, elem_tag);
			iter++;
		}
	}
	ser->map_end();
}

template <class K, class V, class L>
struct multimap_translator
{
	void operator()(SerializerBase *ser, std::multimap<K, V, L> &map, 
			const char *tag = NULL, const char *elem_tag = NULL)
	{
		unsigned long nelem = 0UL;
		nelem = map.size();
		ser->multimap_start(nelem, tag);

		if (ser->iomode() == sd_deserialize)
		{
			if (map.size())
				throw_ser_err(FILE__, __LINE__,"nonempty vector used to create");

			typename std::multimap<K, V, L>::iterator lastentry = map.begin();
			//  cannot do any kind of bulk allocation with multimaps
			for (unsigned int i = 0; i < nelem; ++i)
			{
				typename std::multimap<K, V, L>::value_type mapentry;
				gtranslate(ser, mapentry, elem_tag);
				//  lastentry serves as a hint as to where the new value goes 
				//  (presumably near the last value inserted)
				//  not sure if this really makes things more efficient.
				lastentry = map.insert(lastentry, mapentry);
			}

		}
		else
		{
			assert (ser->iomode() == sd_serialize);
			typename std::multimap<K, V, L>::iterator iter = map.begin();
			while (iter != map.end())
			{
				gtranslate(ser, *iter, elem_tag);
				iter++;
			}
		}

		ser->multimap_end();
	}
};

template <class K, class V, class L>
void translate_multimap(SerializerBase *ser, std::multimap<K, V, L> &map,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned long nelem = 0UL;
	nelem = map.size();
	ser->multimap_start(nelem, tag);

	if (ser->iomode() == sd_deserialize)
	{
		if (map.size())
			throw_ser_err(FILE__, __LINE__,"nonempty vector used to create");

		typename std::multimap<K, V, L>::iterator lastentry = map.begin();
		//  cannot do any kind of bulk allocation with multimaps
		for (unsigned int i = 0; i < nelem; ++i)
		{
			typename std::multimap<K, V, L>::value_type mapentry;
			gtranslate(ser, mapentry, elem_tag);
			//  lastentry serves as a hint as to where the new value goes 
			//  (presumably near the last value inserted)
			//  not sure if this really makes things more efficient.
			lastentry = map.insert(lastentry, mapentry);
		}

	}
	else
	{
		assert (ser->iomode() == sd_serialize);
		typename std::multimap<K, V, L>::iterator iter = map.begin();
		while (iter != map.end())
		{
			gtranslate(ser, *iter, elem_tag);
			iter++;
		}
	}

	ser->multimap_end();
}

template <class S, class K, class V>
void translate_hash_map(S *ser, dyn_hash_map<K, V> &hash, 
		const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{   
	fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s>()\n", 
			__FILE__, __LINE__,
			typeid(K).name(), typeid(V).name()); 

	unsigned long nelem = 0UL;
	nelem = hash.size();
	ser->hash_map_start(nelem, tag);

	if (ser->iomode() == sd_serialize) 
	{
		typename dyn_hash_map<K,V>::iterator iter = hash.begin();

		while (iter != hash.end()) 
		{
			K k = iter->first;
			V v = iter->second;
			ser->translate_base(k, key_tag);
			ser->translate_base(v, value_tag);
			iter++;           
		}
	}
	else 
	{
		//  can we do some kind of preallocation here?
		for (unsigned long i = 0; i < nelem; ++i) 
		{
			K k;
			V v;
			// zero out pointers to silence complaints about assigning
			// new objects to non-NULL pointers
			if (dyn_detail::boost::is_pointer<K>::value)
				memset(&k, 0, sizeof(K));
			if (dyn_detail::boost::is_pointer<V>::value)
				memset(&v, 0, sizeof(V));
			ser->translate_base(k, key_tag);
			ser->translate_base(v, value_tag);
			hash[k] = v;
		}
	}

	ser->hash_map_end();
}

COMMON_EXPORT bool isBinary(Dyninst::SerializerBase *ser);
COMMON_EXPORT bool isOutput(Dyninst::SerializerBase *ser);

typedef void NOTYPE_T;

template<typename T, typename T2 = typename dyn_detail::boost::is_enum<T>::type,
	typename T3 = typename dyn_detail::boost::is_base_of<Serializable, T>::type>
class trans_adaptor {
   public:
      trans_adaptor()  {}

      T * operator()(SerializerBase *ser, T & it, 
			  const char *tag = NULL, const char * /*tag2*/ = NULL)
      {
		 ser->translate_base(it, tag);
         return &it;
      }
};

template <>
class trans_adaptor<char *, dyn_detail::boost::false_type, 
	  dyn_detail::boost::false_type> 
{
   public:
      trans_adaptor()  {}

      char * * operator()(SerializerBase *ser, char * & it, 
			  const char *tag = NULL, const char * /*tag2*/ = NULL)
      {
		  int len = it ? strlen(it) : 0;
		  ser->translate_base(it, len, tag);
		  return &it;
	  }
};

template<typename T>
class trans_adaptor<T, dyn_detail::boost::false_type, 
	  dyn_detail::boost::true_type> 
{
   public:
      trans_adaptor()  {}

      Serializable * operator()(SerializerBase *ser, T & it, const char *tag = NULL, 
            const char * /*tag2*/ = NULL)
      {
         //gtranslate(ser, it, tag);

		  serialize_printf("%s[%d]:  translating serializable: %s\n", 
				  FILE__, __LINE__, typeid(T).name());

		  it.serialize(ser, tag);

		  serialize_printf("%s[%d]:  translated serializable: %s\n", 
				  FILE__, __LINE__, typeid(T).name());

         return &it;
      }
};

template<typename T>
class trans_adaptor<T, dyn_detail::boost::true_type, 
	  dyn_detail::boost::false_type> 
{
   public:
      trans_adaptor()  {}

      Serializable * operator()(SerializerBase *ser, T & it, const char *tag = NULL, 
            const char * /*tag2*/ = NULL)
      {
		  
		  if (sizeof(T) != sizeof(int)) 
			  fprintf(stderr, "%s[%d]:  enum size is %lu, not %lu\n",
					  FILE__, __LINE__, sizeof(T), sizeof(int));
		  int e_int = (int) it;
          ser->translate_base(e_int, tag);
		  if (ser->isInput())
			  it = (T) e_int;
#if 0
         //gtranslate(ser, it, tag);

		  serialize_printf("%s[%d]:  translating serializable: %s\n", 
				  FILE__, __LINE__, typeid(T).name());

		  it.serialize(ser, tag);

		  serialize_printf("%s[%d]:  translated serializable: %s\n", 
				  FILE__, __LINE__, typeid(T).name());
#endif

		  return NULL;
         //return &it;
      }
};

#if 0
template<typename T>
class trans_adaptor<std::vector<T>, dyn_detail::boost::false_type, 
	  dyn_detail::boost::false_type> 
{
   public:
      trans_adaptor() {}

      std::vector<T> * operator()(SerializerBase *ser, std::vector<T> &v,  
            const char *tag = NULL, const char *tag2 = NULL) 
      {
         translate_vector(ser, v, tag, tag2);         //  maybe catch errors here?
         return &v;
      }
};
#endif

template <typename T, bool = dyn_detail::boost::is_base_of<Serializable, typename dyn_detail::boost::remove_pointer<T>::type>::value>
class deref_adaptor 
{
	public:
		deref_adaptor() {}

		T * operator()(SerializerBase *sb, T &it, 
				const char *tag = NULL, const char *tag2 = NULL)
		{
			trans_adaptor<T> ta;
			ta(sb, it, tag, tag2);
			return &it;
		}
};

template <typename T>
class deref_adaptor<T, true> 
{
	public:
		deref_adaptor() {}

		T * operator()(SerializerBase *sb, T *&it, 
				const char *tag = NULL, const char *tag2 = NULL)
		{
			//  need to allocate upon deserialize
			if (!isOutput(sb))
			{
				it = new T();
			}
			gtranslate(sb, *it, tag, tag2);
			return it;
		}
};

template <typename T, typename T2 = typename dyn_detail::boost::is_pointer<T>::type, 
		 typename T3 = typename dyn_detail::boost::is_base_of<Serializable,
		 typename dyn_detail::boost::remove_pointer<T>::type>::type>
class ptr_adaptor 
{
	public:
		ptr_adaptor() {}

		T * operator()(SerializerBase *sb, T &it, 
				const char *tag = NULL, const char *tag2 = NULL)
		{
			trans_adaptor<T> ta;
			ta(sb, it, tag, tag2);
			return &it;
		}
};

template <typename T>
class ptr_adaptor<T, dyn_detail::boost::true_type, dyn_detail::boost::false_type> 
{
	typedef typename dyn_detail::boost::remove_pointer<T>::type pointed_to_t;

	public:
		ptr_adaptor() {}
		T * operator()(SerializerBase *sb, T &it, 
				const char *tag = NULL, const char *tag2 = NULL)
		{
			deref_adaptor<pointed_to_t> da;
			da(sb, it, tag, tag2);
			return &it;
		}
};

template <typename T>
class ptr_adaptor<T, dyn_detail::boost::true_type, dyn_detail::boost::true_type> 
{
	typedef typename dyn_detail::boost::remove_pointer<T>::type pointed_to_t;
	public:
		ptr_adaptor() {}
		T * operator()(SerializerBase *sb, T &it, 
				const char *tag = NULL, const char * = NULL)
		{
			if (!isOutput(sb))
			{
				if (it)
					fprintf(stderr, "%s[%d]:  WARNING:  allocating to non-null pointer: %s\n", 
							FILE__, __LINE__, typeid(T).name());
				serialize_printf("%s[%d]:  allocating new %s\n", 
						FILE__, __LINE__, typeid(T).name());

				it = new pointed_to_t();
			}

			Serializable *res = NULL;
			res = it->serialize(sb, tag);

			if (!isOutput(sb)) 
			{
				T res_typed = dynamic_cast<T>(res);

				if (res && !res_typed)
				{
					fprintf(stderr, "%s[%d]:  ERROR:  ser func allocated bad type object! not %s\n", 
							FILE__, __LINE__, typeid(T).name());
				}

				if (res_typed && (res_typed != it))
				{
					serialize_printf("%s[%d]:  serialize fn for %s reallocated object\n", 
							FILE__, __LINE__, typeid(T).name());
					delete it;
					it = res_typed;
				}
			}

			return &it;
		}
};

//  Another specialization:  do _not_ dereference any char *'s, they're special
//  (maybe want to do this for all fundamental types)
template <>
class ptr_adaptor<char *, dyn_detail::boost::true_type, dyn_detail::boost::false_type> 
{
	public:
		ptr_adaptor() {}
		char * * operator()(SerializerBase *sb, char * &it, 
				const char *tag = NULL, const char *tag2 = NULL)
		{
			trans_adaptor<char *> ta;
			ta(sb, it, tag, tag2);
			return &it;
		}
};

template <typename T, bool  = dyn_detail::boost::is_const<T>::value>
class const_adaptor 
{
	public:
		const_adaptor() {}

		T *operator()(SerializerBase *sb, T &it, 
				const char *tag = 0, const char *tag2 = 0)
		{
			ptr_adaptor<T> ta;
			T *itp = ta(sb, it, tag, tag2);
			return itp;
		}
};

template <typename T>
class const_adaptor<T, true>
{
	typedef typename dyn_detail::boost::remove_cv<T>::type non_const_type;

	public:
	const_adaptor() {}

	T *operator()(SerializerBase *sb, T &it, 
			const char *tag = 0, const char *tag2 = 0)
	{
		gtranslate(sb, const_cast<non_const_type &>(it), tag, tag2);
		return &it;
	}
};

template <typename T>
void gtranslate(SerializerBase *ser, T &it, 
		const char *tag = NULL, const char *tag2 = NULL)
{
   //fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
   //      __FILE__, __LINE__,
   //      "SerializerBase",
   //      typeid(T).name(), &it);

   const_adaptor<T> ta;
   T *itp = ta(ser, it, tag, tag2);

   if (!itp) 
   {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", 
            __FILE__, __LINE__);
   }
}

template <typename T>
void gtranslate(SerializerBase *ser, std::vector<T> &it, 
		const char *tag = NULL, const char *tag2 = NULL)
{
	translate_vector(ser, it, tag, tag2);
}

template <typename T, typename T2>
void gtranslate(SerializerBase *ser, std::pair<T, T2> &it, 
		const char *tag = NULL, const char *tag2 = NULL)
{
	//translate_pair(ser, it, tag, tag2);
	pair_translator<T, T2> pt;
	pt(ser, it, tag, tag2);
}

template <typename K, typename V, typename L>
void gtranslate(SerializerBase *ser, std::multimap<K, V, L> &it, 
		const char *tag = NULL, const char *tag2 = NULL)
{
	translate_multimap(ser, it, tag, tag2);
}

template <typename K, typename V>
void gtranslate(SerializerBase *ser, dyn_hash_map<K, V> &it, 
		const char *tag = NULL, const char *tag2 = NULL)
{
	translate_dyn_hash_map(ser, it, tag, tag2);
}

COMMON_EXPORT bool ifxml_start_element(SerializerBase *sb, const char *tag);
COMMON_EXPORT bool ifxml_end_element(SerializerBase *sb, const char * /*tag*/);
COMMON_EXPORT bool sb_is_input(SerializerBase *sb);
COMMON_EXPORT bool sb_is_output(SerializerBase *sb);

template <typename T>
void gtranslate(SerializerBase *ser, 
      T &it, 
      const char * (*to_str_func)(T), 
      const char *tag = NULL, 
      const char * /*tag2*/ = NULL)
{
   assert(ser);
   int enum_int = 0;
   enum_int = (int) it;

   if (!isBinary(ser)) 
   {
      assert(isOutput(ser));

      // use human-readable tag
      const char *enum_tag = (*to_str_func)(it);
      std::string enum_tag_str(enum_tag);
      assert(enum_tag);

      gtranslate(ser, enum_tag_str, tag, NULL);
   }
   else 
   {
      //  just in/output raw binary value 
      //gtranslate(ser, it);
      gtranslate(ser, enum_int, tag, NULL);
      it = (T) enum_int;
   }
}

template <typename T>
bool gtranslate_w_err(SerializerBase *ser, T &it, 
		const char *tag = NULL, const char *tag2 = NULL)
{
   try 
   {
      gtranslate(ser, it, tag, tag2);
   }

   catch (const SerializerError &err_) 
   {
      fprintf(stderr, "%s[%d]:  gtranslate failed\n", __FILE__, __LINE__);
      printSerErr(err_);
      return false;
   }
   return true;
}

} /* namespace Dyninst */

#endif

#endif

