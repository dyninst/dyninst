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


#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#include "symtabAPI/src/Object.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

extern char errorLine[80];
static SymtabError serr;
static std::string errMsg;

std::vector<Archive *> Archive::allArchives;

SymtabError Archive::getLastError()
{
    return serr;
}

std::string Archive::printError(SymtabError serr)
{
   switch (serr){
      case Obj_Parsing:
         return "Failed to parse the Archive"+errMsg;
      case No_Such_Member:
	    	return "Member not found" + errMsg;
      case Not_An_Archive:
	    	return "File is not an archive";
      default:
         return "Unknown Error";
	}	
}		
		       
bool Archive::openArchive(Archive *&img, std::string filename)
{
 	bool ok;
	for (unsigned i=0;i<allArchives.size();i++)
	{
      if (allArchives[i]->file() == filename)
      {
	    	img = allArchives[i];
         return true;
      }
	}
	img = new Archive(filename ,ok);
	if (ok)	// No errors
		allArchives.push_back(img);	
	else {
      if (img) {
         //fprintf(stderr, "%s[%d]:  deleting archive here\n", FILE__, __LINE__);
         delete img;
      }
		img = NULL;
   }

	return ok;
}

#if 0 
bool Archive::openArchive(Archive *&img, char *mem_image, size_t size)
{
 	bool err;
	img = new Archive(mem_image, size, err);
	if(err == false)
		img = NULL;
	return err;
}
#endif

Archive::Archive(std::string &filename, bool &err) :
   filename_(filename)
{
   mf  = MappedFile::createMappedFile(filename);
   fileOpener *fo_ = fileOpener::openFile(mf->base_addr(), mf->size());
	unsigned char magic_number[2];
    
   if (!fo_->set(0)) 
	{
      sprintf(errorLine, "Error reading file %s\n", 
              filename.c_str());
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
      return;
   }
   if (!fo_->read((void *)magic_number, 2)) 
	{
      sprintf(errorLine, "Error reading file %s\n", 
              filename.c_str());
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
      sprintf(errorLine, "Bad magic number in file %s\n",
              filename.c_str());
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
      sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Seeking to file start" );	
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
	}					     

   char magicNumber[SAIAMAG];
   if (!fo_->read(magicNumber, SAIAMAG))
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Reading magic number" );
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
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Unknown Magic number" );
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
	}	
    
   if (archive->read_arhdr())
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename.c_str(), "Reading file header" );
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
	}	
    
	while (archive->next_offset !=0)
   {
      if (archive->read_mbrhdr())
		{
			sprintf(errorLine, "Error parsing a.out file %s: %s \n",
                 filename.c_str(), "Reading Member Header" );
			serr = Obj_Parsing;
			errMsg = errorLine;
			err = false;
		}	
					     
		std::string member_name = archive->member_name;
		bool err;
		std::string::size_type len = member_name.length();
		if((len >= 4)&&(member_name.substr(len-4,4) == ".imp" || member_name.substr(len-4,4) == ".exp"))
			continue;
        memberToOffsetMapping[member_name] = archive->aout_offset;
        //Do it lazily
		membersByName[member_name] = NULL;
/*        
		Symtab *memImg = new Symtab(filename ,member_name, archive->aout_offset, err);
		membersByName[member_name] = memImg;
*/
   } 	
   if (mf)
      MappedFile::closeMappedFile(mf);
   delete archive;
   mf = NULL;
	err = true;
}

#if 0 
Archive::Archive(char *mem_image, size_t size, bool &err)
{
 	fileOpener *fo_ = fileOpener::openFile(mem_image, size);
	assert(fo_);
	unsigned char magic_number[2];
    
   if (!fo_->set(0)) 
	{
      sprintf(errorLine, "Error reading file %s\n", 
              filename_.c_str());
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
      return;
   }
   if (!fo_->read((void *)magic_number, 2)) 
	{
      sprintf(errorLine, "Error reading file %s\n", 
              filename_.c_str());
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
      sprintf(errorLine, "Bad magic number in file %s\n",
              filename_.c_str());
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
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename_.c_str(), "Seeking to file start" );	
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
	}
	
   char magicNumber[SAIAMAG];
   if (!fo_->read(magicNumber, SAIAMAG))
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename_.c_str(), "Reading magic number" );
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
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename_.c_str(), "Unknown Magic number" );
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
	}    
   if (archive->read_arhdr())
	{
		sprintf(errorLine, "Error parsing a.out file %s: %s \n",
              filename_.c_str(), "Reading File Header" );
		serr = Obj_Parsing;
		errMsg = errorLine;
		err = false;
   }

	while (archive->next_offset !=0)
   {
      if (archive->read_mbrhdr())
		{
			sprintf(errorLine, "Error parsing a.out file %s: %s \n",
                 filename_.c_str(), "Reading Member Header" );
			serr = Obj_Parsing;
			errMsg = errorLine;
			err = false;
		}	
		std::string member_name = archive->member_name;
		bool err;
		std::string::size_type len = member_name.length();
		if((len >= 4)&&(member_name.substr(len-4,4) == ".imp" || member_name.substr(len-4,4) == ".exp"))
			continue;
		Symtab *memImg = new Symtab(mem_image, size,member_name, archive->aout_offset, err);
		membersByName[member_name] = memImg;
		membersByOffset[archive->aout_offset] = memImg;
   } 	
   fprintf(stderr, "%s[%d]:  deleting archive\n", FILE__, __LINE__);
   delete archive;
	err = true;
}
#endif

bool Archive::getMember(Symtab *&img, std::string member_name)
{
   if (membersByName.find(member_name) == membersByName.end())
   {
      serr = No_Such_Member;
      errMsg = "Member Does not exist";
      return false;
   }	
   img = membersByName[member_name];
   if (img == NULL) {
      bool err;
#if 0
      img = new Symtab(filename_ ,member_name, memberToOffsetMapping[member_name], err);
#endif
      std::string fn = file();
      img = new Symtab(fn,member_name, memberToOffsetMapping[member_name], err);
      membersByName[member_name] = img;
   }
   return true;
}

bool Archive::getAllMembers(std::vector <Symtab *> &members)
{
   hash_map <std::string, Symtab *>::iterator iter = membersByName.begin();
   for (; iter!=membersByName.end();iter++) {
      if (iter->second == NULL) {
         bool err;
         string member_name = iter->first;
#if 0
         iter->second = new Symtab(filename_, member_name, memberToOffsetMapping[iter->first], err);
#endif
         iter->second = new Symtab(file(), member_name, memberToOffsetMapping[iter->first], err);
         membersByName[iter->first] = iter->second;
      }
      members.push_back(iter->second);
   }
   return true;	
}

bool Archive::isMemberInArchive(std::string member_name)
{
   if(membersByName.find(member_name) != membersByName.end())
      return true;
   return false;
}

std::string Archive::file()
{
  return filename_;
}

Archive::~Archive()
{
   hash_map <std::string, Symtab *>::iterator iter = membersByName.begin();
   for (; iter!=membersByName.end();iter++) {
      if (iter->second)
         delete (iter->second);
   }
   memberToOffsetMapping.clear();
   for (unsigned i = 0; i < allArchives.size(); i++) {
      if (allArchives[i] == this)
         allArchives.erase(allArchives.begin()+i);
   }
   //fprintf(stderr, "%s[%d]:  archive DTOR\n", FILE__, __LINE__);
   if (mf)
      MappedFile::closeMappedFile(mf);
}
