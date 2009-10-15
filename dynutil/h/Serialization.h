
#if !defined (SERIALIZATION_PUBLIC_H)
#define SERIALIZATION_PUBLIC_H
//  Hopefully just a few definitions allowing for a public interface to 
//  serializing user-providede annotations

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

#define SERIALIZE_CONTROL_ENV_VAR "DYNINST_SERIALIZE_ENABLE"
#define SERIALIZE_DISABLE "disable"
#define SERIALIZE_ONLY "serialize"
#define SERIALIZE_DESERIALIZE "deserialize"
#define SERIALIZE_DESERIALIZE_OR_DIE "deserialize_or_die"

namespace Dyninst {
//  SER_ERR("msg") -- an attempt at "graceful" failure.  If debug flag is set
//  it will assert, otherwise it throws...  leaving the "graceful" aspect
//  to the next (hopefully top-level) exception handler.

#define serialize_printf serializer_printf

COMMON_EXPORT int serializer_printf(const char *format, ...);


COMMON_EXPORT bool &serializer_debug_flag();

#define SER_ERR(cmsg) \
   do { \
      if (serializer_debug_flag()) { \
         fprintf(stderr, "%s", cmsg); \
         assert (0); \
      } else { \
         throw SerializerError(__FILE__, __LINE__, std::string(cmsg)); \
      } \
   } while (0)


class SerializerBase;

typedef enum {sd_serialize, sd_deserialize} iomode_t;

#if 0
typedef bool (*deserialize_and_annotate_t)(SerializerBase *, void *parent);

bool addDeserializeFuncForType(deserialize_and_annotate_t, const std::type_info *);
deserialize_and_annotate_t getDeserializeFuncForType(const std::type_info *);
#endif

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
	ser_xml,
} ser_type_t;

class SerContextBase {
	public:

	SerContextBase()  {}
	virtual ~SerContextBase() {}
	virtual void *getVoidContext() = 0;
};

template <class T>
class SerContext : public SerContextBase
{
	T *scope;

	public:

	SerContext(T *scope_) : scope(scope_) {}
	~SerContext() {}
	void *getVoidContext() {return (void *) scope;}
	T *getScope() {return scope;}
};


class SerFile;
class SerDes;
class SerializerBase {
	friend class Serializable;

	public:
	COMMON_EXPORT static std::vector<SerializerBase *> active_serializers;
	//  TODO:  make these private or protected
	COMMON_EXPORT static dyn_hash_map<std::string, SerializerBase *> active_bin_serializers;
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



SerializerBase *createSerializer(SerContextBase *, std::string, std::string, ser_type_t, iomode_t, bool);


class AnnotatableSparse;
class AnnotatableDense;
//bool serializeAnnotationsWrapper(AnnotatableSparse *, SerializerBase *, const char *);
//bool serializeAnnotationsWrapper(AnnotatableDense *, SerializerBase *, const char *);
COMMON_EXPORT void serialize_annotatable_id(SerializerBase *sb, void *&id, const char *tag);
COMMON_EXPORT bool set_sb_annotatable_sparse_map(SerializerBase *, AnnotatableSparse *, void *);
COMMON_EXPORT bool set_sb_annotatable_dense_map(SerializerBase *, AnnotatableDense *, void *);
COMMON_EXPORT unsigned short get_serializer_index(SerializerBase *sb);
COMMON_EXPORT void annotation_start(SerializerBase *, AnnotationClassID &, void *&, sparse_or_dense_anno_t &, const char *);
COMMON_EXPORT void annotation_end(SerializerBase *);
COMMON_EXPORT AnnotatableSparse *find_sparse_annotatable(SerializerBase *, void *);
COMMON_EXPORT AnnotatableDense *find_dense_annotatable(SerializerBase *, void *);
COMMON_EXPORT bool isEOF(SerializerBase *);


COMMON_EXPORT void annotation_container_start(SerializerBase *sb, void *&id);
COMMON_EXPORT void annotation_container_end(SerializerBase *sb);
COMMON_EXPORT void annotation_container_item_start(SerializerBase *, void *&);
COMMON_EXPORT void annotation_container_item_end(SerializerBase *);
COMMON_EXPORT bool deserialize_container_item(SerializerBase *, void *);
class AnnotationContainerBase;
//void serialize_container_id(SerializerBase *, void *, const char *);

class Serializable {
	bool was_deserialized;

	protected:
	unsigned short active_serializer_index;

	COMMON_EXPORT Serializable() : 
		was_deserialized(false), 
		active_serializer_index((unsigned short) (-1)) {}

	COMMON_EXPORT virtual ~Serializable() {}

	COMMON_EXPORT virtual void serialize_impl(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError) = 0;

	public:

	COMMON_EXPORT unsigned short getID() {return active_serializer_index;}

	COMMON_EXPORT bool serialize(std::string filename, SerContextBase *scb, ser_type_t);

	COMMON_EXPORT bool deserialize(std::string filename, SerContextBase *scb) 
	{
		std::string sername = std::string("Deserializer");

		SerializerBase *serializer = createSerializer(scb, sername, filename, 
				ser_bin, sd_deserialize, /*verbose*/ true);

		if (!serializer) 
		{
			serialize_printf("%s[%d]:  ERROR:  failed to create deserializer for %s\n", 
					FILE__, __LINE__, filename.c_str());
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
			return false;
		}
		catch (...)
		{
			serialize_printf("%s[%d]:  caught unexpected exception\n", FILE__, __LINE__);
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

				fprintf(stderr, "%s[%d]:  reading post-serialize item %d\n", FILE__, __LINE__, op_count);
				ser_operation(serializer, op, NULL);
				switch (op) {
					case sp_add_anno:
						{
						annotation_start(serializer, a_id, parent_id, sod, NULL);
						acb = AnnotationClassBase::findAnnotationClass(a_id);
						if (!acb)
						{
							fprintf(stderr, "%s[%d]:  failed to find annotation type %d\n", 
									FILE__, __LINE__, a_id);
							return false;
						}
						else
						{
							fprintf(stderr, "%s[%d]:  found annotation id %d/%d\n", 
									FILE__, __LINE__, acb->getID(), a_id);
						}

						my_anno = acb->allocate();
						assert(my_anno);

						ser_func_t sf = acb->getSerializeFunc();
						if (!sf)
						{
							fprintf(stderr, "%s[%d]:  failed to find serialization function\n", 
									FILE__, __LINE__);
							return false;
						}

						//  execute the serialization function for this annotation
						fprintf(stderr, "%s[%d]:  calling serialize func for type %s\n",
								FILE__, __LINE__, acb->getTypeName());

						(*sf)(my_anno, serializer, NULL);

						fprintf(stderr, "%s[%d]:  called serialize func for type %s\n",
								FILE__, __LINE__, acb->getTypeName());

						annotation_end(serializer);

						//  we have the (void *) annotation and the annotation type
						//  now lookup the object to which it belonged in the map of annotatable objects
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
#if 0
								if (acb->isSparselyAnnotatable())
								{
									if (!set_sb_annotatable_sparse_map(serializer, my_anno, id))
									{
										fprintf(stderr, "%s[%d]:  failed to set annotatable-anno mapping here\n", 
												FILE__, __LINE__);
									}
								}
#endif
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
								fprintf(stderr, "%s[%d]:  reading post annotation\n", FILE__, __LINE__);
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
								fprintf(stderr, "%s[%d]:  NULL container with id\n", FILE__, __LINE__);
								return false;
							}
							if (!deserialize_container_item(serializer, parent_id))
							{
								fprintf(stderr, "%s[%d]:  failed to deserialize container item w/parent %p\n", FILE__, __LINE__, parent_id);
								return false;
							}
#if 0
							AnnotationContainerBase *cont =  AnnotationContainerBase::getContainer(parent_id);
							if (!cont)
							{
								fprintf(stderr, "%s[%d]:  failed to find container with id %p\n", FILE__, __LINE__, id);
								return false;
							}
							if (!cont->deserialize_item(serializer))
							{
								fprintf(stderr, "%s[%d]:  failed to deserialize container item\n", FILE__, __LINE__);
								return false;
							}
#endif

							annotation_container_item_end(serializer);
							break;
						}
					case sp_rem_cont_item:
						{
							fprintf(stderr, "%s[%d]:  FIXME:  not implemented\n", FILE__, __LINE__);
						break;
						}
					default:
						fprintf(stderr, "%s[%d]:  ERROR:  bad ser operation %d\n", FILE__, __LINE__, op);
						return false;
				};

				if (isEOF(serializer))
				{
					fprintf(stderr, "%s[%d]:  got EOF\n", FILE__, __LINE__);
					return true;
				}

			}
			catch (const Dyninst::SerializerError &err_)
			{
				if (isEOF(serializer))
				{
					serialize_printf("%s[%d]:  got EOF\n", FILE__, __LINE__);
					return true;
				}
				if (serializer_debug_flag()) 
				{
					fprintf(stderr, "%s[%d]:  deserialize caught exception\n", FILE__, __LINE__);
					printSerErr(err_);
				}
				return false;
			}
			catch (...)
			{
				serialize_printf("%s[%d]:  caught unknown exception\n", FILE__, __LINE__);
				if (isEOF(serializer))
				{
					serialize_printf("%s[%d]:  got EOF\n", FILE__, __LINE__);
					return true;
				}
				return false;
			}
			op_count++;
		}

		return true;
	}

	COMMON_EXPORT SerializerBase *lookupExistingSerializer();
	COMMON_EXPORT bool from_cache() {return was_deserialized;}

	COMMON_EXPORT virtual void serialize(SerializerBase *sb,  const char *tag = NULL) THROW_SPEC(SerializerError)
	{
		//  This function must be implemented in the header file so that
		//  the vtables of AnnotatableSparse and AnnotatableDense resolve properly

		//  do base serialization for this class

		sb->magic_check(FILE__, __LINE__);
		serialize_impl(sb, tag);
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

				fprintf(stderr, "%s[%d]:  %sserializing annotatable id %p, this = %p\n", 
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

						fprintf(stderr, "%s[%d]:  set dense annotatable mapping for id %p\n", 
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
				serialize_printf("%s[%d]:  class %s is not annotatable\n", FILE__, __LINE__, typeid(this).name());
			}
		}

		if (is_input(sb))
			was_deserialized = true;

		active_serializer_index = get_serializer_index(sb);
	}
};


class AnnotationContainerBase : public Serializable
{

	friend COMMON_EXPORT bool deserialize_container_item(SerializerBase *, void *);
	protected:
		static dyn_hash_map<void *, AnnotationContainerBase *> containers_by_id;
		COMMON_EXPORT AnnotationContainerBase() {}
		COMMON_EXPORT virtual ~AnnotationContainerBase() {}

		COMMON_EXPORT virtual bool deserialize_item(SerializerBase *) = 0;
	public:

		COMMON_EXPORT virtual void ac_serialize_impl(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError) = 0;

		COMMON_EXPORT virtual void serialize_impl(SerializerBase *sb,  const char *tag = NULL) THROW_SPEC(SerializerError) 
		{
			void *id = this;
			annotation_container_start(sb, id);
			//  serialize the id of this container, then call the container serialize func
			//serialize_annotatable_id(sb, id, "ContainerID");
			ac_serialize_impl(sb, tag);
			annotation_container_end(sb);

			if (is_input(sb))
			{
				//  for deserialize build a hash of container ids
				containers_by_id[id] = this;
			}
		}	

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

		COMMON_EXPORT virtual ser_func_t getSerFunc() = 0;
#if 0
		COMMON_EXPORT virtual AnnotationClassBase *getElementAnnotationClass() = 0;
#endif
};

class SerializerBin;
template <class T>
void cont_ser_func_wrapper(void *it, SerializerBase *sb, const char *)
{
	T *itt =  (T*) it;
		sb->magic_check(FILE__, __LINE__);
	gtranslate(sb, *itt);
		sb->magic_check(FILE__, __LINE__);
}

template <class T, typename IS_POINTER, typename IS_SERIALIZABLE>
class SerFuncExecutor 
{
	AnnotationClass<T> *ac;
	public:
	SerFuncExecutor(AnnotationClass<T> *ac_) : ac(ac_) {}
	void operator()(T &my_item, SerializerBase *sb, const char *tag = NULL) 
	{
		ser_func_t sf = ac->getSerializeFunc();
		assert(sf);
		fprintf(stderr, "%s[%d]:  calling serialize func for type %s\n",
				FILE__, __LINE__, ac->getTypeName());

		sb->magic_check(FILE__, __LINE__);
		(*sf)(&my_item, sb, tag);
		sb->magic_check(FILE__, __LINE__);
	}
};

template <class T>
class SerFuncExecutor<T, boost::true_type, boost::true_type>
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
		fprintf(stderr, "%s[%d]:  calling serialize func for type %s\n",
				FILE__, __LINE__, ac->getTypeName());

		sb->magic_check(FILE__, __LINE__);
		(*sf)(my_item, sb, tag);
		sb->magic_check(FILE__, __LINE__);

		fprintf(stderr, "%s[%d]:  called serialize func for type %s\n",
				FILE__, __LINE__, ac->getTypeName());
	}
};

template <class T>
class AnnotationContainer : public AnnotationContainerBase
{
	//  We internally manage the annotation class (which includes serialize function)
	//  for the elements of this container.  When the container is added as a annotation,
	//  the element annotation class is thus not required in addition to the annotation
	//  class that the user must supply for the container overall.
	AnnotationClass<T> *ac;

		virtual bool deserialize_item(SerializerBase *sb)
		{
			//void *my_item = acb->allocate();
			//assert(my_item);
			T my_item;

			ser_func_t sf = ac->getSerializeFunc();
			if (!sf)
			{
				fprintf(stderr, "%s[%d]:  failed to find serialization function\n", FILE__, __LINE__);
				return false;
			}

			SerFuncExecutor<T, typename boost::is_pointer<T>::type, 
				typename boost::is_base_of<Serializable, 
				typename boost::remove_pointer<T>::type>::type>  
					sfe(ac);

			sfe(my_item, sb);
#if 0
			//  execute the serialization function for this annotation
			if (boost::is_pointer<T>::value)
			{
				if (boost::is_base_of<Serializable, typename boost::remove_pointer<T>::type>::value)
				{
					//  in this case T (and thus my_item) is a pointer to a serializable
					//  object.  Need to allocate

					fprintf(stderr, "%s[%d]:  calling serialize func for type %s\n",
							FILE__, __LINE__, ac->getTypeName());

					my_item = (T) ac->allocate();
					(*sf)(my_item, sb, NULL);
				} 
			}
			else
			{
				fprintf(stderr, "%s[%d]:  calling serialize func for type %s\n",
						FILE__, __LINE__, ac->getTypeName());

				(*sf)(&my_item, sb, NULL);
			}
#endif


			if (!addItem_impl(my_item))
			{
				fprintf(stderr, "%s[%d]:  failed to addItem after deserialize\n", FILE__, __LINE__);
				return false;
			}
			return true;
		}
	public:


		AnnotationContainer() : ac(NULL)
		{
			ser_func_t sf = NULL;
			if (boost::is_fundamental<T>::value)
			{
				//fprintf(stderr, "%s[%d]:  got func for Serializable\n", FILE__, __LINE__ );
				sf = (ser_func_t) cont_ser_func_wrapper<T>;
			}

			std::string aname = std::string(typeid(*this).name()) + std::string("_elem");
			ac = new AnnotationClass<T>(aname, NULL, sf);

		}

		~AnnotationContainer()
		{/* if (ac) delete ac; */}

		//  The routine that actually adds the item and manages whatever data structure
		//  the container is using.  Must be provided by descendant class.
		virtual bool addItem_impl(T t) = 0;
#if 0
		{
			fprintf(stderr, "%s[%d]:  ERROR:  this stub should not be called\n", FILE__, __LINE__);
			return false;
		}
#endif


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
				sb->magic_check(FILE__, __LINE__); //  needs to correspond to deserialize case too
				gtranslate(sb, t, NULL);
				sb->magic_check(FILE__, __LINE__); // needs to correspond to deserialize case too
				annotation_container_item_end(sb);
			}
			return true;
		}	

		virtual ser_func_t getSerFunc()
		{
			if (!ac)
			{
				fprintf(stderr, "%s[%d]:  ERROR:  have AnnotationContainer without Element AC\n", 
						FILE__, __LINE__);
				return NULL;
			}
			return ac->getSerializeFunc();
		}

#if 0
		COMMON_EXPORT virtual AnnotationClassBase *getElementAnnotationClass()
		{
			return ac;
		}
#endif

#if 0
		COMMON_EXPORT virtual void serialize_impl(SerializerBase *, const char *tag) THROW_SPEC(SerializerError) = 0;
		{
			fprintf(stderr, "%s[%d]:  ERROR:  this stub should not be called\n", FILE__, __LINE__);
		}
#endif
		virtual void ac_serialize_impl(SerializerBase *, const char *tag) THROW_SPEC(SerializerError) = 0;
#if 0
		{
			fprintf(stderr, "%s[%d]:  ERROR:  this stub should not be called\n", FILE__, __LINE__);
		}
#endif
};

#if 0
template <class T>
class AnnotationContainer<T *> : public AnnotationContainerBase
{
	//  We internally manage the annotation class (which includes serialize function)
	//  for the elements of this container.  When the container is added as a annotation,
	//  the element annotation class is thus not required in addition to the annotation
	//  class that the user must supply for the container overall.
	AnnotationClass<T> *ac;

		COMMON_EXPORT virtual bool deserialize_item(SerializerBase *sb)
		{
			//void *my_item = acb->allocate();
			//assert(my_item);
			assert(ac);
			T *my_item = (T *) ac->allocate();

			ser_func_t sf = ac->getSerializeFunc();
			if (!sf)
			{
				fprintf(stderr, "%s[%d]:  failed to find serialization function\n");
				return false;
			}

			//  execute the serialization function for this annotation
			fprintf(stderr, "%s[%d]:  calling serialize func for type %s\n",
					FILE__, __LINE__, ac->getTypeName());

			(*sf)(my_item, sb, NULL);

			if (!addItem_impl(my_item))
			{
				fprintf(stderr, "%s[%d]:  failed to addItem after deserialize\n", FILE__, __LINE__);
				return false;
			}
			return true;
		}
	protected:

		COMMON_EXPORT AnnotationContainer() 
		{
			std::string aname = std::string(typeid(*this).name()) + std::string("_elem");
			ac = new AnnotationClass<T>(aname);
		}

		COMMON_EXPORT ~AnnotationContainer()
		{ delete ac; }

		//  The routine that actually adds the item and manages whatever data structure
		//  the container is using.  Must be provided by descendant class.
		COMMON_EXPORT virtual bool addItem_impl(T *t)
		{
			fprintf(stderr, "%s[%d]:  ERROR:  this stub should not be called\n", FILE__, __LINE__);
			return false;
		}

	public:

		COMMON_EXPORT bool addItem(T *t) 
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
		}	

		COMMON_EXPORT virtual ser_func_t getSerFunc()
		{
			if (!ac)
			{
				fprintf(stderr, "%s[%d]:  ERROR:  have AnnotationContainer without Element AC\n", 
						FILE__, __LINE__);
				return NULL;
			}
			return ac->getSerializeFunc();
		}

		COMMON_EXPORT virtual AnnotationClassBase *getElementAnnotationClass()
		{
			return ac;
		}

		COMMON_EXPORT virtual void serialize_impl(SerializerBase *, const char *tag) THROW_SPEC(SerializerError)
		{
			fprintf(stderr, "%s[%d]:  ERROR:  this stub should not be called\n", FILE__, __LINE__);
		}
		COMMON_EXPORT virtual void ac_serialize_impl(SerializerBase *, const char *tag) THROW_SPEC(SerializerError)
		{
			fprintf(stderr, "%s[%d]:  ERROR:  this stub should not be called\n", FILE__, __LINE__);
		}
};
#endif

class SerFile;
class SerDes;
#if 0
class SerializerBase {

	public:
		//  TODO:  make these private or protected
		COMMON_EXPORT static dyn_hash_map<std::string, SerializerBase *> active_bin_serializers;
		static bool global_disable;
	private:

		SerFile *sf;
		SerDes *sd;
		SerContextBase *scon;

		std::string serializer_name;

		typedef dyn_hash_map<std::string, SerializerBase *> subsystem_serializers_t;
		COMMON_EXPORT static dyn_hash_map<std::string, subsystem_serializers_t> all_serializers;

	public:
		COMMON_EXPORT static void globalDisable()
		{
			global_disable = true;
		}
		COMMON_EXPORT static bool serializationDisabled()
		{
			return global_disable; 
		}

		COMMON_EXPORT static void globalEnable()
		{
			global_disable = false;
		}
		COMMON_EXPORT SerContextBase *getContext() {return scon;}
		COMMON_EXPORT virtual bool isXML() = 0;
		COMMON_EXPORT virtual bool isBin ()= 0;
		COMMON_EXPORT bool isInput () {return iomode() == sd_deserialize;}
		COMMON_EXPORT bool isOutput () {return iomode() == sd_serialize;}

		COMMON_EXPORT static void dumpActiveBinSerializers();

		COMMON_EXPORT SerializerBase(SerContextBase *scb, std::string name_, std::string filename, 
				iomode_t dir, bool verbose); 

		COMMON_EXPORT SerializerBase();

		COMMON_EXPORT virtual ~SerializerBase() 
		{
			serialize_printf("%s[%d]:  serializer %p-%sdtor\n", FILE__, __LINE__, 
					this, serializer_name.c_str());
		}

		COMMON_EXPORT virtual SerDes &getSD()  { assert(sd); return *sd;}
		COMMON_EXPORT SerFile &getSF() {assert(sf); return *sf;}
		COMMON_EXPORT std::string &name() {return serializer_name;}
		COMMON_EXPORT static SerializerBase *getSerializer(std::string subsystem, std::string fname);
		COMMON_EXPORT static bool addSerializer(std::string subsystem, std::string fname, SerializerBase *sb);

		COMMON_EXPORT virtual void vector_start(unsigned int &, const char * = NULL);
		COMMON_EXPORT virtual void vector_end();
		COMMON_EXPORT virtual void hash_map_start(unsigned int &size, const char *tag = NULL); 
		COMMON_EXPORT virtual void hash_map_end();
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

		COMMON_EXPORT virtual iomode_t iomode(); 

		COMMON_EXPORT void serialize_annotations(void *, std::vector<ser_rec_t> &sers, const char * = NULL);
};
#endif
#if 0
template <class T>
class ScopedSerializerBase : public SerializerBase
{
	T *scope;
	public:
	ScopedSerializerBase(T *scope_, const char *name_, std::string filename, 
			iomode_t dir, bool verbose) :
		SerializerBase(name_, filename, dir, verbose), scope(scope_) {}
	ScopedSerializerBase(T *scope_) :
		SerializerBase(), scope(scope_) {}
	virtual ~ScopedSerializerBase() {}
	T *getScope() {return scope;}
};
#endif
class SerDesXML;


#if 0
bool start_xml_elem(SerDesXML &, const char *);
bool end_xml_elem(SerDesXML &);
#endif

#if 0
class SerializerXML : public SerializerBase
{
	public:
		COMMON_EXPORT virtual bool isXML() {return true;}
		COMMON_EXPORT virtual bool isBin () {return false;}

		COMMON_EXPORT SerializerXML(SerContextBase *sc, std::string name_, std::string filename,
				iomode_t dir, bool verbose) :
			SerializerBase(sc, name_, filename, dir, verbose) {}

		COMMON_EXPORT virtual ~SerializerXML() {}

		COMMON_EXPORT SerDesXML &getSD_xml();

		COMMON_EXPORT static bool start_xml_element(SerializerBase *sb, const char *tag);
		COMMON_EXPORT static bool end_xml_element(SerializerBase *sb, const char *);
};
#endif

#if 0
class SerDesBin;

class SerializerBin : public SerializerBase {
	friend class SerDesBin;

	public:
	virtual bool isXML() {return false;}
	virtual bool isBin () {return true;}

	SerializerBin()  :
		SerializerBase() {}


	SerializerBin(SerContextBase *s, std::string name_, std::string filename,
			iomode_t dir, bool verbose); 

	virtual ~SerializerBin()
	{
		serialize_printf("%s[%d]:  WELCOME TO SERIALIZER_BIN dtor\n", FILE__, __LINE__);
		dyn_hash_map<std::string, SerializerBase *>::iterator iter;

		SerializerBase *sb = this;
		iter = sb->active_bin_serializers.find(sb->name());

		if (iter == sb->active_bin_serializers.end())
		{
			fprintf(stderr, "%s[%d]:  Weird, no static ptr for name %s\n",
					FILE__, __LINE__, sb->name().c_str());
		}
		else
		{
			serialize_printf("%s[%d]:  Removing active serializer for name %s\n",
					FILE__, __LINE__, sb->name().c_str());
			sb->active_bin_serializers.erase(iter);
		}

	}

	SerDesBin &getSD_bin();

	static SerializerBin *findSerializerByName(const char *name_);

};
#endif

//void vector_start(SerializerBase *, unsigned long &, const char *);
//void vector_end(SerializerBase *);

template <class T>
void translate_vector(SerializerBase *ser, std::vector<T> &vec,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned long nelem = 0UL;
	if (ser->iomode() == sd_serialize)
		nelem = vec.size();

	ser->vector_start(nelem, tag);

	if (ser->iomode() == sd_deserialize) 
	{
		if (vec.size())
			SER_ERR("nonempty vector used to create");

		//  zero size vectors are allowed
		serialize_printf("%s[%d]:  about to resize vector to %lu\n", 
				FILE__, __LINE__, nelem);
		fprintf(stderr, "%s[%d]:  about to resize vector to %lu\n", 
				FILE__, __LINE__, nelem);

		if (nelem)
			vec.resize(nelem);
	}

	for (unsigned long i = 0; i < vec.size(); ++i) 
	{
		T &t = vec[i];
		gtranslate(ser, t, elem_tag);
	}

	ser->vector_end();
}


#if 0
	template <class S, class T>
void translate_vector(S *ser, std::vector<T *> &vec, 
		const char *tag = NULL, const char *elem_tag = NULL) 
{
	unsigned long nelem = vec.size();
	vector_start(ser,nelem, tag);

	//if (ser->iomode() == sd_deserialize) 
	if (sb_is_input(ser)) 
	{
		if (vec.size()) 
			SER_ERR("nonempty vector used to create");

		//  zero size vectors are allowed
		if (nelem) 
		{
			//  block-allocate array of underlying type, then assign to our vector
			//  What happens if an individual elem is later deleted??
			T *chunk_alloc = new T[nelem];
			vec.resize(nelem);
			for (unsigned int i = 0; i < nelem; ++i)
				vec[i] = &(chunk_alloc[i]);
		}
	}

	for (unsigned int i = 0; i < vec.size(); ++i) 
	{
		T &t = *(vec[i]);
		gtranslate(ser, t, elem_tag);
	}

	vector_end(ser);
}
#endif

#if 0
	template <class S, class T>
void translate_vector(S *ser, std::vector<std::vector<T> > &vec, 
		const char *tag = NULL, const char *elem_tag = NULL) 
{
   fprintf(stderr, "%s[%d]:  welcome to translate vector of vectors\n", 
           __FILE__, __LINE__);

   unsigned int nelem = vec.size();
   vector_start(ser,nelem, tag);
   //if (ser->iomode() == sd_deserialize) 
   if (sb_is_input(ser)) 
   {
      if (vec.size())
         SER_ERR("nonempty vector used to create");

      //  zero size vectors are allowed
      //  what it T is a complex type (with inheritance info)??
      //  does resize() call default ctors, or should we do that
      //  manually here? look this up.
      if (nelem)
         vec.resize(nelem);
   }

   for (unsigned int i = 0; i < vec.size(); ++i) 
   {
      std::vector<T> &tv = vec[i];
      translate_vector(ser,tv, tag, elem_tag);
   }

   vector_end(ser);
}
#endif

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
	void operator()(SerializerBase *ser, std::pair<T, T2> &p, const char *tag = NULL, const char *tag2 = NULL)
{
	ser->pair_start();

#if 0
	typename boost::remove_cv< typename std::pair<T, T2>::first_type> &ft_t = p.first;
	typename boost::remove_cv< typename std::pair<T, T2>::second_type> &st_t = p.second;
	if (ser->iomode() == sd_serialize)
	{
		ft_t = p.first;
		st_t = p.second;
	}
#endif
	gtranslate(ser,  p.first, tag);
	gtranslate(ser,  p.second, tag2);

#if 0
	if (ser->iomode() == sd_deserialize)
	{
		p.first = ft_t;
		p.second = ft_t;
	}
#endif
	ser->pair_end();
}
};

template <class S, class K, class V>
void translate_dyn_hash_map(S *ser, dyn_hash_map<K, V> &map,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	fprintf(stderr, "%s[%d]:  welcome to translate_dyn_hash_map<%s, %s>, size = %ld\n", FILE__, __LINE__, typeid(K).name(), typeid(V).name(), map.size());

	unsigned long nelem = 0UL;
	nelem = map.size();
	ser->hash_map_start(nelem, tag);
	fprintf(stderr, "%s[%d]:  translate_hash_map, nelem = %lu\n", FILE__, __LINE__, nelem);

	if (ser->iomode() == sd_deserialize)
	{
		if (map.size())
			SER_ERR("nonempty vector used to create");

		//typename dyn_hash_map<K, V>::iterator lastentry = map.begin();
		//  cannot do any kind of bulk allocation with maps
		for (unsigned long i = 0; i < nelem; ++i)
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
		unsigned long elem_count = 0;
		assert (ser->iomode() == sd_serialize);
		typename dyn_hash_map<K, V>::iterator iter = map.begin();
		while (iter != map.end())
		//for (iter = map.begin(); iter != map.end(); iter++)
		{
			K &a_k = const_cast<K &>(iter->first);
			V &a_v = const_cast<V &>(iter->second);
			gtranslate(ser, a_k, elem_tag);
			gtranslate(ser, a_v, elem_tag);
			iter++;
			elem_count++;
		}
		if (nelem != elem_count)
		{
			fprintf(stderr, "%s[%d]:  elem count failed to match:  %lu, not %lu\n", 
					FILE__, __LINE__, elem_count, nelem);
			abort();
		}
	}
	ser->hash_map_end();
	fprintf(stderr, "%s[%d]:  leaving translate_dyn_hash_map<%s, %s>\n", FILE__, __LINE__, typeid(K).name(), typeid(V).name());
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
			SER_ERR("nonempty vector used to create");

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
	void operator()(SerializerBase *ser, std::multimap<K, V, L> &map, const char *tag = NULL, const char *elem_tag = NULL)
	{
	unsigned long nelem = 0UL;
	nelem = map.size();
	ser->multimap_start(nelem, tag);

	if (ser->iomode() == sd_deserialize)
	{
		if (map.size())
			SER_ERR("nonempty vector used to create");

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
			SER_ERR("nonempty vector used to create");

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
	fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
			__FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de"); 

	if (ser->iomode() == sd_serialize) 
   {
      typename dyn_hash_map<K,V>::iterator iter = hash.begin();
      fprintf(stderr, "%s[%d]:  about to serialize hash with %lu elements\n", 
              __FILE__, __LINE__, hash.size());

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
         ser->translate_base(k, key_tag);
         ser->translate_base(v, value_tag);
         hash[k] = v;
      }
   }

   ser->hash_map_end();
}

template <class S, class K, class V>
void translate_hash_map(S *ser, dyn_hash_map<K, V *> &hash,
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s*>()\n", 
         __FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned long nelem = 0UL;
   nelem = hash.size();
   ser->hash_map_start(nelem, tag);

   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
         __FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

   if (ser->iomode() == sd_serialize) 
   {
      typename dyn_hash_map<K,V *>::iterator iter = hash.begin();

      while (iter != hash.end()) 
      {
         K k = iter->first;
         V *v = iter->second;
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         iter++;
      }
   }
   else 
   {
      //  can we do some kind of preallocation here?
      for (unsigned long i = 0; i < nelem; ++i) 
      {
         K k;
         V *v = new V();
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         hash[k] = v;
      }
   }
   ser->hash_map_end();
}

#if 0
template <class S, class K, class V>
void translate_hash_map(S *ser, dyn_hash_map<K, char *> &hash,
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{
   //  THIS SPECIALIZATION DOES NOT WORK CORRECTLY (YET)
   fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s*>()\n", 
         __FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);

   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
         __FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

   if (ser->iomode() == sd_serialize) 
   {
      typename dyn_hash_map<K,V *>::iterator iter = hash.begin();
      
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
      for (unsigned int i = 0; i < nelem; ++i) 
      {
         K k;
         V v;
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         hash[k] = v;
      }
   }
   ser->hash_map_end();
}
#endif


#if 0
COMMON_EXPORT void trans_adapt(SerializerBase *ser, Serializable &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, Serializable *itp,  const char *tag);

COMMON_EXPORT void trans_adapt(SerializerBase *ser, bool &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, int &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, unsigned int &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, long &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, unsigned long &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, char &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, char *&it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, std::string &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, float &it,  const char *tag);
COMMON_EXPORT void trans_adapt(SerializerBase *ser, double &it,  const char *tag);
#endif

COMMON_EXPORT bool isBinary(Dyninst::SerializerBase *ser);
COMMON_EXPORT bool isOutput(Dyninst::SerializerBase *ser);

typedef void NOTYPE_T;

template<typename T, typename T2 = NOTYPE_T, 
	bool = boost::is_base_of<Serializable, T>::value>
class trans_adaptor {
   public:
      trans_adaptor()  {}

      T * operator()(SerializerBase *ser, T & it, const char *tag = NULL, const char * /*tag2*/ = NULL)
      {
		 ser->translate_base(it, tag);
         return &it;
      }
};

template <typename T2>
class trans_adaptor<char *, T2, false> {
   public:
      trans_adaptor()  {}

      char * * operator()(SerializerBase *ser, char * & it, const char *tag = NULL, const char * /*tag2*/ = NULL)
      {
		  int len = it ? strlen(it) : 0;
		 ser->translate_base(it, len, tag);
         return &it;
      }
};

template<typename T, typename T2>
class trans_adaptor<T, T2, true> 
{
   public:
      trans_adaptor()  {}

      Serializable * operator()(SerializerBase *ser, T & it, const char *tag = NULL, 
            const char * /*tag2*/ = NULL)
      {
         //gtranslate(ser, it, tag);
		  serialize_printf("%s[%d]:  translating serializable: %s\n", FILE__, __LINE__, typeid(T).name());
		  it.serialize(ser, tag);
		  serialize_printf("%s[%d]:  translated serializable: %s\n", FILE__, __LINE__, typeid(T).name());
         return &it;
      }
};

template<typename T, typename T2>
class trans_adaptor<std::vector<T>, T2, false> 
{
   public:
      trans_adaptor() {}

      std::vector<T> * operator()(SerializerBase *ser, std::vector<T> &v, const char *tag = NULL, 
            const char *tag2 = NULL) 
      {
         translate_vector(ser, v, tag, tag2);         //  maybe catch errors here?
         return &v;
      }
};

#if 0
template<class T, class T2>
class trans_adaptor<std::vector<T *>, T2>  {
   public: 
      trans_adaptor()  {}

      std::vector<T*> * operator()(SerializerBase *ser, std::vector<T *> &v, const char *tag = NULL, 
            const char *tag2 = NULL) 
      {
         translate_vector(ser, v, tag, tag2);
         //  maybe catch errors here?
         return &v;
      }
};
#endif

#if 0
template<typename T, typename T2>
class trans_adaptor<std::map<T, T2>, T2,  false >  
{
	public:
		trans_adaptor() {}

		std::map<T, T2> * operator()(SerializerBase *ser, std::map<T, T2> &v, const char *tag = NULL,
				const char *tag2 = NULL)
		{
			translate_map(ser, v, tag, tag2);
			//  maybe catch errors here?
			return &v;
		}
};

template<typename T, typename T2 >
class trans_adaptor<dyn_hash_map<T, T2>, T2, false >  
{
	public:
		trans_adaptor() {}

		dyn_hash_map<T, T2> * operator()(SerializerBase *ser, dyn_hash_map<T, T2> &v, const char *tag = NULL,
				const char *tag2 = NULL)
		{
			translate_dyn_hash_map(ser, v, tag, tag2);
			//  maybe catch errors here?
			return &v;
		}
};

template<typename T, typename T2>
//class trans_adaptor<std::multimap<T, T2>, T2>  
class trans_adaptor<std::multimap<T, T2>, T2, false>  
{
	public:
		trans_adaptor() {}

		std::multimap<T, T2> * operator()(SerializerBase *ser, std::multimap<T, T2> &m, const char *tag = NULL, 
				const char *tag2 = NULL)
		{
			serialize_printf("%s[%d]:  trans_adaptor  -- multimap<%s, %s>\n",
					__FILE__, __LINE__, typeid(T).name(), typeid(T2).name());
			translate_multimap(ser, m, tag, tag2);
			//multimap_translator<T, T2>(ser, m, tag, tag2);
			//  maybe catch errors here?
			return &m;
		}
};

template<typename T, typename T2>
class trans_adaptor<std::pair<T, T2>, T2, false >  
{
	public:
		trans_adaptor() {}

		std::pair<T, T2> * operator()(SerializerBase *ser, std::pair<T, T2> &p, const char *tag = NULL,
				const char *tag2 = NULL)
		{
			serialize_printf("%s[%d]:  trans_adaptor  -- pair<%s, %s>\n",
					__FILE__, __LINE__, typeid(T).name(), typeid(T2).name());
			translate_pair(ser, p, tag, tag2);
			//  maybe catch errors here?
			return &p;
		}
};
#endif



#if 0
typename boost::is_base_of<Serializable,
		                 typename boost::remove_pointer<T>::type>::type
#endif
typedef void NULL_T;
template <typename T, bool = boost::is_base_of<Serializable, typename boost::remove_pointer<T>::type>::value>
class deref_adaptor 
{
	public:
		deref_adaptor() {}
		T * operator()(SerializerBase *sb, T &it, const char *tag = NULL, const char *tag2 = NULL)
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
		T * operator()(SerializerBase *sb, T *&it, const char *tag = NULL, const char *tag2 = NULL)
		{
			//  need to allocate upon deserialize
			if (!isOutput(sb))
			{
				it = new T();
			}
			//trans_adaptor<T> ta;
			//ta(sb, *it, tag, tag2);
			gtranslate(sb, *it, tag, tag2);
			return it;
		}
};

#if 0
//  Expand to include pointers to other common comtainer classes
template <typename T>
class deref_adaptor<std::vector<T> *, false> 
{
	public:
		deref_adaptor() {}
		std::vector<T> ** operator()(SerializerBase *sb, std::vector<T> * &it, const char *tag = NULL, const char *tag2 = NULL)
		{
			//  need to allocate upon deserialize
			if (!isOutput(sb))
			{
				it = new std::vector<T>();
			}
			trans_adaptor<std::vector<T> > ta;
			ta(sb, *it, tag, tag2);
			return &it;
		}
};
#endif

template <class T, bool = boost::is_pointer<T>::value>
class ptr_adaptor 
{
	public:
		ptr_adaptor() {}
		T * operator()(SerializerBase *sb, T &it, const char *tag = NULL, const char *tag2 = NULL)
		{
			trans_adaptor<T> ta;
			ta(sb, it, tag, tag2);
			//deref_adaptor<T> da;
			//da(sb, it, tag, tag2);
			return &it;
		}
};

template <class T>
class ptr_adaptor<T, true> 
{
	public:
	typedef typename boost::remove_pointer<T>::type pointed_to_t;
		ptr_adaptor() {}
		T * operator()(SerializerBase *sb, T &it, const char *tag = NULL, const char *tag2 = NULL)
		{
			deref_adaptor<pointed_to_t> da;
			da(sb, it, tag, tag2);
			return &it;
		}
};

//  Another specialization:  do _not_ dereference any char *'s, they're special
//  (maybe want to do this for all fundamental types)
template <>
class ptr_adaptor<char *, true> 
{
	public:
		ptr_adaptor() {}
		char * * operator()(SerializerBase *sb, char * &it, const char *tag = NULL, const char *tag2 = NULL)
		{
			trans_adaptor<char *> ta;
			ta(sb, it, tag, tag2);
			return &it;
		}
};

template <typename T, bool  = boost::is_const<T>::value>
class const_adaptor {
	public:
	const_adaptor() {}
	T *operator()(SerializerBase *sb, T &it, const char *tag = 0, const char *tag2 = 0)
	{
		ptr_adaptor<T> ta;
		//fprintf(stderr, "%s[%d]: gtranslate: before operation\n", __FILE__, __LINE__);

		T *itp = ta(sb, it, tag, tag2);
		return itp;
	}
};
template <typename T>
class const_adaptor<T, true>
{
	public:
		typedef typename boost::remove_cv<T>::type non_const_type;
	const_adaptor() {}
	T *operator()(SerializerBase *sb, T &it, const char *tag = 0, const char *tag2 = 0)
	{
		//ptr_adaptor<non_const_type> ta;
		//fprintf(stderr, "%s[%d]: gtranslate: before operation\n", __FILE__, __LINE__);

	//	T *itp = ta(sb, const_cast<non_const_type &>(it), tag, tag2);
		gtranslate(sb, const_cast<non_const_type &>(it), tag, tag2);
		return &it;
	}
};

template <class T>
void gtranslate(SerializerBase *ser, T &it, const char *tag = NULL, const char *tag2 = NULL)
{
   //fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
   //      __FILE__, __LINE__,
   //      "SerializerBase",
   //      typeid(T).name(), &it);

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization

   //fprintf(stderr, "%s[%d]: gtranslate: before operation\n", __FILE__, __LINE__);
   const_adaptor<T> ta;
   T *itp = ta(ser, it, tag, tag2);

#if 0
   ptr_adaptor<T> ta;

   T *itp = ta(ser, it, tag, tag2);
#endif

   if (!itp) 
   {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", 
            __FILE__, __LINE__);
   }
}

template <typename T, typename T2>
void gtranslate(SerializerBase *ser, std::pair<T, T2> &it, const char *tag = NULL, const char *tag2 = NULL)
{
	//translate_pair(ser, it, tag, tag2);
	pair_translator<T, T2> pt;
	pt(ser, it, tag, tag2);
}

template <typename K, typename V, typename L>
void gtranslate(SerializerBase *ser, std::multimap<K, V, L> &it, const char *tag = NULL, const char *tag2 = NULL)
{
	translate_multimap(ser, it, tag, tag2);
}
#if 0
template <class T>
void gtranslate(SerializerBase *ser, T &it, const char *tag = NULL, const char *tag2 = NULL)
{
   //fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
   //      __FILE__, __LINE__,
   //      "SerializerBase",
   //      typeid(T).name(), &it);

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization

   trans_adaptor<T> ta;
   //fprintf(stderr, "%s[%d]: gtranslate: before operation\n", __FILE__, __LINE__);

   T *itp = ta(ser, it, tag, tag2);

   if (!itp) 
   {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", 
            __FILE__, __LINE__);
   }
}
#endif

COMMON_EXPORT bool ifxml_start_element(SerializerBase *sb, const char *tag);
COMMON_EXPORT bool ifxml_end_element(SerializerBase *sb, const char * /*tag*/);
COMMON_EXPORT bool sb_is_input(SerializerBase *sb);
COMMON_EXPORT bool sb_is_output(SerializerBase *sb);


template <class T>
void gtranslate(SerializerBase *ser, 
      T &it, 
      const char * (*to_str_func)(T), 
      const char *tag = NULL, 
      const char * /*tag2*/ = NULL)
{
   assert(ser);
   int enum_int = (int) it;

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
      gtranslate(ser, enum_int, tag, NULL);
      it = (T) enum_int;
   }
}

class SerializerError;

template <class T>
bool gtranslate_w_err(SerializerBase *ser, T&it, const char *tag = NULL, const char *tag2 = NULL)
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
