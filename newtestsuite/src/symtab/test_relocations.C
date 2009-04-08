/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#include "symtab_comp.h"
#include "test_lib.h"

#include "Symtab.h"
#include "Symbol.h"
#include "Function.h"
#include "Variable.h"

#include <iostream>
#include <dirent.h>
#include <errno.h>

using namespace Dyninst;
using namespace SymtabAPI;

bool resolve_libc_name(char *buf)
{
	DIR *dirp;
	struct dirent *dp;

	if (NULL == (dirp = opendir("/lib"))) 
	{
		fprintf(stderr, "%s[%d]: couldn’t open /lib: %s", FILE__, __LINE__, strerror(errno));
		return false;
	}

	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			int nelem = strlen("libc.so");
			if ( 0 != strncmp(dp->d_name, "libc.so", nelem))
				continue;

			fprintf(stderr, "found %s\n", dp->d_name);
			sprintf(buf, "/lib/%s", dp->d_name);
			closedir(dirp);
			return true;

		}
	} while (dp != NULL);

	return false;
}

class test_relocations_Mutator : public SymtabMutator {
	std::vector<relocationEntry> relocs;
	char libc_name[128];
	Symtab *libc;

	bool open_libc()
	{
		if (!resolve_libc_name(libc_name))
		{
			fprintf(stderr, "%s[%d]:  cannot find libc....\n", FILE__, __LINE__);
			return false;
		}

		if (!Symtab::openFile(libc, libc_name))
		{
			fprintf(stderr, "%s[%d]:  cannot create libc....\n", FILE__, __LINE__);
			return false;
		}

		return true;
	}

	public:
	test_relocations_Mutator() { };
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_relocations_factory()
{
	return new test_relocations_Mutator();
}

test_results_t test_relocations_Mutator::executeTest()
{
	bool result = symtab->getFuncBindingTable(relocs);

	if (!result || !relocs.size() )
	{
		logerror("%s[%d]: - Unable to find relocations\n", 
				FILE__, __LINE__);
		fprintf(stderr, "%s[%d]: - Unable to find relocations\n", 
				FILE__, __LINE__);
		return FAILED;
	}

#if 0
	fprintf(stderr, "%s[%d]:  have relocs:\n", FILE__, __LINE__);
	for (unsigned int i = 0; i < relocs.size(); ++i)
	{
	   Symbol *s = relocs[i].getDynSym();
	   std::cerr << "      " <<  relocs[i];
	   if (s)
		  std::cerr << "  symname:  " << s->getName() << "  symaddr: " <<s->getAddr() << " symtype = "<< Symbol::symbolType2Str(s->getType()) << "symlinkage = " <<Symbol::symbolLinkage2Str(s->getLinkage()) << " vis = " <<Symbol::symbolVisibility2Str(s->getVisibility()); 
	   std::cerr << std::endl;
   }
#endif

	if (!open_libc())
	{
		fprintf(stderr, "%s[%d]:  failed to open libc\n", FILE__, __LINE__);
		return FAILED;
	}

	bool err = false;

	for (unsigned int i = 0; i < relocs.size(); ++i)
	{
		const std::string &relname = relocs[i].name();
		std::vector<Dyninst::SymtabAPI::Function *> libc_matches;

		if (!libc->findFunctionsByName(libc_matches, relname) || !libc_matches.size())
		{
			fprintf(stderr, "%s[%d]:  failed to find %s in libc\n", FILE__, __LINE__, 
					relname.c_str());
			err = true;
		}
		else
		{
			fprintf(stderr, "%s[%d]:  found %d matches for %s in libc\n", 
					FILE__, __LINE__, libc_matches.size(), relname.c_str());

			const Dyninst::SymtabAPI::Function *f = libc_matches[0];

			if (!f)
			{
				fprintf(stderr, "%s[%d]:  BAD NEWS:  symtabAPI returned NULL func\n", 
						FILE__, __LINE__);
				err = true;
			}
			else 
			{
				Offset off = f->getOffset();
				fprintf(stderr, "\toffset = %p, rel_target_addr = %p, rel_addr = %p\n", off, relocs[i].target_addr(), relocs[i].rel_addr());
			}
		}
	}

	if (err) return FAILED;
#if 0
   if (relocs.size() != 3)
   {
      logerror("%s[%d]: - wrong number of relocs??: %d\n", 
               FILE__, __LINE__, relocs.size());
      fprintf(stderr, "%s[%d]: - wrong number of relocs??: %d\n", 
               FILE__, __LINE__, relocs.size());
      return FAILED;
   }
#endif

   return PASSED;
}

