
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
typedef bool (*deserialize_and_annotate_t)(SerializerBase *, void *parent);

bool addDeserializeFuncForType(deserialize_and_annotate_t, const std::type_info *);
deserialize_and_annotate_t getDeserializeFuncForType(const std::type_info *);

class SerializerError : public std::runtime_error {
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

   std::string file__;
   int line__;
   SerializerErrorType err__;

   public:


   COMMON_EXPORT SerializerError(const std::string &__file__, 
         const int &__line__, 
         const std::string &msg, 
         SerializerErrorType __err__ = ser_err_unspecified) :
      runtime_error(msg),
      file__(__file__),
      line__(__line__),
      err__(__err__)
   {}

   COMMON_EXPORT virtual ~SerializerError() THROW {}

   COMMON_EXPORT std::string file() const {return file__;}
   COMMON_EXPORT int line() const {return line__;}
   COMMON_EXPORT SerializerErrorType code() const {return err__;}
};


COMMON_EXPORT void printSerErr(const SerializerError &err);

class AnnotatableSparse;
class AnnotatableDense;
bool serializeAnnotationsWrapper(AnnotatableSparse *, SerializerBase *, const char *);
bool serializeAnnotationsWrapper(AnnotatableDense *, SerializerBase *, const char *);

class Serializable {
   protected:
      COMMON_EXPORT Serializable() {}
      COMMON_EXPORT virtual ~Serializable() {}

      COMMON_EXPORT virtual void serialize_impl(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError) = 0;
   public:
      //COMMON_EXPORT virtual void serialize(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError);
      COMMON_EXPORT virtual void serialize(SerializerBase *sb,  const char *tag = NULL) THROW_SPEC(SerializerError)
	  {
		  //  do base serialization for this class
		  serialize_impl(sb, tag);

		  //  then serialize all Annotations for which a serialization function has been provided
		  AnnotatableSparse *as = dynamic_cast<AnnotatableSparse *> (this);
		  AnnotatableDense *ad = dynamic_cast<AnnotatableDense *> (this);
		  if (as)
		  {
			  as->serializeAnnotations(sb, tag);
			  //serializeAnnotationsWrapper(as, sb, tag);
		  }
		  else if (ad)
		  {
			  ad->serializeAnnotations(sb, tag);
			  //serializeAnnotationsWrapper(ad, sb, tag);
		  }
		  else
			  fprintf(stderr, "%s[%d]:  class is not annotatable\n", FILE__, __LINE__);

	  }
};

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

		COMMON_EXPORT SerializerBase(SerContextBase *scb, const char *name_, std::string filename, 
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


bool start_xml_elem(SerDesXML &, const char *);
bool end_xml_elem(SerDesXML &);

class SerializerXML : public SerializerBase
{
	public:
		COMMON_EXPORT virtual bool isXML() {return true;}
		COMMON_EXPORT virtual bool isBin () {return false;}

		COMMON_EXPORT SerializerXML(SerContextBase *sc, const char *name_, std::string filename,
				iomode_t dir, bool verbose) :
			SerializerBase(sc, name_, filename, dir, verbose) {}

		COMMON_EXPORT virtual ~SerializerXML() {}

		COMMON_EXPORT SerDesXML &getSD_xml();
#if 0
		COMMON_EXPORT SerDesXML &getSD_xml()
		{
			SerializerBase *sb = this;
			SerDes &sd = sb->getSD();
			SerDesXML *sdxml = dynamic_cast<SerDesXML *> (&sd);
			assert(sdxml);
			return *sdxml;
		}
#endif

		COMMON_EXPORT static bool start_xml_element(SerializerBase *sb, const char *tag);
		COMMON_EXPORT static bool end_xml_element(SerializerBase *sb, const char *);
#if 0
		COMMON_EXPORT static bool start_xml_element(SerializerBase *sb, const char *tag)
		{
			SerializerXML *sxml = dynamic_cast<SerializerXML *>(sb);

			if (!sxml)
			{
				fprintf(stderr, "%s[%d]:  FIXME:  called xml function with non xml serializer\n",
						FILE__, __LINE__);
				return false;
			}

			SerDesXML sdxml = sxml->getSD_xml();
			start_xml_elem(sdxml, tag);
			return true;
		}


		COMMON_EXPORT static bool end_xml_element(SerializerBase *sb, const char *)
		{
			SerializerXML *sxml = dynamic_cast<SerializerXML *>(sb);

			if (!sxml)
			{
				fprintf(stderr, "%s[%d]:  FIXME:  called xml function with non xml serializer\n",
						FILE__, __LINE__);
				return false;
			}

			SerDesXML sdxml = sxml->getSD_xml();
			end_xml_elem(sdxml);
			//end_xml_elem(sdxml.writer);

			return true;
		}
#endif
};

class SerDesBin;

class SerializerBin : public SerializerBase {
	friend class SerDesBin;

	public:
	virtual bool isXML() {return false;}
	virtual bool isBin () {return true;}

	SerializerBin()  :
		SerializerBase() {}


	SerializerBin(SerContextBase *s, const char *name_, std::string filename,
			iomode_t dir, bool verbose) :
		SerializerBase(s, name_, filename, dir, verbose)
	{
		SerializerBase *sb = this;
		if (sb->serializationDisabled())
		{
			fprintf(stderr, "%s[%d]:  Failing to construct Bin Translator:  global disable set\n",
					FILE__, __LINE__);

			throw SerializerError(FILE__, __LINE__,
					std::string("serialization disabled"),
					SerializerError::ser_err_disabled);
		}

		dyn_hash_map<std::string, SerializerBase *>::iterator iter;

		iter = sb->active_bin_serializers.find(std::string(name_));

		if (iter == sb->active_bin_serializers.end())
		{
			serialize_printf("%s[%d]:  Adding Active serializer for name %s\n",
					FILE__, __LINE__, name_);

			sb->active_bin_serializers[std::string(name_)] = this;
		}
		else
		{
			fprintf(stderr, "%s[%d]:  Weird, already have active serializer for name %s\n",
					FILE__, __LINE__, name_);
		}
	}


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
#if 0
	SerDesBin &getSD_bin()
	{
		SerializerBase *sb = this;
		SerDes &sd = sb->getSD();
		SerDesBin *sdbin = dynamic_cast<SerDesBin *> (&sd);
		assert(sdbin);
		return *sdbin;
	}
#endif



	static SerializerBin *findSerializerByName(const char *name_);
#if 0
	static SerializerBin *findSerializerByName(const char *name_)
	{
		dyn_hash_map<std::string, SerializerBase *>::iterator iter;

		iter = SerializerBase::active_bin_serializers.find(std::string(name_));

		if (iter == SerializerBase::active_bin_serializers.end())
		{
			fprintf(stderr, "%s[%d]:  No static ptr for name %s\n",
					FILE__, __LINE__, name_);
			SerializerBase::dumpActiveBinSerializers();
		}
		else
		{
			fprintf(stderr, "%s[%d]:  Found active serializer for name %s\n",
					FILE__, __LINE__, name_);

			return iter->second;
		}

		return NULL;
	}
#endif


};


template <class S, class T>
void translate_vector(S *ser, std::vector<T> &vec,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned int nelem = vec.size();
	ser->vector_start(nelem, tag);

	if (ser->iomode() == sd_deserialize) 
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
		T &t = vec[i];
		gtranslate(ser, t, elem_tag);
	}

	ser->vector_end();
}


	template <class S, class T>
void translate_vector(S *ser, std::vector<T *> &vec, 
		const char *tag = NULL, const char *elem_tag = NULL) 
{
	unsigned int nelem = vec.size();
	ser->vector_start(nelem, tag);

	if (ser->iomode() == sd_deserialize) 
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

	ser->vector_end();
}

	template <class S, class T>
void translate_vector(S *ser, std::vector<std::vector<T> > &vec, 
		const char *tag = NULL, const char *elem_tag = NULL) 
{
   fprintf(stderr, "%s[%d]:  welcome to translate vector of vectors\n", 
           __FILE__, __LINE__);

   unsigned int nelem = vec.size();
   ser->vector_start(nelem, tag);
   if (ser->iomode() == sd_deserialize) 
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

   ser->vector_end();
}

template <class S, class K, class V>
void translate_pair(S *ser, std::pair<K, V> &p,
		const char *tag = NULL, const char *tag2 = NULL)
{
	ser->pair_start();

	gtranslate(ser, p.first, tag);
	gtranslate(ser, p.second, tag2);

	ser->pair_end();
}

template <class S, class K, class V>
void translate_map(S *ser, std::map<K, V> &map,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned int nelem = map.size();
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

template <class S, class K, class V>
void translate_multimap(S *ser, std::multimap<K, V> &map,
		const char *tag = NULL, const char *elem_tag = NULL)
{
	unsigned int nelem = map.size();
	ser->multimap_start(nelem, tag);

	if (ser->iomode() == sd_deserialize)
	{
		if (map.size())
			SER_ERR("nonempty vector used to create");

		typename std::multimap<K, V>::iterator lastentry = map.begin();
		//  cannot do any kind of bulk allocation with multimaps
		for (unsigned int i = 0; i < nelem; ++i)
		{
			typename std::multimap<K, V>::value_type mapentry;
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
		typename std::multimap<K, V>::iterator iter = map.begin();
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

	unsigned int nelem = hash.size();
	ser->hash_map_start(nelem, tag);
	fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
			__FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de"); 

	if (ser->iomode() == sd_serialize) 
   {
      typename dyn_hash_map<K,V>::iterator iter = hash.begin();
      fprintf(stderr, "%s[%d]:  about to serialize hash with %d elements\n", 
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
      for (unsigned int i = 0; i < nelem; ++i) 
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
         V *v = iter->second;
         ser->translate_base(k, key_tag);
         ser->translate_base(*v, value_tag);
         iter++;
      }
   }
   else 
   {
      //  can we do some kind of preallocation here?
      for (unsigned int i = 0; i < nelem; ++i) 
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

COMMON_EXPORT bool isBinary(Dyninst::SerializerBase *ser);
COMMON_EXPORT bool isOutput(Dyninst::SerializerBase *ser);

typedef void NOTYPE_T;
template<class S, class T, class T2 = NOTYPE_T>
class trans_adaptor {
   public:
      trans_adaptor() 
      {
         //fprintf(stderr, "%s[%d]:  trans_adaptor  -- general\n", __FILE__, __LINE__);
      } 

      T * operator()(S *ser, T & it, const char *tag = NULL, const char * /*tag2*/ = NULL)
      {
         trans_adapt(ser, it, tag);
         return &it;
      }
};

template<class S, class T2>
class trans_adaptor<S, Serializable, T2> {
   public:
      trans_adaptor() 
      {
         serialize_printf("%s[%d]:  trans_adaptor  -- general\n", __FILE__, __LINE__);
      } 

      Serializable * operator()(S *ser, Serializable & it, const char *tag = NULL, 
            const char * /*tag2*/ = NULL)
      {
         gtranslate(ser, it, tag);
         return &it;
      }
};

template<class S, class T, class TT2>
class trans_adaptor<S, std::vector<T>, TT2 > {
   public:
      trans_adaptor()
      {
         serialize_printf("%s[%d]:  trans_adaptor  -- vectorl\n", __FILE__, __LINE__);
      }

      std::vector<T> * operator()(S *ser, std::vector<T> &v, const char *tag = NULL, 
            const char *tag2 = NULL) 
      {
         translate_vector(ser, v, tag, tag2);         //  maybe catch errors here?
         return &v;
      }
};

template<class S, class T, class TT2>
class trans_adaptor<S, std::vector<T *>, TT2>  {
   public: 
      trans_adaptor() 
      {
         serialize_printf("%s[%d]:  trans_adaptor  -- vector of ptrs\n", __FILE__, __LINE__);
      }

      std::vector<T*> * operator()(S *ser, std::vector<T *> &v, const char *tag = NULL, 
            const char *tag2 = NULL) 
      {
         translate_vector(ser, v, tag, tag2);
         //  maybe catch errors here?
         return &v;
      }
};

template<class S, class T, class TT2>
class trans_adaptor<S, std::map<T, TT2> >  {
	public:
		trans_adaptor()
		{
			serialize_printf("%s[%d]:  trans_adaptor  -- std::map\n", __FILE__, __LINE__);
		}

		std::map<T, TT2> * operator()(S *ser, std::map<T, TT2> &v, const char *tag = NULL,
				const char *tag2 = NULL)
		{
			translate_map(ser, v, tag, tag2);
			//  maybe catch errors here?
			return &v;
		}
};

template<class S, class T, class TT2>
class trans_adaptor<S, std::multimap<T, TT2>, TT2>  {
	public:
		trans_adaptor()
		{
			serialize_printf("%s[%d]:  trans_adaptor  -- multimap<%s, %s>\n",
					__FILE__, __LINE__, typeid(T).name(), typeid(TT2).name());
		}

		std::multimap<T, TT2> * operator()(S *ser, std::multimap<T, TT2> &m, const char *tag = NULL, 
				const char *tag2 = NULL)
		{
			translate_multimap(ser, m, tag, tag2);
			//  maybe catch errors here?
			return &m;
		}
};

template<class S, class T, class TT2>
class trans_adaptor<S, std::pair<T, TT2> >  {
	public:
		trans_adaptor()
		{
			serialize_printf("%s[%d]:  trans_adaptor  -- pair<%s, %s>\n",
					__FILE__, __LINE__, typeid(T).name(), typeid(TT2).name());
		}

		std::pair<T, TT2> * operator()(S *ser, std::pair<T, TT2> &p, const char *tag = NULL,
				const char *tag2 = NULL)
		{
			translate_pair(ser, p, tag, tag2);
			//  maybe catch errors here?
			return &p;
		}
};

#if 0
template<class T, class ANNO_NAME, bool, annotation_implementation_t>
class Annotatable<T, ANNO_NAME,bool, annotation_implementation_t>;
#endif

#if 0
I really really wish this worked, maybe it still can given some more pounding

template<class S, class T, class ANNO_NAME, bool SERIALIZABLE, annotation_implementation_t IMPL>
class trans_adaptor<S, 
      Annotatable<T, 
      ANNO_NAME, 
      SERIALIZABLE,
      IMPL> &>  {
   public: 
      trans_adaptor() 
      {
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- annotatable<%s, %s, %s>\n", 
               __FILE__, __LINE__, typeid(T).name(), typeid(ANNO_NAME).name(),
               SERIALIZABLE ? "true" : "false");
      }

      Annotatable<T, ANNO_NAME, SERIALIZABLE, IMPL> * operator()(S *ser, 
            Annotatable<T, ANNO_NAME, SERIALIZABLE, IMPL> &v, const char *tag = NULL, 
            const char *tag2 = NULL) 
      {
          if (!SERIALIZABLE) 
          {
             fprintf(stderr, "%s[%d]:  Annotatable<%s, %s, %s>, not serializable\n", 
                   __FILE__, __LINE__, typeid(T).name(), typeid(ANNO_NAME).name(),
                   SERIALIZABLE ? "true" : "false");
             return NULL;
          }

          int nelem = v.size();

          if (0 == nelem) 
          {
             fprintf(stderr, "%s[%d]:  Annotatable<%s, %s, %s>, no annotations\n", 
                   __FILE__, __LINE__, typeid(T).name(), typeid(ANNO_NAME).name(),
                   SERIALIZABLE ? "true" : "false");
             return NULL;
          }

          //  data structure must exist since size > 0

          std::vector<T> &anno_vec = v.getDataStructure();

          //  But is this OK??  (This goes around the usual annotations interface)
          //  probably not, but let's see if it works anyways

          fprintf(stderr, "%s[%d]:  WARNING:  This may not be kosher -- circumventing the anotations interface, think on this\n", __FILE__, __LINE__);
          translate_vector(ser, anno_vec, tag, tag2);

         //  maybe catch errors here?
         return &v;
      }
};
#endif

template <class S, class T>
void gtranslate(S *ser, T &it, const char *tag = NULL, const char *tag2 = NULL)
{
   //fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
   //      __FILE__, __LINE__,
   //      "SerializerBase",
   //      typeid(T).name(), &it);

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization

   trans_adaptor<S, T> ta;
   //fprintf(stderr, "%s[%d]: gtranslate: before operation\n", __FILE__, __LINE__);

   T *itp = ta(ser, it, tag, tag2);

   if (!itp) 
   {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", 
            __FILE__, __LINE__);
   }
}

COMMON_EXPORT bool ifxml_start_element(SerializerBase *sb, const char *tag);
COMMON_EXPORT bool ifxml_end_element(SerializerBase *sb, const char * /*tag*/);

//template<class T> class SerializerBin<T>;
//template<class T> class SerializerXML<T>;

COMMON_EXPORT bool sb_is_input(SerializerBase *sb);
COMMON_EXPORT bool sb_is_output(SerializerBase *sb);

#if 0
template <class T>
bool ifinput(bool (*f)(SerializerBase *, T*), SerializerBase *sb, T *itp)
{
   if (!sb_is_input(sb))
      return false;


   return (*f)(sb, itp);
}

template <class T>
bool ifoutput(bool (*f)(SerializerBase *, T*), SerializerBase *sb, T *itp)
{
   if (!sb_is_output(sb))
      return false;

   return (*f)(sb, itp);
}

template <class T>
bool ifbin(bool (*f)(SerializerBase *, T*), SerializerBase *sb, T *itp)
{
   SerializerBin *sbin = dynamic_cast<SerializerBin *>(sb);

   if (!sbin)
      return false;

   return (*f)(sbin, itp);
}

template <class T>
bool ifxml(bool (*f)(SerializerBase *, T*), SerializerBase *sb, T *itp)
{
   SerializerXML *sxml = dynamic_cast<SerializerXML *>(sb);

   if (!sxml)
      return false;

   return (*f)(sxml, itp);
}
#endif
#if 0
{
   fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
         __FILE__, __LINE__,
         "SerializerBase",
         typeid(TT).name(), &it);

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization
   assert(use_func);
   (*use_func)(ser, it);
}

template<class S, class TT>
void trans_enum(S *ser, TT &it, std::vector<std::string> *enum_tags_ptr) 
{
   assert(enum_tags_ptr);
   std::vector<std::string> &enum_tags = *enum_tags_ptr;
   assert(it < enum_tags.size());
   unsigned int enum_int = (unsigned int) it;
   gtranslate(ser, it);
}
#endif


template <class S, class T>
void gtranslate(S *ser, 
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

template <class S, class T>
bool gtranslate_w_err(S *ser, T&it, const char *tag = NULL, const char *tag2 = NULL)
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

//  These are for testing purposes -- do not use elsewhere
//  Must deallocate serializer that is returned when done.
#if 0
SerializerBase *nonpublic_make_bin_serializer(std::string file);
SerializerBase *nonpublic_make_bin_deserializer(std::string file);
void nonpublic_free_bin_serializer(SerializerBase *sb);
#endif
#if 0
template <class T> class SerializerBin;

template <class T>
inline SerializerBase *nonpublic_make_bin_serializer(T *t, std::string file)
{
	SerializerBin<T> *ser;
	ser = new SerializerBin<T>(t, "SerializerBin", file, sd_serialize, true);
	return ser;
}

template <class T>
inline void nonpublic_free_bin_serializer(SerializerBase *sb)
{
	SerializerBin<T> *sbin = dynamic_cast<SerializerBin<T> *>(sb);
	if (sbin)
	{
		delete(sbin);
	}
	else
		fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);
}

template <class T>
inline SerializerBase *nonpublic_make_bin_deserializer(T *t,std::string file)
{
	SerializerBin<T> *ser;
	ser = new SerializerBin<T>(t, "DeserializerBin", file, sd_deserialize, true);
	return ser;
}
#endif

} /* namespace Dyninst */
#endif
