typedef union {
  unsigned short port;
  char * hostname;
  MC_NetworkNode * node_ptr;
} MRN_YYSTYPE;
#define	PORT	257
#define	HOSTNAME	258
#define	COLON	259
#define	SEMI	260
#define	ARROW	261


extern MRN_YYSTYPE mrn_yylval;
