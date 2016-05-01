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
#include "remotetest.h"
#include "ParameterDict.h"
#include "module.h"
#include "test_lib.h"

#include <cstring>
#include <cassert>
#include <string>
#include <stdio.h>

#if defined(os_freebsd_test)
extern char **environ;
#endif

using namespace std;

#define debug_printf(str, ...) do { if (getDebugLog()) { fprintf(getDebugLog(), str, ## __VA_ARGS__); fflush(getDebugLog()); } } while (0)

static void encodeParams(ParameterDict &params, MessageBuffer &buf)
{
   std::string result;
   
   result = PARAMETER_ARG + std::string(":");
   for (ParameterDict::iterator i=params.begin(); i != params.end(); i++) {
      result += i->first + std::string(":");
      if (dynamic_cast<ParamString *>(i->second)) 
      {
         result += std::string("s:");
         if (i->second->getString() == NULL) {
            result += "<NULL>" + std::string(":");
         }
         else if (*i->second->getString() == '\0') {
            result += "<EMPTY>" + std::string(":");
         }
         else {
            result += i->second->getString() + std::string(":");
         }
      }
      else if (dynamic_cast<ParamInt *>(i->second)) 
      {
         result += std::string("i:");
         char i_buffer[32];
         snprintf(i_buffer, 32, "%d:", i->second->getInt());
         result += i_buffer;
      }
      else if (dynamic_cast<ParamPtr *>(i->second)) 
      {
         result += std::string("p:");
         char p_buffer[32];
         snprintf(p_buffer, 32, "%p:", i->second->getPtr());
         result += p_buffer;
      }
      else {
         result += std::string("n:0x0:"); //NULL Pointer if unset
      }
   }
   result += std::string(";");
   buf.add(result.c_str(), result.length());
}

static char *decodeParams(ParameterDict &params, char *buffer)
{
   params.clear();
   char *cur = my_strtok(buffer, ":");
   assert(strcmp(cur, PARAMETER_ARG) == 0);
   
   for (;;) {
      cur = my_strtok(NULL, ":");
      if (*cur == ';')
         break;
      char *key = strdup(cur);
      cur = my_strtok(NULL, ":");
      char *type = strdup(cur);
      cur = my_strtok(NULL, ":");
      char *value = strdup(cur);
      char *orig_value = value;

      switch (*type) {
         case 's': {
            if (strcmp(value, "<NULL>") == 0)
               value = NULL;
            else if (strcmp(value, "<EMPTY>") == 0)
               value = const_cast<char *>("");
            params[key] = new ParamString(value);
            break;
         }
         case 'i': {
            int val;
            sscanf(value, "%d", &val);
            params[key] = new ParamInt(val);
            break;
         }
         case 'p': {
            unsigned long val;
            sscanf(value, "%lu", &val);
            params[key] = new ParamPtr((void *) val);
            break;
         }
         case 'n': {
            params[key];
            break;
         }
         default:
            debug_printf("BAD: %s %s %s %s\n", cur, key, type, value);
            assert(0);
      }
      free(key);
      free(type);
      free(orig_value);
   }
   char *next = strchr(buffer, ';');
   return next+1;
}

static void encodeTestResult(test_results_t res, MessageBuffer &buf)
{
   char s_buffer[64];
   snprintf(s_buffer, 64, "%s:%d;", TESTRESULT_ARG, res);
   buf.add(s_buffer, strlen(s_buffer));
}

static char *decodeTestResult(test_results_t &res, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   assert(strcmp(cur, TESTRESULT_ARG) == 0);
   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", (int *) &res);
   return strchr(buffer, ';')+1;
}

static void encodeGroup(RunGroup *group, MessageBuffer &buf)
{
   char s_buffer[64];
   snprintf(s_buffer, 64, "%s:%d;", GROUP_ARG, group->index);
   buf.add(s_buffer, strlen(s_buffer));
}

static char *decodeGroup(RunGroup* &group, vector<RunGroup *> &groups, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   assert(strcmp(cur, GROUP_ARG) == 0);
   unsigned int group_index;
   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", &group_index);
   assert(group_index >= 0 && group_index < groups.size());
   group = groups[group_index];
   return strchr(buffer, ';')+1;
}

static void encodeTest(TestInfo *test, MessageBuffer &buf)
{
   char s_buffer[128];
   snprintf(s_buffer, 128, "%s:%d:%d;", TESTINFO_ARG, test->group_index, test->index);
   buf.add(s_buffer, strlen(s_buffer));
}

static char *decodeTest(TestInfo* &test, vector<RunGroup *> &groups, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   assert(strcmp(cur, TESTINFO_ARG) == 0);
   unsigned int group_index, test_index;

   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", &group_index);
   assert(group_index >= 0 && group_index < groups.size());
   RunGroup *group = groups[group_index];

   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", &test_index);
   assert(test_index >= 0 && test_index < group->tests.size());
   
   test = group->tests[test_index];

   return strchr(buffer, ';')+1;
}

static void comp_header(std::string name, MessageBuffer &buffer, const char *call)
{
   buffer.add("C;", 2);
   buffer.add(call, strlen(call));
   buffer.add(";", 1);
   buffer.add(name.c_str(), strlen(name.c_str()));
   buffer.add(";", 1);   
}

static void test_header(TestInfo *test, MessageBuffer &buffer, const char *call)
{
   buffer.add("T;", 2);
   buffer.add(call, strlen(call));
   buffer.add(";", 1);
   char str[128];
   snprintf(str, 128, "%d:%d;", test->group_index, test->index);
   buffer.add(str, strlen(str));
}

static void load_header(MessageBuffer &buffer, std::string name)
{
   buffer.add("L;", 2);
   buffer.add(name.c_str(), name.length());
   buffer.add(";", 1);
}

static void return_header(MessageBuffer &buffer)
{
   buffer.add("R;", 2);
}

static void message_header(MessageBuffer &buffer) 
{
   buffer.add("M;", 2);
}

test_results_t RemoteComponentFE::program_setup(ParameterDict &params)
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_PROGRAM_SETUP);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return CRASHED;
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return CRASHED;
   
   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;
}

test_results_t RemoteComponentFE::program_teardown(ParameterDict &params)
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_PROGRAM_TEARDOWN);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return CRASHED;
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return CRASHED;

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;
}

test_results_t RemoteComponentFE::group_setup(RunGroup *group, ParameterDict &params)
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_GROUP_SETUP);

   encodeGroup(group, buffer);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return CRASHED;
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return CRASHED;

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;   
}

test_results_t RemoteComponentFE::group_teardown(RunGroup *group, ParameterDict &params)
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_GROUP_TEARDOWN);

   encodeGroup(group, buffer);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return CRASHED;
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return CRASHED;

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;
}

test_results_t RemoteComponentFE::test_setup(TestInfo *test, ParameterDict &params)
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_TEST_SETUP);

   encodeTest(test, buffer);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return CRASHED;
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return CRASHED;

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;      
}

test_results_t RemoteComponentFE::test_teardown(TestInfo *test, ParameterDict &params)
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_TEST_TEARDOWN);

   encodeTest(test, buffer);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return CRASHED;
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return CRASHED;

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;         
}

std::string RemoteComponentFE::getLastErrorMsg()
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_ERR_MSG);

   bool bresult = connection->send_message(buffer);
   if (!bresult) return std::string("BE DISCONNECT");
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) return std::string("BE DISCONNECT");

   std::string str;
   decodeString(str, result_msg);

   return str;
}

RemoteComponentFE::RemoteComponentFE(std::string n, Connection *c) :
   connection(c)
{
   //Strip any preceeding 'remote::'
   if (strstr(n.c_str(), "remote::"))
      name = std::string(strchr(n.c_str(), ':')+2);
   else
      name = n;
}

RemoteComponentFE::~RemoteComponentFE()
{
}

RemoteComponentFE *RemoteComponentFE::createRemoteComponentFE(std::string n, Connection *c)
{
   char *libpath = getenv("LD_LIBRARY_PATH");
   if (libpath) {
      setenv_on_remote(std::string("LD_LIBRARY_PATH"), std::string(libpath), c);
   }
      
   MessageBuffer buf;
   load_header(buf, LOAD_COMPONENT);

   encodeString(n, buf);
   bool result = c->send_message(buf);
   if (!result) return NULL;
   char *result_msg;
   result = c->recv_return(result_msg);
   if (!result) return NULL;

   decodeBool(result, result_msg);
   
   if (!result)
      return NULL;
   
   RemoteComponentFE *cmp = new RemoteComponentFE(n, c);
   return cmp;
}

bool RemoteComponentFE::setenv_on_remote(std::string var, std::string str, Connection *c)
{
   MessageBuffer buf;
   load_header(buf, SETENV);
   
   encodeString(var, buf);
   encodeString(str, buf);

   bool result = c->send_message(buf);
   if (!result) return false;
   char *result_msg;
   result = c->recv_return(result_msg);
   if (!result) return false;;
   
   decodeBool(result, result_msg);
   return result;
}

bool RemoteTestFE::hasCustomExecutionPath()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_CUSTOM_PATH);

   bool result = connection->send_message(buffer);
   if (!result) return false;
   char *result_msg;
   result = connection->recv_return(result_msg);
   if (!result) return false;

   bool b;
   decodeBool(b, result_msg);
   return b;
}

test_results_t RemoteTestFE::setup(ParameterDict &params)
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_SETUP);
   encodeParams(params, buffer);

   bool bresult = connection->send_message(buffer);
   if (!bresult) {
      logerror("Mutatee died during setup/send message\n");
      return CRASHED;
   }
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) {
      logerror("Mutatee died during setup/recv return\n");
      return CRASHED;
   }

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;
}

test_results_t RemoteTestFE::executeTest()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_EXECUTE);

   bool bresult = connection->send_message(buffer);
   if (!bresult) {
      logerror("Mutatee died during executeTest/send message\n");
      return CRASHED;
   }
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) {
      logerror("Mutatee died during executeTest/recv return\n");
      return CRASHED;
   }
   test_results_t result;
   decodeTestResult(result, result_msg);

   return result;
}

test_results_t RemoteTestFE::postExecution()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_POST_EXECUTE);

   bool bresult = connection->send_message(buffer);
   if (!bresult) {
      logerror("Mutatee died during postExecution/send message\n");
      return CRASHED;
   }
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) {
      logerror("Mutatee died during postExecution/recv return\n");
      return CRASHED;
   }

   test_results_t result;
   decodeTestResult(result, result_msg);

   return result;
}

test_results_t RemoteTestFE::teardown()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_TEARDOWN);

   bool bresult = connection->send_message(buffer);
   if (!bresult) {
      logerror("Mutatee died during teardown/send message\n");
      return CRASHED;
   }
   char *result_msg;
   bresult = connection->recv_return(result_msg);
   if (!bresult) {
      logerror("Mutatee died during postExecution/recv return\n");
      return CRASHED;
   }

   test_results_t result;
   decodeTestResult(result, result_msg);

   return result;
}

RemoteTestFE::RemoteTestFE(TestInfo *t, Connection *c) :
   test(t),
   connection(c)
{
}

RemoteTestFE::~RemoteTestFE()
{
}

RemoteTestFE *RemoteTestFE::createRemoteTestFE(TestInfo *t, Connection *c) {
   MessageBuffer buf;
   load_header(buf, LOAD_TEST);
   
   encodeTest(t, buf);
   bool result = c->send_message(buf);
   if (!result) return NULL;
   char *result_msg;
   result = c->recv_return(result_msg);
   if (!result) return NULL;

   decodeBool(result, result_msg);
   
   if (!result)
      return NULL;

   RemoteTestFE *test = new RemoteTestFE(t, c);
   return test;
}

RemoteBE::RemoteBE(vector<RunGroup *> &g, Connection *c) :
   groups(g),
   connection(c)
{
}

RemoteBE::~RemoteBE()
{
}

void RemoteBE::dispatch(char *message)
{
   char *message_begin = message+2;
   if (*message == 'C') {
      dispatchComp(message_begin);
   }
   else if (*message == 'T') {
      dispatchTest(message_begin);
   }
   else if (*message == 'L') {
      dispatchLoad(message_begin);
   }
   else if (*message == 'X') {
      dispatchExit(message_begin);
   }
   else {
      debug_printf("Failed to dispatch message %s\n", message);
      assert(0);
   }
}

void RemoteBE::dispatchLoad(char *message)
{
   if (strncmp(message, LOAD_TEST, strlen(LOAD_TEST)) == 0) {
      loadTest(message);
   }
   else if (strncmp(message, LOAD_COMPONENT, strlen(LOAD_COMPONENT)) == 0) {
      loadModule(message);
   }
   else if (strncmp(message, SETENV, strlen(SETENV)) == 0) {
      setenv_on_local(message);
   }
   else {
      assert(0);
   }
}

void RemoteBE::dispatchTest(char *message)
{
   char *tag = strdup(my_strtok(message, ":;"));
   char *group_s = strdup(my_strtok(NULL, ":;"));
   char *test_s = strdup(my_strtok(NULL, ":;"));

   char *args = strchr(message, ';')+1;
   args = strchr(args, ';')+1;

   int group_index, test_index;
   sscanf(group_s, "%d", &group_index);
   sscanf(test_s, "%d", &test_index);
   
   TestMutator *test = getTestBE(group_index, test_index);

   MessageBuffer buffer;
   return_header(buffer);

   if (strcmp(tag, TEST_CUSTOM_PATH) == 0) {
      bool result = test->hasCustomExecutionPath();
      encodeBool(result, buffer);
   }
   else if (strcmp(tag, TEST_SETUP) == 0) {
      ParameterDict params;
      args = decodeParams(params, args);
      test_results_t res = test->setup(params);
      encodeParams(params, buffer);
      encodeTestResult(res, buffer);
   }
   else if (strcmp(tag, TEST_EXECUTE) == 0) {
      test_results_t res = test->executeTest();
      encodeTestResult(res, buffer);
   }
   else if (strcmp(tag, TEST_POST_EXECUTE) == 0) {
      test_results_t res = test->postExecution();
      encodeTestResult(res, buffer);
   }
   else if (strcmp(tag, TEST_TEARDOWN) == 0) {
      test_results_t res = test->teardown();
      encodeTestResult(res, buffer);
   }
   else 
      assert(0);

   connection->send_message(buffer);

   free(tag);
   free(test_s);
}

void RemoteBE::dispatchComp(char *message)
{
   char *tag = strdup(my_strtok(message, ":;"));
   char *name = strdup(my_strtok(NULL, ":;"));

   char *args = strchr(message, ';')+1;
   args = strchr(args, ';')+1;
   
   ComponentTester *compbe = getComponentBE(name);
   assert(compbe);

   MessageBuffer buffer;
   return_header(buffer);

   ParameterDict params;
   RunGroup *group;
   TestInfo *test;
   test_results_t result;
   if (strcmp(tag, COMPONENT_PROGRAM_SETUP) == 0) {
      args = decodeParams(params, args);
      result = compbe->program_setup(params);
   }
   else if (strcmp(tag, COMPONENT_PROGRAM_TEARDOWN) == 0) {
      args = decodeParams(params, args);
      result = compbe->program_teardown(params);
   }
   else if (strcmp(tag, COMPONENT_GROUP_SETUP) == 0) {
      args = decodeGroup(group, groups, args);
      args = decodeParams(params, args);
      result = compbe->group_setup(group, params);
   }
   else if (strcmp(tag, COMPONENT_GROUP_TEARDOWN) == 0) {
      args = decodeGroup(group, groups, args);
      args = decodeParams(params, args);
      result = compbe->group_teardown(group, params);
   }
   else if (strcmp(tag, COMPONENT_TEST_SETUP) == 0) {
      args = decodeTest(test, groups, args);
      args = decodeParams(params, args);
      result = compbe->test_setup(test, params);
   }
   else if (strcmp(tag, COMPONENT_TEST_TEARDOWN) == 0) {
      args = decodeTest(test, groups, args);
      args = decodeParams(params, args);
      result = compbe->test_teardown(test, params);
   }

   if (strcmp(tag, COMPONENT_ERR_MSG) == 0) {      
      std::string str_result = compbe->getLastErrorMsg();
      encodeString(str_result, buffer);
   }
   else {
      encodeParams(params, buffer);
      encodeTestResult(result, buffer);
   }

   connection->send_message(buffer);

   free(tag);
   free(name);
}

void RemoteBE::setenv_on_local(char *message)
{
   assert(strncmp(message, SETENV, strlen(SETENV)) == 0);
   char *args = strchr(message, ';')+1;

   std::string var, str;
   args = decodeString(var, args);
   args = decodeString(str, args);
   
   debug_printf("Setting local environment %s = %s\n", var.c_str(), str.c_str());
   int result = setenv(var.c_str(), str.c_str(), 1);
   
   bool bresult = (result == 0);
   MessageBuffer buffer;
   return_header(buffer);
   encodeBool(bresult, buffer);
   connection->send_message(buffer);
}

void RemoteBE::dispatchExit(char *message)
{
   exit(0);
}

static std::string getLocalComponentName(std::string modname)
{
   int prefix_length = strlen("remote::");
   if (strncmp(modname.c_str(), "remote::", prefix_length) == 0) {
      return std::string(modname.c_str()+prefix_length);
   }
   return modname;
}


ComponentTester *RemoteBE::getComponentBE(std::string name)
{
   map<string, ComponentTester *>::iterator i = nameToComponent.find(getLocalComponentName(name));
   assert(i != nameToComponent.end());
   return i->second;
}

TestMutator *RemoteBE::getTestBE(int group_index, int test_index)
{
   map<pair<int, int>, TestMutator *>::iterator i;
   i = testToMutator.find(pair<int, int>(group_index, test_index));
   assert(i != testToMutator.end());
   return i->second;
}

void RemoteBE::loadModule(char *message) {
   assert(strncmp(message, LOAD_COMPONENT, strlen(LOAD_COMPONENT)) == 0);
   char *args = strchr(message, ';')+1;

   bool error = false;

   string modname;
   decodeString(modname, args);
   modname = getLocalComponentName(modname);

   map<string, ComponentTester *>::iterator i;
   i = nameToComponent.find(modname);

   if (i == nameToComponent.end()) {
      ComponentTester *comp = NULL;
      for (unsigned j=0; j<groups.size(); j++) {
         RunGroup *group = groups[j];
         if (group->modname == modname) {
            bool result = Module::registerGroupInModule(modname, group, false);
            if (!result) {
               error = true;
               goto done;
            }
            if (!comp)
               comp = group->mod->tester;
            assert(comp == group->mod->tester);
         }
      }
      nameToComponent[modname] = comp;
   }
   
  done:
   MessageBuffer buffer;
   return_header(buffer);
   encodeBool(!error, buffer);
   connection->send_message(buffer);   
}

void RemoteBE::loadTest(char *message) {
   assert(strncmp(message, LOAD_TEST, strlen(LOAD_TEST)) == 0);
   char *args = strchr(message, ';')+1;
   
   TestInfo *test;
   decodeTest(test, groups, args);
   int group_index = test->group_index;
   int test_index = test->index;
   RunGroup *group = groups[group_index];

   map<pair<int, int>, TestMutator *>::iterator i;
   i = testToMutator.find(pair<int, int>(group_index, test_index));
   if (i == testToMutator.end()) {
      setupMutatorsForRunGroup(group);
      for (unsigned j=0; j < group->tests.size(); j++) {
         TestMutator *mutator = group->tests[j]->mutator;
         if (mutator) {
            testToMutator[pair<int, int>(group_index, test_index)] = mutator;
         }
      }
   }

   bool module_result, test_result;
   test_result = group->tests[test_index]->mutator != NULL;
   module_result = group->mod != NULL;

   MessageBuffer buffer;
   return_header(buffer);

   encodeBool(test_result && module_result, buffer);
   connection->send_message(buffer);   
}

void RemoteOutputDriver::startNewTest(std::map<std::string, std::string> &, TestInfo *, RunGroup *)
{
   assert(0); //Not expected to be called from BE
}

void RemoteOutputDriver::redirectStream(TestOutputStream stream, const char * filename)
{
   assert(0); //Not expected to be called from BE   
}

void RemoteOutputDriver::logResult(test_results_t result, int stage)
{
   assert(0); //Not expected to be called from BE
}

void RemoteOutputDriver::logCrash(std::string testname)
{
   assert(0); //Not expected to be called from BE
}

void RemoteOutputDriver::log(TestOutputStream stream, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vlog(stream, fmt, args);
  va_end(args);
}

void RemoteOutputDriver::vlog(TestOutputStream stream, const char *fmt, va_list args)
{
   static char buffer[4096];
   vsnprintf(buffer, 4095, fmt, args);
   buffer[4095] = '\0';

   MessageBuffer msg;
   message_header(msg);
   encodeInt(stream, msg);
   encodeString(std::string(buffer), msg);

   connection->send_message(msg);
}

void RemoteOutputDriver::finalizeOutput()
{
   assert(0); //Not expected to be called from BE
}

RemoteOutputDriver::RemoteOutputDriver(Connection *c) :
   connection(c)
{
}

RemoteOutputDriver::~RemoteOutputDriver()
{
}

bool sendRawString(Connection *c, std::string s) {
   MessageBuffer mb;
   mb.add(s.c_str(), s.length());
   return c->send_message(mb);
}

bool sendEnv(Connection *c)
{
   static MessageBuffer buf;
   static bool have_buf = false;
   if (!have_buf) {
      buf.add("E:", 2);

      char env_size[16];
      unsigned env_size_count = 0;
      char **cur = environ;
      while (*(cur++) != NULL) env_size_count++;
      snprintf(env_size, 15, "%d", env_size_count);
      buf.add(env_size, strlen(env_size));
      buf.add(":", 1);

      cur = environ;
      while (*cur != NULL) {
         char *curenv = *cur;
         char *equal = strchr(curenv, '=');
         buf.add(curenv, (unsigned int) (equal - curenv));
         buf.add("", 1);
         
         curenv = equal+1;
         unsigned int eval_size = strlen(curenv) + 1;
         buf.add(curenv, eval_size);
         curenv += eval_size;
         cur++;
      }

      have_buf = true;
   }

   bool result = c->send_message(buf);
   return result;
}

bool sendArgs(char **args, Connection *c)
{
   MessageBuffer buf;
   buf.add("A:", 2);
   
   char args_size[16];
   unsigned args_size_count = 0;
   char **cur = args;
   while (*(cur++) != NULL) args_size_count++;
   snprintf(args_size, 15, "%d", args_size_count);
   buf.add(args_size, strlen(args_size));
   buf.add(":", 1);

   cur = args;
   while (*cur != NULL) {
      char *curarg = *cur;
      unsigned int curarg_size = strlen(curarg)+1;
      buf.add(curarg, curarg_size);
      cur++;
   }

   bool result = c->send_message(buf);
   return result;
}

bool sendLDD(Connection *c, std::string libname, std::string &result)
{
   MessageBuffer buf;
   buf.add("L:", 2);
   buf.add(libname.c_str(), libname.length()+1);

   bool bresult = c->send_message(buf);
   if (!bresult)
      return false;

   char *buffer;
   bresult = c->recv_message(buffer);
   if (!bresult)
      return false;
   
   result = std::string(buffer);
   return true;
}

bool sendGo(Connection *c) {
   MessageBuffer buf;
   buf.add("G:", 2);
   return c->send_message(buf);
}

void handle_message(char *buffer) {
   TestOutputStream stream;
   std::string string;

   buffer = decodeInt(stream, buffer);
   decodeString(string, buffer);
   logerror(string.c_str());
}
