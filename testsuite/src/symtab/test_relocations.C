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

#include "symtab_comp.h"
#include "test_lib.h"

#include "Symtab.h"
#include "Symbol.h"
#include "Function.h"
#include "Variable.h"

#include <iostream>
#include <errno.h>
#if !defined(os_windows_test)
#include <dirent.h>
#endif

using namespace Dyninst;
using namespace SymtabAPI;

bool resolve_libc_name(char *buf)
{
#if defined(os_windows_test)
	return false;
#else
	return false;
#endif
}

class test_relocations_Mutator : public SymtabMutator {
	std::vector<relocationEntry> relocs;
	char libc_name[1024];
	Symtab *libc;
	std::vector<std::string> expected_libc_relocations;

	bool open_libc()
	{
#if defined(os_windows_test)
           return false;
#else

           DIR *dirp;
           struct dirent *dp;
           
           std::vector<std::string> libc_dirs;
           libc_dirs.push_back("/lib");
           libc_dirs.push_back("/lib64");
           // 32-bit ubuntu
           libc_dirs.push_back("/lib/i386-linux-gnu");
           // 64-bit ubuntu
           libc_dirs.push_back("/lib/x86_64-linux-gnu");
           // PowerpcLE 64-bit ubuntu
           libc_dirs.push_back("/lib/powerpc64le-linux-gnu");
           
           for (unsigned i = 0; i < libc_dirs.size(); ++i) {
              if (NULL == (dirp = opendir(libc_dirs[i].c_str()))) {
                 continue;
              }
              
              do {
                 errno = 0;
                 if ((dp = readdir(dirp)) != NULL) {
                    int nelem = strlen("libc.so");
                    if ( 0 != strncmp(dp->d_name, "libc.so", nelem))
                       continue;
                    dprintf("found %s\n", dp->d_name);
                    snprintf(libc_name, 1024, "%s/%s", libc_dirs[i].c_str(), dp->d_name);
                    
                    // Try this one
                    if (Symtab::openFile(libc, libc_name)) {
                       closedir(dirp);
                       return true;
                    }
                 }
              } while (dp != NULL);
              closedir(dirp);
           }
           return false;
#endif
	}

	public:
	test_relocations_Mutator() 
	{ 
		expected_libc_relocations.push_back(std::string("printf"));
		expected_libc_relocations.push_back(std::string("fprintf"));
		expected_libc_relocations.push_back(std::string("sprintf"));
		expected_libc_relocations.push_back(std::string("snprintf"));
		expected_libc_relocations.push_back(std::string("memcpy"));
		expected_libc_relocations.push_back(std::string("strcmp"));
		expected_libc_relocations.push_back(std::string("memset"));
		expected_libc_relocations.push_back(std::string("fopen"));
		expected_libc_relocations.push_back(std::string("fwrite"));
		expected_libc_relocations.push_back(std::string("fread"));
		expected_libc_relocations.push_back(std::string("fclose"));
		expected_libc_relocations.push_back(std::string("__xstat"));
		expected_libc_relocations.push_back(std::string("__lxstat"));
		expected_libc_relocations.push_back(std::string("__fxstat"));
	};
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_relocations_factory()
{
	return new test_relocations_Mutator();
}

test_results_t test_relocations_Mutator::executeTest()
{
	if (createmode == DESERIALIZE)
		return SKIPPED;
#if defined (os_windows_test)
	return SKIPPED;
#endif

#if defined (os_aix_test)
	return SKIPPED;
#endif

	bool result = symtab->getFuncBindingTable(relocs);

	if (!result || !relocs.size() )
	{
		logerror("%s[%d]: - Unable to find relocations\n", 
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


	bool found_one = false;

	for (unsigned int i = 0; i < expected_libc_relocations.size(); ++i)
	{
		int relocation_index ;
		bool found = false;
		for (unsigned int j = 0; j < relocs.size(); ++j)
		{
			const std::string &relname = relocs[j].name();
			if (relname == expected_libc_relocations[i])
			{
				found = true;
				relocation_index = i;
				break;
			}
		}
		if (!found)
		{
#if 0
			fprintf(stderr, "%s[%d]:  could not find relocation for %s\n", 
					FILE__, __LINE__, expected_libc_relocations[i].c_str());
#endif
		}
		else 
		{
			std::vector<Function *> libc_matches;
			if (!libc->findFunctionsByName(libc_matches, expected_libc_relocations[i]) 
					|| !libc_matches.size())
			{
#if 0
				fprintf(stderr, "%s[%d]:  failed to find %s in libc\n", FILE__, __LINE__, 
						expected_libc_relocations[i].c_str());
				err = true;
#endif
			}
			else
			{
				//fprintf(stderr, "%s[%d]:  found %d matches for %s in libc\n", 
			//			FILE__, __LINE__, libc_matches.size(), expected_libc_relocations[i].c_str());

				found_one = true;
#if 0
				const Dyninst::SymtabAPI::Function *f = libc_matches[0];

				if (!f)
				{
					fprintf(stderr, "%s[%d]:  BAD NEWS:  symtabAPI returned NULL func\n", 
							FILE__, __LINE__);
					err = true;
				}
				else 
				{
					//  maybe eventually we can check that the relocation address
					//  properly resolve to the library...  for now existence 
					//  will have to suffice.
					Offset off = f->getOffset();
					//fprintf(stderr, "\toffset = %p, rel_target_addr = %p, rel_addr = %p\n", 
					// off, relocs[relocation_index].target_addr(), relocs[relocation_index].rel_addr());
				}
#endif
			}
		}
	}

	if (!found_one) 
	{
#if 0
		fprintf(stderr, "%s[%d]:  have relocations:\n", FILE__, __LINE__);
		for (unsigned int i = 0; i < relocs.size(); ++i)
		{
			std::cerr << "        " << relocs[i] << std::endl;

		}
#endif
		return FAILED;
	}

	//  OK, enough with libc...  now let's look at the (more controllable) relocations
	//  from libtestA...

	std::vector<std::string> expected_relocs;
	expected_relocs.push_back(std::string("relocation_test_function1"));
	expected_relocs.push_back(std::string("relocation_test_function2"));
	//expected_relocs.push_back(std::string("relocation_test_variable1"));
	//expected_relocs.push_back(std::string("relocation_test_variable2"));
	int num_found = 0;
	for (unsigned int i = 0; i < expected_relocs.size(); ++i)
	{
		bool foundit =  false;
		for (unsigned int j = 0; j < relocs.size(); ++j)
		{
			if (relocs[j].name() == expected_relocs[i])
			{
				foundit = true;
				num_found++;
				break;
			}
		}
		if (!foundit)
			fprintf(stderr, "%s[%d]:  failed to find relocation for %s\n", 
					FILE__, __LINE__, expected_relocs[i].c_str());
	}

	if (num_found != expected_relocs.size())
	{
		fprintf(stderr, "%s[%d]:  found %d relocs, not the expected %ld\n", 
				FILE__, __LINE__, num_found, expected_relocs.size());
		return FAILED;
	}
	return PASSED;
}

