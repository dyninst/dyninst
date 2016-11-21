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
#include "comptester.h"
#include "TestMutator.h"

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

class MessageBuffer;

class TESTLIB_DLL_EXPORT Connection
{
  private:
   static const int accept_timeout = 60;
   static const int recv_timeout = 60;
   static int sockfd;
   static std::string hostname;
   static int port;
   static bool has_hostport;

   int fd;
   bool has_error;

   bool waitForAvailData(int sock, int timeout, bool &sock_error);
  public:
   Connection();
   Connection(std::string host, int port, int fd_exists = -1);
   ~Connection();

   bool server_accept();
   bool server_setup(std::string &hostname, int &port);
   bool client_connect();

   bool send_message(MessageBuffer &buffer);
   bool recv_message(char* &buffer);
   bool recv_return(char* &buffer);

   bool hasError();

   int getFD();
};

TESTLIB_DLL_EXPORT Connection *getConnection();
TESTLIB_DLL_EXPORT void setConnection(Connection *);

class TESTLIB_DLL_EXPORT RemoteComponentFE : public ComponentTester {
  private:
   std::string name;
   Connection *connection;

   RemoteComponentFE(std::string n, Connection *c);
  public:
   static RemoteComponentFE *createRemoteComponentFE(std::string n, Connection *c);
   static bool setenv_on_remote(std::string var, std::string str, Connection *c);
   virtual ~RemoteComponentFE();

   virtual test_results_t program_setup(ParameterDict &params);
   virtual test_results_t program_teardown(ParameterDict &params);
   virtual test_results_t group_setup(RunGroup *group, ParameterDict &params);
   virtual test_results_t group_teardown(RunGroup *group, ParameterDict &params);
   virtual test_results_t test_setup(TestInfo *test, ParameterDict &params);
   virtual test_results_t test_teardown(TestInfo *test, ParameterDict &params);
   virtual std::string getLastErrorMsg();

};

class TESTLIB_DLL_EXPORT RemoteTestFE : public TestMutator {
  private:
   TestInfo *test;
   Connection *connection;

   RemoteTestFE(TestInfo *test_, Connection *c);
  public:
   static RemoteTestFE *createRemoteTestFE(TestInfo *t, Connection *c);
   virtual ~RemoteTestFE();

   virtual bool hasCustomExecutionPath();
   virtual test_results_t setup(ParameterDict &param);
   virtual test_results_t executeTest();
   virtual test_results_t postExecution();
   virtual test_results_t teardown();
};

class RemoteBE {
  private:
   Connection *connection;
   std::vector<RunGroup *> &groups;
   std::map<std::string, ComponentTester *> nameToComponent;
   std::map<std::pair<int, int>, TestMutator *> testToMutator;

   void dispatchComp(char *message);
   void dispatchTest(char *message);
   void dispatchLoad(char *message);
   void dispatchExit(char *message);

   ComponentTester *getComponentBE(std::string name);
   TestMutator *getTestBE(int group_index, int test_index);

   void loadTest(char *message);
   void loadModule(char *message);
   void setenv_on_local(char *message);
  public:
   RemoteBE(std::vector<RunGroup *> &g, Connection *c);
   virtual ~RemoteBE();
   
   void dispatch(char *message);
};

class RemoteOutputDriver : public TestOutputDriver {
  private:
   void send_log(std::string str);
   Connection *connection;
  public:

   RemoteOutputDriver(Connection *c);
   ~RemoteOutputDriver();
   virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group);
   virtual void redirectStream(TestOutputStream stream, const char * filename);
   virtual void logResult(test_results_t result, int stage=-1);
   virtual void logCrash(std::string testname);
   virtual void log(TestOutputStream stream, const char *fmt, ...);
   virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
   virtual void finalizeOutput();
};

bool sendRawString(Connection *c, std::string s);

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

char *decodeInt(int i, char *buffer);
char *decodeBool(bool &b, char *buffer);
char *decodeString(std::string &str, char *buffer);
void encodeInt(int i, MessageBuffer &buf);
void encodeString(std::string str, MessageBuffer &buf);
void encodeBool(bool b, MessageBuffer &buf);
char *my_strtok(char *str, const char *delim);

bool sendEnv(Connection *c);
bool sendArgs(char **args, Connection *c);
bool sendLDD(Connection *c, std::string libname, std::string &result);
bool sendGo(Connection *c);
