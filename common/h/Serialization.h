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

#endif

