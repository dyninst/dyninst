#ifndef __utils_h
#define __utils_h 1

#include <vector>
#include <string>

#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

class Line{
 private:
  std::vector <std::string> words;
 public:
  Line(const char *);
  std::string get_NextWord();
  std::string get_Word(int);
  int get_NumWords();
};

inline unsigned int uintHash(const unsigned int &val) {
  return val;
}

inline unsigned intHash(const int &val) {
  return val;
}

int connect_to_host(int *sock_in, const char * hostname, unsigned short port);
int bind_to_port(int *sock_in, unsigned short *port_in);
int get_socket_connection(int bound_socket);
int get_socket_peer(int connected_socket, char **hostname, unsigned short * port);
unsigned short get_port_from_socket(int sock);
const std::string getHostName(const std::string hostname="");    // e.g. "foo"
const std::string getDomainName(const std::string hostname="");  // "bar.net"
const std::string getNetworkName(const std::string hostname=""); // "foo.bar.net"
const std::string getNetworkAddr(const std::string hostname=""); // "127.0.0.1"

int create_Process(const std::string &remote_shell,
                   const std::string &hostName, const std::string &userName,
                   const std::string &command, const std::vector<std::string> &arg_list);
int remoteCommand(const std::string remoteExecCmd,
                  const std::string hostName, const std::string userName,
                  const std::string command, const std::vector<std::string> &arg_list);
int rshCommand(const std::string &hostName, const std::string &userName, 
               const std::string &command, const std::vector<std::string> &arg_list);
int execCmd(const std::string command, const std::vector<std::string> &args);

extern unsigned int _count;

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

#endif /* __utils_h */
