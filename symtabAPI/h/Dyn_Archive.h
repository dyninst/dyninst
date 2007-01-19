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

 #ifndef Dyn_Archive_h
 #define Dyn_Archive_h
 
 #include <sys/types.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <assert.h>
 #include <vector>
 #include <string>
 #include <ext/hash_map>
 
 
 #include "symtabAPI/h/Dyn_Symtab.h"
 using namespace std;
 using namespace __gnu_cxx; 
 
 class Dyn_Archive{
  public:
 	Dyn_Archive() {}
	static bool openArchive(string &filename, Dyn_Archive *&img);
	static bool openArchive(char *mem_image, size_t image_size, Dyn_Archive *&img);

	bool getMember(string &member_name,Dyn_Symtab *&img);
	bool getMemberByOffset(OFFSET &memberOffset, Dyn_Symtab *&img);
	bool getAllMembers(vector <Dyn_Symtab *> &members);
	bool isMemberInArchive(string &member_name);

	SymtabError getLastError();
	static string printError(SymtabError serr);
	static string file() { return filename_; }
	~Dyn_Archive();

 private:
 	Dyn_Archive(string &filename, bool &err);
	Dyn_Archive(char *mem_image, size_t image_size, bool &err);
 
 private:
 	string filename_;
	char *mem_image_;
	hash_map <string, Dyn_Symtab *> membersByName;
	hash_map <OFFSET, Dyn_Symtab *> membersByOffset;

	// A vector of all Dyn_Archive. Used to avoid duplicating
        // a Dyn_Archive that already exists.
        static vector<Dyn_Archive *> allArchives;
 };

 #endif
