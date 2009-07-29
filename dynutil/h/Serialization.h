
#if !defined (SERIALIZATION_PUBLIC_H)
#define SERIALIZATION_PUBLIC_H
//  Hopefully just a few definitions allowing for a public interface to 
//  serializing user-providede annotations

#include <stdio.h>
#include <assert.h>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include <map>
#include <assert.h>
#include "dyntypes.h"
#include "util.h"

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

class Serializable {
   protected:
      COMMON_EXPORT Serializable() {}
      COMMON_EXPORT virtual ~Serializable() {}

      COMMON_EXPORT virtual void serialize_impl(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError) = 0;
   public:
      COMMON_EXPORT virtual void serialize(SerializerBase *,  const char * = NULL) THROW_SPEC(SerializerError);
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
