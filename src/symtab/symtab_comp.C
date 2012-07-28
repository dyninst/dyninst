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
	symtab = NULL;
	//mutatee_p.setString(group->mutatee);
	compiler_p.setString(group->compiler);

	if (measure) um_group.start();  // Measure resource usage.

#if defined (cap_serialization_test) && !defined(SERIALIZATION_DISABLED)
	if (group->createmode == DESERIALIZE)
		return SKIPPED;

	const char *ser_env = getenv(SERIALIZE_CONTROL_ENV_VAR);
	//  allow user to explicitly disable serialization in environment
	//  check this before we modify any environment vars
	if (ser_env && !strcmp(ser_env, SERIALIZE_DISABLE))
	{
		logerror( "%s[%d]:  serialization is disabled\n", FILE__, __LINE__);
	}
	else
	{
		Symtab *s_open = Symtab::findOpenSymtab(std::string(group->mutatee));
		switch (group->createmode)
		{
			case DESERIALIZE:
				{
					logerror("%s[%d]:  runmode DESERIALIZE\n", FILE__, __LINE__);
					fflush(NULL);
					//  If we have an open symtab with this name, it will just be returned
					//  when we call openFile.  If it is sourced from a regular parse, 
					// this will not trigger deserialization, so we need to close the
					//  existing open symtab if it was not deserialized.
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
					else
					{
						if (s_open) 
						{
							fprintf(stderr, "%s[%d]:  symtab %s already deserialized\n", FILE__, __LINE__, group->mutatee);
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
						//symtab = s_open;
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

				//  Don't think this can happen, but just to be safe, if we have
				//  a deserialized symtab for this mutatee, close it before we
				//  proceed, otherwise it will be returned for the CREATE tests
					if (s_open && s_open->from_cache())
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
					else
						symtab = s_open;
				//  verify that we have an existing cache for this mutatee from which to deserialize
				break;
			default:
				logerror( "%s[%d]:  bad runmode!\n", FILE__, __LINE__);
				return FAILED;
		}
	}
#endif

   if (group->mutatee && group->state != SELFSTART)
   {
	   if (NULL == symtab)
	   {
		   bool result = Symtab::openFile(symtab, std::string(group->mutatee));
		   if (!result || !symtab)
			   return FAILED;

#if defined (cap_serialization_test)
		if  (group->createmode == CREATE)
		{
			// manually trigger the parsing of type and line info here
			// so that everything gets serialized...  for the deserialize tests	
			std::vector<SymtabAPI::Module *> mods;
			bool result = symtab->getAllModules(mods);
			assert(result);
			assert(mods[0]);
			std::vector<Statement *> statements;
			mods[0]->getStatements(statements);
			Type *t = symtab->findType(0xdeadbeef);
		}
#endif
	   }
	   symtab_ptr.setPtr(symtab);
   }
   else
   {
      symtab_ptr.setPtr(NULL);
   }

   if (measure) um_group.end();  // Measure resource usage.

   params["Symtab"] = &symtab_ptr;
   params["createmode"]->setInt(group->createmode);
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
   createmode = (create_mode_t) param["createmode"]->getInt();
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
