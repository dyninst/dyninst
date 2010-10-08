/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "remotetest.h"
#include "ParameterDict.h"
#include "module.h"
#include "test_lib.h"

#include <cstring>
#include <cassert>
#include <string>

using namespace std;

extern FILE *debug_log;
#define debug_printf(str, args...) do { if (debug_log) { fprintf(debug_log, str, args); fflush(debug_log); } } while (0)

#define PARAMETER_ARG "PARAMETER"
#define TESTRESULT_ARG "TESTRESULT"
#define STRING_ARG "STRING"
#define GROUP_ARG "GROUP"
#define TESTINFO_ARG "TESTINFO"
#define BOOL_ARG "BOOL"
#define INT_ARG "INT"

#define COMPONENT_PROGRAM_SETUP "COMP_PROGSETUP"
#define COMPONENT_PROGRAM_TEARDOWN "COMP_PROGTEARDOWN"
#define COMPONENT_GROUP_SETUP "COMP_GROUPSETUP"
#define COMPONENT_GROUP_TEARDOWN "COMP_GROUPTEARDOWN"
#define COMPONENT_TEST_SETUP "COMP_TESTSETUP"
#define COMPONENT_TEST_TEARDOWN "COMP_TESTTEARDOWN"
#define COMPONENT_ERR_MSG "COMP_ERRMESSAGE"

#define TEST_CUSTOM_PATH "TEST_CUSTOMPATH"
#define TEST_SETUP "TEST_SETUP"
#define TEST_EXECUTE "TEST_EXECUTE"
#define TEST_POST_EXECUTE "TEST_POST_EXECUTE"
#define TEST_TEARDOWN "TEST_TEARDOWN"

#define LOAD_TEST "LOAD_TEST"
#define LOAD_COMPONENT "LOAD_COMPONENT"
#define RETURN "RETURN"
#define LOG "LOG"
#define EXIT_MSG "EXIT"

#define SETENV "SETENV"

static char *my_strtok(char *str, const char *delim);

#define my_assert(bval) do { if (!(bval)) { if (debug_log) { fprintf(debug_log, "[%s:%u] - Failed my_assert: %s\n", __FILE__, __LINE__, #bval); fflush(debug_log); } *((int *) 0x0) = 0x0; } } while (0)
static void crash_ifnot()
{
   *((int *) 0x0) = 0x0;
}

static void crash_ifnot(bool b)
{
   if (!b)
      *((int *) 0x0) = 0x0;
}

class MessageBuffer
{
private:
   char *buffer;
   unsigned int size;
   unsigned int cur;
public:
   MessageBuffer() :
      buffer(NULL),
      size(0),
      cur(0)
   {
   }

   ~MessageBuffer() 
   {
      if (buffer)
         free(buffer);
      buffer = NULL;
   }

   void add(const char *b, unsigned int b_size)
   {
      if (!buffer) {
         size = (b_size * 2);
         buffer = (char *) malloc(size);
      }
      if (cur + b_size > size) {
         while (cur + b_size > size)
            size *= 2;
         buffer = (char *) realloc(buffer, size);
      }
      memcpy(buffer+cur, b, b_size);
      cur += b_size;
   }

   char *get_buffer() const {
      char *new_buffer = (char *) malloc(cur*2);
      memset(new_buffer, 0xab, cur*2);
      memcpy(new_buffer, buffer, cur);
      return new_buffer;
   }

   unsigned int get_buffer_size() const {
      return cur;
   }
};

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
      if (dynamic_cast<ParamInt *>(i->second)) 
      {
         result += std::string("i:");
         char i_buffer[32];
         snprintf(i_buffer, 32, "%d:", i->second->getInt());
         result += i_buffer;
      }
      if (dynamic_cast<ParamPtr *>(i->second)) 
      {
         result += std::string("p:");
         char p_buffer[32];
         snprintf(p_buffer, 32, "%lu:", i->second->getPtr());
         result += p_buffer;
      }
   }
   result += std::string(";");
   buf.add(result.c_str(), result.length());
}

static char *decodeParams(ParameterDict &params, char *buffer)
{
   params.clear();
   char *cur = my_strtok(buffer, ":");
   my_assert(strcmp(cur, PARAMETER_ARG) == 0);
   
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
               value = "";
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
         default:
            my_assert(0);
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
   my_assert(strcmp(cur, TESTRESULT_ARG) == 0);
   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", (int *) &res);
   return strchr(buffer, ';')+1;
}

static void encodeInt(int i, MessageBuffer &buf)
{
   char s_buffer[64];
   snprintf(s_buffer, 64, "%s:%d;", INT_ARG, i);
   buf.add(s_buffer, strlen(s_buffer));
}

static char *decodeInt(int i, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   my_assert(strcmp(cur, INT_ARG) == 0);
   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", (int *) &i);
   return strchr(buffer, ';')+1;
}

static void encodeString(std::string str, MessageBuffer &buf)
{
   buf.add(STRING_ARG, strlen(STRING_ARG));
   buf.add(":", 1);
   if (!str.length())
      buf.add("<EMPTY>", strlen("<EMPTY>"));
   else
      buf.add(str.c_str(), str.length());
   buf.add(";", 1);
}

static char *decodeString(std::string &str, char *buffer)
{
   my_assert(strncmp(buffer, STRING_ARG, strlen(STRING_ARG)) == 0);
   char *cur = my_strtok(buffer, ";");
   cur += strlen(STRING_ARG)+1;
   if (strncmp(cur, "<EMPTY>", strlen("<EMPTY>")) == 0)
      str = std::string();
   else
      str = std::string(cur);
   return strchr(buffer, ';')+1;
}

static void encodeBool(bool b, MessageBuffer &buf)
{
   buf.add(BOOL_ARG, strlen(BOOL_ARG));
   buf.add(":", 1);
   string str = b ? "true" : "false";
   buf.add(str.c_str(), str.length());
   buf.add(";", 1);
}

static char *decodeBool(bool &b, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   my_assert(strcmp(cur, BOOL_ARG) == 0);
   cur = my_strtok(NULL, ":;");
   string str = std::string(cur);
   if (str == "true") {
      b = true;
   }
   else if (str == "false") {
      b = false;
   }
   else {
      my_assert(0);
   }
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
   my_assert(strcmp(cur, GROUP_ARG) == 0);
   int group_index;
   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", &group_index);
   my_assert(group_index >= 0 && group_index < groups.size());
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
   my_assert(strcmp(cur, TESTINFO_ARG) == 0);
   int group_index, test_index;

   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", &group_index);
   my_assert(group_index >= 0 && group_index < groups.size());
   RunGroup *group = groups[group_index];

   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", &test_index);
   my_assert(test_index >= 0 && test_index < group->tests.size());
   
   test = group->tests[test_index];

   return strchr(buffer, ';')+1;
}

static void comp_header(std::string name, MessageBuffer &buffer, char *call)
{
   buffer.add("C;", 2);
   buffer.add(call, strlen(call));
   buffer.add(";", 1);
   buffer.add(name.c_str(), strlen(name.c_str()));
   buffer.add(";", 1);   
}

static void test_header(TestInfo *test, MessageBuffer &buffer, char *call)
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

static void exit_header(MessageBuffer &buffer)
{
   buffer.add("X;", 2);
   buffer.add(EXIT_MSG, strlen(EXIT_MSG));
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

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;         
}

std::string RemoteComponentFE::getLastErrorMsg()
{
   MessageBuffer buffer;
   comp_header(name, buffer, COMPONENT_ERR_MSG);

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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
   c->send_message(buf);
   char *result_msg;
   c->recv_return(result_msg);

   bool result;
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

   c->send_message(buf);
   char *result_msg;
   c->recv_return(result_msg);
   
   bool result;
   decodeBool(result, result_msg);
   return result;
}

bool RemoteTestFE::hasCustomExecutionPath()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_CUSTOM_PATH);

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

   bool b;
   decodeBool(b, result_msg);
   return b;
}

test_results_t RemoteTestFE::setup(ParameterDict &params)
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_SETUP);
   encodeParams(params, buffer);

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

   char *next_ret = decodeParams(params, result_msg);
   test_results_t result;
   decodeTestResult(result, next_ret);

   return result;
}

test_results_t RemoteTestFE::executeTest()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_EXECUTE);

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

   test_results_t result;
   decodeTestResult(result, result_msg);

   return result;
}

test_results_t RemoteTestFE::postExecution()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_POST_EXECUTE);

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

   test_results_t result;
   decodeTestResult(result, result_msg);

   return result;
}

test_results_t RemoteTestFE::teardown()
{
   MessageBuffer buffer;
   test_header(test, buffer, TEST_TEARDOWN);

   connection->send_message(buffer);
   char *result_msg;
   connection->recv_return(result_msg);

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
   c->send_message(buf);
   char *result_msg;
   c->recv_return(result_msg);

   bool result;
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
      my_assert(0);
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
      my_assert(0);
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
      my_assert(0);

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
   my_assert(compbe);

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
   my_assert(strncmp(message, SETENV, strlen(SETENV)) == 0);
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
   my_assert(i != nameToComponent.end());
   return i->second;
}

TestMutator *RemoteBE::getTestBE(int group_index, int test_index)
{
   map<pair<int, int>, TestMutator *>::iterator i;
   i = testToMutator.find(pair<int, int>(group_index, test_index));
   my_assert(i != testToMutator.end());
   return i->second;
}

void RemoteBE::loadModule(char *message) {
   my_assert(strncmp(message, LOAD_COMPONENT, strlen(LOAD_COMPONENT)) == 0);
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
            my_assert(comp == group->mod->tester);
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
   my_assert(strncmp(message, LOAD_TEST, strlen(LOAD_TEST)) == 0);
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
   my_assert(0); //Not expected to be called from BE
}

void RemoteOutputDriver::redirectStream(TestOutputStream stream, const char * filename)
{
   my_assert(0); //Not expected to be called from BE   
}

void RemoteOutputDriver::logResult(test_results_t result, int stage)
{
   my_assert(0); //Not expected to be called from BE
}

void RemoteOutputDriver::logCrash(std::string testname)
{
   my_assert(0); //Not expected to be called from BE
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
   my_assert(0); //Not expected to be called from BE
}

RemoteOutputDriver::RemoteOutputDriver(Connection *c) :
   connection(c)
{
}

static void handle_message(char *buffer) {
   TestOutputStream stream;
   std::string string;

   buffer = decodeInt(stream, buffer);
   decodeString(string, buffer);
   logerror(string.c_str());
}

#define SOCKTYPE_UNIX

bool Connection::recv_return(char* &buffer) {
   char *msg;
   for (;;) {
      bool result = recv_message(msg);
      if (!result)
         return false;
   
      if (*msg == 'R') {
         buffer = msg+2;
         return true;
      }
      if (*msg == 'M') {
         handle_message(msg+2);
      }
   }
}

#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

int Connection::sockfd = -1;
std::string Connection::hostname;
int Connection::port;
bool Connection::has_hostport = false;

Connection::Connection() :
   fd(-1),
   has_error(false)
{
}

Connection::Connection(std::string hostname_, int port_) :
   fd(-1)
{
   hostname = hostname_;
   port = port_;
   has_hostport = true;

   has_error = !client_connect();
}

Connection::~Connection()
{
   MessageBuffer buf;
   exit_header(buf);
   send_message(buf);

   if (fd != -1)
      close(fd);
}

bool Connection::hasError()
{
   return has_error;
}

bool Connection::send_message(MessageBuffer &buffer)
{
   buffer.add("\0", 1);

   uint32_t msg_size_unenc = buffer.get_buffer_size();
   my_assert(msg_size_unenc == strlen(buffer.get_buffer())+1);
   uint32_t msg_size = htonl(msg_size_unenc);

   ssize_t result = send(fd, &msg_size, sizeof(uint32_t), 0);
   if (result == -1) {
      debug_printf("[%s:%u] - Error sending data count on socket\n", __FILE__, __LINE__);
      return false;
   }
   debug_printf("[%d] - Send size of %lu, (encoded 0x%lx)\n", 
           getpid(), (unsigned long) msg_size_unenc, (unsigned long) msg_size);
   
   result = send(fd, buffer.get_buffer(), msg_size_unenc, 0);
   if (result == -1) {
     debug_printf("[%s:%u] - Error sending raw data on socket\n", __FILE__, __LINE__);
      return false;
   }
  debug_printf("[%d] - Sent buffer %s\n", getpid(), buffer.get_buffer());
   return true;
}

bool Connection::recv_message(char* &buffer)
{
   static char *cur_buffer = NULL;
   static int cur_buffer_size = 0;
   bool sock_error;

   if (!waitForAvailData(fd, recv_timeout, sock_error))
      return false;
   if (sock_error) {
     debug_printf("[%d] - Recv sock exception\n", getpid());
   }
   
   uint32_t msg_size = 0, enc_msg_size = 0;
   ssize_t result;
   result = recv(fd, &enc_msg_size, sizeof(uint32_t), MSG_WAITALL);
   if (result == -1) {
      int errornum = errno;
      debug_printf("Error receiving data size on socket: %s\n", strerror(errornum));
      return false;
   }
   if (result == 0) {
      debug_printf("[%d] - Recv zero, other size shutdown.\n", getpid());
      return false;
   }
   msg_size = ntohl(enc_msg_size);
   debug_printf("[%d] - Recv size of %lu, (encoded 0x%lx, result = %d)\n", 
                getpid(), (unsigned long) msg_size, enc_msg_size, (int) result);

   my_assert(msg_size < (1024*1024)); //No message over 1MB--should be plenty
   
   if (msg_size == 0) {
      //Other side hung up.
      return false;
   }

   if (msg_size > cur_buffer_size) {
      if (cur_buffer)
         free(cur_buffer);
      cur_buffer = NULL;
   }
   if (cur_buffer == NULL) {
      cur_buffer_size = msg_size+1;
      cur_buffer = (char *) malloc(cur_buffer_size);
   }
   memset(cur_buffer, 0, cur_buffer_size);

   result = recv(fd, cur_buffer, msg_size, MSG_WAITALL);
   if (result == -1) {
     debug_printf("[%s:%u] - Error receiving data on socket\n", __FILE__, __LINE__);
      return false;
   }

   buffer = cur_buffer;
  debug_printf("[%d] - Recv of buffer %s\n", getpid(), buffer);

   return true;
}

bool Connection::waitForAvailData(int sock, int timeout_s, bool &sock_error)
{
   fd_set readfds;
   fd_set exceptfds;
   fd_set writefds;
   FD_ZERO(&readfds);
   FD_ZERO(&exceptfds);
   FD_ZERO(&writefds);
   FD_SET(sock, &readfds);
   FD_SET(sock, &exceptfds);

   struct timeval timeout;
   timeout.tv_sec = timeout_s;
   timeout.tv_usec = 0;
   sock_error = false;

   for (;;) {
      int result = select(sock+1, &readfds, &writefds, &exceptfds, &timeout);
      if (result == -1 && errno == EINTR) 
         continue;
      else if (result == -1) {
         debug_printf("[%s:%u] - Error selecting to accept connections\n", __FILE__, __LINE__);
         return false;
      }
      else if (result == 0) {
        debug_printf("[%s:%u] - Timeout accepting connections\n", __FILE__, __LINE__);
         return false;
      }
      else if (result >= 1) {
         if (FD_ISSET(sock, &readfds) && FD_ISSET(sock, &exceptfds)) {
            sock_error = true;
            return true;
         }
         if (FD_ISSET(sock, &readfds)) {
            return true;
         }
         if (FD_ISSET(sock, &exceptfds)) {
            sock_error = true;
            return false;
         }
         my_assert(0);
      }      
      else {
         my_assert(0);
      }
   }
}

bool Connection::server_accept()
{
   struct sockaddr_in addr;
   socklen_t socklen = sizeof(struct sockaddr_in);
   bool sock_error;

   if (!waitForAvailData(sockfd, accept_timeout, sock_error))
      return false;
       
   my_assert(fd == -1); //One connection at a time.

   fd = accept(sockfd, (sockaddr *) &addr, &socklen);
   if (fd == -1) {
     debug_printf("[%s:%u] - Error accepting connection\n", __FILE__, __LINE__);
      return false;
   }
   
   return true;
}

bool Connection::client_connect()
{
   debug_printf("Trying client_connect to %s:%d\n", hostname.c_str(), port);

   my_assert(has_hostport);
   fd = socket(PF_INET, SOCK_STREAM, 0);
   if (fd == -1) {
      debug_printf("Unable to create client socket: %s\n", strerror(errno));
      return false;
   }

   debug_printf("Trying to get hostname for %s\n", hostname.c_str());
   struct hostent *host = gethostbyname2(hostname.c_str(), AF_INET);
   if (!host) {
      debug_printf("Error looking up hostname %s\n", hostname.c_str());
      return false;
   }
   my_assert(host->h_addrtype = AF_INET);

   debug_printf("Got an %d addresses for hostname %s\n", host->h_length, hostname.c_str());
   if (host->h_length == 0) {
      debug_printf("No addresses with hostname %s\n", hostname.c_str());
      return false;
   } 

   struct sockaddr_in addr;
   struct in_addr iaddr;
   bzero(&addr, sizeof(addr));
   socklen_t socklen = sizeof(struct sockaddr_in);
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port); 
   //iaddr.s_addr = htonl(*((int *) host->h_addr_list[0]));
   inet_aton("172.16.126.164", &iaddr);
   addr.sin_addr = iaddr;
   debug_printf("Connecting to %d.%d.%d.%d:%d\n", 
                (int) (((char *) &addr.sin_addr)[0]),
                (int) (((char *) &addr.sin_addr)[1]),
                (int) (((char *) &addr.sin_addr)[2]),
                (int) (((char *) &addr.sin_addr)[3]),
                (int) addr.sin_port);

   int result = connect(fd, (struct sockaddr *) &addr, socklen);
   if (result == -1) {
      debug_printf("[%s:%u] - Error connecting to server\n", __FILE__, __LINE__);
      return false;
   }

   debug_printf("Successfully connected to %s\n", hostname.c_str());
   return true;
}

bool Connection::server_setup(string &hostname_, int &port_)
{
   if (has_hostport) {
      hostname_ = hostname;
      port_ = port;
      my_assert(sockfd != -1);
      return true;
   }

   sockfd = socket(PF_INET, SOCK_STREAM, 0);
   if (sockfd == -1) {
     debug_printf("Unable to create socket: %s\n", strerror(errno));
      return false;
   }
   
   struct sockaddr_in addr;
   socklen_t socklen = sizeof(struct sockaddr_in);
   
   memset(&addr, 0, socklen);
   addr.sin_family = AF_INET;
   addr.sin_port = 0;
   addr.sin_addr.s_addr = INADDR_ANY;
   
   int result = bind(sockfd, (struct sockaddr *) &addr, socklen);
   if (result != 0){
     debug_printf("Unable to bind socket: %s\n", strerror(errno));
      return false;
   }
   
   result = listen(sockfd, 16);
   if (result == -1) {
     debug_printf("Unable to listen on socket: %s\n", strerror(errno));
      return false;
   }

   result = getsockname(sockfd, (sockaddr *) &addr, &socklen);
   if (result != 0) {
      debug_printf("[%s:%u] - Unable to getsockname on socket\n", __FILE__, __LINE__);
      return false;
   }

   char name_buffer[1024];
   result = gethostname(name_buffer, 1024);
   if (result != 0) {
     debug_printf("[%s:%u] - Unable to get hostname\n", __FILE__, __LINE__);
      return false;
   }

   hostname = name_buffer;
   port = (int) addr.sin_port;

   hostname_ = hostname;
   port_ = port;
   has_hostport = true;
   return true;
}

static Connection *con = NULL;
Connection *getConnection()
{
   return con;
}

void setConnection(Connection *c)
{
   con = c;
}

static char *my_strtok(char *str, const char *delim)
{
   static char *my_str = NULL;
   static char *save_ptr = NULL;
   
   if (str) {
      char *backup_str = strdup(str);
      if (my_str)
         free(my_str);
      my_str = backup_str;
   }
   else {
      my_str = NULL;
   }
   
   return strtok_r(my_str, delim, &save_ptr);
}
