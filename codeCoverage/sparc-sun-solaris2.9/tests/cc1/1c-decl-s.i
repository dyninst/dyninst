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
  tree global_value, local_value, label_value, implicit_decl;
  tree error_locus, limbo_value;
};
extern int pedantic;
struct lang_type
{
  int len;
  tree elts[1];
};
extern tree build_component_ref (), build_conditional_expr (), build_compound_expr ();
extern tree build_unary_op (), build_binary_op (), build_function_call ();
extern tree parser_build_binary_op ();
extern tree build_indirect_ref (), build_array_ref (), build_c_cast ();
extern tree build_modify_expr ();
extern tree c_sizeof (), c_alignof (), c_alignof_expr ();
extern void store_init_value ();
extern tree digest_init ();
extern tree c_expand_start_case ();
extern tree default_conversion ();
extern tree common_type ();
extern tree build_label ();
extern int start_function ();
extern void finish_function ();
extern void store_parm_decls ();
extern tree get_parm_info ();
extern tree combine_parm_decls ();
extern void pushlevel ();
extern tree poplevel ();
extern tree groktypename (), lookup_name ();
extern tree lookup_label (), define_label (), shadow_label ();
extern tree implicitly_declare (), getdecls (), gettags ();
extern tree start_decl ();
extern void finish_decl ();
extern tree start_struct (), finish_struct (), xref_tag ();
extern tree grokfield ();
extern tree start_enum (), finish_enum ();
extern tree build_enumerator ();
extern tree make_index_type ();
extern tree c_build_type_variant ();
extern tree builtin_function ();
extern tree combine_strings ();
extern tree check_case_value ();
extern void binary_op_error ();
extern tree shorten_compare ();
extern char *get_directive_line ();
extern tree truthvalue_conversion ();
extern int maybe_objc_comptypes ();
extern tree maybe_building_objc_message_expr ();
extern tree short_integer_type_node, integer_type_node;
extern tree long_integer_type_node, long_long_integer_type_node;
extern tree short_unsigned_type_node, unsigned_type_node;
extern tree long_unsigned_type_node, long_long_unsigned_type_node;
extern tree ptrdiff_type_node;
extern tree unsigned_char_type_node, signed_char_type_node, char_type_node;
extern tree wchar_type_node, signed_wchar_type_node, unsigned_wchar_type_node;
extern tree float_type_node, double_type_node, long_double_type_node;
extern tree intQI_type_node, unsigned_intQI_type_node;
extern tree intHI_type_node, unsigned_intHI_type_node;
extern tree intSI_type_node, unsigned_intSI_type_node;
extern tree intDI_type_node, unsigned_intDI_type_node;
extern tree void_type_node, ptr_type_node, const_ptr_type_node;
extern tree string_type_node, const_string_type_node;
extern tree char_array_type_node, int_array_type_node, wchar_array_type_node;
extern tree default_function_type;
extern tree double_ftype_double, double_ftype_double_double;
extern tree int_ftype_int, long_ftype_long;
extern tree void_ftype_ptr_ptr_int, int_ftype_ptr_ptr_int;
extern tree void_ftype_ptr_int_int, string_ftype_ptr_ptr;
extern tree int_ftype_string_string, int_ftype_cptr_cptr_sizet;
extern int current_function_returns_value;
extern int current_function_returns_null;
extern int dollars_in_ident;
extern int flag_cond_mismatch;
extern int flag_no_asm;
extern int flag_no_ident;
extern int warn_implicit;
extern int warn_write_strings;
extern int warn_pointer_arith;
extern int warn_strict_prototypes;
extern int warn_redundant_decls;
extern int warn_nested_externs;
extern int warn_cast_qual;
extern int warn_traditional;
extern int warn_format;
extern int warn_char_subscripts;
extern int warn_conversion;
extern int flag_traditional;
extern int warn_parentheses;
extern int (*comptypes_record_hook) ();
extern int system_header_p;
enum rid
{
  RID_UNUSED,
  RID_INT,
  RID_CHAR,
  RID_FLOAT,
  RID_DOUBLE,
  RID_VOID,
  RID_UNUSED1,
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
  RID_NOALIAS,
  RID_MAX
};
extern tree ridpointers[(int) RID_MAX];
extern tree lastiddecl;
extern char *token_buffer;	
extern tree make_pointer_declarator ();
extern void reinit_parse_for_function ();
extern int yylex ();
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
enum decl_context
{ NORMAL,			
  FUNCDEF,			
  PARM,				
  FIELD,			
  BITFIELD,			
  TYPENAME};			
tree error_mark_node;
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
tree void_type_node;
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
tree int_ftype_cptr_cptr_sizet;
tree integer_zero_node;
tree null_pointer_node;
tree integer_one_node;
tree pending_invalid_xref;
char *pending_invalid_xref_file;
int pending_invalid_xref_line;
static tree enum_next_value;
static int enum_overflow;
static tree last_function_parms;
static tree last_function_parm_tags;
static tree current_function_parms;
static tree current_function_parm_tags;
static tree named_labels;
static tree shadowed_labels;
static int c_function_varargs;
tree current_function_decl;
int current_function_returns_value;
int current_function_returns_null;
static int warn_about_return_type;
static int current_extern_inline;
struct binding_level
  {
    tree names;
    tree tags;
    tree shadowed;
    tree blocks;
    tree this_block;
    struct binding_level *level_chain;
    char parm_flag;
    char tag_transparent;
    char subblocks_tag_transparent;
    char keep;
    char keep_if_subblocks;
    int n_incomplete;
    tree parm_order;
  };
static struct binding_level *current_binding_level;
static struct binding_level *free_binding_level;
static struct binding_level *global_binding_level;
static struct binding_level clear_binding_level
  = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static int keep_next_level_flag;
static int keep_next_if_subblocks;
static struct binding_level *label_level_chain;
static tree grokparms (), grokdeclarator ();
tree pushdecl ();
tree builtin_function ();
void shadow_tag_warned ();
static tree lookup_tag ();
static tree lookup_tag_reverse ();
static tree lookup_name_current_level ();
static char *redeclaration_error_message ();
static void layout_array_type ();
int flag_cond_mismatch;
int flag_short_double;
int flag_no_asm;
int flag_no_builtin;
int flag_no_nonansi_builtin;
int flag_traditional;
int flag_signed_bitfields = 1;
int explicit_flag_signed_bitfields = 0;
int flag_no_ident = 0;
int warn_implicit;
int warn_write_strings;
int warn_cast_qual;
int warn_traditional;
int warn_pointer_arith;
int warn_strict_prototypes;
int warn_missing_prototypes;
int warn_redundant_decls = 0;
int warn_nested_externs = 0;
int warn_format;
int warn_char_subscripts = 0;
int warn_conversion;
int warn_parentheses;
int dollars_in_ident = 1 > 1;
char *language_string = "GNU C";
int
c_decode_option (p)
     char *p;
{
  if (!strcmp (p, "-ftraditional") || !strcmp (p, "-traditional"))
    {
      flag_traditional = 1;
      flag_writable_strings = 1;
      dollars_in_ident = 1;
    }
  else if (!strcmp (p, "-fnotraditional") || !strcmp (p, "-fno-traditional"))
    {
      flag_traditional = 0;
      flag_writable_strings = 0;
      dollars_in_ident = 1 > 1;
    }
  else if (!strcmp (p, "-fsigned-char"))
    flag_signed_char = 1;
  else if (!strcmp (p, "-funsigned-char"))
    flag_signed_char = 0;
  else if (!strcmp (p, "-fno-signed-char"))
    flag_signed_char = 0;
  else if (!strcmp (p, "-fno-unsigned-char"))
    flag_signed_char = 1;
  else if (!strcmp (p, "-fsigned-bitfields")
	   || !strcmp (p, "-fno-unsigned-bitfields"))
    {
      flag_signed_bitfields = 1;
      explicit_flag_signed_bitfields = 1;
    }
  else if (!strcmp (p, "-funsigned-bitfields")
	   || !strcmp (p, "-fno-signed-bitfields"))
    {
      flag_signed_bitfields = 0;
      explicit_flag_signed_bitfields = 1;
    }
  else if (!strcmp (p, "-fshort-enums"))
    flag_short_enums = 1;
  else if (!strcmp (p, "-fno-short-enums"))
    flag_short_enums = 0;
  else if (!strcmp (p, "-fcond-mismatch"))
    flag_cond_mismatch = 1;
  else if (!strcmp (p, "-fno-cond-mismatch"))
    flag_cond_mismatch = 0;
  else if (!strcmp (p, "-fshort-double"))
    flag_short_double = 1;
  else if (!strcmp (p, "-fno-short-double"))
    flag_short_double = 0;
  else if (!strcmp (p, "-fasm"))
    flag_no_asm = 0;
  else if (!strcmp (p, "-fno-asm"))
    flag_no_asm = 1;
  else if (!strcmp (p, "-fbuiltin"))
    flag_no_builtin = 0;
  else if (!strcmp (p, "-fno-builtin"))
    flag_no_builtin = 1;
  else if (!strcmp (p, "-fno-ident"))
    flag_no_ident = 1;
  else if (!strcmp (p, "-fident"))
    flag_no_ident = 0;
  else if (!strcmp (p, "-ansi"))
    flag_no_asm = 1, flag_no_nonansi_builtin = 1, dollars_in_ident = 0;
  else if (!strcmp (p, "-Wimplicit"))
    warn_implicit = 1;
  else if (!strcmp (p, "-Wno-implicit"))
    warn_implicit = 0;
  else if (!strcmp (p, "-Wwrite-strings"))
    warn_write_strings = 1;
  else if (!strcmp (p, "-Wno-write-strings"))
    warn_write_strings = 0;
  else if (!strcmp (p, "-Wcast-qual"))
    warn_cast_qual = 1;
  else if (!strcmp (p, "-Wno-cast-qual"))
    warn_cast_qual = 0;
  else if (!strcmp (p, "-Wpointer-arith"))
    warn_pointer_arith = 1;
  else if (!strcmp (p, "-Wno-pointer-arith"))
    warn_pointer_arith = 0;
  else if (!strcmp (p, "-Wstrict-prototypes"))
    warn_strict_prototypes = 1;
  else if (!strcmp (p, "-Wno-strict-prototypes"))
    warn_strict_prototypes = 0;
  else if (!strcmp (p, "-Wmissing-prototypes"))
    warn_missing_prototypes = 1;
  else if (!strcmp (p, "-Wno-missing-prototypes"))
    warn_missing_prototypes = 0;
  else if (!strcmp (p, "-Wredundant-decls"))
    warn_redundant_decls = 1;
  else if (!strcmp (p, "-Wno-redundant-decls"))
    warn_redundant_decls = 0;
  else if (!strcmp (p, "-Wnested-externs"))
    warn_nested_externs = 1;
  else if (!strcmp (p, "-Wno-nested-externs"))
    warn_nested_externs = 0;
  else if (!strcmp (p, "-Wtraditional"))
    warn_traditional = 1;
  else if (!strcmp (p, "-Wno-traditional"))
    warn_traditional = 0;
  else if (!strcmp (p, "-Wformat"))
    warn_format = 1;
  else if (!strcmp (p, "-Wno-format"))
    warn_format = 0;
  else if (!strcmp (p, "-Wchar-subscripts"))
    warn_char_subscripts = 1;
  else if (!strcmp (p, "-Wno-char-subscripts"))
    warn_char_subscripts = 0;
  else if (!strcmp (p, "-Wconversion"))
    warn_conversion = 1;
  else if (!strcmp (p, "-Wno-conversion"))
    warn_conversion = 0;
  else if (!strcmp (p, "-Wparentheses"))
    warn_parentheses = 1;
  else if (!strcmp (p, "-Wno-parentheses"))
    warn_parentheses = 0;
  else if (!strcmp (p, "-Wreturn-type"))
    warn_return_type = 1;
  else if (!strcmp (p, "-Wno-return-type"))
    warn_return_type = 0;
  else if (!strcmp (p, "-Wcomment"))
    ; 
  else if (!strcmp (p, "-Wno-comment"))
    ; 
  else if (!strcmp (p, "-Wcomments"))
    ; 
  else if (!strcmp (p, "-Wno-comments"))
    ; 
  else if (!strcmp (p, "-Wtrigraphs"))
    ; 
  else if (!strcmp (p, "-Wno-trigraphs"))
    ; 
  else if (!strcmp (p, "-Wimport"))
    ; 
  else if (!strcmp (p, "-Wno-import"))
    ; 
  else if (!strcmp (p, "-Wall"))
    {
      extra_warnings = 1;
      warn_uninitialized = 1;
      warn_implicit = 1;
      warn_return_type = 1;
      warn_unused = 1;
      warn_switch = 1;
      warn_format = 1;
      warn_char_subscripts = 1;
      warn_parentheses = 1;
    }
  else
    return 0;
  return 1;
}
void
print_lang_decl ()
{
}
void
print_lang_type ()
{
}
void
print_lang_identifier (file, node, indent)
     FILE *file;
     tree node;
     int indent;
{
  print_node (file, "global",   (((struct lang_identifier *)(node))->global_value), indent + 4);
  print_node (file, "local",   (((struct lang_identifier *)(node))->local_value), indent + 4);
  print_node (file, "label",   (((struct lang_identifier *)(node))->label_value), indent + 4);
  print_node (file, "implicit",   (((struct lang_identifier *)(node))->implicit_decl), indent + 4);
  print_node (file, "error locus",   (((struct lang_identifier *)(node))->error_locus), indent + 4);
  print_node (file, "limbo value",   (((struct lang_identifier *)(node))->limbo_value), indent + 4);
}
static
struct binding_level *
make_binding_level ()
{
  return (struct binding_level *) xmalloc (sizeof (struct binding_level));
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
  return ((current_binding_level->keep_if_subblocks
	   && current_binding_level->blocks != 0)
	  || current_binding_level->keep
	  || current_binding_level->names != 0
	  || (current_binding_level->tags != 0
	      && !current_binding_level->tag_transparent));
}
void
declare_parm_level (definition_flag)
     int definition_flag;
{
  current_binding_level->parm_flag = 1 + definition_flag;
}
int
in_parm_level_p ()
{
  return current_binding_level->parm_flag;
}
void
pushlevel (tag_transparent)
     int tag_transparent;
{
  register struct binding_level *newlevel = (struct binding_level *) 0;
  if (current_binding_level == global_binding_level)
    {
      named_labels = 0;
    }
  if (free_binding_level)
    {
      newlevel = free_binding_level;
      free_binding_level = free_binding_level->level_chain;
    }
  else
    {
      newlevel = make_binding_level ();
    }
  *newlevel = clear_binding_level;
  newlevel->tag_transparent
    = (tag_transparent
       || (current_binding_level
	   ? current_binding_level->subblocks_tag_transparent
	   : 0));
  newlevel->level_chain = current_binding_level;
  current_binding_level = newlevel;
  newlevel->keep = keep_next_level_flag;
  keep_next_level_flag = 0;
  newlevel->keep_if_subblocks = keep_next_if_subblocks;
  keep_next_if_subblocks = 0;
}
tree
poplevel (keep, reverse, functionbody)
     int keep;
     int reverse;
     int functionbody;
{
  register tree link;
  tree decls;
  tree tags = current_binding_level->tags;
  tree subblocks = current_binding_level->blocks;
  tree block = 0;
  tree decl;
  int block_previously_created;
  keep |= current_binding_level->keep;
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
      {
	if (((decl)->decl.abstract_origin) != 0)
	  ((((decl)->decl.abstract_origin))->common.addressable_flag) = 1;
	else
	  output_inline_function (decl);
      }
  block = 0;
  block_previously_created = (current_binding_level->this_block != 0);
  if (block_previously_created)
    block = current_binding_level->this_block;
  else if (keep || functionbody
	   || (current_binding_level->keep_if_subblocks && subblocks != 0))
    block = make_node (BLOCK);
  if (block != 0)
    {
      ((block)->block.vars) = decls;
      ((block)->block.type_tags) = tags;
      ((block)->block.subblocks) = subblocks;
      remember_end_note (block);
    }
  for (link = subblocks; link; link = ((link)->common.chain))
    ((link)->block.supercontext) = block;
  for (link = decls; link; link = ((link)->common.chain))
    {
      if (((link)->decl.name) != 0)
	{
	  if (((link)->decl.external_flag))
	    {
	      if (((link)->common.used_flag))
		((((link)->decl.name))->common.used_flag) = 1;
	      if (((link)->common.addressable_flag))
		((((link)->decl.assembler_name))->common.addressable_flag) = 1;
	    }
	    (((struct lang_identifier *)(((link)->decl.name)))->local_value) = 0;
	}
    }
  for (link = current_binding_level->shadowed; link; link = ((link)->common.chain))
      (((struct lang_identifier *)(((link)->list.purpose)))->local_value) = ((link)->list.value);
  if (functionbody)
    {
      ((block)->block.vars) = 0;
      for (link = named_labels; link; link = ((link)->common.chain))
	{
	  register tree label = ((link)->list.value);
	  if (((label)->decl.initial) == 0)
	    {
	      error_with_decl (label, "label `%s' used but not defined");
	      define_label (input_filename, lineno,
			    ((label)->decl.name));
	    }
	  else if (warn_unused && !((label)->common.used_flag))
	    warning_with_decl (label, "label `%s' defined but not used");
	    (((struct lang_identifier *)(((label)->decl.name)))->label_value) = 0;
	  ((label)->common.chain) = ((block)->block.vars);
	  ((block)->block.vars) = label;
	}
    }
  {
    register struct binding_level *level = current_binding_level;
    current_binding_level = current_binding_level->level_chain;
    level->level_chain = free_binding_level;
    free_binding_level = level;
  }
  if (functionbody)
    ((current_function_decl)->decl.initial) = block;
  else if (block)
    {
      if (!block_previously_created)
        current_binding_level->blocks
          = chainon (current_binding_level->blocks, block);
    }
  else if (subblocks)
    current_binding_level->blocks
      = chainon (current_binding_level->blocks, subblocks);
  if (functionbody)
    for (link = tags; link; link = ((link)->common.chain))
      ((((link)->list.value))->type.context) = current_function_decl;
  else if (block)
    for (link = tags; link; link = ((link)->common.chain))
      ((((link)->list.value))->type.context) = block;
  if (block)
    ((block)->common.used_flag) = 1;
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
set_block (block)
     register tree block;
{
  current_binding_level->this_block = block;
}
void
push_label_level ()
{
  register struct binding_level *newlevel;
  if (free_binding_level)
    {
      newlevel = free_binding_level;
      free_binding_level = free_binding_level->level_chain;
    }
  else
    {
      newlevel = make_binding_level ();
    }
  newlevel->level_chain = label_level_chain;
  label_level_chain = newlevel;
  newlevel->names = named_labels;
  newlevel->shadowed = shadowed_labels;
  named_labels = 0;
  shadowed_labels = 0;
}
void
pop_label_level ()
{
  register struct binding_level *level = label_level_chain;
  tree link, prev;
  for (link = named_labels, prev = 0; link;)
    {
      if (((((link)->list.value))->common.lang_flag_1))
	{
	  if (((((link)->list.value))->decl.linenum) == 0)
	    {
	      error_with_decl ("label `%s' used but not defined",
			       ((link)->list.value));
	      define_label (input_filename, lineno,
			    ((((link)->list.value))->decl.name));
	    }
	  else if (warn_unused && !((((link)->list.value))->common.used_flag))
	    warning_with_decl (((link)->list.value), 
			       "label `%s' defined but not used");
	    (((struct lang_identifier *)(((((link)->list.value))->decl.name)))->label_value) = 0;
	  link = ((link)->common.chain);
	  if (prev)
	    ((prev)->common.chain) = link;
	  else
	    named_labels = link;
	}
      else
	{
	  prev = link;
	  link = ((link)->common.chain);
	}
    }
  for (link = shadowed_labels; link; link = ((link)->common.chain))
    if (((((link)->list.value))->decl.name) != 0)
        (((struct lang_identifier *)(((((link)->list.value))->decl.name)))->label_value)
	= ((link)->list.value);
  named_labels = chainon (named_labels, level->names);
  shadowed_labels = level->shadowed;
  label_level_chain = label_level_chain->level_chain;
  level->level_chain = free_binding_level;
  free_binding_level = level;
}
void
pushtag (name, type)
     tree name, type;
{
  register struct binding_level *b;
  for (b = current_binding_level; b->tag_transparent; b = b->level_chain)
    continue;
  if (name)
    {
      if (((type)->type.name) == 0)
	((type)->type.name) = name;
    }
  if (b == global_binding_level)
    b->tags = perm_tree_cons (name, type, b->tags);
  else
    b->tags = saveable_tree_cons (name, type, b->tags);
  (((type)->common.chain)) = pushdecl (build_decl (TYPE_DECL, (tree) 0, type));
}
static int
duplicate_decls (newdecl, olddecl)
     register tree newdecl, olddecl;
{
  int types_match = comptypes (((newdecl)->common.type), ((olddecl)->common.type));
  int new_is_definition = (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
			   && ((newdecl)->decl.initial) != 0);
  tree oldtype = ((olddecl)->common.type);
  tree newtype = ((newdecl)->common.type);
  if (((enum tree_code) (newtype)->common.code) == ERROR_MARK
      || ((enum tree_code) (oldtype)->common.code) == ERROR_MARK)
    types_match = 0;
  if (((enum tree_code) (olddecl)->common.code) != ((enum tree_code) (newdecl)->common.code))
    {
      if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	  && ((olddecl)->decl.bit_field_flag))
	{
	  if (!((newdecl)->common.public_flag))
	    {
	      if (warn_shadow)
		warning_with_decl (newdecl, "shadowing built-in function `%s'");
	    }
	  else if (((olddecl)->common.unsigned_flag))
	    warning_with_decl (newdecl,
			       "built-in function `%s' declared as non-function");
	  else
	    error_with_decl (newdecl,
			     "built-in function `%s' declared as non-function");
	}
      else if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	       && ((olddecl)->common.unsigned_flag))
	{
	  if (!((newdecl)->common.public_flag))
	    {
	      if (warn_shadow)
		warning_with_decl (newdecl, "shadowing library function `%s'");
	    }
	  else
	    warning_with_decl (newdecl,
			       "library function `%s' declared as non-function");
	}
      else
	{
	  error_with_decl (newdecl, "`%s' redeclared as different kind of symbol");
	  error_with_decl (olddecl, "previous declaration of `%s'");
	}
      return 0;
    }
  if (types_match && ((enum tree_code) (newdecl)->common.code) == PARM_DECL
      && ((olddecl)->common.asm_written_flag) && ! ((newdecl)->common.asm_written_flag))
    return 1;
  if (flag_traditional && ((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
      &&   (((struct lang_identifier *)(((newdecl)->decl.name)))->implicit_decl) == olddecl
      && ((olddecl)->decl.initial) == 0)
    ;
  else if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	   && ((olddecl)->decl.bit_field_flag))
    {
      if (!((newdecl)->common.public_flag))
	{
	  if (warn_shadow)
	    warning_with_decl (newdecl, "shadowing built-in function `%s'");
	  return 0;
	}
      else if (!types_match)
	{
	  tree oldreturntype = ((((olddecl)->common.type))->common.type);
	  tree newreturntype = ((((newdecl)->common.type))->common.type);
          if (((oldreturntype)->type.mode) == ((newreturntype)->type.mode))
            {
	      tree newtype
		= build_function_type (newreturntype,
				       ((((olddecl)->common.type))->type.values));
              types_match = comptypes (((newdecl)->common.type), newtype);
	      if (types_match)
		((olddecl)->common.type) = newtype;
	    }
	}
      if (!types_match)
	{
	  warning_with_decl (newdecl, "conflicting types for built-in function `%s'");
	  return 0;
	}
    }
  else if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	   && ((olddecl)->decl.linenum) == 0)
    {
      if (!((newdecl)->common.public_flag))
	{
	  return 0;
	}
      else if (!types_match)
	{
	  ((newdecl)->common.volatile_flag) |= ((olddecl)->common.volatile_flag);
	}
    }
  else if (!types_match
	   && ((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	   && ((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
	   && ((enum tree_code) (((oldtype)->common.type))->common.code) == POINTER_TYPE
	   && ((enum tree_code) (((newtype)->common.type))->common.code) == POINTER_TYPE
	   && (((olddecl)->decl.in_system_header_flag)
	       || ((newdecl)->decl.in_system_header_flag))
	   && ((((((((newtype)->common.type))->common.type))->type.main_variant) == void_type_node
		&& ((oldtype)->type.values) == 0
		&& self_promoting_args_p (((newtype)->type.values))
		&& ((((oldtype)->common.type))->common.type) == char_type_node)
	       ||
	       (((((newtype)->common.type))->common.type) == char_type_node
		&& ((newtype)->type.values) == 0
		&& self_promoting_args_p (((oldtype)->type.values))
		&& ((((((oldtype)->common.type))->common.type))->type.main_variant) == void_type_node)))
    {
      if (pedantic)
	pedwarn_with_decl (newdecl, "conflicting types for `%s'");
      if (((((((oldtype)->common.type))->common.type))->type.main_variant) == void_type_node)
	((newdecl)->common.type) = newtype = oldtype;
    }
  else if (!types_match
	   && ! (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
		 && ! pedantic
		 && comptypes (((oldtype)->common.type),
			       ((newtype)->common.type))
		 && ((newtype)->type.values) == 0))
    {
      error_with_decl (newdecl, "conflicting types for `%s'");
      if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	  && comptypes (((oldtype)->common.type),
			((newtype)->common.type))
	  && ((((oldtype)->type.values) == 0
	       && ((olddecl)->decl.initial) == 0)
	      ||
	      (((newtype)->type.values) == 0
	       && ((newdecl)->decl.initial) == 0)))
	{
	  register tree t = ((oldtype)->type.values);
	  if (t == 0)
	    t = ((newtype)->type.values);
	  for (; t; t = ((t)->common.chain))
	    {
	      register tree type = ((t)->list.value);
	      if (((t)->common.chain) == 0
		  && ((type)->type.main_variant) != void_type_node)
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
      error_with_decl (olddecl, "previous declaration of `%s'");
    }
  else
    {
      char *errmsg = redeclaration_error_message (newdecl, olddecl);
      if (errmsg)
	{
	  error_with_decl (newdecl, errmsg);
	  error_with_decl (olddecl,
			   ((((olddecl)->decl.initial)
			     && current_binding_level == global_binding_level)
			    ? "`%s' previously defined here"
			    : "`%s' previously declared here"));
	}
      else if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	       && ((olddecl)->decl.initial) != 0
	       && ((oldtype)->type.values) == 0
	       && ((newtype)->type.values) != 0)
	{
	  register tree type, parm;
	  register int nargs;
	  for (parm = ((oldtype)->type.noncopied_parts),
	       type = ((newtype)->type.values),
	       nargs = 1;
	       (((((parm)->list.value))->type.main_variant) != void_type_node
		|| ((((type)->list.value))->type.main_variant) != void_type_node);
	       parm = ((parm)->common.chain), type = ((type)->common.chain), nargs++)
	    {
	      if (((((parm)->list.value))->type.main_variant) == void_type_node
		  || ((((type)->list.value))->type.main_variant) == void_type_node)
		{
		  errmsg = "prototype for `%s' follows and number of arguments";
		  break;
		}
	      if (! comptypes (((parm)->list.value), ((type)->list.value))
		  && (! (flag_traditional
			 && ((((parm)->list.value))->type.main_variant) == integer_type_node
			 && ((((type)->list.value))->type.main_variant) == unsigned_type_node)))
		{
		  errmsg = "prototype for `%s' follows and argument %d";
		  break;
		}
	    }
	  if (errmsg)
	    {
	      error_with_decl (newdecl, errmsg, nargs);
	      error_with_decl (olddecl,
			       "doesn't match non-prototype definition here");
	    }
	  else
	    {
	      warning_with_decl (newdecl, "prototype for `%s' follows");
	      warning_with_decl (olddecl, "non-prototype definition here");
	    }
	}
      else
	{
	  if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	      && ! ((olddecl)->decl.inline_flag) && ((newdecl)->decl.inline_flag)
	      && ((olddecl)->common.used_flag))
	    warning_with_decl (newdecl,
			       "`%s' declared inline after being called");
	  if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	      && ! ((olddecl)->decl.inline_flag) && ((newdecl)->decl.inline_flag)
	      && ((olddecl)->decl.initial) != 0)
	    warning_with_decl (newdecl,
			       "`%s' declared inline after its definition");
	  if (((enum tree_code) (olddecl)->common.code) == FUNCTION_DECL
	      && ((olddecl)->common.public_flag)
	      && !((newdecl)->common.public_flag))
	    warning_with_decl (newdecl, "static declaration for `%s' follows non-static");
	  if (pedantic && ((enum tree_code) (olddecl)->common.code) != FUNCTION_DECL
	      && (((newdecl)->common.readonly_flag) != ((olddecl)->common.readonly_flag)
		  || ((newdecl)->common.volatile_flag) != ((olddecl)->common.volatile_flag)))
	    pedwarn_with_decl (newdecl, "type qualifiers for `%s' conflict with previous decl");
	}
    }
  if (warn_redundant_decls && ((olddecl)->decl.linenum) != 0
      && !(((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL && ((newdecl)->decl.initial) != 0
	   && ((olddecl)->decl.initial) == 0))
    {
      warning_with_decl (newdecl, "redundant redeclaration of `%s' in same scope");
      warning_with_decl (olddecl, "previous declaration of `%s'");
    }
  if (types_match)
    {
      if (((enum tree_code) (newdecl)->common.code) != FUNCTION_DECL || !((olddecl)->decl.bit_field_flag))
	((newdecl)->common.type)
	  = ((olddecl)->common.type)
	    = common_type (newtype, oldtype);
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
	  if (((enum tree_code) (olddecl)->common.code) != FUNCTION_DECL)
	    if (((olddecl)->decl.frame_size) > ((newdecl)->decl.frame_size))
	      ((newdecl)->decl.frame_size) = ((olddecl)->decl.frame_size);
	}
      ((newdecl)->decl.rtl) = ((olddecl)->decl.rtl);
      if (((olddecl)->common.unsigned_flag) && ((olddecl)->common.volatile_flag)
	  && !((newdecl)->common.volatile_flag))
	((olddecl)->common.volatile_flag) = 0;
      if (((newdecl)->common.readonly_flag))
	((olddecl)->common.readonly_flag) = 1;
      if (((newdecl)->common.volatile_flag))
	{
	  ((olddecl)->common.volatile_flag) = 1;
	  if (((enum tree_code) (newdecl)->common.code) == VAR_DECL)
	    make_var_volatile (newdecl);
	}
      if (((newdecl)->decl.initial) == 0 && ((olddecl)->decl.initial) != 0)
	{
	  ((newdecl)->decl.linenum) = ((olddecl)->decl.linenum);
	  ((newdecl)->decl.filename) = ((olddecl)->decl.filename);
	}
      if (((olddecl)->decl.in_system_header_flag))
	((newdecl)->decl.in_system_header_flag) = 1;
      else if (((newdecl)->decl.in_system_header_flag))
	((olddecl)->decl.in_system_header_flag) = 1;
      if (((newdecl)->decl.initial) == 0)
	((newdecl)->decl.initial) = ((olddecl)->decl.initial);
    }
  else
    {
      ((olddecl)->common.type) = ((newdecl)->common.type);
      ((olddecl)->common.readonly_flag) = ((newdecl)->common.readonly_flag);
      ((olddecl)->common.volatile_flag) = ((newdecl)->common.volatile_flag);
      ((olddecl)->common.side_effects_flag) = ((newdecl)->common.side_effects_flag);
    }
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      ((newdecl)->common.public_flag) &= ((olddecl)->common.public_flag);
      ((olddecl)->common.public_flag) = ((newdecl)->common.public_flag);
      if (! ((olddecl)->common.public_flag))
	((((olddecl)->decl.name))->common.public_flag) = 0;
    }
  if (((newdecl)->decl.external_flag))
    {
      ((newdecl)->common.static_flag) = ((olddecl)->common.static_flag);
      ((newdecl)->decl.external_flag) = ((olddecl)->decl.external_flag);
      ((newdecl)->common.public_flag) = ((olddecl)->common.public_flag);
    }
  else
    {
      ((olddecl)->common.static_flag) = ((newdecl)->common.static_flag);
      ((olddecl)->common.public_flag) = ((newdecl)->common.public_flag);
    }
  if (((newdecl)->decl.inline_flag) && ((olddecl)->decl.initial) == 0)
    ((olddecl)->decl.inline_flag) = 1;
  ((newdecl)->decl.inline_flag) = ((olddecl)->decl.inline_flag);
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL
      && ((olddecl)->decl.bit_field_flag)
      && (!types_match || new_is_definition))
    {
      ((olddecl)->common.type) = ((newdecl)->common.type);
      ((olddecl)->decl.bit_field_flag) = 0;
    }
  if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL && !new_is_definition)
    {
      if (((olddecl)->decl.bit_field_flag))
	{
	  ((newdecl)->decl.bit_field_flag) = 1;
	   ((newdecl)->decl.frame_size = (int) (  ((enum built_in_function) (olddecl)->decl.frame_size)));
	}
      else
	((newdecl)->decl.frame_size) = ((olddecl)->decl.frame_size);
      ((newdecl)->decl.result) = ((olddecl)->decl.result);
      ((newdecl)->decl.initial) = ((olddecl)->decl.initial);
      ((newdecl)->decl.saved_insns.r) = ((olddecl)->decl.saved_insns.r);
      ((newdecl)->decl.arguments) = ((olddecl)->decl.arguments);
    }
  {
    register unsigned olddecl_uid = ((olddecl)->decl.uid);
    bcopy ((char *) newdecl + sizeof (struct tree_common),
	   (char *) olddecl + sizeof (struct tree_common),
	   sizeof (struct tree_decl) - sizeof (struct tree_common));
    ((olddecl)->decl.uid) = olddecl_uid;
  }
  return 1;
}
tree
pushdecl (x)
     tree x;
{
  register tree t;
  register tree name = ((x)->decl.name);
  register struct binding_level *b = current_binding_level;
  ((x)->decl.context) = current_function_decl;
  if (((enum tree_code) (x)->common.code) == FUNCTION_DECL && ((x)->decl.initial) == 0)
    ((x)->decl.context) = 0;
  if (warn_nested_externs && ((x)->decl.external_flag) && b != global_binding_level
      && x !=   (((struct lang_identifier *)(name))->implicit_decl))
    warning ("nested extern declaration of `%s'", ((name)->identifier.pointer));
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
	  file = ((t)->decl.filename);
	  line = ((t)->decl.linenum);
	}
      if (t != 0 && duplicate_decls (x, t))
	{
	  if (((enum tree_code) (t)->common.code) == PARM_DECL)
	    {
	      ((t)->common.asm_written_flag) = ((x)->common.asm_written_flag);
	      return t;
	    }
	  if (!flag_traditional && ((name)->common.public_flag)
	      && ! ((x)->common.public_flag) && ! ((x)->decl.external_flag)
	      &&   (((struct lang_identifier *)(name))->implicit_decl) != 0)
	    {
	      pedwarn ("`%s' was declared implicitly `extern' and later `static'",
		       ((name)->identifier.pointer));
	      pedwarn_with_file_and_line (file, line,
					  "previous declaration of `%s'",
					  ((name)->identifier.pointer));
	    }
	  return t;
	}
      if (((enum tree_code) (x)->common.code) == TYPE_DECL)
        {
          if (((x)->decl.linenum) == 0)
            {
	      if (((((x)->common.type))->type.name) == 0)
	        ((((x)->common.type))->type.name) = x;
            }
          else
            {
              tree tt = ((x)->common.type);
              tt = build_type_copy (tt);
              ((tt)->type.name) = x;
              ((x)->common.type) = tt;
            }
        }
      if (((x)->decl.external_flag) && ! ((x)->decl.inline_flag))
	{
	  tree decl;
	  if (  (((struct lang_identifier *)(name))->global_value) != 0
	      && (((  (((struct lang_identifier *)(name))->global_value))->decl.external_flag)
		  || ((  (((struct lang_identifier *)(name))->global_value))->common.public_flag)))
	    decl =   (((struct lang_identifier *)(name))->global_value);
	  else if (  (((struct lang_identifier *)(name))->limbo_value) != 0)
	    decl =   (((struct lang_identifier *)(name))->limbo_value);
	  else
	    decl = 0;
	  if (decl && ! comptypes (((x)->common.type), ((decl)->common.type)))
	    {
	      pedwarn_with_decl (x,
				 "type mismatch with previous external decl");
	      pedwarn_with_decl (decl, "previous external decl of `%s'");
	    }
	}
      if (  (((struct lang_identifier *)(name))->implicit_decl) != 0
	  &&   (((struct lang_identifier *)(name))->global_value) == 0
	  && ((enum tree_code) (x)->common.code) == FUNCTION_DECL
	  && ! comptypes (((x)->common.type),
			  ((  (((struct lang_identifier *)(name))->implicit_decl))->common.type)))
	{
	  warning_with_decl (x, "type mismatch with previous implicit declaration");
	  warning_with_decl (  (((struct lang_identifier *)(name))->implicit_decl),
			     "previous implicit declaration of `%s'");
	}
      if (flag_traditional && ((x)->decl.external_flag)
	  && lookup_name (name) == 0)
	{
	  tree type = ((x)->common.type);
	  while (type)
	    {
	      if (type == error_mark_node)
		break;
	      if (! ((type)->common.permanent_flag))
		{
		  warning_with_decl (x, "type of external `%s' is not global");
		  break;
		}
	      else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE
		       && ((type)->type.values) != 0)
		break;
	      type = ((type)->common.type);
	    }
	  if (type == 0)
	    b = global_binding_level;
	}
      if (b == global_binding_level)
	{
	  if (  (((struct lang_identifier *)(name))->global_value) == 0 && ((x)->common.public_flag))
	    ((name)->common.public_flag) = 1;
	    (((struct lang_identifier *)(name))->global_value) = x;
	    (((struct lang_identifier *)(name))->limbo_value) = 0;
	  if (  (((struct lang_identifier *)(name))->implicit_decl)
	      && ((  (((struct lang_identifier *)(name))->implicit_decl))->common.used_flag))
	    ((x)->common.used_flag) = 1, ((name)->common.used_flag) = 1;
	  if (  (((struct lang_identifier *)(name))->implicit_decl)
	      && ((  (((struct lang_identifier *)(name))->implicit_decl))->common.addressable_flag))
	    ((x)->common.addressable_flag) = 1;
	  if (  (((struct lang_identifier *)(name))->implicit_decl) != 0
	      && ! (((enum tree_code) (x)->common.code) == FUNCTION_DECL
		    && (((((((x)->common.type))->common.type))->type.main_variant)
			== integer_type_node)))
	    pedwarn ("`%s' was previously implicitly declared to return `int'",
		     ((name)->identifier.pointer));
	  if (((name)->common.public_flag)
	      && ! ((x)->common.public_flag) && ! ((x)->decl.external_flag))
	    {
	      if (t != 0 && ((t)->decl.bit_field_flag))
		;
	      else if (t != 0 && ((t)->common.unsigned_flag))
		;
	      else if (  (((struct lang_identifier *)(name))->implicit_decl))
		pedwarn ("`%s' was declared implicitly `extern' and later `static'",
			 ((name)->identifier.pointer));
	      else
		pedwarn ("`%s' was declared `extern' and later `static'",
			 ((name)->identifier.pointer));
	    }
	}
      else
	{
	  tree oldlocal =   (((struct lang_identifier *)(name))->local_value);
	  tree oldglobal =   (((struct lang_identifier *)(name))->global_value);
	    (((struct lang_identifier *)(name))->local_value) = x;
	  if (oldlocal == 0
	      && ((x)->decl.external_flag) && !((x)->decl.inline_flag)
	      && oldglobal != 0
	      && ((enum tree_code) (x)->common.code) == FUNCTION_DECL
	      && ((enum tree_code) (oldglobal)->common.code) == FUNCTION_DECL)
	    {
	      if (! comptypes (((x)->common.type),
			       ((  (((struct lang_identifier *)(name))->global_value))->common.type)))
		pedwarn_with_decl (x, "extern declaration of `%s' doesn't match global one");
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
	      if (  (((struct lang_identifier *)(name))->limbo_value) == 0)
		  (((struct lang_identifier *)(name))->limbo_value) = x;
	    }
	  if (oldlocal != 0 && !((x)->decl.external_flag)
	      && ! current_binding_level->parm_flag
	      && current_binding_level->level_chain->parm_flag
	      && chain_member (oldlocal, current_binding_level->level_chain->names))
	    {
	      if (((enum tree_code) (oldlocal)->common.code) == PARM_DECL)
		pedwarn ("declaration of `%s' shadows a parameter",
			 ((name)->identifier.pointer));
	      else
		pedwarn ("declaration of `%s' shadows a symbol from the parameter list",
			 ((name)->identifier.pointer));
	    }
	  else if (warn_shadow && !((x)->decl.external_flag)
		   && ((x)->decl.linenum) != 0
		   && ! (((x)->decl.abstract_origin) != (tree) 0))
	    {
	      char *warnstring = 0;
	      if (((enum tree_code) (x)->common.code) == PARM_DECL
		  && current_binding_level->parm_flag == 1)
		;
	      else if (oldlocal != 0 && ((enum tree_code) (oldlocal)->common.code) == PARM_DECL)
		warnstring = "declaration of `%s' shadows a parameter";
	      else if (oldlocal != 0)
		warnstring = "declaration of `%s' shadows previous local";
	      else if (  (((struct lang_identifier *)(name))->global_value) != 0
		       &&   (((struct lang_identifier *)(name))->global_value) != error_mark_node)
		warnstring = "declaration of `%s' shadows global declaration";
	      if (warnstring)
		warning (warnstring, ((name)->identifier.pointer));
	    }
	  if (oldlocal != 0)
	    b->shadowed = tree_cons (name, oldlocal, b->shadowed);
	}
      if (((((x)->common.type))->type.size) == 0)
	++b->n_incomplete;
    }
  ((x)->common.chain) = b->names;
  b->names = x;
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
  return t;
}
tree
implicitly_declare (functionid)
     tree functionid;
{
  register tree decl;
  int traditional_warning = 0;
  int implicit_warning;
  push_obstacks_nochange ();
  end_temporary_allocation ();
    decl = build_decl (FUNCTION_DECL, functionid, default_function_type);
  if (((functionid)->common.public_flag) &&   (((struct lang_identifier *)(functionid))->global_value) == 0)
    traditional_warning = 1;
  implicit_warning = (  (((struct lang_identifier *)(functionid))->implicit_decl) == 0);
  ((decl)->decl.external_flag) = 1;
  ((decl)->common.public_flag) = 1;
    (((struct lang_identifier *)(functionid))->implicit_decl) = decl;
  pushdecl (decl);
  maybe_objc_check_decl (decl);
  rest_of_decl_compilation (decl, ((char *)0), 0, 0);
  if (warn_implicit && implicit_warning)
    warning ("implicit declaration of function `%s'",
	     ((functionid)->identifier.pointer));
  else if (warn_traditional && traditional_warning)
    warning ("function `%s' was previously declared within a block",
	     ((functionid)->identifier.pointer));
  gen_aux_info_record (decl, 0, 1, 0);
  pop_obstacks ();
  return decl;
}
static char *
redeclaration_error_message (newdecl, olddecl)
     tree newdecl, olddecl;
{
  if (((enum tree_code) (newdecl)->common.code) == TYPE_DECL)
    {
      if (flag_traditional && ((newdecl)->common.type) == ((olddecl)->common.type))
	return 0;
      return "redefinition of `%s'";
    }
  else if (((enum tree_code) (newdecl)->common.code) == FUNCTION_DECL)
    {
      if (((olddecl)->decl.initial) != 0 && ((newdecl)->decl.initial) != 0
	  && !(((olddecl)->decl.inline_flag) && ((olddecl)->decl.external_flag)
	       && !(((newdecl)->decl.inline_flag) && ((newdecl)->decl.external_flag))))
	return "redefinition of `%s'";
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
  else if (current_binding_level->parm_flag
	   && ((olddecl)->common.asm_written_flag) && !((newdecl)->common.asm_written_flag))
    return 0;
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
  register tree decl =   (((struct lang_identifier *)(id))->label_value);
  if (decl != 0)
    {
      if (((decl)->decl.context) != current_function_decl
	  && ! ((decl)->common.lang_flag_1))
	return shadow_label (id);
      return decl;
    }
  decl = build_decl (LABEL_DECL, id, void_type_node);
  label_rtx (decl);
  ((decl)->decl.context) = current_function_decl;
  ((decl)->decl.mode) = VOIDmode;
  ((decl)->decl.linenum) = lineno;
  ((decl)->decl.filename) = input_filename;
    (((struct lang_identifier *)(id))->label_value) = decl;
  named_labels = tree_cons ((tree) 0, decl, named_labels);
  return decl;
}
tree
shadow_label (name)
     tree name;
{
  register tree decl =   (((struct lang_identifier *)(name))->label_value);
  if (decl != 0)
    {
      shadowed_labels = tree_cons ((tree) 0, decl, shadowed_labels);
        (((struct lang_identifier *)(name))->label_value) = decl = 0;
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
  if (decl != 0 && ((decl)->decl.context) != current_function_decl)
    {
      shadowed_labels = tree_cons ((tree) 0, decl, shadowed_labels);
        (((struct lang_identifier *)(name))->label_value) = 0;
      decl = lookup_label (name);
    }
  if (((decl)->decl.initial) != 0)
    {
      error_with_decl (decl, "duplicate label `%s'");
      return 0;
    }
  else
    {
      ((decl)->decl.initial) = error_mark_node;
      ((decl)->decl.filename) = filename;
      ((decl)->decl.linenum) = line;
      return decl;
    }
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
lookup_tag (code, name, binding_level, thislevel_only)
     enum tree_code code;
     struct binding_level *binding_level;
     tree name;
     int thislevel_only;
{
  register struct binding_level *level;
  for (level = binding_level; level; level = level->level_chain)
    {
      register tree tail;
      for (tail = level->tags; tail; tail = ((tail)->common.chain))
	{
	  if (((tail)->list.purpose) == name)
	    {
	      if (((enum tree_code) (((tail)->list.value))->common.code) != code)
		{
		  pending_invalid_xref = name;
		  pending_invalid_xref_file = input_filename;
		  pending_invalid_xref_line = lineno;
		}
	      return ((tail)->list.value);
	    }
	}
      if (thislevel_only && ! level->tag_transparent)
	return (tree) 0;
    }
  return (tree) 0;
}
void
pending_xref_error ()
{
  if (pending_invalid_xref != 0)
    error_with_file_and_line (pending_invalid_xref_file,
			      pending_invalid_xref_line,
			      "`%s' defined as wrong kind of tag",
			      ((pending_invalid_xref)->identifier.pointer));
  pending_invalid_xref = 0;
}
static tree
lookup_tag_reverse (type)
     tree type;
{
  register struct binding_level *level;
  for (level = current_binding_level; level; level = level->level_chain)
    {
      register tree tail;
      for (tail = level->tags; tail; tail = ((tail)->common.chain))
	{
	  if (((tail)->list.value) == type)
	    return ((tail)->list.purpose);
	}
    }
  return (tree) 0;
}
tree
lookup_name (name)
     tree name;
{
  register tree val;
  if (current_binding_level != global_binding_level
      &&   (((struct lang_identifier *)(name))->local_value))
    val =   (((struct lang_identifier *)(name))->local_value);
  else
    val =   (((struct lang_identifier *)(name))->global_value);
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
void
init_decl_processing ()
{
  register tree endlink;
  tree traditional_ptr_type_node;
  tree memcpy_ftype, strlen_ftype;
  tree void_ftype_any;
  int wchar_type_size;
  tree temp;
  current_function_decl = 0;
  named_labels = 0;
  current_binding_level = (struct binding_level *) 0;
  free_binding_level = (struct binding_level *) 0;
  pushlevel (0);	
  global_binding_level = current_binding_level;
  integer_type_node = make_signed_type (32);
  pushdecl (build_decl (TYPE_DECL, ridpointers[(int) RID_INT],
			integer_type_node));
  char_type_node
    = (flag_signed_char
       ? make_signed_type (8)
       : make_unsigned_type (8));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("char"),
			char_type_node));
  long_integer_type_node = make_signed_type (64);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long int"),
			long_integer_type_node));
  unsigned_type_node = make_unsigned_type (32);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("unsigned int"),
			unsigned_type_node));
  long_unsigned_type_node = make_unsigned_type (64);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long unsigned int"),
			long_unsigned_type_node));
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
  error_mark_node = make_node (ERROR_MARK);
  ((error_mark_node)->common.type) = error_mark_node;
  short_integer_type_node = make_signed_type ((8 * (((8 + 1) / 2) < ( 2) ? ((8 + 1) / 2) : ( 2))));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("short int"),
			short_integer_type_node));
  long_long_integer_type_node = make_signed_type (64);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long long int"),
			long_long_integer_type_node));
  short_unsigned_type_node = make_unsigned_type ((8 * (((8 + 1) / 2) < ( 2) ? ((8 + 1) / 2) : ( 2))));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("short unsigned int"),
			short_unsigned_type_node));
  long_long_unsigned_type_node = make_unsigned_type (64);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long long unsigned int"),
			long_long_unsigned_type_node));
  signed_char_type_node = make_signed_type (8);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("signed char"),
			signed_char_type_node));
  unsigned_char_type_node = make_unsigned_type (8);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("unsigned char"),
			unsigned_char_type_node));
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
  pushdecl (build_decl (TYPE_DECL, ridpointers[(int) RID_FLOAT],
			float_type_node));
  layout_type (float_type_node);
  double_type_node = make_node (REAL_TYPE);
  if (flag_short_double)
    ((double_type_node)->type.precision) = 32;
  else
    ((double_type_node)->type.precision) = 64;
  pushdecl (build_decl (TYPE_DECL, ridpointers[(int) RID_DOUBLE],
			double_type_node));
  layout_type (double_type_node);
  long_double_type_node = make_node (REAL_TYPE);
  ((long_double_type_node)->type.precision) = 64;
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long double"),
			long_double_type_node));
  layout_type (long_double_type_node);
  wchar_type_node
    = ((  (((struct lang_identifier *)(get_identifier ("short unsigned int")))->global_value))->common.type);
  wchar_type_size = ((wchar_type_node)->type.precision);
  signed_wchar_type_node = type_for_size (wchar_type_size, 0);
  unsigned_wchar_type_node = type_for_size (wchar_type_size, 1);
  integer_zero_node =   build_int_2_wide ((int) (0), (int) ( 0));
  ((integer_zero_node)->common.type) = integer_type_node;
  integer_one_node =   build_int_2_wide ((int) (1), (int) ( 0));
  ((integer_one_node)->common.type) = integer_type_node;
  size_zero_node =   build_int_2_wide ((int) (0), (int) ( 0));
  ((size_zero_node)->common.type) = sizetype;
  size_one_node =   build_int_2_wide ((int) (1), (int) ( 0));
  ((size_one_node)->common.type) = sizetype;
  void_type_node = make_node (VOID_TYPE);
  pushdecl (build_decl (TYPE_DECL,
			ridpointers[(int) RID_VOID], void_type_node));
  layout_type (void_type_node);	
  ((void_type_node)->type.align) = 8;
  null_pointer_node =   build_int_2_wide ((int) (0), (int) ( 0));
  ((null_pointer_node)->common.type) = build_pointer_type (void_type_node);
  layout_type (((null_pointer_node)->common.type));
  string_type_node = build_pointer_type (char_type_node);
  const_string_type_node
    = build_pointer_type (build_type_variant (char_type_node, 1, 0));
  char_array_type_node
    = build_array_type (char_type_node, unsigned_char_type_node);
  int_array_type_node
    = build_array_type (integer_type_node, unsigned_char_type_node);
  wchar_array_type_node
    = build_array_type (wchar_type_node, unsigned_char_type_node);
  default_function_type
    = build_function_type (integer_type_node, (tree) 0);
  ptr_type_node = build_pointer_type (void_type_node);
  const_ptr_type_node
    = build_pointer_type (build_type_variant (void_type_node, 1, 0));
  endlink = tree_cons ((tree) 0, void_type_node, (tree) 0);
  void_ftype_any
    = build_function_type (void_type_node, (tree) 0);
  double_ftype_double
    = build_function_type (double_type_node,
			   tree_cons ((tree) 0, double_type_node, endlink));
  double_ftype_double_double
    = build_function_type (double_type_node,
			   tree_cons ((tree) 0, double_type_node,
				      tree_cons ((tree) 0,
						 double_type_node, endlink)));
  int_ftype_int
    = build_function_type (integer_type_node,
			   tree_cons ((tree) 0, integer_type_node, endlink));
  long_ftype_long
    = build_function_type (long_integer_type_node,
			   tree_cons ((tree) 0,
				      long_integer_type_node, endlink));
  void_ftype_ptr_ptr_int
    = build_function_type (void_type_node,
			   tree_cons ((tree) 0, ptr_type_node,
				      tree_cons ((tree) 0, ptr_type_node,
						 tree_cons ((tree) 0,
							    integer_type_node,
							    endlink))));
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
						 tree_cons ((tree) 0,
							    integer_type_node,
							    endlink))));
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
  strlen_ftype		
    = build_function_type (flag_traditional ? integer_type_node : sizetype,
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
  builtin_function ("__builtin_constant_p", int_ftype_int,
		    BUILT_IN_CONSTANT_P, ((char *)0));
  builtin_function ("__builtin_return_address",
		    build_function_type (ptr_type_node, 
					 tree_cons ((tree) 0,
						    unsigned_type_node,
						    endlink)),
		    BUILT_IN_RETURN_ADDRESS, ((char *)0));
  builtin_function ("__builtin_frame_address",
		    build_function_type (ptr_type_node, 
					 tree_cons ((tree) 0,
						    unsigned_type_node,
						    endlink)),
		    BUILT_IN_FRAME_ADDRESS, ((char *)0));
  builtin_function ("__builtin_alloca",
		    build_function_type (ptr_type_node,
					 tree_cons ((tree) 0,
						    sizetype,
						    endlink)),
		    BUILT_IN_ALLOCA, "alloca");
  if (! flag_no_builtin && !flag_no_nonansi_builtin)
    {
      temp = builtin_function ("alloca",
			       build_function_type (ptr_type_node,
						    tree_cons ((tree) 0,
							       sizetype,
							       endlink)),
			       BUILT_IN_ALLOCA, ((char *)0));
      ((temp)->common.unsigned_flag) = 1;
      temp = builtin_function ("_exit", void_ftype_any, NOT_BUILT_IN,
			       ((char *)0));
      ((temp)->common.volatile_flag) = 1;
      ((temp)->common.side_effects_flag) = 1;
      ((temp)->common.unsigned_flag) = 1;
    }
  builtin_function ("__builtin_abs", int_ftype_int, BUILT_IN_ABS, ((char *)0));
  builtin_function ("__builtin_fabs", double_ftype_double, BUILT_IN_FABS,
		    ((char *)0));
  builtin_function ("__builtin_labs", long_ftype_long, BUILT_IN_LABS,
		    ((char *)0));
  builtin_function ("__builtin_ffs", int_ftype_int, BUILT_IN_FFS, ((char *)0));
  builtin_function ("__builtin_saveregs",
		    build_function_type (ptr_type_node, (tree) 0),
		    BUILT_IN_SAVEREGS, ((char *)0));
  builtin_function ("__builtin_classify_type", default_function_type,
		    BUILT_IN_CLASSIFY_TYPE, ((char *)0));
  builtin_function ("__builtin_next_arg",
		    build_function_type (ptr_type_node, endlink),
		    BUILT_IN_NEXT_ARG, ((char *)0));
  builtin_function ("__builtin_args_info",
		    build_function_type (integer_type_node,
					 tree_cons ((tree) 0,
						    integer_type_node,
						    endlink)),
		    BUILT_IN_ARGS_INFO, ((char *)0));
  builtin_function ("__builtin_memcpy", memcpy_ftype,
		    BUILT_IN_MEMCPY, "memcpy");
  builtin_function ("__builtin_memcmp", int_ftype_cptr_cptr_sizet,
		    BUILT_IN_MEMCMP, "memcmp");
  builtin_function ("__builtin_strcmp", int_ftype_string_string,
		    BUILT_IN_STRCMP, "strcmp");
  builtin_function ("__builtin_strcpy", string_ftype_ptr_ptr,
		    BUILT_IN_STRCPY, "strcpy");
  builtin_function ("__builtin_strlen", strlen_ftype,
		    BUILT_IN_STRLEN, "strlen");
  builtin_function ("__builtin_fsqrt", double_ftype_double, 
		    BUILT_IN_FSQRT, "sqrt");
  builtin_function ("__builtin_sin", double_ftype_double, 
		    BUILT_IN_SIN, "sin");
  builtin_function ("__builtin_cos", double_ftype_double, 
		    BUILT_IN_COS, "cos");
  if (!flag_no_builtin)
    {
      builtin_function ("abs", int_ftype_int, BUILT_IN_ABS, ((char *)0));
      builtin_function ("fabs", double_ftype_double, BUILT_IN_FABS, ((char *)0));
      builtin_function ("labs", long_ftype_long, BUILT_IN_LABS, ((char *)0));
      builtin_function ("memcpy", memcpy_ftype, BUILT_IN_MEMCPY, ((char *)0));
      builtin_function ("memcmp", int_ftype_cptr_cptr_sizet, BUILT_IN_MEMCMP,
			((char *)0));
      builtin_function ("strcmp", int_ftype_string_string, BUILT_IN_STRCMP,
			((char *)0));
      builtin_function ("strcpy", string_ftype_ptr_ptr, BUILT_IN_STRCPY,
			((char *)0));
      builtin_function ("strlen", strlen_ftype, BUILT_IN_STRLEN, ((char *)0));
      builtin_function ("sqrt", double_ftype_double, BUILT_IN_FSQRT, ((char *)0));
      builtin_function ("sin", double_ftype_double, BUILT_IN_SIN, ((char *)0));
      builtin_function ("cos", double_ftype_double, BUILT_IN_COS, ((char *)0));
      temp = builtin_function ("abort", void_ftype_any, NOT_BUILT_IN,
			       ((char *)0));
      ((temp)->common.volatile_flag) = 1;
      ((temp)->common.side_effects_flag) = 1;
      temp = builtin_function ("exit", void_ftype_any, NOT_BUILT_IN, ((char *)0));
      ((temp)->common.volatile_flag) = 1;
      ((temp)->common.side_effects_flag) = 1;
    }
  declare_function_name ();
  start_identifier_warnings ();
  init_format_info_table ();
}
tree
builtin_function (name, type, function_code, library_name)
     char *name;
     tree type;
     enum built_in_function function_code;
     char *library_name;
{
  tree decl = build_decl (FUNCTION_DECL, get_identifier (name), type);
  ((decl)->decl.external_flag) = 1;
  ((decl)->common.public_flag) = 1;
  if (flag_traditional && name[0] != '_')
    ((decl)->common.unsigned_flag) = 1;
  if (library_name)
    ((decl)->decl.assembler_name) = get_identifier (library_name);
  make_decl_rtl (decl, ((char *)0), 1);
  pushdecl (decl);
  if (function_code != NOT_BUILT_IN)
    {
      ((decl)->decl.bit_field_flag) = 1;
       ((decl)->decl.frame_size = (int) ( function_code));
    }
  if (name[0] != '_' || name[1] != '_')
    (((decl))->decl.lang_flag_3) = 1;
  return decl;
}
void
shadow_tag (declspecs)
     tree declspecs;
{
  shadow_tag_warned (declspecs, 0);
}
void
shadow_tag_warned (declspecs, warned)
     tree declspecs;
     int warned;
{
  int found_tag = 0;
  register tree link;
  pending_invalid_xref = 0;
  for (link = declspecs; link; link = ((link)->common.chain))
    {
      register tree value = ((link)->list.value);
      register enum tree_code code = ((enum tree_code) (value)->common.code);
      if (code == RECORD_TYPE || code == UNION_TYPE || code == ENUMERAL_TYPE)
	{
	  register tree name = lookup_tag_reverse (value);
	  register tree t;
	  found_tag++;
	  if (name == 0)
	    {
	      if (!warned && code != ENUMERAL_TYPE) 
		{
		  pedwarn ("unnamed struct/union that defines no instances");
		  warned = 1;
		}
	    }
	  else
	    {
	      t = lookup_tag (code, name, current_binding_level, 1);
	      if (t == 0)
		{
		  t = make_node (code);
		  pushtag (name, t);
		}
	    }
	}
      else
	{
	  if (!warned)
	    pedwarn ("useless keyword or type name in empty declaration");
	  warned = 1;
	}
    }
  if (!warned)
    {
      if (found_tag > 1)
	error ("two types specified in one empty declaration");
      if (found_tag == 0)
	pedwarn ("empty declaration");
    }
}
tree
groktypename (typename)
     tree typename;
{
  if (((enum tree_code) (typename)->common.code) != TREE_LIST)
    return typename;
  return grokdeclarator (((typename)->list.value),
			 ((typename)->list.purpose),
			 TYPENAME, 0);
}
tree
groktypename_in_parm_context (typename)
     tree typename;
{
  if (((enum tree_code) (typename)->common.code) != TREE_LIST)
    return typename;
  return grokdeclarator (((typename)->list.value),
			 ((typename)->list.purpose),
			 PARM, 0);
}
int debug_temp_inits = 1;
tree
start_decl (declarator, declspecs, initialized)
     tree declspecs, declarator;
     int initialized;
{
  register tree decl = grokdeclarator (declarator, declspecs,
				       NORMAL, initialized);
  register tree tem;
  int init_written = initialized;
  push_obstacks_nochange ();
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
      case PARM_DECL:
	error ("parameter `%s' is initialized",
	       ((((decl)->decl.name))->identifier.pointer));
	initialized = 0;
	break;
      default:
	if (((((decl)->common.type))->type.size) != 0)
	  {
	    if (((enum tree_code) (((((decl)->common.type))->type.size))->common.code) != INTEGER_CST
		|| ((decl)->decl.lang_flag_0))
	      {
		error ("variable-sized object may not be initialized");
		initialized = 0;
	      }
	  }
	else if (((enum tree_code) (((decl)->common.type))->common.code) != ARRAY_TYPE)
	  {
	    error ("variable `%s' has initializer but incomplete type",
		   ((((decl)->decl.name))->identifier.pointer));
	    initialized = 0;
	  }
	else if (((((((decl)->common.type))->common.type))->type.size) == 0)
	  {
	    error ("elements of array `%s' have incomplete type",
		   ((((decl)->decl.name))->identifier.pointer));
	    initialized = 0;
	  }
      }
  if (initialized)
    {
      ((decl)->decl.external_flag) = 0;
      if (current_binding_level == global_binding_level)
	((decl)->common.static_flag) = 1;
      ((decl)->decl.initial) = error_mark_node;
    }
  if (((enum tree_code) (decl)->common.code) == FUNCTION_DECL)
    gen_aux_info_record (decl, 0, 0, ((((decl)->common.type))->type.values) != 0);
  tem = pushdecl (decl);
  if (current_binding_level != global_binding_level
      && ((tem)->decl.rtl) == 0)
    {
      if (((((tem)->common.type))->type.size) != 0)
	expand_decl (tem);
      else if (((enum tree_code) (((tem)->common.type))->common.code) == ARRAY_TYPE
	       && ((tem)->decl.initial) != 0)
	expand_decl (tem);
    }
  if (init_written)
    {
      if (current_binding_level == global_binding_level && debug_temp_inits)
	temporary_allocation ();
    }
  return tem;
}
void
finish_decl (decl, init, asmspec_tree)
     tree decl, init;
     tree asmspec_tree;
{
  register tree type = ((decl)->common.type);
  int was_incomplete = (((decl)->decl.size) == 0);
  int temporary = allocation_temporary_p ();
  char *asmspec = 0;
  if (asmspec_tree)
    asmspec = ((asmspec_tree)->string.pointer);
  if (init != 0 && ((decl)->decl.initial) == 0)
    init = 0;
  if (((enum tree_code) (decl)->common.code) == PARM_DECL)
    init = 0;
  if (init)
    {
      if (((enum tree_code) (decl)->common.code) != TYPE_DECL)
	store_init_value (decl, init);
      else
	{
	  ((decl)->common.type) = ((init)->common.type);
	  ((decl)->decl.initial) = init = 0;
	}
    }
  pop_obstacks ();
  if (((enum tree_code) (type)->common.code) == ARRAY_TYPE
      && ((type)->type.values) == 0
      && ((enum tree_code) (decl)->common.code) != TYPE_DECL)
    {
      int do_default
	= (((decl)->common.static_flag)
	   ? pedantic && !((decl)->common.public_flag)
	   : !((decl)->decl.external_flag));
      int failure
	= complete_array_type (type, ((decl)->decl.initial), do_default);
      type = ((decl)->common.type);
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
      if (((decl)->decl.size) == 0
	  && (((decl)->common.static_flag)
	      ?
		!((decl)->common.public_flag) || ((decl)->decl.initial)
	      :
		!((decl)->decl.external_flag)))
	{
	  error_with_decl (decl, "storage size of `%s' isn't known");
	  ((decl)->common.type) = error_mark_node;
	}
      if ((((decl)->decl.external_flag) || ((decl)->common.static_flag))
	  && ((decl)->decl.size) != 0
	  && ((enum tree_code) (((decl)->decl.size))->common.code) != INTEGER_CST)
	error_with_decl (decl, "storage size of `%s' isn't constant");
    }
  if (((enum tree_code) (decl)->common.code) == VAR_DECL || ((enum tree_code) (decl)->common.code) == FUNCTION_DECL)
    {
      if (flag_traditional && allocation_temporary_p ())
	{
	  push_obstacks_nochange ();
	  end_temporary_allocation ();
	  maybe_objc_check_decl (decl);
	  rest_of_decl_compilation (decl, asmspec,
				    current_binding_level == global_binding_level,
				    0);
	  pop_obstacks ();
	}
      else
	{
	  maybe_objc_check_decl (decl);
	  rest_of_decl_compilation (decl, asmspec,
				    current_binding_level == global_binding_level,
				    0);
	}
      if (current_binding_level != global_binding_level)
	{
	  if (was_incomplete
	      && ! ((decl)->common.static_flag) && ! ((decl)->decl.external_flag))
	    {
	      ((decl)->common.addressable_flag) = ((decl)->common.used_flag);
	      if (((decl)->decl.size) == 0)
		((decl)->decl.initial) = 0;
	      expand_decl (decl);
	    }
	  if (((enum tree_code) (decl)->common.code) != FUNCTION_DECL)
	    expand_decl_init (decl);
	}
    }
  if (((enum tree_code) (decl)->common.code) == TYPE_DECL)
    {
      maybe_objc_check_decl (decl);
      rest_of_decl_compilation (decl, ((char *)0),
				current_binding_level == global_binding_level,
				0);
    }
  if (!(((enum tree_code) (decl)->common.code) == FUNCTION_DECL && ((decl)->decl.inline_flag))
      && temporary && ((decl)->common.permanent_flag))
    {
      if (((decl)->decl.initial) != 0)
	((decl)->decl.initial) = error_mark_node;
    }
  if (temporary && !allocation_temporary_p ())
    permanent_allocation ();
  if (current_binding_level == global_binding_level)
    get_pending_sizes ();
}
tree
maybe_build_cleanup (decl)
     tree decl;
{
  return (tree) 0;
}
void
push_parm_decl (parm)
     tree parm;
{
  tree decl, olddecl;
  int old_immediate_size_expand = immediate_size_expand;
  immediate_size_expand = 0;
  push_obstacks_nochange ();
  decl = grokdeclarator (((parm)->list.value), ((parm)->list.purpose), PARM, 0);
  if (((decl)->decl.name))
    {
      olddecl = lookup_name (((decl)->decl.name));
      if (pedantic && olddecl != 0 && ((enum tree_code) (olddecl)->common.code) == TYPE_DECL)
	pedwarn_with_decl (decl, "ANSI C forbids parameter `%s' shadowing typedef");
    }
  decl = pushdecl (decl);
  immediate_size_expand = old_immediate_size_expand;
  current_binding_level->parm_order
    = tree_cons ((tree) 0, decl, current_binding_level->parm_order);
  finish_decl (decl, (tree) 0, (tree) 0);
}
void
clear_parm_order ()
{
  current_binding_level->parm_order = (tree) 0;
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
	{
	  int eltsize
	    = int_size_in_bytes (((((initial_value)->common.type))->common.type));
	  maxindex =   build_int_2_wide ((int) (((initial_value)->string.length) / eltsize - 1), (int) ( 0));
	}
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
static tree
grokdeclarator (declarator, declspecs, decl_context, initialized)
     tree declspecs;
     tree declarator;
     enum decl_context decl_context;
     int initialized;
{
  int specbits = 0;
  tree spec;
  tree type = (tree) 0;
  int longlong = 0;
  int constp;
  int volatilep;
  int inlinep;
  int explicit_int = 0;
  int explicit_char = 0;
  tree typedef_decl = 0;
  char *name;
  tree typedef_type = 0;
  int funcdef_flag = 0;
  enum tree_code innermost_code = ERROR_MARK;
  int bitfield = 0;
  int size_varies = 0;
  if (decl_context == BITFIELD)
    bitfield = 1, decl_context = FIELD;
  if (decl_context == FUNCDEF)
    funcdef_flag = 1, decl_context = NORMAL;
  push_obstacks_nochange ();
  if (flag_traditional && allocation_temporary_p ())
    end_temporary_allocation ();
  {
    register tree decl = declarator;
    name = 0;
    while (decl)
      switch (((enum tree_code) (decl)->common.code))
	{
	case ARRAY_REF:
	case INDIRECT_REF:
	case CALL_EXPR:
	  innermost_code = ((enum tree_code) (decl)->common.code);
	  decl = ((decl)->exp.operands[ 0]);
	  break;
	case IDENTIFIER_NODE:
	  name = ((decl)->identifier.pointer);
	  decl = 0;
	  break;
	default:
	  abort ();
	}
    if (name == 0)
      name = "type name";
  }
  if (funcdef_flag && innermost_code != CALL_EXPR)
    return 0;
  if (decl_context == NORMAL && !funcdef_flag
      && current_binding_level->level_chain == global_binding_level)
    decl_context = PARM;
  for (spec = declspecs; spec; spec = ((spec)->common.chain))
    {
      register int i;
      register tree id = ((spec)->list.value);
      if (id == ridpointers[(int) RID_INT])
	explicit_int = 1;
      if (id == ridpointers[(int) RID_CHAR])
	explicit_char = 1;
      if (((enum tree_code) (id)->common.code) == IDENTIFIER_NODE)
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
		  pedwarn ("duplicate `%s'", ((id)->identifier.pointer));
		specbits |= 1 << i;
		goto found;
	      }
	  }
      if (type)
	error ("two or more data types in declaration of `%s'", name);
      else if (((enum tree_code) (id)->common.code) == TYPE_DECL)
	{
	  type = ((id)->common.type);
	  typedef_decl = id;
	}
      else if (((enum tree_code) (id)->common.code) == IDENTIFIER_NODE)
	{
	  register tree t = lookup_name (id);
	  if (((t)->common.type) == error_mark_node)
	    ;
	  else if (!t || ((enum tree_code) (t)->common.code) != TYPE_DECL)
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
  if (type)
    size_varies = ((type)->type.lang_flag_1);
  if (type == 0)
    {
      if (funcdef_flag && warn_return_type
	  && ! (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
			    | (1 << (int) RID_SIGNED) | (1 << (int) RID_UNSIGNED))))
	warn_about_return_type = 1;
      explicit_int = 1;
      type = integer_type_node;
    }
  if ((specbits & 1 << (int) RID_LONG)
      && ((type)->type.main_variant) == double_type_node)
    {
      specbits &= ~ (1 << (int) RID_LONG);
      type = long_double_type_node;
    }
  if (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
		  | (1 << (int) RID_UNSIGNED) | (1 << (int) RID_SIGNED)))
    {
      int ok = 0;
      if (((enum tree_code) (type)->common.code) != INTEGER_TYPE)
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
      || (bitfield && flag_traditional
	  && (! explicit_flag_signed_bitfields || !flag_signed_bitfields))
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
  inlinep = !! (specbits & (1 << (int) RID_INLINE));
  if (constp > 1)
    pedwarn ("duplicate `const'");
  if (volatilep > 1)
    pedwarn ("duplicate `volatile'");
  if (! flag_gen_aux_info && (((type)->common.readonly_flag) || ((type)->common.volatile_flag)))
    type = ((type)->type.main_variant);
  {
    int nclasses = 0;
    if (specbits & 1 << (int) RID_AUTO) nclasses++;
    if (specbits & 1 << (int) RID_STATIC) nclasses++;
    if (specbits & 1 << (int) RID_EXTERN) nclasses++;
    if (specbits & 1 << (int) RID_REGISTER) nclasses++;
    if (specbits & 1 << (int) RID_TYPEDEF) nclasses++;
    if (nclasses > 1)
      error ("multiple storage classes in declaration of `%s'", name);
    else if (funcdef_flag
	     && (specbits
		 & ((1 << (int) RID_REGISTER)
		    | (1 << (int) RID_AUTO)
		    | (1 << (int) RID_TYPEDEF))))
      {
	if (specbits & 1 << (int) RID_AUTO
	    && (pedantic || current_binding_level == global_binding_level))
	  pedwarn ("function definition declared `auto'");
	if (specbits & 1 << (int) RID_REGISTER)
	  error ("function definition declared `register'");
	if (specbits & 1 << (int) RID_TYPEDEF)
	  error ("function definition declared `typedef'");
	specbits &= ~ ((1 << (int) RID_TYPEDEF) | (1 << (int) RID_REGISTER)
		       | (1 << (int) RID_AUTO));
      }
    else if (decl_context != NORMAL && nclasses > 0)
      {
	if (decl_context == PARM && specbits & 1 << (int) RID_REGISTER)
	  ;
	else
	  {
	    error ((decl_context == FIELD
		    ? "storage class specified for structure field `%s'"
		    : (decl_context == PARM
		       ? "storage class specified for parameter `%s'"
		       : "storage class specified for typename")),
		   name);
	    specbits &= ~ ((1 << (int) RID_TYPEDEF) | (1 << (int) RID_REGISTER)
			   | (1 << (int) RID_AUTO) | (1 << (int) RID_STATIC)
			   | (1 << (int) RID_EXTERN));
	  }
      }
    else if (specbits & 1 << (int) RID_EXTERN && initialized && ! funcdef_flag)
      {
	if (current_binding_level == global_binding_level)
	  warning ("`%s' initialized and declared `extern'", name);
	else
	  error ("`%s' has both `extern' and initializer", name);
      }
    else if (specbits & 1 << (int) RID_EXTERN && funcdef_flag
	     && current_binding_level != global_binding_level)
      error ("nested function `%s' declared `extern'", name);
    else if (current_binding_level == global_binding_level
	     && specbits & (1 << (int) RID_AUTO))
      error ("top-level declaration of `%s' specifies `auto'", name);
  }
  while (declarator && ((enum tree_code) (declarator)->common.code) != IDENTIFIER_NODE)
    {
      if (type == error_mark_node)
	{
	  declarator = ((declarator)->exp.operands[ 0]);
	  continue;
	}
      if (((enum tree_code) (declarator)->common.code) == ARRAY_REF)
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
	        while ((((enum tree_code) (size)->common.code) == NOP_EXPR					  || ((enum tree_code) (size)->common.code) == CONVERT_EXPR				  || ((enum tree_code) (size)->common.code) == NON_LVALUE_EXPR)			 && (((size)->common.type)						     == ((((size)->exp.operands[ 0]))->common.type)))		    (size) = ((size)->exp.operands[ 0]);;
	      if (((enum tree_code) (((size)->common.type))->common.code) != INTEGER_TYPE
		  && ((enum tree_code) (((size)->common.type))->common.code) != ENUMERAL_TYPE)
		{
		  error ("size of array `%s' has non-integer type", name);
		  size = integer_one_node;
		}
	      if (pedantic && integer_zerop (size))
		pedwarn ("ANSI C forbids zero-size array `%s'", name);
	      if (((enum tree_code) (size)->common.code) == INTEGER_CST)
		{
		  if ((((size)->int_cst.int_cst_high) < (( integer_zero_node)->int_cst.int_cst_high)			 || (((size)->int_cst.int_cst_high) == (( integer_zero_node)->int_cst.int_cst_high)		     && ((unsigned int) ((size)->int_cst.int_cst_low)			 < (unsigned int) (( integer_zero_node)->int_cst.int_cst_low)))))
		    {
		      error ("size of array `%s' is negative", name);
		      size = integer_one_node;
		    }
		  itype = build_index_type (size_binop (MINUS_EXPR, size,
							size_one_node));
		}
	      else
		{
		  if (pedantic)
		    pedwarn ("ANSI C forbids variable-size array `%s'", name);
		  itype = build_binary_op (MINUS_EXPR, size, integer_one_node,
					   1);
		  size_varies = 1;
		  itype = variable_size (itype);
		  itype = build_index_type (itype);
		}
	    }
	  if (constp || volatilep)
	    type = c_build_type_variant (type, constp, volatilep);
	  type = build_array_type (type, itype);
	  if (size_varies)
	    ((type)->type.lang_flag_1) = 1;
	}
      else if (((enum tree_code) (declarator)->common.code) == CALL_EXPR)
	{
	  int extern_ref = (!(specbits & (1 << (int) RID_AUTO))
			    || current_binding_level == global_binding_level);
	  tree arg_types;
	  if (type == error_mark_node)
	    continue;
	  size_varies = 0;
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
	  if (flag_traditional && ((type)->type.main_variant) == float_type_node)
	    type = double_type_node;
	  if (extern_ref && allocation_temporary_p ())
	    end_temporary_allocation ();
	  arg_types = grokparms (((declarator)->exp.operands[ 1]),
				 funcdef_flag
				 && ((enum tree_code) (((declarator)->exp.operands[ 0]))->common.code) == IDENTIFIER_NODE);
	  type = build_function_type (type, arg_types);
	  declarator = ((declarator)->exp.operands[ 0]);
	  {
	    register tree link;
	    for (link = current_function_parm_tags;
		 link;
		 link = ((link)->common.chain))
	      ((((link)->list.value))->type.context) = type;
	  }
	}
      else if (((enum tree_code) (declarator)->common.code) == INDIRECT_REF)
	{
	  if (pedantic && ((enum tree_code) (type)->common.code) == FUNCTION_TYPE
	      && (constp || volatilep))
	    pedwarn ("ANSI C forbids const or volatile function types");
	  if (constp || volatilep)
	    type = c_build_type_variant (type, constp, volatilep);
	  constp = 0;
	  volatilep = 0;
	  size_varies = 0;
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
		      error ("invalid type modifier within pointer declarator");
		    }
		}
	      if (constp > 1)
		pedwarn ("duplicate `const'");
	      if (volatilep > 1)
		pedwarn ("duplicate `volatile'");
	    }
	  declarator = ((declarator)->exp.operands[ 0]);
	}
      else
	abort ();
    }
  if (specbits & (1 << (int) RID_TYPEDEF))
    {
      tree decl;
      if (pedantic && ((enum tree_code) (type)->common.code) == FUNCTION_TYPE
	  && (constp || volatilep))
	pedwarn ("ANSI C forbids const or volatile function types");
      if (constp || volatilep)
	type = c_build_type_variant (type, constp, volatilep);
      pop_obstacks ();
      decl = build_decl (TYPE_DECL, declarator, type);
      if ((specbits & (1 << (int) RID_SIGNED))
	  || (typedef_decl && (((typedef_decl))->decl.lang_flag_1)))
	(((decl))->decl.lang_flag_1) = 1;
      return decl;
    }
  if (type != 0 && typedef_type != 0
      && ((type)->type.main_variant) == ((typedef_type)->type.main_variant)
      && ((enum tree_code) (type)->common.code) == ARRAY_TYPE && ((type)->type.values) == 0)
    {
      type = build_array_type (((type)->common.type), 0);
      if (size_varies)
	((type)->type.lang_flag_1) = 1;
    }
  if (decl_context == TYPENAME)
    {
      if (pedantic && ((enum tree_code) (type)->common.code) == FUNCTION_TYPE
	  && (constp || volatilep))
	pedwarn ("ANSI C forbids const or volatile function types");
      if (constp || volatilep)
	type = c_build_type_variant (type, constp, volatilep);
      pop_obstacks ();
      return type;
    }
  if (((type)->type.main_variant) == void_type_node && decl_context != PARM)
    {
      error ("variable or field `%s' declared void",
	     ((declarator)->identifier.pointer));
      type = integer_type_node;
    }
  {
    register tree decl;
    if (decl_context == PARM)
      {
	tree type_as_written = type;
	tree main_type;
	if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
	  {
	    type = build_pointer_type
		    (c_build_type_variant (((type)->common.type), constp, volatilep));
	    volatilep = constp = 0;
	    size_varies = 0;
	  }
	else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
	  {
	    if (pedantic && (constp || volatilep))
	      pedwarn ("ANSI C forbids const or volatile function types");
	    type = build_pointer_type (c_build_type_variant (type, constp, volatilep));
	    volatilep = constp = 0;
	  }
	decl = build_decl (PARM_DECL, declarator, type);
	if (size_varies)
	  ((decl)->decl.lang_flag_0) = 1;
	((decl)->decl.initial)    = type;
	main_type = ((type)->type.main_variant);
	if (main_type == float_type_node)
	  ((decl)->decl.initial)    = double_type_node;
	else if (  (((enum tree_code) ((main_type))->common.code) == INTEGER_TYPE				   && (((main_type)->type.main_variant) == char_type_node			       || ((main_type)->type.main_variant) == signed_char_type_node	       || ((main_type)->type.main_variant) == unsigned_char_type_node	       || ((main_type)->type.main_variant) == short_integer_type_node	       || ((main_type)->type.main_variant) == short_unsigned_type_node)))
	  {
	    if (((type)->type.precision) == ((integer_type_node)->type.precision)
		&& ((type)->common.unsigned_flag))
	      ((decl)->decl.initial)    = unsigned_type_node;
	    else
	      ((decl)->decl.initial)    = integer_type_node;
	  }
	((decl)->decl.result) = type_as_written;
      }
    else if (decl_context == FIELD)
      {
	if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
	  {
	    error ("field `%s' declared as a function",
		   ((declarator)->identifier.pointer));
	    type = build_pointer_type (type);
	  }
	else if (((enum tree_code) (type)->common.code) != ERROR_MARK && ((type)->type.size) == 0)
	  {
	    error ("field `%s' has incomplete type",
		   ((declarator)->identifier.pointer));
	    type = error_mark_node;
	  }
	if (((enum tree_code) (type)->common.code) == ARRAY_TYPE && (constp || volatilep))
	  {
	    type = build_array_type (c_build_type_variant (((type)->common.type),
							   constp, volatilep),
				     ((type)->type.values));
	  }
	decl = build_decl (FIELD_DECL, declarator, type);
	if (size_varies)
	  ((decl)->decl.lang_flag_0) = 1;
      }
    else if (((enum tree_code) (type)->common.code) == FUNCTION_TYPE)
      {
	int extern_ref = (!(specbits & (1 << (int) RID_AUTO))
			  || current_binding_level == global_binding_level);
	if (specbits & (1 << (int) RID_AUTO)
	    && (pedantic || current_binding_level == global_binding_level))
	  pedwarn ("invalid storage class for function `%s'",
		 ((declarator)->identifier.pointer));
	if (specbits & (1 << (int) RID_REGISTER))
	  error ("invalid storage class for function `%s'",
		 ((declarator)->identifier.pointer));
	if (current_binding_level != global_binding_level
	    && (specbits & ((1 << (int) RID_STATIC) | (1 << (int) RID_INLINE)))
	    && pedantic)
	  pedwarn ("invalid storage class for function `%s'",
		   ((declarator)->identifier.pointer));
	if (extern_ref && allocation_temporary_p ())
	  end_temporary_allocation ();
	decl = build_decl (FUNCTION_DECL, declarator, type);
	if (pedantic && (constp || volatilep)
	    && ! ((decl)->decl.in_system_header_flag))
	  pedwarn ("ANSI C forbids const or volatile functions");
	if (extern_ref)
	  ((decl)->decl.external_flag) = 1;
	((decl)->common.public_flag)
	  = !(specbits & ((1 << (int) RID_STATIC) | (1 << (int) RID_AUTO)));
	if (inlinep)
	  {
	    tree last = tree_last (((type)->type.values));
	    if (! strcmp (((declarator)->identifier.pointer), "main"))
	      warning ("cannot inline function `main'");
	    else if (last && (((((last)->list.value))->type.main_variant)
			      != void_type_node))
	      warning ("inline declaration ignored for function with `...'");
	    else
	      ((decl)->decl.inline_flag) = 1;
	    if (specbits & (1 << (int) RID_EXTERN))
	      current_extern_inline = 1;
	  }
      }
    else
      {
	int extern_ref = !initialized && (specbits & (1 << (int) RID_EXTERN));
	if (((enum tree_code) (type)->common.code) == ARRAY_TYPE && (constp || volatilep))
	  {
	    type = build_array_type (c_build_type_variant (((type)->common.type),
							   constp, volatilep),
				     ((type)->type.values));
	  }
	if (extern_ref && allocation_temporary_p ())
	  end_temporary_allocation ();
	decl = build_decl (VAR_DECL, declarator, type);
	if (size_varies)
	  ((decl)->decl.lang_flag_0) = 1;
	if (inlinep)
	  pedwarn_with_decl (decl, "variable `%s' declared `inline'");
	((decl)->decl.external_flag) = extern_ref;
	if (current_binding_level == global_binding_level)
	  {
	    ((decl)->common.public_flag)
	      = !(specbits
		  & ((1 << (int) RID_STATIC) | (1 << (int) RID_REGISTER)));
	    ((decl)->common.static_flag) = ! ((decl)->decl.external_flag);
	  }
	else
	  {
	    ((decl)->common.static_flag) = (specbits & (1 << (int) RID_STATIC)) != 0;
	    ((decl)->common.public_flag) = ((decl)->decl.external_flag);
	  }
      }
    if (specbits & (1 << (int) RID_REGISTER))
      ((decl)->decl.regdecl_flag) = 1;
    if (constp)
      ((decl)->common.readonly_flag) = 1;
    if (volatilep)
      {
	((decl)->common.side_effects_flag) = 1;
	((decl)->common.volatile_flag) = 1;
      }
    if (((((decl)->common.type))->common.lang_flag_2))
      mark_addressable (decl);
    pop_obstacks ();
    return decl;
  }
}
tree
c_build_type_variant (type, constp, volatilep)
     tree type;
     int constp, volatilep;
{
  if (((enum tree_code) (type)->common.code) == ARRAY_TYPE)
    type = build_array_type (c_build_type_variant (((type)->common.type),
						   constp, volatilep),
			     ((type)->type.values));
  return build_type_variant (type, constp, volatilep);
}
static tree
grokparms (parms_info, funcdef_flag)
     tree parms_info;
     int funcdef_flag;
{
  tree first_parm = ((parms_info)->common.chain);
  last_function_parms = ((parms_info)->list.purpose);
  last_function_parm_tags = ((parms_info)->list.value);
  if (warn_strict_prototypes && first_parm == 0 && !funcdef_flag
      && !in_system_header)
    warning ("function declaration isn't a prototype");
  if (first_parm != 0
      && ((enum tree_code) (((first_parm)->list.value))->common.code) == IDENTIFIER_NODE)
    {
      if (! funcdef_flag)
	pedwarn ("parameter names (without types) in function declaration");
      last_function_parms = first_parm;
      return 0;
    }
  else
    {
      tree parm;
      tree typelt;
	for (parm = last_function_parms, typelt = first_parm;
	     parm;
	     parm = ((parm)->common.chain))
	  if (((enum tree_code) (parm)->common.code) == PARM_DECL)
	    {
	      tree type = ((typelt)->list.value);
	      if (((type)->type.size) == 0)
		{
		  if (funcdef_flag && ((parm)->decl.name) != 0)
		    error ("parameter `%s' has incomplete type",
			   ((((parm)->decl.name))->identifier.pointer));
		  else
		    warning ("parameter has incomplete type");
		  if (funcdef_flag)
		    {
		      ((typelt)->list.value) = error_mark_node;
		      ((parm)->common.type) = error_mark_node;
		    }
		}
	      typelt = ((typelt)->common.chain);
	    }
      if (first_parm && ! ((first_parm)->common.permanent_flag))
	{
	  tree result = 0;
	  for (typelt = first_parm; typelt; typelt = ((typelt)->common.chain))
	    result = saveable_tree_cons ((tree) 0, ((typelt)->list.value),
					 result);
	  return nreverse (result);
	}
      else
	return first_parm;
    }
}
tree
get_parm_info (void_at_end)
     int void_at_end;
{
  register tree decl, t;
  register tree types = 0;
  int erred = 0;
  tree tags = gettags ();
  tree parms = getdecls ();
  tree new_parms = 0;
  tree order = current_binding_level->parm_order;
  if (void_at_end && parms != 0
      && ((parms)->common.chain) == 0
      && ((((parms)->common.type))->type.main_variant) == void_type_node
      && ((parms)->decl.name) == 0)
    {
      parms = (tree) 0;
      storedecls ((tree) 0);
      return saveable_tree_cons ((tree) 0, (tree) 0,
				 saveable_tree_cons ((tree) 0, void_type_node, (tree) 0));
    }
  for (decl = parms; decl; )
    {
      tree next = ((decl)->common.chain);
      if (((enum tree_code) (decl)->common.code) != PARM_DECL)
	{
	  ((decl)->common.chain) = new_parms;
	  new_parms = decl;
	}
      else if (((decl)->common.asm_written_flag))
	{
	  error_with_decl (decl, "parameter `%s' has just a forward declaration");
	  ((decl)->common.chain) = new_parms;
	  new_parms = decl;
	}
      decl = next;
    }
  for (t = order; t; t = ((t)->common.chain))
    {
      if (((t)->common.chain))
	((((t)->list.value))->common.chain) = ((((t)->common.chain))->list.value);
      else
	((((t)->list.value))->common.chain) = 0;
    }
  new_parms = chainon (order ? nreverse (((order)->list.value)) : 0,
		       new_parms);
  storedecls (new_parms);
  for (decl = new_parms; decl; decl = ((decl)->common.chain))
    if (((enum tree_code) (decl)->common.code) == PARM_DECL)
      {
	tree type = ((decl)->common.type);
	((decl)->decl.initial)    = type;
	types = saveable_tree_cons ((tree) 0, ((decl)->common.type), types);
	if (((((types)->list.value))->type.main_variant) == void_type_node && ! erred
	    && ((decl)->decl.name) == 0)
	  {
	    error ("`void' in parameter list must be the entire list");
	    erred = 1;
	  }
      }
  if (void_at_end)
    return saveable_tree_cons (new_parms, tags,
			       nreverse (saveable_tree_cons ((tree) 0, void_type_node, types)));
  return saveable_tree_cons (new_parms, tags, nreverse (types));
}
void
parmlist_tags_warning ()
{
  tree elt;
  static int already;
  for (elt = current_binding_level->tags; elt; elt = ((elt)->common.chain))
    {
      enum tree_code code = ((enum tree_code) (((elt)->list.value))->common.code);
      if (code == UNION_TYPE && !pedantic)
	continue;
      if (((elt)->list.purpose) != 0)
	warning ("`%s %s' declared inside parameter list",
		 (code == RECORD_TYPE ? "struct"
		  : code == UNION_TYPE ? "union"
		  : "enum"),
		 ((((elt)->list.purpose))->identifier.pointer));
      else
	warning ("anonymous %s declared inside parameter list",
		 (code == RECORD_TYPE ? "struct"
		  : code == UNION_TYPE ? "union"
		  : "enum"));
      if (! already)
	{
	  warning ("its scope is only this definition or declaration,");
	  warning ("which is probably not what you want.");
	  already = 1;
	}
    }
}
tree
xref_tag (code, name)
     enum tree_code code;
     tree name;
{
  int temporary = allocation_temporary_p ();
  register tree ref = lookup_tag (code, name, current_binding_level, 0);
  if (ref)
    return ref;
  push_obstacks_nochange ();
  if (current_binding_level == global_binding_level && temporary)
    end_temporary_allocation ();
  ref = make_node (code);
  if (code == ENUMERAL_TYPE)
    {
      if (pedantic)
	pedwarn ("ANSI C forbids forward references to `enum' types");
      ((ref)->type.mode) = ((unsigned_type_node)->type.mode);
      ((ref)->type.align) = ((unsigned_type_node)->type.align);
      ((ref)->common.unsigned_flag) = 1;
      ((ref)->type.precision) = ((unsigned_type_node)->type.precision);
      ((ref)->type.minval) = ((unsigned_type_node)->type.minval);
      ((ref)->type.maxval) = ((unsigned_type_node)->type.maxval);
    }
  pushtag (name, ref);
  pop_obstacks ();
  return ref;
}
tree
start_struct (code, name)
     enum tree_code code;
     tree name;
{
  register tree ref = 0;
  push_obstacks_nochange ();
  if (current_binding_level == global_binding_level)
    end_temporary_allocation ();
  if (name != 0)
    ref = lookup_tag (code, name, current_binding_level, 1);
  if (ref && ((enum tree_code) (ref)->common.code) == code)
    {
      ((ref)->type.lang_flag_0) = 1;
      if (((ref)->type.values))
	error ((code == UNION_TYPE ? "redefinition of `union %s'"
		: "redefinition of `struct %s'"),
	       ((name)->identifier.pointer));
      return ref;
    }
  ref = make_node (code);
  pushtag (name, ref);
  ((ref)->type.lang_flag_0) = 1;
  return ref;
}
tree
grokfield (filename, line, declarator, declspecs, width)
     char *filename;
     int line;
     tree declarator, declspecs, width;
{
  tree value;
  push_obstacks_nochange ();
  value = grokdeclarator (declarator, declspecs, width ? BITFIELD : FIELD, 0);
  finish_decl (value, (tree) 0, (tree) 0);
  ((value)->decl.initial) = width;
  return value;
}
static int
field_decl_cmp (x, y)
     tree *x, *y;
{
  return (long)((*x)->decl.name) - (long)((*y)->decl.name);
}
tree
finish_struct (t, fieldlist)
     register tree t, fieldlist;
{
  register tree x;
  int old_momentary;
  int toplevel = global_binding_level == current_binding_level;
  ((t)->type.size) = 0;
  if (! (((enum tree_code) (t)->common.code) == UNION_TYPE && ((t)->type.name) == 0) && !pedantic)
    if (in_parm_level_p ())
      {
	if (pedantic)
	  pedwarn ((((enum tree_code) (t)->common.code) == UNION_TYPE ? "union defined inside parms"
		    : "structure defined inside parms"));
	else
	  warning ((((enum tree_code) (t)->common.code) == UNION_TYPE ? "union defined inside parms"
		    : "structure defined inside parms"));
      }
  old_momentary = suspend_momentary ();
  if (fieldlist == 0 && pedantic)
    pedwarn ((((enum tree_code) (t)->common.code) == UNION_TYPE ? "union has no members"
	      : "structure has no members"));
  for (x = fieldlist; x; x = ((x)->common.chain))
    {
      ((x)->decl.context) = t;
      ((x)->decl.saved_insns.i) = 0;
      if (((x)->common.readonly_flag))
	((t)->common.lang_flag_1) = 1;
      else
	{
	  tree t1 = ((x)->common.type);
	  while (((enum tree_code) (t1)->common.code) == ARRAY_TYPE)
	    t1 = ((t1)->common.type);
	  if ((((enum tree_code) (t1)->common.code) == RECORD_TYPE || ((enum tree_code) (t1)->common.code) == UNION_TYPE)
	      && ((t1)->common.lang_flag_1))
	    ((t)->common.lang_flag_1) = 1;
	}
      if (((x)->common.volatile_flag))
	((t)->common.lang_flag_2) = 1;
      if (((x)->decl.lang_flag_0))
	((t)->type.lang_flag_1) = 1;
      if (((x)->common.type) == t)
	error ("nested redefinition of `%s'",
	       ((((t)->type.name))->identifier.pointer));
      if (((x)->decl.initial))
	  while ((((enum tree_code) (((x)->decl.initial))->common.code) == NOP_EXPR					  || ((enum tree_code) (((x)->decl.initial))->common.code) == CONVERT_EXPR				  || ((enum tree_code) (((x)->decl.initial))->common.code) == NON_LVALUE_EXPR)			 && (((((((x)->decl.initial))->common.type))->type.mode)				     == ((((((((x)->decl.initial))->exp.operands[ 0]))->common.type))->type.mode)))	    (((x)->decl.initial)) = ((((x)->decl.initial))->exp.operands[ 0]);;
      if (((x)->decl.initial) && ((enum tree_code) (((x)->decl.initial))->common.code) != INTEGER_CST)
	{
	  error_with_decl (x, "bit-field `%s' width not an integer constant");
	  ((x)->decl.initial) = 0;
	}
      if (((x)->decl.initial)
	  && ((enum tree_code) (((x)->common.type))->common.code) != INTEGER_TYPE
	  && ((enum tree_code) (((x)->common.type))->common.code) != ENUMERAL_TYPE)
	{
	  error_with_decl (x, "bit-field `%s' has invalid type");
	  ((x)->decl.initial) = 0;
	}
      if (((x)->decl.initial) && pedantic
	  && ((((x)->common.type))->type.main_variant) != integer_type_node
	  && ((((x)->common.type))->type.main_variant) != unsigned_type_node)
	pedwarn_with_decl (x, "bit-field `%s' type invalid in ANSI C");
      if (((x)->decl.initial))
	{
	  unsigned int width = ((((x)->decl.initial))->int_cst.int_cst_low);
	  if (tree_int_cst_lt (((x)->decl.initial), integer_zero_node))
	    {
	      ((x)->decl.initial) = 0;
	      error_with_decl (x, "negative width in bit-field `%s'");
	    }
	  else if (((((x)->decl.initial))->int_cst.int_cst_high) != 0
		   || width > ((((x)->common.type))->type.precision))
	    {
	      ((x)->decl.initial) = 0;
	      pedwarn_with_decl (x, "width of `%s' exceeds its type");
	    }
	  else if (width == 0 && ((x)->decl.name) != 0)
	    {
	      error_with_decl (x, "zero width for bit-field `%s'");
	      ((x)->decl.initial) = 0;
	    }
	}
      if (((x)->decl.initial))
	{
	  register int width = ((((x)->decl.initial))->int_cst.int_cst_low);
	  ((x)->decl.saved_insns.i) = width;
	  ((x)->decl.bit_field_flag) = 1;
	  ((x)->decl.initial) = 0;
	  if (width == 0)
	    {
	      ((x)->decl.frame_size) = ((((x)->decl.frame_size)) > ( 64) ? (((x)->decl.frame_size)) : ( 64));
	      ((x)->decl.frame_size) = ((((x)->decl.frame_size)) > (
				    ((((x)->common.type))->type.align)) ? (((x)->decl.frame_size)) : (
				    ((((x)->common.type))->type.align)));
	    }
	}
      else
	{
	  int min_align = (((x)->decl.regdecl_flag) ? 8
			   : ((((x)->common.type))->type.align));
	  ((x)->decl.frame_size) = ((((x)->decl.frame_size)) > ( min_align) ? (((x)->decl.frame_size)) : ( min_align));
	}
    }
  for (x = fieldlist; x && ((x)->common.chain);)
    if (((((x)->common.chain))->decl.name) == 0)
      x = ((x)->common.chain);
    else
      {
	register tree y = fieldlist;
	while (1)
	  {
	    if (((y)->decl.name) == ((((x)->common.chain))->decl.name))
	      break;
	    if (y == x)
	      break;
	    y = ((y)->common.chain);
	  }
	if (((y)->decl.name) == ((((x)->common.chain))->decl.name))
	  {
	    error_with_decl (((x)->common.chain), "duplicate member `%s'");
	    ((x)->common.chain) = ((((x)->common.chain))->common.chain);
	  }
	else x = ((x)->common.chain);
      }
  ((t)->type.values) = fieldlist;
  layout_type (t);
  while (fieldlist
	 && ((fieldlist)->decl.initial))
    fieldlist = ((fieldlist)->common.chain);
  for (x = fieldlist; x;)
    {
      if (((x)->common.chain) && ((((x)->common.chain))->decl.initial))
	((x)->common.chain) = ((((x)->common.chain))->common.chain);
      else x = ((x)->common.chain);
    }
  ((t)->type.values) = fieldlist;
  {
    int len = 0;
    for (x = fieldlist; x; x = ((x)->common.chain))
      {
	if (len > 15)
	  break;
	len += 1;
      }
    if (len > 15)
      {
	tree *field_array;
	char *space;
	len += list_length (x);
	if (allocation_temporary_p ())
	  space = savealloc (sizeof (struct lang_type) + len * sizeof (tree));
	else
	  space = oballoc (sizeof (struct lang_type) + len * sizeof (tree));
	((t)->type.lang_specific) = (struct lang_type *) space;
	((t)->type.lang_specific)->len = len;
	field_array = &((t)->type.lang_specific)->elts[0];
	len = 0;
	for (x = fieldlist; x; x = ((x)->common.chain))
	  field_array[len++] = x;
	qsort (field_array, len, sizeof (tree), field_decl_cmp);
      }
  }
  for (x = ((t)->type.main_variant); x; x = ((x)->type.next_variant))
    {
      ((x)->type.values) = ((t)->type.values);
      ((x)->type.lang_specific) = ((t)->type.lang_specific);
      ((x)->type.align) = ((t)->type.align);
    }
  for (x = fieldlist; x; x = ((x)->common.chain))
    if (((x)->decl.bit_field_flag)
	&&   (((enum tree_code) ((((x)->common.type)))->common.code) == INTEGER_TYPE				   && (((((x)->common.type))->type.main_variant) == char_type_node			       || ((((x)->common.type))->type.main_variant) == signed_char_type_node	       || ((((x)->common.type))->type.main_variant) == unsigned_char_type_node	       || ((((x)->common.type))->type.main_variant) == short_integer_type_node	       || ((((x)->common.type))->type.main_variant) == short_unsigned_type_node)))
    {
      tree type = ((x)->common.type);
      if (((type)->common.unsigned_flag)
	  && (flag_traditional
	      || (((type)->type.precision)
		  == ((integer_type_node)->type.precision))))
	((x)->common.type) = unsigned_type_node;
      else
	((x)->common.type) = integer_type_node;
    }
  if (current_binding_level->n_incomplete != 0)
    {
      tree decl;
      for (decl = current_binding_level->names; decl; decl = ((decl)->common.chain))
	{
	  if (((decl)->common.type) == t
	      && ((enum tree_code) (decl)->common.code) != TYPE_DECL)
	    {
	      layout_decl (decl, 0);
	      maybe_objc_check_decl (decl);
	      rest_of_decl_compilation (decl, ((char *)0), toplevel, 0);
	      if (! toplevel)
		expand_decl (decl);
	      --current_binding_level->n_incomplete;
	    }
	  else if (((((decl)->common.type))->type.size) == 0
		   && ((enum tree_code) (((decl)->common.type))->common.code) == ARRAY_TYPE)
	    {
	      tree element = ((decl)->common.type);
	      while (((enum tree_code) (element)->common.code) == ARRAY_TYPE)
		element = ((element)->common.type);
	      if (element == t)
		layout_array_type (((decl)->common.type));
	    }
	}
    }
  resume_momentary (old_momentary);
  rest_of_type_compilation (t, toplevel);
  pop_obstacks ();
  return t;
}
static void
layout_array_type (t)
     tree t;
{
  if (((enum tree_code) (((t)->common.type))->common.code) == ARRAY_TYPE)
    layout_array_type (((t)->common.type));
  layout_type (t);
}
tree
start_enum (name)
     tree name;
{
  register tree enumtype = 0;
  if (name != 0)
    enumtype = lookup_tag (ENUMERAL_TYPE, name, current_binding_level, 1);
  push_obstacks_nochange ();
  if (current_binding_level == global_binding_level)
    end_temporary_allocation ();
  if (enumtype == 0 || ((enum tree_code) (enumtype)->common.code) != ENUMERAL_TYPE)
    {
      enumtype = make_node (ENUMERAL_TYPE);
      pushtag (name, enumtype);
    }
  ((enumtype)->type.lang_flag_0) = 1;
  if (((enumtype)->type.values) != 0)
    {
      error ("redeclaration of `enum %s'", ((name)->identifier.pointer));
      ((enumtype)->type.values) = 0;
    }
  enum_next_value = integer_zero_node;
  enum_overflow = 0;
  return enumtype;
}
tree
finish_enum (enumtype, values)
     register tree enumtype, values;
{
  register tree pair;
  tree minnode = 0, maxnode = 0;
  register int maxvalue = 0;
  register int minvalue = 0;
  register int i;
  unsigned precision = 0;
  int toplevel = global_binding_level == current_binding_level;
  int temporary = allocation_temporary_p ();
  if (in_parm_level_p ())
    warning ("enum defined inside parms");
  for (pair = values; pair; pair = ((pair)->common.chain))
    {
      tree value = ((pair)->list.value);
      if (pair == values)
	minnode = maxnode = ((pair)->list.value);
      else
	{
	  if (tree_int_cst_lt (maxnode, value))
	    maxnode = value;
	  if (tree_int_cst_lt (value, minnode))
	    minnode = value;
	}
    }
  ((enumtype)->type.minval) = minnode;
  ((enumtype)->type.maxval) = maxnode;
  if (((minnode)->int_cst.int_cst_high) >= 0
      ? tree_int_cst_lt (((unsigned_type_node)->type.maxval), maxnode)
      : (tree_int_cst_lt (minnode, ((integer_type_node)->type.minval))
	 || tree_int_cst_lt (((integer_type_node)->type.maxval), maxnode)))
    precision = ((long_long_integer_type_node)->type.precision);
  else
    {
      maxvalue = ((maxnode)->int_cst.int_cst_low);
      minvalue = ((minnode)->int_cst.int_cst_low);
      if (maxvalue > 0)
	precision = floor_log2_wide ((int) (maxvalue)) + 1;
      if (minvalue < 0)
	{
	  unsigned negprecision = floor_log2_wide ((int) (-minvalue - 1)) + 1;
	  if (negprecision > precision)
	    precision = negprecision;
	  precision += 1;	
	}
      if (!precision)
	precision = 1;
    }
  if (flag_short_enums || precision > ((integer_type_node)->type.precision))
    ((enumtype)->type.precision) = ((type_for_size (precision, 1))->type.precision);
  else
    ((enumtype)->type.precision) = ((integer_type_node)->type.precision);
  ((enumtype)->type.size) = 0;
  layout_type (enumtype);
  ((enumtype)->common.unsigned_flag) = ! tree_int_cst_lt (minnode, integer_zero_node);
  if (((enumtype)->type.precision) <= ((integer_type_node)->type.precision))
    for (pair = values; pair; pair = ((pair)->common.chain))
      {
	((((pair)->list.purpose))->common.type) = enumtype;
	((((pair)->list.purpose))->decl.size) = ((enumtype)->type.size);
	if (((enum tree_code) (((pair)->list.purpose))->common.code) != FUNCTION_DECL)
	  ((((pair)->list.purpose))->decl.frame_size) = ((enumtype)->type.align);
      }
  for (pair = values; pair; pair = ((pair)->common.chain))
    ((pair)->list.purpose) = ((((pair)->list.purpose))->decl.name);
  ((enumtype)->type.values) = values;
  rest_of_type_compilation (enumtype, toplevel);
  pop_obstacks ();
  return enumtype;
}
tree
build_enumerator (name, value)
     tree name, value;
{
  register tree decl;
  if (value)
      while ((((enum tree_code) (value)->common.code) == NOP_EXPR					  || ((enum tree_code) (value)->common.code) == CONVERT_EXPR				  || ((enum tree_code) (value)->common.code) == NON_LVALUE_EXPR)			 && (((value)->common.type)						     == ((((value)->exp.operands[ 0]))->common.type)))		    (value) = ((value)->exp.operands[ 0]);;
  if (value != 0 && ((enum tree_code) (value)->common.code) != INTEGER_CST)
    {
      error ("enumerator value for `%s' not integer constant",
	     ((name)->identifier.pointer));
      value = 0;
    }
  if (value == 0)
    {
      value = enum_next_value;
      if (enum_overflow)
	error ("overflow in enumeration values");
    }
  if (pedantic && ! int_fits_type_p (value, integer_type_node))
    {
      pedwarn ("ANSI C restricts enumerator values to range of `int'");
      value = integer_zero_node;
    }
  enum_next_value = build_binary_op (PLUS_EXPR, value, integer_one_node, 0);
  enum_overflow = tree_int_cst_lt (enum_next_value, value);
  decl = build_decl (CONST_DECL, name, integer_type_node);
  ((decl)->decl.initial) = value;
  ((value)->common.type) = integer_type_node;
  pushdecl (decl);
  return saveable_tree_cons (decl, value, (tree) 0);
}
int
start_function (declspecs, declarator, nested)
     tree declarator, declspecs;
     int nested;
{
  tree decl1, old_decl;
  tree restype;
  current_function_returns_value = 0;  
  current_function_returns_null = 0;
  warn_about_return_type = 0;
  current_extern_inline = 0;
  c_function_varargs = 0;
  named_labels = 0;
  shadowed_labels = 0;
  decl1 = grokdeclarator (declarator, declspecs, FUNCDEF, 1);
  if (decl1 == 0)
    return 0;
  announce_function (decl1);
  if (((((((decl1)->common.type))->common.type))->type.size) == 0)
    {
      error ("return-type is an incomplete type");
      ((decl1)->common.type)
	= build_function_type (void_type_node,
			       ((((decl1)->common.type))->type.values));
    }
  if (warn_about_return_type)
    warning ("return-type defaults to `int'");
  current_function_parms = last_function_parms;
  current_function_parm_tags = last_function_parm_tags;
  ((decl1)->decl.initial) = error_mark_node;
  old_decl = lookup_name_current_level (((decl1)->decl.name));
  if (old_decl != 0 && ((enum tree_code) (((old_decl)->common.type))->common.code) == FUNCTION_TYPE
      && !((old_decl)->decl.bit_field_flag)
      && (((((((decl1)->common.type))->common.type))->type.main_variant)
	  == ((((((old_decl)->common.type))->common.type))->type.main_variant))
      && ((((decl1)->common.type))->type.values) == 0)
    ((decl1)->common.type) = ((old_decl)->common.type);
  if (warn_strict_prototypes
      && ((((decl1)->common.type))->type.values) == 0
      && !(old_decl != 0 && ((((old_decl)->common.type))->type.values) != 0))
    warning ("function declaration isn't a prototype");
  else if (warn_missing_prototypes
	   && ((decl1)->common.public_flag)
	   && !(old_decl != 0 && ((((old_decl)->common.type))->type.values) != 0))
    warning_with_decl (decl1, "no previous prototype for `%s'");
  else if (warn_missing_prototypes
	   && old_decl != 0 && ((old_decl)->common.used_flag)
	   && !(old_decl != 0 && ((((old_decl)->common.type))->type.values) != 0))
    warning_with_decl (decl1, "`%s' was used with no prototype before its definition");
  ((decl1)->decl.external_flag) = current_extern_inline;
  ((decl1)->common.static_flag) = 1;
  if (current_function_decl != 0)
    ((decl1)->common.public_flag) = 0;
  current_function_decl = pushdecl (decl1);
  pushlevel (0);
  declare_parm_level (1);
  current_binding_level->subblocks_tag_transparent = 1;
  make_function_rtl (current_function_decl);
  restype = ((((current_function_decl)->common.type))->common.type);
  if (  (((enum tree_code) ((restype))->common.code) == INTEGER_TYPE				   && (((restype)->type.main_variant) == char_type_node			       || ((restype)->type.main_variant) == signed_char_type_node	       || ((restype)->type.main_variant) == unsigned_char_type_node	       || ((restype)->type.main_variant) == short_integer_type_node	       || ((restype)->type.main_variant) == short_unsigned_type_node)))
    {
      if (((restype)->common.unsigned_flag)
	  && (flag_traditional
	      || (((restype)->type.precision)
		  == ((integer_type_node)->type.precision))))
	restype = unsigned_type_node;
      else
	restype = integer_type_node;
    }
  ((current_function_decl)->decl.result)
    = build_decl (RESULT_DECL, (tree) 0, restype);
  if (!nested)
    temporary_allocation ();
  if (((((current_function_decl)->decl.assembler_name))->common.addressable_flag))
    ((current_function_decl)->common.addressable_flag) = 1;
  return 1;
}
void
c_mark_varargs ()
{
  c_function_varargs = 1;
}
void
store_parm_decls ()
{
  register tree fndecl = current_function_decl;
  register tree parm;
  tree specparms = current_function_parms;
  tree parmtags = current_function_parm_tags;
  register tree parmdecls = getdecls ();
  tree nonparms = 0;
  int prototype = 0;
  if (specparms != 0 && ((enum tree_code) (specparms)->common.code) != TREE_LIST)
    {
      register tree next;
      tree others = 0;
      prototype = 1;
      if (parmdecls != 0)
	{
	  tree decl, link;
	  error_with_decl (fndecl,
			   "parm types given both in parmlist and separately");
	  for (decl = current_binding_level->names;
	       decl; decl = ((decl)->common.chain))
	    if (((decl)->decl.name))
	        (((struct lang_identifier *)(((decl)->decl.name)))->local_value) = 0;
	  for (link = current_binding_level->shadowed;
	       link; link = ((link)->common.chain))
	      (((struct lang_identifier *)(((link)->list.purpose)))->local_value) = ((link)->list.value);
	  current_binding_level->names = 0;
	  current_binding_level->shadowed = 0;
	}
      specparms = nreverse (specparms);
      for (parm = specparms; parm; parm = next)
	{
	  next = ((parm)->common.chain);
	  if (((enum tree_code) (parm)->common.code) == PARM_DECL)
	    {
	      if (((parm)->decl.name) == 0)
		error_with_decl (parm, "parameter name omitted");
	      else if (((((parm)->common.type))->type.main_variant) == void_type_node)
		{
		  error_with_decl (parm, "parameter `%s' declared void");
		  ((parm)->common.type) = error_mark_node;
		}
	      pushdecl (parm);
	    }
	  else
	    {
	      ((parm)->common.chain) = 0;
	      others = chainon (others, parm);
	    }
	}
      ((fndecl)->decl.arguments) = getdecls ();
      for (parm = others; parm; parm = next)
	{
	  next = ((parm)->common.chain);
	  if (((parm)->decl.name) == 0)
	    ;
	  else if (((((parm)->common.type))->type.main_variant) == void_type_node)
	    ;
	  else if (((enum tree_code) (parm)->common.code) != PARM_DECL)
	    pushdecl (parm);
	}
      storetags (chainon (parmtags, gettags ()));
    }
  else
    {
      for (parm = parmdecls; parm; parm = ((parm)->common.chain))
	((parm)->decl.result) = 0;
      for (parm = specparms; parm; parm = ((parm)->common.chain))
	{
	  register tree tail, found = 0;
	  if (((parm)->list.value) == 0)
	    {
	      error_with_decl (fndecl, "parameter name missing from parameter list");
	      ((parm)->list.purpose) = 0;
	      continue;
	    }
	  for (tail = parmdecls; tail; tail = ((tail)->common.chain))
	    if (((tail)->decl.name) == ((parm)->list.value)
		&& ((enum tree_code) (tail)->common.code) == PARM_DECL)
	      {
		found = tail;
		break;
	      }
	  if (found && ((found)->decl.result) != 0)
	    {
	      error_with_decl (found, "multiple parameters named `%s'");
	      found = 0;
	    }
	  if (found && ((((found)->common.type))->type.main_variant) == void_type_node)
	    {
	      error_with_decl (found, "parameter `%s' declared void");
	      ((found)->common.type) = integer_type_node;
	      ((found)->decl.initial)    = integer_type_node;
	      layout_decl (found, 0);
	    }
	  if (found && flag_traditional
	      && ((((found)->common.type))->type.main_variant) == float_type_node)
	    ((found)->common.type) = double_type_node;
	  if (!found)
	    {
	      found = build_decl (PARM_DECL, ((parm)->list.value),
				  integer_type_node);
	      ((found)->decl.initial)    = ((found)->common.type);
	      ((found)->decl.linenum) = ((fndecl)->decl.linenum);
	      ((found)->decl.filename) = ((fndecl)->decl.filename);
	      if (extra_warnings)
		warning_with_decl (found, "type of `%s' defaults to `int'");
	      pushdecl (found);
	    }
	  ((parm)->list.purpose) = found;
	  ((found)->decl.result) = error_mark_node;
	}
      nonparms = 0;
      for (parm = parmdecls; parm; )
	{
	  tree next = ((parm)->common.chain);
	  ((parm)->common.chain) = 0;
	  if (((enum tree_code) (parm)->common.code) != PARM_DECL)
	    nonparms = chainon (nonparms, parm);
	  else
	    {
	      if (((((parm)->common.type))->type.size) == 0)
	        {
	          error_with_decl (parm, "parameter `%s' has incomplete type");
	          ((parm)->common.type) = error_mark_node;
	        }
	      if (((parm)->decl.result) == 0)
	        {
	          error_with_decl (parm,
			           "declaration for parameter `%s' but no such parameter");
	          specparms
		    = chainon (specparms,
			       tree_cons (parm, (tree) 0, (tree) 0));
		}
	    }
	  parm = next;
	}
      parm = specparms;
      ((fndecl)->decl.arguments) = 0;
      {
	register tree last;
	for (last = 0; parm; parm = ((parm)->common.chain))
	  if (((parm)->list.purpose))
	    {
	      if (last == 0)
		((fndecl)->decl.arguments) = ((parm)->list.purpose);
	      else
		((last)->common.chain) = ((parm)->list.purpose);
	      last = ((parm)->list.purpose);
	      ((last)->common.chain) = 0;
	    }
      }
      if (((((fndecl)->common.type))->type.values))
	{
	  register tree type;
	  for (parm = ((fndecl)->decl.arguments),
	       type = ((((fndecl)->common.type))->type.values);
	       parm || (type && (((((type)->list.value))->type.main_variant)
				 != void_type_node));
	       parm = ((parm)->common.chain), type = ((type)->common.chain))
	    {
	      if (parm == 0 || type == 0
		  || ((((type)->list.value))->type.main_variant) == void_type_node)
		{
		  error ("number of arguments doesn't match prototype");
		  break;
		}
	      if (! comptypes (((parm)->decl.initial)   , ((type)->list.value)))
		{
		  if (((((parm)->common.type))->type.main_variant)
		      == ((((type)->list.value))->type.main_variant))
		    {
		      ((parm)->decl.initial)    = ((parm)->common.type);
		      if (pedantic)
			warning ("promoted argument `%s' doesn't match prototype",
				 ((((parm)->decl.name))->identifier.pointer));
		    }
		  else if (! (flag_traditional
			      && ((((parm)->common.type))->type.main_variant) == integer_type_node
			      && ((((type)->list.value))->type.main_variant) == unsigned_type_node))
		    error ("argument `%s' doesn't match prototype",
			   ((((parm)->decl.name))->identifier.pointer));
		}
	    }
	  ((((fndecl)->common.type))->type.noncopied_parts) = 0;
	}
      else
	{
	  register tree actual, type;
	  register tree last = 0;
	  for (parm = ((fndecl)->decl.arguments); parm; parm = ((parm)->common.chain))
	    {
	      type = perm_tree_cons ((tree) 0, ((parm)->decl.initial)   ,
				     (tree) 0);
	      if (last)
		((last)->common.chain) = type;
	      else
		actual = type;
	      last = type;
	    }
	  type = perm_tree_cons ((tree) 0, void_type_node, (tree) 0);
	  if (last)
	    ((last)->common.chain) = type;
	  else
	    actual = type;
	  ((fndecl)->common.type) = build_type_copy (((fndecl)->common.type));
	  ((((fndecl)->common.type))->type.noncopied_parts) = actual;
	}
      storedecls (chainon (nonparms, ((fndecl)->decl.arguments)));
    }
  keep_next_if_subblocks = 1;
  gen_aux_info_record (fndecl, 1, 0, prototype);
  init_function_start (fndecl, input_filename, lineno);
  if (c_function_varargs)
    mark_varargs ();
  declare_function_name ();
  expand_function_start (fndecl, 0);
  if (((fndecl)->decl.name)
      && strcmp (((((fndecl)->decl.name))->identifier.pointer), "main") == 0
      && ((fndecl)->decl.context) == (tree) 0)
    expand_main_function ();
}
tree
combine_parm_decls (specparms, parmlist, void_at_end)
     tree specparms, parmlist;
     int void_at_end;
{
  register tree fndecl = current_function_decl;
  register tree parm;
  tree parmdecls = ((parmlist)->list.purpose);
  tree nonparms = ((parmlist)->list.value);
  tree types = 0;
  for (parm = parmdecls; parm; parm = ((parm)->common.chain))
    ((parm)->decl.result) = 0;
  for (parm = specparms; parm; parm = ((parm)->common.chain))
    {
      register tree tail, found = 0;
      for (tail = parmdecls; tail; tail = ((tail)->common.chain))
	if (((tail)->decl.name) == ((parm)->list.value))
	  {
	    found = tail;
	    break;
	  }
      if (found && ((found)->decl.result) != 0)
	{
	  error_with_decl (found, "multiple parameters named `%s'");
	  found = 0;
	}
      if (found && ((((found)->common.type))->type.main_variant) == void_type_node)
	{
	  error_with_decl (found, "parameter `%s' declared void");
	  ((found)->common.type) = integer_type_node;
	  ((found)->decl.initial)    = integer_type_node;
	  layout_decl (found, 0);
	}
      if (found && flag_traditional
	  && ((((found)->common.type))->type.main_variant) == float_type_node)
	((found)->common.type) = double_type_node;
      if (!found)
	{
	  found = build_decl (PARM_DECL, ((parm)->list.value),
			      integer_type_node);
	  ((found)->decl.initial)    = ((found)->common.type);
	  ((found)->decl.linenum) = ((fndecl)->decl.linenum);
	  ((found)->decl.filename) = ((fndecl)->decl.filename);
	  error (found, "type of parameter `%s' is not declared");
	  pushdecl (found);
	}
      ((parm)->list.purpose) = found;
      ((found)->decl.result) = error_mark_node;
    }
  for (parm = parmdecls; parm; )
    {
      tree next = ((parm)->common.chain);
      ((parm)->common.chain) = 0;
      if (((((parm)->common.type))->type.size) == 0)
	{
	  error_with_decl (parm, "parameter `%s' has incomplete type");
	  ((parm)->common.type) = error_mark_node;
	}
      if (((parm)->decl.result) == 0)
	{
	  error_with_decl (parm,
			   "declaration for parameter `%s' but no such parameter");
	  specparms
	    = chainon (specparms,
		       tree_cons (parm, (tree) 0, (tree) 0));
	}
      parm = next;
    }
  parm = specparms;
  parmdecls = 0;
  {
    register tree last;
    for (last = 0; parm; parm = ((parm)->common.chain))
      if (((parm)->list.purpose))
	{
	  if (last == 0)
	    parmdecls = ((parm)->list.purpose);
	  else
	    ((last)->common.chain) = ((parm)->list.purpose);
	  last = ((parm)->list.purpose);
	  ((last)->common.chain) = 0;
	  types = saveable_tree_cons ((tree) 0, ((parm)->common.type), types);
	}
  }
  if (void_at_end)
    return saveable_tree_cons (parmdecls, nonparms,
			       nreverse (saveable_tree_cons ((tree) 0, void_type_node, types)));
  return saveable_tree_cons (parmdecls, nonparms, nreverse (types));
}
void
finish_function (nested)
     int nested;
{
  register tree fndecl = current_function_decl;
  poplevel (1, 0, 1);
  ((((fndecl)->decl.initial))->block.supercontext) = fndecl;
  ((((fndecl)->decl.result))->decl.context) = fndecl;
  if (flag_traditional && current_function_calls_setjmp)
    {
      setjmp_protect (((fndecl)->decl.initial));
      setjmp_protect_args ();
    }
  expand_function_end (input_filename, lineno);
  can_reach_end = 0;
  rest_of_compilation (fndecl);
  current_function_returns_null |= can_reach_end;
  if (((fndecl)->common.volatile_flag) && current_function_returns_null)
    warning ("`volatile' function does return");
  else if (warn_return_type && can_reach_end
	   && ((((((fndecl)->common.type))->common.type))->type.main_variant) != void_type_node)
    warning ("control reaches end of non-void function");
  else if (extra_warnings
	   && current_function_returns_value && current_function_returns_null)
    warning ("this function may return with or without a value");
  if (! nested)
    permanent_allocation ();
  if (((fndecl)->decl.saved_insns.r) == 0 && ! nested)
    {
      ((fndecl)->decl.initial) = error_mark_node;
      ((fndecl)->decl.arguments) = 0;
    }
  if (! nested)
    {
      current_function_decl = 0;
    }
}
struct c_function
{
  struct c_function *next;
  tree enum_next_value;
  tree named_labels;
  tree shadowed_labels;
  int returns_value;
  int returns_null;
  int warn_about_return_type;
  int extern_inline;
  struct binding_level *binding_level;
};
struct c_function *c_function_chain;
void
push_c_function_context ()
{
  struct c_function *p
    = (struct c_function *) xmalloc (sizeof (struct c_function));
  if (pedantic)
    pedwarn ("ANSI C forbids nested functions");
  push_function_context ();
  p->next = c_function_chain;
  c_function_chain = p;
  p->enum_next_value = enum_next_value;
  p->named_labels = named_labels;
  p->shadowed_labels = shadowed_labels;
  p->returns_value = current_function_returns_value;
  p->returns_null = current_function_returns_null;
  p->warn_about_return_type = warn_about_return_type;
  p->extern_inline = current_extern_inline;
  p->binding_level = current_binding_level;
}
void
pop_c_function_context ()
{
  struct c_function *p = c_function_chain;
  tree link;
  for (link = shadowed_labels; link; link = ((link)->common.chain))
    if (((((link)->list.value))->decl.name) != 0)
        (((struct lang_identifier *)(((((link)->list.value))->decl.name)))->label_value)
	= ((link)->list.value);
  if (((current_function_decl)->decl.saved_insns.r) == 0)
    {
      ((current_function_decl)->decl.initial) = error_mark_node;
      ((current_function_decl)->decl.arguments) = 0;
    }
  pop_function_context ();
  c_function_chain = p->next;
  enum_next_value = p->enum_next_value;
  named_labels = p->named_labels;
  shadowed_labels = p->shadowed_labels;
  current_function_returns_value = p->returns_value;
  current_function_returns_null = p->returns_null;
  warn_about_return_type = p->warn_about_return_type;
  current_extern_inline = p->extern_inline;
  current_binding_level = p->binding_level;
  free (p);
}

extern int target_flags;
extern int hard_regno_nregs[];
extern int hard_regno_mode_ok[64 ];
extern int leaf_function;
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
struct  stat
{
        dev_t           st_dev;
        ino_t           st_ino;
        mode_t          st_mode;
        nlink_t         st_nlink;
        uid_t           st_uid;
        gid_t           st_gid;
        dev_t           st_rdev;
        off_t           st_size;
        time_t          st_atime;
        int             st_spare1;
        time_t          st_mtime;
        int             st_spare2;
        time_t          st_ctime;
        int             st_spare3;
        long            st_blksize;
        long            st_blocks;
        unsigned long   st_gennum;
        long            st_spare4;
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
zz1xmalloc (byte_count)
     size_t byte_count;
{
  pointer_type rv;
  rv = malloc (byte_count);
  if (rv == 0 )
    {
      exit (1);
      return 0;		 
    }
  else
    return rv;
}
pointer_type
zxrealloc (old_space, byte_count)
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
    aux_info_base = (char *) xmalloc (aux_info_size + 1);
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
	aux_info_relocated_name = (char *) xmalloc (base_len + (p-invocation_filename));
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
