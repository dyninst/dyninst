
/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "serialize.h"

#if !defined(os_windows)
#include "common/h/pathName.h"
#include <dlfcn.h>
//libxml2 functions
void *hXML;
#else
#include "windows.h"
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
HINSTANCE hXML;
#endif

#if defined (os_linux) | defined (os_aix)
//  gcc is complaining about the constness of the library-provided
//  typecast (BAD_CAST) (which is defined in xmlstring.h), so we make our own
#define XMLCHAR_CAST (const xmlChar *)
#else
#define XMLCHAR_CAST BAD_CAST
#endif

xmlTextWriterPtr(*my_xmlNewTextWriterFilename)(const char *,int) = NULL;
int(*my_xmlTextWriterStartDocument)(xmlTextWriterPtr, const char *, const char *, const char * ) = NULL;
int(*my_xmlTextWriterStartElement)(xmlTextWriterPtr, const xmlChar *) = NULL;
int(*my_xmlTextWriterWriteFormatElement)(xmlTextWriterPtr,const xmlChar *,const char *,...) = NULL;
int(*my_xmlTextWriterEndDocument)(xmlTextWriterPtr) = NULL;
void(*my_xmlFreeTextWriter)(xmlTextWriterPtr) = NULL;
int(*my_xmlTextWriterWriteFormatAttribute)(xmlTextWriterPtr, const xmlChar *,const char *,...) = NULL;
int(*my_xmlTextWriterEndElement)(xmlTextWriterPtr) = NULL;

SerDesXML::SerDesXML(std::string fname, iomode_t mode, bool verbose) :
  SerDes(fname, mode, verbose)
{
#if defined(_MSC_VER)
   hXML = LoadLibrary(LPCSTR("../../../i386-unknown-nt4.0/lib/libxml2.dll"));
   if(hXML == NULL){
      char buf[1000];
      bool result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf, 1000, NULL);
      SER_ERR(std::string(buf));
    }
    my_xmlNewTextWriterFilename = (xmlTextWriterPtr(*)(const char *,int))GetProcAddress(hXML,"xmlNewTextWriterFilename");
    my_xmlTextWriterStartDocument = (int(*)(xmlTextWriterPtr, const char *, const char *, const char * ))GetProcAddress(hXML,"xmlTextWriterStartDocument");
    my_xmlTextWriterStartElement = (int(*)(xmlTextWriterPtr, const xmlChar *))GetProcAddress(hXML,"xmlTextWriterStartElement");
    my_xmlTextWriterWriteFormatElement = (int(*)(xmlTextWriterPtr,const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatElement");
    my_xmlTextWriterEndDocument = (int(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndDocument");
    my_xmlFreeTextWriter = (void(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlFreeTextWriter");
    my_xmlTextWriterWriteFormatAttribute = (int(*)(xmlTextWriterPtr, const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatAttribute");
    my_xmlTextWriterEndElement = (int(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndElement");
#else
    hXML = dlopen("libxml2.so", RTLD_LAZY);
    if(hXML == NULL){
      SER_ERR("Unable to find libxml2");
      return;
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
    writer = my_xmlNewTextWriterFilename(filename.c_str(), 0);
    if (writer == NULL) {
        SER_ERR("testXmlwriterDoc: Error creating the xml writer");
      return;
    }
    int rc = my_xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartDocument");
    return;
    }

}

SerDesXML::~SerDesXML()
{
    my_xmlFreeTextWriter(writer);

#if defined(_MSC_VER)
    FreeLibrary(hXML);
#else
    //  should we dlclose??
#endif

}

void SerDesXML::vector_start(unsigned int &/*size*/, const char *tag) throw(SerializerError)
{
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::vector_end()
{
    int rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::multimap_start(unsigned int &/*size*/, const char *tag) throw(SerializerError)
{
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::multimap_end()
{
    int rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::annotation_start(const char * /*id*/, const char * /*tag*/) 
{
}

void SerDesXML::annotation_end()
{
}

void SerDesXML::translate(bool &param, const char *tag)
{       
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                "%s", param ? "true" : "false");
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
    
}     

void SerDesXML::translate(char &param, const char *tag)
{       
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                "%c", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
    
}     

void SerDesXML::translate(int &param, const char *tag)
{   
    
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%d", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::translate(unsigned int &param, const char *tag)
{   
  translate( (int) param, tag);
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

void SerDesXML::translate(Address &param, const char *tag)
{

    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%p", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::translate(const char * &param, int /*bufsize*/, const char *tag)
{
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", param);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

void SerDesXML::translate(string &param, const char *tag)
{
    assert(tag);
    assert(param.c_str());
    int rc = my_xmlTextWriterWriteFormatElement(writer, XMLCHAR_CAST tag,
                                                 "%s", param.c_str());
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
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
    int rc = my_xmlTextWriterStartElement(writer, XMLCHAR_CAST tag) ;
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }

    for (unsigned int i = 0; i < param.size(); ++i) 
      translate(param[i], elem_tag);
    

    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
        SER_ERR("testXmlwriterDoc: Error at my_xmlTextWriterStartElement");
    }
}

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
