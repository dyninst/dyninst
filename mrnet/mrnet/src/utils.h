#ifndef utils_h
#define utils_h 1

#include <vector>
#include <string>

#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int createProcess(const std::string &remote_shell,
                  const std::string &hostName,
                  const std::string &userName,
                  const std::string &command,
                  const std::vector<std::string> &arg_list);

int execCmd(const std::string command,
            const std::vector<std::string> &args);

int remoteCommand(const std::string remoteExecCmd,
                  const std::string hostName,
                  const std::string userName,
                  const std::string command,
                  const std::vector<std::string> &arg_list);

int rshCommand(const std::string &hostName,
               const std::string &userName, 
               const std::string &command,
               const std::vector<std::string> &arg_list);

int connectHost(int *sock_in,
                const std::string &hostname,
                unsigned short port);

int bindPort(int *sock_in, unsigned short *port_in);

int getSocketConnection(int bound_socket);

int getSocketPeer(int connected_socket,
                  std::string &hostname,
                  unsigned short * port);

int getPortFromSocket(int sock, unsigned short *port);

int getHostName(std::string & out_hostname,
                const std::string& in_hostname="");      // e.g. "foo"
int getDomainName(std::string & out_hostname,
                  const std::string& in_hostname="");    // e.g. "bar.net"
int getNetworkName(std::string & out_hostname,
                   const std::string& in_hostname="");   // e.g. "foo.bar.net"
int getNetworkAddr(std::string &ipaddr_str,
                   const std::string hostname="");       // "127.0.0.1"

//extern unsigned int _count;

#include <libgen.h>

struct ltstr
{
  bool operator()(std::string s1, std::string s2) const
  {
    return (s1 < s2);
  }
};

extern pthread_key_t tsd_key;
class tsd_t{
 public:
  pthread_t thread_id;
  const char * thread_name;
};

//#define _fprintf(X)  ;
//#define _perror(X) ;
#define _fprintf(X) fprintf X ;
#define _perror(X) perror(X); 
#define MCFL  __FILE__,__LINE__  //used to call mrn_printf(MCFL, ...)
int mrn_printf(int level, const char * file, int line, FILE * fp, const char * format, ...);

#endif /* utils_h */
