extern int target_flags;
extern int hard_regno_nregs[];
extern int hard_regno_mode_ok[64 ];
extern int leaf_function;
enum reg_class { NO_REGS, GENERAL_REGS, FP_REGS, ALL_REGS, LIM_REG_CLASSES };
extern char leaf_reg_remap[];
extern char leaf_reg_backmap[];
extern struct rtx_def *sparc_compare_op0, *sparc_compare_op1;
extern struct rtx_def *gen_compare_reg ();
extern int actual_fsize;
extern int apparent_fsize;
extern int current_function_calls_alloca;
extern int current_function_outgoing_args_size;
extern union tree_node *current_function_decl;
extern struct rtx_def *legitimize_pic_address ();
extern char *singlemove_string ();
extern char *output_move_double ();
extern char *output_move_quad ();
extern char *output_fp_move_double ();
extern char *output_fp_move_quad ();
extern char *output_block_move ();
extern char *output_scc_insn ();
extern char *output_cbranch ();
extern char *output_return ();
extern int flag_pic;
typedef unsigned int	size_t;		 
extern	struct	_iobuf {
	int	_cnt;
	char	*_ptr;
	char	*_base;
	int	_bufsiz;
	short	_flag;
	short	_file;
} _iob[3 ];
typedef	struct _iobuf	FILE;
typedef	long	fpos_t;
extern int 	getc( FILE *__stream );
extern int	getchar( void );
extern int	putc( int __c, FILE *__stream);
extern int	putchar( int __c);
extern int	feof( FILE *__stream );
extern int	ferror( FILE *__stream );
extern int	fileno( FILE *__stream );
extern int	_filbuf( FILE *p);
extern int	_flsbuf( unsigned char x , FILE *p);
typedef char *va_list;
extern void	clearerr( FILE *__stream); 
extern int	fclose( FILE *__stream );
extern FILE *	fdopen( int __filedes, char *__type );
extern int	fflush( FILE *__stream );
extern int	fgetc( FILE *__stream );
extern int	fgetpos( FILE *__stream, fpos_t *__pos );
extern char *	fgets( char *__s, int __n, FILE *__stream );
extern FILE *	fopen( const char *__filename, const char *__type );
extern int	fprintf( FILE *__stream, const char *__format, ... );
extern int	fputc( int __c, FILE *__stream );
extern int	fputs( const char *__s, FILE *__stream );
extern size_t	fread( void *__ptr, size_t __size,
			size_t __nitems, FILE *__stream ); 
extern FILE *	freopen( const char *__filename, const char *__type,
			FILE *__stream );
extern int	fscanf( FILE *__stream, const char *__format, ... );
extern int	fseek( FILE *__stream, long __offset, int __ptrname );
extern int	fsetpos( FILE *__stream, const fpos_t *__pos );
extern long	ftell( FILE *__stream );
extern size_t	fwrite( const void *__ptr, size_t __size,
			size_t __nitems, FILE *__stream );
extern char *	gets( char *__s );	
extern void	perror( const char *__s );
extern FILE  *	popen(const char *__command, const char *__type );
extern int	printf( const char *__format, ... );	
extern int	puts( const char *__s );	
extern int	remove( const char *__filename );
extern int	rename( const char *__from, const char *__to );
extern void	rewind( FILE *__stream );
extern int	scanf( const char *__format, ... );	
extern void	setbuf( FILE *__stream, char *__buf );
extern int	setvbuf( FILE *__stream, char *__buf,
			int __type, size_t __size );
extern int	sscanf( const char *__s, const char *__format, ... );
extern FILE *	tmpfile( void );	
extern char *	tmpnam( char *__s );
extern int	ungetc( int __c, FILE *__stream );
extern int	vfprintf( FILE *__stream, const char *__format, va_list __ap );
extern int	vprintf( const char *__format, va_list __ap );
extern int	vsprintf( char *__s, const char *__format, va_list __ap);
extern char *	tempnam( const char *__dir, const char *__pfx);
extern int	putw( int __w, FILE *__stream );
extern int	getw(FILE *__stream);
extern int	pclose( FILE *__stream );
extern int	sprintf( char *__s, const char *__format, ... );
extern	unsigned short *_pctype;
extern  unsigned short _ctype__[];
int	isalnum( int __c );
int	isalpha( int __c );
int	isascii( int __c );
int	iscntrl( int __c );
int	isdigit( int __c );
int	isgraph( int __c );
int	islower( int __c );
int	isprint( int __c );
int	ispunct( int __c );
int	isspace( int __c );
int	isupper( int __c );
int	isxdigit( int __c );
int	toascii( int __c );
int	_tolower( int __c );
int	_toupper( int __c );
int	tolower( int __c );
int	toupper( int __c );
extern int errno;
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned int	uint;		 
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		 
typedef	volatile char		v_char;
typedef	volatile short		v_short;
typedef	volatile long		v_long;
typedef	volatile unsigned char	vu_char;
typedef	volatile unsigned short	vu_short;
typedef	volatile unsigned long	vu_long;
typedef
		char	s_char;
typedef	struct	_quad { long val[2]; } quad;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef u_long	gno_t;
typedef short	cnt_t;			 
typedef	long	swblk_t;
typedef long	paddr_t;		 
typedef	long	audit_ID_t;
typedef	short	dev_t;
typedef short	gid_t;			 
typedef	unsigned long	ino_t;
typedef unsigned short	mode_t;		 
typedef short	nlink_t;		 
typedef	int	off_t;
typedef int	pid_t;			 
typedef short	uid_t;			 
typedef int	time_t;
typedef int	clock_t;			 
typedef long	key_t;			 
typedef long	fd_mask;
typedef	struct fd_set {
	fd_mask	fds_bits[(((4096	 )+(( (sizeof(fd_mask) * 8		)	)-1))/( (sizeof(fd_mask) * 8		)	)) ];
} fd_set;
struct	stat
{
	dev_t		st_dev;
	ino_t		st_ino;
	mode_t		st_mode;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	off_t		st_size;
	time_t		st_atime;
	int		st_spare1;
	time_t		st_mtime;
	int		st_spare2;
	time_t		st_ctime;
	int		st_spare3;
	long		st_blksize;
	long		st_blocks;
	unsigned long	st_gennum;
	long		st_spare4;
};
extern	mode_t	umask();
extern	int
	chmod(),
	fstat(),
	mkdir(),
	mkfifo(),
	stat();
struct gen_dir {
		unsigned long	gd_ino;	   
		unsigned short	gd_reclen;  
		unsigned short	gd_namelen;  
		char	gd_name[255  + 1];  
	};
typedef	struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	long	dd_bbase;
	long	dd_entno;
	long	dd_bsize;
	char *	dd_buf;
} DIR;
extern	DIR *opendir();
extern	int closedir();
extern	struct 	gen_dir  *readdir();
extern	long telldir();
extern	void seekdir();
typedef int jmp_buf[10 ];
typedef int sigjmp_buf[10 ];
extern void	longjmp( jmp_buf __env, int __val );
extern int	setjmp( jmp_buf __env );
extern int	sigsetjmp(sigjmp_buf __env, int __savemask);
extern void 	siglongjmp(const sigjmp_buf __env, int __val);
extern char *optarg;
extern int optind;
extern int opterr;
struct option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
};
extern int getopt_loser  ();
extern int getopt_long (int argc, char *const *argv, const char *shortopts,
		        const struct option *longopts, int *longind);
extern int getopt_long_only (int argc, char *const *argv,
			     const char *shortopts,
		             const struct option *longopts, int *longind);
extern int _getopt_internal (int argc, char *const *argv,
			     const char *shortopts,
		             const struct option *longopts, int *longind,
			     int long_only);
extern int errno;
extern char *sys_errlist[];
extern char *version_string;
extern char *getpwd ();
typedef void * pointer_type;
typedef const void * const_pointer_type;
extern   void abort ();
extern int kill ();
extern int creat ();
extern void exit ();
extern pointer_type malloc ();
extern pointer_type realloc ();
extern void free ();
extern int read ();
extern int write ();
extern int close ();
extern int fflush ();
extern int atoi ();
extern int puts ();
extern int fputs ();
extern int fputc ();
extern int link ();
extern int unlink ();
extern int access ();
extern int execvp ();
extern int setjmp ();
extern void longjmp ();
extern char *   strcat ();
extern int      strcmp ();
extern char *   strcpy ();
extern int      strncmp ();
extern char *   strncpy ();
extern char *   rindex ();
static const char * const aux_info_suffix = ".X";
static const char * const save_suffix = ".save";
static const char syscalls_filename[] = "SYSCALLS.c";
static const char * const default_syscalls_dir = "/usr/local/lib" ;
static char * syscalls_absolute_filename;
struct unexpansion_struct {
  const char *expanded;
  const char *contracted;
};
typedef struct unexpansion_struct unexpansion;
static const unexpansion unexpansions[] = {
  { "struct _iobuf", "FILE" },
  { 0, 0 }
};
static const int hash_mask = (	(1 << 9)  - 1);
struct default_include { const char *fname; int cplusplus; } include_defaults[]
  = {
    { "not-needed", 1},
    { "not-needed", 0},
    { "/usr/local/include" , 0},
    { "/usr/include" , 0},
    { 0, 0}
    };
struct string_list
{
  char *name;
  struct string_list *next;
};
struct string_list *directory_list;
struct string_list *exclude_list;
static const char * const other_var_style = "varargs";
static const char *varargs_style_indicator = "va_alist" ;
typedef struct hash_table_entry_struct hash_table_entry;
typedef struct def_dec_info_struct def_dec_info;
typedef struct file_info_struct file_info;
typedef struct f_list_chain_item_struct f_list_chain_item;
struct hash_table_entry_struct {
  hash_table_entry *		hash_next;	 
  const char *			symbol;		 
  union {
    const def_dec_info *	_ddip;
    file_info *			_fip;
  } _info;
};
typedef hash_table_entry hash_table[	(1 << 9) ];
struct file_info_struct {
  const hash_table_entry *	hash_entry;  
  const def_dec_info *		defs_decs;   
  time_t			mtime;       
};
struct f_list_chain_item_struct {
  const f_list_chain_item *	chain_next;	 
  const char *			formals_list;	 
};
struct def_dec_info_struct {
  const def_dec_info *	next_in_file;	 
  file_info *        	file;		 
  int        		line;		 
  const char *		ansi_decl;	 
  hash_table_entry *	hash_entry;	 
  unsigned int        	is_func_def;	 
  const def_dec_info *	next_for_func;	 
  unsigned int		f_list_count;	 
  char			prototyped;	 
  const f_list_chain_item * f_list_chain;	 
  const def_dec_info *	definition;	 
  char	        	is_static;	 
  char			is_implicit;	 
  char			written;	 
};
static const char *pname;
static int errors = 0;
static const char *compiler_file_name = "gcc";
static int version_flag = 0;		 
static int quiet_flag = 0;		 
static int nochange_flag = 0;		 
static int nosave_flag = 0;		 
static int keep_flag = 0;		 
static const char ** compile_params = 0;	 
static int local_flag = 0;		 
static int global_flag = 0;		 
static int cplusplus_flag = 0;		 
static const char* nondefault_syscalls_dir = 0;  
static int input_file_name_index = 0;
static int aux_info_file_name_index = 0;
static int n_base_source_files = 0;
static const char **base_source_filenames;
static int current_aux_info_lineno;
static const char *convert_filename;
static const char *invocation_filename;
static const char *orig_text_base;
static const char *orig_text_limit;
static const char *clean_text_base;
static const char *clean_text_limit;
static const char * clean_read_ptr;
static char *repl_text_base;
static char *repl_text_limit;
static char * repl_write_ptr;
static const char *last_known_line_start;
static int last_known_line_number;
static hash_table filename_primary;
static hash_table function_name_primary;
static jmp_buf source_confusion_recovery;
static char *cwd_buffer;
static const char * saved_clean_read_ptr;
static char * saved_repl_write_ptr;
static const char *shortpath ();
pointer_type
xmalloc (byte_count)
     size_t byte_count;
{
  pointer_type rv;
  rv = malloc (byte_count);
  if (rv == 0 )
    {
      fprintf ((&_iob[2]) , "\n%s: virtual memory exceeded\n", pname);
      exit (1);
      return 0;		 
    }
  else
    return rv;
}
pointer_type
xrealloc (old_space, byte_count)
     pointer_type old_space;
     size_t byte_count;
{
  pointer_type rv;
  rv = realloc (old_space, byte_count);
  if (rv == 0 )
    {
      fprintf ((&_iob[2]) , "\n%s: virtual memory exceeded\n", pname);
      exit (1);
      return 0;		 
    }
  else
    return rv;
}
void
xfree (p)
     const_pointer_type p;
{
  if (p)
    free ((  pointer_type) p);
}
static char *
savestring (input, size)
     const char *input;
     unsigned int size;
{
  char *output = (char *) xmalloc (size + 1);
  strcpy (output, input);
  return output;
}
static char *
savestring2 (input1, size1, input2, size2)
     const char *input1;
     unsigned int size1;
     const char *input2;
     unsigned int size2;
{
  char *output = (char *) xmalloc (size1 + size2 + 1);
  strcpy (output, input1);
  strcpy (&output[size1], input2);
  return output;
}
void
fancy_abort ()
{
  fprintf ((&_iob[2]) , "%s: internal abort\n", pname);
  exit (1);
}
static char *
dupstr (s)
     const char *s;
{
  return strcpy ((char *) xmalloc (strlen (s) + 1), s);
}
static char *
dupnstr (s, n)
     const char *s;
     size_t n;
{
  char *ret_val = strncpy ((char *) xmalloc (n + 1), s, n);
  ret_val[n] = '\0';
  return ret_val;
}
static const char *
substr (s1, s2)
     const char *s1;
     const char *const s2;
{
  for (; *s1 ; s1++)
    {
      const char *p1;
      const char *p2;
      int c;
      for (p1 = s1, p2 = s2; c = *p2; p1++, p2++)
        if (*p1 != c)
          goto outer;
      return s1;
outer:
      ;
    }
  return 0;
}
void
save_pointers ()
{
  saved_clean_read_ptr = clean_read_ptr;
  saved_repl_write_ptr = repl_write_ptr;
}
void
restore_pointers ()
{
  clean_read_ptr = saved_clean_read_ptr;
  repl_write_ptr = saved_repl_write_ptr;
}
static int
is_id_char (ch)
     char ch;
{
  return (((_pctype+1)[ch]&(0001	|0002	|0004	))  || (ch == '_') || (ch == '$'));
}
static void
usage ()
{
  fprintf ((&_iob[2]) , "%s: usage '%s [ -VqfnkNlgC ] [ -B <diname> ] [ filename ... ]'\n",
	   pname, pname);
  exit (1);
}
static int
in_system_include_dir (path)
     const char *path;
{
  struct default_include *p;
  if (path[0] != '/')
    abort ();		 
  for (p = include_defaults; p->fname; p++)
    if (!strncmp (path, p->fname, strlen (p->fname))
	&& path[strlen (p->fname)] == '/')
      return 1;
  return 0;
}
static int
is_syscalls_file (fi_p)
     const file_info *fi_p;
{
  char const *f = fi_p->hash_entry->symbol;
  size_t fl = strlen (f), sysl = sizeof (syscalls_filename) - 1;
  return sysl <= fl  &&  strcmp (f + fl - sysl, syscalls_filename) == 0;
}
static int
needs_to_be_converted (file_p)
     const file_info *file_p;
{
  const def_dec_info *ddp;
  if (is_syscalls_file (file_p))
    return 0;
  for (ddp = file_p->defs_decs; ddp; ddp = ddp->next_in_file)
    if (
      !ddp->prototyped
      && (ddp->is_func_def || (!ddp->is_func_def && ddp->definition))
      )
          return -1;
  return 0;
}
static int
directory_specified_p (name)
     const char *name;
{
  struct string_list *p;
  for (p = directory_list; p; p = p->next)
    if (!strncmp (name, p->name, strlen (p->name))
	&& name[strlen (p->name)] == '/')
      {
	const char *q = name + strlen (p->name) + 1;
	while (*q)
	  if (*q++ == '/')
	    goto lose;
	return 1;
      lose: ;
      }
  return 0;
}
static int
file_excluded_p (name)
     const char *name;
{
  struct string_list *p;
  int len = strlen (name);
  for (p = exclude_list; p; p = p->next)
    if (!strcmp (name + len - strlen (p->name), p->name)
	&& name[len - strlen (p->name) - 1] == '/')
      return 1;
  return 0;
}
static struct string_list *
string_list_cons (string, rest)
     char *string;
     struct string_list *rest;
{
  struct string_list *temp
    = (struct string_list *) xmalloc (sizeof (struct string_list));
  temp->next = rest;
  temp->name = string;
  return temp;
}
static void
visit_each_hash_node (hash_tab_p, func)
     const hash_table_entry *hash_tab_p;
     void (*func)();
{
  const hash_table_entry *primary;
  for (primary = hash_tab_p; primary < &hash_tab_p[	(1 << 9) ]; primary++)
    if (primary->symbol)
      {
        hash_table_entry *second;
        (*func)(primary);
        for (second = primary->hash_next; second; second = second->hash_next)
          (*func) (second);
      }
}
static hash_table_entry *
add_symbol (p, s)
     hash_table_entry *p;
     const char *s;
{
  p->hash_next = 0 ;
  p->symbol = dupstr (s);
  p->_info._ddip  = 0 ;
  p->_info._fip  = 0 ;
  return p;
}
static hash_table_entry *
lookup (hash_tab_p, search_symbol)
     hash_table_entry *hash_tab_p;
     const char *search_symbol;
{
  int hash_value = 0;
  const char *search_symbol_char_p = search_symbol;
  hash_table_entry *p;
  while (*search_symbol_char_p)
    hash_value += *search_symbol_char_p++;
  hash_value &= hash_mask;
  p = &hash_tab_p[hash_value];
  if (! p->symbol)
      return add_symbol (p, search_symbol);
  if (!strcmp (p->symbol, search_symbol))
    return p;
  while (p->hash_next)
    {
      p = p->hash_next;
      if (!strcmp (p->symbol, search_symbol))
        return p;
    }
  p->hash_next = (hash_table_entry *) xmalloc (sizeof (hash_table_entry));
  p = p->hash_next;
  return add_symbol (p, search_symbol);
}
static void
free_def_dec (p)
     def_dec_info *p;
{
  xfree (p->ansi_decl);
  {
    const f_list_chain_item * curr;
    const f_list_chain_item * next;
    for (curr = p->f_list_chain; curr; curr = next)
      {
        next = curr->chain_next;
        xfree (curr);
      }
  }
  xfree (p);
}
static char *
unexpand_if_needed (aux_info_line)
     const char *aux_info_line;
{
  static char *line_buf = 0;
  static int line_buf_size = 0;
  const unexpansion* unexp_p;
  int got_unexpanded = 0;
  const char *s;
  char *copy_p = line_buf;
  if (line_buf == 0)
    {
      line_buf_size = 1024;
      line_buf = (char *) xmalloc (line_buf_size);
    }
  copy_p = line_buf;
  for (s = aux_info_line; *s != '\n'; )
    {
      for (unexp_p = unexpansions; unexp_p->expanded; unexp_p++)
        {
          const char *in_p = unexp_p->expanded;
          size_t len = strlen (in_p);
          if (*s == *in_p && !strncmp (s, in_p, len) && !is_id_char (s[len]))
            {
	      int size = strlen (unexp_p->contracted);
              got_unexpanded = 1;
	      if (copy_p + size - line_buf >= line_buf_size)
		{
		  int offset = copy_p - line_buf;
		  line_buf_size *= 2;
		  line_buf_size += size;
		  line_buf = (char *) xrealloc (line_buf, line_buf_size);
		  copy_p = line_buf + offset;
		}
              strcpy (copy_p, unexp_p->contracted);
              copy_p += size;
              s += len;
              goto continue_outer;
            }
        }
      if (copy_p - line_buf == line_buf_size)
	{
	  int offset = copy_p - line_buf;
	  line_buf_size *= 2;
	  line_buf = (char *) xrealloc (line_buf, line_buf_size);
	  copy_p = line_buf + offset;
	}
      *copy_p++ = *s++;
continue_outer: ;
    }
  if (copy_p + 2 - line_buf >= line_buf_size)
    {
      int offset = copy_p - line_buf;
      line_buf_size *= 2;
      line_buf = (char *) xrealloc (line_buf, line_buf_size);
      copy_p = line_buf + offset;
    }
  *copy_p++ = '\n';
  *copy_p++ = '\0';
  return (got_unexpanded ? dupstr (line_buf) : 0);
}
static char *
abspath (cwd, rel_filename)
     const char *cwd;
     const char *rel_filename;
{
  const char *cwd2 = (cwd) ? cwd : cwd_buffer;
  char *const abs_buffer
    = (char *) alloca (strlen (cwd2) + strlen (rel_filename) + 2);
  char *endp = abs_buffer;
  char *outp, *inp;
  {
    const char *src_p;
    if (rel_filename[0] != '/')
      {
        src_p = cwd2;
        while (*endp++ = *src_p++)
          continue;
        *(endp-1) = '/';        		 
      }
    src_p = rel_filename;
    while (*endp++ = *src_p++)
      continue;
  }
  outp = inp = abs_buffer;
  *outp++ = *inp++;        	 
  for (;;)
    {
      if (!inp[0])
        break;
      else if (inp[0] == '/' && outp[-1] == '/')
        {
          inp++;
          continue;
        }
      else if (inp[0] == '.' && outp[-1] == '/')
        {
          if (!inp[1])
                  break;
          else if (inp[1] == '/')
            {
                    inp += 2;
                    continue;
            }
          else if ((inp[1] == '.') && (inp[2] == 0 || inp[2] == '/'))
            {
                    inp += (inp[2] == '/') ? 3 : 2;
                    outp -= 2;
                    while (outp >= abs_buffer && *outp != '/')
              	outp--;
                    if (outp < abs_buffer)
                {
              	  fprintf ((&_iob[2]) , "%s: invalid file name: %s\n",
			   pname, rel_filename);
              	  exit (1);
              	}
                    *++outp = '\0';
                    continue;
            }
        }
      *outp++ = *inp++;
    }
  *outp = '\0';
  if (outp[-1] == '/')
    *--outp  = '\0';
  return dupstr (abs_buffer);
}
static const char *
shortpath (cwd, filename)
     const char *cwd;
     const char *filename;
{
  char *rel_buffer;
  char *rel_buf_p;
  char *cwd_p = cwd_buffer;
  char *path_p;
  int unmatched_slash_count = 0;
  size_t filename_len = strlen (filename);
  path_p = abspath (cwd, filename);
  rel_buf_p = rel_buffer = (char *) xmalloc (filename_len);
  while (*cwd_p && (*cwd_p == *path_p))
    {
      cwd_p++;
      path_p++;
    }
  if (!*cwd_p && (!*path_p || *path_p == '/'))	 
    {
      if (!*path_p)        	 
        return ".";
      else
        return ++path_p;
    }
  else
    {
      if (*path_p)
        {
          --cwd_p;
          --path_p;
          while (*cwd_p != '/')        	 
            {
              --cwd_p;
              --path_p;
            }
          cwd_p++;
          path_p++;
          unmatched_slash_count++;
        }
      while (*cwd_p)
        if (*cwd_p++ == '/')
	  unmatched_slash_count++;
      if (unmatched_slash_count * 3 + strlen (path_p) >= filename_len)
	return filename;
      while (unmatched_slash_count--)
        {
	  if (rel_buffer + filename_len <= rel_buf_p + 3)
	    return filename;
          *rel_buf_p++ = '.';
          *rel_buf_p++ = '/';
        }
      do
	{
	  if (rel_buffer + filename_len <= rel_buf_p)
	    return filename;
	}
      while (*rel_buf_p++ = *path_p++);
      --rel_buf_p;
      if (*(rel_buf_p-1) == '/')
        *--rel_buf_p = '\0';
      return rel_buffer;
    }
}
static file_info *
find_file (filename, do_not_stat)
     char *filename;
     int do_not_stat;
{
  hash_table_entry *hash_entry_p;
  hash_entry_p = lookup (filename_primary, filename);
  if (hash_entry_p->_info._fip )
    return hash_entry_p->_info._fip ;
  else
    {
      struct stat stat_buf;
      file_info *file_p = (file_info *) xmalloc (sizeof (file_info));
      if (do_not_stat)
        stat_buf.st_mtime = (time_t) 0;
      else
        {
          if (stat((char *)filename,  &stat_buf)  == -1)
            {
              fprintf ((&_iob[2]) , "%s: %s: can't get status: %s\n",
		       pname, shortpath (0 , filename), sys_errlist[errno]);
              stat_buf.st_mtime = (time_t) -1;
            }
        }
      hash_entry_p->_info._fip  = file_p;
      file_p->hash_entry = hash_entry_p;
      file_p->defs_decs = 0 ;
      file_p->mtime = stat_buf.st_mtime;
      return file_p;
    }
}
static void
aux_info_corrupted ()
{
  fprintf ((&_iob[2]) , "\n%s: fatal error: aux info file corrupted at line %d\n",
	   pname, current_aux_info_lineno);
  exit (1);
}
static void
check_aux_info (cond)
     int cond;
{
  if (! cond)
    aux_info_corrupted ();
}
static const char *
find_corresponding_lparen (p)
     const char *p;
{
  const char *q;
  int paren_depth;
  for (paren_depth = 1, q = p-1; paren_depth; q--)
    {
      switch (*q)
        {
          case ')':
            paren_depth++;
            break;
          case '(':
            paren_depth--;
            break;
        }
    }
  return ++q;
}
static int
referenced_file_is_newer (l, aux_info_mtime)
     const char *l;
     time_t aux_info_mtime;
{
  const char *p;
  file_info *fi_p;
  char *filename;
  check_aux_info (l[0] == '/');
  check_aux_info (l[1] == '*');
  check_aux_info (l[2] == ' ');
  {
    const char *filename_start = p = l + 3;
    while (*p != ':')
      p++;
    filename = (char *) alloca ((size_t) (p - filename_start) + 1);
    strncpy (filename, filename_start, (size_t) (p - filename_start));
    filename[p-filename_start] = '\0';
  }
  fi_p = find_file (abspath (invocation_filename, filename), 0);
  return (fi_p->mtime > aux_info_mtime);
}
static void
save_def_or_dec (l, is_syscalls)
     const char *l;
     int is_syscalls;
{
  const char *p;
  const char *semicolon_p;
  def_dec_info *def_dec_p = (def_dec_info *) xmalloc (sizeof (def_dec_info));
  def_dec_p->written = 0;
  check_aux_info (l[0] == '/');
  check_aux_info (l[1] == '*');
  check_aux_info (l[2] == ' ');
  {
    const char *filename_start = p = l + 3;
    char *filename;
    while (*p != ':')
      p++;
    filename = (char *) alloca ((size_t) (p - filename_start) + 1);
    strncpy (filename, filename_start, (size_t) (p - filename_start));
    filename[p-filename_start] = '\0';
    def_dec_p->file = find_file (abspath (invocation_filename, filename), is_syscalls);
  }
  {
    const char *line_number_start = ++p;
    char line_number[10];
    while (*p != ':')
      p++;
    strncpy (line_number, line_number_start, (size_t) (p - line_number_start));
    line_number[p-line_number_start] = '\0';
    def_dec_p->line = atoi (line_number);
  }
  p++;	 
  check_aux_info ((*p == 'N') || (*p == 'O') || (*p == 'I'));
  def_dec_p->prototyped = (*p == 'N');
  def_dec_p->is_implicit = (*p == 'I');
  p++;
  check_aux_info ((*p == 'C') || (*p == 'F'));
  def_dec_p->is_func_def = ((*p++ == 'F') || is_syscalls);
  def_dec_p->definition = 0;	 
  check_aux_info (*p++ == ' ');
  check_aux_info (*p++ == '*');
  check_aux_info (*p++ == '/');
  check_aux_info (*p++ == ' ');
  if (!strncmp (p, "static", 6))
    def_dec_p->is_static = -1;
  else if (!strncmp (p, "extern", 6))
    def_dec_p->is_static = 0;
  else
    check_aux_info (0);	 
  {
    const char *ansi_start = p;
    p += 6;	 
    while (*++p != ';')
      continue;
    semicolon_p = p;
    def_dec_p->ansi_decl
      = dupnstr (ansi_start, (size_t) ((semicolon_p+1) - ansi_start));
  }
  p--;
  def_dec_p->f_list_count = 0;
  def_dec_p->f_list_chain = 0 ;
  for (;;)
    {
      const char *left_paren_p = find_corresponding_lparen (p);
      {
        f_list_chain_item *cip =
          (f_list_chain_item *) xmalloc (sizeof (f_list_chain_item));
        cip->formals_list
	  = dupnstr (left_paren_p + 1, (size_t) (p - (left_paren_p+1)));
        cip->chain_next = def_dec_p->f_list_chain;
        def_dec_p->f_list_chain = cip;
      }
      def_dec_p->f_list_count++;
      p = left_paren_p - 2;
      if (*p != ')')
        break;
      else
        check_aux_info (*--p == ')');
    }
  {
    const char *past_fn = p + 1;
    check_aux_info (*past_fn == ' ');
    while (is_id_char (*p))
      p--;
    p++;
    {
      char *fn_string = (char *) alloca (past_fn - p + 1);
      strncpy (fn_string, p, (size_t) (past_fn - p));
      fn_string[past_fn-p] = '\0';
      def_dec_p->hash_entry = lookup (function_name_primary, fn_string);
    }
  }
  {
    const def_dec_info *other;
    for (other = def_dec_p->hash_entry->_info._ddip ; other; other = other->next_for_func)
      {
        if (def_dec_p->line == other->line && def_dec_p->file == other->file)
          {
            if (strcmp (def_dec_p->ansi_decl, other->ansi_decl))
              {
                fprintf ((&_iob[2]) , "%s:%d: declaration of function `%s' takes different forms\n",
			 def_dec_p->file->hash_entry->symbol,
			 def_dec_p->line,
			 def_dec_p->hash_entry->symbol);
                exit (1);
              }
            free_def_dec (def_dec_p);
            return;
          }
      }
  }
  def_dec_p->next_for_func = def_dec_p->hash_entry->_info._ddip ;
  def_dec_p->hash_entry->_info._ddip  = def_dec_p;
  if (!def_dec_p->file->defs_decs)
    {
      def_dec_p->file->defs_decs = def_dec_p;
      def_dec_p->next_in_file = 0 ;
    }
  else
    {
      int line = def_dec_p->line;
      const def_dec_info *prev = 0 ;
      const def_dec_info *curr = def_dec_p->file->defs_decs;
      const def_dec_info *next = curr->next_in_file;
      while (next && (line < curr->line))
        {
          prev = curr;
          curr = next;
          next = next->next_in_file;
        }
      if (line >= curr->line)
        {
          def_dec_p->next_in_file = curr;
          if (prev)
            ((  def_dec_info *) prev)->next_in_file = def_dec_p;
          else
            def_dec_p->file->defs_decs = def_dec_p;
        }
      else	 
        {
          ((  def_dec_info *) curr)->next_in_file = def_dec_p;
          def_dec_p->next_in_file = next;
        }
    }
}
static void
munge_compile_params (params_list)
     const char *params_list;
{
  const char **temp_params
    = (const char **) alloca ((strlen (params_list) + 8) * sizeof (char *));
  int param_count = 0;
  const char *param;
  temp_params[param_count++] = compiler_file_name;
  for (;;)
    {
      while (((_pctype+1)[*params_list]&0010	) )
        params_list++;
      if (!*params_list)
        break;
      param = params_list;
      while (*params_list && !((_pctype+1)[*params_list]&0010	) )
        params_list++;
      if (param[0] != '-')
        temp_params[param_count++]
	  = dupnstr (param, (size_t) (params_list - param));
      else
        {
          switch (param[1])
            {
              case 'g':
              case 'O':
              case 'S':
              case 'c':
                break;		 
              case 'o':
                while (((_pctype+1)[*params_list]&0010	) )
                  params_list++;
                while (*params_list && !((_pctype+1)[*params_list]&0010	) )
                  params_list++;
                break;
              default:
                temp_params[param_count++]
		  = dupnstr (param, (size_t) (params_list - param));
            }
        }
      if (!*params_list)
        break;
    }
  temp_params[param_count++] = "-aux-info";
  aux_info_file_name_index = param_count;
  temp_params[param_count++] = 0 ;
  temp_params[param_count++] = "-S";
  temp_params[param_count++] = "-o";
  temp_params[param_count++] = "/dev/null";
  input_file_name_index = param_count;
  temp_params[param_count++] = 0 ;
  temp_params[param_count++] = 0 ;
  compile_params
    = (const char **) xmalloc (sizeof (char *) * (param_count+1));
  memcpy (compile_params, temp_params, sizeof (char *) * param_count);
}
static int
gen_aux_info_file (base_filename)
     const char *base_filename;
{
  int child_pid;
  if (!input_file_name_index)
    munge_compile_params ("");
  compile_params[input_file_name_index] = shortpath (0 , base_filename);
  compile_params[aux_info_file_name_index]
    = savestring2 (compile_params[input_file_name_index],
	           strlen (compile_params[input_file_name_index]),
		   ".X",
		   2);
  if (!quiet_flag)
    fprintf ((&_iob[2]) , "%s: compiling `%s'\n",
	     pname, compile_params[input_file_name_index]);
  if (child_pid = vfork  ())
    {
      if (child_pid == -1)
        {
          fprintf ((&_iob[2]) , "%s: could not fork process: %s\n",
		   pname, sys_errlist[errno]);
          return 0;
        }
      {
        int wait_status;
        if (wait (&wait_status) == -1)
          {
            fprintf ((&_iob[2]) , "%s: wait failed: %s\n",
		     pname, sys_errlist[errno]);
            return 0;
          }
	if ((wait_status & 0x7F) != 0)
	  {
	    fprintf ((&_iob[2]) , "%s: subprocess got fatal signal %d",
		     pname, (wait_status & 0x7F));
	    return 0;
	  }
	if (((wait_status & 0xFF00) >> 8) != 0)
	  {
	    fprintf ((&_iob[2]) , "%s: %s exited with status %d\n",
		     pname, base_filename, ((wait_status & 0xFF00) >> 8));
	    return 0;
	  }
	return 1;
      }
    }
  else
    {
      if (execvp((char *)compile_params[0], (char **) (char *const *) compile_params) )
        {
	  int e = errno, f = ((int)(((&_iob[2]) )->_file)) ;
	  write (f, pname, strlen (pname));
	  write (f, ": ", 2);
	  write (f, compile_params[0], strlen (compile_params[0]));
	  write (f, ": ", 2);
	  write (f, sys_errlist[e], strlen (sys_errlist[e]));
	  write (f, "\n", 1);
          _exit (1);
        }
      return 1;		 
    }
}
static void
process_aux_info_file (base_source_filename, keep_it, is_syscalls)
     const char *base_source_filename;
     int keep_it;
     int is_syscalls;
{
  size_t base_len = strlen (base_source_filename);
  char * aux_info_filename
    = (char *) alloca (base_len + strlen (aux_info_suffix) + 1);
  char *aux_info_base;
  char *aux_info_limit;
  char *aux_info_relocated_name;
  const char *aux_info_second_line;
  time_t aux_info_mtime;
  size_t aux_info_size;
  int must_create;
  strcpy (aux_info_filename, base_source_filename);
  strcat (aux_info_filename, aux_info_suffix);
  must_create = 0;
start_over: ;
  if (access((char *)aux_info_filename,     4       )  == -1)
    {
      if (errno == 	2		)
	{
	  if (is_syscalls)
	    {
	      fprintf ((&_iob[2]) , "%s: warning: missing SYSCALLS file `%s'\n",
		       pname, aux_info_filename);
	      return;
	    }
	  must_create = 1;
	}
      else
	{
	  fprintf ((&_iob[2]) , "%s: can't read aux info file `%s': %s\n",
		   pname, shortpath (0 , aux_info_filename),
		   sys_errlist[errno]);
	  errors++;
	  return;
	}
    }
  if (must_create)
    {
      if (!gen_aux_info_file (base_source_filename))
	{
	  errors++;
	  return;
	}
      if (access((char *)aux_info_filename,     4       )  == -1)
	{
	  fprintf ((&_iob[2]) , "%s: can't read aux info file `%s': %s\n",
		   pname, shortpath (0 , aux_info_filename),
		   sys_errlist[errno]);
	  errors++;
	  return;
	}
    }
  {
    struct stat stat_buf;
    if (stat((char *)aux_info_filename,  &stat_buf)  == -1)
      {
        fprintf ((&_iob[2]) , "%s: can't get status of aux info file `%s': %s\n",
		 pname, shortpath (0 , aux_info_filename),
		 sys_errlist[errno]);
        errors++;
        return;
      }
    if ((aux_info_size = stat_buf.st_size) == 0)
      return;
    aux_info_mtime = stat_buf.st_mtime;
    if (!is_syscalls)
      {
	if (stat((char *)base_source_filename,  &stat_buf)  == -1)
	  {
	    fprintf ((&_iob[2]) , "%s: can't get status of aux info file `%s': %s\n",
		     pname, shortpath (0 , base_source_filename),
		     sys_errlist[errno]);
	    errors++;
	    return;
	  }
	if (stat_buf.st_mtime > aux_info_mtime)
	  {
	    must_create = 1;
	    goto start_over;
	  }
      }
  }
  {
    int aux_info_file;
    if ((aux_info_file = open((char *)aux_info_filename,         0 ,  0444 ) ) == -1)
      {
        fprintf ((&_iob[2]) , "%s: can't open aux info file `%s' for reading: %s\n",
		 pname, shortpath (0 , aux_info_filename),
		 sys_errlist[errno]);
        return;
      }
    aux_info_base = xmalloc (aux_info_size + 1);
    aux_info_limit = aux_info_base + aux_info_size;
    *aux_info_limit = '\0';
    if (read (aux_info_file, aux_info_base, aux_info_size) != aux_info_size)
      {
        fprintf ((&_iob[2]) , "%s: error reading aux info file `%s': %s\n",
		 pname, shortpath (0 , aux_info_filename),
		 sys_errlist[errno]);
        free (aux_info_base);
        close (aux_info_file);
        return;
      }
    if (close (aux_info_file))
      {
        fprintf ((&_iob[2]) , "%s: error closing aux info file `%s': %s\n",
		 pname, shortpath (0 , aux_info_filename),
		 sys_errlist[errno]);
        free (aux_info_base);
        close (aux_info_file);
        return;
      }
  }
  if (must_create && !keep_it)
    if (	unlink((char *)aux_info_filename)  == -1)
      fprintf ((&_iob[2]) , "%s: can't delete aux info file `%s': %s\n",
	       pname, shortpath (0 , aux_info_filename),
	       sys_errlist[errno]);
  {
    char *p = aux_info_base;
    while (*p != ':')
      p++;
    p++;
    while (*p == ' ')
      p++;
    invocation_filename = p;	 
    while (*p != ' ')
      p++;
    *p++ = '/';
    *p++ = '\0';
    while (*p++ != '\n')
      continue;
    aux_info_second_line = p;
    aux_info_relocated_name = 0;
    if (invocation_filename[0] != '/')
      {
	char *dir_end;
	aux_info_relocated_name = xmalloc (base_len + (p-invocation_filename));
	strcpy (aux_info_relocated_name, base_source_filename);
	dir_end = rindex (aux_info_relocated_name, '/');
	if (dir_end)
	  dir_end++;
	else
	  dir_end = aux_info_relocated_name;
	strcpy (dir_end, invocation_filename);
	invocation_filename = aux_info_relocated_name;
      }
  }
  {
    const char *aux_info_p;
    if (!is_syscalls)
      {
        current_aux_info_lineno = 2;
        for (aux_info_p = aux_info_second_line; *aux_info_p; )
          {
            if (referenced_file_is_newer (aux_info_p, aux_info_mtime))
              {
                free (aux_info_base);
		xfree (aux_info_relocated_name);
                if (keep_it && 	unlink((char *)aux_info_filename)  == -1)
                  {
                    fprintf ((&_iob[2]) , "%s: can't delete file `%s': %s\n",
			     pname, shortpath (0 , aux_info_filename),
			     sys_errlist[errno]);
                    return;
                  }
                goto start_over;
              }
            while (*aux_info_p != '\n')
              aux_info_p++;
            aux_info_p++;
            current_aux_info_lineno++;
          }
      }
    current_aux_info_lineno = 2;
    for (aux_info_p = aux_info_second_line; *aux_info_p;)
      {
        char *unexpanded_line = unexpand_if_needed (aux_info_p);
        if (unexpanded_line)
          {
            save_def_or_dec (unexpanded_line, is_syscalls);
            free (unexpanded_line);
          }
        else
          save_def_or_dec (aux_info_p, is_syscalls);
        while (*aux_info_p != '\n')
          aux_info_p++;
        aux_info_p++;
        current_aux_info_lineno++;
      }
  }
  free (aux_info_base);
  xfree (aux_info_relocated_name);
}
static void
rename_c_file (hp)
     const hash_table_entry *hp;
{
  const char *filename = hp->symbol;
  int last_char_index = strlen (filename) - 1;
  char *const new_filename = (char *) alloca (strlen (filename) + 1);
  if (filename[last_char_index] != 'c' || filename[last_char_index-1] != '.')
    return;
  strcpy (new_filename, filename);
  new_filename[last_char_index] = 'C';
  if (link((char *)filename, (char *) new_filename)  == -1)
    {
      fprintf ((&_iob[2]) , "%s: warning: can't link file `%s' to `%s': %s\n",
	       pname, shortpath (0 , filename),
	       shortpath (0 , new_filename), sys_errlist[errno]);
      errors++;
      return;
    }
  if (	unlink((char *)filename)  == -1)
    {
      fprintf ((&_iob[2]) , "%s: warning: can't delete file `%s': %s\n",
	       pname, shortpath (0 , filename), sys_errlist[errno]);
      errors++;
      return;
    }
}
static void
reverse_def_dec_list (hp)
     const hash_table_entry *hp;
{
  file_info *file_p = hp->_info._fip ;
  const def_dec_info *prev = 0 ;
  const def_dec_info *current = file_p->defs_decs;
  if (!( current = file_p->defs_decs))
    return;        		 
  prev = current;
  if (! (current = current->next_in_file))
    return;        		 
  ((  def_dec_info *) prev)->next_in_file = 0 ;
  while (current)
    {
      const def_dec_info *next = current->next_in_file;
      ((  def_dec_info *) current)->next_in_file = prev;
      prev = current;
      current = next;
    }
  file_p->defs_decs = prev;
}
static const def_dec_info *
find_extern_def (head, user)
     const def_dec_info *head;
     const def_dec_info *user;
{
  const def_dec_info *dd_p;
  const def_dec_info *extern_def_p = 0 ;
  int conflict_noted = 0;
  for (dd_p = head; dd_p; dd_p = dd_p->next_for_func)
    if (dd_p->is_func_def && !dd_p->is_static && dd_p->file == user->file)
      return dd_p;
  for (dd_p = head; dd_p; dd_p = dd_p->next_for_func)
    if (dd_p->is_func_def && !dd_p->is_static)
      {
        if (!extern_def_p)	 
          extern_def_p = dd_p;	 
        else
          {
            if (is_syscalls_file (dd_p->file))
              continue;
            if (is_syscalls_file (extern_def_p->file))
              {
                extern_def_p = dd_p;
                continue;
              }
            if (!conflict_noted)	 
              {
                conflict_noted = 1;
                fprintf ((&_iob[2]) , "%s: conflicting extern definitions of '%s'\n",
			 pname, head->hash_entry->symbol);
                if (!quiet_flag)
                  {
                    fprintf ((&_iob[2]) , "%s: declarations of '%s' will not be converted\n",
			     pname, head->hash_entry->symbol);
                    fprintf ((&_iob[2]) , "%s: conflict list for '%s' follows:\n",
			     pname, head->hash_entry->symbol);
                    fprintf ((&_iob[2]) , "%s:     %s(%d): %s\n",
			     pname,
			     shortpath (0 , extern_def_p->file->hash_entry->symbol),
			     extern_def_p->line, extern_def_p->ansi_decl);
                  }
              }
            if (!quiet_flag)
              fprintf ((&_iob[2]) , "%s:     %s(%d): %s\n",
		       pname,
		       shortpath (0 , dd_p->file->hash_entry->symbol),
		       dd_p->line, dd_p->ansi_decl);
          }
      }
  if (conflict_noted)
    return 0 ;
  if (!extern_def_p)
    {
      for (dd_p = head; dd_p; dd_p = dd_p->next_for_func)
        if (!dd_p->is_func_def && !dd_p->is_static && dd_p->prototyped)
          {
            extern_def_p = dd_p;	 
            if (!quiet_flag)
              fprintf ((&_iob[2]) , "%s: warning: using formals list from %s(%d) for function `%s'\n",
		       pname,
		       shortpath (0 , dd_p->file->hash_entry->symbol),
		       dd_p->line, dd_p->hash_entry->symbol);
            break;
          }
      if (!extern_def_p)
        {
          const char *file = user->file->hash_entry->symbol;
          if (!quiet_flag)
            if (in_system_include_dir (file))
              {
		char *needed = (char *) alloca (strlen (user->ansi_decl) + 1);
                char *p;
                strcpy (needed, user->ansi_decl);
                p = (  char *) substr (needed, user->hash_entry->symbol)
                    + strlen (user->hash_entry->symbol) + 2;
		*p++ = '?';
                strcpy (p, ");");
                fprintf ((&_iob[2]) , "%s: %d: `%s' used but missing from SYSCALLS\n",
			 shortpath (0 , file), user->line,
			 needed+7);	 
              }
        }
    }
  return extern_def_p;
}
static const def_dec_info *
find_static_definition (user)
     const def_dec_info *user;
{
  const def_dec_info *head = user->hash_entry->_info._ddip ;
  const def_dec_info *dd_p;
  int num_static_defs = 0;
  const def_dec_info *static_def_p = 0 ;
  for (dd_p = head; dd_p; dd_p = dd_p->next_for_func)
    if (dd_p->is_func_def && dd_p->is_static && (dd_p->file == user->file))
      {
        static_def_p = dd_p;	 
        num_static_defs++;
      }
  if (num_static_defs == 0)
    {
      if (!quiet_flag)
        fprintf ((&_iob[2]) , "%s: warning: no static definition for `%s' in file `%s'\n",
		 pname, head->hash_entry->symbol,
		 shortpath (0 , user->file->hash_entry->symbol));
    }
  else if (num_static_defs > 1)
    {
      fprintf ((&_iob[2]) , "%s: multiple static defs of `%s' in file `%s'\n",
	       pname, head->hash_entry->symbol,
	       shortpath (0 , user->file->hash_entry->symbol));
      return 0 ;
    }
  return static_def_p;
}
static void
connect_defs_and_decs (hp)
     const hash_table_entry *hp;
{
  const def_dec_info *dd_p;
  const def_dec_info *extern_def_p = 0 ;
  int first_extern_reference = 1;
  for (dd_p = hp->_info._ddip ; dd_p; dd_p = dd_p->next_for_func)
    if (dd_p->prototyped)
      ((  def_dec_info *) dd_p)->definition = dd_p;
  for (dd_p = hp->_info._ddip ; dd_p; dd_p = dd_p->next_for_func)
    if (!dd_p->is_func_def && !dd_p->is_static && !dd_p->definition)
      {
        if (first_extern_reference)
          {
            extern_def_p = find_extern_def (hp->_info._ddip , dd_p);
            first_extern_reference = 0;
          }
        ((  def_dec_info *) dd_p)->definition = extern_def_p;
      }
  for (dd_p = hp->_info._ddip ; dd_p; dd_p = dd_p->next_for_func)
    if (!dd_p->is_func_def && dd_p->is_static && !dd_p->definition)
      {
        const def_dec_info *dd_p2;
        const def_dec_info *static_def;
      ((  def_dec_info *) dd_p)->definition =
        (static_def = find_static_definition (dd_p))
          ? static_def
          : (const def_dec_info *) -1;
      for (dd_p2 = dd_p->next_for_func; dd_p2; dd_p2 = dd_p2->next_for_func)
        if (!dd_p2->is_func_def && dd_p2->is_static
         && !dd_p2->definition && (dd_p2->file == dd_p->file))
          ((  def_dec_info *)dd_p2)->definition = dd_p->definition;
      }
  for (dd_p = hp->_info._ddip ; dd_p; dd_p = dd_p->next_for_func)
    if (dd_p->definition == (def_dec_info *) -1)
      ((  def_dec_info *) dd_p)->definition = 0 ;
}
static int
identify_lineno (clean_p)
     const char *clean_p;
{
  int line_num = 1;
  const char *scan_p;
  for (scan_p = clean_text_base; scan_p <= clean_p; scan_p++)
    if (*scan_p == '\n')
      line_num++;
  return line_num;
}
static void
declare_source_confusing (clean_p)
     const char *clean_p;
{
  if (!quiet_flag)
    {
      if (clean_p == 0)
        fprintf ((&_iob[2]) , "%s: %d: warning: source too confusing\n",
		 shortpath (0 , convert_filename), last_known_line_number);
      else
        fprintf ((&_iob[2]) , "%s: %d: warning: source too confusing\n",
		 shortpath (0 , convert_filename),
		 identify_lineno (clean_p));
    }
  longjmp (source_confusion_recovery, 1);
}
static void
check_source (cond, clean_p)
     int cond;
     const char *clean_p;
{
  if (!cond)
    declare_source_confusing (clean_p);
}
static const char *
seek_to_line (n)
     int n;
{
  if (n < last_known_line_number)
    abort ();
  while (n > last_known_line_number)
    {
      while (*last_known_line_start != '\n')
        check_source (++last_known_line_start < clean_text_limit, 0);
      last_known_line_start++;
      last_known_line_number++;
    }
  return last_known_line_start;
}
static const char *
forward_to_next_token_char (ptr)
     const char *ptr;
{
  for (++ptr; ((_pctype+1)[*ptr]&0010	) ; check_source (++ptr < clean_text_limit, 0))
    continue;
  return ptr;
}
static void
output_bytes (str, len)
     const char *str;
     size_t len;
{
  if ((repl_write_ptr + 1) + len >= repl_text_limit)
    {
      size_t new_size = (repl_text_limit - repl_text_base) << 1;
      char *new_buf = (char *) xrealloc (repl_text_base, new_size);
      repl_write_ptr = new_buf + (repl_write_ptr - repl_text_base);
      repl_text_base = new_buf;
      repl_text_limit = new_buf + new_size;
    }
  memcpy (repl_write_ptr + 1, str, len);
  repl_write_ptr += len;
}
static void
output_string (str)
     const char *str;
{
  output_bytes (str, strlen (str));
}
static void
output_up_to (p)
     const char *p;
{
  size_t copy_length = (size_t) (p - clean_read_ptr);
  const char *copy_start = orig_text_base+(clean_read_ptr-clean_text_base)+1;
  if (copy_length == 0)
    return;
  output_bytes (copy_start, copy_length);
  clean_read_ptr = p;
}
static int
other_variable_style_function (ansi_header)
     const char *ansi_header;
{
  const char *p;
  int len = strlen (varargs_style_indicator);
  for (p = ansi_header; p; )
    {
      const char *candidate;
      if ((candidate = substr (p, varargs_style_indicator)) == 0)
        return 0;
      else
        if (!is_id_char (candidate[-1]) && !is_id_char (candidate[len]))
          return 1;
        else
          p = candidate + 1;
    }
  return 0;
}
static void
edit_fn_declaration (def_dec_p, clean_text_p)
     const def_dec_info *def_dec_p;
     const char *volatile clean_text_p;
{
  const char *start_formals;
  const char *end_formals;
  const char *function_to_edit = def_dec_p->hash_entry->symbol;
  size_t func_name_len = strlen (function_to_edit);
  const char *end_of_fn_name;
  const f_list_chain_item *this_f_list_chain_item;
  const def_dec_info *definition = def_dec_p->definition;
  if (!definition)
    return;
  if (other_variable_style_function (definition->ansi_decl))
    {
      if (!quiet_flag)
        fprintf ((&_iob[2]) , "%s: %d: warning: varargs function declaration not converted\n",
		 shortpath (0 , def_dec_p->file->hash_entry->symbol),
		 def_dec_p->line);
      return;
    }
  save_pointers ();
  if (setjmp (source_confusion_recovery))
    {
      restore_pointers ();
      fprintf ((&_iob[2]) , "%s: declaration of function `%s' not converted\n",
	       pname, function_to_edit);
      return;
    }
  while (*clean_text_p != '\n')
    check_source (++clean_text_p < clean_text_limit, 0);
  clean_text_p--;   
  do
    {
      for (;;)
        {
          while (!is_id_char (*clean_text_p))
            check_source (--clean_text_p > clean_read_ptr, 0);
          while (is_id_char (*clean_text_p))
            check_source (--clean_text_p > clean_read_ptr, 0);
          if (!strncmp (clean_text_p+1, function_to_edit, func_name_len))
            {
              char ch = *(clean_text_p + 1 + func_name_len);
              if (! is_id_char (ch))
                break;			 
            }
        }
      end_of_fn_name = clean_text_p + strlen (def_dec_p->hash_entry->symbol);
      start_formals = forward_to_next_token_char (end_of_fn_name);
    }
  while (*start_formals != '(');
  this_f_list_chain_item = definition->f_list_chain;
  for (;;)
    {
      {
        int depth;
        end_formals = start_formals + 1;
        depth = 1;
        for (; depth; check_source (++end_formals < clean_text_limit, 0))
          {
            switch (*end_formals)
              {
                case '(':
                  depth++;
                  break;
                case ')':
                  depth--;
                  break;
              }
          }
        end_formals--;
      }
      output_up_to (start_formals);
      if (this_f_list_chain_item)
        {
          output_string (this_f_list_chain_item->formals_list);
          this_f_list_chain_item = this_f_list_chain_item->chain_next;
        }
      else
        {
          if (!quiet_flag)
            fprintf ((&_iob[2]) , "%s: warning: too many parameter lists in declaration of `%s'\n",
		     pname, def_dec_p->hash_entry->symbol);
          check_source (0, end_formals);   
        }
      clean_read_ptr = end_formals - 1;
      {
        const char *another_r_paren = forward_to_next_token_char (end_formals);
        if ((*another_r_paren != ')')
            || (*(start_formals = forward_to_next_token_char (another_r_paren)) != '('))
          {
            if (this_f_list_chain_item)
              {
                if (!quiet_flag)
                  fprintf ((&_iob[2]) , "\n%s: warning: too few parameter lists in declaration of `%s'\n",
			   pname, def_dec_p->hash_entry->symbol);
                check_source (0, start_formals);  
              }
            break;
          }
      }
    }
}
static int
edit_formals_lists (end_formals, f_list_count, def_dec_p)
     const char *end_formals;
     unsigned int f_list_count;
     const def_dec_info *def_dec_p;
{
  const char *start_formals;
  int depth;
  start_formals = end_formals - 1;
  depth = 1;
  for (; depth; check_source (--start_formals > clean_read_ptr, 0))
    {
      switch (*start_formals)
        {
          case '(':
            depth--;
            break;
          case ')':
            depth++;
            break;
        }
    }
  start_formals++;
  f_list_count--;
  if (f_list_count)
    {
      const char *next_end;
      next_end = start_formals - 1;
      check_source (next_end > clean_read_ptr, 0);
      while (((_pctype+1)[*next_end]&0010	) )
        check_source (--next_end > clean_read_ptr, 0);
      check_source (*next_end == ')', next_end);
      check_source (--next_end > clean_read_ptr, 0);
      check_source (*next_end == ')', next_end);
      if (edit_formals_lists (next_end, f_list_count, def_dec_p))
        return 1;
    }
  if (f_list_count == 0)
    {
      const char *expected = def_dec_p->hash_entry->symbol;
      const char *func_name_start;
      const char *func_name_limit;
      size_t func_name_len;
      for (func_name_limit = start_formals-1; ((_pctype+1)[*func_name_limit]&0010	) ; )
        check_source (--func_name_limit > clean_read_ptr, 0);
      for (func_name_start = func_name_limit++;
           is_id_char (*func_name_start);
           func_name_start--)
        check_source (func_name_start > clean_read_ptr, 0);
      func_name_start++;
      func_name_len = func_name_limit - func_name_start;
      if (func_name_len == 0)
        check_source (0, func_name_start);
      if (func_name_len != strlen (expected)
	  || strncmp (func_name_start, expected, func_name_len))
        {
          fprintf ((&_iob[2]) , "%s: %d: warning: found `%s' but expected `%s'\n",
		   shortpath (0 , def_dec_p->file->hash_entry->symbol),
		   identify_lineno (func_name_start),
		   dupnstr (func_name_start, func_name_len),
		   expected);
          return 1;
        }
    }
  output_up_to (start_formals);
  {
    unsigned f_list_depth;
    const f_list_chain_item *flci_p = def_dec_p->f_list_chain;
    for (f_list_depth = 0; f_list_depth < f_list_count; f_list_depth++)
      flci_p = flci_p->chain_next;
    output_string (flci_p->formals_list);
  }
  clean_read_ptr = end_formals - 1;
  return 0;
}
static const char *
find_rightmost_formals_list (clean_text_p)
     const char *clean_text_p;
{
  const char *end_formals;
  for (end_formals = clean_text_p; *end_formals != '\n'; end_formals++)
    continue;
  end_formals--;
  while (1)
    {
      char ch;
      const char *l_brace_p;
      while (*end_formals != ')')
        {
          if (((_pctype+1)[*end_formals]&0010	) )
            while (((_pctype+1)[*end_formals]&0010	) )
              check_source (--end_formals > clean_read_ptr, 0);
          else
            check_source (--end_formals > clean_read_ptr, 0);
        }
      ch = *(l_brace_p = forward_to_next_token_char (end_formals));
      if ((ch == '{') || ((_pctype+1)[ch]&(0001	|0002	)) )
        break;
      check_source (--end_formals > clean_read_ptr, 0);
    }
  return end_formals;
}
static void
add_local_decl (def_dec_p, clean_text_p)
     const def_dec_info *def_dec_p;
     const char *clean_text_p;
{
  const char *start_of_block;
  const char *function_to_edit = def_dec_p->hash_entry->symbol;
  if (!local_flag)
    return;
  save_pointers ();
  if (setjmp (source_confusion_recovery))
    {
      restore_pointers ();
      fprintf ((&_iob[2]) , "%s: local declaration for function `%s' not inserted\n",
	       pname, function_to_edit);
      return;
    }
  start_of_block = clean_text_p;
  while (*start_of_block != '{' && *start_of_block != '\n')
    check_source (++start_of_block < clean_text_limit, 0);
  if (*start_of_block != '{')
    {
      if (!quiet_flag)
        fprintf ((&_iob[2]) ,
          "\n%s: %d: warning: can't add declaration of `%s' into macro call\n",
          def_dec_p->file->hash_entry->symbol, def_dec_p->line, 
          def_dec_p->hash_entry->symbol);
      return;
    }
  {
    const char *ep = forward_to_next_token_char (start_of_block) - 1;
    const char *sp;
    for (sp = ep; ((_pctype+1)[*sp]&0010	)  && *sp != '\n'; sp--)
      continue;
    output_up_to (ep);
    {
      const char *decl = def_dec_p->definition->ansi_decl;
      if ((*decl == 'e') && (def_dec_p->file == def_dec_p->definition->file))
        decl += 7;
      output_string (decl);
    }
    output_bytes (sp, (size_t) (ep - sp) + 1);
  }
}
static void
add_global_decls (file_p, clean_text_p)
     const file_info *file_p;
     const char *clean_text_p;
{
  const def_dec_info *dd_p;
  const char *scan_p;
  save_pointers ();
  if (setjmp (source_confusion_recovery))
    {
      restore_pointers ();
      fprintf ((&_iob[2]) , "%s: global declarations for file `%s' not inserted\n",
	       pname, shortpath (0 , file_p->hash_entry->symbol));
      return;
    }
  scan_p = find_rightmost_formals_list (clean_text_p);
  for (;; --scan_p)
    {
      if (scan_p < clean_text_base)
        break;
      check_source (scan_p > clean_read_ptr, 0);
      if (*scan_p == ';')
        break;
    }
  scan_p++;
  while (((_pctype+1)[*scan_p]&0010	) )
    scan_p++;
  scan_p--;
  output_up_to (scan_p);
  {
    int some_decls_added = 0;
    for (dd_p = file_p->defs_decs; dd_p; dd_p = dd_p->next_in_file)
      if (dd_p->is_implicit && dd_p->definition && !dd_p->definition->written)
        {
          const char *decl = dd_p->definition->ansi_decl;
          if (*decl == 'e' && (dd_p->file == dd_p->definition->file))
            decl += 7;
          output_string ("\n");
          output_string (decl);
          some_decls_added = 1;
          ((  def_dec_info *) dd_p->definition)->written = 1;
        }
    if (some_decls_added)
      output_string ("\n\n");
  }
  for (dd_p = file_p->defs_decs; dd_p; dd_p = dd_p->next_in_file)
    if (dd_p->definition)
      ((  def_dec_info *) dd_p->definition)->written = 0;
}
static void
edit_fn_definition (def_dec_p, clean_text_p)
     const def_dec_info *def_dec_p;
     const char *clean_text_p;
{
  const char *end_formals;
  const char *function_to_edit = def_dec_p->hash_entry->symbol;
  save_pointers ();
  if (setjmp (source_confusion_recovery))
    {
      restore_pointers ();
      fprintf ((&_iob[2]) , "%s: definition of function `%s' not converted\n",
	       pname, function_to_edit);
      return;
    }
  end_formals = find_rightmost_formals_list (clean_text_p);
  if (other_variable_style_function (def_dec_p->ansi_decl))
    {
      if (!quiet_flag)
        fprintf ((&_iob[2]) , "%s: %d: warning: definition of %s not converted\n",
		 shortpath (0 , def_dec_p->file->hash_entry->symbol),
		 identify_lineno (end_formals), 
		 other_var_style);
      output_up_to (end_formals);
      return;
    }
  if (edit_formals_lists (end_formals, def_dec_p->f_list_count, def_dec_p))
    {
      restore_pointers ();
      fprintf ((&_iob[2]) , "%s: definition of function `%s' not converted\n",
	       pname, function_to_edit);
      return;
    }
  output_up_to (end_formals);
  {
    const char *end_formals_orig;
    const char *start_body;
    const char *start_body_orig;
    const char *scan;
    const char *scan_orig;
    int have_flotsum = 0;
    int have_newlines = 0;
    for (start_body = end_formals + 1; *start_body != '{';)
      check_source (++start_body < clean_text_limit, 0);
    end_formals_orig = orig_text_base + (end_formals - clean_text_base);
    start_body_orig = orig_text_base + (start_body - clean_text_base);
    scan = end_formals + 1;
    scan_orig = end_formals_orig + 1;
    for (; scan < start_body; scan++, scan_orig++)
      {
        if (*scan == *scan_orig)
          {
            have_newlines |= (*scan_orig == '\n');
            if (!((_pctype+1)[*scan_orig]&0010	) )
              *((  char *)scan_orig) = ' ';  
          }
        else
          have_flotsum = 1;
      }
    if (have_flotsum)
      output_bytes (end_formals_orig + 1,
		    (size_t) (start_body_orig - end_formals_orig) - 1);
    else
      if (have_newlines)
        output_string ("\n");
      else
        output_string (" ");
    clean_read_ptr = start_body - 1;
  }
}
static void
do_cleaning (new_clean_text_base, new_clean_text_limit)
     char *new_clean_text_base;
     char *new_clean_text_limit;
{
  char *scan_p;
  int non_whitespace_since_newline = 0;
  for (scan_p = new_clean_text_base; scan_p < new_clean_text_limit; scan_p++)
    {
      switch (*scan_p)
        {
          case '/':			 
            if (scan_p[1] != '*')
              goto regular;
            non_whitespace_since_newline = 1;
            scan_p[0] = ' ';
            scan_p[1] = ' ';
            scan_p += 2;
            while (scan_p[1] != '/' || scan_p[0] != '*')
              {
                if (!((_pctype+1)[*scan_p]&0010	) )
                  *scan_p = ' ';
                if (++scan_p >= new_clean_text_limit)
                  abort ();
              }
            *scan_p++ = ' ';
            *scan_p = ' ';
            break;
          case '#':			 
            if (non_whitespace_since_newline)
              goto regular;
            *scan_p = ' ';
            while (scan_p[1] != '\n' || scan_p[0] == '\\')
              {
                if (!((_pctype+1)[*scan_p]&0010	) )
                  *scan_p = ' ';
                if (++scan_p >= new_clean_text_limit)
                  abort ();
              }
            *scan_p++ = ' ';
            break;
          case '\'':			 
            non_whitespace_since_newline = 1;
            while (scan_p[1] != '\'' || scan_p[0] == '\\')
              {
                if (scan_p[0] == '\\' && !((_pctype+1)[scan_p[1]]&0010	) )
                  scan_p[1] = ' ';
                if (!((_pctype+1)[*scan_p]&0010	) )
                  *scan_p = ' ';
                if (++scan_p >= new_clean_text_limit)
                  abort ();
              }
            *scan_p++ = ' ';
            break;
          case '"':			 
            non_whitespace_since_newline = 1;
            while (scan_p[1] != '"' || scan_p[0] == '\\')
              {
                if (scan_p[0] == '\\' && !((_pctype+1)[scan_p[1]]&0010	) )
                  scan_p[1] = ' ';
                if (!((_pctype+1)[*scan_p]&0010	) )
                  *scan_p = ' ';
                if (++scan_p >= new_clean_text_limit)
                  abort ();
              }
            *scan_p++ = ' ';
            break;
          case '\\':			 
            if (scan_p[1] != '\n')
              goto regular;
            *scan_p = ' ';
            break;
          case '\n':
            non_whitespace_since_newline = 0;	 
            break;
          case ' ':
          case '\v':
          case '\t':
          case '\r':
          case '\f':
          case '\b':
            break;		 
          default:
regular:
            non_whitespace_since_newline = 1;
            break;
        }
    }
}
static const char *
careful_find_l_paren (p)
     const char *p;
{
  const char *q;
  int paren_depth;
  for (paren_depth = 1, q = p-1; paren_depth; check_source (--q >= clean_text_base, 0))
    {
      switch (*q)
        {
          case ')':
            paren_depth++;
            break;
          case '(':
            paren_depth--;
            break;
        }
    }
  return ++q;
}
static void
scan_for_missed_items (file_p)
     const file_info *file_p;
{
  static const char *scan_p;
  const char *limit = clean_text_limit - 3;
  static const char *backup_limit;
  backup_limit = clean_text_base - 1;
  for (scan_p = clean_text_base; scan_p < limit; scan_p++)
    {
      if (*scan_p == ')')
        {
          static const char *last_r_paren;
          const char *ahead_p;
          last_r_paren = scan_p;
          for (ahead_p = scan_p + 1; ((_pctype+1)[*ahead_p]&0010	) ; )
            check_source (++ahead_p < limit, limit);
          scan_p = ahead_p - 1;
          if (((_pctype+1)[*ahead_p]&(0001	|0002	))  || *ahead_p == '{')
            {
              const char *last_l_paren;
              const int lineno = identify_lineno (ahead_p);
              if (setjmp (source_confusion_recovery))
                continue;
              do
                {
                  last_l_paren = careful_find_l_paren (last_r_paren);
                  for (last_r_paren = last_l_paren-1; ((_pctype+1)[*last_r_paren]&0010	) ; )
                    check_source (--last_r_paren >= backup_limit, backup_limit);
                }
              while (*last_r_paren == ')');
              if (is_id_char (*last_r_paren))
                {
                  const char *id_limit = last_r_paren + 1;
                  const char *id_start;
                  size_t id_length;
                  const def_dec_info *dd_p;
                  for (id_start = id_limit-1; is_id_char (*id_start); )
                    check_source (--id_start >= backup_limit, backup_limit);
                  id_start++;
                  backup_limit = id_start;
                  if ((id_length = (size_t) (id_limit - id_start)) == 0)
                    goto not_missed;
		  {
		    char *func_name = (char *) alloca (id_length + 1);
		    static const char * const stmt_keywords[]
		      = { "if", "while", "for", "switch", "return", 0 };
		    const char * const *stmt_keyword;
		    strncpy (func_name, id_start, id_length);
		    func_name[id_length] = '\0';
		    for (stmt_keyword = stmt_keywords; *stmt_keyword; stmt_keyword++)
		      if (!strcmp (func_name, *stmt_keyword))
			goto not_missed;
		    for (dd_p = file_p->defs_decs; dd_p; dd_p = dd_p->next_in_file)
		      if (dd_p->is_func_def && dd_p->line == lineno)
			goto not_missed;
		    fprintf ((&_iob[2]) , "%s: %d: warning: `%s' excluded by preprocessing\n",
			     shortpath (0 , file_p->hash_entry->symbol),
			     identify_lineno (id_start), func_name);
		    fprintf ((&_iob[2]) , "%s: function definition not converted\n",
			     pname);
		  }
		not_missed: ;
                }
            }
        }
    }
}
static void
edit_file (hp)
     const hash_table_entry *hp;
{
  struct stat stat_buf;
  const file_info *file_p = hp->_info._fip ;
  char *new_orig_text_base;
  char *new_orig_text_limit;
  char *new_clean_text_base;
  char *new_clean_text_limit;
  size_t orig_size;
  size_t repl_size;
  int first_definition_in_file;
  if (!needs_to_be_converted (file_p))
    return;
  convert_filename = file_p->hash_entry->symbol;
  if (!directory_specified_p (convert_filename)
      || file_excluded_p (convert_filename))
    {
      if (!quiet_flag
          )
        fprintf ((&_iob[2]) , "%s: `%s' not converted\n",
		 pname, shortpath (0 , convert_filename));
      return;
    }
  if (nochange_flag)
    fprintf ((&_iob[2]) , "%s: would convert file `%s'\n",
	     pname, shortpath (0 , convert_filename));
  else
    fprintf ((&_iob[2]) , "%s: converting file `%s'\n",
	     pname, shortpath (0 , convert_filename));
  fflush ((&_iob[2]) );
  if (stat((char *)(char *)convert_filename,  &stat_buf)  == -1)
    {
      fprintf ((&_iob[2]) , "%s: can't get status for file `%s': %s\n",
	       pname, shortpath (0 , convert_filename), sys_errlist[errno]);
      return;
    }
  orig_size = stat_buf.st_size;
  orig_text_base = new_orig_text_base = (char *) xmalloc (orig_size + 2);
  orig_text_limit = new_orig_text_limit = new_orig_text_base + orig_size;
  clean_text_base = new_clean_text_base = (char *) xmalloc (orig_size + 2);
  clean_text_limit = new_clean_text_limit = new_clean_text_base + orig_size;
  clean_read_ptr = clean_text_base - 1;
  repl_size = orig_size + (orig_size >> 2) + 4096;
  repl_text_base = (char *) xmalloc (repl_size + 2);
  repl_text_limit = repl_text_base + repl_size - 1;
  repl_write_ptr = repl_text_base - 1;
  {
    int input_file;
    if ((input_file = open((char *)convert_filename,         0 ,  0444) ) == -1)
      {
        fprintf ((&_iob[2]) , "%s: can't open file `%s' for reading: %s\n",
		 pname, shortpath (0 , convert_filename),
		 sys_errlist[errno]);
        return;
      }
    if (read (input_file, new_orig_text_base, orig_size) != orig_size)
      {
        close (input_file);
        fprintf ((&_iob[2]) , "\n%s: error reading input file `%s': %s\n",
		 pname, shortpath (0 , convert_filename),
		 sys_errlist[errno]);
        return;
      }
    close (input_file);
  }
  if (orig_size == 0 || orig_text_limit[-1] != '\n')
    {
      *new_orig_text_limit++ = '\n';
      orig_text_limit++;
    }
  memcpy (new_clean_text_base, orig_text_base,
	  (size_t) (orig_text_limit - orig_text_base));
  do_cleaning (new_clean_text_base, new_clean_text_limit);
  scan_for_missed_items (file_p);
  last_known_line_number = 1;
  last_known_line_start = clean_text_base;
  {
    const def_dec_info *def_dec_p;
    first_definition_in_file = 1;
    def_dec_p = file_p->defs_decs;
    for (; def_dec_p; def_dec_p = def_dec_p->next_in_file)
      {
        const char *clean_text_p = seek_to_line (def_dec_p->line);
        if (global_flag && def_dec_p->is_func_def && first_definition_in_file)
          {
            add_global_decls (def_dec_p->file, clean_text_p);
            first_definition_in_file = 0;
          }
        if (def_dec_p->prototyped
         || (!def_dec_p->is_func_def && !def_dec_p->definition))
          continue;
        if (def_dec_p->is_func_def)
          edit_fn_definition (def_dec_p, clean_text_p);
        else
  	if (def_dec_p->is_implicit)
  	  add_local_decl (def_dec_p, clean_text_p);
  	else
            edit_fn_declaration (def_dec_p, clean_text_p);
      }
  }
  output_up_to (clean_text_limit - 1);
  if (nochange_flag)
    {
      free (new_orig_text_base);
      free (new_clean_text_base);
      free (repl_text_base);
      return;
    }
  if (!nosave_flag)
    {
      char *new_filename =
          (char *) xmalloc (strlen (convert_filename) + strlen (save_suffix) + 2);
      strcpy (new_filename, convert_filename);
      strcat (new_filename, save_suffix);
      if (link((char *)convert_filename, (char *) new_filename)  == -1)
        {
          if (errno == 	17		)
            {
              if (!quiet_flag)
                fprintf ((&_iob[2]) , "%s: warning: file `%s' already saved in `%s'\n",
			 pname,
			 shortpath (0 , convert_filename),
			 shortpath (0 , new_filename));
            }
          else
            {
              fprintf ((&_iob[2]) , "%s: can't link file `%s' to `%s': %s\n",
		       pname,
		       shortpath (0 , convert_filename),
		       shortpath (0 , new_filename),
		       sys_errlist[errno]);
              return;
            }
        }
    }
  if (	unlink((char *)convert_filename)  == -1)
    {
      fprintf ((&_iob[2]) , "%s: can't delete file `%s': %s\n",
	       pname, shortpath (0 , convert_filename), sys_errlist[errno]);
      return;
    }
  {
    int output_file;
    if ((output_file = creat (convert_filename, 0666)) == -1)
      {
        fprintf ((&_iob[2]) , "%s: can't create/open output file `%s': %s\n",
		 pname, shortpath (0 , convert_filename),
		 sys_errlist[errno]);
        return;
      }
    {
      unsigned int out_size = (repl_write_ptr + 1) - repl_text_base;
      if (write (output_file, repl_text_base, out_size) != out_size)
        fprintf ((&_iob[2]) , "%s: error writing file `%s': %s\n",
		 pname, shortpath (0 , convert_filename),
		 sys_errlist[errno]);
    }
    close (output_file);
  }
  free (new_orig_text_base);
  free (new_clean_text_base);
  free (repl_text_base);
  if (chmod((char *)(char *)convert_filename,  stat_buf.st_mode)  == -1)
    fprintf ((&_iob[2]) , "%s: can't change mode of file `%s': %s\n",
	     pname, shortpath (0 , convert_filename), sys_errlist[errno]);
}
static void
do_processing ()
{
  const char * const *base_pp;
  const char * const * const end_pps
    = &base_source_filenames[n_base_source_files];
  int syscalls_len;
  for (base_pp = base_source_filenames; base_pp < end_pps; base_pp++)
    process_aux_info_file (*base_pp, keep_flag, 0);
  if (nondefault_syscalls_dir)
    {
      syscalls_absolute_filename
        = (char *) xmalloc (strlen (nondefault_syscalls_dir)
                            + sizeof (syscalls_filename) + 1);
      strcpy (syscalls_absolute_filename, nondefault_syscalls_dir);
    }
  else
    {
      syscalls_absolute_filename
        = (char *) xmalloc (strlen (default_syscalls_dir)
                            + sizeof (syscalls_filename) + 1);
      strcpy (syscalls_absolute_filename, default_syscalls_dir);
    }
  syscalls_len = strlen (syscalls_absolute_filename);
  if (*(syscalls_absolute_filename + syscalls_len - 1) != '/')
    {
      *(syscalls_absolute_filename + syscalls_len++) = '/';
      *(syscalls_absolute_filename + syscalls_len) = '\0';
    }
  strcat (syscalls_absolute_filename, syscalls_filename);
  process_aux_info_file (syscalls_absolute_filename, 1, 1);
  visit_each_hash_node (filename_primary, reverse_def_dec_list);
  visit_each_hash_node (function_name_primary, connect_defs_and_decs);
  visit_each_hash_node (filename_primary, edit_file);
  if (cplusplus_flag && !nochange_flag)
    visit_each_hash_node (filename_primary, rename_c_file);
}
static struct option longopts[] =
{
  {"version", 0, 0, 'V'},
  {"file_name", 0, 0, 'p'},
  {"quiet", 0, 0, 'q'},
  {"silent", 0, 0, 'q'},
  {"force", 0, 0, 'f'},
  {"keep", 0, 0, 'k'},
  {"nosave", 0, 0, 'N'},
  {"nochange", 0, 0, 'n'},
  {"compiler-options", 1, 0, 'c'},
  {"exclude", 1, 0, 'x'},
  {"directory", 1, 0, 'd'},
  {"local", 0, 0, 'l'},
  {"global", 0, 0, 'g'},
  {"c++", 0, 0, 'C'},
  {"syscalls-dir", 1, 0, 'B'},
  {0, 0, 0, 0}
};
int
main (argc, argv)
     int argc;
     char **const argv;
{
  int longind;
  int c;
  const char *params = "";
  pname = rindex (argv[0], '/');
  pname = pname ? pname+1 : argv[0];
  cwd_buffer = getpwd ();
  if (!cwd_buffer)
    {
      fprintf ((&_iob[2]) , "%s: cannot get working directory: %s\n",
	       pname, sys_errlist[errno]);
      exit (1);
    }
  directory_list = string_list_cons (cwd_buffer, 0 );
  while ((c = getopt_long (argc, argv,
			   "B:c:Cd:gklnNp:qvVx:",
			   longopts, &longind)) != (-1) )
    {
      if (c == 0)		 
	c = longopts[longind].val;
      switch (c)
	{
	case 'p':
	  compiler_file_name = optarg;
	  break;
	case 'd':
	  directory_list
	    = string_list_cons (abspath (0 , optarg), directory_list);
	  break;
	case 'x':
	  exclude_list = string_list_cons (optarg, exclude_list);
	  break;
	case 'v':
	case 'V':
	  version_flag = 1;
	  break;
	case 'q':
	  quiet_flag = 1;
	  break;
	case 'n':
	  nochange_flag = 1;
	  keep_flag = 1;
	  break;
	case 'N':
	  nosave_flag = 1;
	  break;
	case 'k':
	  keep_flag = 1;
	  break;
	case 'c':
	  params = optarg;
	  break;
	case 'l':
	  local_flag = 1;
	  break;
	case 'g':
	  global_flag = 1;
	  break;
	case 'C':
	  cplusplus_flag = 1;
	  break;
	case 'B':
	  nondefault_syscalls_dir = optarg;
	  break;
	default:
	  usage ();
	}
    }
  munge_compile_params (params);
  n_base_source_files = argc - optind;
  base_source_filenames =
    (const char **) xmalloc ((n_base_source_files + 1) * sizeof (char *));
  n_base_source_files = 0;
  for (; optind < argc; optind++)
    {
      const char *path = abspath (0 , argv[optind]);
      int len = strlen (path);
      if (path[len-1] == 'c' && path[len-2] == '.')
	base_source_filenames[n_base_source_files++] = path;
      else
	{
	  fprintf ((&_iob[2]) , "%s: input file names must have .c suffixes: %s\n",
		   pname, shortpath (0 , path));
	  errors++;
	}
    }
  {
    const char *cp;
    for (cp = varargs_style_indicator; ((_pctype+1)[*cp]&(0001	|0002	|0004	))  || *cp == '_'; cp++)
      continue;
    if (*cp != 0)
      varargs_style_indicator = savestring (varargs_style_indicator,
					    cp - varargs_style_indicator);
  }
  if (errors)
    usage ();
  else
    {
      if (version_flag)
        fprintf ((&_iob[2]) , "%s: %s\n", pname, version_string);
      do_processing ();
    }
  if (errors)
    exit (1);
  else
    exit (0);
  return 1;
}
