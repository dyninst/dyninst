typedef union {
  unsigned short port;
  char * hostname;
  MC_NetworkNode * node_ptr;
} YYSTYPE;
#define	PORT	258
#define	HOSTNAME	259
#define	COLON	260
#define	SEMI	261
#define	ARROW	262


extern YYSTYPE yylval;
