#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#define SA struct sockaddr

unsigned int _count;
pthread_key_t tsd_key;

Line::Line(const char * buf)
{
  unsigned int i;
  char * begin, *cur;
  begin = new char[MAX_LINELENGTH];

  begin = strdup(buf);

  for(cur = strtok(begin, " \t\n"); cur != NULL;
      cur = strtok(NULL, " \t\n")){
      words.push_back(std::string(cur));
  }
  free(begin);

  mrn_printf(3, MCFL, stderr, "Number of words: %d\n", words.size());
  for(i=0; i<words.size(); i++){
    mrn_printf(3, MCFL, stderr, "%s ", words[i].c_str());
  }
  mrn_printf(3, MCFL, stderr, "\n");
  return;
}

int Line::get_NumWords()
{
  return words.size();
}

std::string Line::get_Word(int i)
{
  assert( (unsigned int)i < words.size() );
  return words[i];
}

int connect_to_host(int *sock_in, const char * hostname, unsigned short port)
{
  int sock=*sock_in;
  struct sockaddr_in server_addr;
  struct hostent * server_hostent;

  mrn_printf(3, MCFL, stderr, "In connect_to_host(%s:%d) ...\n", hostname, port);

  if(sock == 0){
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1){
      mrn_printf(1, MCFL, stderr, "socket() failed\n");
      perror("socket()");
      return -1;
    }
  }

  server_hostent = gethostbyname(hostname);

  if(server_hostent == NULL){
    perror("gethostbyname()");
    return -1;
  }
  
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr, server_hostent->h_addr_list[0],
         sizeof(struct in_addr));

  if( connect(sock, (SA *)&server_addr, sizeof(server_addr)) == -1){
    perror("connect()");
    return -1;
  }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( sock,
                                IPPROTO_TCP,
                                TCP_NODELAY,
                                &optVal,
                                sizeof(optVal) );
    if( ssoret == -1 )
    {
        mrn_printf(1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

  //char *_hostname; unsigned short _p;
  //if(get_socket_peer(sock, &_hostname, &_p) == -1){
    //mrn_printf(MCFL, stderr, "%s", ""));
    //perror("get_socket_peer()");
  //}
  //else{
    //mrn_printf(MCFL, stderr, "really connected to %s:%d\n", _hostname, _p));
  //}

  mrn_printf(3, MCFL, stderr, "Leaving Connect_to_host(). Returning sock: %d\n", sock);
  *sock_in = sock;
  return 0;
}

int bind_to_port(int *sock_in, unsigned short *port_in)
{
  int sock=*sock_in;
  unsigned short port=*port_in;
  struct sockaddr_in local_addr;

  mrn_printf(3, MCFL, stderr, "In bind_to_port(sock:%d, port:%d)\n", sock, port);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1){
    perror("socket()");
    return -1;
  }

  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(port != 0){
    local_addr.sin_port = htons(port);
    if( bind(sock, (SA *)&local_addr, sizeof(local_addr)) == -1){
      perror("bind()");
      return -1;
    }
  }
  else{
    port = 7000;
    local_addr.sin_port = htons(port);
    while( bind(sock, (SA *)&local_addr, sizeof(local_addr)) == -1){
      if(errno == EADDRINUSE){
        local_addr.sin_port = htons(++port);
	continue;
      }
      else{
        perror("bind()");
        return -1;
      }
    }
    
  }

  if(listen(sock, 1) == -1){
    mrn_printf(1, MCFL, stderr, "%s", "");
    perror("listen()");
    return -1;
  }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( sock,
                                IPPROTO_TCP,
                                TCP_NODELAY,
                                &optVal,
                                sizeof(optVal) );
    if( ssoret == -1 )
    {
        mrn_printf(1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

  *sock_in = sock;
  *port_in = port;
  mrn_printf(3, MCFL, stderr, "Leaving bind_to_port(). Returning sock:%d, port:%d\n",
             sock, port);
  return 0;
}

int get_socket_connection(int bound_socket)
{
  int connected_socket;

  mrn_printf(3, MCFL, stderr, "In get_connection(sock:%d).\n", bound_socket);

  connected_socket = accept(bound_socket, NULL, NULL);
  if(connected_socket == -1){
    mrn_printf(1, MCFL, stderr, "%s", "");
    perror("accept()");
    return -1;
  }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( connected_socket,
                                IPPROTO_TCP,
                                TCP_NODELAY,
                                &optVal,
                                sizeof(optVal) );
    if( ssoret == -1 )
    {
        mrn_printf(1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

  mrn_printf(3, MCFL, stderr, "Leaving get_connection(). Returning sock:%d\n",
             connected_socket);
  return connected_socket;
}

int get_socket_peer(int connected_socket, char **hostname, unsigned short * port)
{
  struct sockaddr_in peer_addr;
  socklen_t peer_addrlen=sizeof(peer_addr);
  char buf[256];

  mrn_printf(3, MCFL, stderr, "In get_socket_peer()\n");

  if( getpeername(connected_socket, (struct sockaddr *)(&peer_addr), &peer_addrlen) == -1){
    mrn_printf(1, MCFL, stderr,"%s", "");
    perror("getpeername()");
    return -1;
  }

  if(inet_ntop(AF_INET, &peer_addr.sin_addr, buf, sizeof(buf)) == NULL){
    mrn_printf(1, MCFL, stderr, "%s", "");
    perror("inet_ntop()");
    return -1;
  }

  *port = ntohs(peer_addr.sin_port);
  *hostname = strdup(buf);
  mrn_printf(3, MCFL, stderr, "Leaving get_socket_peer(). Returning %s:%d\n",
             *hostname, *port);
  return 0;
}

unsigned short get_port_from_socket(int sock){
  struct sockaddr_in local_addr;
  socklen_t sockaddr_len = sizeof(local_addr);


  if( getsockname(sock, (SA *)&local_addr, &sockaddr_len) == -1){
    perror("getsockname");
    return 0;
  }

  return local_addr.sin_port;
}

int get_IP_from_socket(int sock){
  struct sockaddr_in local_addr;
  socklen_t sockaddr_len = sizeof(local_addr);


  if( getsockname(sock, (SA *)&local_addr, &sockaddr_len) == -1){
    perror("getsockname");
    return -1;
  }

  return local_addr.sin_addr.s_addr;
}

int get_IP_from_name(char *name){
  struct hostent * _hostent;

  _hostent = gethostbyname(name);

  if(_hostent == NULL){
    perror("gethostbyname()");
    return -1;
  }
  
  return ((struct in_addr *)(_hostent->h_addr_list[0]))->s_addr;
}

int get_local_IP(){
  struct hostent *hptr;
  struct utsname myname;

  if (uname(&myname) < 0 ){
    perror("uname()");
    return -1;
  }

  if ( (hptr = gethostbyname(myname.nodename)) == NULL){
    perror("gethostbyname()");
    return -1;
  }

  return ((struct in_addr *)(hptr->h_addr_list[0]))->s_addr;
}
  
int connect_socket_by_IP(int IP, short port){
  struct sockaddr_in server_addr;
  int sock;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  if(sock == -1){
    perror("socket()");
    return -1;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = IP;

  if( connect(sock, (SA *)&server_addr, sizeof(server_addr)) == -1){
    perror("connect()");
    return -1;
  }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( sock,
                                IPPROTO_TCP,
                                TCP_NODELAY,
                                &optVal,
                                sizeof(optVal) );
    if( ssoret == -1 )
    {
        mrn_printf(1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

  return sock;
}

// get the current (localhost) machine name, e.g. "grilled"
const std::string getHostName(const std::string _hostname)
{
  char hostname[256];
  std::string hostname_str;

  if(_hostname == ""){
    if (gethostname(hostname,sizeof(hostname)) == -1) {
      mrn_printf(1, MCFL, stderr, "%s", "");
      perror("gethostname()");
      return std::string("");
    }
    hostname_str=hostname;
  }
  else{
    hostname_str=_hostname;
  }

  int idx=hostname_str.find('.');
  if(idx != -1){
    return hostname_str.substr(0, idx);
  }
  else{
    return hostname_str;
  }
}

// get the network domain name from the given hostname (default=localhost)
// e.g. "grilled.cs.wisc.edu" -> "cs.wisc.edu"
const std::string getDomainName (const std::string hostname)
{
  unsigned index=0;
  std::string networkname;

  if (hostname.length() == 0){
    networkname = getNetworkName();
  }
  else{
    networkname = hostname;
  }

  while (index < networkname.length() && networkname[index]!='.')
    index++;

  if (index == networkname.length()) {
    mrn_printf(1, MCFL, stderr, "cannot determine network name for %s\n",
               hostname.c_str());
    return std::string("");
  }
  else {
    const std::string simplename = networkname.substr(0,index);
    const std::string domain = networkname.substr(index+1,networkname.length());
    //why can't a hostname contain numerals?
    //if (simplename.regexEquiv("[0-9]*",false)) {
      //mrn_printf(MCFL, stderr, "Got invalid simplename: %s\n", simplename.c_str());
      //return ("");
    //}
    //else {
      return (domain);
    //}
  }
}

// get the fully-qualified network name for given hostname (default=localhost)
// e.g. "grilled" -> "grilled.cs.wisc.edu"
const std::string getNetworkName (const std::string hostname)
{
  struct hostent *hp;
  char name[256];

  if (!hostname[0]) { // find this machine's hostname
    const std::string thishostname=getHostName();
    strcpy(name,thishostname.c_str());
  }
  else{
    strcpy(name, hostname.c_str());
  }

  hp = gethostbyname(name);
  if (hp == NULL) {
    mrn_printf(3, MCFL, stderr, "Host information not found for %s\n", name);
    return std::string("");
  }

  std::string networkname = std::string(hp->h_name);

  // check that networkname is fully-qualified with domain information
  if (getDomainName(networkname) == ""){
    networkname = getNetworkAddr(networkname); // default to IP address
  }

  return networkname;
}

// get the network IP address for given hostname (default=localhost)
// e.g. "grilled" -> "128.105.166.40"
const std::string getNetworkAddr (const std::string hostname)
{
  struct hostent *hp;

  char name[256];

  if (!hostname[0]) { // find this machine's hostname
    const std::string thishostname=getHostName();
    strcpy(name,thishostname.c_str());
  }
  else{
    strcpy(name,hostname.c_str());
  }

  hp = gethostbyname(name);
  if (hp == NULL) {
    mrn_printf(3, MCFL, stderr, "Host information not found for %s", name);
    return std::string("");
  }

  struct in_addr in;
  memcpy(&in.s_addr, *(hp->h_addr_list), sizeof (in.s_addr));

  return std::string(inet_ntoa(in));
}

int create_Process(const std::string &remote_shell,
                   const std::string &hostName, const std::string &userName,
                   const std::string &command, const std::vector<std::string> &arg_list)
{
  if((hostName == "") ||
     (hostName == "localhost") ||
     (getNetworkName(hostName) == getNetworkName()) ||
     (getNetworkAddr(hostName) == getNetworkAddr()) ){
    return execCmd(command, arg_list);
  }
  else if (remote_shell.length() > 0){
    return remoteCommand(remote_shell, hostName, userName, command, arg_list);
  }
  else{
    return rshCommand(hostName, userName, command, arg_list);
  }
}

// directly exec the command (local).
int execCmd(const std::string command, const std::vector<std::string> &args)
{
  int ret, i;
  int arglist_len = args.size();
  char **arglist = new char*[arglist_len+2];
  char *cmd = strdup(command.c_str());

  mrn_printf(3, MCFL, stderr, "In execCmd(%s) with %d args\n", cmd, arglist_len);

  arglist[0] = strdup(basename(cmd));
  free(cmd); //basename may modify!
  arglist[arglist_len+1] = NULL;
  for (i=0; i<arglist_len; ++i){
    arglist[i+1] = strdup(args[i].c_str());
  }

  ret = fork();
  if (ret == 0) {
    mrn_printf(3, MCFL, stderr, "Forked child calling execvp:");
    for(i=0; arglist[i] != NULL; i++){
      mrn_printf(3, 0,0, stderr, "%s ", arglist[i]);
    }
    mrn_printf(3, 0,0, stderr, "\n");

    execvp(command.c_str(), arglist);
    perror("exec()");
    exit(-1);
  }

  return (ret == -1 ? -1 : 0);
}

// Execute 'command' on a remote machine using 'remote_shell' (which can 
// include arguments) passing an argument list of 'arg_list'

int remoteCommand(const std::string remoteExecCmd,
                  const std::string hostName, const std::string userName,
                  const std::string command, const std::vector<std::string> &arg_list)
{
  unsigned int i;
  std::vector<std::string> remoteExecArgList;
  std::vector<std::string> tmp;
  std::string cmd;

  mrn_printf(3, MCFL, stderr, "In remoteCommand()\n");
#if defined(DEFAULT_RUNAUTH_COMMAND)
  //might be necessary to call runauth to pass credentials
  cmd = DEFAULT_RUNAUTH_COMMAND;
  remoteExecArgList.push_back(remoteExecCmd);
#else
  cmd = remoteExecCmd;
#endif

  // add the hostname and username to arglist
  remoteExecArgList.push_back(hostName);
  if( userName.length() > 0 ) {
    remoteExecArgList.push_back(std::string("-l"));
    remoteExecArgList.push_back(userName);
  }

  // add remote command and its arguments
  remoteExecArgList.push_back(command);
  for( i = 0; i < arg_list.size(); i++ ){
    remoteExecArgList.push_back(arg_list[i]);
  }
  //remoteExecArgList.push_back("-l0");

  // execute the command
  mrn_printf(3, MCFL, stderr, "Calling execCmd: %s ", cmd.c_str());
  for(i=0; i<remoteExecArgList.size(); i++){
    mrn_printf(3, 0,0, stderr, "%s ", remoteExecArgList[i].c_str());
  }
  mrn_printf(3, 0,0, stderr, "\n");

  if( execCmd(cmd, remoteExecArgList ) == -1){
    mrn_printf(1, MCFL, stderr, "execCmd() failed\n");
    return -1;
  }

  mrn_printf(3, MCFL, stderr, "Leaving remoteCommand()\n");
  return 0;
}

// use rsh to get a remote process started.

int rshCommand(const std::string &hostName, const std::string &userName, 
               const std::string &command, const std::vector<std::string> &arg_list)
{
  // ensure we know the user's desired rsh command
  std::string rshCmd;
  const char* rsh = getenv( RSH_COMMAND_ENV );
  if( rsh == NULL ) {
    rshCmd = DEFAULT_RSH_COMMAND;
  }
  else{
    rshCmd = rsh;
  }

  mrn_printf(3, MCFL, stderr, "In rshCmd(). Calling remoteCmd(%s, %s, %s)\n",
	     rshCmd.c_str(), hostName.c_str(), command.c_str());
  return remoteCommand( rshCmd, hostName, userName, command, arg_list);
}

int mrn_printf(int level, const char * file, int line, FILE * fp,
	      const char * format, ...)
{
  int retval;
  va_list arglist;

  if( level > OUTPUT_LEVEL ){
    return 0;
  }

  if( file ){
    // basename modifies 1st arg, so copy
    char tmp_filename[256];
    strncpy(tmp_filename, file, sizeof(tmp_filename));
    
    // get thread name
    const char* thread_name = NULL;
    tsd_t * tsd = (tsd_t*)pthread_getspecific(tsd_key);
    if( tsd != NULL )
      {
	thread_name = tsd->thread_name;
      }
    
    fprintf(fp, "%s:%s:%d: ",
	    (thread_name != NULL) ? thread_name : "<noname (tsd NULL)>",
	    basename(tmp_filename),
	    line);
  }

  va_start(arglist, format);
  retval = vfprintf(fp, format, arglist);
  va_end(arglist);

  return retval;
}
