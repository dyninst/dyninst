.. _`sec:ntHeaders.h`:

ntHeaders.h
###########

.. cpp:namespace:: windows

.. Attention:: Windows support is experimental

.. code:: c

  #define PDSOCKET_ERROR (-1)

.. cpp:type:: int pid_t
.. cpp:type:: int key_t
.. cpp:type:: unsigned int socklen_t
.. cpp:type:: void (*P_sig_handler)(int)

.. cpp:function:: int P_getopt(int argc, char *argv[], const char *optstring)
.. cpp:function:: void P_abort (void) 
.. cpp:function:: int P_close (int FILEDES) 
.. cpp:function:: int P__dup2(int OLD, int NEW) 
.. cpp:function:: void P__exit (int STATUS) 
.. cpp:function:: FILE * P_fdopen (int FILEDES, const char *OPENTYPE) 
.. cpp:function:: FILE * P_fopen (const char *FILENAME, const char *OPENTYPE) 
.. cpp:function:: int P_fstat (int FILEDES, struct stat *BUF) 
.. cpp:function:: int P_getpid () 
.. cpp:function:: off_t P_lseek (int FILEDES, off_t OFFSET, int WHENCE) 
.. cpp:function:: int P__open(const char *FILENAME, int FLAGS, int MODE) 
.. cpp:function:: int P__pclose (FILE *STREAM) 
.. cpp:function:: FILE *P__popen (const char *COMMAND, const char *MODE) 
.. cpp:function:: size_t P_write (int FILEDES, const void *BUFFER, size_t SIZE) 
.. cpp:function:: int P_chdir(const char *path) 
.. cpp:function:: int P_putenv(char *str) 
.. cpp:function:: void P_exit (int STATUS) 
.. cpp:function:: int P_fflush(FILE *stream) 
.. cpp:function:: char * P_fgets (char *S, int COUNT, FILE *STREAM) 
.. cpp:function:: void * P_malloc (size_t SIZE) 
.. cpp:function:: void * P_memcpy (void *A1, const void *A2, size_t SIZE) 
.. cpp:function:: void * P_memset (void *BLOCK, int C, size_t SIZE) 
.. cpp:function:: void P_perror (const char *MESSAGE) 
.. cpp:function:: P_sig_handler P_signal (int SIGNUM, P_sig_handler ACTION) 
.. cpp:function:: char * P_strcat (char *TO, const char *FROM) 
.. cpp:function:: const char * P_strchr (const char *STRING, int C) 
.. cpp:function:: char * P_strchr (char *STRING, int C) 
.. cpp:function:: int P_getpagesize()
.. cpp:function:: int P_strcmp (const char *S1, const char *S2) 
.. cpp:function:: char * P_strcpy (char *TO, const char *FROM) 
.. cpp:function:: char *P_strdup(const char *S) 
.. cpp:function:: size_t P_strlen (const char *S) 
.. cpp:function:: char * P_strncat (char *TO, const char *FROM, size_t SIZE) 
.. cpp:function:: int P_strncmp (const char *S1, const char *S2, size_t SIZE) 
.. cpp:function:: char * P_strncpy (char *TO, const char *FROM, size_t SIZE) 
.. cpp:function:: const char * P_strrchr (const char *STRING, int C) 
.. cpp:function:: char * P_strrchr (char *STRING, int C) 
.. cpp:function:: const char * P_strstr (const char *HAYSTACK, const char *NEEDLE) 
.. cpp:function:: char * P_strstr (char *HAYSTACK, const char *NEEDLE) 
.. cpp:function:: double P_strtod (const char *STRING, char **TAILPTR) 
.. cpp:function:: char * P_strtok (char *NEWSTRING, const char *DELIMITERS) 
.. cpp:function:: long int P_strtol (const char *STRING, char **TAILPTR, int BASE) 
.. cpp:function:: unsigned long int P_strtoul(const char *STRING, char **TAILPTR, int BASE) 
.. cpp:function:: int P_accept (int SOCK, struct sockaddr *ADDR, size_t *LENGTH_PTR) 
.. cpp:function:: int P_bind(int socket, struct sockaddr *addr, size_t len) 
.. cpp:function:: int P_connect(int socket, struct sockaddr *addr, size_t len) 
.. cpp:function:: struct hostent * P_gethostbyname (const char *NAME) 
.. cpp:function:: struct servent * P_getservbyname (const char *NAME, const char *PROTO) 
.. cpp:function:: int P_getsockname (int SOCKET, struct sockaddr *ADDR, size_t *LENGTH_PTR) 
.. cpp:function:: int P_listen (int socket, unsigned int n) 
.. cpp:function:: int P_socket (int NAMESPACE, int STYLE, int PROTOCOL) 
.. cpp:function:: int P_select(int wid, fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tm) 
.. cpp:function:: int P_recv(int s, void *buf, size_t len, int flags) 
.. cpp:function:: int P_mkdir(const char *pathname, int) 
.. cpp:function:: int P_unlink(const char *pathname) 
.. cpp:function:: std::string P_cplus_demangle( const std::string &symbol, bool includeTypes = false )
