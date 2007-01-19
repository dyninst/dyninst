/* 
 * Copyright (c) 1996-2006 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 *
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 *
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

 #include "symtabAPI/h/Dyn_Archive.h"

 extern char errorLine[80];
 static SymtabError serr;
 static string errMsg;

 vector<Dyn_Archive *> Dyn_Archive::allArchives;

 SymtabError Dyn_Archive::getLastError()
 {
 	return serr;
 }

  string Dyn_Archive::printError(SymtabError serr)
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
		       
 bool Dyn_Archive::openArchive(string &filename, Dyn_Archive *&img)
 {
 	bool err;
	for(unsigned i=0;i<allArchives.size();i++)
	{
	    if(allArchives[i]->file() == filename)
	    {
	    	img = allArchives[i];
		return true;
	    }
	}
	img = new Dyn_Archive(filename ,err);
	if(err)	// No errors
		allArchives.push_back(img);	
	else
		img = NULL;
	return err;
 }

 bool Dyn_Archive::openArchive(char *mem_image, size_t size, Dyn_Archive *&img)
 {
 	bool err;
	img = new Dyn_Archive(mem_image, size, err);
	if(err == false)
		img = NULL;
	return err;
 }

 Dyn_Archive::Dyn_Archive(string &filename, bool &err) : filename_(filename)
 {
 	fileOpener *fo_ = fileOpener::openFile(filename);
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
        	sprintf(errorLine, "Not an Archive. Call Dyn_Symtab::openFile"); 
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
	Archive *archive = NULL;
    
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
    		archive = (Archive *) new Archive_32(fo_);
    	else if (!strncmp(magicNumber, AIAMAGBIG, SAIAMAG))
        	archive = (Archive *) new Archive_64(fo_);
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
				             filename_.c_str(), "Reading file header" );
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
					     
		string member_name = archive->member_name;
		bool err;
		string::size_type len = member_name.length();
		if((len >= 4)&&(member_name.substr(len-4,4) == ".imp" || member_name.substr(len-4,4) == ".exp"))
			continue;
		Dyn_Symtab *memImg = new Dyn_Symtab(filename ,member_name, archive->aout_offset, err);
		membersByName[member_name] = memImg;
		membersByOffset[archive->aout_offset] = memImg;
    	} 	
    	delete archive;
	err = true;
 }

 Dyn_Archive::Dyn_Archive(char *mem_image, size_t size, bool &err)
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
        	sprintf(errorLine, "Not an Archive. Call Dyn_Symtab::openFile"); 
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
	Archive *archive = NULL;
    
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
    		archive = (Archive *) new Archive_32(fo_);
    	else if (!strncmp(magicNumber, AIAMAGBIG, SAIAMAG))
        	archive = (Archive *) new Archive_64(fo_);
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
		string member_name = archive->member_name;
		bool err;
		string::size_type len = member_name.length();
		if((len >= 4)&&(member_name.substr(len-4,4) == ".imp" || member_name.substr(len-4,4) == ".exp"))
			continue;
		Dyn_Symtab *memImg = new Dyn_Symtab(mem_image, size,member_name, archive->aout_offset, err);
		membersByName[member_name] = memImg;
		membersByOffset[archive->aout_offset] = memImg;
    	} 	
    	delete archive;
	err = true;
 }

 bool Dyn_Archive::getMember(string &member_name,Dyn_Symtab *&img)
 {
 	if(membersByName.find(member_name) == membersByName.end())
	{
		serr = No_Such_Member;
		errMsg = "Member Does not exist";
		return false;
	}	
	img = membersByName[member_name];
	return true;
 }

 bool Dyn_Archive::getMemberByOffset(OFFSET &memberOffset, Dyn_Symtab *&img)
 {
 	if(membersByOffset.find(memberOffset) == membersByOffset.end())
	{
		serr = No_Such_Member;
		errMsg = "Member Does not exist";
		return false;
	}	
	img = membersByOffset[memberOffset];
	return true;
 }

 bool Dyn_Archive::getAllMembers(vector <Dyn_Symtab *> &members)
 {
 	hash_map <string, Dyn_Symtab *>::iterator iter = membersByName.begin();
	for(; iter!=membersByName.end();iter++)
		members.push_back(iter->second);
	return true;	
 }

 bool Dyn_Archive::isMemberInArchive(string &member_name)
 {
 	hash_map <string, Dyn_Symtab *>::iterator iter = membersByName.begin();
	for(; iter!=membersByName.end();iter++)
	{
		if(iter->first == member_name)
			return true;
	}	
	return false;
 }

 Dyn_Archive::~Dyn_Archive()
 {
 	hash_map <string, Dyn_Symtab *>::iterator iter = membersByName.begin();
	for(; iter!=membersByName.end();iter++)
	    delete (iter->second);
	for (unsigned i = 0; i < allArchives.size(); i++) {
            if (allArchives[i] == this)
                    allArchives.erase(allArchives.begin()+i);
        }
 }
