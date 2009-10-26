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
#include <stdlib.h>

using namespace Dyninst;
using namespace SymtabAPI;

SymtabComponent::SymtabComponent()
{
}

SymtabComponent::~SymtabComponent()
{
}

test_results_t SymtabComponent::program_setup(ParameterDict &params)
{
   return PASSED;
}

test_results_t SymtabComponent::program_teardown(ParameterDict &params)
{
   return PASSED;
}

test_results_t SymtabComponent::group_setup(RunGroup *group, ParameterDict &params)
{
	//mutatee_p.setString(group->mutatee);
	compiler_p.setString(group->compiler);
#if defined (cap_serialization_test)
	const char *ser_env = getenv(SERIALIZE_CONTROL_ENV_VAR);
	//  allow user to explicitly disable serialization in environment
	//  check this before we modify any environment vars
	if (ser_env && !strcmp(ser_env, SERIALIZE_DISABLE))
	{
		logerror( "%s[%d]:  serialization is disabled\n", FILE__, __LINE__);
	}
	else
	{
		switch (group->useAttach)
		{
			case DESERIALIZE:
				{
					logerror("%s[%d]:  runmode DESERIALIZE\n", FILE__, __LINE__);
					fflush(NULL);
					//  If we have an open symtab with this name, it will just be returned
					//  when we call openFile.  If it is sourced from a regular parse, 
					// this will not trigger deserialization, so
					//  we need to close the existing open symtab.
					Symtab *s_open = Symtab::findOpenSymtab(std::string(group->mutatee));
					if (s_open && !s_open->from_cache())
					{
						logerror( "%s[%d]:  closing open symtab for %s\n", 
								FILE__, __LINE__, group->mutatee);
						Symtab::closeSymtab(s_open);
						s_open = Symtab::findOpenSymtab(std::string(group->mutatee));
						if (s_open)
						{ 
							logerror( "%s[%d]:  failed to close symtab\n", FILE__, __LINE__);
							return FAILED;
						}

					}
					enableSerDes<Symtab>(std::string(group->mutatee), true);
					enforceDeserialize<Symtab>(std::string(group->mutatee), true);

					//  verify that we have an existing cache for this mutatee from which to deserialize
				}
				break;
			case CREATE:
				logerror( "%s[%d]:  runmode CREATE\n", FILE__, __LINE__);
				enableSerialize<Symtab>(std::string(group->mutatee), true);
				enableDeserialize<Symtab>(std::string(group->mutatee), false);
				enforceDeserialize<Symtab>(std::string(group->mutatee), false);

				//  verify that we have an existing cache for this mutatee from which to deserialize
				break;
			default:
				logerror( "%s[%d]:  bad runmode!\n", FILE__, __LINE__);
				return FAILED;
		}
	}
#endif
   symtab = NULL;
   if (group->mutatee && group->state != SELFSTART)
   {
      bool result = Symtab::openFile(symtab, std::string(group->mutatee));
      if (!result || !symtab)
         return FAILED;
      symtab_ptr.setPtr(symtab);
   }
   else
   {
      symtab_ptr.setPtr(NULL);
   }
   params["Symtab"] = &symtab_ptr;
   params["useAttach"]->setInt(group->useAttach);
   //params["mutatee"] = &mutatee_p;
   params["compiler"] = &compiler_p;
   return PASSED;
}

test_results_t SymtabComponent::group_teardown(RunGroup *group, ParameterDict &params)
{
   symtab = NULL;
   return PASSED;
}

test_results_t SymtabComponent::test_setup(TestInfo *test, ParameterDict &params)
{
   return PASSED;
}

test_results_t SymtabComponent::test_teardown(TestInfo *test, ParameterDict &params)
{
   return PASSED;
}

test_results_t SymtabMutator::setup(ParameterDict &param)
{
   symtab = (Symtab *) param["Symtab"]->getPtr();
   useAttach = (int) param["useAttach"]->getInt();
   //mutatee = std::string((const char *)param["mutatee"]->getString());
   compiler = std::string((const char *)param["compiler"]->getString());
   return PASSED;
}

SymtabMutator::SymtabMutator() :
   symtab(NULL)
{
}

SymtabMutator::~SymtabMutator()
{
}

std::string SymtabComponent::getLastErrorMsg()
{
   return std::string("");
}

TEST_DLL_EXPORT ComponentTester *componentTesterFactory()
{
   return new SymtabComponent();
}
