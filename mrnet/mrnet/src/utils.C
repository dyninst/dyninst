#include <errno.h>
#include <string.h>


#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <stdio.h>

#define SA struct sockaddr

unsigned int _count;
Line::Line(const char * buf)
{
  unsigned int i;
  char * begin, *cur;
  begin = new char[MAX_LINELENGTH];

  begin = strdup(buf);

  for(cur = strtok(begin, " \t\n"); cur != NULL;
      cur = strtok(NULL, " \t\n")){
      words += string(cur);
  }
  free(begin);

  mc_printf((stderr, "Number of words: %d\n", words.size()));
  for(i=0; i<words.size(); i++){
    mc_printf((stderr, "%s ", words[i].c_str()));
  }
  mc_printf((stderr, "\n"));
  return;
}

int Line::get_NumWords()
{
  return words.size();
}

string Line::get_Word(int i)
{
  assert( (unsigned int)i < words.size() );
  return words[i];
}

int connect_to_host(int *sock_in, const char * hostname, unsigned short port)
{
  int sock=*sock_in;
  struct sockaddr_in server_addr;
  struct hostent * server_hostent;

  mc_printf((stderr, "Connecting to %s:%d ...\n", hostname, port));

  if(sock == 0){
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1){
      mc_printf((stderr, "socket() failed\n"));
      perror("socket()");
      return -1;
    }
    mc_printf((stderr, "socket() succeeded\n"));
  }

  server_hostent = gethostbyname(hostname);

  if(server_hostent == NULL){
    perror("gethostbyname()");
    return -1;
  }
  
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr, server_hostent->h_addr_list[0],
         sizeof(struct in_addr));

  if( connect(sock, (SA *)&server_addr, sizeof(server_addr)) == -1){
    perror("connect()");
    return -1;
  }
  perror("connect()");

  char *_hostname; unsigned short _p;
  if(get_socket_peer(sock, &_hostname, &_p) == -1){
    mc_printf((stderr, ""));
    perror("get_socket_peer()");
  }
  else{
    mc_printf((stderr, "really connected to %s:%d\n", _hostname, _p));
  }

  mc_printf((stderr, "Connect_to_host() returning %d\n", sock));
  *sock_in = sock;
  return 0;
}

int bind_to_port(int *sock_in, unsigned short *port_in)
{
  int sock=*sock_in;
  unsigned short port=*port_in;
  struct sockaddr_in local_addr;

  mc_printf((stderr, "In bind_to_port(sock:%d, port:%d)\n", sock, port));

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1){
    perror("socket()");
    return -1;
  }

  bzero(&local_addr, sizeof(local_addr));
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

  mc_printf((stderr, "calling listen()\n"));
  if(listen(sock, 1) == -1){
    mc_printf((stderr, ""));
    perror("listen()");
    return -1;
  }
  mc_printf((stderr, "listen() succeeded()\n"));

  *sock_in = sock;
  *port_in = port;
  mc_printf((stderr, "bind_to_port() returning sock:%d, port:%d\n", sock, port));
  return 0;
}

int get_socket_connection(int bound_socket)
{
  int connected_socket;

  mc_printf((stderr, "get_connection(sock:%d). Calling accept()\n", bound_socket));

  connected_socket = accept(bound_socket, NULL, NULL);
  if(connected_socket == -1){
    mc_printf((stderr, ""));
    perror("accept()");
    return -1;
  }

  mc_printf((stderr, "accept() succeeded. get_connection() returning sock:%d\n", connected_socket));
  return connected_socket;
}

int get_socket_peer(int connected_socket, char **hostname, unsigned short * port)
{
  struct sockaddr_in peer_addr;
  unsigned int peer_addrlen=sizeof(peer_addr);
  char buf[256];

  mc_printf((stderr, "In get_socket_peer()\n"));

  if( getpeername(connected_socket, (struct sockaddr *)(&peer_addr), &peer_addrlen) == -1){
    mc_printf((stderr,""));
    perror("getpeername()");
    return -1;
  }

  if(inet_ntop(AF_INET, &peer_addr.sin_addr, buf, sizeof(buf)) == NULL){
    mc_printf((stderr, ""));
    perror("inet_ntop()");
    return -1;
  }

  *port = ntohs(peer_addr.sin_port);
  *hostname = strdup(buf);
  mc_printf((stderr, "get_socket_peer() returning %s:%d\n", *hostname, *port));
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

  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = IP;

  if( connect(sock, (SA *)&server_addr, sizeof(server_addr)) == -1){
    perror("connect()");
    return -1;
  }

  return sock;
}

// get the current (localhost) machine name, e.g. "grilled"
const string getHostName()
{
  char hostname[256];

  if (gethostname(hostname,sizeof(hostname)) == -1) {
    mc_printf((stderr, ""));
    perror("gethostname()");
    return string("");
  }
    
  return string(hostname);
}

// get the network domain name from the given hostname (default=localhost)
// e.g. "grilled.cs.wisc.edu" -> "cs.wisc.edu"
const string getDomainName (const string hostname)
{
  unsigned index=0;
  string networkname;

  if (hostname.length() == 0){
    networkname = getNetworkName();
  }
  else{
    networkname = hostname;
  }

  while (index < networkname.length() && networkname[index]!='.')
    index++;

  if (index == networkname.length()) {
    mc_printf((stderr, "cannot determine network name for %s\n",
               hostname.c_str()));
    return string("");
  }
  else {
    const string simplename = networkname.substr(0,index);
    const string domain = networkname.substr(index+1,networkname.length());
    if (simplename.regexEquiv("[0-9]*",false)) {
      mc_printf((stderr, "Got invalid simplename: %s\n", simplename.c_str()));
      return ("");
    }
    else {
      return (domain);
    }
  }
}

// get the fully-qualified network name for given hostname (default=localhost)
// e.g. "grilled" -> "grilled.cs.wisc.edu"
const string getNetworkName (const string hostname)
{
  struct hostent *hp;
  char name[256];

  if (!hostname[0]) { // find this machine's hostname
    const string thishostname=getHostName();
    strcpy(name,thishostname.c_str());
  }
  else{
    strcpy(name, hostname.c_str());
  }

  hp = gethostbyname(name);
  if (hp == NULL) {
    mc_printf((stderr, "Host information not found for %s\n", name));
    return string("");
  }

  string networkname = string(hp->h_name);

  // check that networkname is fully-qualified with domain information
  if (getDomainName(networkname) == ""){
    networkname = getNetworkAddr(networkname); // default to IP address
  }

  return (networkname);
}

// get the network IP address for given hostname (default=localhost)
// e.g. "grilled" -> "128.105.166.40"
const string getNetworkAddr (const string hostname)
{
  struct hostent *hp;

  char name[256];

  if (!hostname[0]) { // find this machine's hostname
    const string thishostname=getHostName();
    strcpy(name,thishostname.c_str());
  }
  else{
    strcpy(name,hostname.c_str());
  }

  hp = gethostbyname(name);
  if (hp == NULL) {
    mc_printf((stderr, "Host information not found for %s", name));
    return string("");
  }

  struct in_addr in;
  memcpy(&in.s_addr, *(hp->h_addr_list), sizeof (in.s_addr));

  return string(inet_ntoa(in));
}

int create_Process(const string &remote_shell,
                   const string &hostName, const string &userName,
                   const string &command, const vector<string> &arg_list)
{
  if((hostName == "") ||
     (hostName == "localhost") ||
     (getNetworkName(hostName) == getNetworkName()) ||
     (getNetworkAddr(hostName) == getNetworkAddr()) ){
    mc_printf((stderr, "calling execCmd()\n"));
    return execCmd(command, arg_list);
  }
  else if (remote_shell.length() > 0){
    mc_printf((stderr, "calling remoteCmd()\n"));
    return remoteCommand(remote_shell, hostName, userName, command, arg_list);
  }
  else{
    mc_printf((stderr, "calling rshCmd()\n"));
    return rshCommand(hostName, userName, command, arg_list);
  }
}

// directly exec the command (local).
int execCmd(const string command, const vector<string> &args)
{
  int ret, i;
  int arglist_len = args.size();
  char **arglist = new char*[arglist_len+2];
  char *cmd = strdup(command.c_str());

  mc_printf((stderr, "In execCmd(%s) with %d args\n", cmd, arglist_len));

  arglist[0] = strdup(basename(cmd));
  free(cmd); //basename may modify!
  arglist[arglist_len+1] = NULL;
  for (int i=0; i<arglist_len; ++i){
    arglist[i+1] = strdup(args[i].c_str());
  }

  ret = fork();
  if (ret == 0) {
    mc_printf((stderr, "Child calling execvp:"));
    for(i=0; arglist[i] != NULL; i++){
      _fprintf((stderr, "%s ", arglist[i]));
    }
    _fprintf((stderr, "\n"));

    execvp(command.c_str(), arglist);
    perror("exec()");
    exit(-1);
  }

  return (ret == -1 ? -1 : 0);
}

// Execute 'command' on a remote machine using 'remote_shell' (which can 
// include arguments) passing an argument list of 'arg_list'

int remoteCommand(const string remoteExecCmd,
                  const string hostName, const string userName,
                  const string command, const vector<string> &arg_list)
{
  unsigned int i;
  vector<string> remoteExecArgList;
  vector<string> tmp;
  string cmd;

#if defined(DEFAULT_RUNAUTH_COMMAND)
  //might be necessary to call runauth to pass credentials
  cmd = DEFAULT_RUNAUTH_COMMAND;
  remoteExecArgList += remoteExecCmd;
#else
  cmd = remoteExecCmd;
#endif

  // add the hostname and username to arglist
  remoteExecArgList += hostName;
  if( userName.length() > 0 ) {
    remoteExecArgList += string("-l");
    remoteExecArgList += userName;
  }

  // add remote command and its arguments
  remoteExecArgList += command;
  for( i = 0; i < arg_list.size(); i++ ){
    remoteExecArgList += arg_list[i];
  }
  //remoteExecArgList += "-l0";

  // execute the command
  mc_printf((stderr, "Calling execCmd: %s ", cmd.c_str()));
  for(i=0; i<remoteExecArgList.size(); i++){
    _fprintf((stderr, "%s ", remoteExecArgList[i].c_str()));
  }
  _fprintf((stderr, "\n"));

  if( execCmd(cmd, remoteExecArgList ) == -1){
    mc_printf((stderr, "execCmd() failed\n"));
    return -1;
  }
  mc_printf((stderr, "execCmd() succeeded\n"));

  return 0;
}

// use rsh to get a remote process started.

int rshCommand(const string &hostName, const string &userName, 
               const string &command, const vector<string> &arg_list)
{
  // ensure we know the user's desired rsh command
  string rshCmd;
  const char* rsh = getenv( RSH_COMMAND_ENV );
  if( rsh == NULL ) {
    rshCmd = DEFAULT_RSH_COMMAND;
  }
  else{
    rshCmd = rsh;
  }

  mc_printf((stderr, "In rshCmd(). Calling remoteCmd(%s, %s, %s)\n",
	     rshCmd.c_str(), hostName.c_str(), command.c_str()));
  return remoteCommand( rshCmd, hostName, userName, command, arg_list);
}

