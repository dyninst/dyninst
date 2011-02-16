#include "comptester.h"
#include "TestMutator.h"

class MessageBuffer;

class Connection
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
   Connection(std::string host, int port);
   ~Connection();

   bool server_accept();
   bool server_setup(std::string &hostname, int &port);
   bool client_connect();

   bool send_message(MessageBuffer &buffer);
   bool recv_message(char* &buffer);
   bool recv_return(char* &buffer);

   bool hasError();
};

Connection *getConnection();
void setConnection(Connection *);

class RemoteComponentFE : public ComponentTester {
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

class RemoteTestFE : public TestMutator {
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
   virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group);
   virtual void redirectStream(TestOutputStream stream, const char * filename);
   virtual void logResult(test_results_t result, int stage=-1);
   virtual void logCrash(std::string testname);
   virtual void log(TestOutputStream stream, const char *fmt, ...);
   virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);
   virtual void finalizeOutput();
};

bool sendRawString(Connection *c, std::string s);
