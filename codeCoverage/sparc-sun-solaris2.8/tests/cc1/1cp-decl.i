typedef unsigned long   size_t;
typedef long    fpos_t;
typedef struct {
	int	_cnt;
	unsigned char	*_ptr;
	unsigned char	*_base;
	int	_bufsiz;
	short	_flag;
	short	_file;
	char	*__newbase;
	void	*_lock;			
	unsigned char	*_bufendp;
} FILE;
extern FILE	_iob[];
extern int     fread();
extern int     fwrite();
extern int	_flsbuf ();
extern int	_filbuf ();
extern int 	ferror ();
extern int 	feof ();
extern void 	clearerr ();
extern int 	putchar ();
extern int 	getchar ();
extern int 	putc ();
extern int 	getc ();
extern int	remove ();
extern int	rename ();
extern FILE 	*tmpfile ();
extern char 	*tmpnam ();
extern int 	fclose ();
extern int 	fflush ();
extern FILE	*fopen ();
extern FILE 	*freopen ();
extern void 	setbuf ();
extern int 	setvbuf ();
extern int	fprintf ();
extern int	fscanf ();
extern int	printf ();
extern int	scanf ();
extern int	sprintf ();
extern int	sscanf ();
typedef struct {
	char	**_a0;		
	int	_offset;		
} va_list;
extern int  vfprintf ();
extern int  vprintf ();
extern int  vsprintf ();
extern int 	fgetc ();
extern char 	*fgets ();
extern int 	fputc ();
extern int 	fputs ();
extern char 	*gets ();
extern int 	puts ();
extern int	ungetc ();
extern int	fgetpos ();
extern int 	fseek ();
extern int	fsetpos ();
extern long	ftell ();
extern void	rewind ();
extern void 	perror ();
typedef signed long     ptrdiff_t;
    typedef unsigned int  wchar_t;
typedef unsigned int wctype_t;
typedef int            time_t;
typedef int             clock_t;
typedef long                    ssize_t; 
typedef	unsigned char	uchar_t;
typedef	unsigned short	ushort_t;
typedef	unsigned int	uint_t;
typedef unsigned long	ulong_t;
typedef	volatile unsigned char	vuchar_t;
typedef	volatile unsigned short	vushort_t;
typedef	volatile unsigned int	vuint_t;
typedef volatile unsigned long	vulong_t;
typedef	struct	{ long r[1]; } *physadr_t;
typedef	struct	label_t	{
	long	val[10];
} label_t;
typedef int		level_t;
typedef	int		daddr_t;	
typedef	char *		caddr_t;	
typedef long *		qaddr_t;        
typedef char *          addr_t;
typedef	uint_t		ino_t;		
typedef short		cnt_t;
typedef int		dev_t;		
typedef	int		chan_t;		
typedef long    off_t;			
typedef unsigned long	rlim_t;		
typedef	int		paddr_t;
typedef	ushort_t	nlink_t;
typedef int    		key_t;		
typedef	uint_t		mode_t;		
typedef uint_t		uid_t;		
typedef uint_t		gid_t;		
typedef	void *		mid_t;		
typedef	int		pid_t;		
typedef char		slab_t[12];	
typedef ulong_t		shmatt_t;	
typedef ulong_t		msgqnum_t;	
typedef ulong_t		msglen_t;	
        typedef unsigned int wint_t;         
typedef unsigned long	sigset_t;
typedef long            timer_t;        
typedef void (*sig_t) ();
typedef pid_t		id_t;		
typedef uint_t	major_t;      
typedef uint_t	minor_t;      
typedef uint_t	devs_t;       
typedef uint_t	unit_t;       
typedef	unsigned long	vm_offset_t;
typedef	unsigned long	vm_size_t;
typedef	uchar_t		uchar;
typedef	ushort_t	ushort;
typedef	uint_t		uint;
typedef ulong_t		ulong;
typedef	physadr_t	physadr;
typedef	uchar_t		u_char;
typedef	ushort_t 	u_short;
typedef	uint_t		u_int;
typedef	ulong_t		u_long;
typedef	vuchar_t	vu_char;
typedef	vushort_t 	vu_short;
typedef	vuint_t		vu_int;
typedef	vulong_t	vu_long;
typedef struct  _quad { int val[2]; } quad;
typedef	long	swblk_t;
typedef u_long	fixpt_t;
typedef int	fd_mask;
typedef	struct fd_set {
	fd_mask	fds_bits[(((4096)+(( (sizeof(fd_mask) * 8		)	)-1))/( (sizeof(fd_mask) * 8		)	))];
} fd_set;
extern void  bzero (); 
struct timeval;
int select ();
extern int 	fileno ();
extern FILE 	*fdopen ();
extern char *cuserid ();
extern int getopt ();
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern char	*ctermid ();
extern int 	getw ();
extern int 	pclose ();
extern int 	putw ();
extern FILE 	*popen ();
extern char 	*tempnam ();
extern void 	setbuffer ();
extern void 	setlinebuf ();
extern int target_flags;
enum reg_class { NO_REGS, GENERAL_REGS, FLOAT_REGS, ALL_REGS,
		 LIM_REG_CLASSES };
extern struct rtx_def *alpha_builtin_saveregs ();
extern struct rtx_def *alpha_compare_op0, *alpha_compare_op1;
extern int alpha_compare_fp_p;
extern char *alpha_function_name;
extern char *current_function_name;
enum machine_mode {
VOIDmode,
QImode,		
HImode,
PSImode,
SImode,
PDImode,
DImode,
TImode,
SFmode,
DFmode,
XFmode,   
TFmode,
SCmode,
DCmode,
XCmode,
TCmode,
BLKmode,
CCmode,
MAX_MACHINE_MODE };
extern char *mode_name[];
enum mode_class { MODE_RANDOM, MODE_INT, MODE_FLOAT, MODE_PARTIAL_INT, MODE_CC,
		  MODE_COMPLEX_INT, MODE_COMPLEX_FLOAT, MAX_MODE_CLASS};
extern enum mode_class mode_class[];
extern int mode_size[];
extern int mode_unit_size[];
extern enum machine_mode mode_wider_mode[];
extern enum machine_mode mode_for_size ();
extern enum machine_mode get_best_mode ();
extern enum machine_mode class_narrowest_mode[];
extern enum machine_mode byte_mode;
extern enum machine_mode word_mode;
enum tree_code {
ERROR_MARK,
IDENTIFIER_NODE,
OP_IDENTIFIER,
TREE_LIST,
TREE_VEC,
BLOCK,
VOID_TYPE,	
INTEGER_TYPE,
REAL_TYPE,
COMPLEX_TYPE,
ENUMERAL_TYPE,
BOOLEAN_TYPE,
CHAR_TYPE,
POINTER_TYPE,
OFFSET_TYPE,
REFERENCE_TYPE,
METHOD_TYPE,
FILE_TYPE,
ARRAY_TYPE,
SET_TYPE,
STRING_TYPE,
RECORD_TYPE,
UNION_TYPE,	
FUNCTION_TYPE,
LANG_TYPE,
INTEGER_CST,
REAL_CST,
COMPLEX_CST,
STRING_CST,
FUNCTION_DECL,
LABEL_DECL,
CONST_DECL,
TYPE_DECL,
VAR_DECL,
PARM_DECL,
RESULT_DECL,
FIELD_DECL,
COMPONENT_REF,
BIT_FIELD_REF,
INDIRECT_REF,
OFFSET_REF,
BUFFER_REF,
ARRAY_REF,
CONSTRUCTOR,
COMPOUND_EXPR,
MODIFY_EXPR,
INIT_EXPR,
TARGET_EXPR,
COND_EXPR,
BIND_EXPR,
CALL_EXPR,
METHOD_CALL_EXPR,
WITH_CLEANUP_EXPR,
PLUS_EXPR,
MINUS_EXPR,
MULT_EXPR,
TRUNC_DIV_EXPR,
CEIL_DIV_EXPR,
FLOOR_DIV_EXPR,
ROUND_DIV_EXPR,
TRUNC_MOD_EXPR,
CEIL_MOD_EXPR,
FLOOR_MOD_EXPR,
ROUND_MOD_EXPR,
RDIV_EXPR,
EXACT_DIV_EXPR,
FIX_TRUNC_EXPR,
FIX_CEIL_EXPR,
FIX_FLOOR_EXPR,
FIX_ROUND_EXPR,
FLOAT_EXPR,
EXPON_EXPR,
NEGATE_EXPR,
MIN_EXPR,
MAX_EXPR,
ABS_EXPR,
FFS_EXPR,
LSHIFT_EXPR,
RSHIFT_EXPR,
LROTATE_EXPR,
RROTATE_EXPR,
BIT_IOR_EXPR,
BIT_XOR_EXPR,
BIT_AND_EXPR,
BIT_ANDTC_EXPR,
BIT_NOT_EXPR,
TRUTH_ANDIF_EXPR,
TRUTH_ORIF_EXPR,
TRUTH_AND_EXPR,
TRUTH_OR_EXPR,
TRUTH_NOT_EXPR,
LT_EXPR,
LE_EXPR,
GT_EXPR,
GE_EXPR,
EQ_EXPR,
NE_EXPR,
IN_EXPR,
SET_LE_EXPR,
CARD_EXPR,
RANGE_EXPR,
CONVERT_EXPR,
NOP_EXPR,
NON_LVALUE_EXPR,
SAVE_EXPR,
RTL_EXPR,
ADDR_EXPR,
REFERENCE_EXPR,
ENTRY_VALUE_EXPR,
COMPLEX_EXPR,
CONJ_EXPR,
REALPART_EXPR,
IMAGPART_EXPR,
PREDECREMENT_EXPR,
PREINCREMENT_EXPR,
POSTDECREMENT_EXPR,
POSTINCREMENT_EXPR,
LABEL_EXPR,
GOTO_EXPR,
RETURN_EXPR,
EXIT_EXPR,
LOOP_EXPR,
  LAST_AND_UNUSED_TREE_CODE	
};
extern char **tree_code_type;
extern int *tree_code_length;
extern char **tree_code_name;
enum built_in_function
{
  NOT_BUILT_IN,
  BUILT_IN_ALLOCA,
  BUILT_IN_ABS,
  BUILT_IN_FABS,
  BUILT_IN_LABS,
  BUILT_IN_FFS,
  BUILT_IN_DIV,
  BUILT_IN_LDIV,
  BUILT_IN_FFLOOR,
  BUILT_IN_FCEIL,
  BUILT_IN_FMOD,
  BUILT_IN_FREM,
  BUILT_IN_MEMCPY,
  BUILT_IN_MEMCMP,
  BUILT_IN_MEMSET,
  BUILT_IN_STRCPY,
  BUILT_IN_STRCMP,
  BUILT_IN_STRLEN,
  BUILT_IN_FSQRT,
  BUILT_IN_SIN,
  BUILT_IN_COS,
  BUILT_IN_GETEXP,
  BUILT_IN_GETMAN,
  BUILT_IN_SAVEREGS,
  BUILT_IN_CLASSIFY_TYPE,
  BUILT_IN_NEXT_ARG,
  BUILT_IN_ARGS_INFO,
  BUILT_IN_CONSTANT_P,
  BUILT_IN_FRAME_ADDRESS,
  BUILT_IN_RETURN_ADDRESS,
  BUILT_IN_CALLER_RETURN_ADDRESS,
  BUILT_IN_NEW,
  BUILT_IN_VEC_NEW,
  BUILT_IN_DELETE,
  BUILT_IN_VEC_DELETE
};
typedef union tree_node *tree;
struct tree_common
{
  union tree_node *chain;
  union tree_node *type;
  enum tree_code code : 8;
  unsigned side_effects_flag : 1;
  unsigned constant_flag : 1;
  unsigned permanent_flag : 1;
  unsigned addressable_flag : 1;
  unsigned volatile_flag : 1;
  unsigned readonly_flag : 1;
  unsigned unsigned_flag : 1;
  unsigned asm_written_flag: 1;
  unsigned used_flag : 1;
  unsigned raises_flag : 1;
  unsigned static_flag : 1;
  unsigned public_flag : 1;
  unsigned private_flag : 1;
  unsigned protected_flag : 1;
  unsigned lang_flag_0 : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
  unsigned lang_flag_5 : 1;
  unsigned lang_flag_6 : 1;
};
struct tree_int_cst
{
  char common[sizeof (struct tree_common)];
  int int_cst_low;
  int int_cst_high;
};
extern double ldexp ();
extern double (atof) ();
extern double real_value_truncate ();
extern double dconst0;
extern double dconst1;
extern double dconst2;
extern double dconstm1;
union real_extract 
{
  double d;
  int i[sizeof (double) / sizeof (int)];
};
double real_value_from_int_cst ();
struct tree_real_cst
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	
  double real_cst;
};
struct tree_string
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	
  int length;
  char *pointer;
};
struct tree_complex
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	
  union tree_node *real;
  union tree_node *imag;
};
struct tree_identifier
{
  char common[sizeof (struct tree_common)];
  int length;
  char *pointer;
};
struct tree_list
{
  char common[sizeof (struct tree_common)];
  union tree_node *purpose;
  union tree_node *value;
};
struct tree_vec
{
  char common[sizeof (struct tree_common)];
  int length;
  union tree_node *a[1];
};
struct tree_exp
{
  char common[sizeof (struct tree_common)];
  int complexity;
  union tree_node *operands[1];
};
struct tree_block
{
  char common[sizeof (struct tree_common)];
  unsigned handler_block_flag : 1;
  unsigned abstract_flag : 1;
  union tree_node *vars;
  union tree_node *type_tags;
  union tree_node *subblocks;
  union tree_node *supercontext;
  union tree_node *abstract_origin;
  struct rtx_def *end_note;
};
struct tree_type
{
  char common[sizeof (struct tree_common)];
  union tree_node *values;
  union tree_node *size;
  unsigned uid;
  enum machine_mode mode : 8;
  unsigned char precision;
  unsigned no_force_blk_flag : 1;
  unsigned lang_flag_0 : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
  unsigned lang_flag_5 : 1;
  unsigned lang_flag_6 : 1;
  unsigned int align;
  union tree_node *pointer_to;
  union tree_node *reference_to;
  int parse_info;
  int symtab_address;
  union tree_node *name;
  union tree_node *minval;
  union tree_node *maxval;
  union tree_node *next_variant;
  union tree_node *main_variant;
  union tree_node *binfo;
  union tree_node *noncopied_parts;
  union tree_node *context;
  struct lang_type *lang_specific;
};
struct tree_decl
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *size;
  unsigned int uid;
  enum machine_mode mode : 8;
  unsigned external_flag : 1;
  unsigned nonlocal_flag : 1;
  unsigned regdecl_flag : 1;
  unsigned inline_flag : 1;
  unsigned bit_field_flag : 1;
  unsigned virtual_flag : 1;
  unsigned ignored_flag : 1;
  unsigned abstract_flag : 1;
  unsigned in_system_header_flag : 1;
  unsigned lang_flag_0 : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
  unsigned lang_flag_5 : 1;
  unsigned lang_flag_6 : 1;
  unsigned lang_flag_7 : 1;
  union tree_node *name;
  union tree_node *context;
  union tree_node *arguments;
  union tree_node *result;
  union tree_node *initial;
  union tree_node *abstract_origin;
  char *print_name;
  union tree_node *assembler_name;
  struct rtx_def *rtl;	
  int frame_size;
  union {
    struct rtx_def *r;
    int i;
  } saved_insns;
  union tree_node *vindex;
  struct lang_decl *lang_specific;
};
union tree_node
{
  struct tree_common common;
  struct tree_int_cst int_cst;
  struct tree_real_cst real_cst;
  struct tree_string string;
  struct tree_complex complex;
  struct tree_identifier identifier;
  struct tree_decl decl;
  struct tree_type type;
  struct tree_list list;
  struct tree_vec vec;
  struct tree_exp exp;
  struct tree_block block;
 };
extern char *xrealloc ();
extern char *oballoc			();
extern char *permalloc			();
extern char *savealloc			();
extern char *xmalloc			();
extern void free			();
extern tree make_node			();
extern tree copy_node			();
extern tree copy_list			();
extern tree make_tree_vec		();
extern tree get_identifier		();
extern tree build ();
extern tree build_nt ();
extern tree build_parse_node ();
extern tree build_int_2_wide		();
extern tree build_real			();
extern tree build_real_from_int_cst 	();
extern tree build_complex		();
extern tree build_string		();
extern tree build1			();
extern tree build_tree_list		();
extern tree build_decl_list		();
extern tree build_decl			();
extern tree build_block			();
extern tree make_signed_type		();
extern tree make_unsigned_type		();
extern tree signed_or_unsigned_type 	();
extern void fixup_unsigned_type		();
extern tree build_pointer_type		();
extern tree build_reference_type 	();
extern tree build_index_type		();
extern tree build_index_2_type		();
extern tree build_array_type		();
extern tree build_function_type		();
extern tree build_method_type		();
extern tree build_offset_type		();
extern tree build_complex_type		();
extern tree array_type_nelts		();
extern tree build_binary_op ();
extern tree build_indirect_ref ();
extern tree build_unary_op		();
extern tree make_tree ();
extern tree build_type_variant		();
extern tree build_type_copy		();
extern void layout_type			();
extern tree type_hash_canon		();
extern void layout_decl			();
extern tree fold			();
extern tree non_lvalue			();
extern tree convert			();
extern tree size_in_bytes		();
extern int int_size_in_bytes		();
extern tree size_binop			();
extern tree size_int			();
extern tree round_up			();
extern tree get_pending_sizes		();
extern tree sizetype;
extern tree chainon			();
extern tree tree_cons			();
extern tree perm_tree_cons		();
extern tree temp_tree_cons		();
extern tree saveable_tree_cons		();
extern tree decl_tree_cons		();
extern tree tree_last			();
extern tree nreverse			();
extern int list_length			();
extern int integer_zerop		();
extern int integer_onep			();
extern int integer_all_onesp		();
extern int integer_pow2p		();
extern int staticp			();
extern int lvalue_or_else		();
extern tree save_expr			();
extern tree variable_size		();
extern tree stabilize_reference		();
extern tree get_unwidened		();
extern tree get_narrower		();
extern tree type_for_mode		();
extern tree type_for_size		();
extern tree unsigned_type		();
extern tree signed_type			();
extern tree maybe_build_cleanup		();
extern tree get_inner_reference		();
extern tree decl_function_context 	();
extern tree decl_type_context		();
extern char *function_cannot_inline_p 	();
extern tree integer_zero_node;
extern tree integer_one_node;
extern tree size_zero_node;
extern tree size_one_node;
extern tree null_pointer_node;
extern tree error_mark_node;
extern tree void_type_node;
extern tree integer_type_node;
extern tree unsigned_type_node;
extern tree char_type_node;
extern char *input_filename;
extern int lineno;
extern int pedantic;
extern int immediate_size_expand;
extern tree current_function_decl;
extern int current_function_calls_setjmp;
extern int current_function_calls_longjmp;
extern int all_types_permanent;
extern char *(*decl_printable_name) ();
extern char *perm_calloc			();
extern tree expand_start_stmt_expr		();
extern tree expand_end_stmt_expr		();
extern void expand_expr_stmt			();
extern void clear_last_expr			();
extern void expand_label			();
extern void expand_goto				();
extern void expand_asm				();
extern void expand_start_cond			();
extern void expand_end_cond			();
extern void expand_start_else			();
extern void expand_start_elseif			();
extern struct nesting *expand_start_loop 	();
extern struct nesting *expand_start_loop_continue_elsewhere 	();
extern void expand_loop_continue_here		();
extern void expand_end_loop			();
extern int expand_continue_loop			();
extern int expand_exit_loop			();
extern int expand_exit_loop_if_false		();
extern int expand_exit_something		();
extern void expand_null_return			();
extern void expand_return			();
extern void expand_start_bindings		();
extern void expand_end_bindings			();
extern tree last_cleanup_this_contour		();
extern void expand_start_case			();
extern void expand_end_case			();
extern int pushcase				();
extern int pushcase_range			();
extern tree invert_truthvalue			();
extern void init_lex				();
extern void init_decl_processing		();
extern void lang_init				();
extern void lang_finish				();
extern int yyparse				();
extern int lang_decode_option			();
extern void pushlevel				();
extern tree poplevel				();
extern void set_block				();
extern tree pushdecl				();
extern tree getdecls				();
extern tree gettags				();
enum rtx_code  {
UNKNOWN ,
NIL ,
EXPR_LIST ,
INSN_LIST ,
MATCH_OPERAND ,
MATCH_SCRATCH ,
MATCH_DUP ,
MATCH_OPERATOR ,
MATCH_PARALLEL ,
MATCH_OP_DUP ,
MATCH_PAR_DUP ,
DEFINE_INSN ,
DEFINE_PEEPHOLE ,
DEFINE_SPLIT ,
DEFINE_COMBINE ,
DEFINE_EXPAND ,
DEFINE_DELAY ,
DEFINE_FUNCTION_UNIT ,
DEFINE_ASM_ATTRIBUTES ,
SEQUENCE ,
ADDRESS ,
DEFINE_ATTR ,
ATTR ,
SET_ATTR ,
SET_ATTR_ALTERNATIVE ,
EQ_ATTR ,
INSN ,
JUMP_INSN ,
CALL_INSN ,
BARRIER ,
CODE_LABEL ,
NOTE ,
INLINE_HEADER ,
PARALLEL ,
ASM_INPUT ,
ASM_OPERANDS ,
UNSPEC ,
UNSPEC_VOLATILE ,
ADDR_VEC ,
ADDR_DIFF_VEC ,
SET ,
USE ,
CLOBBER ,
CALL ,
RETURN ,
TRAP_IF ,
CONST_INT ,
CONST_DOUBLE ,
CONST_STRING ,
CONST ,
PC ,
REG ,
SCRATCH ,
SUBREG ,
STRICT_LOW_PART ,
MEM ,
LABEL_REF ,
SYMBOL_REF ,
CC0 ,
QUEUED ,
IF_THEN_ELSE ,
COND ,
COMPARE ,
PLUS ,
MINUS ,
NEG ,
MULT ,
DIV ,
MOD ,
UDIV ,
UMOD ,
AND ,
IOR ,
XOR ,
NOT ,
LSHIFT ,
ASHIFT ,
ROTATE ,
ASHIFTRT ,
LSHIFTRT ,
ROTATERT ,
SMIN ,
SMAX ,
UMIN ,
UMAX ,
PRE_DEC ,
PRE_INC ,
POST_DEC ,
POST_INC ,
NE ,
EQ ,
GE ,
GT ,
LE ,
LT ,
GEU ,
GTU ,
LEU ,
LTU ,
SIGN_EXTEND ,
ZERO_EXTEND ,
TRUNCATE ,
FLOAT_EXTEND ,
FLOAT_TRUNCATE ,
FLOAT ,
FIX ,
UNSIGNED_FLOAT ,
UNSIGNED_FIX ,
ABS ,
SQRT ,
FFS ,
SIGN_EXTRACT ,
ZERO_EXTRACT ,
HIGH ,
LO_SUM ,
  LAST_AND_UNUSED_RTX_CODE};	
extern int rtx_length[];
extern char *rtx_name[];
extern char *rtx_format[];
extern char rtx_class[];
typedef union rtunion_def
{
  int rtwint;
  int rtint;
  char *rtstr;
  struct rtx_def *rtx;
  struct rtvec_def *rtvec;
  enum machine_mode rttype;
} rtunion;
typedef struct rtx_def
{
  enum rtx_code code : 16;
  enum machine_mode mode : 8;
  unsigned int jump : 1;
  unsigned int call : 1;
  unsigned int unchanging : 1;
  unsigned int volatil : 1;
  unsigned int in_struct : 1;
  unsigned int used : 1;
  unsigned integrated : 1;
  rtunion fld[1];
} *rtx;
typedef struct rtvec_def{
  unsigned num_elem;		
  rtunion elem[1];
} *rtvec;
enum reg_note { REG_DEAD = 1, REG_INC = 2, REG_EQUIV = 3, REG_WAS_0 = 4,
		REG_EQUAL = 5, REG_RETVAL = 6, REG_LIBCALL = 7,
		REG_NONNEG = 8, REG_NO_CONFLICT = 9, REG_UNUSED = 10,
		REG_CC_SETTER = 11, REG_CC_USER = 12, REG_LABEL = 13,
		REG_DEP_ANTI = 14, REG_DEP_OUTPUT = 15 };
extern char *reg_note_name[];
extern char *note_insn_name[];
extern rtx plus_constant_wide		 ();
extern rtx plus_constant_for_output_wide ();
extern rtx gen_rtx ();
extern rtvec gen_rtvec ();
extern rtx read_rtx			();
extern char *xrealloc ();
extern char *xmalloc			();
extern char *oballoc			();
extern char *permalloc			();
extern void free			();
extern rtx rtx_alloc			();
extern rtvec rtvec_alloc		();
extern rtx find_reg_note		();
extern rtx find_regno_note		();
extern int get_integer_term	();
extern rtx get_related_value		();
extern rtx single_set			();
extern rtx find_last_value		();
extern rtx copy_rtx			();
extern rtx copy_rtx_if_shared		();
extern rtx copy_most_rtx		();
extern rtx replace_rtx			();
extern rtvec gen_rtvec_v		();
extern rtx gen_reg_rtx			();
extern rtx gen_label_rtx		();
extern rtx gen_inline_header_rtx	();
extern rtx gen_lowpart_common		();
extern rtx gen_lowpart			();
extern rtx gen_lowpart_if_possible	();
extern rtx gen_highpart			();
extern rtx gen_realpart			();
extern rtx gen_imagpart			();
extern rtx operand_subword		();
extern rtx operand_subword_force	();
extern int subreg_lowpart_p		();
extern rtx make_safe_from		();
extern rtx memory_address		();
extern rtx get_insns			();
extern rtx get_last_insn		();
extern rtx get_last_insn_anywhere	();
extern void start_sequence		();
extern void push_to_sequence		();
extern void end_sequence		();
extern rtx gen_sequence			();
extern rtx immed_double_const		();
extern rtx force_const_mem		();
extern rtx force_reg			();
extern rtx get_pool_constant		();
extern enum machine_mode get_pool_mode	();
extern int get_pool_offset		();
extern rtx simplify_subtraction		();
extern rtx assign_stack_local		();
extern rtx assign_stack_temp		();
extern rtx protect_from_queue		();
extern void emit_queue			();
extern rtx emit_move_insn		();
extern rtx emit_insn_before		();
extern rtx emit_jump_insn_before	();
extern rtx emit_call_insn_before	();
extern rtx emit_barrier_before		();
extern rtx emit_note_before		();
extern rtx emit_insn_after		();
extern rtx emit_jump_insn_after		();
extern rtx emit_barrier_after		();
extern rtx emit_label_after		();
extern rtx emit_note_after		();
extern rtx emit_line_note_after		();
extern rtx emit_insn			();
extern rtx emit_insns			();
extern rtx emit_insns_before		();
extern rtx emit_jump_insn		();
extern rtx emit_call_insn		();
extern rtx emit_label			();
extern rtx emit_barrier			();
extern rtx emit_line_note		();
extern rtx emit_note			();
extern rtx emit_line_note_force		();
extern rtx make_insn_raw		();
extern rtx previous_insn		();
extern rtx next_insn			();
extern rtx prev_nonnote_insn		();
extern rtx next_nonnote_insn		();
extern rtx prev_real_insn		();
extern rtx next_real_insn		();
extern rtx prev_active_insn		();
extern rtx next_active_insn		();
extern rtx prev_label			();
extern rtx next_label			();
extern rtx next_cc0_user		();
extern rtx prev_cc0_setter		();
extern rtx reg_set_last			();
extern rtx next_nondeleted_insn		();
extern enum rtx_code reverse_condition	();
extern enum rtx_code swap_condition	();
extern enum rtx_code unsigned_condition	();
extern enum rtx_code signed_condition	();
extern rtx find_equiv_reg		();
extern rtx squeeze_notes		();
extern rtx delete_insn			();
extern void delete_jump			();
extern rtx get_label_before		();
extern rtx get_label_after		();
extern rtx follow_jumps			();
extern rtx adj_offsettable_operand	();
extern rtx try_split			();
extern rtx split_insns			();
extern rtx simplify_unary_operation	();
extern rtx simplify_binary_operation	();
extern rtx simplify_ternary_operation	();
extern rtx simplify_relational_operation ();
extern rtx nonlocal_label_rtx_list	();
extern rtx gen_move_insn		();
extern rtx gen_jump			();
extern rtx gen_beq			();
extern rtx gen_bge			();
extern rtx gen_ble			();
extern rtx eliminate_constant_term	();
extern rtx expand_complex_abs		();
extern int max_parallel;
extern int asm_noperands		();
extern char *decode_asm_operands	();
extern enum reg_class reg_preferred_class ();
extern enum reg_class reg_alternate_class ();
extern rtx get_first_nonparm_insn	();
extern rtx pc_rtx;
extern rtx cc0_rtx;
extern rtx const0_rtx;
extern rtx const1_rtx;
extern rtx const2_rtx;
extern rtx constm1_rtx;
extern rtx const_true_rtx;
extern rtx const_tiny_rtx[3][(int) MAX_MACHINE_MODE];
extern rtx stack_pointer_rtx;
extern rtx frame_pointer_rtx;
extern rtx arg_pointer_rtx;
extern rtx pic_offset_table_rtx;
extern rtx struct_value_rtx;
extern rtx struct_value_incoming_rtx;
extern rtx static_chain_rtx;
extern rtx static_chain_incoming_rtx;
extern rtx virtual_incoming_args_rtx;
extern rtx virtual_stack_vars_rtx;
extern rtx virtual_stack_dynamic_rtx;
extern rtx virtual_outgoing_args_rtx;
extern rtx find_next_ref		();
extern rtx *find_single_use		();
extern rtx expand_expr ();
extern rtx immed_real_const_1();
extern rtx  output_constant_def ();
extern rtx  immed_real_const	();
extern rtx  immed_real_const_1	();
extern tree make_tree		();
extern int reload_completed;
extern int reload_in_progress;
extern int cse_not_expected;
extern rtx *regno_reg_rtx;
extern char *main_input_filename;
enum debug_info_type
{
  NO_DEBUG,	    
  DBX_DEBUG,	    
  SDB_DEBUG,	    
  DWARF_DEBUG,	    
  XCOFF_DEBUG	    
};
extern enum debug_info_type write_symbols;
enum debug_info_level
{
  DINFO_LEVEL_NONE,	
  DINFO_LEVEL_TERSE,	
  DINFO_LEVEL_NORMAL,	
  DINFO_LEVEL_VERBOSE	
};
extern enum debug_info_level debug_info_level;
extern int use_gnu_debug_info_extensions;
extern int optimize;
extern int obey_regdecls;
extern int quiet_flag;
extern int inhibit_warnings;
extern int extra_warnings;
extern int warn_unused;
extern int warn_uninitialized;
extern int warn_shadow;
extern int warn_switch;
extern int warn_return_type;
extern int warn_cast_align;
extern int warn_id_clash;
extern int id_clash_len;
extern int warn_aggregate_return;
extern int profile_flag;
extern int profile_block_flag;
extern int pedantic;
extern int in_system_header;
extern int flag_print_asm_name;
extern int flag_signed_char;
extern int flag_short_enums;
extern int flag_caller_saves;
extern int flag_pcc_struct_return;
extern int flag_force_mem;
extern int flag_force_addr;
extern int flag_defer_pop;
extern int flag_float_store;
extern int flag_strength_reduce;
extern int flag_unroll_loops;
extern int flag_unroll_all_loops;
extern int flag_cse_follow_jumps;
extern int flag_cse_skip_blocks;
extern int flag_expensive_optimizations;
extern int flag_writable_strings;
extern int flag_no_function_cse;
extern int flag_omit_frame_pointer;
extern int flag_no_peephole;
extern int flag_volatile;
extern int flag_fast_math;
extern int flag_inline_functions;
extern int flag_keep_inline_functions;
extern int flag_no_inline;
extern int flag_syntax_only;
extern int flag_gen_aux_info;
extern int flag_shared_data;
extern int flag_schedule_insns;
extern int flag_schedule_insns_after_reload;
extern int flag_delayed_branch;
extern int flag_pretend_float;
extern int flag_pedantic_errors;
extern int flag_pic;
extern int flag_no_common;
extern int flag_inhibit_size_directive;
extern int flag_verbose_asm;
extern int flag_gnu_linker;
extern int frame_pointer_needed;
extern int can_reach_end;
extern int current_function_has_nonlocal_label;
struct lang_identifier
{
  struct tree_identifier ignore;
  tree global_value, local_value;
  tree class_value;
  tree class_template_info;
  struct lang_id2 *x;
};
struct lang_id2
{
  tree label_value, implicit_decl;
  tree type_desc, as_list, error_locus;
};
extern tree identifier_typedecl_value ();
extern int pedantic;
extern tree purpose_member (), value_member ();
extern tree binfo_member ();
extern tree build_component_ref (), build_conditional_expr ();
extern tree build_x_compound_expr (), build_compound_expr ();
extern tree build_unary_op (), build_binary_op (), build_function_call ();
extern tree build_binary_op_nodefault ();
extern tree build_indirect_ref (), build_array_ref (), build_c_cast ();
extern tree build_modify_expr ();
extern tree c_sizeof (), c_alignof ();
extern tree store_init_value ();
extern tree digest_init ();
extern tree c_expand_start_case ();
extern tree default_conversion ();
extern int comptypes (), compparms (), compexcepttypes ();
extern tree common_type ();
extern tree build_label ();
extern tree exception_throw_decl;
extern int start_function ();
extern void finish_function ();
extern void store_parm_decls ();
extern tree get_parm_info ();
extern void pushlevel ();
extern tree poplevel ();
extern tree groktypename(), lookup_name();
extern tree lookup_label(), define_label(), shadow_label ();
extern tree implicitly_declare(), getdecls(), gettags ();
extern tree start_decl();
extern void finish_decl();
extern tree start_struct(), finish_struct(), xref_tag(), xref_defn_tag();
extern tree finish_exception ();
extern tree grokfield(), grokbitfield ();
extern tree start_enum(), finish_enum();
extern tree build_enumerator();
extern tree make_index_type ();
extern tree make_anon_name ();
extern void cplus_decl_attributes ();
extern tree combine_strings ();
extern tree check_case_value ();
extern void binary_op_error ();
extern tree shorten_compare ();
extern tree truthvalue_conversion ();
extern tree double_type_node, long_double_type_node, float_type_node;
extern tree char_type_node, unsigned_char_type_node, signed_char_type_node;
extern tree ptrdiff_type_node;
extern tree short_integer_type_node, short_unsigned_type_node;
extern tree long_integer_type_node, long_unsigned_type_node;
extern tree long_long_integer_type_node, long_long_unsigned_type_node;
extern tree unsigned_type_node;
extern tree string_type_node, char_array_type_node, int_array_type_node;
extern tree wchar_array_type_node;
extern tree wchar_type_node, signed_wchar_type_node, unsigned_wchar_type_node;
extern tree intQI_type_node, unsigned_intQI_type_node;
extern tree intHI_type_node, unsigned_intHI_type_node;
extern tree intSI_type_node, unsigned_intSI_type_node;
extern tree intDI_type_node, unsigned_intDI_type_node;
extern int current_function_returns_value;
extern int current_function_returns_null;
extern tree current_function_return_value;
extern tree ridpointers[];
extern tree ansi_opname[];
extern tree ansi_assopname[];
extern int dollars_in_ident;
extern int flag_cond_mismatch;
extern int flag_no_asm;
extern int flag_gnu_xref;
extern int flag_gnu_binutils;
extern int flag_no_ident;
extern int warn_implicit;
extern int warn_return_type, explicit_warn_return_type;
extern int warn_write_strings;
extern int warn_pointer_arith;
extern int warn_strict_prototypes;
extern int warn_parentheses;
extern int warn_char_subscripts;
extern int warn_cast_qual;
extern int warn_traditional;
extern int warn_nonvdtor;
extern int flag_traditional;
extern int flag_signed_bitfields;
extern int write_virtuals;
extern int interface_only, interface_unknown;
extern int flag_elide_constructors;
extern int flag_handle_exceptions;
extern int flag_ansi_exceptions;
extern int flag_default_inline;
extern int flag_cadillac;
enum cplus_tree_code {
  __DUMMY = LAST_AND_UNUSED_TREE_CODE,
DELETE_EXPR,
SCOPE_REF,
MEMBER_REF,
TYPE_EXPR,
NEW_EXPR,
CPLUS_CATCH_DECL,
TEMPLATE_DECL,
TEMPLATE_TYPE_PARM,
TEMPLATE_CONST_PARM,
UNINSTANTIATED_P_TYPE,
  LAST_CPLUS_TREE_CODE
};
enum languages { lang_c, lang_cplusplus };
enum conversion_type { ptr_conv, constptr_conv, int_conv, real_conv, last_conversion_type };
struct lang_type
{
  struct
    {
      unsigned has_type_conversion : 1;
      unsigned has_int_conversion : 1;
      unsigned has_float_conversion : 1;
      unsigned has_init_ref : 1;
      unsigned gets_init_ref : 1;
      unsigned gets_init_aggr : 1;
      unsigned has_assignment : 1;
      unsigned gets_assignment : 1;
      unsigned needs_constructor : 1;
      unsigned has_default_ctor : 1;
      unsigned uses_multiple_inheritance : 1;
      unsigned const_needs_init : 1;
      unsigned ref_needs_init : 1;
      unsigned gets_const_init_ref : 1;
      unsigned has_const_assign_ref : 1;
      unsigned gets_const_assign_ref : 1;
      unsigned vtable_needs_writing : 1;
      unsigned has_assign_ref : 1;
      unsigned gets_assign_ref : 1;
      unsigned gets_new : 1;
      unsigned gets_delete : 1;
      unsigned has_call_overloaded : 1;
      unsigned has_array_ref_overloaded : 1;
      unsigned has_arrow_overloaded : 1;
      unsigned local_typedecls : 1;
      unsigned interface_only : 1;
      unsigned interface_unknown : 1;
      unsigned needs_virtual_reinit : 1;
      unsigned declared_exception : 1;
      unsigned declared_class : 1;
      unsigned being_defined : 1;
      unsigned redefined : 1;
      unsigned marked : 1;
      unsigned marked2 : 1;
      unsigned marked3 : 1;
      unsigned marked4 : 1;
      unsigned marked5 : 1;
      unsigned marked6 : 1;
      unsigned use_template : 2;
      unsigned debug_requested : 1;
      unsigned dynamic : 1;
      unsigned has_method_call_overloaded : 1;
      unsigned private_attr : 1;
      unsigned alters_visibilities : 1;
      unsigned got_semicolon : 1;
      unsigned dummy : 1;
      unsigned n_vancestors : 16;
    } type_flags;
  int cid;
  int n_ancestors;
  int vsize;
  int max_depth;
  union tree_node *vbinfo[2];
  union tree_node *baselink_vec;
  union tree_node *vfields;
  union tree_node *vbases;
  union tree_node *vbase_size;
  union tree_node *tags;
  char *memoized_table_entry;
  char *search_slot;
  enum machine_mode mode : 8;
  unsigned char size_unit;
  unsigned char align;
  unsigned char sep_unit;
  union tree_node *sep;
  union tree_node *size;
  union tree_node *base_init_list;
  union tree_node *abstract_virtuals;
  union tree_node *as_list;
  union tree_node *id_as_list;
  union tree_node *binfo_as_list;
  union tree_node *vtbl_ptr;
  union tree_node *instance_variable;
  union tree_node *friend_classes;
  char *mi_matrix;
  union tree_node *conversions[last_conversion_type];
  union tree_node *dossier;
};
struct lang_decl_flags
{
  enum languages language : 8;
  unsigned operator_attr : 1;
  unsigned constructor_attr : 1;
  unsigned returns_first_arg : 1;
  unsigned preserves_first_arg : 1;
  unsigned friend_attr : 1;
  unsigned static_function : 1;
  unsigned const_memfunc : 1;
  unsigned volatile_memfunc : 1;
  unsigned abstract_virtual : 1;
  unsigned permanent_attr : 1 ;
  unsigned constructor_for_vbase_attr : 1;
  unsigned dummy : 13;
  tree visibility;
  tree context;
};
struct lang_decl
{
  struct lang_decl_flags decl_flags;
  struct template_info *template_info;
  tree main_decl_variant;
  struct pending_inline *pending_inline_info;
  tree vbase_init_list;
  tree chain;
};
enum tag_types { record_type, class_type, union_type, enum_type, exception_type };
extern int strict_prototype;
extern int strict_prototypes_lang_c, strict_prototypes_lang_cplusplus;
extern int flag_labels_ok;
extern int flag_detailed_statistics;
extern int warn_overloaded_virtual;
extern tree void_list_node;
extern tree void_zero_node;
extern tree default_function_type;
extern tree define_function ();
extern tree build_member_type ();
extern tree build_push_scope ();
extern void finish_builtin_type ();
extern tree vtable_entry_type;
extern tree __t_desc_type_node, __i_desc_type_node, __m_desc_type_node;
extern tree class_star_type_node;
extern tree build_vtable_entry ();
extern tree build_vfn_ref ();
extern tree finish_table ();
extern tree typedecl_for_tag ();
extern tree identifier_class_value ();
extern tree constructor_name ();
extern int complete_array_type ();
extern tree coerce_new_type (), coerce_delete_type ();
extern tree error_mark_list;
extern tree ptr_type_node;
extern tree class_type_node, record_type_node, union_type_node, enum_type_node;
extern tree exception_type_node, unknown_type_node;
extern tree get_temp_name (), get_temp_aggr (), get_temp_regvar ();
extern tree cleanup_after_call ();
extern tree build_type_conversion ();
extern tree convert_force ();
extern tree maybe_convert_decl_to_const ();
extern char *lang_printable_name ();
extern char *fndecl_as_string ();
extern char *build_overload_name ();
extern tree vtbl_mask;
extern tree vtbl_type_node;
extern tree get_parm_types ();
extern tree grokopexpr (), getaggrs (), groktypefield ();
extern tree grok_method_quals (), grok_enum_decls ();
extern void finish_anon_union();
extern tree long_long_integer_type_node, long_long_unsigned_type_node;
extern tree integer_two_node, integer_three_node;
extern tree get_first_matching_virtual (), get_abstract_virtuals ();
extern tree build_x_conditional_expr ();
extern tree merge_component_comparisons ();
extern tree build_x_unary_op (), build_x_binary_op ();
extern tree build_component_addr ();
extern tree build_x_function_call ();
extern tree require_complete_type ();
extern tree build_x_indirect_ref (), build_x_array_ref ();
extern tree build_x_modify_expr (), build_x_modify_op_expr ();
extern tree build_m_component_ref ();
extern tree build_component_type_expr ();
extern tree build_x_arrow ();
extern tree build_component_ref_1 ();
extern tree datatype (), unary_complex_lvalue (), target_type ();
extern tree build_return_stmt ();
extern tree convert_arguments (), commonparms ();
extern tree cplus_size_in_bytes ();
extern tree cplus_sizeof (), cplus_sizeof_nowarn ();
extern tree error_not_base_type ();
extern tree binfo_or_else ();
extern void my_friendly_abort ();
extern void error_with_aggr_type ();
extern tree build_let ();
extern tree decl_type_context ();
extern tree build1 ();
extern tree build_cplus_new ();
extern tree build_cplus_array_type ();
extern tree build_cplus_method_type ();
extern tree build_classtype_variant ();
extern tree hash_tree_cons (), hash_tree_chain (), hash_chainon ();
extern tree list_hash_lookup_or_cons ();
extern tree layout_basetypes ();
extern tree copy_to_permanent ();
extern tree get_decl_list ();
extern tree break_out_cleanups ();
extern tree break_out_calls ();
extern tree array_type_nelts_total ();
extern tree array_type_nelts_top ();
extern tree current_exception_type;
extern tree current_exception_decl;
extern tree current_exception_object;
extern tree build_exception_variant ();
extern tree lookup_exception_type (), lookup_exception_cname ();
extern tree lookup_exception_object ();
extern tree cplus_expand_start_catch ();
extern tree cplus_expand_end_try ();
extern void finish_exception_decl ();
extern tree current_class_name;
extern tree current_class_type;
extern tree current_lang_name, lang_name_cplusplus, lang_name_c;
extern tree convert_pointer_to (), convert_pointer_to_vbase ();
extern tree convert_to_reference (), convert_to_aggr (), convert_aggr ();
extern tree build_x_new (), build_x_delete ();
extern tree build_new (), build_vec_new (), build_delete (), build_vec_delete ();
extern tree make_destructor_name ();
extern tree build_scoped_ref (), build_vfield_ref ();
extern tree build_method_call (), build_overload_call ();
extern tree build_type_pathname ();
extern tree start_method ();
extern tree finish_method ();
extern tree lookup_field (), lookup_nested_field (), lookup_fnfields ();
void pushclass (), popclass (), pushclasstype ();
extern tree build_operator_fnname (), build_opfncall (), build_type_conversion ();
extern tree original_function_name;
extern tree build_decl_overload (), build_typename_overload ();
extern tree build_destructor_call ();
extern tree resolve_scope_to_name ();
extern tree build_scoped_method_call ();
extern tree current_class_name, current_class_type, current_class_decl, C_C_D;
extern tree current_vtable_decl;
extern tree resolve_offset_ref ();
extern void check_base_init ();
extern void do_member_init ();
extern tree global_base_init_list;
extern tree current_base_init_list, current_member_init_list;
extern tree get_member_function ();
extern tree build_member_call (), build_offset_ref ();
extern tree build_virtual_init ();
extern int current_function_assigns_this;
extern int current_function_just_assigned_this;
extern int current_function_parms_stored;
enum visibility_type {
  visibility_default,
  visibility_public,
  visibility_private,
  visibility_protected,
  visibility_default_virtual,
  visibility_public_virtual,
  visibility_private_virtual
};
enum visibility_type compute_visibility ();
extern tree current_unit_name, current_unit_language;
extern char *operator_name_string ();
struct pending_inline
{
  struct pending_inline *next;	
  int lineno;			
  char *filename;		
  tree fndecl;			
  int token;			
  int token_value;		
  char *buf;			
  int len;			
  tree parm_vec, bindings;	
  unsigned int can_free : 1;	
  unsigned int deja_vu : 1;	
  unsigned int interface : 2;	
};
extern tree combine_strings ();
extern int yylex ();
extern struct pending_inline *pending_inlines;
extern char *print_fndecl_with_types ();
extern tree hack_identifier ();
extern tree hack_operator ();
extern int flag_all_virtual;
extern int flag_this_is_variable;
extern int flag_int_enum_equivalence;
extern int flag_gc;
extern int flag_dossier;
extern int current_function_obstack_index;
extern int current_function_obstack_usage;
enum overload_flags { NO_SPECIAL = 0, DTOR_FLAG, OP_FLAG, TYPENAME_FLAG };
extern tree default_conversion (), pushdecl (), pushdecl_top_level ();
extern tree push_overloaded_decl ();
extern void push_overloaded_decl_top_level ();
extern tree make_instance_name (), do_decl_overload ();
extern tree maybe_build_cleanup ();
extern tree build_instantiated_decl (), instantiate_type ();
extern tree require_instantiated_type ();
extern tree build_vtbl_ref ();
extern tree make_anon_parm_name ();
extern int resolves_to_fixed_type_p ();
extern tree do_friend ();
extern void grokclassfn ();
extern tree current_class_decl, C_C_D;	
extern tree current_class_name;	
extern tree current_class_type;	
extern tree get_temp_name (), get_temp_aggr (), get_temp_regvar ();
extern tree get_decl_list ();
extern tree build_method_call ();
extern tree build_type_conversion ();
extern tree build_functional_cast ();
extern tree decl_constant_value ();
extern tree resolve_offset_ref ();
extern tree build_vbase_delete ();
extern char *operator_name_string ();
extern void compiler_error_with_decl ();
extern tree build_opid ();
extern tree do_identifier ();
extern tree arbitrate_lookup ();
extern char **opname_tab, **assignop_tab;
extern tree build_lang_decl (), build_lang_field_decl ();
extern tree make_lang_type ();
extern tree cons_up_default_function ();
extern tree convert_from_reference ();
extern tree init_vbase_pointers ();
extern tree build_vbase_pointer (), build_vbase_path ();
extern tree lookup_fnfield (), next_baselink ();
extern tree build_vbase_vtables_init ();
extern tree get_binfo ();
extern tree get_vbase_types ();
extern tree get_baselinks ();
extern tree make_binfo (), copy_binfo ();
extern tree binfo_value (), virtual_member ();
extern tree virtual_offset ();
extern tree reverse_path ();
tree protect_value_from_gc ();
tree build_headof ();
tree build_classof ();
tree build_t_desc ();
tree build_i_desc ();
tree build_m_desc ();
struct template_info {
  tree parm_vec;
  tree bindings;
  char *text;
  int length;
  char *filename;
  int lineno;
  tree aggr;
};
extern tree end_template_parm_list ();
extern tree process_template_parm ();
extern tree lookup_template_class ();
extern tree instantiate_template ();
extern tree instantiate_class_template ();
extern int processing_template_decl, processing_template_defn;
extern void GNU_xref_start_scope ();
extern void GNU_xref_end_scope ();
enum rid
{
  RID_UNUSED,
  RID_INT,
  RID_CHAR,
  RID_FLOAT,
  RID_DOUBLE,
  RID_VOID,
  RID_UNUSED1,
  RID_CLASS,
  RID_RECORD,
  RID_UNION,
  RID_ENUM,
  RID_LONGLONG,
  RID_UNSIGNED,
  RID_SHORT,
  RID_LONG,
  RID_AUTO,
  RID_STATIC,
  RID_EXTERN,
  RID_REGISTER,
  RID_TYPEDEF,
  RID_SIGNED,
  RID_CONST,
  RID_VOLATILE,
  RID_INLINE,
  RID_WCHAR,
  RID_FRIEND,
  RID_VIRTUAL,
  RID_EXCEPTION,
  RID_RAISES,
  RID_PUBLIC,
  RID_PRIVATE,
  RID_PROTECTED,
  RID_MAX
};
extern tree ridpointers[(int) RID_MAX];
extern tree lastiddecl;
extern char *token_buffer;	
extern int looking_for_typename;
extern tree make_pointer_declarator (), make_reference_declarator ();
extern void reinit_parse_for_function ();
extern void reinit_parse_for_method ();
extern int yylex ();
extern void (*signal())();
extern int raise();
struct sigaction {
	void	(*sa_handler) (); 
	sigset_t sa_mask;		
	int	sa_flags;		
};
typedef union sigval {
	int 	sival_int;
	void	*sival_ptr;
} sigval_t;
typedef struct sigevent {
	union sigval	sigev_value;	
	int		sigev_signo;	
	int		sigev_notify;	
} sigevent_t;
extern int kill (); 
extern int sigaction (); 
extern int sigprocmask ();
extern int sigsuspend ();  
extern int sigemptyset ();
extern int sigfillset ();
extern int sigaddset ();
extern int sigdelset ();
extern int sigismember ();
extern int sigpending ();
typedef int sig_atomic_t;
struct  sigcontext {
	long    sc_onstack;		
	long    sc_mask;		
	long	sc_pc;			
	long	sc_ps;			
	long	sc_regs[32];		
	long	sc_ownedfp;		
	long	sc_fpregs[32];		
	unsigned long sc_fpcr;		
	unsigned long sc_fp_control;	
	long sc_reserved1;		
	long sc_reserved2;		
	size_t	sc_ssize;		
	caddr_t	sc_sbase;		
	unsigned long sc_traparg_a0;	
	unsigned long sc_traparg_a1;	
	unsigned long sc_traparg_a2;	
	unsigned long sc_fp_trap_pc;	
	unsigned long sc_fp_trigger_sum; 
	unsigned long sc_fp_trigger_inst; 
};
extern sigset_t cantmasksigset;
struct	sigvec {
	void	(*sv_handler)();	
	int     sv_mask;        
	int     sv_flags;    
};                           
extern int sigvec();
extern int killpg();
struct  sigstack {
        char    *ss_sp;                 
        int     ss_onstack;             
};
typedef struct  sigaltstack {
        caddr_t	ss_sp;			
        int     ss_flags;		
        size_t	ss_size;		
} stack_t;
extern int sigblock();
extern int sigpause();
extern int sigreturn();
extern int sigsetmask();
extern int sigstack(); 
extern int siginterrupt();
extern int sigaltstack();
extern void (*sigset())();
extern int sighold();
extern int sigrelse();
extern int sigignore();
extern int sigsendset();
extern int sigsend();
  void (*ssignal ()) ();
  int gsignal ();
struct _obstack_chunk		
{
  char  *limit;			
  struct _obstack_chunk *prev;	
  char	contents[4];		
};
struct obstack		
{
  long	chunk_size;		
  struct _obstack_chunk* chunk;	
  char	*object_base;		
  char	*next_free;		
  char	*chunk_limit;		
  long temp;		
  int   alignment_mask;		
  struct _obstack_chunk *(*chunkfun) (); 
  void (*freefun) ();		
  char *extra_arg;		
  unsigned use_extra_arg:1;	
  unsigned maybe_empty_object:1;
};
extern void _obstack_newchunk ();
extern void _obstack_free ();
extern void _obstack_begin ();
extern void _obstack_begin_1 ();
extern struct obstack permanent_obstack;
struct stack_level
{
  struct stack_level *prev;
  struct obstack *obstack;
  tree *first;
  int limit;
};
struct stack_level *push_stack_level ();
struct stack_level *pop_stack_level ();
static struct obstack decl_obstack;
static struct stack_level *decl_stack;
enum decl_context
{ NORMAL,			
  FUNCDEF,			
  PARM,				
  FIELD,			
  BITFIELD,			
  TYPENAME,			
  MEMFUNCDEF			
};
extern tree this_identifier, in_charge_identifier;
extern tree last_function_parms;
extern tree pending_statics;
extern tree static_aggregates;
extern tree pending_addressable_inlines;
static tree grokparms ();
tree grokdeclarator ();
tree pushdecl (), pushdecl_class_level ();
void pop_implicit_try_blocks ();
extern void init_search_processing ();
extern void init_class_processing (); 
extern void init_init_processing ();
extern void init_exception_processing ();
extern void init_gc_processing ();
extern void init_cadillac ();
 void grokclassfn ();
 tree grokoptypename ();
static tree lookup_tag ();
static tree lookup_tag_reverse ();
static tree lookup_name_current_level ();
static void maybe_globalize_type (), globalize_nested_type ();
static tree lookup_nested_type ();
static char *redeclaration_error_message ();
int parmlist_is_exprlist ();
static int parmlist_is_random ();
void grok_ctor_properties ();
static void grok_op_properties ();
static void expand_static_init ();
static void deactivate_exception_cleanups ();
static void revert_static_member_fn ();
void adjust_type_value ();
tree finish_table ();
tree error_mark_node;
tree error_mark_list;
tree short_integer_type_node;
tree integer_type_node;
tree long_integer_type_node;
tree long_long_integer_type_node;
tree short_unsigned_type_node;
tree unsigned_type_node;
tree long_unsigned_type_node;
tree long_long_unsigned_type_node;
tree ptrdiff_type_node;
tree unsigned_char_type_node;
tree signed_char_type_node;
tree char_type_node;
tree wchar_type_node;
tree signed_wchar_type_node;
tree unsigned_wchar_type_node;
tree float_type_node;
tree double_type_node;
tree long_double_type_node;
tree intQI_type_node;
tree intHI_type_node;
tree intSI_type_node;
tree intDI_type_node;
tree unsigned_intQI_type_node;
tree unsigned_intHI_type_node;
tree unsigned_intSI_type_node;
tree unsigned_intDI_type_node;
tree void_type_node, void_list_node;
tree void_zero_node;
tree ptr_type_node, const_ptr_type_node;
tree string_type_node, const_string_type_node;
tree char_array_type_node;
tree int_array_type_node;
tree wchar_array_type_node;
tree default_function_type;
tree double_ftype_double, double_ftype_double_double;
tree int_ftype_int, long_ftype_long;
tree void_ftype_ptr_ptr_int, int_ftype_ptr_ptr_int, void_ftype_ptr_int_int;
tree string_ftype_ptr_ptr, int_ftype_string_string;
tree sizet_ftype_string;
tree int_ftype_cptr_cptr_sizet;
tree vtable_entry_type;
tree __t_desc_type_node, __i_desc_type_node, __m_desc_type_node;
tree __t_desc_array_type, __i_desc_array_type, __m_desc_array_type;
tree class_star_type_node;
tree class_type_node, record_type_node, union_type_node, enum_type_node;
tree exception_type_node, unknown_type_node;
tree maybe_gc_cleanup;
tree vtbl_mask;
tree vtbl_type_node;
tree empty_init_node;
tree dtor_label;
tree ctor_label;
tree unhandled_exception_fndecl;
tree abort_fndecl;
extern rtx cleanup_label, return_label;
rtx original_result_rtx;
rtx base_init_insns;
tree this_identifier, in_charge_identifier;
static tree named_label_uses;
tree static_aggregates;
tree pending_addressable_inlines;
static tree overloads_to_forget;
tree integer_zero_node;
tree null_pointer_node;
tree integer_one_node, integer_two_node, integer_three_node;
tree pending_invalid_xref;
char *pending_invalid_xref_file;
int pending_invalid_xref_line;
static tree enum_next_value;
tree last_function_parms;
static tree last_function_parm_tags;
static tree current_function_parms;
static tree current_function_parm_tags;
static tree named_labels;
static tree shadowed_labels;
tree current_function_decl;
int current_function_returns_value;
int current_function_returns_null;
tree current_function_return_value;
extern int warn_redundant_decls;
static int warn_about_return_type;
static int current_extern_inline;
extern int flag_short_double;
extern int flag_ansi;
extern tree *current_lang_base, *current_lang_stack;
char *language_string = "GNU C++";
int current_function_assigns_this;
int current_function_just_assigned_this;
int current_function_parms_stored;
int current_function_obstack_index;
int current_function_obstack_usage;
extern int spew_debug;
struct stack_level *
push_decl_level (stack, obstack)
     struct stack_level *stack;
     struct obstack *obstack;
{
  struct stack_level tem;
  tem.prev = stack;
  return push_stack_level (obstack, &tem, sizeof (tem));
}
static struct stack_level *
pop_decl_level (stack)
     struct stack_level *stack;
{
  tree *bp, *tp;
  struct obstack *obstack = stack->obstack;
  bp = stack->first;
  tp = (tree *)((obstack)->next_free);
  while (tp != bp)
    {
      --tp;
      if (*tp != (tree) 0)
	  (((struct lang_identifier *)(((*tp)->decl.name)))->class_value) = (tree) 0;
    }
  return pop_stack_level (stack);
}
struct binding_level
  {
    tree names;
    tree tags;
    tree shadowed;
    tree class_shadowed;
    tree type_shadowed;
    tree blocks;
    tree this_block;
    struct binding_level *level_chain;
    unsigned short n_incomplete;
    unsigned parm_flag : 4;
    unsigned keep : 3;
    unsigned tag_transparent : 1;
    unsigned more_cleanups_ok : 1;
    unsigned have_cleanups : 1;
    unsigned more_exceptions_ok : 1;
    unsigned have_exceptions : 1;
    unsigned accept_any : 1;
    unsigned pseudo_global : 1;
  };
static struct binding_level *current_binding_level;
static struct binding_level *class_binding_level;
static struct binding_level *free_binding_level;
static struct binding_level *global_binding_level;
static struct binding_level clear_binding_level;
static int keep_next_level_flag;
static void
push_binding_level (newlevel, tag_transparent, keep)
     struct binding_level *newlevel;
     int tag_transparent, keep;
{
  ((void)(1));
  *newlevel = clear_binding_level;
  if (class_binding_level)
    {
      newlevel->level_chain = class_binding_level;
      class_binding_level = 0;
    }
  else
    {
      newlevel->level_chain = current_binding_level;
    }
  current_binding_level = newlevel;
  newlevel->tag_transparent = tag_transparent;
  newlevel->more_cleanups_ok = 1;
  newlevel->more_exceptions_ok = 1;
  newlevel->keep = keep;
  ((void)(1));
}
static void
pop_binding_level ()
{
  ((void)(1));
  if (global_binding_level)
    {
      if (current_binding_level == global_binding_level)
	my_friendly_abort (123);
    }
  {
    register struct binding_level *level = current_binding_level;
    current_binding_level = current_binding_level->level_chain;
    level->level_chain = free_binding_level;
    free_binding_level = level;
    if (current_binding_level->parm_flag == 2)
      {
	class_binding_level = current_binding_level;
	do
	  {
	    current_binding_level = current_binding_level->level_chain;
	  }
	while (current_binding_level->parm_flag == 2);
      }
  }
  ((void)(1));
}
int
global_bindings_p ()
{
  return current_binding_level == global_binding_level;
}
void
keep_next_level ()
{
  keep_next_level_flag = 1;
}
int
kept_level_p ()
{
  return (current_binding_level->blocks != 0
	  || current_binding_level->keep
	  || current_binding_level->names != 0
	  || (current_binding_level->tags != 0
	      && !current_binding_level->tag_transparent));
}
void
declare_parm_level ()
{
  current_binding_level->parm_flag = 1;
}
void
declare_implicit_exception ()
{
  current_binding_level->parm_flag = 3;
}
int
have_exceptions_p ()
{
  return current_binding_level->have_exceptions;
}
void
declare_uninstantiated_type_level ()
{
  current_binding_level->accept_any = 1;
}
int
uninstantiated_type_level_p ()
{
  return current_binding_level->accept_any;
}
void
declare_pseudo_global_level ()
{
  current_binding_level->pseudo_global = 1;
}
int
pseudo_global_level_p ()
{
  return current_binding_level->pseudo_global;
}
void
pushlevel (tag_transparent)
     int tag_transparent;
{
  register struct binding_level *newlevel = (struct binding_level *) 0;
  if (current_binding_level == global_binding_level)
    my_friendly_assert (named_labels == (tree) 0, 134);
  if (free_binding_level)
    {
      newlevel = free_binding_level;
      free_binding_level = free_binding_level->level_chain;
    }
  else
    {
      newlevel = (struct binding_level *) xmalloc (sizeof (struct binding_level));
    }
  push_binding_level (newlevel, tag_transparent, keep_next_level_flag);
  GNU_xref_start_scope (newlevel);
  keep_next_level_flag = 0;
}
void
pushlevel_temporary (tag_transparent)
     int tag_transparent;
{
  pushlevel (tag_transparent);
  current_binding_level->keep = 2;
  clear_last_expr ();
  expand_start_bindings (0);
}
tree
poplevel (keep, reverse, functionbody)
     int keep;
     int reverse;
     int functionbody;
{
  register tree link;
  tree decls;
  int tmp = functionbody;
  int implicit_try_block = current_binding_level->parm_flag == 3;
  int real_functionbody = current_binding_level->keep == 2
    ? ((functionbody = 0), tmp) : functionbody;
  tree tags = functionbody >= 0 ? current_binding_level->tags : 0;
  tree subblocks = functionbody >= 0 ? current_binding_level->blocks : 0;
  tree block = 0;
  tree decl;
  int block_previously_created;
  ((void)(1));
  GNU_xref_end_scope (current_binding_level,
		      current_binding_level->level_chain,
		      current_binding_level->parm_flag,
		      current_binding_level->keep,
		      current_binding_level->tag_transparent);
  if (current_binding_level->keep == 1)
    keep = 1;
  if (reverse)
    current_binding_level->names
      = decls = nreverse (current_binding_level->names);
  else
    decls = current_binding_level->names;
  for (decl = decls; decl; decl = ((decl)->common.chain))
    if (((enum tree_code) (decl)->common.code) == FUNCTION_DECL
	&& ! ((decl)->common.asm_written_flag)
	&& ((decl)->decl.initial) != 0
	&& ((decl)->common.addressable_flag))
      output_inline_function (decl);
  block = 0;
  block_previously_created = (current_binding_level->this_block != 0);
  if (block_previously_created)
    block = current_binding_level->this_block;
  else if (keep == 1 || functionbody)
    block = make_node (BLOCK);
  if (block != 0)
    {
      ((block)->block.vars) = decls;
      ((block)->block.type_tags) = tags;
      ((block)->block.subblocks) = subblocks;
      remember_end_note (block);
    }
  if (keep >= 0)
    for (link = subblocks; link; link = ((link)->common.chain))
      ((link)->block.supercontext) = block;
  for (link = decls; link; link = ((link)->common.chain))
    {
      if (((link)->decl.name) != 0)
	{
	  if (((link)->decl.external_flag))
	    {
	      if (((link)->common.used_flag))
		((((link)->decl.assembler_name))->common.used_flag) = 1;
	      if (((link)->common.addressable_flag))
		((((link)->decl.assembler_name))->common.addressable_flag) = 1;
	    }
	    (((struct lang_identifier *)(((link)->decl.name)))->local_value) = 0;
	}
    }
  for (link = current_binding_level->shadowed; link; link = ((link)->common.chain))
      (((struct lang_identifier *)(((link)->list.purpose)))->local_value) = ((link)->list.value);
  for (link = current_binding_level->class_shadowed;
       link; link = ((link)->common.chain))
      (((struct lang_identifier *)(((link)->list.purpose)))->class_value) = ((link)->list.value);
  for (link = current_binding_level->type_shadowed;
       link; link = ((link)->common.chain))
    (((((link)->list.purpose))->common.type)) = ((link)->list.value);
  if (functionbody)
    {
      ((block)->block.vars) = 0;
      for (link = named_labels; link; link = ((link)->common.chain))
	{
	  register tree label = ((link)->list.value);
	  if (((label)->decl.initial) == 0)
	    {
	      error_with_decl (label, "label `%s' used but not defined");
	      define_label (input_filename, 1, ((label)->decl.name));
	    }
	  else if (warn_unused && !((label)->common.used_flag))
	    warning_with_decl (label, 
			       "label `%s' defined but not used");
	    (((struct lang_identifier *)(((label)->decl.name)))->x == 0 ? ((struct lang_identifier *)(((label)->decl.name)))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(((label)->decl.name)))->x->label_value = ( 0));
          ((label)->common.chain) = ((block)->block.vars);
          ((block)->block.vars) = label;
	}
      named_labels = 0;
    }
  {
    struct binding_level *level_chain;
    level_chain = current_binding_level->level_chain;
    if (level_chain)
      {
	tree labels;
	for (labels = named_label_uses; labels; labels = ((labels)->common.chain))
	  if (((labels)->common.type) == (tree)current_binding_level)
	    {
	      ((labels)->common.type) = (tree)level_chain;
	      ((labels)->list.purpose) = level_chain->names;
	    }
      }
  }
  tmp = current_binding_level->keep;
  pop_binding_level ();
  if (functionbody)
    ((current_function_decl)->decl.initial) = block;
  else if (block)
    {
      if (!block_previously_created)
        current_binding_level->blocks
          = chainon (current_binding_level->blocks, block);
    }
  else if (subblocks)
    if (keep == 2)
      current_binding_level->blocks = chainon (subblocks, current_binding_level->blocks);
    else
      current_binding_level->blocks
        = chainon (current_binding_level->blocks, subblocks);
  if (tmp == 2 && !implicit_try_block)
    {
      expand_end_bindings (getdecls (), keep, 1);
      block = poplevel (keep, reverse, real_functionbody);
    }
  if (block)
    ((block)->common.used_flag) = 1;
  ((void)(1));
  return block;
}
void
delete_block (block)
     tree block;
{
  tree t;
  if (current_binding_level->blocks == block)
    current_binding_level->blocks = ((block)->common.chain);
  for (t = current_binding_level->blocks; t;)
    {
      if (((t)->common.chain) == block)
	((t)->common.chain) = ((block)->common.chain);
      else
	t = ((t)->common.chain);
    }
  ((block)->common.chain) = 0;
  ((block)->common.used_flag) = 0;
}
void
insert_block (block)
     tree block;
{
  ((block)->common.used_flag) = 1;
  current_binding_level->blocks
    = chainon (current_binding_level->blocks, block);
}
void
add_block_current_level (block)
     tree block;
{
  current_binding_level->blocks
    = chainon (current_binding_level->blocks, block);
}
void
set_block (block)
    register tree block;
{
  current_binding_level->this_block = block;
}
void
pushlevel_class ()
{
  ((void)(1));
  pushlevel (0);
  decl_stack = push_decl_level (decl_stack, &decl_obstack);
  class_binding_level = current_binding_level;
  class_binding_level->parm_flag = 2;
  do
    {
      current_binding_level = current_binding_level->level_chain;
    }
  while (current_binding_level->parm_flag == 2);
  ((void)(1));
}
tree
poplevel_class ()
{
  register struct binding_level *level = class_binding_level;
  tree block = 0;
  tree shadowed;
  ((void)(1));
  if (level == 0)
    {
      while (current_binding_level && class_binding_level == 0)
	block = poplevel (0, 0, 0);
      if (current_binding_level == 0)
	fatal ("syntax error too serious");
      level = class_binding_level;
    }
  decl_stack = pop_decl_level (decl_stack);
  for (shadowed = level->shadowed; shadowed; shadowed = ((shadowed)->common.chain))
      (((struct lang_identifier *)(((shadowed)->list.purpose)))->local_value) = ((shadowed)->list.value);
  for (shadowed = level->class_shadowed; shadowed; shadowed = ((shadowed)->common.chain))
      (((struct lang_identifier *)(((shadowed)->list.purpose)))->class_value) = ((shadowed)->list.value);
  for (shadowed = level->type_shadowed; shadowed; shadowed = ((shadowed)->common.chain))
    (((((shadowed)->list.purpose))->common.type)) = ((shadowed)->list.value);
  GNU_xref_end_scope (class_binding_level,
		      class_binding_level->level_chain,
		      class_binding_level->parm_flag,
		      class_binding_level->keep,
		      class_binding_level->tag_transparent);
  class_binding_level = level->level_chain;
  if (class_binding_level->parm_flag != 2)
    class_binding_level = 0;
  level->level_chain = free_binding_level;
  free_binding_level = level;
  ((void)(1));
  return block;
}
int no_print_functions = 0;
int no_print_builtins = 0;
void
print_binding_level (lvl)
     struct binding_level *lvl;
{
  tree t;
  int i = 0, len;
  fprintf ((&_iob[2]), " blocks=");
  fprintf ((&_iob[2]), sizeof (int) == sizeof (char *) ? "%x" : "%lx", lvl->blocks);
  fprintf ((&_iob[2]), " n_incomplete=%d parm_flag=%d keep=%d",
	   lvl->n_incomplete, lvl->parm_flag, lvl->keep);
  if (lvl->tag_transparent)
    fprintf ((&_iob[2]), " tag-transparent");
  if (lvl->more_cleanups_ok)
    fprintf ((&_iob[2]), " more-cleanups-ok");
  if (lvl->have_cleanups)
    fprintf ((&_iob[2]), " have-cleanups");
  if (lvl->more_exceptions_ok)
    fprintf ((&_iob[2]), " more-exceptions-ok");
  if (lvl->have_exceptions)
    fprintf ((&_iob[2]), " have-exceptions");
  fprintf ((&_iob[2]), "\n");
  if (lvl->names)
    {
      fprintf ((&_iob[2]), " names:\t");
      for (t = lvl->names; t; t = ((t)->common.chain))
	{
	  if (no_print_functions && (((enum tree_code) (t)->common.code) == FUNCTION_DECL)) 
	    continue;
	  if (no_print_builtins
	      && (((enum tree_code) (t)->common.code) == TYPE_DECL)
	      && (!strcmp(((t)->decl.filename),"<built-in>")))
	    continue;
	  if (((enum tree_code) (t)->common.code) == FUNCTION_DECL)
	    len = 3;
	  else
	    len = 2;
	  i += len;
	  if (i > 6)
	    {
	      fprintf ((&_iob[2]), "\n\t");
	      i = len;
	    }
	  print_node_brief ((&_iob[2]), "", t, 0);
	  if (((enum tree_code) (t)->common.code) == ERROR_MARK)
	    break;
	}
      if (i)
        fprintf ((&_iob[2]), "\n");
    }
  if (lvl->tags)
    {
      fprintf ((&_iob[2]), " tags:\t");
      i = 0;
      for (t = lvl->tags; t; t = ((t)->common.chain))
	{
	  if (((t)->list.purpose) == (tree) 0)
	    len = 3;
	  else if (((t)->list.purpose) == (((((((t)->list.value))->type.name))->decl.name)))
	    len = 2;
	  else
	    len = 4;
	  i += len;
	  if (i > 5)
	    {
	      fprintf ((&_iob[2]), "\n\t");
	      i = len;
	    }
	  if (((t)->list.purpose) == (tree) 0)
	    {
	      print_node_brief ((&_iob[2]), "<unnamed-typedef", ((t)->list.value), 0);
	      fprintf ((&_iob[2]), ">");
	    }
	  else if (((t)->list.purpose) == (((((((t)->list.value))->type.name))->decl.name)))
	    print_node_brief ((&_iob[2]), "", ((t)->list.value), 0);
	  else
	    {
	      print_node_brief ((&_iob[2]), "<typedef", ((t)->list.purpose), 0);
	      print_node_brief ((&_iob[2]), "", ((t)->list.value), 0);
	      fprintf ((&_iob[2]), ">");
	    }
	}
      if (i)
	fprintf ((&_iob[2]), "\n");
    }
  if (lvl->shadowed)
    {
      fprintf ((&_iob[2]), " shadowed:");
      for (t = lvl->shadowed; t; t = ((t)->common.chain))
	{
	  fprintf ((&_iob[2]), " %s ", ((((t)->list.purpose))->identifier.pointer));
	}
      fprintf ((&_iob[2]), "\n");
    }
  if (lvl->class_shadowed)
    {
      fprintf ((&_iob[2]), " class-shadowed:");
      for (t = lvl->class_shadowed; t; t = ((t)->common.chain))
	{
	  fprintf ((&_iob[2]), " %s ", ((((t)->list.purpose))->identifier.pointer));
	}
      fprintf ((&_iob[2]), "\n");
    }
  if (lvl->type_shadowed)
    {
      fprintf ((&_iob[2]), " type-shadowed:");
      for (t = lvl->type_shadowed; t; t = ((t)->common.chain))
        {
	  fprintf ((&_iob[2]), " %s ", ((((t)->list.purpose))->identifier.pointer));
        }
      fprintf ((&_iob[2]), "\n");
    }
}
void
print_other_binding_stack (stack)
     struct binding_level *stack;
{
  struct binding_level *level;
  for (level = stack; level != global_binding_level; level = level->level_chain)
    {
      fprintf ((&_iob[2]), "binding level ");
      fprintf ((&_iob[2]), sizeof (int) == sizeof (char *) ? "%x" : "%lx", level);
      fprintf ((&_iob[2]), "\n");
      print_binding_level (level);
    }
}
void
print_binding_stack ()
{
  struct binding_level *b;
  fprintf ((&_iob[2]), "current_binding_level=");
  fprintf ((&_iob[2]), sizeof (int) == sizeof (char *) ? "%x" : "%lx", current_binding_level);
  fprintf ((&_iob[2]), "\nclass_binding_level=");
  fprintf ((&_iob[2]), sizeof (int) == sizeof (char *) ? "%x" : "%lx", class_binding_level);
  fprintf ((&_iob[2]), "\nglobal_binding_level=");
  fprintf ((&_iob[2]), sizeof (int) == sizeof (char *) ? "%x" : "%lx", global_binding_level);
  fprintf ((&_iob[2]), "\n");
  if (class_binding_level)
    {
      for (b = class_binding_level; b; b = b->level_chain)
	if (b == current_binding_level)
	  break;
      if (b)
	b = class_binding_level;
      else
	b = current_binding_level;
    }
  else
    b = current_binding_level;
  print_other_binding_stack (b);
  fprintf ((&_iob[2]), "global:\n");
  print_binding_level (global_binding_level);
}
struct saved_scope {
  struct binding_level *old_binding_level;
  tree old_bindings;
  struct saved_scope *prev;
  tree class_name, class_type, class_decl, function_decl;
  struct binding_level *class_bindings;
};
static struct saved_scope *current_saved_scope;
extern tree prev_class_type;
void
push_to_top_level ()
{
  struct saved_scope *s =
    (struct saved_scope *) xmalloc (sizeof (struct saved_scope));
  struct binding_level *b = current_binding_level;
  tree old_bindings = (tree) 0;
  for (; b; b = b->level_chain)
    {
      tree t;
      for (t = b->names; t; t = ((t)->common.chain))
	if (b != global_binding_level)
	  {
	    tree binding, t1, t2 = t;
	    tree id = ((t2)->decl.assembler_name);
	    if (!id
		|| (!  (((struct lang_identifier *)(id))->local_value)
		    && !  (((struct lang_identifier *)(id))->class_value)))
	      continue;
	    for (t1 = old_bindings; t1; t1 = ((t1)->common.chain))
	      if (((t1)->vec.a[ 0]) == id)
		goto skip_it;
	    binding = make_tree_vec (4);
	    if (id)
	      {
		my_friendly_assert (((enum tree_code) (id)->common.code) == IDENTIFIER_NODE, 135);
		((binding)->vec.a[ 0]) = id;
		((binding)->vec.a[ 1]) = (((id)->common.type));
		((binding)->vec.a[ 2]) =   (((struct lang_identifier *)(id))->local_value);
		((binding)->vec.a[ 3]) =   (((struct lang_identifier *)(id))->class_value);
		  (((struct lang_identifier *)(id))->local_value) = 0;
		  (((struct lang_identifier *)(id))->class_value) = 0;
		adjust_type_value (id);
	      }
	    ((binding)->common.chain) = old_bindings;
	    old_bindings = binding;
	    skip_it:
	    ;
	  }
      if (b != global_binding_level)
        for (t = b->type_shadowed; t; t = ((t)->common.chain))
          (((((t)->list.purpose))->common.type) =  ((t)->list.value));
    }
  s->old_binding_level = current_binding_level;
  current_binding_level = global_binding_level;
  s->class_name = current_class_name;
  s->class_type = current_class_type;
  s->class_decl = current_class_decl;
  s->function_decl = current_function_decl;
  s->class_bindings = class_binding_level;
  current_class_name = current_class_type = current_class_decl = 0;
  current_function_decl = 0;
  class_binding_level = 0;
  s->prev = current_saved_scope;
  s->old_bindings = old_bindings;
  current_saved_scope = s;
  ((void)(1));
}
void
pop_from_top_level ()
{
  struct saved_scope *s = current_saved_scope;
  tree t;
  ((void)(1));
  current_binding_level = s->old_binding_level;
  current_saved_scope = s->prev;
  for (t = s->old_bindings; t; t = ((t)->common.chain))
    {
      tree id = ((t)->vec.a[ 0]);
      if (id)
	{
	  (((id)->common.type)) = ((t)->vec.a[ 1]);
	    (((struct lang_identifier *)(id))->local_value) = ((t)->vec.a[ 2]);
	    (((struct lang_identifier *)(id))->class_value) = ((t)->vec.a[ 3]);
	}
    }
  current_class_name = s->class_name;
  current_class_type = s->class_type;
  current_class_decl = s->class_decl;
  if (current_class_type)
    C_C_D = (((current_class_type)->type.lang_specific)->instance_variable);
  else
    C_C_D = (tree) 0;
  current_function_decl = s->function_decl;
  class_binding_level = s->class_bindings;
  free (s);
  ((void)(1));
}
void
set_identifier_type_value (id, type)
     tree id;
     tree type;
{
  if (current_binding_level != global_binding_level)
    {
      tree old_type_value = (((id)->common.type));
      current_binding_level->type_shadowed
	= tree_cons (id, old_type_value, current_binding_level->type_shadowed);
    }
  else if (class_binding_level)
    {
      tree old_type_value = (((id)->common.type));
      class_binding_level->type_shadowed
	= tree_cons (id, old_type_value, class_binding_level->type_shadowed);
    }      
  (((id)->common.type) =  type);
}
void
set_identifier_local_value (id, type)
     tree id;
     tree type;
{
  if (current_binding_level != global_binding_level)
    {
      tree old_local_value =   (((struct lang_identifier *)(id))->local_value);
      current_binding_level->shadowed
	= tree_cons (id, old_local_value, current_binding_level->shadowed);
    }
  else if (class_binding_level)
    {
      tree old_local_value =   (((struct lang_identifier *)(id))->local_value);
      class_binding_level->shadowed
	= tree_cons (id, old_local_value, class_binding_level->shadowed);
    }      
    (((struct lang_identifier *)(id))->local_value) = type;
}
static void
set_nested_typename (decl, classname, name, type)
     tree decl, classname, name, type;
{
  my_friendly_assert (((enum tree_code) (decl)->common.code) == TYPE_DECL, 136);
  if (classname != 0)
    {
      char *buf;
      my_friendly_assert (((enum tree_code) (classname)->common.code) == IDENTIFIER_NODE, 137);
      my_friendly_assert (((enum tree_code) (name)->common.code) == IDENTIFIER_NODE, 138);
      buf = (char *) alloca (4 + ((classname)->identifier.length)
			     + ((name)->identifier.length));
      sprintf (buf, "%s::%s", ((classname)->identifier.pointer),
	       ((name)->identifier.pointer));
      ((decl)->decl.arguments) = get_identifier (buf);
      (((((decl)->decl.arguments))->common.type) =  type);
    }
  else
    ((decl)->decl.arguments) = name;
}
void
pushtag (name, type)
     tree name, type;
{
  register struct binding_level *b;
  if (class_binding_level)
    b = class_binding_level;
  else
    {
      b = current_binding_level;
      while (b->tag_transparent) b = b->level_chain;
    }
  if (b == global_binding_level)
    b->tags = perm_tree_cons (name, type, b->tags);
  else
    b->tags = saveable_tree_cons (name, type, b->tags);
  if (name)
    {
      if (((type)->type.name) == 0)
        ((type)->type.name) = name;
      if ((((name)->common.type)) != type
	  && (((enum tree_code) (type)->common.code) != RECORD_TYPE
	      || class_binding_level == 0
	      || !(((type)->type.lang_specific)->type_flags.declared_exception)))
        {
          register tree d;
	  if (current_class_type == 0
	      || ((current_class_type)->type.size) != (tree) 0)
	    {
	      if (current_lang_name == lang_name_cplusplus)
		d = lookup_nested_type (type, current_class_type ? ((current_class_type)->type.name) : (tree) 0);
	      else
		d = (tree) 0;
	      if (d == (tree) 0)
		{
		  d = build_decl (TYPE_DECL, name, type);
		  ((d)->decl.assembler_name) = get_identifier (build_overload_name (type, 1, 1));
		  ((d)->decl.linenum) = 0;
		  set_identifier_type_value (name, type);
		}
	      else
		d = ((d)->type.name);
	      if (! (((name)->identifier.pointer)[0] == '$' 				  && ((name)->identifier.pointer)[1] == '_')
		  && ((type)->type.name)
		  && (((enum tree_code) (((type)->type.name))->common.code) != TYPE_DECL
		      || lookup_name (name, 1) != ((type)->type.name)))
		{
		  if (class_binding_level)
		    d = pushdecl_class_level (d);
		  else
		    d = pushdecl (d);
		}
	    }
	  else
	    {
	      d = build_lang_field_decl (TYPE_DECL, name, type);
	      set_identifier_type_value (name, type);
	      d = pushdecl_class_level (d);
	    }
	  if ((((name)->identifier.pointer)[0] == '$' 				  && ((name)->identifier.pointer)[1] == '_'))
	    ((d)->decl.ignored_flag) = 1;
	  ((type)->type.name) = d;
	  if ((current_class_type == (tree) 0
	       && current_function_decl == (tree) 0)
	      || current_lang_name != lang_name_cplusplus)
	    ((d)->decl.arguments) = name;
	  else if (current_function_decl != (tree) 0)
	    {
	      set_nested_typename (d, ((current_function_decl)->decl.assembler_name),
				   name, type);
	      ((d)->decl.context) = current_function_decl;
	    }
	  else if (((current_class_type)->type.size) == (tree) 0)
	    {
	      set_nested_typename (d, ((((current_class_type)->type.name))->decl.arguments),
				   name, type);
	      ((d)->decl.context) = current_class_type;
	      (((d)->decl.lang_specific)->decl_flags.context) = current_class_type;
	    }
        }
      if (b->parm_flag == 2)
	{
	  (((type)->common.lang_flag_0)) = 1;
	    (((struct lang_identifier *)(name))->class_value) = ((type)->type.name);
	  if (((current_class_type)->type.size) == (tree) 0)
	    (((current_class_type)->type.lang_specific)->tags) = b->tags;
	}
    }
  if (((enum tree_code) (((type)->type.name))->common.code) == TYPE_DECL)
    (((type)->common.chain)) = ((type)->type.name);
  else
    {
      (((type)->common.chain)) = pushdecl (build_decl (TYPE_DECL, 0, type));
    }
}
static int anon_cnt = 0;
tree
make_anon_name ()
{
  char buf[32];
  sprintf (buf, "$_%d", anon_cnt++);
  return get_identifier (buf);
}
void
clear_anon_tags ()
{
  register struct binding_level *b;
  register tree tags;
  static int last_cnt = 0;
  if (last_cnt == anon_cnt)
    return;
  b = current_binding_level;
  while (b->tag_transparent)
    b = b->level_chain;
  tags = b->tags;
  while (tags)
    {
      if (((tags)->list.purpose) == (tree) 0)
	break;
      if ((((((tags)->list.purpose))->identifier.pointer)[0] == '$' 				  && ((((tags)->list.purpose))->identifier.pointer)[1] == '_'))
	((tags)->list.purpose) = (tree) 0;
      tags = ((tags)->common.chain);
    }
  last_cnt = anon_cnt;
}
static int
decls_match (newdecl, olddecl)
     tree newdecl, olddecl;
{
  int types_match;
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL && ((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL)
    {
      tree f1 = ((newdecl)->common.type);
      tree f2 = ((olddecl)->common.type);
      tree p1 = ((f1)->type.values);
      tree p2 = ((f2)->type.values);
      if (((enum tree_code) (f1)->common.code) == METHOD_TYPE && (((olddecl)->decl.lang_specific)->decl_flags.static_function))
	revert_static_member_fn (&f1, &newdecl, &p1);
      else if (((enum tree_code) (f2)->common.code) == METHOD_TYPE
	       && (((newdecl)->decl.lang_specific)->decl_flags.static_function))
	revert_static_member_fn (&f2, &olddecl, &p2);
      if (((enum tree_code) (f1)->common.code) != ((enum tree_code) (f2)->common.code))
	{
	  if (((enum tree_code) (f1)->common.code) == OFFSET_TYPE)
	    compiler_error_with_decl (newdecl, "`%s' redeclared as member function");
	  else
	    compiler_error_with_decl (newdecl, "`%s' redeclared as non-member function");
	  return 0;
	}
      if (comptypes (((((f1)->common.type))->type.main_variant),
		     ((((f2)->common.type))->type.main_variant), 1))
	types_match = compparms (p1, p2, 1);
      else types_match = 0;
    }
  else
    {
      if (((newdecl)->common.type) == error_mark_node)
	types_match = ((olddecl)->common.type) == error_mark_node;
      else if (((olddecl)->common.type) == (tree) 0)
	types_match = ((newdecl)->common.type) == (tree) 0;
      else
	types_match = comptypes (((newdecl)->common.type), ((olddecl)->common.type), 1);
    }
  return types_match;
}
static int
duplicate_decls (newdecl, olddecl)
     register tree newdecl, olddecl;
{
  extern struct obstack permanent_obstack;
  unsigned olddecl_uid = ((olddecl)->decl.uid);
  int olddecl_friend = 0, types_match;
  int new_defines_function;
  register unsigned saved_old_decl_uid;
  register int saved_old_decl_friend_p;
  if (((enum tree_code) (olddecl)->common.code) == TREE_LIST
      && ((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      tree olddecls = olddecl;
      if (((olddecls)->list.value) == (tree) 0)
	{
	  ((olddecls)->list.value) = newdecl;
	  return 1;
	}
      while (olddecls)
	{
	  if (decls_match (newdecl, ((olddecls)->list.value)))
	    {
	      if (((enum tree_code) (newdecl)->common.code) == VAR_DECL)
		;
	      else if ((((newdecl)->decl.lang_specific)->decl_flags.language)
		       != (((((olddecls)->list.value))->decl.lang_specific)->decl_flags.language))
		{
		  error_with_decl (newdecl, "declaration of `%s' with different language linkage");
		  error_with_decl (((olddecls)->list.value), "previous declaration here");
		}
	      types_match = 1;
	      break;
	    }
	  olddecls = ((olddecls)->common.chain);
	}
      if (olddecls)
	olddecl = ((olddecl)->list.value);
      else
	return 1;
    }
  else
    {
      if (((enum tree_code) (olddecl)->common.code) != TREE_LIST)
	olddecl_friend = ((olddecl)->decl.lang_specific) && (((olddecl)->decl.lang_specific)->decl_flags.friend_attr);
      types_match = decls_match (newdecl, olddecl);
    }
  if ((((newdecl)->common.type) && ((enum tree_code) (((newdecl)->common.type))->common.code) == ERROR_MARK)
      || (((olddecl)->common.type) && ((enum tree_code) (((olddecl)->common.type))->common.code) == ERROR_MARK))
    types_match = 0;
  if (((enum tree_code) (olddecl)->common.code) != ((enum tree_code) (newdecl)->common.code))
    {
      error_with_decl (newdecl, "`%s' redeclared as different kind of symbol");
      if (((enum tree_code) (olddecl)->common.code) == TREE_LIST)
	olddecl = ((olddecl)->list.value);
      error_with_decl (olddecl, "previous declaration of `%s'");
      return 0;
    }
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      if (((olddecl)->decl.vindex))
	((newdecl)->decl.vindex) = ((olddecl)->decl.vindex);
      if (((olddecl)->decl.context))
	((newdecl)->decl.context) = ((olddecl)->decl.context);
      if ((((olddecl)->decl.lang_specific)->decl_flags.context))
	(((newdecl)->decl.lang_specific)->decl_flags.context) = (((olddecl)->decl.lang_specific)->decl_flags.context);
      if ((((newdecl)->decl.lang_specific)->chain) == 0)
	(((newdecl)->decl.lang_specific)->chain) = (((olddecl)->decl.lang_specific)->chain);
      if ((((newdecl)->decl.lang_specific)->pending_inline_info) == 0)
	(((newdecl)->decl.lang_specific)->pending_inline_info) = (((olddecl)->decl.lang_specific)->pending_inline_info);
    }
  if (flag_traditional && ((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
      &&   (((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x    ? ((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x->implicit_decl : 0) == olddecl)
    ;
  else if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	   && ((olddecl)->decl.bit_field_flag))
    {
      if (!types_match)
	{
	  error_with_decl (newdecl, "declaration of `%s'");
	  error_with_decl (olddecl, "conflicts with built-in declaration `%s'");
	}
    }
  else if (!types_match)
    {
      tree oldtype = ((olddecl)->common.type);
      tree newtype = ((newdecl)->common.type);
      int give_error = 0;
      if (current_class_type == (tree) 0
	  ||   (((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x    ? ((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x->error_locus : 0) != current_class_type)
	{
	  give_error = 1;
	  error_with_decl (newdecl, "conflicting types for `%s'");
	}
      if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	  && comptypes (((oldtype)->common.type),
			((newtype)->common.type), 1)
	  && ((((oldtype)->type.values) == 0
	       && ((olddecl)->decl.initial) == 0)
	      || (((newtype)->type.values) == 0
		  && ((newdecl)->decl.initial) == 0)))
	{
	  register tree t = ((oldtype)->type.values);
	  if (t == 0)
	    t = ((newtype)->type.values);
	  for (; t; t = ((t)->common.chain))
	    {
	      register tree type = ((t)->list.value);
	      if (((t)->common.chain) == 0 && type != void_type_node)
		{
		  error ("A parameter list with an ellipsis can't match");
		  error ("an empty parameter name list declaration.");
		  break;
		}
	      if (((type)->type.main_variant) == float_type_node
		  ||   (((enum tree_code) ((type))->common.code) == INTEGER_TYPE				   && (((type)->type.main_variant) == char_type_node			       || ((type)->type.main_variant) == signed_char_type_node	       || ((type)->type.main_variant) == unsigned_char_type_node	       || ((type)->type.main_variant) == short_integer_type_node	       || ((type)->type.main_variant) == short_unsigned_type_node)))
		{
		  error ("An argument type that has a default promotion");
		  error ("can't match an empty parameter name list declaration.");
		  break;
		}
	    }
	}
      if (give_error)
	error_with_decl (olddecl, "previous declaration of `%s'");
      if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
	  && (((newdecl)->decl.lang_specific)->decl_flags.constructor_attr))
	{
	  tree tmp = ((((newtype)->type.values))->common.chain);
	  if (tmp != (tree) 0
	      && (((((tmp)->list.value))->type.main_variant)
		  == ((newtype)->type.maxval)))
	    {
	      tree parm = ((((newdecl)->decl.arguments))->common.chain);
	      tree argtypes
		= hash_tree_chain (build_reference_type (((tmp)->list.value)),
				   ((tmp)->common.chain));
	      ((parm)->decl.initial)   
		= ((parm)->common.type)
		  = ((((tmp)->list.value))->type.reference_to);
	      ((newdecl)->common.type) = newtype
		= build_cplus_method_type (((newtype)->type.maxval),
					   ((newtype)->common.type), argtypes);
	      error ("constructor cannot take as argument the type being constructed");
	        (((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x == 0 ? ((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(((newdecl)->decl.assembler_name)))->x->error_locus = ( current_class_type));
	    }
	}
    }
  else
    {
      char *errmsg = redeclaration_error_message (newdecl, olddecl);
      if (errmsg)
	{
	  error_with_decl (newdecl, errmsg);
	  if (((olddecl)->decl.name) != (tree) 0)
	    error_with_decl (olddecl,
			     (((olddecl)->decl.initial)
			      && current_binding_level == global_binding_level)
			        ? "`%s' previously defined here"
			        : "`%s' previously declared here");
	}
      else if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	       && ((olddecl)->decl.initial) != 0
	       && ((((olddecl)->common.type))->type.values) == 0
	       && ((((newdecl)->common.type))->type.values) != 0)
	{
	  warning_with_decl (newdecl, "prototype for `%s'");
	  warning_with_decl (olddecl,
			     "follows non-prototype definition here");
	}
      if (pedantic
	  && (((newdecl)->common.readonly_flag) != ((olddecl)->common.readonly_flag)
	      || ((newdecl)->common.volatile_flag) != ((olddecl)->common.volatile_flag)))
	error_with_decl (newdecl, "type qualifiers for `%s' conflict with previous decl");
    }
  if (((enum tree_code) (olddecl)->common.code) == TYPE_DECL)
    {
      if (((((newdecl)->common.type))->type.lang_specific)
          && ((((olddecl)->common.type))->type.lang_specific))
	{
	  (((((newdecl)->common.type))->type.lang_specific)->vsize)
	    = (((((olddecl)->common.type))->type.lang_specific)->vsize);
	  (((((newdecl)->common.type))->type.lang_specific)->friend_classes)
	    = (((((olddecl)->common.type))->type.lang_specific)->friend_classes);
	}
    }
  new_defines_function = (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
			  && ((newdecl)->decl.initial) != 0);
  if (warn_redundant_decls && ((olddecl)->decl.linenum) != 0
      && !(new_defines_function && ((olddecl)->decl.initial) == 0))
    {
      warning_with_decl (newdecl, "redundant redeclaration of `%s' in same scope");
      warning_with_decl (olddecl, "previous declaration of `%s'");
    }
  if (types_match)
    {
      tree oldtype = ((olddecl)->common.type);
      tree newtype = common_type (((newdecl)->common.type), ((olddecl)->common.type));
      if (((enum tree_code) (newdecl)->common.code) == VAR_DECL)
	(((newdecl)->decl.lang_flag_2)) |= (((olddecl)->decl.lang_flag_2));
      else if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
	  && (((((newdecl)->common.type))->type.noncopied_parts)
	      != ((((olddecl)->common.type))->type.noncopied_parts)))
	{
	  tree ctype = (tree) 0;
	  ctype = (((newdecl)->decl.lang_specific)->decl_flags.context);
	  ((newdecl)->common.type) = build_exception_variant (ctype, newtype,
							 ((((newdecl)->common.type))->type.noncopied_parts));
	  ((olddecl)->common.type) = build_exception_variant (ctype, newtype,
							 ((oldtype)->type.noncopied_parts));
	  if (! compexcepttypes (((newdecl)->common.type), ((olddecl)->common.type)))
	    {
	      error_with_decl (newdecl, "declaration of `%s' raises different exceptions...");
	      error_with_decl (olddecl, "...from previous declaration here");
	    }
	}
      ((newdecl)->common.type) = ((olddecl)->common.type) = newtype;
      if (oldtype != ((newdecl)->common.type))
	{
	  if (((newdecl)->common.type) != error_mark_node)
	    layout_type (((newdecl)->common.type));
	  if (((enum tree_code) (newdecl)->common.code) != FUNCTION_DECL
	      && ((enum tree_code) (newdecl)->common.code) != TYPE_DECL
	      && ((enum tree_code) (newdecl)->common.code) != CONST_DECL)
	    layout_decl (newdecl, 0);
	}
      else
	{
	  ((newdecl)->decl.size) = ((olddecl)->decl.size);
	}
      if (((newdecl)->common.readonly_flag))
	((olddecl)->common.readonly_flag) = 1;
      if (((newdecl)->common.volatile_flag))
	((olddecl)->common.volatile_flag) = 1;
      if (((newdecl)->decl.initial) == 0)
	((newdecl)->decl.initial) = ((olddecl)->decl.initial);
      if ((((olddecl)->decl.lang_specific)
	   && !(((olddecl)->decl.lang_specific)->decl_flags.abstract_virtual))
	  || ((olddecl)->decl.rtl) != ((abort_fndecl)->decl.rtl))
	((newdecl)->decl.rtl) = ((olddecl)->decl.rtl);
    }
  else
    {
      tree oldstatic = value_member (olddecl, static_aggregates);
      if (oldstatic)
	((oldstatic)->list.value) = error_mark_node;
      ((olddecl)->common.type) = ((newdecl)->common.type);
      ((olddecl)->common.readonly_flag) = ((newdecl)->common.readonly_flag);
      ((olddecl)->common.volatile_flag) = ((newdecl)->common.volatile_flag);
      ((olddecl)->common.side_effects_flag) = ((newdecl)->common.side_effects_flag);
    }
  if (((newdecl)->decl.external_flag))
    {
      ((newdecl)->common.static_flag) = ((olddecl)->common.static_flag);
      ((newdecl)->decl.external_flag) = ((olddecl)->decl.external_flag);
      if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
	{
	  ((newdecl)->common.public_flag) &= ((olddecl)->common.public_flag);
	  ((olddecl)->common.public_flag) = ((newdecl)->common.public_flag);
	  if (! ((olddecl)->common.public_flag))
	    ((((olddecl)->decl.assembler_name))->common.public_flag) = 0;
	}
      else
	((newdecl)->common.public_flag) = ((olddecl)->common.public_flag);
    }
  else
    {
      ((olddecl)->common.static_flag) = ((newdecl)->common.static_flag);
      if (((enum tree_code) (newdecl)->common.code) == VAR_DECL
	  && ((newdecl)->common.readonly_flag) && ((newdecl)->common.static_flag)
	  && ! (((newdecl)->decl.lang_flag_2)))
	((newdecl)->common.public_flag) = 0;
      ((olddecl)->common.public_flag) = ((newdecl)->common.public_flag);
    }
  if (((newdecl)->decl.inline_flag) && ((olddecl)->decl.initial) == 0)
    ((olddecl)->decl.inline_flag) = 1;
  ((newdecl)->decl.inline_flag) = ((olddecl)->decl.inline_flag);
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      if (new_defines_function)
	(((newdecl)->decl.lang_specific)->decl_flags.language) = (((olddecl)->decl.lang_specific)->decl_flags.language);
      else
	{
	  if (((olddecl)->decl.bit_field_flag))
	    {
	      ((newdecl)->decl.bit_field_flag) = 1;
	       ((newdecl)->decl.frame_size = (int) (  ((enum built_in_function) (olddecl)->decl.frame_size)));
	      ((newdecl)->decl.rtl) = ((olddecl)->decl.rtl);
	    }
	  else
	    ((newdecl)->decl.frame_size) = ((olddecl)->decl.frame_size);
	  ((newdecl)->decl.result) = ((olddecl)->decl.result);
	  if (((newdecl)->decl.saved_insns.r) = ((olddecl)->decl.saved_insns.r))
	    ((newdecl)->decl.initial) = ((olddecl)->decl.initial);
	  if (((olddecl)->decl.arguments))
	    ((newdecl)->decl.arguments) = ((olddecl)->decl.arguments);
	}
    }
  ((newdecl)->common.addressable_flag) = ((olddecl)->common.addressable_flag);
  ((newdecl)->common.asm_written_flag) = ((olddecl)->common.asm_written_flag);
  if (((olddecl)->decl.lang_specific))
    (((newdecl)->decl.lang_flag_3)) = (((olddecl)->decl.lang_flag_3));
  saved_old_decl_uid = ((olddecl)->decl.uid);
  saved_old_decl_friend_p
    = ((olddecl)->decl.lang_specific) ? (((olddecl)->decl.lang_specific)->decl_flags.friend_attr) : 0;
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      int function_size;
      struct lang_decl *ol = ((olddecl)->decl.lang_specific);
      struct lang_decl *nl = ((newdecl)->decl.lang_specific);
      function_size = sizeof (struct tree_decl);
      bcopy ((char *) newdecl + sizeof (struct tree_common),
	     (char *) olddecl + sizeof (struct tree_common),
	     function_size - sizeof (struct tree_common));
      if ((char *)newdecl + ((function_size + sizeof (struct lang_decl)
			     + ((&permanent_obstack)->alignment_mask))
			     & ~ ((&permanent_obstack)->alignment_mask))
	  == ((&permanent_obstack)->next_free))
	{
	  (((newdecl)->decl.lang_specific)->main_decl_variant) = olddecl;
	  ((olddecl)->decl.lang_specific) = ol;
	  bcopy ((char *)nl, (char *)ol, sizeof (struct lang_decl));
	  ( (&permanent_obstack)->temp = (char *)( newdecl) - (char *) (&permanent_obstack)->chunk,			  (((&permanent_obstack)->temp > 0 && (&permanent_obstack)->temp < (&permanent_obstack)->chunk_limit - (char *) (&permanent_obstack)->chunk)   ? (int) ((&permanent_obstack)->next_free = (&permanent_obstack)->object_base					    = (&permanent_obstack)->temp + (char *) (&permanent_obstack)->chunk)				   : (_obstack_free ((&permanent_obstack), (&permanent_obstack)->temp + (char *) (&permanent_obstack)->chunk), 0)));
	}
      else if (((ol)->decl_flags.permanent_attr))
	{
	  if ((((olddecl)->decl.lang_specific)->main_decl_variant) == olddecl)
	    {
	      extern tree free_lang_decl_chain;
	      tree free_lang_decl = (tree) ol;
	      ((free_lang_decl)->common.chain) = free_lang_decl_chain;
	      free_lang_decl_chain = free_lang_decl;
	    }
	  else
	    {
	    }
	}
    }
  else
    {
      bcopy ((char *) newdecl + sizeof (struct tree_common),
	     (char *) olddecl + sizeof (struct tree_common),
	     sizeof (struct tree_decl) - sizeof (struct tree_common)
	     + tree_code_length [(int)((enum tree_code) (newdecl)->common.code)] * sizeof (char *));
    }
  ((olddecl)->decl.uid) = olddecl_uid;
  if (olddecl_friend)
    (((olddecl)->decl.lang_specific)->decl_flags.friend_attr) = 1;
  ((olddecl)->decl.uid) = saved_old_decl_uid;
  if (((olddecl)->decl.lang_specific))
    (((olddecl)->decl.lang_specific)->decl_flags.friend_attr) |= saved_old_decl_friend_p;
  return 1;
}
void
adjust_type_value (id)
     tree id;
{
  tree t;
  if (current_binding_level != global_binding_level)
    {
      if (current_binding_level != class_binding_level)
	{
	  t =   (((struct lang_identifier *)(id))->local_value);
	  if (t && ((enum tree_code) (t)->common.code) == TYPE_DECL)
	    {
	    set_it:
	      (((id)->common.type) =  ((t)->common.type));
	      return;
	    }
	}
      else
	my_friendly_abort (7);
      if (current_class_type)
	{
	  t =   (((struct lang_identifier *)(id))->class_value);
	  if (t && ((enum tree_code) (t)->common.code) == TYPE_DECL)
	    goto set_it;
	}
    }
  t =   (((struct lang_identifier *)(id))->global_value);
  if (t && ((enum tree_code) (t)->common.code) == TYPE_DECL)
    goto set_it;
  if (t && ((enum tree_code) (t)->common.code) == TEMPLATE_DECL)
    (((id)->common.type) =  (tree) 0);
}
tree
pushdecl (x)
     tree x;
{
  register tree t;
  register tree name = ((x)->decl.assembler_name);
  register struct binding_level *b = current_binding_level;
  if (x != current_function_decl
      && (((enum tree_code) (x)->common.code) != FUNCTION_DECL
	  || !((x)->decl.virtual_flag)))
    ((x)->decl.context) = current_function_decl;
  if (((enum tree_code) (x)->common.code) == TYPE_DECL)
    name = ((x)->decl.name);
  if (name)
    {
      char *file;
      int line;
      t = lookup_name_current_level (name);
      if (t != 0 && t == error_mark_node)
	{
	  t = 0;
	  error_with_decl (x, "`%s' used prior to declaration");
	}
      if (t != 0)
	{
	  if (((enum tree_code) (t)->common.code) == PARM_DECL)
	    {
	      if (((t)->decl.context) == (tree) 0)
		fatal ("parse errors have confused me too much");
	    }
	  file = ((t)->decl.filename);
	  line = ((t)->decl.linenum);
	}
      if (t != 0 && ((enum tree_code) (t)->common.code) != ((enum tree_code) (x)->common.code))
	{
	  if (((enum tree_code) (t)->common.code) == TYPE_DECL || ((enum tree_code) (x)->common.code) == TYPE_DECL)
	    {
	      ;
	    }
	  else if (duplicate_decls (x, t))
	    return t;
	}
      else if (t != 0 && duplicate_decls (x, t))
	{
	  if (!flag_traditional && ((name)->common.public_flag)
	      && ! ((x)->common.public_flag) && ! ((x)->decl.external_flag) && ! ((x)->decl.inline_flag))
	    {
	      if (current_function_decl == x)
		current_function_decl = t;
	      if (  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0))
		warning ("`%s' was declared implicitly `extern' and later `static'",
			 lang_printable_name (t));
	      else
		warning ("`%s' was declared `extern' and later `static'",
			 lang_printable_name (t));
	      warning_with_file_and_line (file, line,
					  "previous declaration of `%s'",
					  lang_printable_name (t));
	    }
	  return t;
	}
      if (((enum tree_code) (x)->common.code) == TYPE_DECL)
	{
	  tree name = ((((x)->common.type))->type.name);
	  if (name == (tree) 0 || ((enum tree_code) (name)->common.code) != TYPE_DECL)
	    {
              name = x;
              if (global_bindings_p ())
                ((((x)->common.type))->type.name) = x;
	    }
	  else
	    {
	      tree tname = ((name)->decl.name);
	      if (global_bindings_p () && (((tname)->identifier.pointer)[0] == '$' 				  && ((tname)->identifier.pointer)[1] == '_'))
		{
		  ((((x)->common.type))->type.name) = x;
		  pushtag (tname, ((x)->common.type));
		}
	    }
	  my_friendly_assert (((enum tree_code) (name)->common.code) == TYPE_DECL, 140);
	  if (((name)->decl.name) && !((name)->decl.arguments))
	    set_nested_typename (x, current_class_name, ((name)->decl.name),
				 ((x)->common.type));
	  if (((((x)->common.type))->type.name) && (((((((x)->common.type))->type.name))->decl.name)))
            set_identifier_type_value (((x)->decl.name), ((x)->common.type));
	}
      if (((x)->decl.external_flag) &&   (((struct lang_identifier *)(name))->global_value) != 0
	  && (((  (((struct lang_identifier *)(name))->global_value))->decl.external_flag)
	      || ((  (((struct lang_identifier *)(name))->global_value))->common.public_flag))
	  && !((x)->decl.inline_flag))
	{
	  if (! comptypes (((x)->common.type),
			   ((  (((struct lang_identifier *)(name))->global_value))->common.type), 1))
	    {
	      warning_with_decl (x,
				 "type mismatch with previous external decl");
	      warning_with_decl (  (((struct lang_identifier *)(name))->global_value),
				 "previous external decl of `%s'");
	    }
	}
      if (flag_traditional && ((x)->decl.external_flag)
	  && lookup_name (name, 0) == 0)
	b = global_binding_level;
      if (b == global_binding_level)
	{
	  if (((enum tree_code) (x)->common.code) == VAR_DECL
	      && ((x)->common.readonly_flag) && ! (((x)->decl.lang_flag_2)))
	    ((x)->common.public_flag) = 0;
	  if (  (((struct lang_identifier *)(name))->global_value) == 0 && ((x)->common.public_flag))
	    ((name)->common.public_flag) = 1;
	  if (((enum tree_code) (x)->common.code) != TYPE_DECL
	      || t == (tree) 0
	      || ((enum tree_code) (t)->common.code) == TYPE_DECL)
	      (((struct lang_identifier *)(name))->global_value) = x;
	  if (  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0)
	      && ((  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0))->common.used_flag))
	    ((x)->common.used_flag) = 1;
	  if (  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0)
	      && ((  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0))->common.addressable_flag))
	    ((x)->common.addressable_flag) = 1;
	  if (  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0) != 0
	      && ! (((enum tree_code) (x)->common.code) == FUNCTION_DECL
		    && ((((x)->common.type))->common.type) == integer_type_node))
	    warning ("`%s' was previously implicitly declared to return `int'",
		     lang_printable_name (x));
	  if (((name)->common.public_flag)
	      && ((enum tree_code) (x)->common.code) != TYPE_DECL
	      && ! ((x)->common.public_flag) && ! ((x)->decl.external_flag))
	    {
	      if (  (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->implicit_decl : 0))
		warning ("`%s' was declared implicitly `extern' and later `static'",
			 lang_printable_name (x));
	      else
		warning ("`%s' was declared `extern' and later `static'",
			 lang_printable_name (x));
	    }
	}
      else
	{
	  tree oldlocal =   (((struct lang_identifier *)(name))->local_value);
	  tree oldglobal =   (((struct lang_identifier *)(name))->global_value);
	  set_identifier_local_value (name, x);
	  if (oldlocal == 0
	      && ((x)->decl.external_flag) && !((x)->decl.inline_flag)
	      && oldglobal != 0
	      && ((enum tree_code) (x)->common.code) == FUNCTION_DECL
	      && ((enum tree_code) (oldglobal)->common.code) == FUNCTION_DECL)
	    {
	      if (! comptypes (((x)->common.type), ((oldglobal)->common.type), 1))
		warning_with_decl (x, "extern declaration of `%s' doesn't match global one");
	      else
		{
		  if (((oldglobal)->decl.inline_flag))
		    {
		      ((x)->decl.inline_flag) = ((oldglobal)->decl.inline_flag);
		      ((x)->decl.initial) = (current_function_decl == oldglobal
					  ? 0 : ((oldglobal)->decl.initial));
		      ((x)->decl.saved_insns.r) = ((oldglobal)->decl.saved_insns.r);
		      ((x)->decl.arguments) = ((oldglobal)->decl.arguments);
		      ((x)->decl.result) = ((oldglobal)->decl.result);
		      ((x)->common.asm_written_flag) = ((oldglobal)->common.asm_written_flag);
		      ((x)->decl.abstract_origin) = oldglobal;
		    }
		  if (((oldglobal)->decl.bit_field_flag))
		    {
		      ((x)->decl.bit_field_flag) = ((oldglobal)->decl.bit_field_flag);
		       ((x)->decl.frame_size = (int) (  ((enum built_in_function) (oldglobal)->decl.frame_size)));
		    }
		  if (((((oldglobal)->common.type))->type.values) != 0
		      && ((oldglobal)->decl.initial)
		      && ((((x)->common.type))->type.values) == 0)
		    ((x)->common.type) = ((oldglobal)->common.type);
		}
	    }
	  if (oldlocal == 0
	      && oldglobal == 0
	      && ((x)->decl.external_flag)
	      && ((x)->common.public_flag))
	    {
	      ((name)->common.public_flag) = 1;
	    }
	  if ((((x)->decl.abstract_origin) != (tree) 0))
	    ;
	  else if (oldlocal != 0 && !((x)->decl.external_flag)
	      && ((enum tree_code) (oldlocal)->common.code) == PARM_DECL
	      && ((enum tree_code) (x)->common.code) != PARM_DECL)
	    {
	      struct binding_level *b = current_binding_level->level_chain;
	      if (cleanup_label)
		b = b->level_chain;
	      if (b->parm_flag == 1)
		warning ("declaration of `%s' shadows a parameter",
			 ((name)->identifier.pointer));
	    }
	  else if (warn_shadow && !((x)->decl.external_flag)
		   && ((x)->decl.linenum) != 0
		   && ! (((x)->decl.abstract_origin) != (tree) 0))
	    {
	      char *warnstring = 0;
	      if (oldlocal != 0 && ((enum tree_code) (oldlocal)->common.code) == PARM_DECL)
		warnstring = "declaration of `%s' shadows a parameter";
	      else if (  (((struct lang_identifier *)(name))->class_value) != 0)
		warnstring = "declaration of `%s' shadows a member of `this'";
	      else if (oldlocal != 0)
		warnstring = "declaration of `%s' shadows previous local";
	      else if (oldglobal != 0)
		warnstring = "declaration of `%s' shadows global declaration";
	      if (warnstring)
		warning (warnstring, ((name)->identifier.pointer));
	    }
	  if (oldlocal != 0)
	    b->shadowed = tree_cons (name, oldlocal, b->shadowed);
	}
      if (((enum tree_code) (x)->common.code) != TEMPLATE_DECL
	  && ((enum tree_code) (x)->common.code) != CPLUS_CATCH_DECL
	  && ((((x)->common.type))->type.size) == 0
	  &&   ((( ARRAY_TYPE) == ((enum tree_code) (((x)->common.type))->common.code)			       && (((((((x)->common.type))->common.type))->type.lang_flag_5)))	   || (((((x)->common.type))->type.lang_flag_5))))
	{
	  if (++b->n_incomplete == 0)
	    error ("too many incomplete variables at this point");
	}
    }
  if (((enum tree_code) (x)->common.code) == TYPE_DECL && name != (tree) 0)
    {
      adjust_type_value (name);
      if (current_class_name)
	{
	  if (!((x)->decl.arguments))
	    set_nested_typename (x, current_class_name, ((x)->decl.name),
				 ((x)->common.type));
	  adjust_type_value (((x)->decl.arguments));
	}
    }
  ((x)->common.chain) = b->names;
  b->names = x;
  if (! (b != global_binding_level || ((x)->common.permanent_flag)))
    my_friendly_abort (124);
  return x;
}
tree
pushdecl_top_level (x)
     tree x;
{
  register tree t;
  register struct binding_level *b = current_binding_level;
  current_binding_level = global_binding_level;
  t = pushdecl (x);
  current_binding_level = b;
  if (class_binding_level)
    b = class_binding_level;
  if (((enum tree_code) (x)->common.code) == TYPE_DECL)
    {
      tree name = ((x)->decl.name);
      tree newval;
      tree *ptr = 0;
      for (; b != global_binding_level; b = b->level_chain)
        {
          tree shadowed = b->type_shadowed;
          for (; shadowed; shadowed = ((shadowed)->common.chain))
            if (((shadowed)->list.purpose) == name)
              {
		ptr = &((shadowed)->list.value);
              }
        }
      newval = ((x)->common.type);
      if (ptr == 0)
        {
	  (((name)->common.type) =  newval);
	}
      else
        {
	  *ptr = newval;
        }
    }
  return t;
}
void
push_overloaded_decl_top_level (x, forget)
     tree x;
     int forget;
{
  struct binding_level *b = current_binding_level;
  current_binding_level = global_binding_level;
  push_overloaded_decl (x, forget);
  current_binding_level = b;
}
tree
pushdecl_class_level (x)
     tree x;
{
  register tree name = ((x)->decl.name);
  if (name)
    {
      tree oldclass =   (((struct lang_identifier *)(name))->class_value);
      if (oldclass)
	class_binding_level->class_shadowed
	  = tree_cons (name, oldclass, class_binding_level->class_shadowed);
        (((struct lang_identifier *)(name))->class_value) = x;
      ( (((&decl_obstack)->next_free + sizeof (char *) > (&decl_obstack)->chunk_limit)		   ? (_obstack_newchunk ((&decl_obstack), sizeof (char *)), 0) : 0),		  *((char **)(((&decl_obstack)->next_free+=sizeof(char *))-sizeof(char *))) = ((char *) x));
      if (((enum tree_code) (x)->common.code) == TYPE_DECL && !((x)->decl.arguments))
	set_nested_typename (x, current_class_name, name, ((x)->common.type));
    }
  return x;
}
int
overloaded_globals_p (list)
     tree list;
{
  my_friendly_assert (((enum tree_code) (list)->common.code) == TREE_LIST, 142);
  if ((((list)->common.lang_flag_0)))
    return -1;
  if (((enum tree_code) (((list)->list.purpose))->common.code) == IDENTIFIER_NODE)
    return 1;
  return 0;
}
tree
push_overloaded_decl (decl, forgettable)
     tree decl;
     int forgettable;
{
  tree orig_name = ((decl)->decl.name);
  tree glob =   (((struct lang_identifier *)(orig_name))->global_value);
  (((decl)->decl.lang_flag_4)) = 1;
  if (glob)
    {
      if (((enum tree_code) (glob)->common.code) != TREE_LIST)
	{
	  if ((((decl)->decl.lang_specific)->decl_flags.language) == lang_c)
	    {
	      if (((enum tree_code) (glob)->common.code) == FUNCTION_DECL)
		{
		  if ((((glob)->decl.lang_specific)->decl_flags.language) == lang_c)
		    {
		      error_with_decl (decl, "C-language function `%s' overloaded here");
		      error_with_decl (glob, "Previous C-language version of this function was `%s'");
		    }
		}
	      else
		my_friendly_abort (9);
	    }
	  if (forgettable
	      && ! flag_traditional
	      && ((glob)->common.permanent_flag) == 1
	      && !global_bindings_p ())
	    overloads_to_forget = tree_cons (orig_name, glob, overloads_to_forget);
	  if (((enum tree_code) (glob)->common.code) == ADDR_EXPR)
	    glob = ((glob)->exp.operands[ 0]);
	  if (((enum tree_code) (glob)->common.code) == FUNCTION_DECL
	      && (((glob)->decl.lang_specific)->decl_flags.language) != (((decl)->decl.lang_specific)->decl_flags.language)
	      && comptypes (((glob)->common.type), ((decl)->common.type), 1))
	    {
	      if (current_lang_stack == current_lang_base)
		{
		  (((decl)->decl.lang_specific)->decl_flags.language) = (((glob)->decl.lang_specific)->decl_flags.language);
		  return glob;
		}
	      else
		{
		  error_with_decl (decl, "conflicting language contexts for declaration of `%s';");
		  error_with_decl (glob, "conflicts with previous declaration here");
		}
	    }
	  if (pedantic && ((enum tree_code) (glob)->common.code) == VAR_DECL)
	    {
	      my_friendly_assert ((*tree_code_type[(int) (((enum tree_code) (glob)->common.code))]) == 'd', 143);
	      error_with_decl (glob, "non-function declaration `%s'");
	      error_with_decl (decl, "conflicts with function declaration `%s'");
	    }
	  glob = tree_cons (orig_name, glob, (tree) 0);
	  glob = tree_cons (((glob)->list.purpose), decl, glob);
	    (((struct lang_identifier *)(orig_name))->global_value) = glob;
	  ((glob)->common.type) = unknown_type_node;
	  return decl;
	}
      if (((glob)->list.value) == (tree) 0)
	{
	  ((glob)->list.value) = decl;
	  return decl;
	}
      if (((enum tree_code) (decl)->common.code) != TEMPLATE_DECL)
        {
          tree name = ((decl)->decl.assembler_name);
          tree tmp;
	  for (tmp = glob; tmp; tmp = ((tmp)->common.chain))
	    {
	      if (((enum tree_code) (((tmp)->list.value))->common.code) == FUNCTION_DECL
		  && (((((tmp)->list.value))->decl.lang_specific)->decl_flags.language) != (((decl)->decl.lang_specific)->decl_flags.language)
		  && comptypes (((((tmp)->list.value))->common.type), ((decl)->common.type),
				1))
		{
		  error_with_decl (decl,
				   "conflicting language contexts for declaration of `%s';");
		  error_with_decl (((tmp)->list.value),
				   "conflicts with previous declaration here");
		}
	      if (((enum tree_code) (((tmp)->list.value))->common.code) != TEMPLATE_DECL
		  && ((((tmp)->list.value))->decl.assembler_name) == name)
		return decl;
	    }
	}
    }
  if ((((decl)->decl.lang_specific)->decl_flags.language) == lang_c)
    {
      tree decls = glob;
      while (decls && (((((decls)->list.value))->decl.lang_specific)->decl_flags.language) == lang_cplusplus)
	decls = ((decls)->common.chain);
      if (decls)
	{
	  error_with_decl (decl, "C-language function `%s' overloaded here");
	  error_with_decl (((decls)->list.value), "Previous C-language version of this function was `%s'");
	}
    }
  if (forgettable
      && ! flag_traditional
      && (glob == 0 || ((glob)->common.permanent_flag) == 1)
      && !global_bindings_p ()
      && !pseudo_global_level_p ())
    overloads_to_forget = tree_cons (orig_name, glob, overloads_to_forget);
  glob = tree_cons (orig_name, decl, glob);
    (((struct lang_identifier *)(orig_name))->global_value) = glob;
  ((glob)->common.type) = unknown_type_node;
  return decl;
}
tree
implicitly_declare (functionid)
     tree functionid;
{
  register tree decl;
  int temp = allocation_temporary_p ();
  push_obstacks_nochange ();
  if (temp && (flag_traditional || !warn_implicit
	       || current_binding_level == global_binding_level))
    end_temporary_allocation ();
    decl = build_lang_decl (FUNCTION_DECL, functionid, default_function_type);
  ((decl)->decl.external_flag) = 1;
  ((decl)->common.public_flag) = 1;
  pushdecl (decl);
  rest_of_decl_compilation (decl, 0, 0, 0);
  if (warn_implicit
      &&   (((struct lang_identifier *)(functionid))->x    ? ((struct lang_identifier *)(functionid))->x->implicit_decl : 0) == 0)
    {
      pedwarn ("implicit declaration of function `%s'",
	       ((functionid)->identifier.pointer));
    }
    (((struct lang_identifier *)(functionid))->x == 0 ? ((struct lang_identifier *)(functionid))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(functionid))->x->implicit_decl = ( decl));
  pop_obstacks ();
  return decl;
}
static char *
redeclaration_error_message (newdecl, olddecl)
     tree newdecl, olddecl;
{
  if (((enum tree_code) (newdecl)->common.code) == TYPE_DECL)
    {
      if (((olddecl)->common.type) == ((newdecl)->common.type))
	return 0;
      else
	return "redefinition of `%s'";
    }
  else if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      if (((olddecl)->decl.lang_specific) && (((olddecl)->decl.lang_specific)->decl_flags.abstract_virtual))
	return 0;
      if (((olddecl)->decl.initial) != 0 && ((newdecl)->decl.initial) != 0
	  && !(((olddecl)->decl.inline_flag) && ((olddecl)->decl.external_flag)
	       && !(((newdecl)->decl.inline_flag) && ((newdecl)->decl.external_flag))))
	{
	  if (((olddecl)->decl.name) == (tree) 0)
	    return "`%s' not declared in class";
	  else
	    return "redefinition of `%s'";
	}
      return 0;
    }
  else if (current_binding_level == global_binding_level)
    {
      if (((newdecl)->decl.external_flag) || ((olddecl)->decl.external_flag))
	return 0;
      if (((olddecl)->decl.initial) != 0 && ((newdecl)->decl.initial) != 0)
	return "redefinition of `%s'";
      if (((olddecl)->common.public_flag) != ((newdecl)->common.public_flag))
	return "conflicting declarations of `%s'";
      return 0;
    }
  else
    {
      if (!(((newdecl)->decl.external_flag) && ((olddecl)->decl.external_flag)))
	return "redeclaration of `%s'";
      return 0;
    }
}
tree
lookup_label (id)
     tree id;
{
  register tree decl =   (((struct lang_identifier *)(id))->x    ? ((struct lang_identifier *)(id))->x->label_value : 0);
  if ((decl == 0
      || ((decl)->decl.linenum) == 0)
      && (named_label_uses == 0
	  || ((named_label_uses)->list.purpose) != current_binding_level->names
	  || ((named_label_uses)->list.value) != decl))
    {
      named_label_uses
	= tree_cons (current_binding_level->names, decl, named_label_uses);
      ((named_label_uses)->common.type) = (tree)current_binding_level;
    }
  if (decl != 0)
    {
      if (((decl)->decl.context) != current_function_decl
	  && ! ((decl)->common.lang_flag_1))
	return shadow_label (id);
      return decl;
    }
  decl = build_decl (LABEL_DECL, id, void_type_node);
  ((decl)->decl.context) = current_function_decl;
  ((decl)->decl.mode) = VOIDmode;
  ((decl)->decl.linenum) = lineno;
  ((decl)->decl.filename) = input_filename;
    (((struct lang_identifier *)(id))->x == 0 ? ((struct lang_identifier *)(id))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(id))->x->label_value = ( decl));
  named_labels = tree_cons ((tree) 0, decl, named_labels);
  ((named_label_uses)->list.value) = decl;
  return decl;
}
tree
shadow_label (name)
     tree name;
{
  register tree decl =   (((struct lang_identifier *)(name))->x    ? ((struct lang_identifier *)(name))->x->label_value : 0);
  if (decl != 0)
    {
      shadowed_labels = tree_cons ((tree) 0, decl, shadowed_labels);
        (((struct lang_identifier *)(name))->x == 0 ? ((struct lang_identifier *)(name))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(name))->x->label_value = ( 0));
        (((struct lang_identifier *)(decl))->x == 0 ? ((struct lang_identifier *)(decl))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(decl))->x->label_value = ( 0));
    }
  return lookup_label (name);
}
tree
define_label (filename, line, name)
     char *filename;
     int line;
     tree name;
{
  tree decl = lookup_label (name);
  current_binding_level->more_cleanups_ok = 0;
  if (decl != 0 && ((decl)->decl.context) != current_function_decl)
    {
      shadowed_labels = tree_cons ((tree) 0, decl, shadowed_labels);
        (((struct lang_identifier *)(name))->x == 0 ? ((struct lang_identifier *)(name))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(name))->x->label_value = ( 0));
      decl = lookup_label (name);
    }
  if (((decl)->decl.initial) != 0)
    {
      error_with_decl (decl, "duplicate label `%s'");
      return 0;
    }
  else
    {
      tree uses, prev;
      ((decl)->decl.initial) = error_mark_node;
      ((decl)->decl.filename) = filename;
      ((decl)->decl.linenum) = line;
      for (prev = 0, uses = named_label_uses;
	   uses;
	   prev = uses, uses = ((uses)->common.chain))
	if (((uses)->list.value) == decl)
	  {
	    struct binding_level *b = current_binding_level;
	    while (1)
	      {
		tree new_decls = b->names;
		tree old_decls = ((tree)b == ((uses)->common.type)
				  ? ((uses)->list.purpose) : (tree) 0);
		while (new_decls != old_decls)
		  {
		    if (((enum tree_code) (new_decls)->common.code) == VAR_DECL
			&& ! (!strncmp (((((new_decls)->decl.name))->identifier.pointer), "_$tmp_", sizeof ("_$tmp_")-1))
			&& ((((new_decls)->decl.initial) != (tree) 0
			     && ((new_decls)->decl.initial) != error_mark_node)
			    || (((((new_decls)->common.type))->type.lang_flag_3))))
		      {
			if (  (((struct lang_identifier *)(decl))->x    ? ((struct lang_identifier *)(decl))->x->error_locus : 0) == (tree) 0)
			  error_with_decl (decl, "invalid jump to label `%s'");
			  (((struct lang_identifier *)(decl))->x == 0 ? ((struct lang_identifier *)(decl))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(decl))->x->error_locus = ( current_function_decl));
			error_with_decl (new_decls, "crosses initialization of `%s'");
		      }
		    new_decls = ((new_decls)->common.chain);
		  }
		if ((tree)b == ((uses)->common.type))
		  break;
		b = b->level_chain;
	      }
	    if (prev)
	      ((prev)->common.chain) = ((uses)->common.chain);
	    else
	      named_label_uses = ((uses)->common.chain);
	  }
      current_function_return_value = (tree) 0;
      return decl;
    }
}
void
define_case_label (decl)
     tree decl;
{
  tree cleanup = last_cleanup_this_contour ();
  if (cleanup)
    {
      static int explained = 0;
      error_with_decl (((cleanup)->list.purpose), "destructor needed for `%s'");
      error ("where case label appears here");
      if (!explained)
	{
	  error ("(enclose actions of previous case statements requiring");
	  error ("destructors in their own binding contours.)");
	  explained = 1;
	}
    }
  current_binding_level->more_cleanups_ok = 0;
  current_function_return_value = (tree) 0;
}
tree
getdecls ()
{
  return current_binding_level->names;
}
tree
gettags ()
{
  return current_binding_level->tags;
}
static void
storedecls (decls)
     tree decls;
{
  current_binding_level->names = decls;
}
static void
storetags (tags)
     tree tags;
{
  current_binding_level->tags = tags;
}
static tree
lookup_tag (form, name, binding_level, thislevel_only)
     enum tree_code form;
     struct binding_level *binding_level;
     tree name;
     int thislevel_only;
{
  register struct binding_level *level;
  for (level = binding_level; level; level = level->level_chain)
    {
      register tree tail;
      if ((((name)->identifier.pointer)[0] == '$' 				  && ((name)->identifier.pointer)[1] == '_'))
	for (tail = level->tags; tail; tail = ((tail)->common.chain))
	  {
	    if ((((((((tail)->list.value))->type.name))->decl.name)) == name)
	      return ((tail)->list.value);
	  }
      else
	for (tail = level->tags; tail; tail = ((tail)->common.chain))
	  {
	    if (((tail)->list.purpose) == name)
	      {
		enum tree_code code = ((enum tree_code) (((tail)->list.value))->common.code);
		if (code != form
		    && !(form != ENUMERAL_TYPE
			 && (code == TEMPLATE_DECL
			     || code == UNINSTANTIATED_P_TYPE)))
		  {
		    error ("`%s' defined as wrong kind of tag",
			   ((name)->identifier.pointer));
		  }
		return ((tail)->list.value);
	      }
	  }
      if (thislevel_only && ! level->tag_transparent)
	return (tree) 0;
      if (current_class_type && level->level_chain == global_binding_level)
	{
	  tree context = current_class_type;
	  while (context)
	    {
	      switch ((*tree_code_type[(int) (((enum tree_code) (context)->common.code))]))
		{
		case 't':
		  {
		    tree these_tags = (((context)->type.lang_specific)->tags);
		    if ((((name)->identifier.pointer)[0] == '$' 				  && ((name)->identifier.pointer)[1] == '_'))
		      while (these_tags)
			{
			  if ((((((((these_tags)->list.value))->type.name))->decl.name))
			      == name)
			    return ((tail)->list.value);
			  these_tags = ((these_tags)->common.chain);
			}
		    else
		      while (these_tags)
			{
			  if (((these_tags)->list.purpose) == name)
			    {
			      if (((enum tree_code) (((these_tags)->list.value))->common.code) != form)
				{
				  error ("`%s' defined as wrong kind of tag in class scope",
					 ((name)->identifier.pointer));
				}
			      return ((tail)->list.value);
			    }
			  these_tags = ((these_tags)->common.chain);
			}
		    if (((context)->type.size) == (tree) 0)
		      goto no_context;
		    context = ((((context)->type.name))->decl.context);
		    break;
		  case 'd':
		    context = ((context)->decl.context);
		    break;
		  default:
		    my_friendly_abort (10);
		  }
		  continue;
		}
	    no_context:
	      break;
	    }
	}
    }
  return (tree) 0;
}
void
set_current_level_tags_transparency (tags_transparent)
     int tags_transparent;
{
  current_binding_level->tag_transparent = tags_transparent;
}
static tree
lookup_tag_reverse (type, name)
     tree type;
     tree name;
{
  register struct binding_level *level;
  for (level = current_binding_level; level; level = level->level_chain)
    {
      register tree tail;
      for (tail = level->tags; tail; tail = ((tail)->common.chain))
	{
	  if (((tail)->list.value) == type)
	    {
	      if (name)
		((tail)->list.purpose) = name;
	      return ((tail)->list.purpose);
	    }
	}
    }
  return (tree) 0;
}
tree
typedecl_for_tag (tag)
     tree tag;
{
  struct binding_level *b = current_binding_level;
  if (((enum tree_code) (((tag)->type.name))->common.code) == TYPE_DECL)
    return ((tag)->type.name);
  while (b)
    {
      tree decls = b->names;
      while (decls)
	{
	  if (((enum tree_code) (decls)->common.code) == TYPE_DECL && ((decls)->common.type) == tag)
	    break;
	  decls = ((decls)->common.chain);
	}
      if (decls)
	return decls;
      b = b->level_chain;
    }
  return (tree) 0;
}
static void
globalize_nested_type (type)
     tree type;
{
  tree t, prev = (tree) 0, d = ((type)->type.name);
  struct binding_level *b;
  my_friendly_assert (((enum tree_code) (d)->common.code) == TYPE_DECL, 144);
  if (  (((struct lang_identifier *)(((d)->decl.name)))->global_value) == d)
    return;
  if ((((((d)->decl.name))->common.type) ? 1 : 0))
    {
      if (value_member (type, global_binding_level->tags))
	return;
    }
  set_identifier_type_value (((d)->decl.arguments), (tree) 0);
  ((d)->decl.arguments) = ((d)->decl.name);
  ((d)->decl.context) = (tree) 0;
  if (class_binding_level)
    b = class_binding_level;
  else
    b = current_binding_level;
  while (b != global_binding_level)
    {
      prev = (tree) 0;
      if (b->parm_flag == 2)
	for (t = b->tags; t != (tree) 0; prev = t, t = ((t)->common.chain))
	  if (((t)->list.value) == type)
	    goto found;
      b = b->level_chain;
    }
  prev = (tree) 0;
  if (b->parm_flag == 2)
    for (t = b->tags; t != (tree) 0; prev = t, t = ((t)->common.chain))
      if (((t)->list.value) == type)
	goto foundglobal;
  return;
foundglobal:
  print_node_brief ((&_iob[2]), "Tried to globalize already-global type ",
		    type, 0);
  my_friendly_abort (11);
found:
  if (prev)
    ((prev)->common.chain) = ((t)->common.chain);
  else
    b->tags = ((t)->common.chain);
  set_identifier_type_value (((t)->list.purpose), ((t)->list.value));
  global_binding_level->tags
    = perm_tree_cons (((t)->list.purpose), ((t)->list.value),
		      global_binding_level->tags);
  if (current_class_type != (tree) 0)
    {
      for (t = (((current_class_type)->type.lang_specific)->tags), prev = (tree) 0;
	   t != (tree) 0;
	   prev = t, t = ((t)->common.chain))
	if (((t)->list.value) == type)
	  break;
      if (t != (tree) 0)
	{
	  if (prev)
	    ((prev)->common.chain) = ((t)->common.chain);
	  else
	    (((current_class_type)->type.lang_specific)->tags) = ((t)->common.chain);
	}
    }
  pushdecl_top_level (d);
}
static void
maybe_globalize_type (type)
     tree type;
{
  if ((((((enum tree_code) (type)->common.code) == RECORD_TYPE
	 || ((enum tree_code) (type)->common.code) == UNION_TYPE)
	&& ! (((type)->type.lang_specific)->type_flags.being_defined))
       || ((enum tree_code) (type)->common.code) == ENUMERAL_TYPE)
      && ((type)->type.size) == (tree) 0
      && !(((type)->type.name) != (tree) 0
	   && (((((type)->type.name))->decl.name)) != (tree) 0
	   && ! ((((((((type)->type.name))->decl.name)))->common.type) ? 1 : 0))
      )
    globalize_nested_type (type);
}
static tree
lookup_nested_type (type, context)
     tree type;
     tree context;
{
  if (context == (tree) 0)
    return (tree) 0;
  while (context)
    {
      switch (((enum tree_code) (context)->common.code))
	{
	case TYPE_DECL:
	  {
	    tree ctype = ((context)->common.type);
	    tree match = value_member (type, (((ctype)->type.lang_specific)->tags));
	    if (match)
	      return ((match)->list.value);
	    context = ((context)->decl.context);
	  }
	  break;
	case FUNCTION_DECL:
	  return (((((type)->type.name))->decl.name)) ? lookup_name ((((((type)->type.name))->decl.name)), 1) : (tree) 0;
	  break;
	default:
	  my_friendly_abort (12);
	}
    }
  return (tree) 0;
}
tree
lookup_name (name, prefer_type)
     tree name;
     int prefer_type;
{
  register tree val;
  if (current_binding_level != global_binding_level
      &&   (((struct lang_identifier *)(name))->local_value))
    val =   (((struct lang_identifier *)(name))->local_value);
  else if (current_class_type)
    {
      val =   (((struct lang_identifier *)(name))->class_value);
      if (val == (tree) 0
	  && ((current_class_type)->type.size) == 0
	  && (((current_class_type)->type.lang_specific)->type_flags.local_typedecls))
	{
	  val = lookup_field (current_class_type, name, 0, prefer_type==-1);
	  if (val == error_mark_node)
	    return val;
	  if (val && ((enum tree_code) (val)->common.code) != TYPE_DECL)
	    val = (tree) 0;
	}
      if (val == (tree) 0)
	val = lookup_nested_field (name);
      if (val == (tree) 0)
	val =   (((struct lang_identifier *)(name))->global_value);
    }
  else
    val =   (((struct lang_identifier *)(name))->global_value);
  if (val)
    {
      extern int looking_for_typename;
      if (((enum tree_code) (val)->common.code) == TYPE_DECL || looking_for_typename < 0)
	return val;
      if ((((name)->common.type) ? 1 : 0))
	{
	  register tree val_as_type = (((((name)->common.type)))->type.name);
	  if (val == val_as_type || prefer_type > 0
	      || looking_for_typename > 0)
	    return val_as_type;
	  if (prefer_type == 0)
	    return val;
	  return arbitrate_lookup (name, val, val_as_type);
	}
      if (((val)->common.type) == error_mark_node)
	return error_mark_node;
    }
  return val;
}
static tree
lookup_name_current_level (name)
     tree name;
{
  register tree t;
  if (current_binding_level == global_binding_level)
    return   (((struct lang_identifier *)(name))->global_value);
  if (  (((struct lang_identifier *)(name))->local_value) == 0)
    return 0;
  for (t = current_binding_level->names; t; t = ((t)->common.chain))
    if (((t)->decl.name) == name)
      break;
  return t;
}
static void sigsegv ()
{
  signal (11	, (void (*)())0);
  my_friendly_abort (0);
}
static tree *builtin_type_tdescs_arr;
static int builtin_type_tdescs_len, builtin_type_tdescs_max;
static void
record_builtin_type (rid_index, name, type)
     enum rid rid_index;
     char *name;
     tree type;
{
  tree rname = (tree) 0, tname = (tree) 0;
  tree tdecl;
  if ((int) rid_index < (int) RID_MAX)
    rname = ridpointers[(int) rid_index];
  if (name)
    tname = get_identifier (name);
  if (tname)
    {
      tdecl = pushdecl (build_decl (TYPE_DECL, tname, type));
      set_identifier_type_value (tname, (tree) 0);
      if ((int) rid_index < (int) RID_MAX)
	  (((struct lang_identifier *)(tname))->global_value) = tdecl;
    }
  if (rname != (tree) 0)
    {
      if (tname != (tree) 0)
	{
	  set_identifier_type_value (rname, (tree) 0);
	    (((struct lang_identifier *)(rname))->global_value) = tdecl;
	}
      else
	{
	  tdecl = pushdecl (build_decl (TYPE_DECL, rname, type));
	  set_identifier_type_value (rname, (tree) 0);
	}
    }
  if (flag_dossier)
    {
      if (builtin_type_tdescs_len+5 >= builtin_type_tdescs_max)
	{
	  builtin_type_tdescs_max *= 2;
	  builtin_type_tdescs_arr
	    = (tree *)xrealloc (builtin_type_tdescs_arr,
				builtin_type_tdescs_max * sizeof (tree));
	}
      builtin_type_tdescs_arr[builtin_type_tdescs_len++] = type;
      if (((enum tree_code) (type)->common.code) != POINTER_TYPE)
	{
	  builtin_type_tdescs_arr[builtin_type_tdescs_len++]
	    = build_pointer_type (type);
	  builtin_type_tdescs_arr[builtin_type_tdescs_len++]
	    = build_type_variant (((type)->type.pointer_to), 1, 0);
	}
      if (((enum tree_code) (type)->common.code) != VOID_TYPE)
	{
	  builtin_type_tdescs_arr[builtin_type_tdescs_len++]
	    = build_reference_type (type);
	  builtin_type_tdescs_arr[builtin_type_tdescs_len++]
	    = build_type_variant (((type)->type.reference_to), 1, 0);
	}
    }
}
static void
output_builtin_tdesc_entries ()
{
  extern struct obstack permanent_obstack;
  if (builtin_type_tdescs_arr == 0)
    return;
  push_obstacks (&permanent_obstack, &permanent_obstack);
  while (builtin_type_tdescs_len > 0)
    {
      tree type = builtin_type_tdescs_arr[--builtin_type_tdescs_len];
      tree tdesc = build_t_desc (type, 0);
      ((tdesc)->common.asm_written_flag) = 0;
      build_t_desc (type, 2);
    }
  free (builtin_type_tdescs_arr);
  builtin_type_tdescs_arr = 0;
  pop_obstacks ();
}
static void
push_overloaded_decl_1 (x)
     tree x;
{
  push_overloaded_decl (x, 0);
}
void
init_decl_processing ()
{
  register tree endlink, int_endlink, double_endlink, ptr_endlink;
  tree fields[20];
  tree traditional_ptr_type_node;
  tree memcpy_ftype;
  int wchar_type_size;
  lang_name_cplusplus = get_identifier ("C++");
  lang_name_c = get_identifier ("C");
  current_lang_name = lang_name_c;
  current_function_decl = (tree) 0;
  named_labels = (tree) 0;
  named_label_uses = (tree) 0;
  current_binding_level = (struct binding_level *) 0;
  free_binding_level = (struct binding_level *) 0;
  signal (11	, sigsegv);
  gcc_obstack_init (&decl_obstack);
  if (flag_dossier)
    {
      builtin_type_tdescs_max = 100;
      builtin_type_tdescs_arr = (tree *)xmalloc (100 * sizeof (tree));
    }
  error_mark_node = make_node (ERROR_MARK);
  ((error_mark_node)->common.permanent_flag) = 1;
  ((error_mark_node)->common.type) = error_mark_node;
  error_mark_list = build_tree_list (error_mark_node, error_mark_node);
  ((error_mark_list)->common.type) = error_mark_node;
  pushlevel (0);	
  global_binding_level = current_binding_level;
  this_identifier = get_identifier ("this");
  in_charge_identifier = get_identifier ("__in$chrg");
  integer_type_node = make_signed_type (32);
  record_builtin_type (RID_INT, 0, integer_type_node);
  char_type_node =
    (flag_signed_char
     ? make_signed_type (8)
     : make_unsigned_type (8));
  record_builtin_type (RID_CHAR, "char", char_type_node);
  long_integer_type_node = make_signed_type (64);
  record_builtin_type (RID_LONG, "long int", long_integer_type_node);
  unsigned_type_node = make_unsigned_type (32);
  record_builtin_type (RID_UNSIGNED, "unsigned int", unsigned_type_node);
  long_unsigned_type_node = make_unsigned_type (64);
  record_builtin_type (RID_MAX, "long unsigned int", long_unsigned_type_node);
  record_builtin_type (RID_MAX, "unsigned long", long_unsigned_type_node);
  if (flag_traditional)
    sizetype = long_integer_type_node;
  else
    sizetype
      = ((  (((struct lang_identifier *)(get_identifier ("long unsigned int")))->global_value))->common.type);
  ptrdiff_type_node
    = ((  (((struct lang_identifier *)(get_identifier ("long int")))->global_value))->common.type);
  ((((integer_type_node)->type.size))->common.type) = sizetype;
  ((((char_type_node)->type.size))->common.type) = sizetype;
  ((((unsigned_type_node)->type.size))->common.type) = sizetype;
  ((((long_unsigned_type_node)->type.size))->common.type) = sizetype;
  ((((long_integer_type_node)->type.size))->common.type) = sizetype;
  short_integer_type_node = make_signed_type ((8 * (((8 + 1) / 2) < ( 2) ? ((8 + 1) / 2) : ( 2))));
  record_builtin_type (RID_SHORT, "short int", short_integer_type_node);
  long_long_integer_type_node = make_signed_type (64);
  record_builtin_type (RID_MAX, "long long int", long_long_integer_type_node);
  short_unsigned_type_node = make_unsigned_type ((8 * (((8 + 1) / 2) < ( 2) ? ((8 + 1) / 2) : ( 2))));
  record_builtin_type (RID_MAX, "short unsigned int", short_unsigned_type_node);
  record_builtin_type (RID_MAX, "unsigned short", short_unsigned_type_node);
  long_long_unsigned_type_node = make_unsigned_type (64);
  record_builtin_type (RID_MAX, "long long unsigned int", long_long_unsigned_type_node);
  record_builtin_type (RID_MAX, "long long unsigned", long_long_unsigned_type_node);
  signed_char_type_node = make_signed_type (8);
  record_builtin_type (RID_MAX, "signed char", signed_char_type_node);
  unsigned_char_type_node = make_unsigned_type (8);
  record_builtin_type (RID_MAX, "unsigned char", unsigned_char_type_node);
  intQI_type_node = make_signed_type ((8 * mode_size[(int)(QImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, intQI_type_node));
  intHI_type_node = make_signed_type ((8 * mode_size[(int)(HImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, intHI_type_node));
  intSI_type_node = make_signed_type ((8 * mode_size[(int)(SImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, intSI_type_node));
  intDI_type_node = make_signed_type ((8 * mode_size[(int)(DImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, intDI_type_node));
  unsigned_intQI_type_node = make_unsigned_type ((8 * mode_size[(int)(QImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, unsigned_intQI_type_node));
  unsigned_intHI_type_node = make_unsigned_type ((8 * mode_size[(int)(HImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, unsigned_intHI_type_node));
  unsigned_intSI_type_node = make_unsigned_type ((8 * mode_size[(int)(SImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, unsigned_intSI_type_node));
  unsigned_intDI_type_node = make_unsigned_type ((8 * mode_size[(int)(DImode)]));
  pushdecl (build_decl (TYPE_DECL, (tree) 0, unsigned_intDI_type_node));
  float_type_node = make_node (REAL_TYPE);
  ((float_type_node)->type.precision) = 32;
  record_builtin_type (RID_FLOAT, 0, float_type_node);
  layout_type (float_type_node);
  double_type_node = make_node (REAL_TYPE);
  if (flag_short_double)
    ((double_type_node)->type.precision) = 32;
  else
    ((double_type_node)->type.precision) = 64;
  record_builtin_type (RID_DOUBLE, 0, double_type_node);
  layout_type (double_type_node);
  long_double_type_node = make_node (REAL_TYPE);
  ((long_double_type_node)->type.precision) = 64;
  record_builtin_type (RID_MAX, "long double", long_double_type_node);
  layout_type (long_double_type_node);
  integer_zero_node =   build_int_2_wide ((int) (0), (int) ( 0));
  ((integer_zero_node)->common.type) = integer_type_node;
  integer_one_node =   build_int_2_wide ((int) (1), (int) ( 0));
  ((integer_one_node)->common.type) = integer_type_node;
  integer_two_node =   build_int_2_wide ((int) (2), (int) ( 0));
  ((integer_two_node)->common.type) = integer_type_node;
  integer_three_node =   build_int_2_wide ((int) (3), (int) ( 0));
  ((integer_three_node)->common.type) = integer_type_node;
  empty_init_node = build_nt (CONSTRUCTOR, (tree) 0, (tree) 0);
  size_zero_node = size_int (0);
  size_one_node = size_int (1);
  void_type_node = make_node (VOID_TYPE);
  record_builtin_type (RID_VOID, 0, void_type_node);
  layout_type (void_type_node); 
  void_list_node = build_tree_list ((tree) 0, void_type_node);
  ((void_list_node)->common.unsigned_flag)  = 1;
  null_pointer_node =   build_int_2_wide ((int) (0), (int) ( 0));
  ((null_pointer_node)->common.type) = build_pointer_type (void_type_node);
  layout_type (((null_pointer_node)->common.type));
  void_zero_node =   build_int_2_wide ((int) (0), (int) ( 0));
  ((void_zero_node)->common.type) = void_type_node;
  string_type_node = build_pointer_type (char_type_node);
  const_string_type_node = build_pointer_type (build_type_variant (char_type_node, 1, 0));
  record_builtin_type (RID_MAX, 0, string_type_node);
  char_array_type_node
    = build_array_type (char_type_node, unsigned_char_type_node);
  int_array_type_node
    = build_array_type (integer_type_node, unsigned_char_type_node);
  class_star_type_node = build_pointer_type (make_lang_type (RECORD_TYPE));
  default_function_type
    = build_function_type (integer_type_node, (tree) 0);
  build_pointer_type (default_function_type);
  ptr_type_node = build_pointer_type (void_type_node);
  const_ptr_type_node = build_pointer_type (build_type_variant (void_type_node, 1, 0));
  record_builtin_type (RID_MAX, 0, ptr_type_node);
  endlink = void_list_node;
  int_endlink = tree_cons ((tree) 0, integer_type_node, endlink);
  double_endlink = tree_cons ((tree) 0, double_type_node, endlink);
  ptr_endlink = tree_cons ((tree) 0, ptr_type_node, endlink);
  double_ftype_double
    = build_function_type (double_type_node, double_endlink);
  double_ftype_double_double
    = build_function_type (double_type_node,
			   tree_cons ((tree) 0, double_type_node, double_endlink));
  int_ftype_int
    = build_function_type (integer_type_node, int_endlink);
  long_ftype_long
    = build_function_type (long_integer_type_node,
			   tree_cons ((tree) 0, long_integer_type_node, endlink));
  void_ftype_ptr_ptr_int
    = build_function_type (void_type_node,
			   tree_cons ((tree) 0, ptr_type_node,
				      tree_cons ((tree) 0, ptr_type_node,
						 int_endlink)));
  int_ftype_cptr_cptr_sizet
    = build_function_type (integer_type_node,
			   tree_cons ((tree) 0, const_ptr_type_node,
				      tree_cons ((tree) 0, const_ptr_type_node,
						 tree_cons ((tree) 0,
							    sizetype,
							    endlink))));
  void_ftype_ptr_int_int
    = build_function_type (void_type_node,
			   tree_cons ((tree) 0, ptr_type_node,
				      tree_cons ((tree) 0, integer_type_node,
						 int_endlink)));
  string_ftype_ptr_ptr		
    = build_function_type (string_type_node,
			   tree_cons ((tree) 0, string_type_node,
				      tree_cons ((tree) 0,
						 const_string_type_node,
						 endlink)));
  int_ftype_string_string	
    = build_function_type (integer_type_node,
			   tree_cons ((tree) 0, const_string_type_node,
				      tree_cons ((tree) 0,
						 const_string_type_node,
						 endlink)));
  sizet_ftype_string		
    = build_function_type (sizetype,
			   tree_cons ((tree) 0, const_string_type_node,
				      endlink));
  traditional_ptr_type_node
    = (flag_traditional ? string_type_node : ptr_type_node);
  memcpy_ftype	
    = build_function_type (traditional_ptr_type_node,
			   tree_cons ((tree) 0, ptr_type_node,
				      tree_cons ((tree) 0, const_ptr_type_node,
						 tree_cons ((tree) 0,
							    sizetype,
							    endlink))));
  vtbl_type_node
    = build_array_type (ptr_type_node, (tree) 0);
  layout_type (vtbl_type_node);
  vtbl_type_node = build_type_variant (vtbl_type_node, 1, 0);
  record_builtin_type (RID_MAX, 0, vtbl_type_node);
    define_function ("__builtin_constant_p",  int_ftype_int, 
		    BUILT_IN_CONSTANT_P, (void (*)())pushdecl,  0);
    define_function ("__builtin_alloca", 
		    build_function_type (ptr_type_node,
					 tree_cons ((tree) 0,
						    sizetype,
						    endlink)), 
		    BUILT_IN_ALLOCA, (void (*)())pushdecl,  "alloca");
    define_function ("__builtin_abs",  int_ftype_int,  BUILT_IN_ABS, (void (*)())pushdecl,  0);
    define_function ("__builtin_fabs",  double_ftype_double,  BUILT_IN_FABS, (void (*)())pushdecl,  0);
    define_function ("__builtin_labs",  long_ftype_long,  BUILT_IN_LABS, (void (*)())pushdecl,  0);
    define_function ("__builtin_ffs",  int_ftype_int,  BUILT_IN_FFS, (void (*)())pushdecl,  0);
    define_function ("__builtin_fsqrt",  double_ftype_double,  BUILT_IN_FSQRT, (void (*)())pushdecl,  0);
    define_function ("__builtin_sin",  double_ftype_double,  BUILT_IN_SIN, (void (*)())pushdecl,  0);
    define_function ("__builtin_cos",  double_ftype_double,  BUILT_IN_COS, (void (*)())pushdecl,  0);
    define_function ("__builtin_saveregs", 
		    build_function_type (ptr_type_node, (tree) 0), 
		    BUILT_IN_SAVEREGS, (void (*)())pushdecl,  0);
    define_function ("__builtin_classify_type",  default_function_type, 
		    BUILT_IN_CLASSIFY_TYPE, (void (*)())pushdecl,  0);
    define_function ("__builtin_next_arg", 
		    build_function_type (ptr_type_node, endlink), 
		    BUILT_IN_NEXT_ARG, (void (*)())pushdecl,  0);
    define_function ("__builtin_memcpy",  memcpy_ftype, 
		    BUILT_IN_MEMCPY, (void (*)())pushdecl,  "memcpy");
    define_function ("__builtin_memcmp",  int_ftype_cptr_cptr_sizet, 
		    BUILT_IN_MEMCMP, (void (*)())pushdecl,  "memcmp");
    define_function ("__builtin_strcmp",  int_ftype_string_string, 
		    BUILT_IN_STRCMP, (void (*)())pushdecl,  "strcmp");
    define_function ("__builtin_strcpy",  string_ftype_ptr_ptr, 
		    BUILT_IN_STRCPY, (void (*)())pushdecl,  "strcpy");
    define_function ("__builtin_strlen",  sizet_ftype_string, 
		    BUILT_IN_STRLEN, (void (*)())pushdecl,  "strlen");
    define_function ("memcpy",  memcpy_ftype,  BUILT_IN_MEMCPY, (void (*)())pushdecl,  0);
    define_function ("memcmp",  int_ftype_cptr_cptr_sizet,  BUILT_IN_MEMCMP, (void (*)())pushdecl,  0);
    define_function ("strcmp",  int_ftype_string_string,  BUILT_IN_STRCMP, (void (*)())pushdecl,  0);
    define_function ("strcpy",  string_ftype_ptr_ptr,  BUILT_IN_STRCPY, (void (*)())pushdecl,  0);
    define_function ("strlen",  sizet_ftype_string,  BUILT_IN_STRLEN, (void (*)())pushdecl,  0);
  unknown_type_node = make_node (LANG_TYPE);
  pushdecl (build_decl (TYPE_DECL,
			get_identifier ("unknown type"),
			unknown_type_node));
  ((unknown_type_node)->type.size) = ((void_type_node)->type.size);
  ((unknown_type_node)->type.align) = 1;
  ((unknown_type_node)->type.mode) = ((void_type_node)->type.mode);
  ((unknown_type_node)->common.type) = unknown_type_node;
  ((unknown_type_node)->type.pointer_to) = unknown_type_node;
  ((unknown_type_node)->type.reference_to) = unknown_type_node;
  wchar_type_node
    = ((  (((struct lang_identifier *)(get_identifier ("short unsigned int")))->global_value))->common.type);
  wchar_type_size = ((wchar_type_node)->type.precision);
  signed_wchar_type_node = make_signed_type (wchar_type_size);
  unsigned_wchar_type_node = make_unsigned_type (wchar_type_size);
  wchar_type_node
    = ((wchar_type_node)->common.unsigned_flag)
      ? unsigned_wchar_type_node
      : signed_wchar_type_node;
  record_builtin_type (RID_WCHAR, "__wchar_t", wchar_type_node);
  wchar_array_type_node
    = build_array_type (wchar_type_node, unsigned_char_type_node);
  if (flag_gc)
    {
        define_function ("__gc_main",  default_function_type,  NOT_BUILT_IN, (void (*)())pushdecl,  0);
      pushdecl (lookup_name (get_identifier ("__gc_main"), 0));
    }
  vtable_entry_type = make_lang_type (RECORD_TYPE);
  fields[0] = build_lang_field_decl (FIELD_DECL, get_identifier ("delta"), short_integer_type_node);
  fields[1] = build_lang_field_decl (FIELD_DECL, get_identifier ("index"), short_integer_type_node);
  fields[2] = build_lang_field_decl (FIELD_DECL, get_identifier ("pfn"), ptr_type_node);
  finish_builtin_type (vtable_entry_type, "$vtbl_ptr_type", fields, 2,
		       double_type_node);
  fields[3] = copy_node (fields[2]);
  ((fields[3])->common.type) = short_integer_type_node;
  ((fields[3])->decl.name) = get_identifier ("delta2");
  ((fields[3])->decl.mode) = ((short_integer_type_node)->type.mode);
  ((fields[3])->decl.size) = ((short_integer_type_node)->type.size);
  ((fields[3])->common.unsigned_flag) = 0;
  ((fields[2])->common.chain) = fields[3];
  vtable_entry_type = build_type_variant (vtable_entry_type, 1, 0);
  record_builtin_type (RID_MAX, "$vtbl_ptr_type", vtable_entry_type);
  if (flag_dossier)
    {
      __t_desc_type_node = make_lang_type (RECORD_TYPE);
      __i_desc_type_node = make_lang_type (RECORD_TYPE);
      __m_desc_type_node = make_lang_type (RECORD_TYPE);
      __t_desc_array_type = build_array_type (((__t_desc_type_node)->type.pointer_to), (tree) 0);
      __i_desc_array_type = build_array_type (((__i_desc_type_node)->type.pointer_to), (tree) 0);
      __m_desc_array_type = build_array_type (((__m_desc_type_node)->type.pointer_to), (tree) 0);
      fields[0] = build_lang_field_decl (FIELD_DECL, get_identifier ("name"),
					 string_type_node);
      fields[1] = build_lang_field_decl (FIELD_DECL, get_identifier ("size"),
					 unsigned_type_node);
      fields[2] = build_lang_field_decl (FIELD_DECL, get_identifier ("bits"),
					 unsigned_type_node);
      fields[3] = build_lang_field_decl (FIELD_DECL, get_identifier ("points_to"),
					 ((__t_desc_type_node)->type.pointer_to));
      fields[4] = build_lang_field_decl (FIELD_DECL,
					 get_identifier ("ivars_count"),
					 integer_type_node);
      fields[5] = build_lang_field_decl (FIELD_DECL,
					 get_identifier ("meths_count"),
					 integer_type_node);
      fields[6] = build_lang_field_decl (FIELD_DECL, get_identifier ("ivars"),
					 build_pointer_type (__i_desc_array_type));
      fields[7] = build_lang_field_decl (FIELD_DECL, get_identifier ("meths"),
					 build_pointer_type (__m_desc_array_type));
      fields[8] = build_lang_field_decl (FIELD_DECL, get_identifier ("parents"),
					 build_pointer_type (__t_desc_array_type));
      fields[9] = build_lang_field_decl (FIELD_DECL, get_identifier ("vbases"),
					 build_pointer_type (__t_desc_array_type));
      fields[10] = build_lang_field_decl (FIELD_DECL, get_identifier ("offsets"),
					 build_pointer_type (integer_type_node));
      finish_builtin_type (__t_desc_type_node, "__t_desc", fields, 10, integer_type_node);
      fields[0] = build_lang_field_decl (FIELD_DECL, get_identifier ("name"),
					 string_type_node);
      fields[1] = build_lang_field_decl (FIELD_DECL, get_identifier ("offset"),
					 integer_type_node);
      fields[2] = build_lang_field_decl (FIELD_DECL, get_identifier ("type"),
					 ((__t_desc_type_node)->type.pointer_to));
      finish_builtin_type (__i_desc_type_node, "__i_desc", fields, 2, integer_type_node);
      fields[0] = build_lang_field_decl (FIELD_DECL, get_identifier ("name"),
					 string_type_node);
      fields[1] = build_lang_field_decl (FIELD_DECL, get_identifier ("vindex"),
					 integer_type_node);
      fields[2] = build_lang_field_decl (FIELD_DECL, get_identifier ("vcontext"),
					 ((__t_desc_type_node)->type.pointer_to));
      fields[3] = build_lang_field_decl (FIELD_DECL, get_identifier ("return_type"),
					 ((__t_desc_type_node)->type.pointer_to));
      fields[4] = build_lang_field_decl (FIELD_DECL, get_identifier ("address"),
					 build_pointer_type (default_function_type));
      fields[5] = build_lang_field_decl (FIELD_DECL, get_identifier ("parm_count"),
					 short_integer_type_node);
      fields[6] = build_lang_field_decl (FIELD_DECL, get_identifier ("required_parms"),
					 short_integer_type_node);
      fields[7] = build_lang_field_decl (FIELD_DECL, get_identifier ("parm_types"),
					 build_pointer_type (build_array_type (((__t_desc_type_node)->type.pointer_to), (tree) 0)));
      finish_builtin_type (__m_desc_type_node, "__m_desc", fields, 7, integer_type_node);
    }
  current_lang_name = lang_name_cplusplus;
  if (flag_dossier)
    {
      int i = builtin_type_tdescs_len;
      while (i > 0)
	{
	  tree tdesc = build_t_desc (builtin_type_tdescs_arr[--i], 0);
	  ((tdesc)->common.asm_written_flag) = 1;
	  ((((tdesc)->exp.operands[ 0]))->common.public_flag) = 1;
	}
    }
    do {					    tree __name = ansi_opname[(int) NEW_EXPR];		    tree __type = 
		 build_function_type (ptr_type_node,
				      tree_cons ((tree) 0, sizetype,
						 void_list_node));			    define_function (((__name)->identifier.pointer), __type, 
		 NOT_BUILT_IN,			     (void (*)())push_overloaded_decl_1,			     ((build_decl_overload (__name, ((__type)->type.values), 0))->identifier.pointer));  } while (0);
    do {					    tree __name = ansi_opname[(int) DELETE_EXPR];		    tree __type = 
		 build_function_type (void_type_node,
				      tree_cons ((tree) 0, ptr_type_node,
						 void_list_node));			    define_function (((__name)->identifier.pointer), __type, 
		 NOT_BUILT_IN,			     (void (*)())push_overloaded_decl_1,			     ((build_decl_overload (__name, ((__type)->type.values), 0))->identifier.pointer));  } while (0);
  abort_fndecl
    = define_function ("abort",
		       build_function_type (void_type_node, void_list_node),
		       NOT_BUILT_IN, 0, 0);
  unhandled_exception_fndecl
    = define_function ("__unhandled_exception",
		       build_function_type (void_type_node, (tree) 0),
		       NOT_BUILT_IN, 0, 0);
  init_class_processing ();
  init_init_processing ();
  init_search_processing ();
  if (flag_handle_exceptions)
    {
      if (flag_handle_exceptions == 2)
	flag_this_is_variable = 2;
      init_exception_processing ();
    }
  if (flag_gc)
    init_gc_processing ();
  if (flag_no_inline)
    flag_inline_functions = 0, flag_default_inline = 0;
  if (flag_cadillac)
    init_cadillac ();
  declare_function_name ();
  warn_return_type = 1;
}
tree
define_function (name, type, function_code, pfn, library_name)
     char *name;
     tree type;
     enum built_in_function function_code;
     void (*pfn)();
     char *library_name;
{
  tree decl = build_lang_decl (FUNCTION_DECL, get_identifier (name), type);
  ((decl)->decl.external_flag) = 1;
  ((decl)->common.public_flag) = 1;
  if (pfn) (*pfn) (decl);
  if (library_name)
    ((decl)->decl.assembler_name) = get_identifier (library_name);
  make_function_rtl (decl);
  if (function_code != NOT_BUILT_IN)
    {
      ((decl)->decl.bit_field_flag) = 1;
       ((decl)->decl.frame_size = (int) ( function_code));
    }
  return decl;
}
void
shadow_tag (declspecs)
     tree declspecs;
{
  int found_tag = 0;
  int warned = 0;
  register tree link;
  register enum tree_code code, ok_code = ERROR_MARK;
  register tree t = (tree) 0;
  for (link = declspecs; link; link = ((link)->common.chain))
    {
      register tree value = ((link)->list.value);
      code = ((enum tree_code) (value)->common.code);
      if (  (code == RECORD_TYPE || code == UNION_TYPE) || code == ENUMERAL_TYPE)
	{
	  register tree name = ((value)->type.name);
	  if (name == (tree) 0)
	    name = lookup_tag_reverse (value, (tree) 0);
	  if (name && ((enum tree_code) (name)->common.code) == TYPE_DECL)
	    name = ((name)->decl.name);
	  if (class_binding_level)
	    t = lookup_tag (code, name, class_binding_level, 1);
	  else
	    t = lookup_tag (code, name, current_binding_level, 1);
	  if (t == 0)
	    {
	      push_obstacks (&permanent_obstack, &permanent_obstack);
	      if (  (code == RECORD_TYPE || code == UNION_TYPE))
		t = make_lang_type (code);
	      else
		t = make_node (code);
	      pushtag (name, t);
	      pop_obstacks ();
	      ok_code = code;
	      break;
	    }
	  else if (name != 0 || code == ENUMERAL_TYPE)
	    ok_code = code;
	  if (ok_code != ERROR_MARK)
	    found_tag++;
	  else
	    {
	      if (!warned)
		warning ("useless keyword or type name in declaration");
	      warned = 1;
	    }
	}
    }
  if (ok_code == UNION_TYPE
      && t != (tree) 0
      && ((((enum tree_code) (((t)->type.name))->common.code) == IDENTIFIER_NODE
	   && (((((t)->type.name))->identifier.pointer)[0] == '$' 				  && ((((t)->type.name))->identifier.pointer)[1] == '_'))
	  || (((enum tree_code) (((t)->type.name))->common.code) == TYPE_DECL
	      && ((((((((t)->type.name))->decl.name)))->identifier.pointer)[0] == '$' 				  && (((((((t)->type.name))->decl.name)))->identifier.pointer)[1] == '_'))))
    {
      if (((t)->type.values))
	{
	  tree decl = grokdeclarator ((tree) 0, declspecs, NORMAL, 0, (tree) 0);
	  finish_anon_union (decl);
	}
      else
	error ("anonymous union cannot have a function member");
    }
  else if (ok_code == RECORD_TYPE
	   && found_tag == 1
	   && ((t)->type.lang_specific)
	   && (((t)->type.lang_specific)->type_flags.declared_exception))
    {
      if (((t)->type.size))
	error_with_aggr_type (t, "redeclaration of exception `%s'");
      else
	{
	  tree ename, decl;
	  push_obstacks (&permanent_obstack, &permanent_obstack);
	  pushclass (t, 0);
	  finish_exception (t, (tree) 0);
	  ename = ((t)->type.name);
	  if (((enum tree_code) (ename)->common.code) == TYPE_DECL)
	    ename = ((ename)->decl.name);
	  decl = build_lang_field_decl (VAR_DECL, ename, t);
	  finish_exception_decl (current_class_name, decl);
	  end_exception_decls ();
	  pop_obstacks ();
	}
    }
  else if (!warned && found_tag > 1)
    warning ("multiple types in one declaration");
}
tree
groktypename (typename)
     tree typename;
{
  if (((enum tree_code) (typename)->common.code) != TREE_LIST)
    return typename;
  return grokdeclarator (((typename)->list.value),
			 ((typename)->list.purpose),
			 TYPENAME, 0, (tree) 0);
}
int debug_temp_inits = 1;
tree
start_decl (declarator, declspecs, initialized, raises)
     tree declspecs, declarator;
     int initialized;
     tree raises;
{
  register tree decl;
  register tree type, tem;
  tree context;
  extern int have_extern_spec;
  extern int used_extern_spec;
  int init_written = initialized;
  if (have_extern_spec && !used_extern_spec)
    {
      declspecs = decl_tree_cons ((tree) 0, get_identifier ("extern"), declspecs);
      used_extern_spec = 1;
    }
  decl = grokdeclarator (declarator, declspecs, NORMAL, initialized, raises);
  if (decl == (tree) 0 || decl == void_type_node)
    return (tree) 0;
  type = ((decl)->common.type);
  if (((decl)->common.static_flag)
      && (((type)->type.lang_flag_4))
      && ((decl)->common.permanent_flag) == 0)
    {
      push_obstacks (&permanent_obstack, &permanent_obstack);
      decl = copy_node (decl);
      if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
	{
	  tree itype = ((type)->type.values);
	  if (itype && ! ((itype)->common.permanent_flag))
	    {
	      itype = build_index_type (copy_to_permanent (((itype)->type.maxval)));
	      type = build_cplus_array_type (((type)->common.type), itype);
	      ((decl)->common.type) = type;
	    }
	}
      pop_obstacks ();
    }
  if (((enum tree_code) (type)->common.code) == RECORD_TYPE
      && (((type)->type.lang_specific)->type_flags.declared_exception))
    return decl;
  push_obstacks_nochange ();
  context
    = (((enum tree_code) (decl)->common.code) == FUNCTION_DECL && ((decl)->decl.virtual_flag))
      ? (((decl)->decl.lang_specific)->decl_flags.context)
      : ((decl)->decl.context);
  if (processing_template_decl)
    {
      tree d;
      if (((enum tree_code) (decl)->common.code) == FUNCTION_DECL)
        {
          tree args;
          args = copy_to_permanent (last_function_parms);
          if (((enum tree_code) (((decl)->common.type))->common.code) == METHOD_TYPE)
            {
	      tree t = ((decl)->common.type);
              tree decl;
              t = ((t)->type.maxval); 
	      if (((enum tree_code) (t)->common.code) != UNINSTANTIATED_P_TYPE)
		{
		  t = build_pointer_type (t); 
		  t = build_type_variant (t, flag_this_is_variable <= 0,
					  0); 
		  t = build (PARM_DECL, t, this_identifier);
		  ((t)->common.chain) = args;
		  args = t;
		}
	    }
          ((decl)->decl.arguments) = args;
        }
      d = build_lang_decl (TEMPLATE_DECL, ((decl)->decl.name), ((decl)->common.type));
      ((d)->common.public_flag) = ((decl)->common.public_flag) = 0;
      ((d)->common.static_flag) = ((decl)->common.static_flag);
      ((d)->decl.external_flag) = (((decl)->decl.external_flag)
			   && !(context && !(((decl)->decl.lang_flag_2))));
      ((d)->decl.result) = decl;
      (((d)->decl.lang_flag_4)) = 1;
      decl = d;
    }
  if (context && ((context)->type.size) != (tree) 0)
    {
      if ((((decl)->decl.lang_flag_2)) == 0)
	((decl)->decl.external_flag) = 0;
      if (((decl)->decl.lang_specific))
	(((decl)->decl.lang_flag_3)) = 0;
      pushclass (context, 2);
    }
  if ((((type)->type.lang_flag_4))
      && current_binding_level->more_cleanups_ok == 0)
    pushlevel_temporary (1);
  if (initialized)
    switch (((enum tree_code) (decl)->common.code))
      {
      case TYPE_DECL:
	if (pedantic || list_length (declspecs) > 1)
	  {
	    error ("typedef `%s' is initialized",
		   ((((decl)->decl.name))->identifier.pointer));
	    initialized = 0;
	  }
	break;
      case FUNCTION_DECL:
	error ("function `%s' is initialized like a variable",
	       ((((decl)->decl.name))->identifier.pointer));
	initialized = 0;
	break;
      default:
	if (((type)->type.size) != 0)
	  ;                     
	else if (((enum tree_code) (type)->common.code) != ARRAY_TYPE)
	  {
	    error ("variable `%s' has initializer but incomplete type",
		   ((((decl)->decl.name))->identifier.pointer));
	    initialized = 0;
	  }
	else if (((((type)->common.type))->type.size) == 0)
	  {
	    error ("elements of array `%s' have incomplete type",
		   ((((decl)->decl.name))->identifier.pointer));
	    initialized = 0;
	  }
      }
  if (!initialized
      && ((enum tree_code) (decl)->common.code) != TYPE_DECL
      && ((enum tree_code) (decl)->common.code) != TEMPLATE_DECL
      && (((type)->type.lang_flag_5)) && ! ((decl)->decl.external_flag))
    {
      if (((type)->type.size) == 0)
	{
	  error ("aggregate `%s' has incomplete type and cannot be initialized",
		 ((((decl)->decl.name))->identifier.pointer));
	  ((decl)->common.type) = error_mark_node;
	  type = error_mark_node;
	}
      else
	{
	  initialized = (((type)->type.lang_flag_3));
	}
    }
  if (initialized)
    {
      if (current_binding_level != global_binding_level
	  && ((decl)->decl.external_flag))
	warning ("declaration of `%s' has `extern' and is initialized",
		 ((((decl)->decl.name))->identifier.pointer));
      ((decl)->decl.external_flag) = 0;
      if (current_binding_level == global_binding_level)
	((decl)->common.static_flag) = 1;
      ((decl)->decl.initial) = error_mark_node;
    }
  if ((((enum tree_code) (decl)->common.code) != PARM_DECL && ((decl)->decl.context) != (tree) 0)
      || (((enum tree_code) (decl)->common.code) == TEMPLATE_DECL && !global_bindings_p ())
      || ((enum tree_code) (type)->common.code) == LANG_TYPE)
    tem = decl;
  else
    {
      tem = pushdecl (decl);
      if (((enum tree_code) (tem)->common.code) == TREE_LIST)
	{
	  tree tem2 = value_member (decl, tem);
	  if (tem2 != (tree) 0)
	    tem = ((tem2)->list.value);
	  else
	    {
	      while (tem && ! decls_match (decl, ((tem)->list.value)))
		tem = ((tem)->common.chain);
	      if (tem == (tree) 0)
		tem = decl;
	      else
		tem = ((tem)->list.value);
	    }
	}
    }
  if (((enum tree_code) (decl)->common.code) == FUNCTION_DECL && (((decl)->decl.lang_flag_4)))
    tem = push_overloaded_decl (tem, 1);
  else if (((enum tree_code) (decl)->common.code) == TEMPLATE_DECL)
    {
      tree result = ((decl)->decl.result);
      if (((result)->decl.context) != (tree) 0)
	{
          tree type;
          type = ((result)->decl.context);
          my_friendly_assert (((enum tree_code) (type)->common.code) == UNINSTANTIATED_P_TYPE, 145);
          if ( 1)
            {
	      return tem;
	    }
          my_friendly_abort (13);
        }
      else if (((enum tree_code) (result)->common.code) == FUNCTION_DECL)
        tem = push_overloaded_decl (tem, 0);
      else if (((enum tree_code) (result)->common.code) == VAR_DECL
	       || ((enum tree_code) (result)->common.code) == TYPE_DECL)
	{
	  sorry ("non-function templates not yet supported");
	  return (tree) 0;
	}
      else
	my_friendly_abort (14);
    }
  if (init_written
      && ! (((enum tree_code) (tem)->common.code) == PARM_DECL
	    || (((tem)->common.readonly_flag)
		&& (((enum tree_code) (tem)->common.code) == VAR_DECL
		    || ((enum tree_code) (tem)->common.code) == FIELD_DECL))))
    {
      if (current_binding_level == global_binding_level && debug_temp_inits)
	{
	  if ((((type)->type.lang_flag_3)) || ((enum tree_code) (type)->common.code) == REFERENCE_TYPE)
	    ;
	  else
	    temporary_allocation ();
	}
    }
  if (flag_cadillac)
    cadillac_start_decl (tem);
  return tem;
}
static void
make_temporary_for_reference (decl, ctor_call, init, cleanupp)
     tree decl, ctor_call, init;
     tree *cleanupp;
{
  tree type = ((decl)->common.type);
  tree target_type = ((type)->common.type);
  tree tmp, tmp_addr;
  if (ctor_call)
    {
      tmp_addr = ((((ctor_call)->exp.operands[ 1]))->list.value);
      if (((enum tree_code) (tmp_addr)->common.code) == NOP_EXPR)
	tmp_addr = ((tmp_addr)->exp.operands[ 0]);
      my_friendly_assert (((enum tree_code) (tmp_addr)->common.code) == ADDR_EXPR, 146);
      tmp = ((tmp_addr)->exp.operands[ 0]);
    }
  else
    {
      tmp = get_temp_name (target_type,
			   current_binding_level == global_binding_level);
      tmp_addr = build_unary_op (ADDR_EXPR, tmp, 0);
    }
  ((tmp_addr)->common.type) = build_pointer_type (target_type);
  ((decl)->decl.initial) = convert (((target_type)->type.pointer_to), tmp_addr);
  ((((decl)->decl.initial))->common.type) = type;
  if ((((target_type)->type.lang_flag_3)))
    {
      if (current_binding_level == global_binding_level)
	{
	  make_decl_rtl (tmp, 0, 1);
	  static_aggregates = perm_tree_cons (init, tmp, static_aggregates);
	}
      else
	{
	  if (ctor_call != (tree) 0)
	    init = ctor_call;
	  else
	    init = build_method_call (tmp, constructor_name (target_type),
				      build_tree_list ((tree) 0, init),
				      (tree) 0, (3));
	  ((decl)->decl.initial) = build (COMPOUND_EXPR, type, init,
				       ((decl)->decl.initial));
	  *cleanupp = maybe_build_cleanup (tmp);
	}
    }
  else
    {
      ((tmp)->decl.initial) = init;
      ((tmp)->common.static_flag) = current_binding_level == global_binding_level;
      finish_decl (tmp, init, 0, 0);
    }
  if (((tmp)->common.static_flag))
    preserve_initializer ();
}
static void
grok_reference_init (decl, type, init, cleanupp)
     tree decl, type, init;
     tree *cleanupp;
{
  char *errstr = 0;
  int is_reference;
  tree tmp;
  tree this_ptr_type, actual_init;
  if (init == (tree) 0)
    {
      if (((decl)->decl.lang_specific) == 0 || (((decl)->decl.lang_flag_3)) == 0)
	{
	  error ("variable declared as reference not initialized");
	  if (((enum tree_code) (decl)->common.code) == VAR_DECL)
	    ((decl)->decl.arguments= error_mark_node);
	}
      return;
    }
  if (((enum tree_code) (init)->common.code) == TREE_LIST)
    init = build_compound_expr (init);
  is_reference = ((enum tree_code) (((init)->common.type))->common.code) == REFERENCE_TYPE;
  tmp = is_reference ? convert_from_reference (init) : init;
  if (is_reference)
    {
      if (! comptypes (((((type)->common.type))->type.main_variant),
		       ((((tmp)->common.type))->type.main_variant), 0))
	errstr = "initialization of `%s' from dissimilar reference type";
      else if (((((type)->common.type))->common.readonly_flag)
	       >= ((((((init)->common.type))->common.type))->common.readonly_flag))
	{
	  is_reference = 0;
	  init = tmp;
	}
    }
  else
    {
      if (((enum tree_code) (((type)->common.type))->common.code) != ARRAY_TYPE
	  && ((enum tree_code) (((init)->common.type))->common.code) == ARRAY_TYPE)
	{
	  init = default_conversion (init);
	}
      if (((enum tree_code) (((type)->common.type))->common.code) == ((enum tree_code) (((init)->common.type))->common.code))
	{
	  if (comptypes (((((type)->common.type))->type.main_variant),
			 ((((init)->common.type))->type.main_variant), 0))
	    {
	      if (((((type)->common.type))->common.volatile_flag) && ((init)->common.readonly_flag))
		errstr = "cannot initialize a reference to a volatile T with a const T";
	      else if (((((type)->common.type))->common.readonly_flag) && ((init)->common.volatile_flag))
		errstr = "cannot initialize a reference to a const T with a volatile T";
	      else if (!((((type)->common.type))->common.volatile_flag)
		       && !((((type)->common.type))->common.readonly_flag))
		{
		  if (((init)->common.readonly_flag))
		    errstr = "cannot initialize a reference to T with a const T";
		  else if (((init)->common.volatile_flag))
		    errstr = "cannot initialize a reference to T with a volatile T";
		}
	    }
	  else
	    init = convert (((type)->common.type), init);
	}
      else if (init != error_mark_node
	       && ! comptypes (((((type)->common.type))->type.main_variant),
			       ((((init)->common.type))->type.main_variant), 0))
	errstr = "invalid type conversion for reference";
    }
  if (errstr)
    {
      if ((((((tmp)->common.type))->type.lang_flag_5)))
	{
	  tmp = build_type_conversion (CONVERT_EXPR, type, init, 0);
	  if (tmp != (tree) 0)
	    {
	      init = tmp;
	      if (tmp == error_mark_node)
		errstr = "ambiguous pointer conversion";
	      else
		errstr = 0;
	      is_reference = 1;
	    }
	  else
	    {
	      tmp = build_type_conversion (CONVERT_EXPR, ((type)->common.type), init, 0);
	      if (tmp != (tree) 0)
		{
		  init = tmp;
		  if (tmp == error_mark_node)
		    errstr = "ambiguous pointer conversion";
		  else
		    errstr = 0;
		  is_reference = 0;
		}
	    }
	}
      else if ((((((type)->common.type))->type.lang_flag_5))
	       && (((((type)->common.type))->type.lang_flag_1)))
	{
	  tmp = get_temp_name (((type)->common.type),
			       current_binding_level == global_binding_level);
	  tmp = build_method_call (tmp, constructor_name (((type)->common.type)),
				   build_tree_list ((tree) 0, init),
				   (tree) 0, (3));
	  if (tmp == (tree) 0 || tmp == error_mark_node)
	    {
	      if (((enum tree_code) (decl)->common.code) == VAR_DECL)
		((decl)->decl.arguments= error_mark_node);
	      error_with_decl (decl, "constructor failed to build reference initializer");
	      return;
	    }
	  make_temporary_for_reference (decl, tmp, init, cleanupp);
	  goto done;
	}
    }
  if (errstr)
    {
      error_with_decl (decl, errstr);
      if (((enum tree_code) (decl)->common.code) == VAR_DECL)
	((decl)->decl.arguments= error_mark_node);
      return;
    }
  this_ptr_type = build_pointer_type (((type)->common.type));
  if (is_reference)
    {
      if (((init)->common.side_effects_flag))
	((decl)->decl.initial) = save_expr (init);
      else
	((decl)->decl.initial) = init;
    }
  else if (lvalue_p (init))
    {
      tmp = build_unary_op (ADDR_EXPR, init, 0);
      if (((enum tree_code) (tmp)->common.code) == ADDR_EXPR
	  && ((enum tree_code) (((tmp)->exp.operands[ 0]))->common.code) == WITH_CLEANUP_EXPR)
	{
	  *cleanupp = ((((tmp)->exp.operands[ 0]))->exp.operands[ 2]);
	  ((((tmp)->exp.operands[ 0]))->exp.operands[ 2]) = error_mark_node;
	}
      if ((((((this_ptr_type)->common.type))->type.lang_flag_5)))
	((decl)->decl.initial) = convert_pointer_to (((this_ptr_type)->common.type), tmp);
      else
	((decl)->decl.initial) = convert (this_ptr_type, tmp);
      ((decl)->decl.initial) = save_expr (((decl)->decl.initial));
      if (((decl)->decl.initial) == current_class_decl)
	((decl)->decl.initial) = copy_node (current_class_decl);
      ((((decl)->decl.initial))->common.type) = type;
    }
  else if ((actual_init = unary_complex_lvalue (ADDR_EXPR, init)))
    {
      if (((enum tree_code) (actual_init)->common.code) == ADDR_EXPR
	  && ((enum tree_code) (((actual_init)->exp.operands[ 0]))->common.code) == TARGET_EXPR)
	actual_init = save_expr (actual_init);
      ((decl)->decl.initial) = convert_pointer_to (((this_ptr_type)->common.type), actual_init);
      ((decl)->decl.initial) = save_expr (((decl)->decl.initial));
      ((((decl)->decl.initial))->common.type) = type;
    }
  else if (((((type)->common.type))->common.readonly_flag))
    make_temporary_for_reference (decl, (tree) 0, init, cleanupp);
  else
    {
      error_with_decl (decl, "type mismatch in initialization of `%s' (use `const')");
      ((decl)->decl.initial) = error_mark_node;
    }
 done:
  if (((((type)->common.type))->type.size))
    {
      init = convert_from_reference (decl);
      if (((decl)->common.permanent_flag))
	init = copy_to_permanent (init);
      ((decl)->decl.arguments= init);
    }
  if (((decl)->common.static_flag) && ! ((((decl)->decl.initial))->common.constant_flag))
    {
      expand_static_init (decl, ((decl)->decl.initial));
      ((decl)->decl.initial) = 0;
    }
}
void
finish_decl (decl, init, asmspec_tree, need_pop)
     tree decl, init;
     tree asmspec_tree;
     int need_pop;
{
  register tree type;
  tree cleanup = (tree) 0, ttype;
  int was_incomplete;
  int temporary = allocation_temporary_p ();
  char *asmspec = 0;
  int was_readonly = 0;
  if (! decl)
    {
      if (init)
	error ("assignment (not initialization) in declaration");
      return;
    }
  if (asmspec_tree)
    {
      asmspec = ((asmspec_tree)->string.pointer);
      ((decl)->decl.rtl) = 0;
    }
  type = ((decl)->common.type);
  was_incomplete = (((decl)->decl.size) == 0);
  if (((enum tree_code) (decl)->common.code) == TYPE_DECL)
    {
      if (init && ((decl)->decl.initial))
	{
	  ((decl)->common.type) = type = ((init)->common.type);
	  ((decl)->decl.initial) = init = 0;
	}
      if ((((type)->type.lang_flag_5)) && ((decl)->decl.name))
	{
	  if (((((decl)->decl.name))->common.type) && ((decl)->common.type) != type)
	    warning ("shadowing previous type declaration of `%s'",
		     ((((decl)->decl.name))->identifier.pointer));
	  set_identifier_type_value (((decl)->decl.name), type);
	  (((type)->type.lang_specific)->type_flags.got_semicolon) = 1;
	}
      GNU_xref_decl (current_function_decl, decl);
      rest_of_decl_compilation (decl, 0,
				((decl)->decl.context) == 0, 0);
      goto finish_end;
    }
  if (type != error_mark_node && (((type)->type.lang_flag_5))
      && (((type)->type.lang_specific)->type_flags.declared_exception))
    {
      finish_exception_decl ((tree) 0, decl);
      (((type)->type.lang_specific)->type_flags.got_semicolon) = 1;
      goto finish_end;
    }
  if (((enum tree_code) (decl)->common.code) != FUNCTION_DECL)
    {
      ttype = target_type (type);
    }
  if (! ((decl)->decl.external_flag) && ((decl)->common.readonly_flag)
      && (((type)->type.lang_flag_3)))
    {
      was_readonly = 1;
      ((decl)->common.readonly_flag) = 0;
    }
  if (((enum tree_code) (decl)->common.code) == FIELD_DECL)
    {
      if (init && init != error_mark_node)
	my_friendly_assert (((init)->common.permanent_flag), 147);
      if (asmspec)
	{
	  ((((decl)->common.type))->decl.rtl) = 0;
	  ((decl)->decl.assembler_name) = get_identifier (asmspec);
	  make_decl_rtl (decl, asmspec, 0);
	}
    }
  else if (init != 0 && ((decl)->decl.initial) == 0)
    init = 0;
  else if (((decl)->decl.external_flag))
    ;
  else if (((enum tree_code) (type)->common.code) == REFERENCE_TYPE)
    {
      grok_reference_init (decl, type, init, &cleanup);
      init = 0;
    }
  GNU_xref_decl (current_function_decl, decl);
  if (((enum tree_code) (decl)->common.code) == FIELD_DECL || ((decl)->decl.external_flag))
    ;
  else if (((enum tree_code) (decl)->common.code) == CONST_DECL)
    {
      my_friendly_assert (((enum tree_code) (decl)->common.code) != REFERENCE_TYPE, 148);
      ((decl)->decl.initial) = init;
      my_friendly_assert (init != 0, 149);
      init = 0;
    }
  else if (init)
    {
      if ((((type)->type.lang_flag_3)))
	{
	  if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
	    init = digest_init (type, init, 0);
	  else if (((enum tree_code) (init)->common.code) == CONSTRUCTOR
		   && ((init)->exp.operands[ 1]) != (tree) 0)
	    {
	      error_with_decl (decl, "`%s' must be initialized by constructor, not by `{...}'");
	      init = error_mark_node;
	    }
	  if (current_binding_level == global_binding_level)
	    {
	      tree value = digest_init (type, empty_init_node, 0);
	      ((decl)->decl.initial) = value;
	    }
	  else
	    ((decl)->decl.initial) = error_mark_node;
	}
      else
	{
	  if (((enum tree_code) (init)->common.code) != TREE_VEC)
	    init = store_init_value (decl, init);
	  if (init)
	    ((decl)->decl.initial) = error_mark_node;
	}
    }
  else if ((*tree_code_type[(int) (((enum tree_code) (type)->common.code))]) == 't'
	   && ((((type)->type.lang_flag_5)) || (((type)->type.lang_flag_3))))
    {
      tree ctype = type;
      while (((enum tree_code) (ctype)->common.code) == ARRAY_TYPE)
	ctype = ((ctype)->common.type);
      if (! (((ctype)->type.lang_specific)->type_flags.needs_constructor))
	{
	  if ((((ctype)->type.lang_specific)->type_flags.const_needs_init))
	    error_with_decl (decl, "structure `%s' with uninitialized const members");
	  if ((((ctype)->type.lang_specific)->type_flags.ref_needs_init))
	    error_with_decl (decl, "structure `%s' with uninitialized reference members");
	}
      if (((enum tree_code) (decl)->common.code) == VAR_DECL
	  && !(((type)->type.lang_flag_3))
	  && (((type)->common.readonly_flag) || ((decl)->common.readonly_flag)))
	error_with_decl (decl, "uninitialized const `%s'");
      if (flag_pic == 0
	  && ((decl)->common.static_flag)
	  && ((decl)->common.public_flag)
	  && ! ((decl)->decl.external_flag)
	  && ((enum tree_code) (decl)->common.code) == VAR_DECL
	  && (((type)->type.lang_flag_3))
	  && (((decl)->decl.initial) == 0
	      || ((decl)->decl.initial) == error_mark_node))
	{
	  tree value = digest_init (type, empty_init_node, 0);
	  ((decl)->decl.initial) = value;
	}
    }
  else if (((enum tree_code) (decl)->common.code) == VAR_DECL
	   && ((enum tree_code) (type)->common.code) != REFERENCE_TYPE
	   && (((type)->common.readonly_flag) || ((decl)->common.readonly_flag)))
    {
	{
	  error_with_decl (decl, "uninitialized const `%s'");
	}
    }
  if (current_binding_level == global_binding_level && temporary)
    end_temporary_allocation ();
  if (((enum tree_code) (type)->common.code) == ARRAY_TYPE
      && ((type)->type.values) == 0
      && ((enum tree_code) (decl)->common.code) != TYPE_DECL)
    {
      int do_default
	= (((decl)->common.static_flag)
	   ? pedantic && ((decl)->decl.external_flag)
	   : !((decl)->decl.external_flag));
      tree initializer = init ? init : ((decl)->decl.initial);
      int failure = complete_array_type (type, initializer, do_default);
      if (failure == 1)
	error_with_decl (decl, "initializer fails to determine size of `%s'");
      if (failure == 2)
	{
	  if (do_default)
	    error_with_decl (decl, "array size missing in `%s'");
	  else if (!pedantic && ((decl)->common.static_flag))
	    ((decl)->decl.external_flag) = 1;
	}
      if (pedantic && ((type)->type.values) != 0
	  && tree_int_cst_lt (((((type)->type.values))->type.maxval),
			      integer_zero_node))
	error_with_decl (decl, "zero-size array `%s'");
      layout_decl (decl, 0);
    }
  if (((enum tree_code) (decl)->common.code) == VAR_DECL)
    {
      if (((decl)->common.static_flag) && ((decl)->decl.size) == 0)
	{
	  if (!((decl)->decl.external_flag) || ((decl)->decl.initial) != 0)
	    error_with_decl (decl, "storage size of `%s' isn't known");
	  init = 0;
	}
      else if (!((decl)->decl.external_flag) && ((decl)->decl.size) == 0)
	{
	  error_with_decl (decl, "storage size of `%s' isn't known");
	  ((decl)->common.type) = error_mark_node;
	}
      else if (!((decl)->decl.external_flag) && (((ttype)->type.lang_flag_5)))
	note_debug_info_needed (ttype);
      if ((((decl)->decl.external_flag) || ((decl)->common.static_flag))
	  && ((decl)->decl.size) != 0 && ! ((((decl)->decl.size))->common.constant_flag))
	error_with_decl (decl, "storage size of `%s' isn't constant");
      if (!((decl)->decl.external_flag) && (((type)->type.lang_flag_4)))
	{
	  int yes = suspend_momentary ();
	  if (init && ((enum tree_code) (init)->common.code) == WITH_CLEANUP_EXPR
	      && comptypes (((decl)->common.type), ((init)->common.type), 1))
	    {
	      cleanup = ((init)->exp.operands[ 2]);
	      init = ((init)->exp.operands[ 0]);
	      current_binding_level->have_cleanups = 1;
	      current_binding_level->more_exceptions_ok = 0;
	    }
	  else
	    cleanup = maybe_build_cleanup (decl);
	  resume_momentary (yes);
	}
    }
  else if (((enum tree_code) (decl)->common.code) == PARM_DECL && (((type)->type.lang_flag_4)))
    {
      if (temporary)
	end_temporary_allocation ();
      cleanup = maybe_build_cleanup (decl);
      if (temporary)
	resume_temporary_allocation ();
    }
  if (((enum tree_code) (decl)->common.code) == VAR_DECL || ((enum tree_code) (decl)->common.code) == FUNCTION_DECL
      || ((enum tree_code) (decl)->common.code) == RESULT_DECL)
    {
      int toplev = current_binding_level == global_binding_level;
      int was_temp
	= ((flag_traditional
	    || (((decl)->common.static_flag) && (((type)->type.lang_flag_4))))
	   && allocation_temporary_p ());
      if (was_temp)
	end_temporary_allocation ();
      if (cleanup && current_binding_level->parm_flag == 3)
	{
	  pop_implicit_try_blocks (decl);
	  current_binding_level->more_exceptions_ok = 0;
	}
      if (((enum tree_code) (decl)->common.code) == VAR_DECL
	  && current_binding_level != global_binding_level
	  && ! ((decl)->common.static_flag)
	  && type_needs_gc_entry (type))
	((decl)->decl.result) = size_int (++current_function_obstack_index);
      if (((enum tree_code) (decl)->common.code) == VAR_DECL && ((decl)->decl.virtual_flag))
	make_decl_rtl (decl, 0, toplev);
      else if (((enum tree_code) (decl)->common.code) == VAR_DECL
	       && ((decl)->common.readonly_flag)
	       && ((decl)->decl.initial) != 0
	       && ((decl)->decl.initial) != error_mark_node
	       && ((decl)->decl.initial) != empty_init_node)
	{
	  ((decl)->decl.initial) = save_expr (((decl)->decl.initial));
	  if (asmspec)
	    ((decl)->decl.assembler_name) = get_identifier (asmspec);
	  if (! toplev
	      && ((decl)->common.static_flag)
	      && ! ((decl)->common.side_effects_flag)
	      && ! ((decl)->common.public_flag)
	      && ! ((decl)->decl.external_flag)
	      && ! (((type)->type.lang_flag_4))
	      && ((decl)->decl.mode) != BLKmode)
	    {
	      ((decl)->decl.rtl) = gen_reg_rtx (((decl)->decl.mode));
	      store_expr (((decl)->decl.initial), ((decl)->decl.rtl), 0);
	      ((decl)->common.asm_written_flag) = 1;
	    }
	  else if (toplev)
	    {
	      ((decl)->common.used_flag) = 1;
	      if (((decl)->common.static_flag) && !interface_unknown)
		{
		  ((decl)->common.public_flag) = 1;
		  ((decl)->decl.external_flag) = interface_only;
		}
	      make_decl_rtl (decl, asmspec, toplev);
	    }
	  else
	    rest_of_decl_compilation (decl, asmspec, toplev, 0);
	}
      else if (((enum tree_code) (decl)->common.code) == VAR_DECL
	       && ((decl)->decl.lang_specific)
	       && (((decl)->decl.lang_flag_3)))
	{
	  if (((decl)->common.static_flag))
	    if (init == 0
		)
	      {
		((decl)->decl.external_flag) = 1;
		make_decl_rtl (decl, asmspec, 1);
	      }
	    else
	      rest_of_decl_compilation (decl, asmspec, toplev, 0);
	  else
	    goto finish_end0;
	}
      else
	rest_of_decl_compilation (decl, asmspec, toplev, 0);
      if (was_temp)
	resume_temporary_allocation ();
      if (type != error_mark_node
	  && ((type)->type.lang_specific)
	  && (((type)->type.lang_specific)->abstract_virtuals))
	abstract_virtuals_error (decl, type);
      else if ((((enum tree_code) (type)->common.code) == FUNCTION_TYPE
		|| ((enum tree_code) (type)->common.code) == METHOD_TYPE)
	       && ((((type)->common.type))->type.lang_specific)
	       && (((((type)->common.type))->type.lang_specific)->abstract_virtuals))
	abstract_virtuals_error (decl, ((type)->common.type));
      if (((enum tree_code) (decl)->common.code) == FUNCTION_DECL)
	{
	  if ((((decl)->decl.lang_flag_4)))
	    {
	      tree parmtypes = ((type)->type.values);
	      tree prev = (tree) 0;
	      tree original_name = ((decl)->decl.name);
	      struct lang_decl *tmp_lang_decl = ((decl)->decl.lang_specific);
	      copy_decl_lang_specific (decl);
	      while (parmtypes && parmtypes != void_list_node)
		{
		  if (((parmtypes)->list.purpose))
		    {
		      tree fnname, fndecl;
		      tree *argp = prev
			? & ((prev)->common.chain)
			  : & ((type)->type.values);
		      *argp = (tree) 0;
		      fnname = build_decl_overload (original_name, ((type)->type.values), 0);
		      *argp = parmtypes;
		      fndecl = build_decl (FUNCTION_DECL, fnname, type);
		      ((fndecl)->decl.external_flag) = ((decl)->decl.external_flag);
		      ((fndecl)->common.public_flag) = ((decl)->common.public_flag);
		      ((fndecl)->decl.inline_flag) = ((decl)->decl.inline_flag);
		      ((fndecl)->common.used_flag) = 1;
		      ((fndecl)->common.asm_written_flag) = 1;
		      ((fndecl)->decl.initial) = (tree) 0;
		      ((fndecl)->decl.lang_specific) = ((decl)->decl.lang_specific);
		      fndecl = pushdecl (fndecl);
		      ((fndecl)->decl.initial) = error_mark_node;
		      ((fndecl)->decl.rtl) = ((decl)->decl.rtl);
		    }
		  prev = parmtypes;
		  parmtypes = ((parmtypes)->common.chain);
		}
	      ((decl)->decl.lang_specific) = tmp_lang_decl;
	    }
	}
      else if (((decl)->decl.external_flag))
	;
      else if (((decl)->common.static_flag) && type != error_mark_node)
	{
	  if ((((type)->type.lang_flag_3)) || init != (tree) 0)
	    expand_static_init (decl, init);
	  else if ((((type)->type.lang_flag_4)))
	    static_aggregates = perm_tree_cons ((tree) 0, decl,
						static_aggregates);
	  if (flag_gc && type_needs_gc_entry (type))
	    build_static_gc_entry (decl, type);
	}
      else if (current_binding_level != global_binding_level)
	{
	  if (was_incomplete && ! ((decl)->common.static_flag))
	    {
	      ((decl)->common.addressable_flag) = ((decl)->common.used_flag);
	      if (((decl)->decl.size) == 0)
		((decl)->decl.initial) = 0;
	      expand_decl (decl);
	    }
	  else if (! ((decl)->common.asm_written_flag)
		   && (((type)->type.size) != 0 || ((enum tree_code) (type)->common.code) == ARRAY_TYPE))
	    {
	      if (((decl)->decl.rtl) == 0)
		expand_decl (decl);
	      else if (cleanup)
		{
		  expand_decl_cleanup ((tree) 0, cleanup);
		  cleanup = 0;
		}
	    }
	  if (((decl)->decl.size) && type != error_mark_node)
	    {
	      expand_decl_init (decl);
	      if (init || (((type)->type.lang_flag_3)))
		{
		  emit_line_note (((decl)->decl.filename), ((decl)->decl.linenum));
		  expand_aggr_init (decl, init, 0);
		}
	      if ((((type)->type.lang_flag_3)))
		((decl)->common.used_flag) = 0;
	      if (cleanup)
		{
		  if (! expand_decl_cleanup (decl, cleanup))
		    error_with_decl (decl, "parser lost in parsing declaration of `%s'");
		}
	    }
	}
    finish_end0:
      {
	tree context = ((decl)->decl.context);
	if (context
	    && (*tree_code_type[(int) (((enum tree_code) (context)->common.code))]) == 't'
	    && (((enum tree_code) (decl)->common.code) == VAR_DECL
		|| (((enum tree_code) (decl)->common.code) == FUNCTION_DECL
		    && ((context)->type.size) != 0
		    && (((((context)->type.name))->decl.name)) == current_class_name
		    )))
	  popclass (1);
      }
    }
 finish_end:
  if (need_pop)
    {
      pop_obstacks ();
    }
  if (was_readonly)
    ((decl)->common.readonly_flag) = 1;
  if (flag_cadillac)
    cadillac_finish_decl (decl);
}
static void
expand_static_init (decl, init)
     tree decl;
     tree init;
{
  tree oldstatic = value_member (decl, static_aggregates);
  if (oldstatic)
    {
      if (((oldstatic)->list.purpose))
	error_with_decl (decl, "multiple initializations given for `%s'");
    }
  else if (current_binding_level != global_binding_level)
    {
      tree temp;
      push_obstacks (&permanent_obstack, &permanent_obstack);
      temp = get_temp_name (integer_type_node, 1);
      rest_of_decl_compilation (temp, (tree) 0, 0, 0);
      expand_start_cond (build_binary_op (EQ_EXPR, temp,
					  integer_zero_node), 0);
      expand_assignment (temp, integer_one_node, 0, 0);
      if ((((((decl)->common.type))->type.lang_flag_3)))
	{
	  expand_aggr_init (decl, init, 0);
	  do_pending_stack_adjust ();
	}
      else
	expand_assignment (decl, init, 0, 0);
      expand_end_cond ();
      if ((((((decl)->common.type))->type.lang_flag_4)))
	{
	  static_aggregates = perm_tree_cons (temp, decl, static_aggregates);
	  ((static_aggregates)->common.static_flag) = 1;
	}
      pop_obstacks ();
    }
  else
    {
      if (! (((((decl)->common.type))->type.lang_flag_3)))
	preserve_initializer ();
      static_aggregates = perm_tree_cons (init, decl, static_aggregates);
    }
}
int
complete_array_type (type, initial_value, do_default)
     tree type;
     tree initial_value;
     int do_default;
{
  register tree maxindex = (tree) 0;
  int value = 0;
  if (initial_value)
    {
      if (((enum tree_code) (initial_value)->common.code) == STRING_CST)
	maxindex =   build_int_2_wide ((int) (((initial_value)->string.length) - 1), (int) ( 0));
      else if (((enum tree_code) (initial_value)->common.code) == CONSTRUCTOR)
	{
	  register int nelts
	    = list_length (((initial_value)->exp.operands[ 1]));
	  maxindex =   build_int_2_wide ((int) (nelts - 1), (int) ( 0));
	}
      else
	{
	  if (initial_value != error_mark_node)
	    value = 1;
	  maxindex =   build_int_2_wide ((int) (1), (int) ( 0));
	}
    }
  if (!maxindex)
    {
      if (do_default)
	maxindex =   build_int_2_wide ((int) (1), (int) ( 0));
      value = 2;
    }
  if (maxindex)
    {
      ((type)->type.values) = build_index_type (maxindex);
      if (!((maxindex)->common.type))
	((maxindex)->common.type) = ((type)->type.values);
    }
  layout_type (type);
  return value;
}
static int
member_function_or_else (ctype, cur_type, string)
     tree ctype, cur_type;
     char *string;
{
  if (ctype && ctype != cur_type)
    {
      error (string, ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)));
      return 0;
    }
  return 1;
}
static tree
grokfndecl (ctype, type, declarator, virtualp, flags, quals, raises, check)
     tree ctype, type;
     tree declarator;
     int virtualp;
     enum overload_flags flags;
     tree quals, raises;
     int check;
{
  tree cname, decl;
  int staticp = ctype && ((enum tree_code) (type)->common.code) == FUNCTION_TYPE;
  if (ctype)
    cname = ((enum tree_code) (((ctype)->type.name))->common.code) == TYPE_DECL
      ? (((((ctype)->type.name))->decl.name)) : ((ctype)->type.name);
  else
    cname = (tree) 0;
  if (raises)
    {
      type = build_exception_variant (ctype, type, raises);
      raises = ((type)->type.noncopied_parts);
    }
  decl = build_lang_decl (FUNCTION_DECL, declarator, type);
  if (((type)->common.volatile_flag))
      ((decl)->common.volatile_flag) = 1;
  if (staticp)
    {
      (((decl)->decl.lang_specific)->decl_flags.static_function) = 1;
      ((decl)->decl.context) = ctype;
      (((decl)->decl.lang_specific)->decl_flags.context) = ctype;
    }
  ((decl)->decl.external_flag) = 1;
  if (quals != (tree) 0 && ((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
    {
      error ("functions cannot have method qualifiers");
      quals = (tree) 0;
    }
  if (declarator == ansi_opname[(int) DELETE_EXPR])
    {
      tree args = ((type)->type.values);
      int style1, style2;
      if (ctype && args && ((enum tree_code) (type)->common.code) == METHOD_TYPE)
	args = ((args)->common.chain);
      style1 = type_list_equal (args,
				tree_cons ((tree) 0, ptr_type_node,
					   void_list_node));
      style2 = style1 != 0 ? 0 :
	type_list_equal (args,
			 tree_cons ((tree) 0, ptr_type_node,
				    tree_cons ((tree) 0, sizetype,
					       void_list_node)));
      if (ctype == (tree) 0)
	{
	  if (! style1)
	    error ("global operator delete must be declared as taking a single argument of type void*");
	}
      else
	if (! style1 && ! style2)
	  error ("operator delete cannot be overloaded");
    }
  if (check < 0)
    return decl;
  if (flags == NO_SPECIAL && ctype && constructor_name (cname) == declarator)
    {
      tree tmp;
      (((decl)->decl.lang_specific)->decl_flags.constructor_attr) = 1;
      grokclassfn (ctype, declarator, decl, flags, quals);
      if (check)
	check_classfn (ctype, declarator, decl, flags);
      grok_ctor_properties (ctype, decl);
      if (check == 0)
	{
	  tmp = lookup_name (((decl)->decl.assembler_name), 0);
	  if (tmp == 0)
	      (((struct lang_identifier *)(((decl)->decl.assembler_name)))->global_value) = decl;
	  else if (((enum tree_code) (tmp)->common.code) != ((enum tree_code) (decl)->common.code))
	    error_with_decl (decl, "inconsistent declarations for `%s'");
	  else
	    {
	      duplicate_decls (decl, tmp);
	      decl = tmp;
	    }
	  make_decl_rtl (decl, (tree) 0, 1);
	}
    }
  else
    {
      tree tmp;
      if (ctype != (tree) 0)
	grokclassfn (ctype, cname, decl, flags, quals);
      if (((((decl)->decl.name))->common.lang_flag_2))
	grok_op_properties (decl, virtualp);
      if (ctype != (tree) 0 && check)
	check_classfn (ctype, cname, decl, flags);
      if (ctype == (tree) 0 || check)
	return decl;
      tmp = lookup_name (((decl)->decl.assembler_name), 0);
      if (tmp == 0)
	  (((struct lang_identifier *)(((decl)->decl.assembler_name)))->global_value) = decl;
      else if (((enum tree_code) (tmp)->common.code) != ((enum tree_code) (decl)->common.code))
	error_with_decl (decl, "inconsistent declarations for `%s'");
      else
	{
	  duplicate_decls (decl, tmp);
	  decl = tmp;
	}
      make_decl_rtl (decl, (tree) 0, 1);
      {
	tree binfos = (((((ctype)->type.binfo)))->vec.a[ 4]);
	int i, n_baselinks = binfos ? ((binfos)->vec.length) : 0;
	for (i = 0; i < n_baselinks; i++)
	  {
	    tree base_binfo = ((binfos)->vec.a[ i]);
	    if ((((((base_binfo)->common.type))->common.lang_flag_2)) || flag_all_virtual == 1)
	      {
		tmp = get_first_matching_virtual (base_binfo, decl,
						  flags == DTOR_FLAG);
		if (tmp)
		  {
		    if (staticp)
		      {
			error_with_decl (decl, "method `%s' may not be declared static");
			error_with_decl (tmp, "(since `%s' declared virtual in base class.)");
			break;
		      }
		    virtualp = 1;
		    if (((((((base_binfo)->common.type))->common.lang_flag_3))
			 || (((ctype)->type.lang_specific)->type_flags.uses_multiple_inheritance))
			&& ((base_binfo)->common.type) != ((tmp)->decl.context))
		      tmp = get_first_matching_virtual (((((tmp)->decl.context))->type.binfo),
							decl, flags == DTOR_FLAG);
		    if (value_member (tmp, ((decl)->decl.vindex)) == (tree) 0)
		      {
			tree argtypes = ((((decl)->common.type))->type.values);
			tree base_variant = ((((argtypes)->list.value))->common.type);
			argtypes = commonparms (((((((tmp)->common.type))->type.values))->common.chain),
						((argtypes)->common.chain));
			type = build_cplus_method_type (base_variant, ((type)->common.type), argtypes);
			if (raises)
			  {
			    type = build_exception_variant (ctype, type, raises);
			    raises = ((type)->type.noncopied_parts);
			  }
			((decl)->common.type) = type;
			((decl)->decl.vindex)
			  = tree_cons ((tree) 0, tmp, ((decl)->decl.vindex));
		      }
		  }
	      }
	  }
      }
      if (virtualp)
	{
	  if (((decl)->decl.vindex) == (tree) 0)
	    ((decl)->decl.vindex) = error_mark_node;
	  ((((decl)->decl.name))->common.lang_flag_1) = 1;
	  if (ctype && (((ctype)->type.lang_specific)->type_flags.vtable_needs_writing)
	      &&   (((struct lang_identifier *)((((((ctype)->type.name))->decl.name))))->class_template_info) == (tree) 0
	      && (write_virtuals == 2
		  || (write_virtuals == 3
		      && ! (((ctype)->type.lang_specific)->type_flags.interface_unknown))))
	    ((decl)->common.public_flag) = 1;
	}
    }
  return decl;
}
static tree
grokvardecl (ctype, type, declarator, specbits, initialized)
     tree ctype, type;
     tree declarator;
     int specbits, initialized;
{
  tree decl;
  if (((enum tree_code) (type)->common.code) == OFFSET_TYPE)
    {
      tree field = lookup_field (((type)->type.maxval),
				 declarator, 0, 0);
      if (field == (tree) 0 || ((enum tree_code) (field)->common.code) != VAR_DECL)
	{
	  tree basetype = ((type)->type.maxval);
	  error ("`%s' is not a static member of class `%s'",
		 ((declarator)->identifier.pointer),
		 ((((((((basetype)->type.name))->decl.name)))->identifier.pointer)));
	  type = ((type)->common.type);
	  decl = build_lang_field_decl (VAR_DECL, declarator, type);
	  ((decl)->decl.context) = basetype;
	  (((decl)->decl.lang_specific)->decl_flags.context) = basetype;
	}
      else
	{
	  tree f_type = ((field)->common.type);
	  tree o_type = ((type)->common.type);
	  if (((f_type)->type.size) == (tree) 0)
	    {
	      if (((enum tree_code) (f_type)->common.code) != ((enum tree_code) (o_type)->common.code)
		  || (((enum tree_code) (f_type)->common.code) == ARRAY_TYPE
		      && ((f_type)->common.type) != ((o_type)->common.type)))
		error ("redeclaration of type for `%s'",
		       ((declarator)->identifier.pointer));
	      else if (((o_type)->type.size) != (tree) 0)
		((field)->common.type) = type;
	    }
	  else if (f_type != o_type)
	    error ("redeclaration of type for `%s'",
		   ((declarator)->identifier.pointer));
	  decl = field;
	  if (initialized && ((decl)->decl.initial)
	      && (((enum tree_code) (((decl)->decl.initial))->common.code) != CONSTRUCTOR
		  || ((((decl)->decl.initial))->exp.operands[ 1]) != (tree) 0))
	    error_with_aggr_type (((decl)->decl.context),
				  "multiple initializations of static member `%s::%s'",
				  ((((decl)->decl.name))->identifier.pointer));
	}
    }
  else decl = build_decl (VAR_DECL, declarator, type);
  if (specbits & (1 << (int) RID_EXTERN))
    {
      (((decl)->decl.lang_flag_2)) = 1;
      ((decl)->decl.external_flag) = !initialized;
    }
  if (((decl)->decl.context) != 0
      && (((((decl)->decl.context))->type.lang_flag_5)))
    {
      ((decl)->common.public_flag) = 1;
      ((decl)->common.static_flag) = 1;
      ((decl)->decl.external_flag) = !initialized;
    }
  else if (current_binding_level == global_binding_level)
    {
      ((decl)->common.public_flag) = !(specbits & (1 << (int) RID_STATIC));
      ((decl)->common.static_flag) = ! ((decl)->decl.external_flag);
    }
  else
    {
      ((decl)->common.static_flag) = (specbits & (1 << (int) RID_STATIC)) != 0;
      ((decl)->common.public_flag) = ((decl)->decl.external_flag);
    }
  return decl;
}
enum return_types { return_normal, return_ctor, return_dtor, return_conversion, };
tree
grokdeclarator (declarator, declspecs, decl_context, initialized, raises)
     tree declspecs;
     tree declarator;
     enum decl_context decl_context;
     int initialized;
     tree raises;
{
  extern int current_class_depth;
  int specbits = 0;
  int nclasses = 0;
  tree spec;
  tree type = (tree) 0;
  int longlong = 0;
  int constp;
  int volatilep;
  int virtualp, friendp, inlinep, staticp;
  int explicit_int = 0;
  int explicit_char = 0;
  tree typedef_decl = 0;
  char *name;
  tree typedef_type = 0;
  int funcdef_flag = 0;
  enum tree_code innermost_code = ERROR_MARK;
  int bitfield = 0;
  int size_varies = 0;
  tree init = 0;
  enum return_types return_type = return_normal;
  tree dname = (tree) 0;
  tree ctype = current_class_type;
  tree ctor_return_type = (tree) 0;
  enum overload_flags flags = NO_SPECIAL;
  int seen_scope_ref = 0;
  tree quals = 0;
  if (decl_context == FUNCDEF)
    funcdef_flag = 1, decl_context = NORMAL;
  else if (decl_context == MEMFUNCDEF)
    funcdef_flag = -1, decl_context = FIELD;
  else if (decl_context == BITFIELD)
    bitfield = 1, decl_context = FIELD;
  if (flag_traditional && allocation_temporary_p ())
    end_temporary_allocation ();
  {
    tree type, last = 0;
    register tree decl = declarator;
    name = 0;
    while (decl && ((enum tree_code) (decl)->common.code) == ARRAY_REF)
      {
	last = decl;
	decl = ((decl)->exp.operands[ 0]);
      }
    if (decl && declspecs
        && ((enum tree_code) (decl)->common.code) == CALL_EXPR
        && ((decl)->exp.operands[ 0])
        && (((enum tree_code) (((decl)->exp.operands[ 0]))->common.code) == IDENTIFIER_NODE
	    || ((enum tree_code) (((decl)->exp.operands[ 0]))->common.code) == SCOPE_REF))
      {
        type = ((enum tree_code) (((declspecs)->list.value))->common.code) == IDENTIFIER_NODE
          ? lookup_name (((declspecs)->list.value), 1) :
        ((((((declspecs)->list.value))->type.lang_flag_5))
         ? ((((declspecs)->list.value))->type.name) : (tree) 0);
        if (type && ((enum tree_code) (type)->common.code) == TYPE_DECL
            && (((((type)->common.type))->type.lang_flag_5))
            && parmlist_is_exprlist (((decl)->exp.operands[ 1])))
          {
            if (decl_context == FIELD
                && ((((decl)->exp.operands[ 1]))->common.chain))
              {
                sorry ("initializer lists for field declarations");
                decl = ((decl)->exp.operands[ 0]);
		if (last)
		  {
		    ((last)->exp.operands[ 0]) = decl;
		    decl = declarator;
		  }
                declarator = decl;
                init = error_mark_node;
                goto bot;
              }
            else
              {
		init = ((decl)->exp.operands[ 1]);
		if (last)
		  {
		    ((last)->exp.operands[ 0]) = ((decl)->exp.operands[ 0]);
		    if (pedantic && init)
		      {
		      error ("arrays cannot take initializers");
		      init = error_mark_node;
		    }
		  }
		else
		  declarator = ((declarator)->exp.operands[ 0]);
		decl = start_decl (declarator, declspecs, 1, (tree) 0);
		finish_decl (decl, init, (tree) 0, 1);
		return 0;
              }
          }
        if (parmlist_is_random (((decl)->exp.operands[ 1])))
          {
	    decl = ((decl)->exp.operands[ 0]);
	    if (((enum tree_code) (decl)->common.code) == SCOPE_REF)
	      {
		if (((decl)->exp.complexity))
		  my_friendly_abort (15);
		decl = ((decl)->exp.operands[ 1]);
	      }
	    if (((enum tree_code) (decl)->common.code) == IDENTIFIER_NODE)
	      name = ((decl)->identifier.pointer);
	    if (name)
	      error ("bad parameter list specification for function `%s'",
		     name);
	    else
	      error ("bad parameter list specification for function");
            return void_type_node;
          }
      bot:
        ;
      }
    else
      decl = declarator;
    while (decl)
      switch (((enum tree_code) (decl)->common.code))
        {
	case COND_EXPR:
	  ctype = (tree) 0;
	  decl = ((decl)->exp.operands[ 0]);
	  break;
	case BIT_NOT_EXPR:      
	  {
	    tree name = ((decl)->exp.operands[ 0]);
	    tree rename = (tree) 0;
	    my_friendly_assert (flags == NO_SPECIAL, 152);
	    flags = DTOR_FLAG;
	    return_type = return_dtor;
	    my_friendly_assert (((enum tree_code) (name)->common.code) == IDENTIFIER_NODE, 153);
	    if (ctype == (tree) 0)
	      {
		if (current_class_type == (tree) 0)
		  {
		    error ("destructors must be member functions");
		    flags = NO_SPECIAL;
		  }
		else
		  {
		    tree t = constructor_name (current_class_name);
		    if (t != name)
		      rename = t;
		  }
	      }
	    else
	      {
		tree t = constructor_name (ctype);
		if (t != name)
		  rename = t;
	      }
	    if (rename)
	      {
		error ("destructor `%s' must match class name `%s'",
		       ((name)->identifier.pointer),
		       ((rename)->identifier.pointer));
		((decl)->exp.operands[ 0]) = rename;
	      }
	    decl = name;
	  }
	  break;
	case ADDR_EXPR:         
	case ARRAY_REF:
	case INDIRECT_REF:
	  ctype = (tree) 0;
	  innermost_code = ((enum tree_code) (decl)->common.code);
	  decl = ((decl)->exp.operands[ 0]);
	  break;
	case CALL_EXPR:
	  innermost_code = ((enum tree_code) (decl)->common.code);
	  decl = ((decl)->exp.operands[ 0]);
	  if (decl_context == FIELD && ctype == (tree) 0)
	    ctype = current_class_type;
	  if (ctype != (tree) 0
	      && decl != (tree) 0 && flags != DTOR_FLAG
	      && decl == constructor_name (ctype))
	    {
	      return_type = return_ctor;
	      ctor_return_type = ctype;
	    }
	  ctype = (tree) 0;
	  break;
	case IDENTIFIER_NODE:
	  dname = decl;
	  name = ((decl)->identifier.pointer);
	  decl = 0;
	  break;
	case RECORD_TYPE:
	case UNION_TYPE:
	case ENUMERAL_TYPE:
	  error ("declarator name missing");
	  dname = ((decl)->type.name);
	  if (dname && ((enum tree_code) (dname)->common.code) == TYPE_DECL)
	    dname = ((dname)->decl.name);
	  name = dname ? ((dname)->identifier.pointer) : "<nameless>";
	  declspecs = temp_tree_cons ((tree) 0, decl, declspecs);
	  decl = 0;
	  break;
	case TYPE_EXPR:
	  if (ctype == (tree) 0)
	    {
	      error ("operator `%s' must be declared as a member",
		     ((((((decl)->common.type))->list.value))->identifier.pointer));
	      return (tree) 0;
	    }
	  ctype = (tree) 0;
	  my_friendly_assert (flags == NO_SPECIAL, 154);
	  flags = TYPENAME_FLAG;
	  name = "operator <typename>";
	  decl = ((decl)->exp.operands[ 0]);
	  return_type = return_conversion;
	  break;
	case SCOPE_REF:
	  if (seen_scope_ref == 1)
	    error ("multiple `::' terms in declarator invalid");
	  seen_scope_ref += 1;
	  {
	    tree cname = ((decl)->exp.operands[ 0]);
	    if (cname == (tree) 0)
	      ctype = (tree) 0;
	    else if (  (((enum tree_code) (cname)->common.code) == RECORD_TYPE || ((enum tree_code) (cname)->common.code) == UNION_TYPE)
		     || ((enum tree_code) (cname)->common.code) == UNINSTANTIATED_P_TYPE)
	      ctype = cname;
	    else if (! is_aggr_typedef (cname, 1))
	      {
		((decl)->exp.operands[ 0]) = 0;
	      }
	    else if (((decl)->exp.operands[ 1])
		     && ((enum tree_code) (((decl)->exp.operands[ 1]))->common.code) == INDIRECT_REF)
	      {
		((decl)->exp.operands[ 0]) = (((cname)->common.type));
	      }
	    else if (ctype == (tree) 0)
	      {
		ctype = (((cname)->common.type));
		((decl)->exp.operands[ 0]) = ctype;
	      }
	    else if (((decl)->exp.complexity) == current_class_depth)
	      ((decl)->exp.operands[ 0]) = ctype;
	    else
	      {
		if (! (get_base_distance ((((cname)->common.type)),  ctype, 0, 0) >= 0))
		  {
		    error ("type `%s' is not derived from type `%s'",
			   ((cname)->identifier.pointer),
			   ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)));
		    ((decl)->exp.operands[ 0]) = 0;
		  }
		else
		  {
		    ctype = (((cname)->common.type));
		    ((decl)->exp.operands[ 0]) = ctype;
		  }
	      }
	    decl = ((decl)->exp.operands[ 1]);
	    if (ctype)
	      {
		if (((enum tree_code) (decl)->common.code) == IDENTIFIER_NODE
		    && constructor_name (ctype) == decl)
		  {
		    return_type = return_ctor;
		    ctor_return_type = ctype;
		  }
		else if (((enum tree_code) (decl)->common.code) == BIT_NOT_EXPR
			 && ((enum tree_code) (((decl)->exp.operands[ 0]))->common.code) == IDENTIFIER_NODE
			 && constructor_name (ctype) == ((decl)->exp.operands[ 0]))
		  {
		    return_type = return_dtor;
		    ctor_return_type = ctype;
		    flags = DTOR_FLAG;
		    decl = ((decl)->exp.operands[ 0]);
		  }
	      }
	  }
	  break;
	case ERROR_MARK:
	  decl = (tree) 0;
	  break;
	default:
	  my_friendly_assert (0, 155);
	}
    if (name == 0)
      name = "type name";
  }
  if (funcdef_flag && innermost_code != CALL_EXPR)
    return 0;
  if (decl_context == NORMAL
      && current_binding_level->level_chain == global_binding_level)
    decl_context = PARM;
  for (spec = declspecs; spec; spec = ((spec)->common.chain))
    {
      register int i;
      register tree id = ((spec)->list.value);
      if (((enum tree_code) (spec)->common.code) != TREE_LIST)
	return 0;
      if (((enum tree_code) (id)->common.code) == IDENTIFIER_NODE)
	{
	  if (id == ridpointers[(int) RID_INT])
	    {
	      if (type)
		error ("extraneous `int' ignored");
	      else
		{
		  explicit_int = 1;
		  type = ((  (((struct lang_identifier *)(id))->global_value))->common.type);
		}
	      goto found;
	    }
	  if (id == ridpointers[(int) RID_CHAR])
	    {
	      if (type)
		error ("extraneous `char' ignored");
	      else
		{
		  explicit_char = 1;
		  type = ((  (((struct lang_identifier *)(id))->global_value))->common.type);
		}
	      goto found;
	    }
	  if (id == ridpointers[(int) RID_WCHAR])
	    {
	      if (type)
		error ("extraneous `__wchar_t' ignored");
	      else
		{
		  type = ((  (((struct lang_identifier *)(id))->global_value))->common.type);
		}
	      goto found;
	    }
	  if ((((id)->common.type) ? 1 : 0))
	    {
	      if (type)
		error ("multiple declarations `%s' and `%s'",
		       ((type)->identifier.pointer),
		       ((id)->identifier.pointer));
	      else
		type = (((id)->common.type));
	      goto found;
	    }
	  for (i = (int) RID_UNSIGNED; i < (int) RID_MAX; i++)
	    {
	      if (ridpointers[i] == id)
		{
		  if (i == (int) RID_LONG && specbits & (1<<i))
		    {
		      if (pedantic)
			pedwarn ("duplicate `%s'", ((id)->identifier.pointer));
		      else if (longlong)
		        error ("`long long long' is too long for GCC");
		      else
			longlong = 1;
		    }
		  else if (specbits & (1 << i))
		    warning ("duplicate `%s'", ((id)->identifier.pointer));
		  specbits |= 1 << i;
		  goto found;
		}
	    }
	}
      if (type)
	error ("two or more data types in declaration of `%s'", name);
      else if (((enum tree_code) (id)->common.code) == IDENTIFIER_NODE)
	{
	  register tree t = lookup_name (id, 1);
	  if (!t || ((enum tree_code) (t)->common.code) != TYPE_DECL)
	    error ("`%s' fails to be a typedef or built in type",
		   ((id)->identifier.pointer));
	  else
	    {
	      type = ((t)->common.type);
	      typedef_decl = t;
	    }
	}
      else if (((enum tree_code) (id)->common.code) != ERROR_MARK)
	type = id;
    found: {}
    }
  typedef_type = type;
  if (type == 0)
    {
      explicit_int = -1;
      if (return_type == return_dtor)
	type = void_type_node;
      else if (return_type == return_ctor)
	type = ((ctor_return_type)->type.pointer_to);
      else
	{
	  if (funcdef_flag && explicit_warn_return_type
	      && return_type == return_normal
	      && ! (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
				| (1 << (int) RID_SIGNED) | (1 << (int) RID_UNSIGNED))))
	    warn_about_return_type = 1;
	  type = integer_type_node;
	}
    }
  else if (return_type == return_dtor)
    {
      error ("return type specification for destructor invalid");
      type = void_type_node;
    }
  else if (return_type == return_ctor)
    {
      error ("return type specification for constructor invalid");
      type = ((ctor_return_type)->type.pointer_to);
    }
  else if ((specbits & (1 << (int) RID_FRIEND))
	   && (((type)->type.lang_flag_5))
	   && ! (((type)->type.lang_specific)->type_flags.being_defined)
	   && ((type)->type.size) == (tree) 0
	   && ! ((((((((type)->type.name))->decl.name)))->identifier.pointer)[0] == '$' 				  && (((((((type)->type.name))->decl.name)))->identifier.pointer)[1] == '_')
	   && current_function_decl == (tree) 0
	   && decl_context != PARM)
    {
      globalize_nested_type (type);
    }
  ctype = (tree) 0;
  if ((specbits & 1 << (int) RID_LONG)
      && ((type)->type.main_variant) == double_type_node)
    {
      specbits &= ~ (1 << (int) RID_LONG);
      type = build_type_variant (long_double_type_node, ((type)->common.readonly_flag),
				 ((type)->common.volatile_flag));
    }
  if (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
		  | (1 << (int) RID_UNSIGNED) | (1 << (int) RID_SIGNED)))
    {
      int ok = 0;
      if (((enum tree_code) (type)->common.code) == REAL_TYPE)
	error ("short, signed or unsigned invalid for `%s'", name);
      else if (((enum tree_code) (type)->common.code) != INTEGER_TYPE || type == wchar_type_node)
	error ("long, short, signed or unsigned invalid for `%s'", name);
      else if ((specbits & 1 << (int) RID_LONG)
	       && (specbits & 1 << (int) RID_SHORT))
	error ("long and short specified together for `%s'", name);
      else if (((specbits & 1 << (int) RID_LONG)
		|| (specbits & 1 << (int) RID_SHORT))
	       && explicit_char)
	error ("long or short specified with char for `%s'", name);
      else if (((specbits & 1 << (int) RID_LONG)
		|| (specbits & 1 << (int) RID_SHORT))
	       && ((enum tree_code) (type)->common.code) == REAL_TYPE)
	error ("long or short specified with floating type for `%s'", name);
      else if ((specbits & 1 << (int) RID_SIGNED)
	       && (specbits & 1 << (int) RID_UNSIGNED))
	error ("signed and unsigned given together for `%s'", name);
      else
	{
	  ok = 1;
	  if (!explicit_int && !explicit_char && pedantic)
	    {
	      pedwarn ("long, short, signed or unsigned used invalidly for `%s'",
		       name);
	      if (flag_pedantic_errors)
		ok = 0;
	    }
	}
      if (! ok)
	{
	  specbits &= ~((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
			| (1 << (int) RID_UNSIGNED) | (1 << (int) RID_SIGNED));
	  longlong = 0;
	}
    }
  if (specbits & 1 << (int) RID_UNSIGNED
      || (bitfield && flag_traditional)
      || (bitfield && ! flag_signed_bitfields
	  && (explicit_int || explicit_char
	      || ! (typedef_decl != 0
		    && (((typedef_decl))->decl.lang_flag_1)))
	  && ((enum tree_code) (type)->common.code) != ENUMERAL_TYPE
	  && !(specbits & 1 << (int) RID_SIGNED)))
    {
      if (longlong)
	type = long_long_unsigned_type_node;
      else if (specbits & 1 << (int) RID_LONG)
	type = long_unsigned_type_node;
      else if (specbits & 1 << (int) RID_SHORT)
	type = short_unsigned_type_node;
      else if (type == char_type_node)
	type = unsigned_char_type_node;
      else if (typedef_decl)
	type = unsigned_type (type);
      else
	type = unsigned_type_node;
    }
  else if ((specbits & 1 << (int) RID_SIGNED)
	   && type == char_type_node)
    type = signed_char_type_node;
  else if (longlong)
    type = long_long_integer_type_node;
  else if (specbits & 1 << (int) RID_LONG)
    type = long_integer_type_node;
  else if (specbits & 1 << (int) RID_SHORT)
    type = short_integer_type_node;
  constp = !! (specbits & 1 << (int) RID_CONST) + ((type)->common.readonly_flag);
  volatilep = !! (specbits & 1 << (int) RID_VOLATILE) + ((type)->common.volatile_flag);
  staticp = 0;
  inlinep = !! (specbits & (1 << (int) RID_INLINE));
  if (constp > 1)
    warning ("duplicate `const'");
  if (volatilep > 1)
    warning ("duplicate `volatile'");
  virtualp = specbits & (1 << (int) RID_VIRTUAL);
  if (specbits & (1 << (int) RID_STATIC))
    staticp = 1 + (decl_context == FIELD);
  if (virtualp && staticp == 2)
    {
      error ("member `%s' cannot be declared both virtual and static", name);
      staticp = 0;
    }
  friendp = specbits & (1 << (int) RID_FRIEND);
  specbits &= ~ ((1 << (int) RID_VIRTUAL) | (1 << (int) RID_FRIEND));
  if (specbits)
    {
      if (specbits & 1 << (int) RID_STATIC) nclasses++;
      if (specbits & 1 << (int) RID_EXTERN) nclasses++;
      if (decl_context == PARM && nclasses > 0)
	error ("storage class specifiers invalid in parameter declarations");
      if (specbits & 1 << (int) RID_TYPEDEF)
	{
	  if (decl_context == PARM)
	    error ("typedef declaration invalid in parameter declaration");
	  nclasses++;
	}
      if (specbits & 1 << (int) RID_AUTO) nclasses++;
      if (specbits & 1 << (int) RID_REGISTER) nclasses++;
    }
  if (virtualp && current_class_name == (tree) 0)
    {
      error ("virtual outside class declaration");
      virtualp = 0;
    }
  if (nclasses > 1)
    error ("multiple storage classes in declaration of `%s'", name);
  else if (decl_context != NORMAL && nclasses > 0)
    {
      if (decl_context == PARM
	  && ((specbits & (1 << (int) RID_REGISTER)) | (1 << (int) RID_AUTO)))
	;
      else if (decl_context == FIELD && (specbits & (1 << (int) RID_TYPEDEF)))
	{
	  tree loc_typedecl;
	  register int i = sizeof (struct lang_decl_flags) / sizeof (int);
	  register int *pi;
	  pushlevel (0);
	  loc_typedecl = start_decl (declarator, declspecs, initialized, (tree) 0);
	  pi = (int *) permalloc (sizeof (struct lang_decl_flags));
	  while (i > 0)
	    pi[--i] = 0;
	  ((loc_typedecl)->decl.lang_specific) = (struct lang_decl *) pi;
	  poplevel (0, 0, 0);
	  if (  (((struct lang_identifier *)(((loc_typedecl)->decl.name)))->class_value))
	    error_with_decl (loc_typedecl,
			     "typedef of `%s' in class scope hides previous declaration");
	  return loc_typedecl;
	}
      else if (decl_context == FIELD
 	       && (specbits & (1 << (int) RID_STATIC)))
	;
      else
	{
	  if (decl_context == FIELD)
	    {
	      tree tmp = ((declarator)->exp.operands[ 0]);
	      register int op = ((tmp)->common.lang_flag_2);
	      error ("storage class specified for %s `%s'",
		     op ? "member operator" : "structure field",
		     op ? operator_name_string (tmp) : name);
	    }
	  else
	    error ((decl_context == PARM
		    ? "storage class specified for parameter `%s'"
		    : "storage class specified for typename"), name);
	  specbits &= ~ ((1 << (int) RID_REGISTER) | (1 << (int) RID_AUTO)
			 | (1 << (int) RID_EXTERN));
	}
    }
  else if (specbits & 1 << (int) RID_EXTERN && initialized && !funcdef_flag)
    {
      if (current_binding_level == global_binding_level)
	warning ("`%s' initialized and declared `extern'", name);
      else
	error ("`%s' has both `extern' and initializer", name);
    }
  else if (specbits & 1 << (int) RID_EXTERN && funcdef_flag
	   && current_binding_level != global_binding_level)
    error ("nested function `%s' declared `extern'", name);
  else if (current_binding_level == global_binding_level)
    {
      if (specbits & (1 << (int) RID_AUTO))
	error ("top-level declaration of `%s' specifies `auto'", name);
    }
  while (declarator && ((enum tree_code) (declarator)->common.code) != IDENTIFIER_NODE)
    {
      if (((enum tree_code) (type)->common.code) == ERROR_MARK)
	{
	  if (((enum tree_code) (declarator)->common.code) == SCOPE_REF)
	    declarator = ((declarator)->exp.operands[ 1]);
	  else
	    declarator = ((declarator)->exp.operands[ 0]);
	  continue;
	}
      if (quals != (tree) 0
	  && (declarator == (tree) 0
	      || ((enum tree_code) (declarator)->common.code) != SCOPE_REF))
	{
	  if (ctype == (tree) 0 && ((enum tree_code) (type)->common.code) == METHOD_TYPE)
	    ctype = ((type)->type.maxval);
	  if (ctype != (tree) 0)
	    {
	      tree dummy = build_decl (TYPE_DECL, (tree) 0, type);
	      ctype = grok_method_quals (ctype, dummy, quals);
	      type = ((dummy)->common.type);
	      quals = (tree) 0;
	    }
	}
      switch (((enum tree_code) (declarator)->common.code))
	{
	case ARRAY_REF:
	  maybe_globalize_type (type);
	  {
	    register tree itype = (tree) 0;
	    register tree size = ((declarator)->exp.operands[ 1]);
	    declarator = ((declarator)->exp.operands[ 0]);
	    if (((type)->type.main_variant) == void_type_node)
	      {
		error ("declaration of `%s' as array of voids", name);
		type = error_mark_node;
	      }
	    if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
	      {
		error ("declaration of `%s' as array of functions", name);
		type = error_mark_node;
	      }
	    if (size == error_mark_node)
	      type = error_mark_node;
	    if (type == error_mark_node)
	      continue;
	    if (size)
	      {
		int yes = suspend_momentary ();
		if (((enum tree_code) (size)->common.code) == NOP_EXPR
		    && ((size)->common.type) == ((((size)->exp.operands[ 0]))->common.type))
		  size = ((size)->exp.operands[ 0]);
		if (((enum tree_code) (size)->common.code) == TEMPLATE_CONST_PARM)
		  goto dont_grok_size;
		if (((enum tree_code) (((size)->common.type))->common.code) != INTEGER_TYPE
		    && ((enum tree_code) (((size)->common.type))->common.code) != ENUMERAL_TYPE)
		  {
		    error ("size of array `%s' has non-integer type", name);
		    size = integer_one_node;
		  }
		if (  (((size)->common.readonly_flag) && (*tree_code_type[(int) (((enum tree_code) (size)->common.code))]) == 'd'))
		  size = decl_constant_value (size);
		if (pedantic && integer_zerop (size))
		  pedwarn ("ANSI C++ forbids zero-size array `%s'", name);
		if (((size)->common.constant_flag))
		  {
		    if ((((size)->int_cst.int_cst_high) < (( integer_zero_node)->int_cst.int_cst_high)			 || (((size)->int_cst.int_cst_high) == (( integer_zero_node)->int_cst.int_cst_high)		     && ((unsigned int) ((size)->int_cst.int_cst_low)			 < (unsigned int) (( integer_zero_node)->int_cst.int_cst_low)))))
		      {
			error ("size of array `%s' is negative", name);
			size = integer_one_node;
		      }
		    itype = build_index_type (size_binop (MINUS_EXPR, size,
							  integer_one_node));
		  }
		else
		  {
		    if (pedantic)
		      pedwarn ("ANSI C++ forbids variable-size array `%s'", name);
		  dont_grok_size:
		    itype = build_binary_op (MINUS_EXPR, size, integer_one_node);
		    size_varies = 1;
		    itype = variable_size (itype);
		    itype = build_index_type (itype);
		  }
		resume_momentary (yes);
	      }
	    if (constp || volatilep)
	      type = build_type_variant (type, constp, volatilep);
	    type = build_cplus_array_type (type, itype);
	    ctype = (tree) 0;
	  }
	  break;
	case CALL_EXPR:
	  maybe_globalize_type (type);
	  {
	    tree arg_types;
	    if (constp || volatilep)
	      {
		type = build_type_variant (type, constp, volatilep);
		if ((((type)->type.lang_flag_5)))
		  build_pointer_type (type);
		constp = 0;
		volatilep = 0;
	      }
	    if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
	      {
		error ("`%s' declared as function returning a function", name);
		type = integer_type_node;
	      }
	    if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
	      {
		error ("`%s' declared as function returning an array", name);
		type = integer_type_node;
	      }
	    if (ctype == (tree) 0
		&& decl_context == FIELD
		&& (friendp == 0 || dname == current_class_name))
	      ctype = current_class_type;
	    if (ctype && flags == TYPENAME_FLAG)
	      (((ctype)->type.lang_specific)->type_flags.has_type_conversion) = 1;
	    if (ctype && constructor_name (ctype) == dname)
	      {
		if (flags == DTOR_FLAG)
		  {
		    if (staticp == 2)
		      error ("destructor cannot be static member function");
		    if (((type)->common.readonly_flag))
		      {
			error ("destructors cannot be declared `const'");
			return void_type_node;
		      }
		    if (((type)->common.volatile_flag))
		      {
			error ("destructors cannot be declared `volatile'");
			return void_type_node;
		      }
		    if (decl_context == FIELD)
		      {
			if (! member_function_or_else (ctype, current_class_type,
						       "destructor for alien class `%s' cannot be a member"))
			  return void_type_node;
		      }
		  }
		else            
		  {
		    if (staticp == 2)
		      error ("constructor cannot be static member function");
		    if (virtualp)
		      {
			pedwarn ("constructors cannot be declared virtual");
			virtualp = 0;
		      }
		    if (((type)->common.readonly_flag))
		      {
			error ("constructors cannot be declared `const'");
			return void_type_node;
 		      }
		    if (((type)->common.volatile_flag))
		      {
			error ("constructors cannot be declared `volatile'");
			return void_type_node;
		      }
		    if (specbits & ~((1 << (int) RID_INLINE)|(1 << (int) RID_STATIC)))
		      error ("return value type specifier for constructor ignored");
		    type = ((ctype)->type.pointer_to);
		    if (decl_context == FIELD)
		      {
			if (! member_function_or_else (ctype, current_class_type,
						       "constructor for alien class `%s' cannot be member"))
			  return void_type_node;
			(((ctype)->type.lang_flag_1)) = 1;
			my_friendly_assert (return_type == return_ctor, 156);
		      }
		  }
		if (decl_context == FIELD)
		  staticp = 0;
	      }
	    else if (friendp && virtualp)
	      {
		error ("virtual functions cannot be friends");
		specbits ^= (1 << (int) RID_FRIEND);
	      }
	    if (decl_context == NORMAL && friendp)
	      error ("friend declaration not in class definition");
	    quals = ((declarator)->exp.operands[ 2]);
	    if (flag_traditional
		&& ((type)->type.main_variant) == float_type_node)
	      {
		type = build_type_variant (double_type_node,
					   ((type)->common.readonly_flag),
					   ((type)->common.volatile_flag));
	      }
	    {
	      int funcdef_p;
	      tree inner_parms = ((declarator)->exp.operands[ 1]);
	      tree inner_decl = ((declarator)->exp.operands[ 0]);
	      declarator = ((declarator)->exp.operands[ 0]);
	      if (inner_decl && ((enum tree_code) (inner_decl)->common.code) == SCOPE_REF)
		inner_decl = ((inner_decl)->exp.operands[ 1]);
	      funcdef_p =
		(inner_decl &&
		 (((enum tree_code) (inner_decl)->common.code) == IDENTIFIER_NODE
		  || ((enum tree_code) (inner_decl)->common.code) == TYPE_EXPR)) ? funcdef_flag : 0;
	      arg_types = grokparms (inner_parms, funcdef_p);
	    }
	    if (declarator)
	      {
		if (((enum tree_code) (declarator)->common.code) == BIT_NOT_EXPR)
		  {
		    declarator = ((declarator)->exp.operands[ 0]);
		    if (strict_prototype == 0 && arg_types == (tree) 0)
		      arg_types = void_list_node;
		    else if (arg_types == (tree) 0
			     || arg_types != void_list_node)
		      {
			error ("destructors cannot be specified with parameters");
			arg_types = void_list_node;
		      }
		  }
	      }
	    if (((type)->common.readonly_flag) || ((type)->common.volatile_flag))
	      {
		int constp = ((type)->common.readonly_flag);
		int volatilep = ((type)->common.volatile_flag);
		type = build_function_type (((type)->type.main_variant),
					    flag_traditional
					    ? 0
					    : arg_types);
		type = build_type_variant (type, constp, volatilep);
	      }
	    else
	      type = build_function_type (type,
					  flag_traditional
					  ? 0
					  : arg_types);
	  }
	  break;
	case ADDR_EXPR:
	case INDIRECT_REF:
	  maybe_globalize_type (type);
	  if (((enum tree_code) (type)->common.code) == REFERENCE_TYPE)
	    {
	      error ("cannot declare %s to references",
		     ((enum tree_code) (declarator)->common.code) == ADDR_EXPR
		     ? "references" : "pointers");
	      declarator = ((declarator)->exp.operands[ 0]);
	      continue;
	    }
	  if (constp || volatilep)
	    {
	      type = build_type_variant (type, constp, volatilep);
	      if ((((type)->type.lang_flag_5)))
		build_pointer_type (type);
	      constp = 0;
	      volatilep = 0;
	    }
	  if (((enum tree_code) (declarator)->common.code) == ADDR_EXPR)
	    {
	      if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
		{
		  error ("cannot declare references to functions; use pointer to function instead");
		  type = build_pointer_type (type);
		}
	      else
		{
		  if (((type)->type.main_variant) == void_type_node)
		    error ("invalid type: `void &'");
		  else
		    type = build_reference_type (type);
		}
	    }
	  else
	    type = build_pointer_type (type);
	  if (((declarator)->common.type))
	    {
	      register tree typemodlist;
	      int erred = 0;
	      for (typemodlist = ((declarator)->common.type); typemodlist;
		   typemodlist = ((typemodlist)->common.chain))
		{
		  if (((typemodlist)->list.value) == ridpointers[(int) RID_CONST])
		    constp++;
		  else if (((typemodlist)->list.value) == ridpointers[(int) RID_VOLATILE])
		    volatilep++;
		  else if (!erred)
		    {
		      erred = 1;
		      error ("invalid type modifier within %s declarator",
			     ((enum tree_code) (declarator)->common.code) == ADDR_EXPR
			     ? "reference" : "pointer");
		    }
		}
	      if (constp > 1)
		warning ("duplicate `const'");
	      if (volatilep > 1)
		warning ("duplicate `volatile'");
	    }
	  declarator = ((declarator)->exp.operands[ 0]);
	  ctype = (tree) 0;
	  break;
	case SCOPE_REF:
	  {
	    tree sname = ((declarator)->exp.operands[ 1]);
	    if (((enum tree_code) (sname)->common.code) == BIT_NOT_EXPR)
	      sname = ((sname)->exp.operands[ 0]);
	    if (((declarator)->exp.complexity) == 0)
 ;
	    else if (friendp && (((declarator)->exp.complexity) < 2))
	       ;
	    else if (((declarator)->exp.complexity) == current_class_depth)
	      {
		((declarator)->exp.complexity) -= 1;
		popclass (1);
	      }
	    else
	      my_friendly_abort (16);
	    if (((declarator)->exp.operands[ 0]) == (tree) 0)
	      {
		declarator = sname;
		continue;
	      }
	    ctype = ((declarator)->exp.operands[ 0]);
	    if (sname == (tree) 0)
	      goto done_scoping;
	    if (((enum tree_code) (sname)->common.code) == IDENTIFIER_NODE)
	      {
		if (((ctype)->type.main_variant) == current_class_type || friendp)
		  {
		    if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
		      type = build_cplus_method_type (build_type_variant (ctype, constp, volatilep),
						      ((type)->common.type), ((type)->type.values));
		    else
		      {
			if (((ctype)->type.main_variant) != current_class_type)
			  {
			    error ("cannot declare member `%s::%s' within this class",
				   ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)), name);
			    return void_type_node;
			  }
			else if (extra_warnings)
			  warning ("extra qualification `%s' on member `%s' ignored",
				   ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)), name);
			type = build_offset_type (ctype, type);
		      }
		  }
		else if (((ctype)->type.size) != 0
			 || (specbits & (1<<(int)RID_TYPEDEF)))
		  {
		    tree t;
		    if (explicit_int == -1 && decl_context == FIELD
			&& funcdef_flag == 0)
		      {
			t = lookup_field (ctype, sname, 0, 0);
			if (t)
			  {
			    t = build_lang_field_decl (FIELD_DECL, build_nt (SCOPE_REF, ctype, t), type);
			    ((t)->decl.initial) = init;
			    return t;
			  }
			t = lookup_fnfields (((ctype)->type.binfo), sname, 0);
			if (t)
			  {
			    if (flags == DTOR_FLAG)
			      t = ((t)->list.value);
			    else if (((ctype)->type.maxval)
				     && ((t)->list.value) == ((((ctype)->type.maxval))->vec.a[ 0]))
			      {
				t = (((((t)->list.value))->decl.lang_specific)->chain);
				if (t == (tree) 0)
				  error ("class `%s' does not have any constructors", ((sname)->identifier.pointer));
				t = build_tree_list ((tree) 0, t);
			      }
			    t = build_lang_field_decl (FIELD_DECL, build_nt (SCOPE_REF, ctype, t), type);
			    ((t)->decl.initial) = init;
			    return t;
			  }
			if (flags == TYPENAME_FLAG)
			  error_with_aggr_type (ctype, "type conversion is not a member of structure `%s'");
			else
			  error ("field `%s' is not a member of structure `%s'",
				 ((sname)->identifier.pointer),
				 ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)));
		      }
		    if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
		      type = build_cplus_method_type (build_type_variant (ctype, constp, volatilep),
						      ((type)->common.type), ((type)->type.values));
		    else
		      {
			if (current_class_type)
			  {
			    if (((ctype)->type.main_variant) != current_class_type)
			      {
				error ("cannot declare member `%s::%s' within this class",
				       ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)), name);
			      return void_type_node;
			      }
			    else if (extra_warnings)
			      warning ("extra qualification `%s' on member `%s' ignored",
				       ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)), name);
			  }
			type = build_offset_type (ctype, type);
		      }
		  }
		else if (uses_template_parms (ctype))
		  {
                    enum tree_code c;
                    if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
		      {
			type = build_cplus_method_type (build_type_variant (ctype, constp, volatilep),
							((type)->common.type),
							((type)->type.values));
			c = FUNCTION_DECL;
		      }
  		  }
		else
		  sorry ("structure `%s' not yet defined",
			 ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)));
		declarator = sname;
	      }
	    else if (((enum tree_code) (sname)->common.code) == TYPE_EXPR)
	      {
		sname = grokoptypename (sname, 0);
		my_friendly_assert (((enum tree_code) (sname)->common.code) == CALL_EXPR, 157);
		type = ((((sname)->exp.operands[ 0]))->common.type);
		((declarator)->exp.operands[ 1]) = ((sname)->exp.operands[ 0]);
		((sname)->exp.operands[ 0]) = declarator;
		declarator = sname;
		continue;
	      }
	    else if (((enum tree_code) (sname)->common.code) == SCOPE_REF)
	      my_friendly_abort (17);
	    else
	      {
	      done_scoping:
		declarator = ((declarator)->exp.operands[ 1]);
		if (declarator && ((enum tree_code) (declarator)->common.code) == CALL_EXPR)
		  ;
		else
		  {
		    if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
		      type = build_cplus_method_type (build_type_variant (ctype, constp, volatilep), ((type)->common.type), ((type)->type.values));
		    else
		      type = build_offset_type (ctype, type);
		  }
	      }
	  }
	  break;
	case BIT_NOT_EXPR:
	  declarator = ((declarator)->exp.operands[ 0]);
	  break;
	case TYPE_EXPR:
	  declarator = grokoptypename (declarator, 0);
	  if (explicit_int != -1)
	    if (comp_target_types (type, ((((declarator)->exp.operands[ 0]))->common.type), 1) == 0)
	      error ("type conversion function declared to return incongruent type");
	    else
	      pedwarn ("return type specified for type conversion function");
	  type = ((((declarator)->exp.operands[ 0]))->common.type);
	  maybe_globalize_type (type);
	  break;
	case RECORD_TYPE:
	case UNION_TYPE:
	case ENUMERAL_TYPE:
	  declarator = 0;
	  break;
	case ERROR_MARK:
	  declarator = 0;
	  break;
	default:
	  my_friendly_assert (0, 158);
	}
    }
  if (specbits & (1 << (int) RID_TYPEDEF))
    {
      tree decl;
      if (constp || volatilep)
	type = build_type_variant (type, constp, volatilep);
      if (((type)->type.name)
	  && ((enum tree_code) (((type)->type.name))->common.code) == TYPE_DECL
	  && ((((((((type)->type.name))->decl.name)))->identifier.pointer)[0] == '$' 				  && (((((((type)->type.name))->decl.name)))->identifier.pointer)[1] == '_'))
	{
	  lookup_tag_reverse (type, declarator);
	  (((((type)->type.name))->decl.name)) = declarator;
	}
      decl = build_decl (TYPE_DECL, declarator, type);
      if (quals)
	{
	  if (ctype == (tree) 0)
	    {
	      if (((enum tree_code) (type)->common.code) != METHOD_TYPE)
		error_with_decl (decl, "invalid type qualifier for non-method type");
	      else
		ctype = ((type)->type.maxval);
	    }
	  if (ctype != (tree) 0)
	    grok_method_quals (ctype, decl, quals);
	}
      if ((specbits & (1 << (int) RID_SIGNED))
	  || (typedef_decl && (((typedef_decl))->decl.lang_flag_1)))
	(((decl))->decl.lang_flag_1) = 1;
      return decl;
    }
  if (type == typedef_type && ((enum tree_code) (type)->common.code) == ARRAY_TYPE
      && ((type)->type.values) == 0)
    {
      type = build_cplus_array_type (((type)->common.type), ((type)->type.values));
    }
  if (decl_context == TYPENAME)
    {
      if (constp || volatilep)
	type = build_type_variant (type, constp, volatilep);
      if (friendp)
	{
	  if (current_class_type)
	    make_friend_class (current_class_type, ((type)->type.main_variant));
	  else
	    error("trying to make class `%s' a friend of global scope",
		  ((((((((type)->type.name))->decl.name)))->identifier.pointer)));
	  type = void_type_node;
	}
      else if (quals)
	{
	  tree dummy = build_decl (TYPE_DECL, declarator, type);
	  if (ctype == (tree) 0)
	    {
	      my_friendly_assert (((enum tree_code) (type)->common.code) == METHOD_TYPE, 159);
	      ctype = ((type)->type.maxval);
	    }
	  grok_method_quals (ctype, dummy, quals);
	  type = ((dummy)->common.type);
	}
      return type;
    }
  if (((type)->type.main_variant) == void_type_node && decl_context != PARM)
    {
      if (declarator != (tree) 0
	  && ((enum tree_code) (declarator)->common.code) == IDENTIFIER_NODE)
	error ("variable or field `%s' declared void", name);
      else
	error ("variable or field declared void");
      type = integer_type_node;
    }
  {
    register tree decl;
    if (decl_context == PARM)
      {
	tree parmtype = type;
	if (ctype)
	  error ("cannot use `::' in parameter declaration");
	if (virtualp)
	  error ("parameter declared `virtual'");
	if (quals)
	  error ("`const' and `volatile' function specifiers invalid in parameter declaration");
	if (friendp)
	  error ("invalid friend declaration");
	if (raises)
	  error ("invalid raises declaration");
	if (((enum tree_code) (type)->common.code) == REFERENCE_TYPE)
	  type = ((type)->common.type);
	if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
	  {
	    if (parmtype == type)
	      {
		type = build_pointer_type
		  (build_type_variant (((type)->common.type), constp, volatilep));
		volatilep = constp = 0;
	      }
	    else
	      type = build_pointer_type (((type)->common.type));
	  }
	else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
	  type = build_pointer_type (type);
	else if (((enum tree_code) (type)->common.code) == OFFSET_TYPE)
	  type = build_pointer_type (type);
	if (((enum tree_code) (parmtype)->common.code) == REFERENCE_TYPE)
	  {
	    type = build_type_variant (build_reference_type (type), constp, volatilep);
	    constp = volatilep = 0;
	  }
	decl = build_decl (PARM_DECL, declarator, type);
	((decl)->decl.initial)    = type;
	if (((type)->type.main_variant) == float_type_node)
	  ((decl)->decl.initial)    = build_type_variant (double_type_node,
						     ((type)->common.readonly_flag),
						     ((type)->common.volatile_flag));
	else if (  (((enum tree_code) ((type))->common.code) == INTEGER_TYPE				   && (((type)->type.main_variant) == char_type_node			       || ((type)->type.main_variant) == signed_char_type_node	       || ((type)->type.main_variant) == unsigned_char_type_node	       || ((type)->type.main_variant) == short_integer_type_node	       || ((type)->type.main_variant) == short_unsigned_type_node)))
	  {
	    tree argtype;
	    if (((type)->common.unsigned_flag)
		&& (flag_traditional
		    || ((type)->type.precision)
			== ((integer_type_node)->type.precision)))
	      argtype = unsigned_type_node;
	    else
	      argtype = integer_type_node;
	    ((decl)->decl.initial)    = build_type_variant (argtype,
						       ((type)->common.readonly_flag),
						       ((type)->common.volatile_flag));
	  }
      }
    else if (decl_context == FIELD)
      {
	if (type == error_mark_node)
	  {
	    decl = (tree) 0;
	  }
	else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
	  {
	    if (friendp == 0)
	      {
		if (ctype == (tree) 0)
		  ctype = current_class_type;
		if (ctype == (tree) 0)
		  {
		    register int op = ((declarator)->common.lang_flag_2);
		    error ("can't make %s `%s' into a method -- not in a class",
			   op ? "operator" : "function",
			   op ? operator_name_string (declarator) : ((declarator)->identifier.pointer));
		    return void_type_node;
		  }
		if (virtualp && ((enum tree_code) (ctype)->common.code) == UNION_TYPE)
		  {
		    error ("function `%s' declared virtual inside a union",
			   ((declarator)->identifier.pointer));
		    return void_type_node;
		  }
		if (staticp < 2
		    && declarator != ansi_opname[(int) NEW_EXPR]
		    && declarator != ansi_opname[(int) DELETE_EXPR])
		  type = build_cplus_method_type (build_type_variant (ctype, constp, volatilep),
						  ((type)->common.type), ((type)->type.values));
	      }
	    decl = grokfndecl (ctype, type, declarator, virtualp, flags, quals, raises, friendp ? -1 : 0);
	    ((decl)->decl.inline_flag) = inlinep;
	    if ((specbits & (1 << (int) RID_EXTERN))
		|| (ctype != (tree) 0 && funcdef_flag >= 0)
		|| (friendp
                    && !(specbits & (1 << (int) RID_STATIC))
                    && !(specbits & (1 << (int) RID_INLINE))))
	      ((decl)->common.public_flag) = 1;
	  }
	else if (((enum tree_code) (type)->common.code) == METHOD_TYPE)
	  {
	    decl = grokfndecl (ctype, type, declarator, virtualp, flags, quals, raises, friendp ? -1 : 0);
	    ((decl)->decl.inline_flag) = inlinep;
	    ((decl)->common.public_flag) = 1;
	  }
	else if (((enum tree_code) (type)->common.code) == RECORD_TYPE
		 && (((type)->type.lang_specific)->type_flags.declared_exception))
	  {
	    decl = build_lang_field_decl (VAR_DECL, declarator, type);
	    if (ctype == (tree) 0)
	      ctype = current_class_type;
	    finish_exception_decl (((enum tree_code) (((ctype)->type.name))->common.code) == TYPE_DECL
				   ? (((((ctype)->type.name))->decl.name)) : ((ctype)->type.name), decl);
	    return void_type_node;
	  }
	else if (((type)->type.size) == 0 && !staticp
		 && (((enum tree_code) (type)->common.code) != ARRAY_TYPE || initialized == 0))
	  {
	    if (declarator)
	      error ("field `%s' has incomplete type",
		     ((declarator)->identifier.pointer));
	    else
	      error ("field has incomplete type");
	    type = error_mark_node;
	    decl = (tree) 0;
	  }
	else
	  {
	    if (friendp)
	      {
		if (declarator)
		  error ("`%s' is neither function nor method; cannot be declared friend",
			 ((declarator)->identifier.pointer));
		else
		  {
		    error ("invalid friend declaration");
		    return void_type_node;
		  }
		friendp = 0;
	      }
	    decl = (tree) 0;
	  }
	if (friendp)
	  {
	    if (ctype == current_class_type)
	      warning ("member functions are implicitly friends of their class");
	    else if (decl && ((decl)->decl.name))
	      return do_friend (ctype, declarator, decl,
				last_function_parms, flags, quals);
	    else return void_type_node;
	  }
	if (decl == 0)
	  {
	    if (virtualp)
	      error ("field declared `virtual'");
	    if (quals)
	      error ("`const' and `volatile' function specifiers invalid in field declaration");
	    if (friendp)
	      error ("invalid friend declaration");
	    if (raises)
	      error ("invalid raises declaration");
	    if (initialized)
	      {
		if (staticp)
		  error ("static member `%s' must be defined separately from its declaration",
			  ((declarator)->identifier.pointer));
		else if (!constp || pedantic)
		  error ("ANSI C++ forbids initialization of %s `%s'",
			 (constp && pedantic) ? "const member" : "member",
			 ((declarator)->identifier.pointer));
	      }
	    if (staticp || (constp && initialized))
	      {
		decl = build_lang_field_decl (VAR_DECL, declarator, type);
		if (staticp || ((enum tree_code) (type)->common.code) == ARRAY_TYPE)
		  ((decl)->common.static_flag) = 1;
		((decl)->common.public_flag) = 1;
		((decl)->decl.external_flag) = !initialized;
	      }
	    else
	      decl = build_lang_field_decl (FIELD_DECL, declarator, type);
	  }
      }
    else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE || ((enum tree_code) (type)->common.code) == METHOD_TYPE)
      {
	int was_overloaded = 0;
	tree original_name = declarator;
	if (! declarator) return (tree) 0;
	if (specbits & ((1 << (int) RID_AUTO) | (1 << (int) RID_REGISTER)))
	  error ("invalid storage class for function `%s'", name);
	if (current_binding_level != global_binding_level
	    && (specbits & ((1 << (int) RID_STATIC) | (1 << (int) RID_INLINE)))
	    && pedantic)
	  pedwarn ("invalid storage class for function `%s'", name);
	if (ctype == (tree) 0)
	  {
	    if (virtualp)
	      {
		error ("virtual non-class function `%s'", name);
		virtualp = 0;
	      }
	    if (current_lang_name == lang_name_cplusplus
		&& ! (((original_name)->identifier.length) == 4
		      && ((original_name)->identifier.pointer)[0] == 'm'
		      && strcmp (((original_name)->identifier.pointer), "main") == 0)
		&& ! (((original_name)->identifier.length) > 10
		      && ((original_name)->identifier.pointer)[0] == '_'
		      && ((original_name)->identifier.pointer)[1] == '_'
		      && strncmp (((original_name)->identifier.pointer)+2, "builtin_", 8) == 0))
	      {
		declarator = build_decl_overload (dname, ((type)->type.values), 0);
		was_overloaded = 1;
	      }
	  }
	else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE && staticp < 2)
	  type = build_cplus_method_type (build_type_variant (ctype, constp, volatilep),
					  ((type)->common.type), ((type)->type.values));
	decl = grokfndecl (ctype, type, original_name, virtualp, flags, quals,
			   raises,
			   processing_template_decl ? 0 : friendp ? 2 : 1);
	if (ctype == (tree) 0)
	  ((decl)->decl.assembler_name) = declarator;
	if (staticp == 1)
	  {
	    int illegal_static = 0;
	    if (((enum tree_code) (type)->common.code) == METHOD_TYPE)
	      {
		error_with_decl (decl,
				 "cannot declare member function `%s' to have static linkage");
		illegal_static = 1;
	      }
	    else if (! was_overloaded
		     && ! ctype
		     && ! strcmp (((original_name)->identifier.pointer), "main"))
	      {
		error ("cannot declare function `main' to have static linkage");
		illegal_static = 1;
	      }
	    if (illegal_static)
	      {
		staticp = 0;
		specbits &= ~(1 << (int)RID_STATIC);
	      }
	  }
	((decl)->common.public_flag)
	  = ((ctype
	      && ! (((ctype)->type.lang_specific)->type_flags.interface_unknown)
	      && ! (((ctype)->type.lang_specific)->type_flags.interface_only))
	    || !(specbits & ((1 << (int) RID_STATIC)
			     | (1 << (int) RID_INLINE))));
	if (inlinep)
	  {
	    tree last = tree_last (((type)->type.values));
	    if (! was_overloaded
		&& ! ctype
		&& ! strcmp (((original_name)->identifier.pointer), "main"))
	      warning ("cannot inline function `main'");
	    else if (last && last != void_list_node)
	      warning ("inline declaration ignored for function with `...'");
	    else
	      ((decl)->decl.inline_flag) = 1;
	    if (specbits & (1 << (int) RID_EXTERN))
	      {
		current_extern_inline = 1;
		if (pedantic)
		  error ("ANSI C++ does not permit `extern inline'");
		else if (flag_ansi)
		  warning ("ANSI C++ does not permit `extern inline'");
	      }
	  }
	if (was_overloaded)
	  (((decl)->decl.lang_flag_4)) = 1;
      }
    else
      {
	if (virtualp)
	  error ("variable declared `virtual'");
	if (inlinep)
	  warning ("variable declared `inline'");
	if (quals)
	  error ("`const' and `volatile' function specifiers invalid in field declaration");
	if (friendp)
	  error ("invalid friend declaration");
	if (raises)
	  error ("invalid raises declaration");
	decl = grokvardecl (ctype, type, declarator, specbits, initialized);
	if (ctype)
	  {
	    if (staticp == 1)
	      {
	        error ("cannot declare member `%s' to have static linkage",
		       lang_printable_name (decl));
	        staticp = 0;
	        specbits &= ~(1 << (int)RID_STATIC);
	      }
	    if (specbits & 1 << (int) RID_EXTERN)
	      {
	        error ("cannot explicitly declare member `%s' to have extern linkage",
		       lang_printable_name (decl));
	        specbits &= ~(1 << (int)RID_EXTERN);
	      }
	  }
      }
    if (specbits & (1 << (int) RID_REGISTER))
      ((decl)->decl.regdecl_flag) = 1;
    if (constp)
      ((decl)->common.readonly_flag) = ((enum tree_code) (type)->common.code) != REFERENCE_TYPE;
    if (volatilep)
      {
	((decl)->common.side_effects_flag) = 1;
	((decl)->common.volatile_flag) = 1;
      }
    return decl;
  }
}
int
parmlist_is_exprlist (exprs)
     tree exprs;
{
  if (exprs == (tree) 0 || ((exprs)->common.unsigned_flag) )
    return 0;
  if (current_binding_level == global_binding_level)
    {
      while (exprs)
	{
	  if (((enum tree_code) (((exprs)->list.value))->common.code) != IDENTIFIER_NODE)
	    return 1;
	  exprs = ((exprs)->common.chain);
	}
      return 0;
    }
  return 1;
}
static int
parmlist_is_random (parms)
     tree parms;
{
  if (parms == (tree) 0)
    return 0;
  if (((enum tree_code) (parms)->common.code) != TREE_LIST)
    return 1;
  while (parms)
    {
      if (parms == void_list_node)
	return 0;
      if (((enum tree_code) (((parms)->list.value))->common.code) != TREE_LIST)
	return 1;
      if (((((parms)->list.value))->common.type) == unknown_type_node)
	return 1;
      parms = ((parms)->common.chain);
    }
  return 0;
}
static void
require_complete_types_for_parms (parms)
     tree parms;
{
  while (parms)
    {
      tree type = ((parms)->common.type);
      if (((type)->type.size) == 0)
	{
	  if (((parms)->decl.name))
	    error ("parameter `%s' has incomplete type",
		   ((((parms)->decl.name))->identifier.pointer));
	  else
	    error ("parameter has incomplete type");
	  ((parms)->common.type) = error_mark_node;
	}
      parms = ((parms)->common.chain);
    }
}
static tree
grokparms (first_parm, funcdef_flag)
     tree first_parm;
     int funcdef_flag;
{
  tree result = (tree) 0;
  tree decls = (tree) 0;
  if (first_parm != 0
      && ((enum tree_code) (((first_parm)->list.value))->common.code) == IDENTIFIER_NODE)
    {
      if (! funcdef_flag)
	warning ("parameter names (without types) in function declaration");
      last_function_parms = first_parm;
      return 0;
    }
  else
    {
      register tree parm, chain;
      int any_init = 0, any_error = 0, saw_void = 0;
      if (first_parm != (tree) 0)
	{
	  tree last_result = (tree) 0;
	  tree last_decl = (tree) 0;
	  for (parm = first_parm; parm != (tree) 0; parm = chain)
	    {
	      tree type, list_node = parm;
	      register tree decl = ((parm)->list.value);
	      tree init = ((parm)->list.purpose);
	      chain = ((parm)->common.chain);
	      if (decl != void_type_node && ((enum tree_code) (decl)->common.code) != TREE_LIST)
		{
		  if (((enum tree_code) (decl)->common.code) == STRING_CST)
		    error ("invalid string constant `%s'",
			   ((decl)->string.pointer));
		  else if (((enum tree_code) (decl)->common.code) == INTEGER_CST)
		    error ("invalid integer constant in parameter list, did you forget to give parameter name?");
		  continue;
		}
	      if (decl != void_type_node)
		{
		  decl = grokdeclarator (((decl)->list.value),
					 ((decl)->list.purpose),
					 PARM, init != (tree) 0, (tree) 0);
		  if (! decl)
		    continue;
		  type = ((decl)->common.type);
		  if (((type)->type.main_variant) == void_type_node)
		    decl = void_type_node;
		  else if (((enum tree_code) (type)->common.code) == METHOD_TYPE)
		    {
		      if (((decl)->decl.name))
			error ("parameter `%s' invalidly declared method type",
			       ((((decl)->decl.name))->identifier.pointer));
		      else
			error ("parameter invalidly declared method type");
		      type = build_pointer_type (type);
		      ((decl)->common.type) = type;
		    }
		  else if (((enum tree_code) (type)->common.code) == OFFSET_TYPE)
		    {
		      if (((decl)->decl.name))
			error ("parameter `%s' invalidly declared offset type",
			       ((((decl)->decl.name))->identifier.pointer));
		      else
			error ("parameter invalidly declared offset type");
		      type = build_pointer_type (type);
		      ((decl)->common.type) = type;
		    }
                  else if (((enum tree_code) (type)->common.code) == RECORD_TYPE
                           && ((type)->type.lang_specific)
                           && (((type)->type.lang_specific)->abstract_virtuals))
                    {
                      abstract_virtuals_error (decl, type);
                      any_error = 1;  
                    }
		}
	      if (decl == void_type_node)
		{
		  if (result == (tree) 0)
		    {
		      result = void_list_node;
		      last_result = result;
		    }
		  else
		    {
		      ((last_result)->common.chain) = void_list_node;
		      last_result = void_list_node;
		    }
		  saw_void = 1;
		  if (chain
		      && (chain != void_list_node || ((chain)->common.chain)))
		    error ("`void' in parameter list must be entire list");
		  break;
		}
	      ((decl)->decl.initial)    = ((decl)->common.type);
	      if (!any_error)
		{
		  if (init)
		    {
		      any_init++;
		      if (((enum tree_code) (init)->common.code) == SAVE_EXPR)
			(((init)->common.lang_flag_2)) = 1;
		      else if (((enum tree_code) (init)->common.code) == VAR_DECL)
			{
			  if (  (((struct lang_identifier *)(((init)->decl.name)))->local_value))
			    {
			      error_with_decl (init, "local variable `%s' may not be used as a default argument");
			      any_error = 1;
			    }
			  else if (  (((init)->common.readonly_flag) && (*tree_code_type[(int) (((enum tree_code) (init)->common.code))]) == 'd'))
			    init = decl_constant_value (init);
			}
		      else
			init = require_instantiated_type (type, init, integer_zero_node);
		    }
		  else if (any_init)
		    {
		      error ("all trailing parameters must have default arguments");
		      any_error = 1;
		    }
		}
	      else
		init = (tree) 0;
	      if (decls == (tree) 0)
		{
		  decls = decl;
		  last_decl = decls;
		}
	      else
		{
		  ((last_decl)->common.chain) = decl;
		  last_decl = decl;
		}
	      if (((list_node)->common.permanent_flag))
		{
		  ((list_node)->list.purpose) = init;
		  ((list_node)->list.value) = type;
		  ((list_node)->common.chain) = 0;
		}
	      else
		list_node = saveable_tree_cons (init, type, (tree) 0);
	      if (result == (tree) 0)
		{
		  result = list_node;
		  last_result = result;
		}
	      else
		{
		  ((last_result)->common.chain) = list_node;
		  last_result = list_node;
		}
	    }
	  if (last_result)
	    ((last_result)->common.chain) = (tree) 0;
	  if (last_decl != (tree) 0)
	    ((last_decl)->common.chain) = (tree) 0;
	}
    }
  last_function_parms = decls;
  if (funcdef_flag > 0)
    require_complete_types_for_parms (last_function_parms);
  return result;
}
void
grok_ctor_properties (ctype, decl)
     tree ctype, decl;
{
  tree parmtypes = (((((((decl)->common.type))->type.values))->common.chain));
  tree parmtype = parmtypes ? ((parmtypes)->list.value) : void_type_node;
  if (parmtypes && ((parmtypes)->common.chain)
      && ((enum tree_code) (((((parmtypes)->common.chain))->list.value))->common.code) == REFERENCE_TYPE
      && (((((((((parmtypes)->common.chain))->list.value))->common.type))->common.lang_flag_3)))
    {
      parmtypes = ((parmtypes)->common.chain);
      parmtype = ((parmtypes)->list.value);
    }
  if (((enum tree_code) (parmtype)->common.code) == REFERENCE_TYPE
      && ((((parmtype)->common.type))->type.main_variant) == ctype)
    {
      if (((parmtypes)->common.chain) == (tree) 0
	  || ((parmtypes)->common.chain) == void_list_node
	  || ((((parmtypes)->common.chain))->list.purpose))
	{
	  (((ctype)->type.lang_specific)->type_flags.has_init_ref) = 1;
	  (((ctype)->type.lang_specific)->type_flags.gets_init_ref) = 1;
	  if (((((parmtype)->common.type))->common.readonly_flag))
	    (((ctype)->type.lang_specific)->type_flags.gets_const_init_ref) = 1;
	}
      else
	(((ctype)->type.lang_specific)->type_flags.gets_init_aggr) = 1;
    }
  else if (((parmtype)->type.main_variant) == ctype)
    {
      if (((parmtypes)->common.chain) != (tree) 0
	  && ((parmtypes)->common.chain) == void_list_node)
	error ("invalid constructor; you probably meant `%s (%s&)'",
	       ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)),
	       ((((((((ctype)->type.name))->decl.name)))->identifier.pointer)));
        (((struct lang_identifier *)(((decl)->decl.name)))->x == 0 ? ((struct lang_identifier *)(((decl)->decl.name)))->x = (struct lang_id2 *)perm_calloc (1, sizeof (struct lang_id2)) : 0,    ((struct lang_identifier *)(((decl)->decl.name)))->x->error_locus = ( ctype));
      (((ctype)->type.lang_specific)->type_flags.gets_init_aggr) = 1;
    }
  else if (((enum tree_code) (parmtype)->common.code) == VOID_TYPE
	   || ((parmtypes)->list.purpose) != (tree) 0)
    (((ctype)->type.lang_specific)->type_flags.has_default_ctor) = 1;
}
static void
grok_op_properties (decl, virtualp)
     tree decl;
     int virtualp;
{
  tree argtypes = ((((decl)->common.type))->type.values);
  if ((((decl)->decl.lang_specific)->decl_flags.static_function))
    {
      if (((decl)->decl.name) == ansi_opname[(int) NEW_EXPR])
	{
	  if (virtualp)
	    error ("`operator new' cannot be declared virtual");
	  if (argtypes == (tree) 0)
	    ((decl)->common.type) =
	      build_function_type (ptr_type_node,
				   hash_tree_chain (integer_type_node,
						    void_list_node));
	  else
	    decl = coerce_new_type (((current_class_name)->common.type),
				    ((decl)->common.type));
	}
      else if (((decl)->decl.name) == ansi_opname[(int) DELETE_EXPR])
	{
	  if (virtualp)
	    error ("`operator delete' cannot be declared virtual");
	  if (argtypes == (tree) 0)
	    ((decl)->common.type) =
	      build_function_type (void_type_node,
				   hash_tree_chain (ptr_type_node,
						    void_list_node));
	  else
	    decl = coerce_delete_type (((current_class_name)->common.type),
				       ((decl)->common.type));
	}
      else
	error_with_decl (decl, "`%s' cannot be a static member function");
    }
  else if (((decl)->decl.name) == ansi_opname[(int) MODIFY_EXPR])
    {
      tree parmtypes;
      tree parmtype;
      if (argtypes == (tree) 0)
	{
	  error_with_decl (decl, "too few arguments to `%s'");
	  return;
	}
      parmtypes = ((argtypes)->common.chain);
      parmtype = parmtypes ? ((parmtypes)->list.value) : void_type_node;
      if (((enum tree_code) (parmtype)->common.code) == REFERENCE_TYPE
	  && ((parmtype)->common.type) == current_class_type)
	{
	  (((current_class_type)->type.lang_specific)->type_flags.has_assign_ref) = 1;
	  (((current_class_type)->type.lang_specific)->type_flags.gets_assign_ref) = 1;
	  if (((((parmtype)->common.type))->common.readonly_flag))
	    (((current_class_type)->type.lang_specific)->type_flags.gets_const_init_ref) = 1;
	}
    }
}
static int xref_next_defn = 0;
tree
xref_defn_tag (code_type_node, name, binfo)
     tree code_type_node;
     tree name, binfo;
{
  tree rv, ncp;
  xref_next_defn = 1;
  if (class_binding_level)
    {
      tree n1;
      char *buf;
      n1 =   (((struct lang_identifier *)(current_class_name))->local_value);
      if (n1)
	n1 = ((n1)->decl.name);
      else
	n1 = current_class_name;
      buf = (char *) alloca (4 + ((n1)->identifier.length)
			     + ((name)->identifier.length));
      sprintf (buf, "%s::%s", ((n1)->identifier.pointer),
	       ((name)->identifier.pointer));
      ncp = get_identifier (buf);
      rv = xref_tag (code_type_node, name, binfo);
      pushdecl_top_level (build_lang_decl (TYPE_DECL, ncp, rv));
    }
  else
    {
      rv = xref_tag (code_type_node, name, binfo);
    }
  xref_next_defn = 0;
  return rv;
}
tree
xref_tag (code_type_node, name, binfo)
     tree code_type_node;
     tree name, binfo;
{
  enum tag_types tag_code;
  enum tree_code code;
  int temp = 0;
  int i, len;
  register tree ref;
  struct binding_level *b
    = (class_binding_level ? class_binding_level : current_binding_level);
  tag_code = (enum tag_types) ((code_type_node)->int_cst.int_cst_low);
  switch (tag_code)
    {
    case record_type:
    case class_type:
    case exception_type:
      code = RECORD_TYPE;
      len = list_length (binfo);
      break;
    case union_type:
      code = UNION_TYPE;
      if (binfo)
	{
	  error ("derived union `%s' invalid", ((name)->identifier.pointer));
	  binfo = (tree) 0;
	}
      len = 0;
      break;
    case enum_type:
      code = ENUMERAL_TYPE;
      break;
    default:
      my_friendly_abort (18);
    }
  if (xref_next_defn)
    {
      xref_next_defn = 0;
      ref = lookup_tag (code, name, b, 1);
    }
  else
    {
      ref = lookup_tag (code, name, b, 0);
      if (! ref)
	{
	  ref = lookup_name (name, 1);
	  if (ref && ((enum tree_code) (ref)->common.code) == TYPE_DECL
	      && ((enum tree_code) (((ref)->common.type))->common.code) == code)
	    ref = ((ref)->common.type);
	  else
	    ref = (tree) 0;
	}
    }
  push_obstacks_nochange ();
  if (! ref)
    {
      temp = allocation_temporary_p ();
      if (temp)
	end_temporary_allocation ();
      if (code == ENUMERAL_TYPE)
	{
	  ref = make_node (ENUMERAL_TYPE);
	  ((ref)->type.mode) = ((unsigned_type_node)->type.mode);
	  ((ref)->type.align) = ((unsigned_type_node)->type.align);
	  ((ref)->common.unsigned_flag) = 1;
	  ((ref)->type.precision) = ((unsigned_type_node)->type.precision);
	  ((ref)->type.minval) = ((unsigned_type_node)->type.minval);
	  ((ref)->type.maxval) = ((unsigned_type_node)->type.maxval);
	  pushtag (name, ref);
	  if (flag_cadillac)
	    cadillac_start_enum (ref);
	}
      else if (tag_code == exception_type)
	{
	  ref = make_lang_type (code);
	  (((ref)->type.lang_specific)->type_flags.declared_exception) = 1;
	  pushtag (name, ref);
	  if (flag_cadillac)
	    cadillac_start_struct (ref);
	}
      else
	{
	  extern tree pending_vtables;
	  struct binding_level *old_b = class_binding_level;
	  int needs_writing;
	  ref = make_lang_type (code);
	  switch (write_virtuals)
	    {
	    case 0:
	    case 1:
	      needs_writing = 1;
	      break;
	    case 2:
	      needs_writing = !! value_member (name, pending_vtables);
	      break;
	    case 3:
	      needs_writing
		= ! ((((ref)->type.lang_specific)->type_flags.interface_only) || (((ref)->type.lang_specific)->type_flags.interface_unknown));
	      break;
	    default:
	      needs_writing = 0;
	    }
	  (((ref)->type.lang_specific)->type_flags.vtable_needs_writing) = needs_writing;
	  pushtag (name, ref);
	  class_binding_level = old_b;
	  if (flag_cadillac)
	    cadillac_start_struct (ref);
	}
    }
  else
    {
      if (  (code == RECORD_TYPE || code == UNION_TYPE))
	{
	  if ((((ref)->type.lang_flag_5))
	      && ((tag_code == exception_type)
		  != ((((ref)->type.lang_specific)->type_flags.declared_exception) == 1)))
	    {
	      error ("type `%s' is both exception and aggregate type",
		     ((name)->identifier.pointer));
	      (((ref)->type.lang_specific)->type_flags.declared_exception) = (tag_code == exception_type);
	    }
	}
      if (b == global_binding_level && !class_binding_level
	  &&   (((struct lang_identifier *)(name))->global_value) == (tree) 0)
	  (((struct lang_identifier *)(name))->global_value) = ((ref)->type.name);
      if (binfo)
	{
	  tree tt1 = binfo;
	  tree tt2 = ((((ref)->type.binfo))->vec.a[ 4]);
	  if (((((ref)->type.binfo))->vec.a[ 4]))
	    for (i = 0; tt1; i++, tt1 = ((tt1)->common.chain))
	      if (((tt1)->list.value) != (((((((((tt2)->vec.a[ i]))->common.type))->type.name))->decl.name)))
		{
		  error ("redeclaration of derivation chain of type `%s'",
			 ((name)->identifier.pointer));
		  break;
		}
	  if (tt1 == (tree) 0)
	    goto just_return;
	  end_temporary_allocation ();
	}
    }
  if (binfo)
    {
      tree binfos;
      ((((ref)->type.lang_specific)->type_flags.marked) = 1);
      (((((ref)->type.binfo)))->vec.a[ 4]) = binfos = make_tree_vec (len);
      for (i = 0; binfo; binfo = ((binfo)->common.chain))
	{
	  int via_public = (tag_code != class_type
			    || ((binfo)->list.purpose) == (tree)visibility_public
			    || ((binfo)->list.purpose) == (tree)visibility_public_virtual);
	  int via_protected = ((binfo)->list.purpose) == (tree)visibility_protected;
	  int via_virtual = (((binfo)->list.purpose) == (tree)visibility_private_virtual
			     || ((binfo)->list.purpose) == (tree)visibility_public_virtual
			     || ((binfo)->list.purpose) == (tree)visibility_default_virtual);
	  tree basetype = ((((binfo)->list.value))->common.type);
	  tree base_binfo;
	  GNU_xref_hier (((name)->identifier.pointer),
			 ((((binfo)->list.value))->identifier.pointer),
			 via_public, via_virtual, 0);
	  if (basetype && ((enum tree_code) (basetype)->common.code) == TYPE_DECL)
	    basetype = ((basetype)->common.type);
	  if (!basetype || ((enum tree_code) (basetype)->common.code) != RECORD_TYPE)
	    {
	      error ("base type `%s' fails to be a struct or class type",
		     ((((binfo)->list.value))->identifier.pointer));
	      continue;
	    }
	  else
	    {
	      if ((((basetype)->type.lang_specific)->type_flags.marked))
		{
		  if (basetype == ref)
		    error_with_aggr_type (basetype, "recursive type `%s' undefined");
		  else
		    error_with_aggr_type (basetype, "duplicate base type `%s' invalid");
		  continue;
		}
	      base_binfo = make_binfo (integer_zero_node, basetype,
				  (((((basetype)->type.binfo)))->vec.a[ 2]),
 				  (((((basetype)->type.binfo)))->vec.a[ 3]), 0);
	      ((binfos)->vec.a[ i]) = base_binfo;
	      ((base_binfo)->common.public_flag) = via_public;
 	      ((base_binfo)->common.protected_flag) = via_protected;
	      ((base_binfo)->common.static_flag) = via_virtual;
	      ((((basetype)->type.lang_specific)->type_flags.marked) = 1);
	      if (via_virtual || (((basetype)->common.lang_flag_3)))
		{
		  (((ref)->common.lang_flag_3)) = 1;
		  (((ref)->common.lang_flag_1)) = 1;
		}
	      (((ref)->type.lang_specific)->type_flags.gets_assignment) |= (((basetype)->type.lang_specific)->type_flags.gets_assignment);
	      (((ref)->type.lang_specific)->type_flags.has_method_call_overloaded) |= (((basetype)->type.lang_specific)->type_flags.has_method_call_overloaded);
	      (((ref)->type.lang_specific)->type_flags.gets_new) |= (((basetype)->type.lang_specific)->type_flags.gets_new);
	      (((ref)->type.lang_specific)->type_flags.gets_delete) |= (((basetype)->type.lang_specific)->type_flags.gets_delete);
	      (((ref)->type.lang_specific)->type_flags.local_typedecls) |= (((basetype)->type.lang_specific)->type_flags.local_typedecls);
	      i += 1;
	    }
	}
      if (i)
	((binfos)->vec.length) = i;
      else
	(((((ref)->type.binfo)))->vec.a[ 4]) = (tree) 0;
      if (i > 1)
	(((ref)->type.lang_specific)->type_flags.uses_multiple_inheritance) = 1;
      else if (i == 1)
	(((ref)->type.lang_specific)->type_flags.uses_multiple_inheritance)
	  = (((((((binfos)->vec.a[ 0]))->common.type))->type.lang_specific)->type_flags.uses_multiple_inheritance);
      if ((((ref)->type.lang_specific)->type_flags.uses_multiple_inheritance))
	(((ref)->common.lang_flag_1)) = 1;
      while (--i >= 0)
	((((((((binfos)->vec.a[ i]))->common.type))->type.lang_specific)->type_flags.marked) = 0);
      ((((ref)->type.lang_specific)->type_flags.marked) = 0);
    }
 just_return:
  if (((ref)->type.size) == (tree) 0
      && ref != current_class_type
      &&   (((enum tree_code) (ref)->common.code) == RECORD_TYPE || ((enum tree_code) (ref)->common.code) == UNION_TYPE))
    {
      if (tag_code == class_type)
	(((ref)->type.lang_specific)->type_flags.declared_class) = 1;
      else if (tag_code == record_type)
	(((ref)->type.lang_specific)->type_flags.declared_class) = 0;
    }
  pop_obstacks ();
  return ref;
}
tree
start_enum (name)
     tree name;
{
  register tree enumtype = 0;
  struct binding_level *b
    = (class_binding_level ? class_binding_level : current_binding_level);
  if (name != 0)
    enumtype = lookup_tag (ENUMERAL_TYPE, name, b, 1);
  if (enumtype == 0 || ((enum tree_code) (enumtype)->common.code) != ENUMERAL_TYPE)
    {
      enumtype = make_node (ENUMERAL_TYPE);
      pushtag (name, enumtype);
    }
  if (((enumtype)->type.values) != 0)
    {
      error ("redeclaration of `enum %s'", ((name)->identifier.pointer));
      ((enumtype)->type.values) = 0;
    }
  ((enumtype)->type.precision) = ((integer_type_node)->type.precision);
  ((enumtype)->type.size) = 0;
  fixup_unsigned_type (enumtype);
  enum_next_value = copy_node (integer_zero_node);
  GNU_xref_decl (current_function_decl, enumtype);
  return enumtype;
}
tree
finish_enum (enumtype, values)
     register tree enumtype, values;
{
  register tree pair;
  register int maxvalue = 0;
  register int minvalue = 0;
  register int i;
  ((enumtype)->type.values) = values;
  if (values)
    {
      int value = ((((values)->list.value))->int_cst.int_cst_low);
      ((((values)->list.value))->common.type) = enumtype;
      minvalue = maxvalue = value;
      for (pair = ((values)->common.chain); pair; pair = ((pair)->common.chain))
	{
	  value = ((((pair)->list.value))->int_cst.int_cst_low);
	  if (value > maxvalue)
	    maxvalue = value;
	  else if (value < minvalue)
	    minvalue = value;
	  ((((pair)->list.value))->common.type) = enumtype;
	}
    }
  if (flag_short_enums)
    {
      for (i = maxvalue; i; i >>= 1)
	((enumtype)->type.precision)++;
      if (!((enumtype)->type.precision))
	((enumtype)->type.precision) = 1;
      ((enumtype)->type.size) = 0;
      fixup_unsigned_type (enumtype);
    }
  ((((enumtype)->type.maxval))->int_cst.int_cst_low) = maxvalue;
  if (minvalue < 0)
    {
      ((((enumtype)->type.minval))->int_cst.int_cst_low) = minvalue;
      ((((enumtype)->type.minval))->int_cst.int_cst_high) = -1;
      ((enumtype)->common.unsigned_flag) = 0;
    }
  if (flag_cadillac)
    cadillac_finish_enum (enumtype);
    rest_of_type_compilation (enumtype, global_bindings_p ());
  return enumtype;
}
tree
build_enumerator (name, value)
     tree name, value;
{
  tree decl, result;
  int shareable = 1;
  if (value != 0)
    {
      if (  (((value)->common.readonly_flag) && (*tree_code_type[(int) (((enum tree_code) (value)->common.code))]) == 'd'))
	{
	  value = decl_constant_value (value);
	  shareable = 0;
	}
      if (((enum tree_code) (value)->common.code) != INTEGER_CST)
	{
	  error ("enumerator value for `%s' not integer constant",
		 ((name)->identifier.pointer));
	  value = 0;
	}
    }
  if (value == 0)
    value = enum_next_value;
  if (value)
      while ((((enum tree_code) (value)->common.code) == NOP_EXPR					  || ((enum tree_code) (value)->common.code) == CONVERT_EXPR				  || ((enum tree_code) (value)->common.code) == NON_LVALUE_EXPR)			 && (((value)->common.type)						     == ((((value)->exp.operands[ 0]))->common.type)))		    (value) = ((value)->exp.operands[ 0]);;
  if (value == integer_zero_node)
    value =   build_int_2_wide ((int) (0), (int) ( 0));
  else if (value == integer_one_node)
    value =   build_int_2_wide ((int) (1), (int) ( 0));
  else if (((enum tree_code) (value)->common.code) == INTEGER_CST
	   && (shareable == 0
	       || ((enum tree_code) (((value)->common.type))->common.code) == ENUMERAL_TYPE))
    {
      value = copy_node (value);
      ((value)->common.type) = integer_type_node;
    }
  result = saveable_tree_cons (name, value, (tree) 0);
  if (current_class_type == (tree) 0 || current_function_decl != (tree) 0)
    {
      decl = build_decl (CONST_DECL, name, integer_type_node);
      ((decl)->decl.initial) = value;
      pushdecl (decl);
      GNU_xref_decl (current_function_decl, decl);
    }
  enum_next_value = build_binary_op_nodefault (PLUS_EXPR, value,
					       integer_one_node, PLUS_EXPR);
  if (enum_next_value == integer_one_node)
    enum_next_value = copy_node (enum_next_value);
  return result;
}
tree
grok_enum_decls (type, decl)
     tree type, decl;
{
  struct binding_level *b = class_binding_level;
  tree tag = (tree) 0;
  tree values;
  while (b)
    {
      tag = value_member (type, b->tags);
      if (tag)
	break;
      b = b->level_chain;
    }
  if (b == 0 || (b != class_binding_level) || ((tag)->common.addressable_flag))
    return decl;
  else
    ((tag)->common.addressable_flag) = 1;
  values = ((type)->type.values);
  while (values)
    {
      tree next = build_lang_field_decl (CONST_DECL, ((values)->list.purpose), type);
      ((next)->common.readonly_flag) = 1;
      ((next)->decl.initial) = ((values)->list.value);
      ((next)->common.chain) = decl;
      decl = next;
      pushdecl_class_level (decl);
      values = ((values)->common.chain);
    }
  return decl;
}
int
start_function (declspecs, declarator, raises, pre_parsed_p)
     tree declarator, declspecs, raises;
     int pre_parsed_p;
{
  extern tree EHS_decl;
  tree decl1, olddecl;
  tree ctype = (tree) 0;
  tree fntype;
  tree restype;
  extern int have_extern_spec;
  extern int used_extern_spec;
  int doing_friend = 0;
  if (flag_handle_exceptions && EHS_decl == (tree) 0)
    init_exception_processing_1 ();
  my_friendly_assert (((void_list_node)->list.value) == void_type_node, 160);
  my_friendly_assert (((void_list_node)->common.chain) == (tree) 0, 161);
  current_function_returns_value = 0;
  current_function_returns_null = 0;
  warn_about_return_type = 0;
  current_extern_inline = 0;
  current_function_assigns_this = 0;
  current_function_just_assigned_this = 0;
  current_function_parms_stored = 0;
  original_result_rtx = 0;
  current_function_obstack_index = 0;
  current_function_obstack_usage = 0;
  clear_temp_name ();
  if (have_extern_spec && !used_extern_spec)
    {
      declspecs = decl_tree_cons ((tree) 0, get_identifier ("extern"), declspecs);
      used_extern_spec = 1;
    }
  if (pre_parsed_p)
    {
      decl1 = declarator;
      last_function_parms = ((decl1)->decl.arguments);
      last_function_parm_tags = 0;
      fntype = ((decl1)->common.type);
      if (((enum tree_code) (fntype)->common.code) == METHOD_TYPE)
	ctype = ((fntype)->type.maxval);
      if (!ctype && (((decl1)->decl.lang_specific)->decl_flags.friend_attr))
	{
	  ctype = ((((decl1)->common.chain))->common.type);
	  if (((enum tree_code) (ctype)->common.code) != RECORD_TYPE)
	    ctype = (tree) 0;
	  else
	    doing_friend = 1;
	}
      if ( !(((decl1)->decl.vindex)
	     && write_virtuals >= 2
	     && (((ctype)->type.lang_specific)->type_flags.vtable_needs_writing)))
	current_extern_inline = ((decl1)->common.public_flag) && ((decl1)->decl.inline_flag);
      raises = ((fntype)->type.noncopied_parts);
      require_complete_types_for_parms (last_function_parms);
    }
  else
    {
      decl1 = grokdeclarator (declarator, declspecs, FUNCDEF, 1, raises);
      if (decl1 == 0 || ((enum tree_code) (decl1)->common.code) != FUNCTION_DECL) return 0;
      fntype = ((decl1)->common.type);
      restype = ((fntype)->common.type);
      if ((((restype)->type.lang_flag_5))
	  && ! (((restype)->type.lang_specific)->type_flags.got_semicolon))
	{
	  error_with_aggr_type (restype, "semicolon missing after declaration of `%s'");
	  shadow_tag (build_tree_list ((tree) 0, restype));
	  (((restype)->type.lang_specific)->type_flags.got_semicolon) = 1;
	  if (((enum tree_code) (fntype)->common.code) == FUNCTION_TYPE)
	    fntype = build_function_type (integer_type_node,
					  ((fntype)->type.values));
	  else
	    fntype = build_cplus_method_type (build_type_variant (((fntype)->type.maxval), ((decl1)->common.readonly_flag), ((decl1)->common.side_effects_flag)),
					      integer_type_node,
					      ((fntype)->type.values));
	  ((decl1)->common.type) = fntype;
	}
      if (((enum tree_code) (fntype)->common.code) == METHOD_TYPE)
	ctype = ((fntype)->type.maxval);
      else if (((((decl1)->decl.name))->identifier.length) == 4
	       && ! strcmp (((((decl1)->decl.name))->identifier.pointer), "main")
	       && ((decl1)->decl.context) == (tree) 0)
	{
	  if (((((decl1)->common.type))->common.type) != integer_type_node)
	    {
	      warning ("return type for `main' changed to integer type");
	      ((decl1)->common.type) = fntype = default_function_type;
	    }
	  warn_about_return_type = 0;
	}
    }
  if (! warn_implicit &&   (((struct lang_identifier *)(((decl1)->decl.name)))->x    ? ((struct lang_identifier *)(((decl1)->decl.name)))->x->implicit_decl : 0) != 0)
    warning_with_decl (  (((struct lang_identifier *)(((decl1)->decl.name)))->x    ? ((struct lang_identifier *)(((decl1)->decl.name)))->x->implicit_decl : 0),
		       "`%s' implicitly declared before its definition");
  current_function_decl = decl1;
  if (flag_cadillac)
    cadillac_start_function (decl1);
  else
    announce_function (decl1);
  if (((((fntype)->common.type))->type.size) == 0)
    {
      if ((((((fntype)->common.type))->type.lang_flag_5)))
	error_with_aggr_type (((fntype)->common.type),
			      "return-type `%s' is an incomplete type");
      else
	error ("return-type is an incomplete type");
      if (ctype)
	((decl1)->common.type)
	  = build_cplus_method_type (build_type_variant (ctype, ((decl1)->common.readonly_flag), ((decl1)->common.side_effects_flag)),
				     void_type_node,
				     (((((((decl1)->common.type))->type.values))->common.chain)));
      else
	((decl1)->common.type)
	  = build_function_type (void_type_node,
				 ((((decl1)->common.type))->type.values));
      ((decl1)->decl.result) = build_decl (RESULT_DECL, 0, ((fntype)->common.type));
    }
  if (warn_about_return_type)
    warning ("return-type defaults to `int'");
  ((decl1)->decl.initial) = error_mark_node;
  olddecl = 0;
  ((decl1)->common.static_flag) = 1;
  if (interface_unknown == 0)
    {
      ((decl1)->common.public_flag) = 1;
      ((decl1)->decl.external_flag) = interface_only;
    }
  else
    ((decl1)->decl.external_flag) = current_extern_inline;
  if (ctype == (tree) 0 && current_lang_name == lang_name_cplusplus)
    {
      olddecl = lookup_name_current_level (((decl1)->decl.name));
      if (olddecl && ((enum tree_code) (olddecl)->common.code) != FUNCTION_DECL)
	olddecl = (tree) 0;
      if (olddecl && ((decl1)->decl.name) != ((olddecl)->decl.name))
	{
	  olddecl = lookup_name_current_level (((decl1)->decl.assembler_name));
	  if (olddecl == (tree) 0)
	    olddecl = decl1;
	}
      if (olddecl && olddecl != decl1
	  && ((decl1)->decl.name) == ((olddecl)->decl.name))
	{
	  if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	      && (decls_match (decl1, olddecl)
		  || comp_target_parms (((((decl1)->common.type))->type.values),
					((((olddecl)->common.type))->type.values), 1)))
	    {
	      olddecl = (((olddecl)->decl.lang_specific)->main_decl_variant);
	      ((decl1)->decl.assembler_name) = ((olddecl)->decl.assembler_name);
	      (((decl1)->decl.lang_flag_4)) = (((olddecl)->decl.lang_flag_4));
	      if (((olddecl)->decl.initial))
		redeclaration_error_message (decl1, olddecl);
	      if (! duplicate_decls (decl1, olddecl))
		my_friendly_abort (19);
	      decl1 = olddecl;
	    }
	  else
	    olddecl = (tree) 0;
	}
    }
  if (olddecl)
    current_function_decl = olddecl;
  else if (pre_parsed_p == 0)
    {
      current_function_decl = pushdecl (decl1);
      if (((enum tree_code) (current_function_decl)->common.code) == TREE_LIST)
	{
	  decl1 = (((decl1)->decl.lang_specific)->main_decl_variant);
	  current_function_decl = decl1;
	}
      else
	{
	  decl1 = current_function_decl;
	  (((decl1)->decl.lang_specific)->main_decl_variant) = decl1;
	}
      fntype = ((decl1)->common.type);
    }
  else
    current_function_decl = decl1;
  if ((((decl1)->decl.lang_flag_4)))
    decl1 = push_overloaded_decl (decl1, 1);
  if (ctype != 0 && (((decl1)->decl.lang_specific)->decl_flags.static_function))
    {
      if (((enum tree_code) (fntype)->common.code) == METHOD_TYPE)
	((decl1)->common.type) = fntype
	  = build_function_type (((fntype)->common.type),
				 ((((fntype)->type.values))->common.chain));
      last_function_parms = ((last_function_parms)->common.chain);
      ((decl1)->decl.arguments) = last_function_parms;
      ctype = 0;
    }
  restype = ((fntype)->common.type);
  pushlevel (0);
  current_binding_level->parm_flag = 1;
  current_function_parms = last_function_parms;
  current_function_parm_tags = last_function_parm_tags;
  GNU_xref_function (decl1, current_function_parms);
  make_function_rtl (decl1);
  if (ctype)
    {
      pushclass (ctype, 1);
      if (! doing_friend)
	{
	  current_class_decl = last_function_parms;
	  my_friendly_assert (current_class_decl != (tree) 0
		  && ((enum tree_code) (current_class_decl)->common.code) == PARM_DECL, 162);
	  if (((enum tree_code) (((current_class_decl)->common.type))->common.code) == POINTER_TYPE)
	    {
	      tree variant = ((((current_class_decl)->common.type))->common.type);
	      if ((((ctype)->type.lang_specific)->instance_variable) == (tree) 0)
		{
		  C_C_D = build1 (INDIRECT_REF, current_class_type, current_class_decl);
		  (((ctype)->type.lang_specific)->instance_variable) = C_C_D;
		}
	      else
		{
		  C_C_D = (((ctype)->type.lang_specific)->instance_variable);
		  ((C_C_D)->exp.operands[ 0]) = current_class_decl;
		}
	      ((C_C_D)->common.readonly_flag) = ((variant)->common.readonly_flag);
	      ((C_C_D)->common.side_effects_flag) = ((variant)->common.volatile_flag);
	      ((C_C_D)->common.volatile_flag) = ((variant)->common.volatile_flag);
	    }
	  else
	    C_C_D = current_class_decl;
	}
    }
  else
    {
      if ((((decl1)->decl.lang_specific)->decl_flags.static_function))
	pushclass (((decl1)->decl.context), 2);
      else
	push_memoized_context (0, 1);
    }
  temporary_allocation ();
  if (  (((enum tree_code) ((restype))->common.code) == INTEGER_TYPE				   && (((restype)->type.main_variant) == char_type_node			       || ((restype)->type.main_variant) == signed_char_type_node	       || ((restype)->type.main_variant) == unsigned_char_type_node	       || ((restype)->type.main_variant) == short_integer_type_node	       || ((restype)->type.main_variant) == short_unsigned_type_node)))
    {
      if (((restype)->common.unsigned_flag)
	  && (flag_traditional
	      || ((restype)->type.precision)
		   == ((integer_type_node)->type.precision)))
	restype = unsigned_type_node;
      else
	restype = integer_type_node;
    }
  if (((decl1)->decl.result) == (tree) 0)
    ((decl1)->decl.result) = build_decl (RESULT_DECL, 0, restype);
  if ((((((decl1)->decl.assembler_name))->identifier.pointer)[1] == '$'))
    {
      dtor_label = build_decl (LABEL_DECL, (tree) 0, (tree) 0);
      ctor_label = (tree) 0;
    }
  else
    {
      dtor_label = (tree) 0;
      if ((((decl1)->decl.lang_specific)->decl_flags.constructor_attr))
	ctor_label = build_decl (LABEL_DECL, (tree) 0, (tree) 0);
    }
  if (((((decl1)->decl.assembler_name))->common.addressable_flag))
    ((decl1)->common.addressable_flag) = 1;
  return 1;
}
void
store_parm_decls ()
{
  register tree fndecl = current_function_decl;
  register tree parm;
  int parms_have_cleanups = 0;
  tree eh_decl;
  tree specparms = current_function_parms;
  tree parmtags = current_function_parm_tags;
  tree nonparms = 0;
  if (current_binding_level == global_binding_level)
    fatal ("parse errors have confused me too much");
  init_function_start (fndecl, input_filename, lineno);
  declare_function_name ();
  expand_start_bindings (0);
  if (flag_handle_exceptions)
    {
      setup_exception_throw_decl ();
      eh_decl = current_binding_level->names;
      current_binding_level->names = ((current_binding_level->names)->common.chain);
    }
  if (flag_handle_exceptions)
    expand_start_try (integer_one_node, 0, 1);
  if (specparms != 0)
    {
      register tree next;
      storedecls ((tree) 0);
      for (parm = nreverse (specparms); parm; parm = next)
	{
	  next = ((parm)->common.chain);
	  if (((enum tree_code) (parm)->common.code) == PARM_DECL)
	    {
	      tree cleanup = maybe_build_cleanup (parm);
	      if (((parm)->decl.name) == 0)
		{
		  pushdecl (parm);
		}
	      else if (((((parm)->common.type))->type.main_variant) == void_type_node)
		error_with_decl (parm, "parameter `%s' declared void");
	      else
		{
		  if (((enum tree_code) (((parm)->common.type))->common.code) == REFERENCE_TYPE
		      && ((((((parm)->common.type))->common.type))->type.size))
		    ((parm)->decl.arguments= convert_from_reference (parm));
		  pushdecl (parm);
		}
	      if (cleanup)
		{
		  expand_decl (parm);
		  expand_decl_cleanup (parm, cleanup);
		  parms_have_cleanups = 1;
		}
	    }
	  else
	    {
	      ((parm)->common.chain) = 0;
	      nonparms = chainon (nonparms, parm);
	    }
	}
      ((fndecl)->decl.arguments) = getdecls ();
      storetags (chainon (parmtags, gettags ()));
    }
  else
    ((fndecl)->decl.arguments) = 0;
  storedecls (chainon (nonparms, ((fndecl)->decl.arguments)));
  ((fndecl)->decl.saved_insns.r) = 0;
  expand_function_start (fndecl, parms_have_cleanups);
  if (flag_handle_exceptions)
    {
      pushdecl (eh_decl);
      expand_decl_init (eh_decl);
    }
  if (parms_have_cleanups)
    {
      pushlevel (0);
      expand_start_bindings (0);
    }
  current_function_parms_stored = 1;
  if (flag_gc)
    {
      maybe_gc_cleanup = build_tree_list ((tree) 0, error_mark_node);
      expand_decl_cleanup ((tree) 0, maybe_gc_cleanup);
    }
  if (((fndecl)->decl.name)
      && ((((fndecl)->decl.name))->identifier.length) == 4
      && strcmp (((((fndecl)->decl.name))->identifier.pointer), "main") == 0
      && ((fndecl)->decl.context) == (tree) 0)
    {
      expand_main_function ();
      if (flag_gc)
	expand_expr (build_function_call (lookup_name (get_identifier ("__gc_main"), 0), (tree) 0),
		     0, VOIDmode, 0);
      if (flag_dossier)
	output_builtin_tdesc_entries ();
    }
}
void
store_return_init (return_id, init)
     tree return_id, init;
{
  tree decl = ((current_function_decl)->decl.result);
  if (pedantic)
    error ("ANSI C++ does not permit named return values");
  if (return_id != (tree) 0)
    {
      if (((decl)->decl.name) == 0)
	{
	  ((decl)->decl.name) = return_id;
	  ((decl)->decl.assembler_name) = return_id;
	}
      else
	error ("return identifier `%s' already in place",
	       ((((decl)->decl.name))->identifier.pointer));
    }
  if ((((current_function_decl)->decl.lang_specific)->decl_flags.constructor_attr))
    {
      error ("can't redefine default return value for constructors");
      return;
    }
  if (((decl)->decl.name) != 0)
    {
      if (((decl)->decl.regdecl_flag))
	{
	  original_result_rtx = ((decl)->decl.rtl);
	  ((decl)->decl.rtl) = gen_reg_rtx (((decl)->decl.mode));
	}
      ((decl)->decl.initial) = init;
      pushdecl (decl);
      finish_decl (decl, init, 0, 0);
    }
}
static void
build_default_constructor (fndecl)
     tree fndecl;
{
  int i =   (((((current_class_type)->type.binfo))->vec.a[ 4]) ? ((((((current_class_type)->type.binfo))->vec.a[ 4]))->vec.length) : 0);
  tree parm = ((((fndecl)->decl.arguments))->common.chain);
  tree fields = ((current_class_type)->type.values);
  tree binfos = ((((current_class_type)->type.binfo))->vec.a[ 4]);
  if ((((current_class_type)->common.lang_flag_3)))
    parm = ((parm)->common.chain);
  parm = ((tree)(parm)->decl.arguments);
  while (--i >= 0)
    {
      tree basetype = ((binfos)->vec.a[ i]);
      if ((((basetype)->type.lang_specific)->type_flags.gets_init_ref))
	{
	  tree name = ((basetype)->type.name);
	  if (((enum tree_code) (name)->common.code) == TYPE_DECL)
	    name = ((name)->decl.name);
	  current_base_init_list = tree_cons (name, parm, current_base_init_list);
	}
    }
  for (; fields; fields = ((fields)->common.chain))
    {
      tree name, init;
      if (((fields)->common.static_flag))
	continue;
      if (((enum tree_code) (fields)->common.code) != FIELD_DECL)
	continue;
      if (((fields)->decl.name))
	{
	  if ((!strncmp (((((fields)->decl.name))->identifier.pointer), "_vptr$", sizeof("_vptr$")-1)))
	    continue;
	  if ((((((fields)->decl.name))->identifier.pointer)[3] == '$'   && ((((fields)->decl.name))->identifier.pointer)[2] == 'b'  && ((((fields)->decl.name))->identifier.pointer)[1] == 'v'))
	    continue;
	  if (  (((struct lang_identifier *)(((fields)->decl.name)))->class_value) != fields)
	    continue;
	}
      init = build (COMPONENT_REF, ((fields)->common.type), parm, fields);
      if (((fields)->decl.regdecl_flag) )
	name = build (COMPONENT_REF, ((fields)->common.type), C_C_D, fields);
      else
	{
	  name = ((fields)->decl.name);
	  init = build_tree_list ((tree) 0, init);
	}
      current_member_init_list
	= tree_cons (name, init, current_member_init_list);
    }
}
void
finish_function (lineno, call_poplevel)
     int lineno;
     int call_poplevel;
{
  register tree fndecl = current_function_decl;
  tree fntype = ((fndecl)->common.type), ctype = (tree) 0;
  rtx head, last_parm_insn, mark;
  extern int sets_exception_throw_decl;
  tree no_return_label = 0;
  if (! current_function_parms_stored)
    {
      call_poplevel = 0;
      store_parm_decls ();
    }
  if (write_symbols != NO_DEBUG && ((enum tree_code) (fntype)->common.code) != METHOD_TYPE)
    {
      tree ttype = target_type (fntype);
      tree parmdecl;
      if ((((ttype)->type.lang_flag_5)))
	note_debug_info_needed (ttype);
      for (parmdecl = ((fndecl)->decl.arguments); parmdecl; parmdecl = ((parmdecl)->common.chain))
	{
	  ttype = target_type (((parmdecl)->common.type));
	  if ((((ttype)->type.lang_flag_5)))
	    note_debug_info_needed (ttype);
	}
    }
  do_pending_stack_adjust ();
  if (dtor_label)
    {
      tree binfo = ((current_class_type)->type.binfo);
      tree cond = integer_one_node;
      tree exprstmt, vfields;
      tree in_charge_node = lookup_name (in_charge_identifier, 0);
      tree virtual_size;
      int ok_to_optimize_dtor = 0;
      if (current_function_assigns_this)
	cond = build (NE_EXPR, integer_type_node,
		      current_class_decl, integer_zero_node);
      else
	{
	  int n_baseclasses =   (((((current_class_type)->type.binfo))->vec.a[ 4]) ? ((((((current_class_type)->type.binfo))->vec.a[ 4]))->vec.length) : 0);
	  mark = get_last_insn ();
	  last_parm_insn = get_first_nonparm_insn ();
	  if ((flag_this_is_variable & 1) == 0)
	    ok_to_optimize_dtor = 1;
	  else if (mark == last_parm_insn)
	    ok_to_optimize_dtor
	      = (n_baseclasses == 0
		 || (n_baseclasses == 1
		     && ((((((((((((current_class_type)->type.binfo)))->vec.a[ 4]))->vec.a[ ( 0)]))->common.type))->type.lang_flag_2))));
	}
      pushlevel (0);
      if (current_function_assigns_this)
	{
	  current_function_assigns_this = 0;
	  current_function_just_assigned_this = 0;
	}
      (((current_class_type)->type.lang_flag_2)) = 0;
      if ((((current_class_type)->common.lang_flag_3))
	  || (((current_class_type)->type.lang_specific)->type_flags.gets_delete))
	exprstmt = build_delete (current_class_type, C_C_D, integer_zero_node,
				 (8)|(512), 0, 0);
      else
	exprstmt = build_delete (current_class_type, C_C_D, in_charge_node,
				 (8)|(512), 0, 0);
      if (exprstmt != error_mark_node
	  && (((enum tree_code) (exprstmt)->common.code) != NOP_EXPR
	      || ((exprstmt)->exp.operands[ 0]) != integer_zero_node
	      || (((current_class_type)->common.lang_flag_3))))
	{
	  expand_label (dtor_label);
	  if (cond != integer_one_node)
	    expand_start_cond (cond, 0);
	  if (exprstmt != void_zero_node)
	    expand_expr_stmt (exprstmt);
	  if ((((current_class_type)->common.lang_flag_3)))
	    {
	      tree vbases = nreverse (copy_list ((((current_class_type)->type.lang_specific)->vbases)));
	      expand_start_cond (build (BIT_AND_EXPR, integer_type_node,
					in_charge_node, integer_two_node), 0);
	      while (vbases)
		{
		  if ((((((vbases)->common.type))->type.lang_flag_4)))
		    {
		      tree ptr = convert_pointer_to_vbase (vbases, current_class_decl);
		      expand_expr_stmt (build_delete (((((vbases)->common.type))->type.pointer_to),
						      ptr, integer_zero_node,
						      (8)|(512)|(32), 0, 0));
		    }
		  vbases = ((vbases)->common.chain);
		}
	      expand_end_cond ();
	    }
	  do_pending_stack_adjust ();
	  if (cond != integer_one_node)
	    expand_end_cond ();
	}
      (((current_class_type)->type.lang_flag_2)) = 1;
      virtual_size = c_sizeof (current_class_type);
      if ((((current_class_type)->type.lang_specific)->type_flags.gets_delete))
	exprstmt =
	  build_method_call
	    (build1 (NOP_EXPR,
		     ((current_class_type)->type.pointer_to), error_mark_node),
	     ansi_opname[(int) DELETE_EXPR],
	     tree_cons ((tree) 0, current_class_decl,
			build_tree_list ((tree) 0, virtual_size)),
	     (tree) 0, (3));
      else if ((((current_class_type)->common.lang_flag_3)))
	exprstmt = build_x_delete (ptr_type_node, current_class_decl, 0,
				   virtual_size);
      else
	exprstmt = 0;
      if (exprstmt)
	{
	  cond = build (BIT_AND_EXPR, integer_type_node,
			in_charge_node, integer_one_node);
	  expand_start_cond (cond, 0);
	  expand_expr_stmt (exprstmt);
	  expand_end_cond ();
	}
      poplevel (2, 0, 0);
      mark = get_last_insn ();
      last_parm_insn = get_first_nonparm_insn ();
      if (last_parm_insn == 0)
	last_parm_insn = mark;
      else
	last_parm_insn = previous_insn (last_parm_insn);
      if ((((current_class_type)->type.lang_specific)->vfields))
	{
	  for (vfields = (((current_class_type)->type.lang_specific)->vfields);
	       ((vfields)->common.chain);
	       vfields = ((vfields)->common.chain))
	    {
	      tree vf_decl = current_class_decl;
	      if (((vfields)->list.purpose))
		vf_decl = convert_pointer_to (((vfields)->list.purpose), vf_decl);
	      if (vf_decl != error_mark_node)
		expand_expr_stmt (build_virtual_init (binfo,
						      ((vfields)->list.value),
						      vf_decl));
	    }
	  expand_expr_stmt (build_virtual_init (binfo, binfo,
						current_class_decl));
	}
      if ((((current_class_type)->common.lang_flag_3)))
	expand_expr_stmt (build_vbase_vtables_init (binfo, binfo,
						    C_C_D, current_class_decl, 0));
      if (! ok_to_optimize_dtor)
	{
	  cond = build_binary_op (NE_EXPR, current_class_decl, integer_zero_node);
	  expand_start_cond (cond, 0);
	}
      if (mark != get_last_insn ())
	reorder_insns (next_insn (mark), get_last_insn (), last_parm_insn);
      if (! ok_to_optimize_dtor)
	expand_end_cond ();
    }
  else if (current_function_assigns_this)
    {
      if ((((current_function_decl)->decl.lang_specific)->decl_flags.constructor_attr))
	{
	  expand_label (ctor_label);
	  ctor_label = (tree) 0;
	  if (call_poplevel)
	    {
	      tree decls = getdecls ();
	      if (flag_handle_exceptions == 2)
		deactivate_exception_cleanups ();
	      expand_end_bindings (decls, decls != 0, 0);
	      poplevel (decls != 0, 0, 0);
	    }
	  c_expand_return (current_class_decl);
	}
      else if (((((
			((current_function_decl)->decl.result))->common.type))->type.main_variant) != void_type_node
	       && return_label != 0)
	no_return_label = build_decl (LABEL_DECL, (tree) 0, (tree) 0);
      current_function_assigns_this = 0;
      current_function_just_assigned_this = 0;
      base_init_insns = 0;
    }
  else if ((((fndecl)->decl.lang_specific)->decl_flags.constructor_attr))
    {
      tree allocated_this;
      tree cond, thenclause;
      tree abstract_virtuals = (((current_class_type)->type.lang_specific)->abstract_virtuals);
      (((current_class_type)->type.lang_specific)->abstract_virtuals) = (tree) 0;
      (((fndecl)->decl.lang_specific)->decl_flags.returns_first_arg) = 1;
      if (flag_this_is_variable > 0)
	{
	  cond = build_binary_op (EQ_EXPR, current_class_decl, integer_zero_node);
	  thenclause = build_modify_expr (current_class_decl, NOP_EXPR,
					  build_new ((tree) 0, current_class_type, void_type_node, 0));
	  if (flag_handle_exceptions == 2)
	    {
	      tree cleanup, cleanup_deallocate;
	      tree virtual_size;
	      virtual_size = c_sizeof (current_class_type);
	      allocated_this = build_decl (VAR_DECL, (tree) 0, ptr_type_node);
	      ((allocated_this)->decl.regdecl_flag) = 1;
	      ((allocated_this)->decl.initial) = error_mark_node;
	      expand_decl (allocated_this);
	      expand_decl_init (allocated_this);
	      cleanup = (((current_class_type)->type.lang_specific)->type_flags.gets_delete)
		? build_opfncall (DELETE_EXPR, (3), allocated_this, virtual_size)
		  : build_delete (((allocated_this)->common.type), allocated_this,
				  integer_three_node,
				  (3)|(32), 1, 0);
	      cleanup_deallocate
		= build_modify_expr (current_class_decl, NOP_EXPR, integer_zero_node);
	      cleanup = tree_cons ((tree) 0, cleanup,
				   build_tree_list ((tree) 0, cleanup_deallocate));
	      expand_decl_cleanup (allocated_this,
				   build (COND_EXPR, integer_type_node,
					  build (NE_EXPR, integer_type_node,
						 allocated_this, integer_zero_node),
					  build_compound_expr (cleanup),
					  integer_zero_node));
	    }
	}
      else if ((((current_class_type)->type.lang_specific)->type_flags.gets_new))
	build_method_call (build1 (NOP_EXPR, ((current_class_type)->type.pointer_to), error_mark_node),
			   ansi_opname[(int) NEW_EXPR],
			   build_tree_list ((tree) 0, integer_zero_node),
			   (tree) 0, (3));
      (((current_class_type)->type.lang_specific)->abstract_virtuals) = abstract_virtuals;
      head = get_insns ();
      mark = get_last_insn ();
      if (flag_this_is_variable > 0)
	{
	  expand_start_cond (cond, 0);
	  expand_expr_stmt (thenclause);
	  if (flag_handle_exceptions == 2)
	    expand_assignment (allocated_this, current_class_decl, 0, 0);
	  expand_end_cond ();
	}
      if (((fndecl)->decl.name) == (tree) 0
	  && ((((fndecl)->decl.arguments))->common.chain) != (tree) 0)
	build_default_constructor (fndecl);
      emit_insns (base_init_insns);
      base_init_insns = 0;
      last_parm_insn = get_first_nonparm_insn ();
      if (last_parm_insn == 0) last_parm_insn = mark;
      else last_parm_insn = previous_insn (last_parm_insn);
      if (mark != get_last_insn ())
	reorder_insns (next_insn (mark), get_last_insn (), last_parm_insn);
      expand_label (ctor_label);
      ctor_label = (tree) 0;
      if (flag_handle_exceptions == 2)
	{
	  expand_assignment (allocated_this, integer_zero_node, 0, 0);
	  if (call_poplevel)
	    deactivate_exception_cleanups ();
	}
      pop_implicit_try_blocks ((tree) 0);
      if (call_poplevel)
	{
	  expand_end_bindings (getdecls (), 1, 0);
	  poplevel (1, 1, 0);
	}
      c_expand_return (current_class_decl);
      current_function_assigns_this = 0;
      current_function_just_assigned_this = 0;
    }
  else if (((((fndecl)->decl.name))->identifier.length) == 4
	   && ! strcmp (((((fndecl)->decl.name))->identifier.pointer), "main")
	   && ((fndecl)->decl.context) == (tree) 0)
    {
      c_expand_return (integer_zero_node);
    }
  else if (return_label != 0
	   && current_function_return_value == 0
	   && ! ((((current_function_decl)->decl.result))->decl.name))
    no_return_label = build_decl (LABEL_DECL, (tree) 0, (tree) 0);
  if (flag_gc)
    expand_gc_prologue_and_epilogue ();
  if (obey_regdecls && current_vtable_decl)
    use_variable (((((current_vtable_decl)->exp.operands[ 0]))->decl.rtl));
  if (no_return_label)
    {
      ((no_return_label)->decl.context) = fndecl;
      ((no_return_label)->decl.initial) = error_mark_node;
      ((no_return_label)->decl.filename) = input_filename;
      ((no_return_label)->decl.linenum) = lineno;
      expand_goto (no_return_label);
    }
  if (cleanup_label)
    {
      expand_end_bindings (0, 0, 0);
      poplevel (0, 0, 0);
    }
  ((((fndecl)->decl.result))->decl.context) = ((fndecl)->decl.initial);
  if (flag_traditional && current_function_calls_setjmp)
    setjmp_protect (((fndecl)->decl.initial));
  if (cleanup_label)
    emit_label (cleanup_label);
  if (exception_throw_decl && sets_exception_throw_decl == 0)
    expand_assignment (exception_throw_decl, integer_zero_node, 0, 0);
  if (flag_handle_exceptions)
    {
      expand_end_try ();
      expand_start_except (0, 0);
      expand_end_except ();
    }
  expand_end_bindings (0, 0, 0);
  if (original_result_rtx)
    fixup_result_decl (((fndecl)->decl.result), original_result_rtx);
  if (no_return_label || cleanup_label)
    emit_jump (return_label);
  if (no_return_label)
    {
      expand_label (no_return_label);
    }
  if (current_class_name)
    {
      ctype = current_class_type;
      popclass (1);
    }
  else
    pop_memoized_context (1);
  while (overloads_to_forget)
    {
        (((struct lang_identifier *)(((overloads_to_forget)->list.purpose)))->global_value)
	= ((overloads_to_forget)->list.value);
      overloads_to_forget = ((overloads_to_forget)->common.chain);
    }
  expand_function_end (input_filename, lineno);
  if (current_binding_level->parm_flag != 1)
    my_friendly_abort (122);
  poplevel (1, 0, 1);
  ((((fndecl)->decl.initial))->block.supercontext) = fndecl;
  can_reach_end = 0;
  if (flag_pic
      && ((((fndecl)->decl.lang_specific)->decl_flags.constructor_attr)
	  || (((((fndecl)->decl.assembler_name))->identifier.pointer)[1] == '$'))
      && (((((fntype)->type.maxval))->type.lang_specific)->type_flags.needs_virtual_reinit))
    ((fndecl)->decl.inline_flag) = 0;
  if (((fndecl)->decl.external_flag)
      && (((fndecl)->decl.inline_flag) == 0
	  || flag_no_inline
	  || function_cannot_inline_p (fndecl)))
    {
      extern int rtl_dump_and_exit;
      int old_rtl_dump_and_exit = rtl_dump_and_exit;
      int inline_spec = ((fndecl)->decl.inline_flag);
      rtl_dump_and_exit = 1;
      if (flag_no_inline)
	((fndecl)->decl.inline_flag) = 0;
      rest_of_compilation (fndecl);
      rtl_dump_and_exit = old_rtl_dump_and_exit;
      ((fndecl)->decl.inline_flag) = inline_spec;
    }
  else
    {
      rest_of_compilation (fndecl);
    }
  if (ctype && ((fndecl)->common.asm_written_flag))
    note_debug_info_needed (ctype);
  current_function_returns_null |= can_reach_end;
  if ((((fndecl)->decl.lang_specific)->decl_flags.constructor_attr) || ((((fndecl)->decl.result))->decl.name) != 0)
    current_function_returns_null = 0;
  if (((fndecl)->common.volatile_flag) && current_function_returns_null)
    warning ("`volatile' function does return");
  else if (warn_return_type && current_function_returns_null
	   && ((((fntype)->common.type))->type.main_variant) != void_type_node)
    {
      pedwarn ("control reaches end of non-void function");
    }
  else if (extra_warnings
	   && current_function_returns_value && current_function_returns_null)
    warning ("this function may return with or without a value");
  permanent_allocation ();
  if (flag_cadillac)
    cadillac_finish_function (fndecl);
  if (((fndecl)->decl.saved_insns.r) == 0)
    {
      ((fndecl)->decl.initial) = error_mark_node;
      if (! (((fndecl)->decl.lang_specific)->decl_flags.constructor_attr)
	  || !(((((fntype)->type.maxval))->common.lang_flag_3)))
	((fndecl)->decl.arguments) = 0;
    }
  current_function_decl = (tree) 0;
  named_label_uses = (tree) 0;
  clear_anon_parm_name ();
}
tree
start_method (declspecs, declarator, raises)
     tree declarator, declspecs, raises;
{
  tree fndecl = grokdeclarator (declarator, declspecs, MEMFUNCDEF, 0, raises);
  if (fndecl == 0)
    return 0;
  if (((fndecl)->type.main_variant) == void_type_node)
    return fndecl;
  if (((enum tree_code) (fndecl)->common.code) != FUNCTION_DECL)
    return 0;
  if ((((fndecl)->decl.lang_flag_3)))
    {
      if (  (((struct lang_identifier *)(((fndecl)->decl.assembler_name)))->x    ? ((struct lang_identifier *)(((fndecl)->decl.assembler_name)))->x->error_locus : 0) != current_class_type)
	error_with_decl (fndecl, "`%s' is already defined in class %s",
			 ((((((((((fndecl)->decl.context))->type.name))->decl.name)))->identifier.pointer)));
      return void_type_node;
    }
  if (flag_default_inline && !processing_template_defn)
    ((fndecl)->decl.inline_flag) = 1;
  preserve_data ();
  if (! (((fndecl)->decl.lang_specific)->decl_flags.friend_attr))
    {
      if ((((fndecl)->decl.lang_specific)->chain) != (tree) 0)
	{
	  fndecl = copy_node (fndecl);
	}
      if ((((fndecl)->decl.lang_specific)->decl_flags.constructor_attr))
	grok_ctor_properties (current_class_type, fndecl);
      else if (((((fndecl)->decl.name))->common.lang_flag_2))
	grok_op_properties (fndecl, ((fndecl)->decl.virtual_flag));
    }
  finish_decl (fndecl, 0, 0, 0);
  pushlevel (0);
  current_binding_level->parm_flag = 1;
  (((fndecl)->decl.lang_flag_3)) = 1;
  return fndecl;
}
tree
finish_method (decl)
     tree decl;
{
  register tree fndecl = decl;
  tree old_initial;
  tree context = ((fndecl)->decl.context);
  register tree link;
  if (((decl)->type.main_variant) == void_type_node)
    return decl;
  old_initial = ((fndecl)->decl.initial);
  for (link = current_binding_level->names; link; link = ((link)->common.chain))
    {
      if (((link)->decl.name) != 0)
	  (((struct lang_identifier *)(((link)->decl.name)))->local_value) = 0;
      my_friendly_assert (((enum tree_code) (link)->common.code) != FUNCTION_DECL, 163);
      ((link)->decl.context) = 0;
    }
  for (link = current_binding_level->shadowed; link; link = ((link)->common.chain))
        (((struct lang_identifier *)(((link)->list.purpose)))->local_value) = ((link)->list.value);
  for (link = current_binding_level->class_shadowed;
       link; link = ((link)->common.chain))
      (((struct lang_identifier *)(((link)->list.purpose)))->class_value) = ((link)->list.value);
  for (link = current_binding_level->type_shadowed;
       link; link = ((link)->common.chain))
    (((((link)->list.purpose))->common.type)) = ((link)->list.value);
  GNU_xref_end_scope (current_binding_level,
		      current_binding_level->level_chain,
		      current_binding_level->parm_flag,
		      current_binding_level->keep,
		      current_binding_level->tag_transparent);
  pop_binding_level ();
  ((fndecl)->decl.initial) = old_initial;
  if ((((fndecl)->decl.lang_specific)->decl_flags.friend_attr))
    {
      (((current_class_type)->type.noncopied_parts))
	= tree_cons ((tree) 0, fndecl, (((current_class_type)->type.noncopied_parts)));
      decl = void_type_node;
    }
  return decl;
}
void
hack_incomplete_structures (type)
     tree type;
{
  tree decl;
  if (current_binding_level->n_incomplete == 0)
    return;
  if (!type) 
    return;
  for (decl = current_binding_level->names; decl; decl = ((decl)->common.chain))
    if (((decl)->common.type) == type
	|| (((decl)->common.type)
	    && ((enum tree_code) (((decl)->common.type))->common.code) == ARRAY_TYPE
	    && ((((decl)->common.type))->common.type) == type))
      {
	if (((enum tree_code) (decl)->common.code) == TYPE_DECL)
	  layout_type (((decl)->common.type));
	else
	  {
	    int toplevel = global_binding_level == current_binding_level;
	    if (((enum tree_code) (((decl)->common.type))->common.code) == ARRAY_TYPE
		&& ((((decl)->common.type))->common.type) == type)
	      layout_type (((decl)->common.type));
	    layout_decl (decl, 0);
	    rest_of_decl_compilation (decl, 0, toplevel, 0);
	    if (! toplevel)
	      {
		expand_decl (decl);
		expand_decl_cleanup (decl, maybe_build_cleanup (decl));
		expand_decl_init (decl);
	      }
	  }
	my_friendly_assert (current_binding_level->n_incomplete > 0, 164);
	--current_binding_level->n_incomplete;
      }
}
int building_cleanup;
tree
maybe_build_cleanup (decl)
     tree decl;
{
  tree type = ((decl)->common.type);
  if ((((type)->type.lang_flag_4)))
    {
      int temp = 0, flags = (3)|(512);
      tree rval;
      int old_building_cleanup = building_cleanup;
      building_cleanup = 1;
      if (((enum tree_code) (decl)->common.code) != PARM_DECL)
	temp = suspend_momentary ();
      if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
	rval = decl;
      else
	{
	  mark_addressable (decl);
	  rval = build_unary_op (ADDR_EXPR, decl, 0);
	}
      if (! (((type)->common.lang_flag_3))
	  || flag_expensive_optimizations)
	flags |= (8);
      if (((((((rval)->common.type))->common.type))->common.readonly_flag))
	rval = build1 (NOP_EXPR, ((((((((rval)->common.type))->common.type))->type.main_variant))->type.pointer_to), rval);
      rval = build_delete (((rval)->common.type), rval, integer_two_node, flags, 0, 0);
      if ((((type)->common.lang_flag_3))
	  && ! (((type)->type.lang_flag_2)))
	rval = build_compound_expr (tree_cons ((tree) 0, rval,
					       build_tree_list ((tree) 0, build_vbase_delete (type, decl))));
      current_binding_level->have_cleanups = 1;
      current_binding_level->more_exceptions_ok = 0;
      if (((enum tree_code) (decl)->common.code) != PARM_DECL)
	resume_momentary (temp);
      building_cleanup = old_building_cleanup;
      return rval;
    }
  return 0;
}
void
cplus_expand_expr_stmt (exp)
     tree exp;
{
  if (((exp)->common.type) == unknown_type_node)
    {
      if (((enum tree_code) (exp)->common.code) == ADDR_EXPR || ((enum tree_code) (exp)->common.code) == TREE_LIST)
	error ("address of overloaded function with no contextual type information");
      else if (((enum tree_code) (exp)->common.code) == COMPONENT_REF)
	warning ("useless reference to a member function name, did you forget the ()?");
    }
  else
    {
      int remove_implicit_immediately = 0;
      if (((enum tree_code) (exp)->common.code) == FUNCTION_DECL)
	{
	  warning_with_decl (exp, "reference, not call, to function `%s'");
	  warning ("at this point in file");
	}
      if (((exp)->common.raises_flag))
	{
	  my_friendly_assert (flag_handle_exceptions, 165);
	  if (flag_handle_exceptions == 2)
	    {
	      if (! current_binding_level->more_exceptions_ok)
		{
		  extern struct nesting *nesting_stack, *block_stack;
		  remove_implicit_immediately
		    = (nesting_stack != block_stack);
		  cplus_expand_start_try (1);
		}
	      current_binding_level->have_exceptions = 1;
	    }
	}
      expand_expr_stmt (break_out_cleanups (exp));
      if (remove_implicit_immediately)
	pop_implicit_try_blocks ((tree) 0);
    }
  expand_cleanups_to ((tree) 0);
}
void
finish_stmt ()
{
  extern struct nesting *cond_stack, *loop_stack, *case_stack;
  if (current_function_assigns_this
      || ! current_function_just_assigned_this)
    return;
  if ((((current_function_decl)->decl.lang_specific)->decl_flags.constructor_attr))
    {
      if (cond_stack || loop_stack || case_stack)
	return;
      emit_insns (base_init_insns);
      check_base_init (current_class_type);
    }
  current_function_assigns_this = 1;
  if (flag_cadillac)
    cadillac_finish_stmt ();
}
void
pop_implicit_try_blocks (decl)
     tree decl;
{
  if (decl)
    {
      my_friendly_assert (current_binding_level->parm_flag == 3, 166);
      current_binding_level->names = ((decl)->common.chain);
    }
  while (current_binding_level->parm_flag == 3)
    {
      tree name = get_identifier ("(compiler error)");
      tree orig_ex_type = current_exception_type;
      tree orig_ex_decl = current_exception_decl;
      tree orig_ex_obj = current_exception_object;
      tree decl = cplus_expand_end_try (2);
      cplus_expand_start_except (name, decl);
      cplus_expand_reraise ((tree) 0);
      current_exception_type = orig_ex_type;
      current_exception_decl = orig_ex_decl;
      current_exception_object = orig_ex_obj;
      cplus_expand_end_except (error_mark_node);
    }
  if (decl)
    {
      ((decl)->common.chain) = current_binding_level->names;
      current_binding_level->names = decl;
    }
}
void
push_exception_cleanup (addr)
     tree addr;
{
  tree decl = build_decl (VAR_DECL, get_identifier ("exception cleanup"), ptr_type_node);
  tree cleanup;
  decl = pushdecl (decl);
  ((decl)->decl.regdecl_flag) = 1;
  store_init_value (decl, addr);
  expand_decl (decl);
  expand_decl_init (decl);
  cleanup = build (COND_EXPR, integer_type_node,
		   build (NE_EXPR, integer_type_node,
			  decl, integer_zero_node),
		   build_delete (((addr)->common.type), decl,
				 lookup_name (in_charge_identifier, 0),
				 (3)|(512), 0, 0),
		   integer_zero_node);
  expand_decl_cleanup (decl, cleanup);
}
static void
deactivate_exception_cleanups ()
{
  struct binding_level *b = current_binding_level;
  tree xyzzy = get_identifier ("exception cleanup");
  while (b != class_binding_level)
    {
      if (b->parm_flag == 3)
	{
	  tree decls = b->names;
	  while (decls)
	    {
	      if (((decls)->decl.name) == xyzzy)
		expand_assignment (decls, integer_zero_node, 0, 0);
	      decls = ((decls)->common.chain);
	    }
	}
      b = b->level_chain;
    }
}
static void
revert_static_member_fn (fn, decl, argtypes)
     tree *fn, *decl, *argtypes;
{
  tree tmp, function = *fn;
  *argtypes = ((*argtypes)->common.chain);
  tmp = build_function_type (((function)->common.type), *argtypes);
  tmp = build_type_variant (tmp, ((function)->common.readonly_flag),
			    ((function)->common.volatile_flag));
  tmp = build_exception_variant (((function)->type.maxval), tmp,
				 ((function)->type.noncopied_parts));
  ((*decl)->common.type) = tmp;
  *fn = tmp;
  (((*decl)->decl.lang_specific)->decl_flags.static_function) = 1;
}
