
#if !defined (SERIALIZATION_PUBLIC_H)
#define SERIALIZATION_PUBLIC_H
//  Hopefully just a few definitions allowing for a public interface to 
//  serializing user-providede annotations

#include <stdexcept>
#include <stdio.h>
#include "dynutil/h/dyntypes.h"
#include "common/h/Types.h"

//  SER_ERR("msg") -- an attempt at "graceful" failure.  If debug flag is set
//  it will assert, otherwise it throws...  leaving the "graceful" aspect
//  to the next (hopefully top-level) exception handler.


#define SER_ERR(cmsg) \
   do { \
      if (serializer_debug_flag()) { \
         fprintf(stderr, "%s", cmsg); \
         assert (0); \
      } else { \
         throw SerializerError(FILE__, __LINE__, std::string(cmsg)); \
      } \
   } while (0)


class SerializerBase;
typedef enum {sd_serialize, sd_deserialize} iomode_t;

class DLLEXPORT Serializable {
   protected:
      Serializable() {}
      virtual ~Serializable() {}

   public:
      virtual void serialize(SerializerBase *,  const char * = NULL) = 0;
};

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


   SerializerError(const std::string &__file__, 
         const int &__line__, 
         const std::string &msg, 
         SerializerErrorType __err__ = ser_err_unspecified) :
      runtime_error(msg),
      file__(__file__),
      line__(__line__),
      err__(__err__)
   {}

   virtual ~SerializerError() throw() {}

   std::string file() const {return file__;}
   int line() const {return line__;}
   SerializerErrorType code() const {return err__;}
};


DLLEXPORT void printSerErr(const SerializerError &err);

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
         FILE__, __LINE__);

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
void translate_hash_map(S *ser, dyn_hash_map<K, V> &hash, 
      const char *tag = NULL, const char *key_tag = NULL, const char *value_tag = NULL)
{   
   fprintf(stderr, "%s[%d]:  welcome to translate_hash_map<%s, %s>()\n", 
         FILE__, __LINE__,
         typeid(K).name(), typeid(V).name()); 

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);
   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
         FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de"); 

   if (ser->iomode() == sd_serialize) 
   {
      typename dyn_hash_map<K,V>::iterator iter = hash.begin();
      fprintf(stderr, "%s[%d]:  about to serialize hash with %d elements\n", 
            FILE__, __LINE__, hash.size());

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
         FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);

   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
         FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

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
         FILE__, __LINE__,
         typeid(K).name(), typeid(V).name());

   unsigned int nelem = hash.size();
   ser->hash_map_start(nelem, tag);

   fprintf(stderr, "%s[%d]:  after hash_map start, mode = %sserialize\n", 
         FILE__, __LINE__, ser->iomode() == sd_serialize ? "" : "de");

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


DLLEXPORT void trans_adapt(SerializerBase *ser, Serializable &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, Serializable *itp,  const char *tag);

DLLEXPORT void trans_adapt(SerializerBase *ser, bool &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, int &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, unsigned int &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, long &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, unsigned long &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, char &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, char *&it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, std::string &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, float &it,  const char *tag);
DLLEXPORT void trans_adapt(SerializerBase *ser, double &it,  const char *tag);

typedef void NOTYPE_T;
template<class S, class T, class T2 = NOTYPE_T>
class trans_adaptor {
   public:
      trans_adaptor() 
      {
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- general\n", FILE__, __LINE__);
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
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- general\n", FILE__, __LINE__);
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
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- vectorl\n", FILE__, __LINE__);
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
         fprintf(stderr, "%s[%d]:  trans_adaptor  -- vector of ptrs\n", FILE__, __LINE__);
      }

      std::vector<T*> * operator()(S *ser, std::vector<T *> &v, const char *tag = NULL, 
            const char *tag2 = NULL) 
      {
         translate_vector(ser, v, tag, tag2);
         //  maybe catch errors here?
         return &v;
      }
};


template <class S, class TT>
void gtranslate(S *ser, TT&it, const char *tag = NULL, const char *tag2 = NULL)
{
   fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
         FILE__, __LINE__,
         "SerializerBase",
         typeid(TT).name(), &it);

   //  Maybe just need to do try/catch here since the template mapping may 
   //  change the type of return value thru template specialization

   trans_adaptor<S, TT> ta;
   fprintf(stderr, "%s[%d]: gtranslate: before operation\n", FILE__, __LINE__);

   TT *itp = ta(ser, it, tag, tag2);

   if (!itp) 
   {
      fprintf(stderr, "%s[%d]: translate adaptor failed to de/serialize\n", 
            FILE__, __LINE__);
   }
}

DLLEXPORT bool ifxml_start_element(SerializerBase *sb, const char *tag);
DLLEXPORT bool ifxml_end_element(SerializerBase *sb, const char * /*tag*/);

class SerializerBin;
class SerializerXML;

bool sb_is_input(SerializerBase *sb);
bool sb_is_output(SerializerBase *sb);

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

#if 0
template <class S, class TT>
void gtranslate(S *ser, TT&it, void (*use_func)(SerializerBase *, TT &, void *), const char *tag = NULL, const char *tag2)
{
   fprintf(stderr, "%s[%d]:  welcome to gtranslate<%s, %s>(%p)\n",
         FILE__, __LINE__,
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

DLLEXPORT bool isBinary(SerializerBase *ser);
DLLEXPORT bool isOutput(SerializerBase *ser);

template <class S, class TT>
void gtranslate(S *ser, 
      TT &it, 
      const char * (*to_str_func)(TT), 
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
      it = (TT) enum_int;
   }
}

class SerializerError;

template <class S, class TT>
bool gtranslate_w_err(S *ser, TT&it, const char *tag = NULL, const char *tag2 = NULL)
{

   try 
   {
      gtranslate(ser, it, tag, tag2);
   }

   catch (const SerializerError &err_) 
   {
      fprintf(stderr, "%s[%d]:  gtranslate failed\n", FILE__, __LINE__);
      printSerErr(err_);
      return false;
   }
   return true;
}

#endif
