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

#include "common/src/serialize.h"

#if !defined(SERIALIZATION_DISABLED)

#if !defined(os_windows)
#include "common/src/pathName.h"
#include <dlfcn.h>
#if defined (cap_have_libxml)
//libxml2 functions
void *hXML;
#endif
#else
#include "windows.h"
#if defined (cap_have_libxml)
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
HINSTANCE hXML;
#endif
#endif

#if defined (cap_have_libxml)
#if defined (os_linux)
//  gcc is complaining about the constness of the library-provided
//  typecast (BAD_CAST) (which is defined in xmlstring.h), so we make our own
#define XMLCHAR_CAST (const xmlChar *)
#else
#define XMLCHAR_CAST BAD_CAST
#endif

#else
#define XMLCHAR_CAST (const char *)
#endif

#if 0
SerDesXML &SerializerXML::getSD_xml()
{
   SerDes &sd = getSD();
   SerDesXML *sdxml = dynamic_cast<SerDesXML *> (&sd);
   assert(sdxml);
   return *sdxml;
}
#endif


#if 0
bool end_xml_element(SerializerBase *, const char *);
#endif



// DLLEXPORT SerDesXML(xmlTextWriterPtr w, iomode_t mode)  : SerDes(mode), writer(w) { }
// DLLEXPORT static xmlTextWriterPtr init(std::string fname, iomode_t mode, bool verbose);

#if defined (cap_have_libxml)
xmlTextWriterPtr(*my_xmlNewTextWriterFilename)(const char *,int) = NULL;

int (*my_xmlTextWriterStartDocument)(xmlTextWriterPtr, 
      const char *, const char *, const char * ) = NULL;

int (*my_xmlTextWriterStartElement)(xmlTextWriterPtr, 
      const xmlChar *) = NULL;

int (*my_xmlTextWriterWriteFormatElement)(xmlTextWriterPtr,
      const xmlChar *,const char *,...) = NULL;

int (*my_xmlTextWriterEndDocument)(xmlTextWriterPtr) = NULL;

void (*my_xmlFreeTextWriter)(xmlTextWriterPtr) = NULL;

int (*my_xmlTextWriterWriteFormatAttribute)(xmlTextWriterPtr, 
      const xmlChar *,const char *,...) = NULL;

int (*my_xmlTextWriterEndElement)(xmlTextWriterPtr) = NULL;

xmlTextWriterPtr SerDesXML::init(std::string fname, iomode_t /*mode*/, bool /*verbose*/) 
{
#if defined(_MSC_VER)
   hXML = LoadLibrary(LPCSTR("../../../i386-unknown-nt4.0/lib/libxml2.dll"));
   if (hXML == NULL)
   {
      char buf[1000];
      DWORD result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf, 1000, NULL);
      SER_ERR(buf);
   }

    my_xmlNewTextWriterFilename = 
       (xmlTextWriterPtr (*)(const char *,int)) GetProcAddress(hXML,"xmlNewTextWriterFilename");
    
    my_xmlTextWriterStartDocument = 
       (int (*)(xmlTextWriterPtr, const char *, const char *, const char * ))GetProcAddress(hXML,"xmlTextWriterStartDocument");
    
    my_xmlTextWriterStartElement = 
       (int (*)(xmlTextWriterPtr, const xmlChar *))GetProcAddress(hXML,"xmlTextWriterStartElement");

    my_xmlTextWriterWriteFormatElement = 
       (int (*)(xmlTextWriterPtr,const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatElement");

    my_xmlTextWriterEndDocument = 
       (int (*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndDocument");

    my_xmlFreeTextWriter = 
       (void (*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlFreeTextWriter");

    my_xmlTextWriterWriteFormatAttribute = 
       (int (*)(xmlTextWriterPtr, const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatAttribute");
    
    my_xmlTextWriterEndElement = 
       (int (*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndElement");

#else

    hXML = dlopen("libxml2.so", RTLD_LAZY);

    if (hXML == NULL)
    {
      SER_ERR("Unable to find libxml2");
      return NULL;
    }

    my_xmlNewTextWriterFilename = (xmlTextWriterPtr(*)(const char *,int))dlsym(hXML,"xmlNewTextWriterFilename");
    my_xmlTextWriterStartDocument = (int(*)(xmlTextWriterPtr, const char *, const char *, const char * ))dlsym(hXML,"xmlTextWriterStartDocument");
    my_xmlTextWriterStartElement = (int(*)(xmlTextWriterPtr, const xmlChar *))dlsym(hXML,"xmlTextWriterStartElement");
    my_xmlTextWriterWriteFormatElement = (int(*)(xmlTextWriterPtr,const xmlChar *,const char *,...))dlsym(hXML,"xmlTextWriterWriteFormatElement");
    my_xmlTextWriterEndDocument = (int(*)(xmlTextWriterPtr))dlsym(hXML,"xmlTextWriterEndDocument");
    my_xmlFreeTextWriter = (void(*)(xmlTextWriterPtr))dlsym(hXML,"xmlFreeTextWriter");
    my_xmlTextWriterWriteFormatAttribute = (int(*)(xmlTextWriterPtr, const xmlChar *,const char *,...))dlsym(hXML,"xmlTextWriterWriteFormatAttribute");
    my_xmlTextWriterEndElement = (int(*)(xmlTextWriterPtr))dlsym(hXML,"xmlTextWriterEndElement");

#endif

    /* Create a new XmlWriter for DOM */

    xmlTextWriterPtr writer = my_xmlNewTextWriterFilename(fname.c_str(), 0);

    if (writer == NULL) 
    {
        SER_ERR("testXmlwriterDoc: Error creating the xml writer");
      return NULL;
    }

    int rc = my_xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);

    if (rc < 0) 
    {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartDocument");
    return NULL;
    }

    return writer;
}

#endif // defined (cap_have_libxml)


namespace Dyninst {
#if defined (cap_have_libxml)
//int (*my_xmlTextWriterStartElement)(xmlTextWriterPtr, 
//      const xmlChar *) = NULL;
bool start_xml_elem(void *writer, const char *tag)
{
    int rc = my_xmlTextWriterStartElement((xmlTextWriterPtr) writer, XMLCHAR_CAST tag);
    return (rc >= 0);
}
//    my_xmlTextWriterEndElement(writer);

bool end_xml_elem(void *writer)
{
    int rc = my_xmlTextWriterEndElement((xmlTextWriterPtr) writer);
    return (rc >= 0);
}

bool write_xml_elem(void *writer, const char *tag, const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);

   int rc = my_xmlTextWriterWriteFormatElement((xmlTextWriterPtr) writer, XMLCHAR_CAST tag, fmt, ap);

   if (rc < 0) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

   va_end(ap);

   return (rc >= 0);
}
#else

bool start_xml_elem(void * /*writer*/, const char * /*xmlChar*/)
{
   fprintf(stderr, "%s[%d]:  xml output is disabled\n", FILE__, __LINE__);
   return false;
}

bool end_xml_elem(void * /*writer*/)
{
   fprintf(stderr, "%s[%d]:  xml output is disabled\n", FILE__, __LINE__);
   return false;
}

bool write_xml_elem(void * /*writer*/, const char * /*tag*/, const char * /*fmt*/, ...)
{
   fprintf(stderr, "%s[%d]:  xml output is disabled\n", FILE__, __LINE__);
   return false;
}
#endif
}

namespace Dyninst {
bool start_xml_elem(SerDesXML &s, const char *tag)
{
	return start_xml_elem(s.writer, tag);
}
bool end_xml_elem(SerDesXML &s)
{
	return end_xml_elem(s.writer);
}
}

namespace Dyninst {
bool ifxml_start_element(SerializerBase *sb, const char *tag)
{
#if 0
   SerializerXML *sxml = dynamic_cast<SerializerXML *>(sb);
   if (!sxml) 
   {
      return false;
   }

   if (sxml->iomode() == sd_deserialize) 
   {
      fprintf(stderr, "%s[%d]:  ERROR:  request to deserialize xml\n", FILE__, __LINE__);
      return false;
   }
#endif
   if (!sb->isXML())
	   return false;
   if (!sb->isOutput())
   {
      fprintf(stderr, "%s[%d]:  ERROR:  request to deserialize xml\n", FILE__, __LINE__);
      return false;
   }

   SerDes &sd = sb->getSD();
   SerDesXML *sdxml = dynamic_cast<SerDesXML *>(&sd);
   assert(sdxml);
   ::start_xml_elem(sdxml->writer, tag);

#if 0
   sxml->getSD_xml().start_element(tag);
#endif

   return true;
}
}

namespace Dyninst {
COMMON_EXPORT bool ifxml_end_element(SerializerBase *sb, const char * /*tag*/)
{
#if 0
   SerializerXML *sxml = dynamic_cast<SerializerXML *>(sb);

   if (!sxml) 
   {
      return false;
   }

   if (sxml->iomode() == sd_deserialize) 
   {
      fprintf(stderr, "%s[%d]:  ERROR:  request to deserialize xml\n", FILE__, __LINE__);
      return false;
   }
#endif
   if (!sb->isXML())
	   return false;
   if (!sb->isOutput())
   {
      fprintf(stderr, "%s[%d]:  ERROR:  request to deserialize xml\n", FILE__, __LINE__);
      return false;
   }

   SerDes &sd = sb->getSD();
   SerDesXML *sdxml = dynamic_cast<SerDesXML *>(&sd);
   assert(sdxml);
   ::end_xml_elem(sdxml->writer);
   
#if 0
   sxml->getSD_xml().end_element();
#endif

   return true;
}
}

bool SerializerXML::start_xml_element(SerializerBase *sb, const char *tag)
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
#if 0
bool SerializerXML::start_xml_element(SerializerBase *sb, const char *tag)
{
	SerializerXML *sxml = dynamic_cast<SerializerXML *>(sb);

	if (!sxml) 
	{
      fprintf(stderr, "%s[%d]:  FIXME:  called xml function with non xml serializer\n", 
            FILE__, __LINE__);
      return false;
   }

   SerDesXML sdxml = sxml->getSD_xml();
   start_xml_elem(sdxml.writer, tag);

#if 0
   sdxml.start_element(tag); 
#endif

   return true;
}
#endif

SerDesXML &SerializerXML::getSD_xml()
{
	SerializerBase *sb = this;
	SerDes &sd = sb->getSD();
	SerDesXML *sdxml = dynamic_cast<SerDesXML *> (&sd);
	assert(sdxml);
	return *sdxml;
}

bool SerializerXML::end_xml_element(SerializerBase * sb, const char  * /*tag*/)
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

#if 0
   sdxml.end_element(); 
#endif
   return true;
}

SerDesXML::~SerDesXML()
{
#if defined (cap_have_libxml)
    my_xmlFreeTextWriter(writer);
#endif

}

void SerDesXML::vector_start(unsigned long &/*size*/, const char *tag) DECLTHROW(SerializerError)
{
   bool rc = ::start_xml_elem(writer, tag);

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::vector_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::multimap_start(unsigned long &/*size*/, const char *tag) DECLTHROW(SerializerError)
{
   bool rc = ::start_xml_elem(writer, tag);

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
#if 0
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::multimap_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::pair_start(const char *tag) DECLTHROW(SerializerError)
{
   bool rc = ::start_xml_elem(writer, tag);

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
#if 0
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::pair_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}
void SerDesXML::hash_map_start(unsigned long &/*size*/, const char *tag) DECLTHROW(SerializerError)
{
   bool rc = ::start_xml_elem(writer,  tag);

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
#if 0
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::hash_map_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::annotation_start(Dyninst::AnnotationClassID &a_id, void *& parent_id, sparse_or_dense_anno_t &sod, const char * /*id*/, const char * tag) 
{
   bool rc = ::start_xml_elem(writer, tag);

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
   translate(a_id, "annotationID");
   translate((Address &)parent_id, "annotatableID");
   translate((int &) sod, "SparseOrDense");
   //char sodstr[12];
   //sprintf(sodstr, "%s", sod == sparse ? "sparse" : "dense");
   //const char *sodstr = (sod == sparse) ? "sparse" : "dense";
   //translate((const char *&)const_cast<const char *>(sodstr), 12, "SparseOrDense");
}

void SerDesXML::annotation_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

}

void SerDesXML::annotation_container_start(void *& id) 
{
   bool rc = ::start_xml_elem(writer, "AnnotationContainer");

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
   translate((Address &)id, "containerID");
}

void SerDesXML::annotation_container_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

}

void SerDesXML::annotation_container_item_start(void *& id) 
{
   bool rc = ::start_xml_elem(writer, "AnnotationContainerItem");

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
   translate((Address &)id, "containerID");
}

void SerDesXML::annotation_container_item_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

}
void SerDesXML::annotation_list_start(Address &/*id*/, unsigned long &/*nelem*/, const char * tag) 
{
   bool rc = ::start_xml_elem(writer, tag);

   if (!rc)
   {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
}

void SerDesXML::annotation_list_end()
{
   bool rc = ::end_xml_elem(writer);
   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

}
void SerDesXML::translate(bool &param, const char *tag)
{       
   bool rc = write_xml_elem(writer, tag,
         "%s", param ? "true" : "false");

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                "%s", param ? "true" : "false");
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
    
}     

void SerDesXML::translate(char &param, const char *tag)
{       
   bool rc = write_xml_elem(writer, tag,
         "%c", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                "%c", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
    
}     

void SerDesXML::translate(int &param, const char *tag)
{   
   bool rc = write_xml_elem(writer, tag,
         "%d", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
    
#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%d", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(long &param, const char *tag)
{   
   bool rc = write_xml_elem(writer, tag,
         "%l", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
    
#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%l", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(short &param, const char *tag)
{   
   bool rc = write_xml_elem(writer, tag,
         "%h", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
    
#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%h", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(unsigned short &param, const char *tag)
{   
   bool rc = write_xml_elem(writer, tag,
         "%h", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
    
#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%h", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}
void SerDesXML::translate(unsigned int &param, const char *tag)
{   
  translate( param, tag);
}

#if 0
void SerDesXML::translate(OFFSET &param, const char *tag)
{

    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%lx", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}
#endif

void SerDesXML::translate(float &param, const char *tag)
{
   bool rc = write_xml_elem(writer, tag,
         "%e", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%e", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(double &param, const char *tag)
{
   bool rc = write_xml_elem(writer, tag,
         "%g", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%g", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(Address &param, const char *tag)
{
   bool rc = write_xml_elem(writer, tag,
         "%p", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%p", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(void * &param, const char *tag)
{
   bool rc = write_xml_elem(writer, tag,
         "%p", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%p", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}
void SerDesXML::translate(const char * &param, int /*bufsize*/, const char *tag)
{
   bool rc = write_xml_elem(writer, tag,
         "%s", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(char * &param, int /*bufsize*/, const char *tag)
{
   bool rc = write_xml_elem(writer, tag,
         "%s", param);

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

void SerDesXML::translate(std::string &param, const char *tag)
{
   assert(tag);
   assert(param.c_str());

   bool rc = write_xml_elem(writer, tag,
         "%s", param.c_str());

   if (!rc) 
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", param.c_str());
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

#if 0
void SerDesXML::translate(pdstring &param, const char *tag)
{
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", param.c_str());
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}
#endif

void SerDesXML::translate(std::vector<std::string> &param, const char *tag,
                          const char *elem_tag)
{
   bool rc = ::start_xml_elem(writer, tag);
   if (!rc)
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }

#if 0
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag) ;
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif

    for (unsigned int i = 0; i < param.size(); ++i) 
      translate(param[i], elem_tag);
    

   rc = ::end_xml_elem(writer);
   if (!rc)
   {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterEndElement");
   }

#if 0
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
#endif
}

#if 0
void SerDesXML::start_element(const char *tag)
{
   int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag) ;
   if (rc < 0) {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
}

void SerDesXML::end_element()
{
   int rc = my_xmlTextWriterEndElement(writer);
   if (rc < 0) {
      SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
   }
}

void SerDesXML::xml_value(const char *s, const char *tag)
{
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", s);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}
#endif

#endif
