
extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

extern	struct	_iobuf {
	int	_cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int	_bufsiz;
	short	_flag;
	char	_file;		 
} _iob[];

extern struct _iobuf 	*fopen();
extern struct _iobuf 	*fdopen();
extern struct _iobuf 	*freopen();
extern struct _iobuf 	*popen();
extern struct _iobuf 	*tmpfile();
extern long	ftell();
extern char	*fgets();
extern char	*gets();

extern char	*ctermid();
extern char	*cuserid();
extern char	*tempnam();
extern char	*tmpnam();

typedef	int	faultcode_t;	 

void	(*signal())();

void  (*sigset())();
int   sighold();
int   sigrelse();
int   sigignore();

struct	sigvec {
	void	(*sv_handler)();	 
	int	sv_mask;		 
	int	sv_flags;		 
};

struct	sigstack {
	char	*ss_sp;			 
	int	ss_onstack;		 
};

struct	sigcontext {
	int	sc_onstack;		 
	int	sc_mask;		 

	int	sc_sp;			 
	int	sc_pc;			 
	int	sc_ps;			 

};

extern int sigsetjmp(), setjmp(), _setjmp();
extern void siglongjmp(), longjmp(), _longjmp();

typedef int jmp_buf[58 ];

typedef	int sigjmp_buf[58 +1];

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		 
typedef	unsigned int	uint;		 

typedef	struct	_physadr { short r[1]; } *physadr;
typedef	struct	label_t	{
	int	val[13];
} label_t;

typedef	struct	_quad { long val[2]; } quad;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	u_long	ino_t;
typedef	long	swblk_t;
typedef	int	size_t;
typedef	long	time_t;
typedef	short	dev_t;
typedef	long	off_t;
typedef	u_short	uid_t;
typedef	u_short	gid_t;
typedef	long	key_t;

typedef	long	fd_mask;

typedef	struct fd_set {
	fd_mask	fds_bits[(((256 )+(( (sizeof(fd_mask) * 8		)	)-1))/( (sizeof(fd_mask) * 8		)	)) ];
} fd_set;

typedef	char *	addr_t;

struct	stat
{
	dev_t	st_dev;
	ino_t	st_ino;
	unsigned short st_mode;
	short	st_nlink;
	short	st_uid;
	short	st_gid;
	dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	int	st_spare1;
	time_t	st_mtime;
	int	st_spare2;
	time_t	st_ctime;
	int	st_spare3;
	long	st_blksize;
	long	st_blocks;
	long	st_spare4[2];
};

struct tms {
	time_t	tms_utime;		 
	time_t	tms_stime;		 
	time_t	tms_cutime;		 
	time_t	tms_cstime;		 
};

struct tm {
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
	char	*tm_zone;
	long	tm_gmtoff;
};

extern	struct tm *gmtime(), *localtime();
extern	char *asctime(), *ctime();
extern	void tzset(), tzsetwall();

enum tree_code {

  ERROR_MARK, 

  IDENTIFIER_NODE, 

  OP_IDENTIFIER, 

  TREE_LIST, 

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

  LABEL_STMT, 

  GOTO_STMT, 

  RETURN_STMT, 

  EXPR_STMT, 

  WITH_STMT, 

  LET_STMT, 

  IF_STMT, 

  EXIT_STMT, 

  CASE_STMT, 

  LOOP_STMT, 

  COMPOUND_STMT, 

  ASM_STMT, 

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
  FRIEND_DECL, 

  COMPONENT_REF, 

  INDIRECT_REF, 

  OFFSET_REF, 

  BUFFER_REF, 

  ARRAY_REF, 

  CONSTRUCTOR, 

  COMPOUND_EXPR, 

  MODIFY_EXPR, 

  INIT_EXPR, 

  NEW_EXPR, 

  DELETE_EXPR, 

  COND_EXPR, 

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

  SAVE_EXPR, 

  RTL_EXPR, 

  ADDR_EXPR, 

  REFERENCE_EXPR, 

  WRAPPER_EXPR, 
  ANTI_WRAPPER_EXPR, 

  ENTRY_VALUE_EXPR, 

  COMPLEX_EXPR, 

  CONJ_EXPR, 

  REALPART_EXPR, 
  IMAGPART_EXPR, 

  PREDECREMENT_EXPR, 
  PREINCREMENT_EXPR, 
  POSTDECREMENT_EXPR, 
  POSTINCREMENT_EXPR, 

  LAST_AND_UNUSED_TREE_CODE	 

};

extern char *tree_code_type[];

extern int tree_code_length[];

enum machine_mode {

 VOIDmode, 

 QImode, 		 
 HImode, 

 PSImode, 
 SImode, 
 PDImode, 
 DImode, 
 TImode, 
 QFmode, 
 HFmode, 		 
 SFmode, 
 DFmode, 
 XFmode, 	 
 TFmode, 
 CQImode, 
 CHImode, 	 
 CSImode, 
 CDImode, 
 CTImode, 
 CQFmode, 
 CHFmode, 	 
 CSFmode, 
 CDFmode, 
 CXFmode, 
 CTFmode, 

 BImode, 	 

 BLKmode, 

 EPmode, 

};

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
  BUILT_IN_FSQRT,
  BUILT_IN_GETEXP,
  BUILT_IN_GETMAN,
  BUILT_IN_SAVEREGS,

  BUILT_IN_NEW,
  BUILT_IN_VEC_NEW,
  BUILT_IN_DELETE,
  BUILT_IN_VEC_DELETE,
};

typedef union tree_node *tree;

struct tree_common
{
  int uid;
  union tree_node *chain;
  union tree_node *type;
  enum tree_code code : 8;

  unsigned external_attr : 1;
  unsigned public_attr : 1;
  unsigned static_attr : 1;
  unsigned volatile_attr : 1;
  unsigned packed_attr : 1;
  unsigned readonly_attr : 1;
  unsigned literal_attr : 1;
  unsigned nonlocal_attr : 1;
  unsigned permanent_attr : 1;
  unsigned addressable_attr : 1;
  unsigned regdecl_attr : 1;
  unsigned this_vol_attr : 1;
  unsigned unsigned_attr : 1;
  unsigned asm_written_attr: 1;
  unsigned inline_attr : 1;
  unsigned used_attr : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
};

struct tree_int_cst
{
  char common[sizeof (struct tree_common)];
  long int_cst_low;
  long int_cst_high;
};

extern double ldexp ();

extern double atof ();

union real_extract 
{
  double  d;
  int i[sizeof (double ) / sizeof (int)];
};

struct tree_real_cst
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	 

  double  real_cst;
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
  union tree_node *global_value;
  union tree_node *local_value;
  union tree_node *label_value;
  union tree_node *implicit_decl;
  union tree_node *error_locus;
};

struct tree_list
{
  char common[sizeof (struct tree_common)];
  union tree_node *purpose;
  union tree_node *value;
};

struct tree_exp
{
  char common[sizeof (struct tree_common)];
  int complexity;
  union tree_node *operands[1];
};

struct tree_type
{
  char common[sizeof (struct tree_common)];
  union tree_node *values;
  union tree_node *sep;
  union tree_node *size;

  enum machine_mode mode : 8;
  unsigned char size_unit;
  unsigned char align;
  unsigned char sep_unit;

  union tree_node *pointer_to;
  union tree_node *reference_to;
  int parse_info;
  int symtab_address;
  union tree_node *name;
  union tree_node *max;
  union tree_node *next_variant;
  union tree_node *main_variant;
  union tree_node *basetypes;
  union tree_node *noncopied_parts;
  struct lang_type *lang_specific;
};

struct tree_decl
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *size;
  enum machine_mode mode;
  unsigned char size_unit;
  unsigned char align;
  unsigned char voffset_unit;
  union tree_node *name;
  union tree_node *context;
  int offset;
  union tree_node *voffset;
  union tree_node *arguments;
  union tree_node *result;
  union tree_node *initial;
  struct rtx_def *rtl;	 

  int frame_size;		 
  struct rtx_def *saved_insns;	 

  int block_symtab_address;
  struct lang_decl *lang_specific;
};

struct tree_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *body;
};

struct tree_if_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *cond, *thenpart, *elsepart;
};

struct tree_bind_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *body, *vars, *supercontext, *bind_size, *type_tags;
};

struct tree_case_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *index, *case_list;
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
  struct tree_exp exp;
  struct tree_stmt stmt;
  struct tree_if_stmt if_stmt;
  struct tree_bind_stmt bind_stmt;
  struct tree_case_stmt case_stmt;
};
extern char *oballoc ();
extern char *permalloc ();

extern tree make_node ();

extern tree copy_node ();

extern tree get_identifier ();

extern tree build_int_2 ();
extern tree build_real ();
extern tree build_real_from_string ();
extern tree build_real_from_int_cst ();
extern tree build_complex ();
extern tree build_string ();
extern tree build ();
extern tree build_nt ();
extern tree build_tree_list ();
extern tree build_op_identifier ();
extern tree build_decl ();
extern tree build_let ();

extern tree make_signed_type ();
extern tree make_unsigned_type ();
extern void fixup_unsigned_type ();
extern tree build_pointer_type ();
extern tree build_reference_type ();
extern tree build_index_type ();
extern tree build_array_type ();
extern tree build_function_type ();
extern tree build_method_type ();
extern tree build_offset_type ();
extern tree array_type_nelts ();

extern tree build_binary_op ();
extern tree build_indirect_ref ();
extern tree build_unary_op ();

extern tree build_type_variant ();

extern void layout_type ();

extern tree type_hash_canon ();

extern void layout_decl ();

extern tree fold ();

extern tree combine ();

extern tree convert ();
extern tree convert_units ();
extern tree size_in_bytes ();
extern tree genop ();
extern tree build_int ();
extern tree get_pending_sizes ();

extern tree sizetype;

extern tree chainon ();

extern tree tree_cons (), perm_tree_cons (), temp_tree_cons ();
extern tree saveable_tree_cons ();

extern tree tree_last ();

extern tree nreverse ();

extern int list_length ();

extern int integer_zerop ();

extern int integer_onep ();

extern int integer_all_onesp ();

extern int type_unsigned_p ();

extern int staticp ();

extern int lvalue_or_else ();

extern tree save_expr ();

extern tree stabilize_reference ();

extern tree get_unwidened ();

extern tree get_narrower ();

extern tree type_for_size ();

extern tree unsigned_type ();

extern tree signed_type ();

extern tree get_floating_type ();

extern char *function_cannot_inline_p ();

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

extern int pedantic;

extern int immediate_size_expand;

extern tree current_function_decl;

extern int current_function_calls_setjmp;

extern int all_types_permanent;

extern tree expand_start_stmt_expr ();
extern tree expand_end_stmt_expr ();
extern void expand_expr_stmt(), clear_last_expr();
extern void expand_label(), expand_goto(), expand_asm();
extern void expand_start_cond(), expand_end_cond();
extern void expand_start_else(), expand_end_else();
extern void expand_start_loop(), expand_start_loop_continue_elsewhere();
extern void expand_loop_continue_here();
extern void expand_end_loop();
extern int expand_continue_loop();
extern int expand_exit_loop(), expand_exit_loop_if_false();
extern int expand_exit_something();

extern void expand_start_delayed_expr ();
extern tree expand_end_delayed_expr ();
extern void expand_emit_delayed_expr ();

extern void expand_null_return(), expand_return();
extern void expand_start_bindings(), expand_end_bindings();
extern void expand_start_case(), expand_end_case();
extern int pushcase(), pushcase_range ();
extern void expand_start_function(), expand_end_function();

extern int pedantic;

extern tree build_component_ref(), build_conditional_expr(), build_compound_expr();
extern tree build_unary_op(), build_binary_op(), build_function_call();
extern tree build_binary_op_nodefault ();
extern tree build_indirect_ref(), build_array_ref(), build_c_cast();
extern tree build_modify_expr();
extern tree c_sizeof (), c_alignof ();
extern void store_init_value ();
extern tree digest_init ();
extern tree c_expand_start_case ();
extern tree default_conversion ();

extern tree commontype ();

extern tree build_label ();

extern int start_function ();
extern void finish_function ();
extern void store_parm_decls ();
extern tree get_parm_info ();

extern void pushlevel(), poplevel();

extern tree groktypename(), lookup_name();

extern tree lookup_label(), define_label();

extern tree implicitly_declare(), getdecls(), gettags ();

extern tree start_decl();
extern void finish_decl();

extern tree start_struct(), finish_struct(), xref_tag();
extern tree grokfield();

extern tree start_enum(), finish_enum();
extern tree build_enumerator();

extern tree make_index_type ();

extern tree double_type_node, long_double_type_node, float_type_node;
extern tree char_type_node, unsigned_char_type_node, signed_char_type_node;

extern tree short_integer_type_node, short_unsigned_type_node;
extern tree long_integer_type_node, long_unsigned_type_node;
extern tree unsigned_type_node;
extern tree string_type_node, char_array_type_node, int_array_type_node;

extern int current_function_returns_value;
extern int current_function_returns_null;

extern int lineno;

extern tree ridpointers[];

extern tree current_function_decl;

extern int dollars_in_ident;

extern int flag_signed_char;

extern int flag_cond_mismatch;

extern int flag_no_asm;

extern int warn_implicit;

extern int warn_return_type;

extern int warn_write_strings;

extern int warn_pointer_arith;

extern int warn_strict_prototypes;

extern int warn_cast_qual;

extern int flag_traditional;

enum rtx_code  {

  UNKNOWN , 

  NIL , 

  EXPR_LIST , 

  INSN_LIST , 

  MATCH_OPERAND , 

  MATCH_DUP , 

  MATCH_OPERATOR , 

  DEFINE_INSN , 

  DEFINE_PEEPHOLE , 

  DEFINE_COMBINE , 

  DEFINE_EXPAND , 

  SEQUENCE , 

  ADDRESS , 

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

  ADDR_VEC , 

  ADDR_DIFF_VEC , 

  SET , 

  USE , 

  CLOBBER , 

  CALL , 

  RETURN , 

  CONST_INT , 

  CONST_DOUBLE , 

  CONST , 

  PC , 

  REG , 

  SUBREG , 

  STRICT_LOW_PART , 

  MEM , 

  LABEL_REF , 

  SYMBOL_REF , 

  CC0 , 

  QUEUED , 

  IF_THEN_ELSE , 

  COMPARE , 

  PLUS , 

  MINUS , 

  NEG , 

  MULT , 

  DIV , 
  MOD , 

  UMULT , 
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

  LAST_AND_UNUSED_RTX_CODE};	 

extern int rtx_length[];

extern char *rtx_name[];

extern char *rtx_format[];

extern char *mode_name[];

enum mode_class { MODE_RANDOM, MODE_INT, MODE_FLOAT,
		  MODE_COMPLEX_INT, MODE_COMPLEX_FLOAT, MODE_FUNCTION };

extern enum mode_class mode_class[];

extern int mode_size[];

extern int mode_unit_size[];

typedef union rtunion_def
{
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
		REG_NONNEG = 8 };

extern char *reg_note_name[];

extern char *note_insn_name[];

extern rtx rtx_alloc ();
extern rtvec rtvec_alloc ();
extern rtx find_reg_note ();
extern rtx gen_rtx ();
extern rtx copy_rtx ();
extern rtvec gen_rtvec ();
extern rtvec gen_rtvec_v ();
extern rtx gen_reg_rtx ();
extern rtx gen_label_rtx ();
extern rtx gen_inline_header_rtx ();
extern rtx gen_lowpart ();
extern rtx gen_highpart ();
extern int subreg_lowpart_p ();
extern rtx make_safe_from ();
extern rtx memory_address ();
extern rtx get_insns ();
extern rtx get_last_insn ();
extern rtx start_sequence ();
extern rtx gen_sequence ();
extern rtx expand_expr ();
extern rtx output_constant_def ();
extern rtx immed_real_const ();
extern rtx immed_real_const_1 ();
extern rtx immed_double_const ();
extern rtx force_const_double_mem ();
extern rtx force_const_mem ();
extern rtx get_parm_real_loc ();
extern rtx assign_stack_local ();
extern rtx protect_from_queue ();
extern void emit_queue ();
extern rtx emit_move_insn ();
extern rtx emit_insn ();
extern rtx emit_jump_insn ();
extern rtx emit_call_insn ();
extern rtx emit_call_insn_before ();
extern rtx emit_insn_before ();
extern rtx emit_insn_after ();
extern rtx emit_label ();
extern rtx emit_barrier ();
extern rtx emit_note ();
extern rtx emit_line_note ();
extern rtx emit_line_note_force ();
extern rtx prev_real_insn ();
extern rtx next_real_insn ();
extern rtx next_nondeleted_insn ();
extern rtx plus_constant ();
extern rtx find_equiv_reg ();
extern rtx delete_insn ();
extern rtx adj_offsetable_operand ();

extern int max_parallel;

extern int asm_noperands ();
extern char *decode_asm_operands ();

extern enum reg_class reg_preferred_class ();

extern rtx get_first_nonparm_insn ();

extern rtx pc_rtx;
extern rtx cc0_rtx;
extern rtx const0_rtx;
extern rtx const1_rtx;
extern rtx fconst0_rtx;
extern rtx dconst0_rtx;

extern rtx stack_pointer_rtx;
extern rtx frame_pointer_rtx;
extern rtx arg_pointer_rtx;
extern rtx struct_value_rtx;
extern rtx struct_value_incoming_rtx;
extern rtx static_chain_rtx;
extern rtx static_chain_incoming_rtx;

extern char *main_input_filename;

enum debugger { NO_DEBUG = 0, GDB_DEBUG = 1, DBX_DEBUG = 2, SDB_DEBUG = 3,
		EXTENDED_DBX_DEBUG = 4 };

extern enum debugger write_symbols;

extern int use_gdb_dbx_extensions;

extern int optimize;

extern int obey_regdecls;

extern int quiet_flag;

extern int inhibit_warnings;

extern int extra_warnings;

extern int warn_unused;

extern int warn_shadow;

extern int warn_switch;

extern int warn_id_clash;
extern int id_clash_len;

extern int profile_flag;

extern int profile_block_flag;

extern int pedantic;

extern int flag_caller_saves;

extern int flag_pcc_struct_return;

extern int flag_force_mem;

extern int flag_force_addr;

extern int flag_defer_pop;

extern int flag_float_store;

extern int flag_combine_regs;

extern int flag_strength_reduce;

extern int flag_writable_strings;

extern int flag_no_function_cse;

extern int flag_omit_frame_pointer;

extern int frame_pointer_needed;

extern int flag_no_peephole;

extern int flag_volatile;

extern int flag_inline_functions;

extern int flag_keep_inline_functions;

extern int flag_syntax_only;

extern int flag_shared_data;

extern int yydebug;

extern struct _iobuf  *finput;

extern int reload_completed;
extern int rtx_equal_function_value_matters;

extern void init_lex ();
extern void init_decl_processing ();
extern void init_tree ();
extern void init_rtl ();
extern void init_optabs ();
extern void init_reg_sets ();
extern void dump_flow_info ();
extern void dump_local_alloc ();

void rest_of_decl_compilation ();
void error ();
void error_with_file_and_line ();
void set_target_switch ();

int target_flags;

char *input_filename;

char *main_input_filename;

extern int lineno;

extern tree current_function_decl;

char *dump_base_name;

int rtl_dump = 0;
int rtl_dump_and_exit = 0;
int jump_opt_dump = 0;
int cse_dump = 0;
int loop_dump = 0;
int flow_dump = 0;
int combine_dump = 0;
int local_reg_dump = 0;
int global_reg_dump = 0;
int jump2_opt_dump = 0;

enum debugger write_symbols = NO_DEBUG;

int use_gdb_dbx_extensions;

int optimize = 0;

int flag_caller_saves = 0;

int flag_pcc_struct_return = 0;

int flag_force_mem = 0;

int flag_force_addr = 0;

int flag_defer_pop = 1;

int flag_float_store = 0;

int flag_combine_regs = 0;

int flag_strength_reduce = 0;

int flag_writable_strings = 0;

int flag_no_function_cse = 0;

int flag_omit_frame_pointer = 0;

int flag_no_peephole = 0;

int flag_volatile;

int obey_regdecls = 0;

int quiet_flag = 0;

int inhibit_warnings = 0;

int extra_warnings = 0;

int warn_unused;

int warn_shadow;

int warn_switch;

int warn_id_clash;
int id_clash_len;

int errorcount = 0;
int warningcount = 0;
int sorrycount = 0;

int profile_flag = 0;

int profile_block_flag;

int pedantic = 0;

int flag_inline_functions;

int flag_keep_inline_functions;

int flag_syntax_only;

int flag_shared_data;

char *asm_file_name;

char *sym_file_name;

struct { char *string; int *variable; int on_value;} f_options[] =
{
  {"float-store", &flag_float_store, 1},
  {"volatile", &flag_volatile, 1},
  {"defer-pop", &flag_defer_pop, 1},
  {"omit-frame-pointer", &flag_omit_frame_pointer, 1},
  {"strength-reduce", &flag_strength_reduce, 1},
  {"writable-strings", &flag_writable_strings, 1},
  {"peephole", &flag_no_peephole, 0},
  {"force-mem", &flag_force_mem, 1},
  {"force-addr", &flag_force_addr, 1},
  {"combine-regs", &flag_combine_regs, 1},
  {"function-cse", &flag_no_function_cse, 0},
  {"inline-functions", &flag_inline_functions, 1},
  {"keep-inline-functions", &flag_keep_inline_functions, 1},
  {"syntax-only", &flag_syntax_only, 1},
  {"shared-data", &flag_shared_data, 1},
  {"caller-saves", &flag_caller_saves, 1},
  {"pcc-struct-return", &flag_pcc_struct_return, 1}
};

struct _iobuf  *asm_out_file;
struct _iobuf  *rtl_dump_file;
struct _iobuf  *jump_opt_dump_file;
struct _iobuf  *cse_dump_file;
struct _iobuf  *loop_dump_file;
struct _iobuf  *flow_dump_file;
struct _iobuf  *combine_dump_file;
struct _iobuf  *local_reg_dump_file;
struct _iobuf  *global_reg_dump_file;
struct _iobuf  *jump2_opt_dump_file;

int parse_time;
int varconst_time;
int integration_time;
int jump_time;
int cse_time;
int loop_time;
int flow_time;
int combine_time;
int local_alloc_time;
int global_alloc_time;
int final_time;
int symout_time;
int dump_time;

int
gettime ()
{

  struct tms tms;

  if (quiet_flag)
    return 0;

  times (&tms);
  return (tms.tms_utime + tms.tms_stime) * (1000000 / 60		);

}

void
print_time (str, total)
     char *str;
     int total;
{
  fprintf ((&_iob[2]) ,
	   "time in %s: %d.%06d\n",
	   str, total / 1000000, total % 1000000);
}

int
count_error (warningp)
     int warningp;
{
  if (warningp && inhibit_warnings)
    return 0;

  if (warningp)
    warningcount++;
  else
    errorcount++;

  return 1;
}

void
pfatal_with_name (name)
     char *name;
{
  fprintf ((&_iob[2]) , "cc1: ");
  perror (name);
  exit (35);
}

void
fatal_io_error (name)
     char *name;
{
  fprintf ((&_iob[2]) , "cc1:%s: I/O error\n", name);
  exit (35);
}

void
fatal (s, v)
     char *s;
{
  error (s, v);
  exit (34);
}
static int need_error_newline;

static tree last_error_function = 0 ;

void
announce_function (decl)
     tree decl;
{
  if (! quiet_flag)
    {
      fprintf ((&_iob[2]) , " %s", ((((decl)->decl.name) )->identifier.pointer) );
      fflush ((&_iob[2]) );
      need_error_newline = 1;
      last_error_function = current_function_decl;
    }
}

static void
report_error_function (file)
     char *file;
{
  if (need_error_newline)
    {
      fprintf ((&_iob[2]) , "\n");
      need_error_newline = 0;
    }

  if (last_error_function != current_function_decl)
    {
      if (file)
	fprintf ((&_iob[2]) , "%s: ", file);

      if (current_function_decl == 0 )
	fprintf ((&_iob[2]) , "At top level:\n");
      else
	fprintf ((&_iob[2]) , "In function %s:\n",
		 ((((current_function_decl)->decl.name) )->identifier.pointer) );

      last_error_function = current_function_decl;
    }
}

void
error (s, v, v2)
     char *s;
     int v;			 
     int v2;			 
{
  error_with_file_and_line (input_filename, lineno, s, v, v2);
}

void
error_with_file_and_line (file, line, s, v, v2)
     char *file;
     int line;
     char *s;
     int v;
     int v2;
{
  count_error (0);

  report_error_function (file);

  if (file)
    fprintf ((&_iob[2]) , "%s:%d: ", file, line);
  else
    fprintf ((&_iob[2]) , "cc1: ");
  fprintf ((&_iob[2]) , s, v, v2);
  fprintf ((&_iob[2]) , "\n");
}

void
error_with_decl (decl, s, v)
     tree decl;
     char *s;
     int v;
{
  count_error (0);

  report_error_function (((decl)->decl.filename) );

  fprintf ((&_iob[2]) , "%s:%d: ",
	   ((decl)->decl.filename) , ((decl)->decl.linenum) );

  if (((decl)->decl.name) )
    fprintf ((&_iob[2]) , s, ((((decl)->decl.name) )->identifier.pointer) , v);
  else
    fprintf ((&_iob[2]) , s, "((anonymous))", v);
  fprintf ((&_iob[2]) , "\n");
}

void
error_for_asm (insn, s, v, v2)
     rtx insn;
     char *s;
     int v;			 
     int v2;			 
{
  rtx temp;
  char *filename;
  int line;
  rtx body = ((insn)->fld[3].rtx) ;
  rtx asmop;

  if (	((body)->code)  == SET && 	((((body)->fld[1].rtx) )->code)  == ASM_OPERANDS)
    asmop = ((body)->fld[1].rtx) ;
  else if (	((body)->code)  == ASM_OPERANDS)
    asmop = body;
  else if (	((body)->code)  == PARALLEL
	   && 	((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->code)  == SET)
    asmop = ((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->fld[1].rtx) ;
  else if (	((body)->code)  == PARALLEL
	   && 	((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->code)  == ASM_OPERANDS)
    asmop = ((body)->fld[ 0].rtvec->elem[ 0].rtx) ;

  filename = (((asmop))->fld[ 5].rtstr)  ;
  line = (((asmop))->fld[ 6].rtint)  ;
  error_with_file_and_line (filename, line, s, v, v2);
}

void
warning_with_file_and_line (file, line, s, v, v2)
     char *file;
     int line;
     char *s;
     int v;
     int v2;
{
  if (count_error (1) == 0)
    return;

  report_error_function (file);

  if (file)
    fprintf ((&_iob[2]) , "%s:%d: ", file, line);
  else
    fprintf ((&_iob[2]) , "cc1: ");

  fprintf ((&_iob[2]) , "warning: ");
  fprintf ((&_iob[2]) , s, v, v2);
  fprintf ((&_iob[2]) , "\n");
}

void
warning (s, v, v2)
     char *s;
     int v;			 
     int v2;
{
  warning_with_file_and_line (input_filename, lineno, s, v, v2);
}

void
warning_with_decl (decl, s, v)
     tree decl;
     char *s;
     int v;
{
  if (count_error (1) == 0)
    return;

  report_error_function (((decl)->decl.filename) );

  fprintf ((&_iob[2]) , "%s:%d: ",
	   ((decl)->decl.filename) , ((decl)->decl.linenum) );

  fprintf ((&_iob[2]) , "warning: ");
  if (((decl)->decl.name) )
    fprintf ((&_iob[2]) , s, ((((decl)->decl.name) )->identifier.pointer) , v);
  else
    fprintf ((&_iob[2]) , s, "((anonymous))", v);
  fprintf ((&_iob[2]) , "\n");
}

void
sorry (s, v, v2)
     char *s;
     int v, v2;
{
  sorrycount++;
  if (input_filename)
    fprintf ((&_iob[2]) , "%s:%d: ", input_filename, lineno);
  else
    fprintf ((&_iob[2]) , "cc1: ");

  fprintf ((&_iob[2]) , "sorry, not implemented: ");
  fprintf ((&_iob[2]) , s, v, v2);
  fprintf ((&_iob[2]) , "\n");
}

void
really_sorry (s, v, v2)
     char *s;
     int v, v2;
{
  if (input_filename)
    fprintf ((&_iob[2]) , "%s:%d: ", input_filename, lineno);
  else
    fprintf ((&_iob[2]) , "c++: ");

  fprintf ((&_iob[2]) , "sorry, not implemented: ");
  fprintf ((&_iob[2]) , s, v, v2);
  fatal (" (fatal)\n");
}

void
botch (s)
{
  abort ();
}

int
xmalloc (size)
     unsigned size;
{
  register int value = (int) malloc (size);
  if (value == 0)
    fatal ("Virtual memory exhausted.");
  return value;
}

int
xrealloc (ptr, size)
     char *ptr;
     int size;
{
  int result = realloc (ptr, size);
  if (!result)
    fatal ("Virtual memory exhausted.");
  return result;
}

int
exact_log2 (x)
     register unsigned int x;
{
  register int log = 0;
  for (log = 0; log < 32 ; log++)
    if (x == (1 << log))
      return log;
  return -1;
}

int
floor_log2 (x)
     register unsigned int x;
{
  register int log = 0;
  for (log = 0; log < 32 ; log++)
    if ((x & ((-1) << log)) == 0)
      return log - 1;
  return 32  - 1;
}

int float_handled;
jmp_buf float_handler;

void
set_float_handler (handler)
     jmp_buf handler;
{
  float_handled = (handler != 0);
  if (handler)
    memcpy ( float_handler,handler, sizeof (float_handler)) ;
}

static void
float_signal ()
{
  if (float_handled == 0)
    abort ();
  warning ("floating overflow in constant folding");
  float_handled = 0;
  longjmp (float_handler, 1);
}

static void
pipe_closed ()
{
  fatal ("output pipe has been closed");
}

static void
compile_file (name)
     char *name;
{
  tree globals;
  int start_time;
  int dump_base_name_length;

  int name_specified = name != 0;

  if (dump_base_name == 0)
    dump_base_name = name ? name : "gccdump";
  dump_base_name_length = strlen (dump_base_name);

  parse_time = 0;
  varconst_time = 0;
  integration_time = 0;
  jump_time = 0;
  cse_time = 0;
  loop_time = 0;
  flow_time = 0;
  combine_time = 0;
  local_alloc_time = 0;
  global_alloc_time = 0;
  final_time = 0;
  symout_time = 0;
  dump_time = 0;

  if (name == 0 || !strcmp (name, "-"))
    {
      finput = (&_iob[0]) ;
      name = "stdin";
    }
  else
    finput = fopen (name, "r");
  if (finput == 0)
    pfatal_with_name (name);

  init_tree ();
  init_lex ();
  init_rtl ();
  init_emit_once ();
  init_decl_processing ();
  init_optabs ();

  if (rtl_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".rtl");
      rtl_dump_file = fopen (dumpname, "w");
      if (rtl_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (jump_opt_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".jump");
      jump_opt_dump_file = fopen (dumpname, "w");
      if (jump_opt_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (cse_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".cse");
      cse_dump_file = fopen (dumpname, "w");
      if (cse_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (loop_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".loop");
      loop_dump_file = fopen (dumpname, "w");
      if (loop_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (flow_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".flow");
      flow_dump_file = fopen (dumpname, "w");
      if (flow_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (combine_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 10);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".combine");
      combine_dump_file = fopen (dumpname, "w");
      if (combine_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (local_reg_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".lreg");
      local_reg_dump_file = fopen (dumpname, "w");
      if (local_reg_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (global_reg_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".greg");
      global_reg_dump_file = fopen (dumpname, "w");
      if (global_reg_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (jump2_opt_dump)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 7);
      strcpy (dumpname, dump_base_name);
      strcat (dumpname, ".jump2");
      jump2_opt_dump_file = fopen (dumpname, "w");
      if (jump2_opt_dump_file == 0)
	pfatal_with_name (dumpname);
    }

  if (! name_specified && asm_file_name == 0)
    asm_out_file = (&_iob[1]) ;
  else
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      int len = strlen (dump_base_name);
      strcpy (dumpname, dump_base_name);
      if (len > 2 && ! strcmp (".c", dumpname + len - 2))
	dumpname[len - 2] = 0;
      else if (len > 2 && ! strcmp (".i", dumpname + len - 2))
	dumpname[len - 2] = 0;
      else if (len > 3 && ! strcmp (".co", dumpname + len - 3))
	dumpname[len - 3] = 0;
      strcat (dumpname, ".s");
      if (asm_file_name == 0)
	{
	  asm_file_name = (char *) malloc (strlen (dumpname) + 1);
	  strcpy (asm_file_name, dumpname);
	}
      if (!strcmp (asm_file_name, "-"))
	asm_out_file = (&_iob[1]) ;
      else
	asm_out_file = fopen (asm_file_name, "w");
      if (asm_out_file == 0)
	pfatal_with_name (asm_file_name);
    }

  input_filename = name;

  ungetc (check_newline (), finput);

  if (main_input_filename == 0)
    main_input_filename = name;

  fprintf (asm_out_file, "#NO_APP\n"); ;

  fprintf (asm_out_file, "gcc_compiled.:\n");

  if (write_symbols == GDB_DEBUG)
    {
      register char *dumpname = (char *) xmalloc (dump_base_name_length + 6);
      int len = strlen (dump_base_name);
      strcpy (dumpname, dump_base_name);
      if (len > 2 && ! strcmp (".c", dumpname + len - 2))
	dumpname[len - 2] = 0;
      else if (len > 2 && ! strcmp (".i", dumpname + len - 2))
	dumpname[len - 2] = 0;
      else if (len > 3 && ! strcmp (".co", dumpname + len - 3))
	dumpname[len - 3] = 0;
      strcat (dumpname, ".sym");
      if (sym_file_name == 0)
	sym_file_name = dumpname;
      symout_init (sym_file_name, asm_out_file, main_input_filename);
    }

  if (write_symbols == DBX_DEBUG)
    dbxout_init (asm_out_file, main_input_filename);

  init_final (main_input_filename);

  start_time = gettime ();

  yyparse ();

  parse_time += gettime () - start_time;

  parse_time -= varconst_time;

  globals = getdecls ();

  {
    tree decl;
    for (decl = globals; decl; decl = ((decl)->common.chain) )
      {
	if (((decl)->common.code)  == VAR_DECL && ((decl)->common.static_attr) 
	    && ! ((decl)->common.asm_written_attr) )
	  rest_of_decl_compilation (decl, 0, 1, 1);
	if (((decl)->common.code)  == FUNCTION_DECL
	    && ! ((decl)->common.asm_written_attr) 
	    && ((decl)->decl.initial)  != 0
	    && ((decl)->common.addressable_attr) )
	  output_inline_function (decl);

	if (warn_unused
	    && ((decl)->common.code)  == FUNCTION_DECL
	    && ((decl)->decl.initial)  == 0
	    && ((decl)->common.external_attr) 
	    && ! ((decl)->common.public_attr) )
	  warning_with_decl (decl, "`%s' declared but never defined");

	if (warn_unused
	    && (((decl)->common.code)  == FUNCTION_DECL
		|| ((decl)->common.code)  == VAR_DECL)
	    && ! ((decl)->common.external_attr) 
	    && ! ((decl)->common.public_attr) 
	    && ! ((decl)->common.used_attr) 
	    && ! ((decl)->common.inline_attr) )
	  warning_with_decl (decl, "`%s' defined but not used");
      }
  }

  if (write_symbols == DBX_DEBUG)
    do { int otime = gettime (); 
	     {
	       dbxout_tags (gettags ());
	       dbxout_types (get_permanent_types ());
	     }; symout_time += gettime () - otime; } while (0) ;

  if (write_symbols == GDB_DEBUG)
    do { int otime = gettime (); 
	     {
	       struct stat statbuf;
	       fstat (((finput)->_file) , &statbuf);
	       symout_types (get_permanent_types ());
	       symout_top_blocks (globals, gettags ());
	       symout_finish (name, statbuf.st_ctime);
	     }; symout_time += gettime () - otime; } while (0) ;

  end_final (main_input_filename);

  if (rtl_dump)
    fclose (rtl_dump_file);

  if (jump_opt_dump)
    fclose (jump_opt_dump_file);

  if (cse_dump)
    fclose (cse_dump_file);

  if (loop_dump)
    fclose (loop_dump_file);

  if (flow_dump)
    fclose (flow_dump_file);

  if (combine_dump)
    {
      dump_combine_total_stats (combine_dump_file);
      fclose (combine_dump_file);
    }

  if (local_reg_dump)
    fclose (local_reg_dump_file);

  if (global_reg_dump)
    fclose (global_reg_dump_file);

  if (jump2_opt_dump)
    fclose (jump2_opt_dump_file);

  fclose (finput);
  if ((((asm_out_file)->_flag&040 )!=0)  != 0)
    fatal_io_error (asm_file_name);
  fclose (asm_out_file);

  if (! quiet_flag)
    {
      fprintf ((&_iob[2]) ,"\n");
      print_time ("parse", parse_time);
      print_time ("integration", integration_time);
      print_time ("jump", jump_time);
      print_time ("cse", cse_time);
      print_time ("loop", loop_time);
      print_time ("flow", flow_time);
      print_time ("combine", combine_time);
      print_time ("local-alloc", local_alloc_time);
      print_time ("global-alloc", global_alloc_time);
      print_time ("final", final_time);
      print_time ("varconst", varconst_time);
      print_time ("symout", symout_time);
      print_time ("dump", dump_time);
    }
}

void
rest_of_decl_compilation (decl, asmspec, top_level, at_end)
     tree decl;
     tree asmspec;
     int top_level;
     int at_end;
{

  if (((decl)->common.static_attr)  || ((decl)->common.external_attr) )
    do { int otime = gettime (); 
	     {
	       make_decl_rtl (decl, asmspec, top_level);
	       if (! (! at_end && top_level
		      && (((decl)->decl.initial)  == 0
			  || ((decl)->decl.initial)  == error_mark_node)))
		 assemble_variable (decl, top_level, write_symbols, at_end);
	     }; varconst_time += gettime () - otime; } while (0) ;

  else if (write_symbols == DBX_DEBUG && ((decl)->common.code)  == TYPE_DECL)
    do { int otime = gettime ();  dbxout_symbol (decl, 0); varconst_time += gettime () - otime; } while (0) ;

  if (top_level)
    {
      if (write_symbols == GDB_DEBUG)
	{
	  do { int otime = gettime (); 
		   {
		     symout_types (get_temporary_types ());
		   }; symout_time += gettime () - otime; } while (0) ;

	}
      else
	get_temporary_types ();
    }
}

void
rest_of_compilation (decl)
     tree decl;
{
  register rtx insns;
  int start_time = gettime ();
  int tem;

  if (((decl)->decl.saved_insns)  == 0)
    {

      if (flag_inline_functions || ((decl)->common.inline_attr) )
	{
	  do { int otime = gettime (); 
		   {
		     int specd = ((decl)->common.inline_attr) ;
		     char *lose = function_cannot_inline_p (decl);
		     if (lose != 0 && specd)
		       warning_with_decl (decl, lose);
		     if (lose == 0)
		       save_for_inline (decl);
		     else
		       ((decl)->common.inline_attr)  = 0;
		   }; integration_time += gettime () - otime; } while (0) ;
	}

      insns = get_insns ();

      if (rtl_dump)
	do { int otime = gettime (); 
		 {
		   fprintf (rtl_dump_file, "\n;; Function %s\n\n",
			    ((((decl)->decl.name) )->identifier.pointer) );
		   if (((decl)->decl.saved_insns) )
		     fprintf (rtl_dump_file, ";; (integrable)\n\n");
		   print_rtl (rtl_dump_file, insns);
		   fflush (rtl_dump_file);
		 }; dump_time += gettime () - otime; } while (0) ;

      if (((decl)->common.public_attr)  == 0
	  && ((decl)->common.inline_attr) 
	  && ! flag_keep_inline_functions)
	goto exit_rest_of_compilation;
    }

  if (rtl_dump_and_exit)
    goto exit_rest_of_compilation;

  ((decl)->common.asm_written_attr)  = 1;

  insns = get_insns ();

  unshare_all_rtl (insns);

  if (optimize || extra_warnings || warn_return_type
      || ((decl)->common.this_vol_attr) )
    do { int otime = gettime ();  jump_optimize (insns, 0, 0); jump_time += gettime () - otime; } while (0) ;

  if (jump_opt_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (jump_opt_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	       print_rtl (jump_opt_dump_file, insns);
	       fflush (jump_opt_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  if (optimize)
    {
      do { int otime = gettime ();  reg_scan (insns, max_reg_num (), 0); cse_time += gettime () - otime; } while (0) ;

      do { int otime = gettime ();  tem = cse_main (insns, max_reg_num ()); cse_time += gettime () - otime; } while (0) ;

      if (tem)
	do { int otime = gettime ();  jump_optimize (insns, 0, 0); jump_time += gettime () - otime; } while (0) ;
    }

  if (cse_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (cse_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	       print_rtl (cse_dump_file, insns);
	       fflush (cse_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  if (loop_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (loop_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	     }; dump_time += gettime () - otime; } while (0) ;

  if (optimize)
    {
      do { int otime = gettime (); 
	       {
		 reg_scan (insns, max_reg_num (), 1);
		 loop_optimize (insns, loop_dump ? loop_dump_file : 0);
	       }; loop_time += gettime () - otime; } while (0) ;
    }

  if (loop_dump)
    do { int otime = gettime (); 
	     {
	       print_rtl (loop_dump_file, insns);
	       fflush (loop_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  if (optimize)		 
    obey_regdecls = 0;	 

  regclass_init ();

  if (flow_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (flow_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	     }; dump_time += gettime () - otime; } while (0) ;

  if (obey_regdecls)
    {
      do { int otime = gettime (); 
	       {
		 regclass (insns, max_reg_num ());
		 stupid_life_analysis (insns, max_reg_num (),
				       flow_dump_file);
	       }; flow_time += gettime () - otime; } while (0) ;
    }
  else
    {

      do { int otime = gettime ();  flow_analysis (insns, max_reg_num (),
					 flow_dump_file); flow_time += gettime () - otime; } while (0) ;
      if (extra_warnings)
	uninitialized_vars_warning (((decl)->decl.initial) );
    }

  if (flow_dump)
    do { int otime = gettime (); 
	     {
	       print_rtl (flow_dump_file, insns);
	       fflush (flow_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  if (optimize)
    do { int otime = gettime ();  combine_instructions (insns, max_reg_num ()); combine_time += gettime () - otime; } while (0) ;

  if (combine_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (combine_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	       dump_combine_stats (combine_dump_file);
	       print_rtl (combine_dump_file, insns);
	       fflush (combine_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  if (!obey_regdecls)
    do { int otime = gettime (); 
	     {
	       regclass (insns, max_reg_num ());
	       local_alloc ();
	     }; local_alloc_time += gettime () - otime; } while (0) ;

  if (local_reg_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (local_reg_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	       dump_flow_info (local_reg_dump_file);
	       dump_local_alloc (local_reg_dump_file);
	       print_rtl (local_reg_dump_file, insns);
	       fflush (local_reg_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  if (global_reg_dump)
    do { int otime = gettime (); 
	     fprintf (global_reg_dump_file, "\n;; Function %s\n\n",
		      ((((decl)->decl.name) )->identifier.pointer) ); dump_time += gettime () - otime; } while (0) ;

  do { int otime = gettime (); 
	   {
	     if (!obey_regdecls)
	       global_alloc (global_reg_dump ? global_reg_dump_file : 0);
	     else
	       reload (insns, 0,
		       global_reg_dump ? global_reg_dump_file : 0);
	   }; global_alloc_time += gettime () - otime; } while (0) ;

  if (global_reg_dump)
    do { int otime = gettime (); 
	     {
	       dump_global_regs (global_reg_dump_file);
	       print_rtl (global_reg_dump_file, insns);
	       fflush (global_reg_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  rtx_equal_function_value_matters = 1;
  reload_completed = 1;

  if (optimize)
    {
      do { int otime = gettime ();  jump_optimize (insns, 1, 1); jump_time += gettime () - otime; } while (0) ;
    }

  if (jump2_opt_dump)
    do { int otime = gettime (); 
	     {
	       fprintf (jump2_opt_dump_file, "\n;; Function %s\n\n",
			((((decl)->decl.name) )->identifier.pointer) );
	       print_rtl (jump2_opt_dump_file, insns);
	       fflush (jump2_opt_dump_file);
	     }; dump_time += gettime () - otime; } while (0) ;

  do { int otime = gettime (); 
	   {
	     assemble_function (decl);
	     final_start_function (insns, asm_out_file,
				   write_symbols, optimize);
	     final (insns, asm_out_file,
		    write_symbols, optimize, 0);
	     final_end_function (insns, asm_out_file,
				 write_symbols, optimize);
	     fflush (asm_out_file);
	   }; final_time += gettime () - otime; } while (0) ;

  if (write_symbols == GDB_DEBUG)
    {
      do { int otime = gettime (); 
	       {
		 symout_types (get_permanent_types ());
		 symout_types (get_temporary_types ());

		 ((decl)->decl.block_symtab_address) 
		   = symout_function (((decl)->decl.initial) ,
				      ((decl)->decl.arguments)  , 0);
		 symout_function_end ();
	       }; symout_time += gettime () - otime; } while (0) ;
    }
  else
    get_temporary_types ();

  if (write_symbols == DBX_DEBUG)
    do { int otime = gettime ();  dbxout_function (decl); symout_time += gettime () - otime; } while (0) ;

 exit_rest_of_compilation:

  rtx_equal_function_value_matters = 0;
  reload_completed = 0;

  clear_const_double_mem ();

  parse_time -= gettime () - start_time;
}

int
main (argc, argv, envp)
     int argc;
     char **argv;
     char **envp;
{
  register int i;
  char *filename = 0;
  int print_mem_flag = 0;

  signal (8	, float_signal);

  signal (13	, pipe_closed);

  flag_signed_char = 1 ;

  obey_regdecls = 1;

  init_reg_sets ();

  target_flags = 0;
  set_target_switch ("");

  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-' && argv[i][1] != 0)
      {
	register char *str = argv[i] + 1;
	if (str[0] == 'Y')
	  str++;

	if (str[0] == 'm')
	  set_target_switch (&str[1]);
	else if (!strcmp (str, "dumpbase"))
	  {
	    dump_base_name = argv[++i];
	  }
	else if (str[0] == 'd')
	  {
	    register char *p = &str[1];
	    while (*p)
	      switch (*p++)
		{
		case 'c':
		  combine_dump = 1;
		  break;
		case 'f':
		  flow_dump = 1;
		  break;
		case 'g':
		  global_reg_dump = 1;
		  break;
		case 'j':
		  jump_opt_dump = 1;
		  break;
		case 'J':
		  jump2_opt_dump = 1;
		  break;
		case 'l':
		  local_reg_dump = 1;
		  break;
		case 'L':
		  loop_dump = 1;
		  break;
		case 'm':
		  print_mem_flag = 1;
		  break;
		case 'r':
		  rtl_dump = 1;
		  break;
		case 's':
		  cse_dump = 1;
		  break;
		case 'y':
		  yydebug = 1;
		  break;
		}
	  }
	else if (str[0] == 'f')
	  {
	    int j;
	    register char *p = &str[1];
	    int found = 0;

	    for (j = 0;
		 !found && j < sizeof (f_options) / sizeof (f_options[0]);
		 j++)
	      {
		if (!strcmp (p, f_options[j].string))
		  {
		    *f_options[j].variable = f_options[j].on_value;

		    found = 1;
		  }
		if (p[0] == 'n' && p[1] == 'o' && p[2] == '-'
		    && ! strcmp (p+3, f_options[j].string))
		  {
		    *f_options[j].variable = ! f_options[j].on_value;
		    found = 1;
		  }
	      }

	    if (found)
	      ;
	    else if (!strncmp (p, "fixed-", 6))
	      fix_register (&p[6], 1, 1);
	    else if (!strncmp (p, "call-used-", 10))
	      fix_register (&p[10], 0, 1);
	    else if (!strncmp (p, "call-saved-", 11))
	      fix_register (&p[11], 0, 0);
	    else if (! lang_decode_option (argv[i]))
	      error ("Invalid option `%s'", argv[i]);	      
	  }
	else if (!strcmp (str, "noreg"))
	  ;
	else if (!strcmp (str, "opt"))
	  optimize = 1, obey_regdecls = 0;
	else if (!strcmp (str, "O"))
	  optimize = 1, obey_regdecls = 0;
	else if (!strcmp (str, "pedantic"))
	  pedantic = 1;
	else if (lang_decode_option (argv[i]))
	  ;
	else if (!strcmp (str, "quiet"))
	  quiet_flag = 1;
	else if (!strcmp (str, "version"))
	  {
	    extern char *version_string, *language_string;
	    fprintf ((&_iob[2]) , "%s version %s", language_string, version_string);

	    fprintf ((&_iob[2]) , " (68k, MIT syntax)"); ;

	    fprintf ((&_iob[2]) , " compiled by GNU C version %s.\n", "1.35");

	  }
	else if (!strcmp (str, "w"))
	  inhibit_warnings = 1;
	else if (!strcmp (str, "W"))
	  extra_warnings = 1;
	else if (!strcmp (str, "Wunused"))
	  warn_unused = 1;
	else if (!strcmp (str, "Wshadow"))
	  warn_shadow = 1;
	else if (!strcmp (str, "Wswitch"))
	  warn_switch = 1;
	else if (!strncmp (str, "Wid-clash-", 10))
	  {
	    char *endp = str + 10;

	    while (*endp)
	      {
		if (*endp >= '0' && *endp <= '9')
		  endp++;
		else
		  error ("Invalid option `%s'", argv[i]);
	      }
	    warn_id_clash = 1;
	    id_clash_len = atoi (str + 10);
	  }
	else if (!strcmp (str, "p"))
	  profile_flag = 1;
	else if (!strcmp (str, "a"))
	  {

	    profile_block_flag = 1;

	  }
	else if (!strcmp (str, "gg"))
	  write_symbols = GDB_DEBUG;

	else if (!strcmp (str, "g"))
	  write_symbols = DBX_DEBUG;
	else if (!strcmp (str, "G"))
	  write_symbols = DBX_DEBUG;

	else if (!strcmp (str, "symout"))
	  {
	    if (write_symbols == NO_DEBUG)
	      write_symbols = GDB_DEBUG;
	    sym_file_name = argv[++i];
	  }
	else if (!strcmp (str, "o"))
	  {
	    asm_file_name = argv[++i];
	  }
	else
	  error ("Invalid option `%s'", argv[i]);
      }
    else
      filename = argv[i];

  {	if ((target_flags & 0100) ) target_flags &= ~2;	} ;

  init_reg_sets_1 ();

  compile_file (filename);

  if (errorcount)
    exit (33 );
  if (sorrycount)
    exit (33 );
  exit (0 );
  return 34;
}

struct {char *name; int value;} target_switches []
  = { { "68020", 5},	{ "c68020", 5},	{ "68881", 2},	{ "bitfield", 4},	{ "68000", -5},	{ "c68000", -5},	{ "soft-float", -0102},	{ "nobitfield", -4},	{ "rtd", 8},	{ "nortd", -8},	{ "short", 040},	{ "noshort", -040},	{ "fpa", 0100},	{ "nofpa", -0100},	{ "", 7 }} ;

void
set_target_switch (name)
     char *name;
{
  register int j;
  for (j = 0; j < sizeof target_switches / sizeof target_switches[0]; j++)
    if (!strcmp (target_switches[j].name, name))
      {
	if (target_switches[j].value < 0)
	  target_flags &= ~-target_switches[j].value;
	else
	  target_flags |= target_switches[j].value;
	return;
      }
  error ("Invalid option `%s'", name);
}

