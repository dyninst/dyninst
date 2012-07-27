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


#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#include "symtabAPI/src/Object.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

extern char errorLine[];

Archive::Archive(std::string &filename, bool &ok) :
   basePtr(NULL), symbolTableParsed(false)
{
   mf  = MappedFile::createMappedFile(filename);
   if( mf == NULL ) {
       serr = Not_A_File;
       errMsg = "Failed to locate file";
       ok = false;
       return;
   }
   fileOpener *fo_ = fileOpener::openFile(mf->base_addr(), mf->size());
   fo_->set_file(mf->filename());

   assert(fo_);
    unsigned char magic_number[2];
    
   if (!fo_->set(0)) 
	{
      sprintf(errorLine, "Error reading file %s\n", 
              filename.c_str());
		serr = Obj_Parsing;
		errMsg = errorLine;
		ok = false;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
      //MappedFile::closeMappedFile(mf);
      return;
   }
   if (!fo_->read((void *)magic_number, 2)) 
	{
      sprintf(errorLine, "Error reading file %s\n", 
              filename.c_str());
		serr = Obj_Parsing;
		errMsg = errorLine;
		ok = false;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
      //MappedFile::closeMappedFile(mf);
      return;
   }

   // a.out file: magic number = 0x01df
   // archive file: magic number = 0x3c62 "<b", actually "<bigaf>"
   // or magic number = "<a", actually "<aiaff>"
   if (magic_number[0] == 0x01)
	{
		serr = Not_An_Archive;
      sprintf(errorLine, "Not an Archive. Call Symtab::openFile"); 
		errMsg = errorLine;
		ok = false;
      //fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
      //MappedFile::closeMappedFile(mf);
		return;
	}
   else if ( magic_number[0] != '<')
	{
      sprintf(errorLine, "Bad magic number in file %s\n",
              filename.c_str());
		serr = Obj_Parsing;
		errMsg = errorLine;
		ok = false;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
     // MappedFile::closeMappedFile(mf);
		return;
   }
	xcoffArchive *archive = NULL;
    
   // Determine archive type
   // Start at the beginning...
   if (!fo_->set(0))
	{
      sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Seeking to file start" );	
		serr = Obj_Parsing;
		errMsg = errorLine;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
		ok = false;
	}					     

   char magicNumber[SAIAMAG];
   if (!fo_->read(magicNumber, SAIAMAG))
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Reading magic number" );
		serr = Obj_Parsing;
		errMsg = errorLine;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
		ok = false;
	}	

   if (!strncmp(magicNumber, AIAMAG, SAIAMAG))
      archive = (xcoffArchive *) new xcoffArchive_32(fo_);
   else if (!strncmp(magicNumber, AIAMAGBIG, SAIAMAG))
      archive = (xcoffArchive *) new xcoffArchive_64(fo_);
   else
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Unknown Magic number" );
		serr = Obj_Parsing;
		errMsg = errorLine;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
		ok = false;
	}	
    
   if (archive->read_arhdr())
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Reading file header" );
		serr = Obj_Parsing;
		errMsg = errorLine;
      fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
		ok = false;
	}	
    
   while (archive->next_offset !=0)
   {
      if (archive->read_mbrhdr())
      {
         sprintf(errorLine, "Error parsing a.out file %s: %s \n",
               filename.c_str(), "Reading Member Header" );
         serr = Obj_Parsing;
         errMsg = errorLine;
         fprintf(stderr, "%s[%d]:  error in Archive ctor\n", FILE__, __LINE__);
         ok = false;
      }	

      std::string member_name = archive->member_name;
      std::string::size_type len = member_name.length();
      if ((len >= 4)&&(member_name.substr(len-4,4) == ".imp" || 
               member_name.substr(len-4,4) == ".exp"))
         continue;

      // Member is parsed lazily
      ArchiveMember *newMember = new ArchiveMember(member_name, archive->aout_offset);
      membersByOffset[archive->aout_offset] = newMember;
      membersByName[member_name] = newMember;
   } 	

   //  why do we close the file mapping??  I must be missing something
#if 0 
   if (mf)
      MappedFile::closeMappedFile(mf);
   delete archive;
   mf = NULL;
#endif

   ok = true;
}

Archive::Archive(char *mem_image, size_t size, bool &err)
    : basePtr(NULL), symbolTableParsed(false)
{
    mf  = MappedFile::createMappedFile(mem_image, size);
    fileOpener *fo_ = fileOpener::openFile(mf->base_addr(), mf->size());

    assert(fo_);
    unsigned char magic_number[2];

    if (!fo_->set(0)) 
    {
        sprintf(errorLine, "Error reading memory image %p with size %u\n",
                mem_image, (unsigned int)size);
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
        return;
    }
    if (!fo_->read((void *)magic_number, 2)) 
    {
        sprintf(errorLine, "Error reading memory image %p with size %u\n",
                mem_image, (unsigned int)size);
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
        return;
    }

    // a.out file: magic number = 0x01df
    // archive file: magic number = 0x3c62 "<b", actually "<bigaf>"
    // or magic number = "<a", actually "<aiaff>"
    if (magic_number[0] == 0x01)
    {
        serr = Not_An_Archive;
        sprintf(errorLine, "Not an Archive. Call Symtab::openFile"); 
        errMsg = errorLine;
        err = false;
        return;
    }
    else if ( magic_number[0] != '<')
    {
        sprintf(errorLine, "Error reading memory image %p with size %u\n",
                mem_image, (unsigned int)size);
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
        return;
    }
    xcoffArchive *archive = NULL;

    // Determine archive type
    // Start at the beginning...
    if (!fo_->set(0))
    {
        sprintf(errorLine, "Error parsing memory image %p with size %u : %s\n",
                mem_image, (unsigned int)size, "Seeking to file start");
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
    }

    char magicNumber[SAIAMAG];
    if (!fo_->read(magicNumber, SAIAMAG))
    {
        sprintf(errorLine, "Error parsing memory image %p with size %u : %s\n",
                mem_image, (unsigned int)size, "Reading magic number");
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
    }	

    if (!strncmp(magicNumber, AIAMAG, SAIAMAG))
        archive = (xcoffArchive *) new xcoffArchive_32(fo_);
    else if (!strncmp(magicNumber, AIAMAGBIG, SAIAMAG))
        archive = (xcoffArchive *) new xcoffArchive_64(fo_);
    else
    {
        sprintf(errorLine, "Error parsing memory image %p with size %u : %s\n",
                mem_image, (unsigned int)size, "Unknown magic number");
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
    }    
    if (archive->read_arhdr())
    {
        sprintf(errorLine, "Error parsing memory image %p with size %u : %s\n",
                mem_image, (unsigned int)size, "Reading file header");
        serr = Obj_Parsing;
        errMsg = errorLine;
        err = false;
    }

    while (archive->next_offset !=0)
    {
        if (archive->read_mbrhdr())
        {
            sprintf(errorLine, "Error parsing memory image %p with size %u : %s\n",
                    mem_image, (unsigned int)size, "Reading memory header");
            serr = Obj_Parsing;
            errMsg = errorLine;
            err = false;
        }	
        std::string member_name = archive->member_name;
        std::string::size_type len = member_name.length();
        if((len >= 4)&&(member_name.substr(len-4,4) == ".imp" || member_name.substr(len-4,4) == ".exp"))
            continue;

        ArchiveMember *newMember = new ArchiveMember(member_name, archive->aout_offset);
        membersByOffset[archive->aout_offset] = newMember;
        membersByName[member_name] = newMember;
    }

    fprintf(stderr, "%s[%d]:  deleting archive\n", FILE__, __LINE__);
    delete archive;
    err = true;
}

bool Archive::parseMember(Symtab *&img, ArchiveMember *member) 
{
    bool err;
    if (mf->getFD() == -1) {
        img = new Symtab((char *)mf->base_addr(),mf->size(),
                         member->getName(), member->getOffset(), err, basePtr);
    } else {
        if (!mf) {
            fprintf(stderr, "%s[%d]:  ERROR: mf has not been set\n", FILE__, __LINE__);
            return false;
        }

        std::string pname = mf->pathname();
        if (pname.length()) {
            img = new Symtab(std::string(pname), member->getName(), member->getOffset(),
                  err, basePtr);
        } else {
            fprintf(stderr, "%s[%d]:  WARNING:  got mapped file with non existant path name\n", FILE__, __LINE__);
            img = new Symtab(std::string(""), member->getName(), member->getOffset(),
                  err, basePtr);
        }
    }

    if( !err ) {
        img->parentArchive_ = this;
        member->setSymtab(img);
    }

    return !err;
}

bool Archive::parseSymbolTable() 
{
    //TODO
    serr = Obj_Parsing;
    errMsg = "parsing of the global symbol table in a XCOFF archive is currently unimplemented";
    return false;
}
