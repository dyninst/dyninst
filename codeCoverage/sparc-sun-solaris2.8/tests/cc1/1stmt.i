
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

MAX_MACHINE_MODE };

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

extern rtx gen_tstsi ();

extern rtx gen_tsthi ();

extern rtx gen_tstqi ();

extern rtx gen_tstsf ();

extern rtx gen_tstdf ();

extern rtx gen_cmpsi ();

extern rtx gen_cmphi ();

extern rtx gen_cmpqi ();

extern rtx gen_cmpdf ();

extern rtx gen_cmpsf ();

extern rtx gen_movsi ();

extern rtx gen_movhi ();

extern rtx gen_movstricthi ();

extern rtx gen_movqi ();

extern rtx gen_movstrictqi ();

extern rtx gen_movsf ();

extern rtx gen_movdf ();

extern rtx gen_movdi ();

extern rtx gen_pushasi ();

extern rtx gen_truncsiqi2 ();

extern rtx gen_trunchiqi2 ();

extern rtx gen_truncsihi2 ();

extern rtx gen_zero_extendhisi2 ();

extern rtx gen_zero_extendqihi2 ();

extern rtx gen_zero_extendqisi2 ();

extern rtx gen_extendhisi2 ();

extern rtx gen_extendqihi2 ();

extern rtx gen_extendqisi2 ();

extern rtx gen_extendsfdf2 ();

extern rtx gen_truncdfsf2 ();

extern rtx gen_floatsisf2 ();

extern rtx gen_floatsidf2 ();

extern rtx gen_floathisf2 ();

extern rtx gen_floathidf2 ();

extern rtx gen_floatqisf2 ();

extern rtx gen_floatqidf2 ();

extern rtx gen_ftruncdf2 ();

extern rtx gen_ftruncsf2 ();

extern rtx gen_fixsfqi2 ();

extern rtx gen_fixsfhi2 ();

extern rtx gen_fixsfsi2 ();

extern rtx gen_fixdfqi2 ();

extern rtx gen_fixdfhi2 ();

extern rtx gen_fixdfsi2 ();

extern rtx gen_fix_truncsfsi2 ();

extern rtx gen_fix_truncdfsi2 ();

extern rtx gen_addsi3 ();

extern rtx gen_addhi3 ();

extern rtx gen_addqi3 ();

extern rtx gen_adddf3 ();

extern rtx gen_addsf3 ();

extern rtx gen_subsi3 ();

extern rtx gen_subhi3 ();

extern rtx gen_subqi3 ();

extern rtx gen_subdf3 ();

extern rtx gen_subsf3 ();

extern rtx gen_mulhi3 ();

extern rtx gen_mulhisi3 ();

extern rtx gen_mulsi3 ();

extern rtx gen_umulhi3 ();

extern rtx gen_umulhisi3 ();

extern rtx gen_umulsi3 ();

extern rtx gen_muldf3 ();

extern rtx gen_mulsf3 ();

extern rtx gen_divhi3 ();

extern rtx gen_divhisi3 ();

extern rtx gen_divsi3 ();

extern rtx gen_udivhi3 ();

extern rtx gen_udivhisi3 ();

extern rtx gen_udivsi3 ();

extern rtx gen_divdf3 ();

extern rtx gen_divsf3 ();

extern rtx gen_modhi3 ();

extern rtx gen_modhisi3 ();

extern rtx gen_umodhi3 ();

extern rtx gen_umodhisi3 ();

extern rtx gen_divmodsi4 ();

extern rtx gen_udivmodsi4 ();

extern rtx gen_andsi3 ();

extern rtx gen_andhi3 ();

extern rtx gen_andqi3 ();

extern rtx gen_iorsi3 ();

extern rtx gen_iorhi3 ();

extern rtx gen_iorqi3 ();

extern rtx gen_xorsi3 ();

extern rtx gen_xorhi3 ();

extern rtx gen_xorqi3 ();

extern rtx gen_negsi2 ();

extern rtx gen_neghi2 ();

extern rtx gen_negqi2 ();

extern rtx gen_negsf2 ();

extern rtx gen_negdf2 ();

extern rtx gen_abssf2 ();

extern rtx gen_absdf2 ();

extern rtx gen_one_cmplsi2 ();

extern rtx gen_one_cmplhi2 ();

extern rtx gen_one_cmplqi2 ();

extern rtx gen_ashlsi3 ();

extern rtx gen_ashlhi3 ();

extern rtx gen_ashlqi3 ();

extern rtx gen_ashrsi3 ();

extern rtx gen_ashrhi3 ();

extern rtx gen_ashrqi3 ();

extern rtx gen_lshlsi3 ();

extern rtx gen_lshlhi3 ();

extern rtx gen_lshlqi3 ();

extern rtx gen_lshrsi3 ();

extern rtx gen_lshrhi3 ();

extern rtx gen_lshrqi3 ();

extern rtx gen_rotlsi3 ();

extern rtx gen_rotlhi3 ();

extern rtx gen_rotlqi3 ();

extern rtx gen_rotrsi3 ();

extern rtx gen_rotrhi3 ();

extern rtx gen_rotrqi3 ();

extern rtx gen_extv ();

extern rtx gen_extzv ();

extern rtx gen_insv ();

extern rtx gen_seq ();

extern rtx gen_sne ();

extern rtx gen_sgt ();

extern rtx gen_sgtu ();

extern rtx gen_slt ();

extern rtx gen_sltu ();

extern rtx gen_sge ();

extern rtx gen_sgeu ();

extern rtx gen_sle ();

extern rtx gen_sleu ();

extern rtx gen_beq ();

extern rtx gen_bne ();

extern rtx gen_bgt ();

extern rtx gen_bgtu ();

extern rtx gen_blt ();

extern rtx gen_bltu ();

extern rtx gen_bge ();

extern rtx gen_bgeu ();

extern rtx gen_ble ();

extern rtx gen_bleu ();

extern rtx gen_casesi_1 ();

extern rtx gen_casesi_2 ();

extern rtx gen_casesi ();

extern rtx gen_jump ();

extern rtx gen_call ();

extern rtx gen_call_value ();

extern rtx gen_return ();

enum expand_modifier {EXPAND_NORMAL, EXPAND_SUM, EXPAND_CONST_ADDRESS};

extern int cse_not_expected;

extern rtx save_expr_regs;

struct args_size
{
  int constant;
  tree var;
};

enum direction {none, upward, downward};   

enum insn_code {
  CODE_FOR_tstsi = 2,
  CODE_FOR_tsthi = 3,
  CODE_FOR_tstqi = 4,
  CODE_FOR_tstsf = 5,
  CODE_FOR_tstdf = 8,
  CODE_FOR_cmpsi = 11,
  CODE_FOR_cmphi = 12,
  CODE_FOR_cmpqi = 13,
  CODE_FOR_cmpdf = 14,
  CODE_FOR_cmpsf = 17,
  CODE_FOR_movsi = 31,
  CODE_FOR_movhi = 32,
  CODE_FOR_movstricthi = 33,
  CODE_FOR_movqi = 34,
  CODE_FOR_movstrictqi = 35,
  CODE_FOR_movsf = 36,
  CODE_FOR_movdf = 37,
  CODE_FOR_movdi = 38,
  CODE_FOR_pushasi = 39,
  CODE_FOR_truncsiqi2 = 40,
  CODE_FOR_trunchiqi2 = 41,
  CODE_FOR_truncsihi2 = 42,
  CODE_FOR_zero_extendhisi2 = 43,
  CODE_FOR_zero_extendqihi2 = 44,
  CODE_FOR_zero_extendqisi2 = 45,
  CODE_FOR_extendhisi2 = 49,
  CODE_FOR_extendqihi2 = 50,
  CODE_FOR_extendqisi2 = 51,
  CODE_FOR_extendsfdf2 = 52,
  CODE_FOR_truncdfsf2 = 55,
  CODE_FOR_floatsisf2 = 58,
  CODE_FOR_floatsidf2 = 61,
  CODE_FOR_floathisf2 = 64,
  CODE_FOR_floathidf2 = 65,
  CODE_FOR_floatqisf2 = 66,
  CODE_FOR_floatqidf2 = 67,
  CODE_FOR_ftruncdf2 = 68,
  CODE_FOR_ftruncsf2 = 69,
  CODE_FOR_fixsfqi2 = 70,
  CODE_FOR_fixsfhi2 = 71,
  CODE_FOR_fixsfsi2 = 72,
  CODE_FOR_fixdfqi2 = 73,
  CODE_FOR_fixdfhi2 = 74,
  CODE_FOR_fixdfsi2 = 75,
  CODE_FOR_fix_truncsfsi2 = 76,
  CODE_FOR_fix_truncdfsi2 = 77,
  CODE_FOR_addsi3 = 78,
  CODE_FOR_addhi3 = 80,
  CODE_FOR_addqi3 = 82,
  CODE_FOR_adddf3 = 84,
  CODE_FOR_addsf3 = 87,
  CODE_FOR_subsi3 = 90,
  CODE_FOR_subhi3 = 92,
  CODE_FOR_subqi3 = 94,
  CODE_FOR_subdf3 = 96,
  CODE_FOR_subsf3 = 99,
  CODE_FOR_mulhi3 = 102,
  CODE_FOR_mulhisi3 = 103,
  CODE_FOR_mulsi3 = 104,
  CODE_FOR_umulhi3 = 105,
  CODE_FOR_umulhisi3 = 106,
  CODE_FOR_umulsi3 = 107,
  CODE_FOR_muldf3 = 108,
  CODE_FOR_mulsf3 = 111,
  CODE_FOR_divhi3 = 114,
  CODE_FOR_divhisi3 = 115,
  CODE_FOR_divsi3 = 116,
  CODE_FOR_udivhi3 = 117,
  CODE_FOR_udivhisi3 = 118,
  CODE_FOR_udivsi3 = 119,
  CODE_FOR_divdf3 = 120,
  CODE_FOR_divsf3 = 123,
  CODE_FOR_modhi3 = 126,
  CODE_FOR_modhisi3 = 127,
  CODE_FOR_umodhi3 = 128,
  CODE_FOR_umodhisi3 = 129,
  CODE_FOR_divmodsi4 = 130,
  CODE_FOR_udivmodsi4 = 131,
  CODE_FOR_andsi3 = 132,
  CODE_FOR_andhi3 = 133,
  CODE_FOR_andqi3 = 134,
  CODE_FOR_iorsi3 = 137,
  CODE_FOR_iorhi3 = 138,
  CODE_FOR_iorqi3 = 139,
  CODE_FOR_xorsi3 = 140,
  CODE_FOR_xorhi3 = 141,
  CODE_FOR_xorqi3 = 142,
  CODE_FOR_negsi2 = 143,
  CODE_FOR_neghi2 = 144,
  CODE_FOR_negqi2 = 145,
  CODE_FOR_negsf2 = 146,
  CODE_FOR_negdf2 = 149,
  CODE_FOR_abssf2 = 152,
  CODE_FOR_absdf2 = 155,
  CODE_FOR_one_cmplsi2 = 158,
  CODE_FOR_one_cmplhi2 = 159,
  CODE_FOR_one_cmplqi2 = 160,
  CODE_FOR_ashlsi3 = 167,
  CODE_FOR_ashlhi3 = 168,
  CODE_FOR_ashlqi3 = 169,
  CODE_FOR_ashrsi3 = 170,
  CODE_FOR_ashrhi3 = 171,
  CODE_FOR_ashrqi3 = 172,
  CODE_FOR_lshlsi3 = 173,
  CODE_FOR_lshlhi3 = 174,
  CODE_FOR_lshlqi3 = 175,
  CODE_FOR_lshrsi3 = 176,
  CODE_FOR_lshrhi3 = 177,
  CODE_FOR_lshrqi3 = 178,
  CODE_FOR_rotlsi3 = 179,
  CODE_FOR_rotlhi3 = 180,
  CODE_FOR_rotlqi3 = 181,
  CODE_FOR_rotrsi3 = 182,
  CODE_FOR_rotrhi3 = 183,
  CODE_FOR_rotrqi3 = 184,
  CODE_FOR_extv = 188,
  CODE_FOR_extzv = 189,
  CODE_FOR_insv = 193,
  CODE_FOR_seq = 205,
  CODE_FOR_sne = 206,
  CODE_FOR_sgt = 207,
  CODE_FOR_sgtu = 208,
  CODE_FOR_slt = 209,
  CODE_FOR_sltu = 210,
  CODE_FOR_sge = 211,
  CODE_FOR_sgeu = 212,
  CODE_FOR_sle = 213,
  CODE_FOR_sleu = 214,
  CODE_FOR_beq = 215,
  CODE_FOR_bne = 216,
  CODE_FOR_bgt = 217,
  CODE_FOR_bgtu = 218,
  CODE_FOR_blt = 219,
  CODE_FOR_bltu = 220,
  CODE_FOR_bge = 221,
  CODE_FOR_bgeu = 222,
  CODE_FOR_ble = 223,
  CODE_FOR_bleu = 224,
  CODE_FOR_casesi_1 = 235,
  CODE_FOR_casesi_2 = 236,
  CODE_FOR_casesi = 237,
  CODE_FOR_jump = 239,
  CODE_FOR_call = 243,
  CODE_FOR_call_value = 244,
  CODE_FOR_return = 245,
  CODE_FOR_nothing };

typedef struct optab
{
  enum rtx_code code;
  struct {
    enum insn_code insn_code;
    char *lib_call;
  } handlers [(int) MAX_MACHINE_MODE ];
} * optab;

extern rtx (*insn_gen_function[]) ();

extern optab add_optab;
extern optab sub_optab;
extern optab smul_optab;	 
extern optab umul_optab;	 
extern optab smul_widen_optab;	 

extern optab umul_widen_optab;
extern optab sdiv_optab;	 
extern optab sdivmod_optab;	 
extern optab udiv_optab;
extern optab udivmod_optab;
extern optab smod_optab;	 
extern optab umod_optab;
extern optab flodiv_optab;	 
extern optab ftrunc_optab;	 
extern optab and_optab;		 
extern optab andcb_optab;	 
extern optab ior_optab;		 
extern optab xor_optab;		 
extern optab ashl_optab;	 
extern optab ashr_optab;	 
extern optab lshl_optab;	 
extern optab lshr_optab;	 
extern optab rotl_optab;	 
extern optab rotr_optab;	 

extern optab mov_optab;		 
extern optab movstrict_optab;	 

extern optab cmp_optab;		 
extern optab tst_optab;		 

extern optab neg_optab;		 
extern optab abs_optab;		 
extern optab one_cmpl_optab;	 
extern optab ffs_optab;		 

enum optab_methods
{
  OPTAB_DIRECT,
  OPTAB_LIB,
  OPTAB_WIDEN,
  OPTAB_LIB_WIDEN,
};
typedef rtx (*rtxfun) ();

extern rtxfun bcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];

extern rtxfun setcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];

rtx expand_binop ();

rtx sign_expand_binop ();

rtx expand_unop ();

rtx negate_rtx ();

void init_fixtab ();
void init_floattab ();

void expand_fix ();

void expand_float ();

rtx gen_add2_insn ();
rtx gen_sub2_insn ();
rtx gen_move_insn ();

void emit_clr_insn ();

void emit_0_to_1_insn ();

void emit_cmp_insn ();

void convert_move ();

rtx convert_to_mode ();

void emit_library_call ();

rtx force_operand ();

rtx expr_size ();

rtx plus_constant ();

rtx lookup_static_chain ();

rtx eliminate_constant_term ();

rtx memory_address ();

rtx memory_address_noforce ();

rtx change_address ();

int rtx_equal_p ();

rtx stabilize ();

rtx copy_all_regs ();

rtx copy_to_reg ();

rtx copy_addr_to_reg ();

rtx copy_to_mode_reg ();

rtx copy_to_suggested_reg ();

rtx force_reg ();

rtx force_not_mem ();

void adjust_stack ();

void anti_adjust_stack ();

rtx function_value ();

rtx hard_function_value ();

rtx hard_libcall_value ();

void copy_function_value ();

rtx round_push ();

rtx store_bit_field ();
rtx extract_bit_field ();
rtx expand_shift ();
rtx expand_bit_and ();
rtx expand_mult ();
rtx expand_divmod ();
rtx get_structure_value_addr ();
rtx expand_stmt_expr ();

void jumpifnot ();
void jumpif ();
void do_jump ();

rtx assemble_static_space ();

extern int max_regno;

extern short *reg_n_refs;

extern short *reg_n_sets;

extern short *reg_n_deaths;

extern int *reg_n_calls_crossed;

extern int *reg_live_length;

extern short *reg_renumber;

extern char regs_ever_live[56 ];

extern char *reg_names[56 ];

extern short *regno_first_uid;

extern short *regno_last_uid;

extern char *regno_pointer_flag;

extern rtx *regno_reg_rtx;

extern int caller_save_needed;

int current_function_pops_args;

int current_function_returns_struct;

int current_function_returns_pcc_struct;

int current_function_needs_context;

int current_function_calls_setjmp;

int current_function_args_size;

int current_function_pretend_args_size;

char *current_function_name;

rtx return_label;

rtx save_expr_regs;

rtx stack_slot_list;

char *emit_filename;
int emit_lineno;

static rtx parm_birth_insn;

static tree this_function;

static int frame_offset;

static int invalid_stack_slot;

static rtx tail_recursion_label;

static rtx tail_recursion_reentry;

static tree last_expr_type;
static rtx last_expr_value;

static tree rtl_expr_chain;

static rtx last_parm_insn;

static void expand_goto_internal ();
static int expand_fixup ();
static void fixup_gotos ();
static void expand_cleanups ();
static void fixup_cleanups ();
static void expand_null_return_1 ();
static int tail_recursion_args ();
static void fixup_stack_slots ();
static rtx fixup_stack_1 ();
static rtx fixup_memory_subreg ();
static rtx walk_fixup_memory_subreg ();
static void fixup_var_refs ();
static void fixup_var_refs_insns ();
static rtx fixup_var_refs_1 ();
static rtx parm_stack_loc ();
static void optimize_bit_field ();
static void do_jump_if_equal ();

static void balance_case_nodes ();
static void emit_case_nodes ();
static void group_case_nodes ();
static void emit_jump_if_reachable ();

struct case_node
{
  struct case_node	*left;
  struct case_node	*right;
  struct case_node	*parent;
  tree			low;
  tree			high;
  tree			test_label;
  tree			code_label;
};

typedef struct case_node case_node;
typedef struct case_node *case_node_ptr;

struct nesting
{
  struct nesting *all;
  struct nesting *next;
  int depth;
  rtx exit_label;
  union
    {
      struct
	{
	  rtx else_label;
	  rtx after_label;
	} cond;
      struct
	{
	  rtx start_label;
	  rtx end_label;

	  rtx continue_label;
	} loop;
      struct
	{
	  rtx stack_level;

	  rtx first_insn;
	  struct nesting *innermost_stack_block;

	  tree cleanups;

	  tree outer_cleanups;

	  struct label_chain *label_chain;
	} block;

      struct
	{

	  rtx start;

	  struct case_node *case_list;
	  tree default_label;
	  tree index_expr;
	  tree nominal_type;
	  short num_ranges;
	} case_stmt;
    } data;
};

struct nesting *block_stack;

struct nesting *stack_block_stack;

struct nesting *cond_stack;

struct nesting *loop_stack;

struct nesting *case_stack;

struct nesting *nesting_stack;

int nesting_depth;

static rtx
label_rtx (label)
     tree label;
{
  if (((label)->common.code)  != LABEL_DECL)
    abort ();

  if (((label)->decl.rtl) )
    return ((label)->decl.rtl) ;

  return ((label)->decl.rtl)  = gen_label_rtx ();
}

void
emit_jump (label)
     rtx label;
{
  do_pending_stack_adjust ();
  emit_jump_insn (gen_jump (label));
  emit_barrier ();
}

struct goto_fixup
{
  struct goto_fixup *next;

  rtx before_jump;

  tree target;
  rtx target_rtl;

  rtx stack_level;

  tree cleanup_list_list;
};

static struct goto_fixup *goto_fixup_chain;

struct label_chain
{
  struct label_chain *next;
  tree label;
};

void
expand_label (body)
     tree body;
{
  struct label_chain *p;

  do_pending_stack_adjust ();
  emit_label (label_rtx (body));

  if (stack_block_stack != 0)
    {
      p = (struct label_chain *) oballoc (sizeof (struct label_chain));
      p->next = stack_block_stack->data.block.label_chain;
      stack_block_stack->data.block.label_chain = p;
      p->label = body;
    }
}

void
expand_goto (body)
     tree body;
{
  expand_goto_internal (body, label_rtx (body), 0);
}

static void
expand_goto_internal (body, label, last_insn)
     tree body;
     rtx label;
     rtx last_insn;
{
  struct nesting *block;
  rtx stack_level = 0;

  if (	((label)->code)  != CODE_LABEL)
    abort ();

  if (((label)->fld[1].rtx)  != 0)
    {

      for (block = block_stack; block; block = block->next)
	{
	  if (((block->data.block.first_insn)->fld[0].rtint)  < ((label)->fld[0].rtint) )
	    break;
	  if (block->data.block.stack_level != 0)
	    stack_level = block->data.block.stack_level;
	  if (block->data.block.cleanups != 0)
	    expand_cleanups (block->data.block.cleanups, 0);
	}

      if (stack_level)
	emit_move_insn (stack_pointer_rtx, stack_level);

      if (body != 0 && ((body)->common.packed_attr) )
	error ("jump to `%s' invalidly jumps into binding contour",
	       ((((body)->decl.name) )->identifier.pointer) );
    }

  else if (! expand_fixup (body, label, last_insn))
    {

      if (body != 0)
	((body)->common.addressable_attr)  = 1;
    }

  emit_jump (label);
}

static int
expand_fixup (tree_label, rtl_label, last_insn)
     tree tree_label;
     rtx rtl_label;
     rtx last_insn;
{
  struct nesting *block, *end_block;

  if (cond_stack
      && (rtl_label == cond_stack->data.cond.else_label
	  || rtl_label == cond_stack->data.cond.after_label))
    end_block = cond_stack;

  else if (loop_stack
      && (rtl_label == loop_stack->data.loop.start_label
	  || rtl_label == loop_stack->data.loop.end_label
	  || rtl_label == loop_stack->data.loop.continue_label))
    end_block = loop_stack;
  else
    end_block = 0;

  if (end_block)
    {
      struct nesting *next_block = end_block->all;
      block = block_stack;

      while (next_block && next_block != block)
	next_block = next_block->all;

      if (next_block)
	return 0;

      next_block = block_stack->next;
      for (block = block_stack; block != end_block; block = block->all)
	if (block == next_block)
	  next_block = next_block->next;
      end_block = next_block;
    }

  for (block = block_stack; block != end_block; block = block->next)
    if (block->data.block.stack_level != 0
	|| block->data.block.cleanups != 0)
      break;

  if (block != end_block)
    {
      struct goto_fixup *fixup
	= (struct goto_fixup *) oballoc (sizeof (struct goto_fixup));

      do_pending_stack_adjust ();
      fixup->before_jump = last_insn ? last_insn : get_last_insn ();
      fixup->target = tree_label;
      fixup->target_rtl = rtl_label;
      fixup->stack_level = 0;
      fixup->cleanup_list_list
	= (block->data.block.outer_cleanups || block->data.block.cleanups
	   ? tree_cons (0, block->data.block.cleanups,
			block->data.block.outer_cleanups)
	   : 0);
      fixup->next = goto_fixup_chain;
      goto_fixup_chain = fixup;
    }

  return block != 0;
}

static void
fixup_gotos (thisblock, stack_level, cleanup_list, first_insn, dont_jump_in)
     struct nesting *thisblock;
     rtx stack_level;
     tree cleanup_list;
     rtx first_insn;
     int dont_jump_in;
{
  register struct goto_fixup *f, *prev;

  for (prev = 0, f = goto_fixup_chain; f; prev = f, f = f->next)
    {
      if (f->before_jump == 0)
	{
	  if (prev != 0)
	    prev->next = f->next;
	}

      else if (((f->target_rtl)->fld[1].rtx)  != 0)
	{

	  if (f->target != 0
	      && (dont_jump_in || stack_level || cleanup_list)
	      && ((first_insn)->fld[0].rtint)  > ((f->before_jump)->fld[0].rtint) 
	      && ! ((f->target)->common.addressable_attr) )
	    {
	      error_with_decl (f->target,
			       "label `%s' used before containing binding contour");
	      ((f->target)->common.addressable_attr)  = 1;
	    }

	  if (f->cleanup_list_list)
	    {
	      tree lists;
	      for (lists = f->cleanup_list_list; lists; lists = ((lists)->common.chain) )

		if (((lists)->common.addressable_attr) 
		    && ((lists)->list.value)  != 0)
		  fixup_cleanups (((lists)->list.value) , &f->before_jump);
	    }

	  if (f->stack_level)
	    emit_insn_after (gen_move_insn (stack_pointer_rtx, f->stack_level),
			     f->before_jump);
	  f->before_jump = 0;
	}

      else if (thisblock != 0)
	{
	  tree lists = f->cleanup_list_list;
	  for (; lists; lists = ((lists)->common.chain) )

	    if (((lists)->common.chain)  == thisblock->data.block.outer_cleanups)
	      ((lists)->common.addressable_attr)  = 1;

	  if (stack_level)
	    f->stack_level = stack_level;
	}
    }
}

void
expand_asm (body)
     tree body;
{
  emit_insn (gen_rtx (ASM_INPUT, VOIDmode,
		      ((body)->string.pointer) ));
  last_expr_type = 0;
}

void
expand_asm_operands (string, outputs, inputs, clobbers, vol, filename, line)
     tree string, outputs, inputs, clobbers;
     int vol;
     char *filename;
     int line;
{
  rtvec argvec, constraints;
  rtx body;
  int ninputs = list_length (inputs);
  int noutputs = list_length (outputs);
  int nclobbers = list_length (clobbers);
  tree tail;
  register int i;
  rtx *output_rtx = (rtx *) __builtin_alloca  (noutputs * sizeof (rtx));
  rtx insn;

  last_expr_type = 0;

  for (i = 0, tail = outputs; tail; tail = ((tail)->common.chain) , i++)
    {
      tree val = ((tail)->list.value) ;
      int j;
      int found_equal;

      if (((val)->common.type)  == error_mark_node)
	return;

      found_equal = 0;
      for (j = 0; j < ((((tail)->list.purpose) )->string.length) ; j++)
	{
	  if (((((tail)->list.purpose) )->string.pointer) [j] == '+')
	    {
	      error ("input operand constraint contains `+'");
	      return;
	    }
	  if (((((tail)->list.purpose) )->string.pointer) [j] == '=')
	    found_equal = 1;
	}
      if (! found_equal)
	{
	  error ("output operand constraint lacks `='");
	  return;
	}

      if (((val)->common.code)  != VAR_DECL
	  && ((val)->common.code)  != PARM_DECL
	  && ((val)->common.code)  != INDIRECT_REF)
	{
	  rtx reg = gen_reg_rtx (((((val)->common.type) )->type.mode) );
	  tree t = build_nt (SAVE_EXPR, val, reg);

	  save_expr_regs = gen_rtx (EXPR_LIST, VOIDmode, reg, save_expr_regs);
	  ((tail)->list.value)  = t;
	  ((t)->common.type)  = ((val)->common.type) ;
	}
      output_rtx[i] = expand_expr (((tail)->list.value) , 0, VOIDmode, 0);
    }

  if (ninputs + noutputs > 5 )
    {
      error ("more than %d operands in `asm'", 5 );
      return;
    }

  argvec = rtvec_alloc (ninputs);
  constraints = rtvec_alloc (ninputs);

  body = gen_rtx (ASM_OPERANDS, VOIDmode,
		  ((string)->string.pointer) , "", 0, argvec, constraints,
		  filename, line);
  ((body)->volatil)  = vol;

  i = 0;
  for (tail = inputs; tail; tail = ((tail)->common.chain) )
    {
      int j;

      if (((((tail)->list.value) )->common.type)  == error_mark_node)
	return;
      if (((tail)->list.purpose)  == (tree) 0  )
	{
	  error ("hard register `%s' listed as input operand to `asm'",
		 ((((tail)->list.value) )->string.pointer)  );
	  return;
	}

      for (j = 0; j < ((((tail)->list.purpose) )->string.length) ; j++)
	if (((((tail)->list.purpose) )->string.pointer) [j] == '='
	    || ((((tail)->list.purpose) )->string.pointer) [j] == '+')
	  {
	    error ("input operand constraint contains `%c'",
		   ((((tail)->list.purpose) )->string.pointer) [j]);
	    return;
	  }

      ((body)->fld[ 3].rtvec->elem[ i].rtx)        
	= expand_expr (((tail)->list.value) , 0, VOIDmode, 0);
      ((body)->fld[ 4].rtvec->elem[ i].rtx)        
	= gen_rtx (ASM_INPUT, ((((((tail)->list.value) )->common.type) )->type.mode) ,
		   ((((tail)->list.purpose) )->string.pointer) );
      i++;
    }

  for (i = 0; i < ninputs; i++)
    ((body)->fld[ 3].rtvec->elem[ i].rtx)  = protect_from_queue (((body)->fld[ 3].rtvec->elem[ i].rtx) , 0);

  for (i = 0; i < noutputs; i++)
    output_rtx[i] = protect_from_queue (output_rtx[i], 1);

  if (noutputs == 1 && nclobbers == 0)
    {
      ((body)->fld[ 1].rtstr)  = ((((outputs)->list.purpose) )->string.pointer) ;
      insn = emit_insn (gen_rtx (SET, VOIDmode, output_rtx[0], body));
    }
  else if (noutputs == 0 && nclobbers == 0)
    {
      insn = emit_insn (body);
    }
  else
    {
      rtx obody = body;
      int num = noutputs;
      if (num == 0) num = 1;
      body = gen_rtx (PARALLEL, VOIDmode, rtvec_alloc (num + nclobbers));

      for (i = 0, tail = outputs; tail; tail = ((tail)->common.chain) , i++)
	{
	  ((body)->fld[ 0].rtvec->elem[ i].rtx) 
	    = gen_rtx (SET, VOIDmode,
		       output_rtx[i],
		       gen_rtx (ASM_OPERANDS, VOIDmode,
				((string)->string.pointer) ,
				((((tail)->list.purpose) )->string.pointer) ,
				i, argvec, constraints,
				filename, line));
	  ((((((body)->fld[ 0].rtvec->elem[ i].rtx) )->fld[1].rtx) )->volatil)  = vol;
	}

      if (i == 0)
	((body)->fld[ 0].rtvec->elem[ i++].rtx)  = obody;

      for (tail = clobbers; tail; tail = ((tail)->common.chain) , i++)
	{
	  int j;
	  char *regname = ((((tail)->list.value) )->string.pointer) ;
	  extern char *reg_names[];
	  for (j = 0; j < 56 ; j++)
	    if (!strcmp (regname, reg_names[j]))
	      break;
	  if (j == 56 )
	    {
	      error ("unknown register name `%s' in `asm'", regname);
	      return;
	    }

	  ((body)->fld[ 0].rtvec->elem[ i].rtx) 
	    = gen_rtx (CLOBBER, VOIDmode, gen_rtx (REG, QImode, j));
	}

      insn = emit_insn (body);
    }

  last_expr_type = 0;
}

int expr_stmts_for_value;

void
expand_expr_stmt (exp)
     tree exp;
{

  if (extra_warnings && expr_stmts_for_value == 0 && !((exp)->common.volatile_attr) 
      && exp != error_mark_node)
    warning_with_file_and_line (emit_filename, emit_lineno,
				"statement with no effect");
  last_expr_type = ((exp)->common.type) ;
  if (! flag_syntax_only)
    last_expr_value = expand_expr (exp, expr_stmts_for_value ? 0 : const0_rtx,
				   VOIDmode, 0);
  emit_queue ();
}

void
clear_last_expr ()
{
  last_expr_type = 0;
}

tree
expand_start_stmt_expr ()
{
  rtx save = start_sequence ();

  int momentary = suspend_momentary ();
  tree t = make_node (RTL_EXPR);
  resume_momentary (momentary);
  (*(struct rtx_def **) &(t)->exp.operands[1])  = save;
  expr_stmts_for_value++;
  return t;
}

tree
expand_end_stmt_expr (t)
     tree t;
{
  rtx saved = (*(struct rtx_def **) &(t)->exp.operands[1]) ;

  do_pending_stack_adjust ();

  if (last_expr_type == 0)
    {
      last_expr_type = void_type_node;
      last_expr_value = const0_rtx;
    }
  ((t)->common.type)  = last_expr_type;
  (*(struct rtx_def **) &(t)->exp.operands[1])  = last_expr_value;
  (*(struct rtx_def **) &(t)->exp.operands[0])  = get_insns ();

  rtl_expr_chain = tree_cons ((tree) 0  , t, rtl_expr_chain);

  end_sequence (saved);

  ((t)->common.volatile_attr)  = 1;
  ((t)->common.this_vol_attr)  = volatile_refs_p (last_expr_value);

  last_expr_type = 0;
  expr_stmts_for_value--;

  return t;
}

void
expand_start_cond (cond, exitflag)
     tree cond;
     int exitflag;
{
  struct nesting *thiscond
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  thiscond->next = cond_stack;
  thiscond->all = nesting_stack;
  thiscond->depth = ++nesting_depth;
  thiscond->data.cond.after_label = 0;
  thiscond->data.cond.else_label = gen_label_rtx ();
  thiscond->exit_label = exitflag ? thiscond->data.cond.else_label : 0;
  cond_stack = thiscond;
  nesting_stack = thiscond;

  do_jump (cond, thiscond->data.cond.else_label, 0 );
}

void
expand_end_cond ()
{
  struct nesting *thiscond = cond_stack;

  do_pending_stack_adjust ();
  emit_label (thiscond->data.cond.else_label);

  do { int initial_depth = nesting_stack->depth;	do { struct nesting *this = cond_stack;	cond_stack = this->next;	nesting_stack = this->all;	nesting_depth = this->depth;	free (this); }	while (nesting_depth > initial_depth); } while (0) ;
  last_expr_type = 0;
}

void
expand_start_else ()
{
  cond_stack->data.cond.after_label = gen_label_rtx ();
  if (cond_stack->exit_label != 0)
    cond_stack->exit_label = cond_stack->data.cond.after_label;
  emit_jump (cond_stack->data.cond.after_label);
  if (cond_stack->data.cond.else_label)
    emit_label (cond_stack->data.cond.else_label);
}

void
expand_end_else ()
{
  struct nesting *thiscond = cond_stack;

  do_pending_stack_adjust ();

  if (thiscond->data.cond.after_label)
    emit_label (thiscond->data.cond.after_label);

  do { int initial_depth = nesting_stack->depth;	do { struct nesting *this = cond_stack;	cond_stack = this->next;	nesting_stack = this->all;	nesting_depth = this->depth;	free (this); }	while (nesting_depth > initial_depth); } while (0) ;
  last_expr_type = 0;
}

void
expand_start_loop (exit_flag)
     int exit_flag;
{
  register struct nesting *thisloop
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  thisloop->next = loop_stack;
  thisloop->all = nesting_stack;
  thisloop->depth = ++nesting_depth;
  thisloop->data.loop.start_label = gen_label_rtx ();
  thisloop->data.loop.end_label = gen_label_rtx ();
  thisloop->data.loop.continue_label = thisloop->data.loop.start_label;
  thisloop->exit_label = exit_flag ? thisloop->data.loop.end_label : 0;
  loop_stack = thisloop;
  nesting_stack = thisloop;

  do_pending_stack_adjust ();
  emit_queue ();
  emit_note (0, -4 );
  emit_label (thisloop->data.loop.start_label);
}

void
expand_start_loop_continue_elsewhere (exit_flag)
     int exit_flag;
{
  expand_start_loop (exit_flag);
  loop_stack->data.loop.continue_label = gen_label_rtx ();
}

void
expand_loop_continue_here ()
{
  do_pending_stack_adjust ();
  emit_note (0, -8 );
  emit_label (loop_stack->data.loop.continue_label);
}

void
expand_end_loop ()
{
  register rtx insn = get_last_insn ();
  register rtx start_label = loop_stack->data.loop.start_label;

  do_pending_stack_adjust ();

  if (optimize
      &&
      ! (	((insn)->code)  == JUMP_INSN
	 && 	((((insn)->fld[3].rtx) )->code)  == SET
	 && ((((insn)->fld[3].rtx) )->fld[0].rtx)  == pc_rtx
	 && 	((((((insn)->fld[3].rtx) )->fld[1].rtx) )->code)  == IF_THEN_ELSE))
    {

      for (insn = loop_stack->data.loop.start_label; insn; insn= ((insn)->fld[2].rtx) )
	if (	((insn)->code)  == JUMP_INSN && 	((((insn)->fld[3].rtx) )->code)  == SET
	    && ((((insn)->fld[3].rtx) )->fld[0].rtx)  == pc_rtx
	    && 	((((((insn)->fld[3].rtx) )->fld[1].rtx) )->code)  == IF_THEN_ELSE
	    &&
	    ((	((((((((insn)->fld[3].rtx) )->fld[1].rtx) )->fld[ 1].rtx) )->code)  == LABEL_REF
	      && (((((((((insn)->fld[3].rtx) )->fld[1].rtx) )->fld[ 1].rtx) )->fld[ 0].rtx) 
		  == loop_stack->data.loop.end_label))
	     ||
	     (	((((((((insn)->fld[3].rtx) )->fld[1].rtx) )->fld[ 2].rtx) )->code)  == LABEL_REF
	      && (((((((((insn)->fld[3].rtx) )->fld[1].rtx) )->fld[ 2].rtx) )->fld[ 0].rtx) 
		  == loop_stack->data.loop.end_label))))
	  break;
      if (insn != 0)
	{

	  register rtx newstart_label = gen_label_rtx ();

	  emit_label_after (newstart_label, ((start_label)->fld[1].rtx) );
	  reorder_insns (start_label, insn, get_last_insn ());
	  emit_jump_insn_after (gen_jump (start_label), ((newstart_label)->fld[1].rtx) );
	  emit_barrier_after (((newstart_label)->fld[1].rtx) );
	  start_label = newstart_label;
	}
    }

  emit_jump (start_label);
  emit_note (0, -5 );
  emit_label (loop_stack->data.loop.end_label);

  do { int initial_depth = nesting_stack->depth;	do { struct nesting *this = loop_stack;	loop_stack = this->next;	nesting_stack = this->all;	nesting_depth = this->depth;	free (this); }	while (nesting_depth > initial_depth); } while (0) ;

  last_expr_type = 0;
}

int
expand_continue_loop ()
{
  last_expr_type = 0;
  if (loop_stack == 0)
    return 0;
  expand_goto_internal (0, loop_stack->data.loop.continue_label, 0);
  return 1;
}

int
expand_exit_loop ()
{
  last_expr_type = 0;
  if (loop_stack == 0)
    return 0;
  expand_goto_internal (0, loop_stack->data.loop.end_label, 0);
  return 1;
}

int
expand_exit_loop_if_false (cond)
     tree cond;
{
  last_expr_type = 0;
  if (loop_stack == 0)
    return 0;
  do_jump (cond, loop_stack->data.loop.end_label, 0 );
  return 1;
}

int
expand_exit_something ()
{
  struct nesting *n;
  last_expr_type = 0;
  for (n = nesting_stack; n; n = n->all)
    if (n->exit_label != 0)
      {
	expand_goto_internal (0, n->exit_label, 0);
	return 1;
      }

  return 0;
}

void
expand_null_return ()
{
  expand_null_return_1 (0);
}

static void
expand_null_return_1 (last_insn)
     rtx last_insn;
{
  clear_pending_stack_adjust ();
  do_pending_stack_adjust ();
  last_expr_type = 0;

  if (current_function_returns_pcc_struct)
    {
      expand_goto_internal (0, return_label, last_insn);
      return;
    }

  if ((0) )
    {
      emit_jump_insn (gen_return ());
      emit_barrier ();
      return;
    }

  expand_goto_internal (0, return_label, last_insn);
}

void
expand_return (retval)
     tree retval;
{
  register rtx val = 0;
  register rtx op0;
  tree retval_rhs;
  int cleanups;
  struct nesting *block;

  cleanups = 0;
  for (block = block_stack; block; block = block->next)
    if (block->data.block.cleanups != 0)
      {
	cleanups = 1;
	break;
      }

  if (((retval)->common.code)  == RESULT_DECL)
    retval_rhs = retval;
  else if ((((retval)->common.code)  == MODIFY_EXPR || ((retval)->common.code)  == INIT_EXPR)
	   && ((((retval)->exp.operands[ 0]) )->common.code)  == RESULT_DECL)
    retval_rhs = ((retval)->exp.operands[ 1]) ;
  else if (((retval)->common.type)  == void_type_node)
    retval_rhs = retval;
  else
    retval_rhs = (tree) 0  ;

  if (optimize && retval_rhs != 0
      && frame_offset == 0 
      && ((retval_rhs)->common.code)  == CALL_EXPR
      && ((((retval_rhs)->exp.operands[ 0]) )->common.code)  == ADDR_EXPR
      && ((((retval_rhs)->exp.operands[ 0]) )->exp.operands[ 0])  == this_function

      && tail_recursion_args (((retval_rhs)->exp.operands[ 1]) ,
			      ((this_function)->decl.arguments)  ))
    {
      if (tail_recursion_label == 0)
	{
	  tail_recursion_label = gen_label_rtx ();
	  emit_label_after (tail_recursion_label,
			    tail_recursion_reentry);
	}
      expand_goto_internal (0, tail_recursion_label, get_last_insn ());
      emit_barrier ();
      return;
    }

  if ((0)  && ! cleanups
      && ! current_function_returns_pcc_struct)
    {

      if (retval_rhs)
	switch (((retval_rhs)->common.code) )
	  {
	  case EQ_EXPR:
	  case NE_EXPR:
	  case GT_EXPR:
	  case GE_EXPR:
	  case LT_EXPR:
	  case LE_EXPR:
	  case TRUTH_ANDIF_EXPR:
	  case TRUTH_ORIF_EXPR:
	  case TRUTH_AND_EXPR:
	  case TRUTH_OR_EXPR:
	  case TRUTH_NOT_EXPR:
	    op0 = gen_label_rtx ();
	    val = ((((this_function)->decl.result) )->decl.rtl) ;
	    jumpifnot (retval_rhs, op0);
	    emit_move_insn (val, const1_rtx);
	    emit_insn (gen_rtx (USE, VOIDmode, val));
	    expand_null_return ();
	    emit_label (op0);
	    emit_move_insn (val, const0_rtx);
	    emit_insn (gen_rtx (USE, VOIDmode, val));
	    expand_null_return ();
	    return;
	  }
    }

  if (cleanups
      && retval_rhs != 0
      && ((retval_rhs)->common.type)  != void_type_node
      && 	((((((this_function)->decl.result) )->decl.rtl) )->code)  == REG)
    {
      rtx last_insn;
      val = expand_expr (retval_rhs, 0, VOIDmode, 0);
      emit_queue ();
      last_insn = get_last_insn ();
      emit_move_insn (((((this_function)->decl.result) )->decl.rtl) , val);
      val = ((((this_function)->decl.result) )->decl.rtl) ;

      if (	((val)->code)  == REG)
	emit_insn (gen_rtx (USE, VOIDmode, val));
      expand_null_return_1 (last_insn);
    }
  else
    {

      val = expand_expr (retval, 0, VOIDmode, 0);
      emit_queue ();

      val = ((((this_function)->decl.result) )->decl.rtl) ;
      if (	((val)->code)  == REG)
	emit_insn (gen_rtx (USE, VOIDmode, val));
      expand_null_return ();
    }
}

int
drop_through_at_end_p ()
{
  rtx insn = get_last_insn ();
  while (insn && 	((insn)->code)  == NOTE)
    insn = ((insn)->fld[1].rtx) ;
  return insn && 	((insn)->code)  != BARRIER;
}

static int
tail_recursion_args (actuals, formals)
     tree actuals, formals;
{
  register tree a = actuals, f = formals;
  register int i;
  register rtx *argvec;

  for (a = actuals, f = formals, i = 0; a && f; a = ((a)->common.chain) , f = ((f)->common.chain) , i++)
    {
      if (((((a)->list.value) )->common.type)  != ((f)->common.type) )
	return 0;
      if (	((((f)->decl.rtl) )->code)  != REG || ((f)->decl.mode)  == BLKmode)
	return 0;
    }
  if (a != 0 || f != 0)
    return 0;

  argvec = (rtx *) __builtin_alloca  (i * sizeof (rtx));

  for (a = actuals, i = 0; a; a = ((a)->common.chain) , i++)
    argvec[i] = expand_expr (((a)->list.value) , 0, VOIDmode, 0);

  for (a = actuals, i = 0; a; a = ((a)->common.chain) , i++)
    {
      int copy = 0;
      register int j;
      for (f = formals, j = 0; j < i; f = ((f)->common.chain) , j++)
	if (reg_mentioned_p (((f)->decl.rtl) , argvec[i]))
	  { copy = 1; break; }
      if (copy)
	argvec[i] = copy_to_reg (argvec[i]);
    }

  for (f = formals, a = actuals, i = 0; f;
       f = ((f)->common.chain) , a = ((a)->common.chain) , i++)
    {
      if (((f)->decl.mode)  == 	((argvec[i])->mode) )
	emit_move_insn (((f)->decl.rtl) , argvec[i]);
      else
	convert_move (((f)->decl.rtl) , argvec[i],
		      ((((((a)->list.value) )->common.type) )->common.unsigned_attr) );
    }

  return 1;
}

void
expand_start_bindings (exit_flag)
     int exit_flag;
{
  struct nesting *thisblock
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  rtx note = emit_note (0, -2 );

  thisblock->next = block_stack;
  thisblock->all = nesting_stack;
  thisblock->depth = ++nesting_depth;
  thisblock->data.block.stack_level = 0;
  thisblock->data.block.cleanups = 0;

  thisblock->data.block.outer_cleanups
    = (block_stack
       ? tree_cons ((tree) 0  , block_stack->data.block.cleanups,
		    block_stack->data.block.outer_cleanups)
       : 0);
  thisblock->data.block.label_chain = 0;
  thisblock->data.block.innermost_stack_block = stack_block_stack;
  thisblock->data.block.first_insn = note;
  thisblock->exit_label = exit_flag ? gen_label_rtx () : 0;
  block_stack = thisblock;
  nesting_stack = thisblock;
}

void
use_variable (rtl)
     rtx rtl;
{
  if (	((rtl)->code)  == REG)
    emit_insn (gen_rtx (USE, VOIDmode, rtl));
  else if (	((rtl)->code)  == MEM
	   && 	((((rtl)->fld[ 0].rtx) )->code)  == REG
	   && ((rtl)->fld[ 0].rtx)  != frame_pointer_rtx
	   && ((rtl)->fld[ 0].rtx)  != arg_pointer_rtx)
    emit_insn (gen_rtx (USE, VOIDmode, ((rtl)->fld[ 0].rtx) ));
}

static void
use_variable_after (rtl, insn)
     rtx rtl, insn;
{
  if (	((rtl)->code)  == REG)
    emit_insn_after (gen_rtx (USE, VOIDmode, rtl), insn);
  else if (	((rtl)->code)  == MEM
	   && 	((((rtl)->fld[ 0].rtx) )->code)  == REG
	   && ((rtl)->fld[ 0].rtx)  != frame_pointer_rtx
	   && ((rtl)->fld[ 0].rtx)  != arg_pointer_rtx)
    emit_insn_after (gen_rtx (USE, VOIDmode, ((rtl)->fld[ 0].rtx) ), insn);
}

void
expand_end_bindings (vars, mark_ends, dont_jump_in)
     tree vars;
     int mark_ends;
     int dont_jump_in;
{
  register struct nesting *thisblock = block_stack;
  register tree decl;

  if (warn_unused)
    for (decl = vars; decl; decl = ((decl)->common.chain) )
      if (! ((decl)->common.used_attr)  && ((decl)->common.code)  == VAR_DECL)
	warning_with_decl (decl, "unused variable `%s'");

  if (mark_ends)
    emit_note (0, -3 );
  else
    ((thisblock->data.block.first_insn)->fld[4].rtint)  = -1 ;

  if (thisblock->exit_label)
    {
      do_pending_stack_adjust ();
      emit_label (thisblock->exit_label);
    }

  if (dont_jump_in
      || thisblock->data.block.stack_level != 0
      || thisblock->data.block.cleanups != 0)
    {
      struct label_chain *chain;

      for (chain = thisblock->data.block.label_chain; chain; chain = chain->next)
	{
	  ((chain->label)->common.packed_attr)  = 1;

	  if (((chain->label)->common.addressable_attr) )
	    error_with_decl (chain->label,
			     "label `%s' used before containing binding contour");
	}
    }

  if (thisblock->data.block.stack_level != 0
      || thisblock->data.block.cleanups != 0)
    {

      expand_cleanups (thisblock->data.block.cleanups, 0);

      if (thisblock->data.block.stack_level != 0)
	{
	  do_pending_stack_adjust ();
	  emit_move_insn (stack_pointer_rtx,
			  thisblock->data.block.stack_level);
	}

      fixup_gotos (thisblock,
		   thisblock->data.block.stack_level,
		   thisblock->data.block.cleanups,
		   thisblock->data.block.first_insn,
		   dont_jump_in);
    }

  if (obey_regdecls)
    for (decl = vars; decl; decl = ((decl)->common.chain) )
      {
	rtx rtl = ((decl)->decl.rtl) ;
	if (((decl)->common.code)  == VAR_DECL && rtl != 0)
	  use_variable (rtl);
      }

  stack_block_stack = thisblock->data.block.innermost_stack_block;
  do { int initial_depth = nesting_stack->depth;	do { struct nesting *this = block_stack;	block_stack = this->next;	nesting_stack = this->all;	nesting_depth = this->depth;	free (this); }	while (nesting_depth > initial_depth); } while (0) ;
}

void
expand_decl (decl, cleanup)
     register tree decl;
     tree cleanup;
{
  struct nesting *thisblock = block_stack;
  tree type;

  if (cleanup != 0)
    {
      thisblock->data.block.cleanups
	= temp_tree_cons (decl, cleanup, thisblock->data.block.cleanups);
      stack_block_stack = thisblock;
    }

  if (decl == (tree) 0  )
    {
      if (cleanup == 0)
	abort ();
      return;
    }

  type = ((decl)->common.type) ;

  if (((decl)->common.code)  != VAR_DECL)
    return;
  if (((decl)->common.static_attr)  || ((decl)->common.external_attr) )
    return;

  if (type == error_mark_node)
    ((decl)->decl.rtl)  = gen_rtx (MEM, BLKmode, const0_rtx);
  else if (((decl)->decl.size)  == 0)
    {
      if (((decl)->decl.initial)  == 0)
	((decl)->decl.rtl)  = assign_stack_local (((decl)->decl.mode) , 0);
      else

	((decl)->decl.rtl)  = gen_rtx (MEM, BLKmode, gen_reg_rtx (SImode ));
    }
  else if (((decl)->decl.mode)  != BLKmode

	   && !(flag_float_store
		&& ((type)->common.code)  == REAL_TYPE)
	   && ! ((decl)->common.volatile_attr) 
	   && ! ((decl)->common.addressable_attr) 
	   && (((decl)->common.regdecl_attr)  || ! obey_regdecls))
    {
      ((decl)->decl.rtl)  = gen_reg_rtx (((decl)->decl.mode) );
      if (((type)->common.code)  == POINTER_TYPE)
	mark_reg_pointer (((decl)->decl.rtl) );
      ((((decl)->decl.rtl) )->volatil)  = 1;
    }
  else if (((((decl)->decl.size) )->common.literal_attr) )
    {
      rtx oldaddr = 0;
      rtx addr;

      if (((decl)->decl.rtl)  != 0)
	{
	  if (	((((decl)->decl.rtl) )->code)  != MEM
	      || 	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  != REG)
	    abort ();
	  oldaddr = ((((decl)->decl.rtl) )->fld[ 0].rtx) ;
	}

      ((decl)->decl.rtl) 
	= assign_stack_local (((decl)->decl.mode) ,
			      (((((decl)->decl.size) )->int_cst.int_cst_low) 
			       * ((decl)->decl.size_unit) 
			       + 8  - 1)
			      / 8 );
      if (oldaddr)
	{
	  addr = force_operand (((((decl)->decl.rtl) )->fld[ 0].rtx) , oldaddr);
	  emit_move_insn (oldaddr, addr);
	}

      ((((decl)->decl.rtl) )->in_struct) 
	= (((((decl)->common.type) )->common.code)  == ARRAY_TYPE
	   || ((((decl)->common.type) )->common.code)  == RECORD_TYPE
	   || ((((decl)->common.type) )->common.code)  == UNION_TYPE);

    }
  else
    {
      rtx address, size;

      frame_pointer_needed = 1;

      if (thisblock->data.block.stack_level == 0)
	{
	  do_pending_stack_adjust ();
	  thisblock->data.block.stack_level
	    = copy_to_reg (stack_pointer_rtx);
	  stack_block_stack = thisblock;
	}

      size = expand_expr (convert_units (((decl)->decl.size) ,
					 ((decl)->decl.size_unit) ,
					 8 ),
			  0, VOIDmode, 0);

      if (((decl)->decl.size_unit)  % 16 )
	{

	  size = round_push (size);
	}

      anti_adjust_stack (size);

      address = copy_to_reg (stack_pointer_rtx);

      ((decl)->decl.rtl)  = gen_rtx (MEM, ((decl)->decl.mode) , address);
    }

  if (((decl)->common.volatile_attr) )
    ((((decl)->decl.rtl) )->volatil)  = 1;
  if (((decl)->common.readonly_attr) )
    ((((decl)->decl.rtl) )->unchanging)  = 1;

  if (obey_regdecls)
    use_variable (((decl)->decl.rtl) );
}

void
expand_decl_init (decl)
     tree decl;
{
  if (((decl)->common.static_attr) )
    return;

  if (((decl)->decl.initial)  == error_mark_node)
    {
      enum tree_code code = ((((decl)->common.type) )->common.code) ;
      if (code == INTEGER_TYPE || code == REAL_TYPE || code == ENUMERAL_TYPE
	  || code == POINTER_TYPE)
	expand_assignment (decl, convert (((decl)->common.type) , integer_zero_node),
			   0, 0);
      emit_queue ();
    }
  else if (((decl)->decl.initial)  && ((((decl)->decl.initial) )->common.code)  != TREE_LIST)
    {
      emit_line_note (((decl)->decl.filename) , ((decl)->decl.linenum) );
      expand_assignment (decl, ((decl)->decl.initial) , 0, 0);
      emit_queue ();
    }
}

void
expand_anon_union_decl (decl, cleanup, decl_elts)
     tree decl, cleanup, decl_elts;
{
  struct nesting *thisblock = block_stack;
  rtx x;

  expand_decl (decl, cleanup);
  x = ((decl)->decl.rtl) ;

  while (decl_elts)
    {
      tree decl_elt = ((decl_elts)->list.value) ;
      tree cleanup_elt = ((decl_elts)->list.purpose) ;

      ((decl_elt)->decl.rtl) 
	= (	((x)->mode)  != BLKmode

	   ? gen_rtx (SUBREG, ((((decl_elt)->common.type) )->type.mode) , x, 0)
	   : x);

      if (cleanup != 0)
	thisblock->data.block.cleanups
	  = temp_tree_cons (decl_elt, cleanup_elt,
			    thisblock->data.block.cleanups);

      decl_elts = ((decl_elts)->common.chain) ;
    }
}

static void
expand_cleanups (list, dont_do)
     tree list;
     tree dont_do;
{
  tree tail;
  for (tail = list; tail; tail = ((tail)->common.chain) )
    if (dont_do == 0 || ((tail)->list.purpose)  != dont_do)
      {
	if (((((tail)->list.value) )->common.code)  == TREE_LIST)
	  expand_cleanups (((tail)->list.value) , dont_do);
	else
	  expand_expr (((tail)->list.value) , const0_rtx, VOIDmode, 0);
      }
}

static void
fixup_cleanups (list, before_jump)
     tree list;
     rtx *before_jump;
{
  rtx beyond_jump = get_last_insn ();
  rtx new_before_jump;

  expand_cleanups (list, 0);
  new_before_jump = get_last_insn ();

  reorder_insns (((beyond_jump)->fld[2].rtx) , new_before_jump, *before_jump);
  *before_jump = new_before_jump;
}

void
move_cleanups_up ()
{
  struct nesting *block = block_stack;
  struct nesting *outer = block->next;

  outer->data.block.cleanups
    = chainon (block->data.block.cleanups,
	       outer->data.block.cleanups);
  block->data.block.cleanups = 0;
}

void
expand_start_case (exit_flag, expr, type)
     int exit_flag;
     tree expr;
     tree type;
{
  register struct nesting *thiscase
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  thiscase->next = case_stack;
  thiscase->all = nesting_stack;
  thiscase->depth = ++nesting_depth;
  thiscase->exit_label = exit_flag ? gen_label_rtx () : 0;
  thiscase->data.case_stmt.case_list = 0;
  thiscase->data.case_stmt.index_expr = expr;
  thiscase->data.case_stmt.nominal_type = type;
  thiscase->data.case_stmt.default_label = 0;
  thiscase->data.case_stmt.num_ranges = 0;
  case_stack = thiscase;
  nesting_stack = thiscase;

  do_pending_stack_adjust ();

  if (	((get_last_insn ())->code)  != NOTE)
    emit_note (0, -1 );

  thiscase->data.case_stmt.start = get_last_insn ();
}

void
expand_start_case_dummy ()
{
  register struct nesting *thiscase
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  thiscase->next = case_stack;
  thiscase->all = nesting_stack;
  thiscase->depth = ++nesting_depth;
  thiscase->exit_label = 0;
  thiscase->data.case_stmt.case_list = 0;
  thiscase->data.case_stmt.start = 0;
  thiscase->data.case_stmt.nominal_type = 0;
  thiscase->data.case_stmt.default_label = 0;
  thiscase->data.case_stmt.num_ranges = 0;
  case_stack = thiscase;
  nesting_stack = thiscase;
}

void
expand_end_case_dummy ()
{
  do { int initial_depth = nesting_stack->depth;	do { struct nesting *this = case_stack;	case_stack = this->next;	nesting_stack = this->all;	nesting_depth = this->depth;	free (this); }	while (nesting_depth > initial_depth); } while (0) ;
}

int
pushcase (value, label)
     register tree value;
     register tree label;
{
  register struct case_node **l;
  register struct case_node *n;
  tree index_type;
  tree nominal_type;

  if (! (case_stack && case_stack->data.case_stmt.start))
    return 1;

  index_type = ((case_stack->data.case_stmt.index_expr)->common.type) ;
  nominal_type = case_stack->data.case_stmt.nominal_type;

  if (index_type == error_mark_node)
    return 0;

  if (value != 0)
    value = convert (nominal_type, value);

  if (value != 0 && ! int_fits_type_p (value, index_type))
    return 3;

  if (value == 0)
    {
      if (case_stack->data.case_stmt.default_label != 0)
	return 2;
      case_stack->data.case_stmt.default_label = label;
    }
  else
    {

      for (l = &case_stack->data.case_stmt.case_list;
	   *l != 0 && tree_int_cst_lt ((*l)->high, value);
	   l = &(*l)->right)
	;
      if (*l)
	{

	  if (! tree_int_cst_lt (value, (*l)->low))
	    return 2;
	}

      n = (struct case_node *) oballoc (sizeof (struct case_node));
      n->left = 0;
      n->right = *l;
      n->high = n->low = copy_node (value);
      n->code_label = label;
      n->test_label = 0;
      *l = n;
    }

  expand_label (label);
  return 0;
}

int
pushcase_range (value1, value2, label)
     register tree value1, value2;
     register tree label;
{
  register struct case_node **l;
  register struct case_node *n;
  tree index_type;
  tree nominal_type;

  if (! (case_stack && case_stack->data.case_stmt.start))
    return 1;

  index_type = ((case_stack->data.case_stmt.index_expr)->common.type) ;
  nominal_type = case_stack->data.case_stmt.nominal_type;

  if (index_type == error_mark_node)
    return 0;

  if (value1 != 0)
    value1 = convert (nominal_type, value1);
  if (value2 != 0)
    value2 = convert (nominal_type, value2);

  if (value1 != 0 && ! int_fits_type_p (value1, index_type))
    return 3;

  if (value2 != 0 && ! int_fits_type_p (value2, index_type))
    return 3;

  if (tree_int_cst_lt (value2, value1))
    return 4;

  if (tree_int_cst_equal (value1, value2))
    return pushcase (value1, label);

  for (l = &case_stack->data.case_stmt.case_list;
       *l != 0 && tree_int_cst_lt ((*l)->high, value1);
       l = &(*l)->right)
    ;
  if (*l)
    {

      if (! tree_int_cst_lt (value2, (*l)->low))
	return 2;
    }

  n = (struct case_node *) oballoc (sizeof (struct case_node));
  n->left = 0;
  n->right = *l;
  n->low = copy_node (value1);
  n->high = copy_node (value2);
  n->code_label = label;
  n->test_label = 0;
  *l = n;

  expand_label (label);

  case_stack->data.case_stmt.num_ranges++;

  return 0;
}

void
check_for_full_enumeration_handling ()
{
  tree index_expr = case_stack->data.case_stmt.index_expr;

  if (((index_expr)->common.code)  == INTEGER_CST)
    return;
  else
    {
      register struct case_node *n;
      register tree chain;
      tree enum_node = ((index_expr)->exp.operands[ 0]) ;

      for (chain = ((((enum_node)->common.type) )->type.values) ;
           chain; 
           chain = ((chain)->common.chain) )
        {

          for (n = case_stack->data.case_stmt.case_list; 
               n && tree_int_cst_lt (n->high, ((chain)->list.value) );
               n = n->right)
            ;

          if (!(n && tree_int_cst_equal (n->low, ((chain)->list.value) )))
	    warning ("enumerated value `%s' not handled in switch",
		     ((((chain)->list.purpose) )->identifier.pointer) );
        }

      for (n = case_stack->data.case_stmt.case_list; n; n = n->right)
        {
          for (chain = (( ((enum_node)->common.type) )->type.values) ;
               chain && !tree_int_cst_equal (n->low, ((chain)->list.value) ); 
               chain = ((chain)->common.chain) )
            ;

          if (!chain)
	    warning ("case value `%d' not in enumerated type `%s'",
		     ((n->low)->int_cst.int_cst_low) , 
		     ((((((((enum_node)->common.type) )->type.name) )->decl.name) )->identifier.pointer) );
        }
    }
}

void
expand_end_case (orig_index)
     tree orig_index;
{
  tree minval, maxval, range;
  rtx default_label = 0;
  register struct case_node *n;
  int count;
  rtx index;
  rtx table_label = gen_label_rtx ();
  int ncases;
  rtx *labelvec;
  register int i;
  rtx before_case;
  register struct nesting *thiscase = case_stack;
  tree index_expr = thiscase->data.case_stmt.index_expr;

  do_pending_stack_adjust ();

  if (((index_expr)->common.type)  != error_mark_node)
    {

      if (!thiscase->data.case_stmt.default_label 
	  && ((((orig_index)->common.type) )->common.code)  == ENUMERAL_TYPE
	  && warn_switch)
	check_for_full_enumeration_handling ();

      if (thiscase->data.case_stmt.default_label == 0)
	{
	  thiscase->data.case_stmt.default_label
	    = build_decl (LABEL_DECL, (tree) 0  , (tree) 0  );
	  expand_label (thiscase->data.case_stmt.default_label);
	}
      default_label = label_rtx (thiscase->data.case_stmt.default_label);

      before_case = get_last_insn ();

      group_case_nodes (thiscase->data.case_stmt.case_list);

      count = 0;
      for (n = thiscase->data.case_stmt.case_list; n; n = n->right)
	{
	  if (((n->low)->common.code)  != INTEGER_CST)
	    abort ();
	  if (((n->high)->common.code)  != INTEGER_CST)
	    abort ();

	  n->low = convert (((index_expr)->common.type) , n->low);
	  n->high = convert (((index_expr)->common.type) , n->high);

	  if (count++ == 0)
	    {
	      minval = n->low;
	      maxval = n->high;
	    }
	  else
	    {
	      if ((((n->low)->int_cst.int_cst_high)  < (( minval)->int_cst.int_cst_high) 	|| (((n->low)->int_cst.int_cst_high)  == (( minval)->int_cst.int_cst_high) 	&& ((unsigned) ((n->low)->int_cst.int_cst_low)  < (unsigned) (( minval)->int_cst.int_cst_low) ))) )
		minval = n->low;
	      if ((((maxval)->int_cst.int_cst_high)  < (( n->high)->int_cst.int_cst_high) 	|| (((maxval)->int_cst.int_cst_high)  == (( n->high)->int_cst.int_cst_high) 	&& ((unsigned) ((maxval)->int_cst.int_cst_low)  < (unsigned) (( n->high)->int_cst.int_cst_low) ))) )
		maxval = n->high;
	    }
	  if (! tree_int_cst_equal (n->low, n->high))
	    count++;
	}

      if (count != 0)
	range = combine (MINUS_EXPR, maxval, minval);

      if (count == 0 || ((((index_expr)->common.type) )->common.code)  == ERROR_MARK)
	{
	  expand_expr (index_expr, const0_rtx, VOIDmode, 0);
	  emit_queue ();
	  emit_jump (default_label);
	}

      else if (((range)->int_cst.int_cst_high)  != 0

	       || count < 4

	       || (unsigned) (((range)->int_cst.int_cst_low) ) > 10 * count
	       || ((index_expr)->common.code)  == INTEGER_CST)
	{
	  index = expand_expr (index_expr, 0, VOIDmode, 0);
	  emit_queue ();
	  do_pending_stack_adjust ();

	  index = protect_from_queue (index, 0);
	  if (	((index)->code)  == MEM)
	    index = copy_to_reg (index);
	  if (	((index)->code)  == CONST_INT
	      || ((index_expr)->common.code)  == INTEGER_CST)
	    {

	      if (((index_expr)->common.code)  != INTEGER_CST)
		{
		  index_expr = build_int_2 (((index)->fld[0].rtint) , 0);
		  index_expr = convert (((index_expr)->common.type) , index_expr);
		}

	      for (n = thiscase->data.case_stmt.case_list;
		   n;
		   n = n->right)
		{
		  if (! tree_int_cst_lt (index_expr, n->low)
		      && ! tree_int_cst_lt (n->high, index_expr))
		    break;
		}
	      if (n)
		emit_jump (label_rtx (n->code_label));
	      else
		emit_jump (default_label);
	    }
	  else
	    {

	      balance_case_nodes (&thiscase->data.case_stmt.case_list, 0);
	      emit_case_nodes (index, thiscase->data.case_stmt.case_list,
			       default_label,
			       ((((index_expr)->common.type) )->common.unsigned_attr) );
	      emit_jump_if_reachable (default_label);
	    }
	}
      else
	{

	  if (((((index_expr)->common.type) )->type.mode)  == DImode)
	    {
	      index_expr = build (MINUS_EXPR, ((index_expr)->common.type) ,
				  index_expr, minval);
	      minval = integer_zero_node;
	    }
	  if (((((index_expr)->common.type) )->type.mode)  != SImode)
	    index_expr = convert (type_for_size ( (8  * mode_size[(int)(SImode)]) , 0),
				  index_expr);
	  index = expand_expr (index_expr, 0, VOIDmode, 0);
	  emit_queue ();
	  index = protect_from_queue (index, 0);
	  do_pending_stack_adjust ();

	  emit_jump_insn (gen_casesi (index, expand_expr (minval, 0, VOIDmode, 0),
				      expand_expr (range, 0, VOIDmode, 0),
				      table_label, default_label));

	  ncases = ((range)->int_cst.int_cst_low)  + 1;
	  labelvec = (rtx *) __builtin_alloca  (ncases * sizeof (rtx));
	  memset (labelvec,0, ncases * sizeof (rtx)) ;

	  for (n = thiscase->data.case_stmt.case_list; n; n = n->right)
	    {
	      register int i
		= ((n->low)->int_cst.int_cst_low)  - ((minval)->int_cst.int_cst_low) ;

	      while (i + ((minval)->int_cst.int_cst_low) 
		     <= ((n->high)->int_cst.int_cst_low) )
		labelvec[i++]
		  = gen_rtx (LABEL_REF, SImode , label_rtx (n->code_label));
	    }

	  for (i = 0; i < ncases; i++)
	    if (labelvec[i] == 0)
	      labelvec[i] = gen_rtx (LABEL_REF, SImode , default_label);

	  emit_label (table_label);

	  emit_jump_insn (gen_rtx (ADDR_DIFF_VEC, HImode ,
				   gen_rtx (LABEL_REF, SImode , table_label),
				   gen_rtvec_v (ncases, labelvec)));

	  emit_barrier ();

	}

      reorder_insns (((before_case)->fld[2].rtx) , get_last_insn (),
		     thiscase->data.case_stmt.start);
    }
  if (thiscase->exit_label)
    emit_label (thiscase->exit_label);

  do { int initial_depth = nesting_stack->depth;	do { struct nesting *this = case_stack;	case_stack = this->next;	nesting_stack = this->all;	nesting_depth = this->depth;	free (this); }	while (nesting_depth > initial_depth); } while (0) ;
}

static void
do_jump_if_equal (op1, op2, label, unsignedp)
     rtx op1, op2, label;
     int unsignedp;
{
  if (	((op1)->code)  == CONST_INT
      && 	((op2)->code)  == CONST_INT)
    {
      if (((op1)->fld[0].rtint)  == ((op2)->fld[0].rtint) )
	emit_jump (label);
    }
  else
    {
      emit_cmp_insn (op1, op2, 0, unsignedp, 0);
      emit_jump_insn (gen_beq (label));
    }
}

static void
group_case_nodes (head)
     case_node_ptr head;
{
  case_node_ptr node = head;

  while (node)
    {
      rtx lb = next_real_insn (label_rtx (node->code_label));
      case_node_ptr np = node;

      while (((np = np->right) != 0)
	     && next_real_insn (label_rtx (np->code_label)) == lb
	     && tree_int_cst_equal (np->low,
				    combine (PLUS_EXPR, node->high,
					     build_int_2 (1, 0))))
	{
	  node->high = np->high;
	}

      node->right = np;
      node = np;
    }
}

static void
balance_case_nodes (head, parent)
     case_node_ptr *head;
     case_node_ptr parent;
{
  register case_node_ptr np;

  np = *head;
  if (np)
    {
      int i = 0;
      int ranges = 0;
      register case_node_ptr *npp;
      case_node_ptr left;

      while (np)
	{
	  if (!tree_int_cst_equal (np->low, np->high))
	    ranges++;
	  i++;
	  np = np->right;
	}
      if (i > 2)
	{
	  npp = head;
	  left = *npp;
	  if (i == 3)
	    npp = &(*npp)->right;
	  else
	    {

	      i = (i + ranges + 1) / 2;
	      while (1)
		{
		  if (!tree_int_cst_equal ((*npp)->low, (*npp)->high))
		    i--;
		  i--;
		  if (i <= 0)
		    break;
		  npp = &(*npp)->right;
		}
	    }
	  *head = np = *npp;
	  *npp = 0;
	  np->parent = parent;
	  np->left = left;

	  balance_case_nodes (&np->left, np);
	  balance_case_nodes (&np->right, np);
	}
      else
	{

	  np = *head;
	  np->parent = parent;
	  for (; np->right; np = np->right)
	    np->right->parent = np;
	}
    }
}

static int
node_has_low_bound (node)
     case_node_ptr node;
{
  tree low_minus_one;
  case_node_ptr pnode;

  if (node->left)
    {
      low_minus_one = combine (MINUS_EXPR, node->low, build_int_2 (1, 0));
      if (tree_int_cst_lt (low_minus_one, node->low))
	for (pnode = node->parent; pnode; pnode = pnode->parent)
	  {
	    if (tree_int_cst_equal (low_minus_one, pnode->high))
	      return 1;

	    if (node->left)
	      break;
	  }
    }
  return 0;
}

static int
node_has_high_bound (node)
     case_node_ptr node;
{
  tree high_plus_one;
  case_node_ptr pnode;

  if (node->right == 0)
    {
      high_plus_one = combine (PLUS_EXPR, node->high, build_int_2 (1, 0));
      if (tree_int_cst_lt (node->high, high_plus_one))
	for (pnode = node->parent; pnode; pnode = pnode->parent)
	  {
	    if (tree_int_cst_equal (high_plus_one, pnode->low))
	      return 1;

	    if (node->right)
	      break;
	  }
    }
  return 0;
}

static int
node_is_bounded (node)
     case_node_ptr node;
{
  if (node->left || node->right)
    return 0;
  return node_has_low_bound (node) && node_has_high_bound (node);
}

static void
emit_jump_if_reachable (label)
     rtx label;
{
  rtx last_insn;

  if (	((get_last_insn ())->code)  != BARRIER)
    emit_jump (label);
}

static void
emit_case_nodes (index, node, default_label, unsignedp)
     rtx index;
     case_node_ptr node;
     rtx default_label;
     int unsignedp;
{
  typedef rtx rtx_function ();
  rtx_function *gen_bgt_pat = unsignedp ? gen_bgtu : gen_bgt;
  rtx_function *gen_bge_pat = unsignedp ? gen_bgeu : gen_bge;
  rtx_function *gen_blt_pat = unsignedp ? gen_bltu : gen_blt;
  rtx_function *gen_ble_pat = unsignedp ? gen_bleu : gen_ble;

  if (node->test_label)
    {

      emit_jump_if_reachable (default_label);
      expand_label (node->test_label);
    }
  if (tree_int_cst_equal (node->low, node->high))
    {
      do_jump_if_equal (index, expand_expr (node->low, 0, VOIDmode, 0),
			label_rtx (node->code_label), unsignedp);
      if (node->right)
	{
	  if (node->left)
	    {
	      emit_cmp_insn (index, expand_expr (node->high, 0, VOIDmode, 0), 0, 0, 0);

	      if (node_is_bounded (node->right))
		{
		  emit_jump_insn (gen_bgt_pat (label_rtx (node->right->code_label)));
		  if (node_is_bounded (node->left))
		    emit_jump (label_rtx (node->left->code_label));
		  else
		    emit_case_nodes (index, node->left,
				     default_label, unsignedp);
		}
	      else
		{
		  if (node_is_bounded (node->left))
		    emit_jump_insn (gen_blt_pat (label_rtx (node->left->code_label)));
		  else
		    {
		      node->right->test_label =
			build_decl (LABEL_DECL, (tree) 0  , (tree) 0  );
		      emit_jump_insn (gen_bgt_pat (label_rtx (node->right->test_label)));
		      emit_case_nodes (index, node->left,
				       default_label, unsignedp);
		    }
		  emit_case_nodes (index, node->right,
				   default_label, unsignedp);
		}
	    }
	  else
	    {

	      if (node->right->right && !node_has_low_bound (node))
		{
		  emit_cmp_insn (index, expand_expr (node->high, 0, VOIDmode, 0), 0, 0, 0);
		  emit_jump_insn (gen_blt_pat (default_label));
		}
	      if (node_is_bounded (node->right))
		emit_jump (label_rtx (node->right->code_label));
	      else
		emit_case_nodes (index, node->right, default_label, unsignedp);
	    }
	}
      else if (node->left)
	{
	  if (node_is_bounded (node->left))
	    emit_jump (label_rtx (node->left->code_label));
	  else
	    emit_case_nodes (index, node->left, default_label, unsignedp);
	}
    }
  else
    {
      if (node->right)
	{
	  if (node->left)
	    {
	      emit_cmp_insn (index, expand_expr (node->high, 0, VOIDmode, 0), 0, 0, 0);
	      if (node_is_bounded (node->right))
		{

		  emit_jump_insn (gen_bgt_pat (label_rtx (node->right->code_label)));
		}
	      else
		{

		  node->right->test_label =
		    build_decl (LABEL_DECL, (tree) 0  , (tree) 0  );
		  emit_jump_insn (gen_bgt_pat (label_rtx (node->right->test_label)));
		}
	      emit_cmp_insn (index, expand_expr (node->low, 0, VOIDmode, 0), 0, 0, 0);
	      emit_jump_insn (gen_bge_pat (label_rtx (node->code_label)));
	      if (node_is_bounded (node->left))
		{

		  emit_jump (label_rtx (node->left->code_label));
		}
	      else
		emit_case_nodes (index, node->left, default_label, unsignedp);

	      if (node->right->test_label)
		emit_case_nodes (index, node->right, default_label, unsignedp);
	    }
	  else
	    {
	      if (!node_has_low_bound (node))
		{
		  emit_cmp_insn (index, expand_expr (node->low, 0, VOIDmode, 0), 0, 0, 0);
		  emit_jump_insn (gen_blt_pat (default_label));
		}
	      emit_cmp_insn (index, expand_expr (node->high, 0, VOIDmode, 0), 0, 0, 0);
	      emit_jump_insn (gen_ble_pat (label_rtx (node->code_label)));
	      if (node_is_bounded (node->right))
		{

		  emit_jump (label_rtx (node->right->code_label));
		}
	      else
		emit_case_nodes (index, node->right, default_label, unsignedp);
	    }
	}
      else if (node->left)
	{
	  if (!node_has_high_bound (node))
	    {
	      emit_cmp_insn (index, expand_expr (node->high, 0, VOIDmode, 0), 0, 0, 0);
	      emit_jump_insn (gen_bgt_pat (default_label));
	    }
	  emit_cmp_insn (index, expand_expr (node->low, 0, VOIDmode, 0), 0, 0, 0);
	  emit_jump_insn (gen_bge_pat (label_rtx (node->code_label)));
	  if (node_is_bounded (node->left))
	    {

	      emit_jump (label_rtx (node->left->code_label));
	    }
	  else
	    emit_case_nodes (index, node->left, default_label, unsignedp);
	}
      else
	{

	  if (!node_has_high_bound (node))
	    {
	      emit_cmp_insn (index, expand_expr (node->high, 0, VOIDmode, 0), 0, 0, 0);
	      emit_jump_insn (gen_bgt_pat (default_label));
	    }
	  if (!node_has_low_bound (node))
	    {
	      emit_cmp_insn (index, expand_expr (node->low, 0, VOIDmode, 0), 0, 0, 0);
	      emit_jump_insn (gen_bge_pat (label_rtx (node->code_label)));
	    }

	}
    }
}

int
get_frame_size ()
{

  return -frame_offset;

}

rtx
assign_stack_local (mode, size)
     enum machine_mode mode;
     int size;
{
  register rtx x, addr;
  int bigend_correction = 0;

  frame_pointer_needed = 1;

  size = (((size + (16  / 8 ) - 1)
	   / (16  / 8 ))
	  * (16  / 8 ));

  if (mode != BLKmode)
    bigend_correction = size - 	(mode_size[(int)(mode)]) ;

  frame_offset -= size;

  addr = gen_rtx (PLUS, SImode , frame_pointer_rtx,
		  gen_rtx (CONST_INT, VOIDmode,
			   (frame_offset + bigend_correction)));

  if (! memory_address_p (mode, addr))
    invalid_stack_slot = 1;

  x = gen_rtx (MEM, mode, addr);

  stack_slot_list = gen_rtx (EXPR_LIST, VOIDmode, x, stack_slot_list);

  return x;
}

void
put_var_into_stack (decl)
     tree decl;
{
  register rtx reg = ((decl)->decl.rtl) ;
  register rtx new;

  if (reg == 0)
    return;
  if (	((reg)->code)  != REG)
    return;

  new = parm_stack_loc (reg);
  if (new == 0)
    new = assign_stack_local (	((reg)->mode) , 	(mode_size[(int)(	((reg)->mode) )]) );

  ((reg)->fld[ 0].rtx)  = ((new)->fld[ 0].rtx) ;
  ((reg)->volatil)  = 0;
  ((reg)->code = ( MEM)) ;

  ((reg)->in_struct) 
    = (((((decl)->common.type) )->common.code)  == ARRAY_TYPE
       || ((((decl)->common.type) )->common.code)  == RECORD_TYPE
       || ((((decl)->common.type) )->common.code)  == UNION_TYPE);

  fixup_var_refs (reg);
}
static void
fixup_var_refs (var)
     rtx var;
{
  extern rtx sequence_stack;
  rtx stack = sequence_stack;
  tree pending;

  stack = sequence_stack;

  fixup_var_refs_insns (var, get_insns (), stack == 0);

  for (; stack; stack = ((((stack)->fld[ 1].rtx) )->fld[ 1].rtx) )
    {
      push_to_sequence (((stack)->fld[ 0].rtx) );
      fixup_var_refs_insns (var, ((stack)->fld[ 0].rtx) ,
			    ((((stack)->fld[ 1].rtx) )->fld[ 1].rtx)  == 0);
      end_sequence ();
    }

  for (pending = rtl_expr_chain; pending; pending = ((pending)->common.chain) )
    {
      rtx seq = (*(struct rtx_def **) &(((pending)->list.value) )->exp.operands[0]) ;
      if (seq != const0_rtx && seq != 0)
	{
	  push_to_sequence (seq);
	  fixup_var_refs_insns (var, seq, 0);
	  end_sequence ();
	}
    }
}

static void
fixup_var_refs_insns (var, insn, toplevel)
     rtx var;
     rtx insn;
     int toplevel;
{
  while (insn)
    {
      rtx next = ((insn)->fld[2].rtx) ;
      rtx note;
      if (	((insn)->code)  == INSN || 	((insn)->code)  == CALL_INSN
	  || 	((insn)->code)  == JUMP_INSN)
	{

	  if (toplevel
	      && 	((((insn)->fld[3].rtx) )->code)  == SET
	      && ((((insn)->fld[3].rtx) )->fld[0].rtx)  == var
	      && rtx_equal_p (((((insn)->fld[3].rtx) )->fld[1].rtx) , var))
	    {
	      next = delete_insn (insn);
	      if (insn == last_parm_insn)
		last_parm_insn = ((next)->fld[1].rtx) ;
	    }
	  else
	    fixup_var_refs_1 (var, ((insn)->fld[3].rtx) , insn);

	  for (note = ((insn)->fld[6].rtx) ; note; note = ((note)->fld[ 1].rtx) )
	    if (	((note)->code)  != INSN_LIST)
	      ((note)->fld[ 0].rtx)  = walk_fixup_memory_subreg (((note)->fld[ 0].rtx) , insn);
	}
      insn = next;
    }
}
static rtx
fixup_var_refs_1 (var, x, insn)
     register rtx var;
     register rtx x;
     rtx insn;
{
  register int i;
  enum rtx_code  code = 	((x)->code) ;
  register char *fmt;
  register rtx tem;

  switch (code)
    {
    case MEM:
      if (var == x)
	{
	  x = fixup_stack_1 (x, insn);
	  tem = gen_reg_rtx (	((x)->mode) );
	  emit_insn_before (gen_move_insn (tem, x), insn);
	  return tem;
	}
      break;

    case REG:
    case CC0:
    case PC:
    case CONST_INT:
    case CONST:
    case SYMBOL_REF:
    case LABEL_REF:
    case CONST_DOUBLE:
      return x;

    case SIGN_EXTRACT:
    case ZERO_EXTRACT:

    case SUBREG:
      tem = x;
      while (	((tem)->code)  == SUBREG || 	((tem)->code)  == SIGN_EXTRACT
	     || 	((tem)->code)  == ZERO_EXTRACT)
	tem = ((tem)->fld[ 0].rtx) ;
      if (tem == var)
	{
	  x = fixup_stack_1 (x, insn);
	  tem = gen_reg_rtx (	((x)->mode) );
	  if (	((x)->code)  == SUBREG)
	    x = fixup_memory_subreg (x, insn);
	  emit_insn_before (gen_move_insn (tem, x), insn);
	  return tem;
	}
      break;

    case SET:
      if (	((((x)->fld[0].rtx) )->code)  == SIGN_EXTRACT
	  || 	((((x)->fld[0].rtx) )->code)  == ZERO_EXTRACT)
	optimize_bit_field (x, insn, 0);
      if (	((((x)->fld[1].rtx) )->code)  == SIGN_EXTRACT
	  || 	((((x)->fld[1].rtx) )->code)  == ZERO_EXTRACT)
	optimize_bit_field (x, insn, 0);

      {
	rtx dest = ((x)->fld[0].rtx) ;
	rtx src = ((x)->fld[1].rtx) ;
	rtx outerdest = dest;
	rtx outersrc = src;

	while (	((dest)->code)  == SUBREG || 	((dest)->code)  == STRICT_LOW_PART
	       || 	((dest)->code)  == SIGN_EXTRACT
	       || 	((dest)->code)  == ZERO_EXTRACT)
	  dest = ((dest)->fld[ 0].rtx) ;
	while (	((src)->code)  == SUBREG
	       || 	((src)->code)  == SIGN_EXTRACT
	       || 	((src)->code)  == ZERO_EXTRACT)
	  src = ((src)->fld[ 0].rtx) ;

        if (src != var && dest != var)
	  break;

	if ((	((outerdest)->code)  == SIGN_EXTRACT
	     || 	((outerdest)->code)  == ZERO_EXTRACT)
	    && 	((((outerdest)->fld[ 0].rtx) )->code)  == SUBREG
	    && ((((outerdest)->fld[ 0].rtx) )->fld[0].rtx)  == var)
	  ((outerdest)->fld[ 0].rtx)  = fixup_memory_subreg (((outerdest)->fld[ 0].rtx) , insn);

	if ((	((outersrc)->code)  == SIGN_EXTRACT
	     || 	((outersrc)->code)  == ZERO_EXTRACT)
	    && 	((((outersrc)->fld[ 0].rtx) )->code)  == SUBREG
	    && ((((outersrc)->fld[ 0].rtx) )->fld[0].rtx)  == var)
	  ((outersrc)->fld[ 0].rtx)  = fixup_memory_subreg (((outersrc)->fld[ 0].rtx) , insn);

	if ((	((outerdest)->code)  == SIGN_EXTRACT
	     || 	((outerdest)->code)  == ZERO_EXTRACT)
	    && 	((((outerdest)->fld[ 0].rtx) )->code)  == MEM
	    && 	((((outerdest)->fld[ 0].rtx) )->mode)  != QImode)
	  {
	    ((outerdest)->fld[ 0].rtx)  = copy_rtx (((outerdest)->fld[ 0].rtx) );
	    ((((outerdest)->fld[ 0].rtx) )->mode = ( QImode)) ;
	  }

	if ((	((outersrc)->code)  == SIGN_EXTRACT
	     || 	((outersrc)->code)  == ZERO_EXTRACT)
	    && 	((((outersrc)->fld[ 0].rtx) )->code)  == MEM
	    && 	((((outersrc)->fld[ 0].rtx) )->mode)  != QImode)
	  {
	    ((outersrc)->fld[ 0].rtx)  = copy_rtx (((outersrc)->fld[ 0].rtx) );
	    ((((outersrc)->fld[ 0].rtx) )->mode = ( QImode)) ;
	  }

	if (dest == var && 	((((x)->fld[0].rtx) )->code)  == STRICT_LOW_PART)
	  ((x)->fld[0].rtx)  = ((((x)->fld[0].rtx) )->fld[ 0].rtx) ;

	if (	((((x)->fld[1].rtx) )->code)  == REG || 	((((x)->fld[0].rtx) )->code)  == REG
	    || (	((((x)->fld[1].rtx) )->code)  == SUBREG
		&& 	((((((x)->fld[1].rtx) )->fld[0].rtx) )->code)  == REG)
	    || (	((((x)->fld[0].rtx) )->code)  == SUBREG
		&& 	((((((x)->fld[0].rtx) )->fld[0].rtx) )->code)  == REG))
	  {
	    if (src == var && 	((((x)->fld[1].rtx) )->code)  == SUBREG)
	      ((x)->fld[1].rtx)  = fixup_memory_subreg (((x)->fld[1].rtx) , insn);
	    if (dest == var && 	((((x)->fld[0].rtx) )->code)  == SUBREG)
	      ((x)->fld[0].rtx)  = fixup_memory_subreg (((x)->fld[0].rtx) , insn);
	    return fixup_stack_1 (x, insn);
	  }

	if (dest == var)
	  {
	    rtx temp;
	    rtx fixeddest;
	    tem = ((x)->fld[0].rtx) ;
	    if (	((tem)->code)  == STRICT_LOW_PART)
	      tem = ((tem)->fld[ 0].rtx) ;
	    if (	((tem)->code)  == SUBREG)
	      tem = fixup_memory_subreg (tem, insn);
	    fixeddest = fixup_stack_1 (tem, insn);
	    temp = gen_reg_rtx (	((tem)->mode) );
	    emit_insn_after (gen_move_insn (fixeddest, temp), insn);
	    ((x)->fld[0].rtx)  = temp;
	  }
      }
    }

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	((x)->fld[ i].rtx)  = fixup_var_refs_1 (var, ((x)->fld[ i].rtx) , insn);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
	    ((x)->fld[ i].rtvec->elem[ j].rtx) 
	      = fixup_var_refs_1 (var, ((x)->fld[ i].rtvec->elem[ j].rtx) , insn);
	}
    }
  return x;
}

static rtx
fixup_memory_subreg (x, insn)
     rtx x;
     rtx insn;
{
  int offset = ((x)->fld[1].rtint)  * 4 ;
  rtx addr = ((((x)->fld[0].rtx) )->fld[ 0].rtx) ;
  enum machine_mode mode = 	((x)->mode) ;
  rtx saved, result;

  offset += ((((4 ) < ( 	(mode_size[(int)(	((((x)->fld[0].rtx) )->mode) )]) )) ? (4 ) : ( 	(mode_size[(int)(	((((x)->fld[0].rtx) )->mode) )]) )) 
	     - (((4 ) < ( 	(mode_size[(int)(mode)]) )) ? (4 ) : ( 	(mode_size[(int)(mode)]) )) );

  addr = plus_constant (addr, offset);
  if (memory_address_p (mode, addr))
    return change_address (((x)->fld[0].rtx) , mode, addr);
  saved = start_sequence ();
  result = change_address (((x)->fld[0].rtx) , mode, addr);
  emit_insn_before (gen_sequence (), insn);
  end_sequence (saved);
  return result;
}

static rtx
walk_fixup_memory_subreg (x, insn)
     register rtx x;
     rtx insn;
{
  register enum rtx_code code;
  register char *fmt;
  register int i;

  if (x == 0)
    return 0;

  code = 	((x)->code) ;

  if (code == SUBREG && 	((((x)->fld[0].rtx) )->code)  == MEM)
    return fixup_memory_subreg (x, insn);

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	((x)->fld[ i].rtx)  = walk_fixup_memory_subreg (((x)->fld[ i].rtx) , insn);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
	    ((x)->fld[ i].rtvec->elem[ j].rtx) 
	      = walk_fixup_memory_subreg (((x)->fld[ i].rtvec->elem[ j].rtx) , insn);
	}
    }
  return x;
}

static rtx
fixup_stack_1 (x, insn)
     rtx x;
     rtx insn;
{
  register int i;
  register enum rtx_code  code = 	((x)->code) ;
  register char *fmt;

  if (code == MEM)
    {
      register rtx ad = ((x)->fld[ 0].rtx) ;

      if (	((ad)->code)  == PLUS
	  && ((ad)->fld[ 0].rtx)  == frame_pointer_rtx
	  && 	((((ad)->fld[ 1].rtx) )->code)  == CONST_INT)
	{
	  rtx temp;
	  if (memory_address_p (	((x)->mode) , ad))
	    return x;
	  temp = gen_reg_rtx (	((ad)->mode) );
	  emit_insn_before (gen_move_insn (temp, ad), insn);
	  return change_address (x, VOIDmode, temp);
	}
      return x;
    }

  fmt = 	(rtx_format[(int)(code)]) ;
  for (i = 	(rtx_length[(int)(code)])  - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	((x)->fld[ i].rtx)  = fixup_stack_1 (((x)->fld[ i].rtx) , insn);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
	    ((x)->fld[ i].rtvec->elem[ j].rtx)  = fixup_stack_1 (((x)->fld[ i].rtvec->elem[ j].rtx) , insn);
	}
    }
  return x;
}

static void
optimize_bit_field (body, insn, equiv_mem)
     rtx body;
     rtx insn;
     rtx *equiv_mem;
{
  register rtx bitfield;
  int destflag;

  if (	((((body)->fld[0].rtx) )->code)  == SIGN_EXTRACT
      || 	((((body)->fld[0].rtx) )->code)  == ZERO_EXTRACT)
    bitfield = ((body)->fld[0].rtx) , destflag = 1;
  else
    bitfield = ((body)->fld[1].rtx) , destflag = 0;

  if (	((((bitfield)->fld[ 1].rtx) )->code)  == CONST_INT
      && 	((((bitfield)->fld[ 2].rtx) )->code)  == CONST_INT
      && (((((bitfield)->fld[ 1].rtx) )->fld[0].rtint)  ==  (8  * mode_size[(int)(QImode)]) 
	  || ((((bitfield)->fld[ 1].rtx) )->fld[0].rtint)  ==  (8  * mode_size[(int)(HImode)]) )
      && ((((bitfield)->fld[ 2].rtx) )->fld[0].rtint)  % ((((bitfield)->fld[ 1].rtx) )->fld[0].rtint)  == 0)
    {
      register rtx memref = 0;

      if (	((((bitfield)->fld[ 0].rtx) )->code)  == MEM)
	memref = ((bitfield)->fld[ 0].rtx) ;
      else if (	((((bitfield)->fld[ 0].rtx) )->code)  == REG
	       && equiv_mem != 0)
	memref = equiv_mem[((((bitfield)->fld[ 0].rtx) )->fld[0].rtint) ];
      else if (	((((bitfield)->fld[ 0].rtx) )->code)  == SUBREG
	       && 	((((((bitfield)->fld[ 0].rtx) )->fld[0].rtx) )->code)  == MEM)
	memref = ((((bitfield)->fld[ 0].rtx) )->fld[0].rtx) ;
      else if (	((((bitfield)->fld[ 0].rtx) )->code)  == SUBREG
	       && equiv_mem != 0
	       && 	((((((bitfield)->fld[ 0].rtx) )->fld[0].rtx) )->code)  == REG)
	memref = equiv_mem[((((((bitfield)->fld[ 0].rtx) )->fld[0].rtx) )->fld[0].rtint) ];

      if (memref
	  && ! mode_dependent_address_p (((memref)->fld[ 0].rtx) )
	  && offsetable_address_p (0, 	((bitfield)->mode) , ((memref)->fld[ 0].rtx) ))
	{

	  register int offset
	    = ((((bitfield)->fld[ 2].rtx) )->fld[0].rtint)  /  (8  * mode_size[(int)(QImode)]) ;
	  if (	((((bitfield)->fld[ 0].rtx) )->code)  == SUBREG)
	    {
	      offset += ((((bitfield)->fld[ 0].rtx) )->fld[1].rtint)  * 4 ;

	      offset -= ((((4 ) < (
			      	(mode_size[(int)(	((((bitfield)->fld[ 0].rtx) )->mode) )]) )) ? (4 ) : (
			      	(mode_size[(int)(	((((bitfield)->fld[ 0].rtx) )->mode) )]) )) 
			 - (((4 ) < (
					(mode_size[(int)(	((memref)->mode) )]) )) ? (4 ) : (
					(mode_size[(int)(	((memref)->mode) )]) )) );

	    }

	  memref = gen_rtx (MEM,
			    (((((bitfield)->fld[ 1].rtx) )->fld[0].rtint)  ==  (8  * mode_size[(int)(QImode)]) 
			     ? QImode : HImode),
			    ((memref)->fld[ 0].rtx) );

	  if (destflag)
	    {
	      ((body)->fld[0].rtx) 
		= adj_offsetable_operand (memref, offset);
	      if (!  (	((((body)->fld[1].rtx) )->code)  == LABEL_REF || 	((((body)->fld[1].rtx) )->code)  == SYMBOL_REF	|| 	((((body)->fld[1].rtx) )->code)  == CONST_INT	|| 	((((body)->fld[1].rtx) )->code)  == CONST)  )
		{
		  rtx src = ((body)->fld[1].rtx) ;
		  while (	((src)->code)  == SUBREG
			 && ((src)->fld[1].rtint)  == 0)
		    src = ((src)->fld[0].rtx) ;
		  if (	((src)->mode)  != 	((memref)->mode) )
		    src = gen_lowpart (	((memref)->mode) , ((body)->fld[1].rtx) );
		  ((body)->fld[1].rtx)  = src;
		}
	      else if (	((((body)->fld[1].rtx) )->mode)  != VOIDmode
		       && 	((((body)->fld[1].rtx) )->mode)  != 	((memref)->mode) )

		abort ();
	    }
	  else
	    {
	      rtx dest = ((body)->fld[0].rtx) ;

	      while (	((dest)->code)  == SUBREG
		     && ((dest)->fld[1].rtint)  == 0)
		dest = ((dest)->fld[0].rtx) ;
	      ((body)->fld[0].rtx)  = dest;

	      memref = adj_offsetable_operand (memref, offset);
	      if (	((dest)->mode)  == 	((memref)->mode) )
		((body)->fld[1].rtx)  = memref;
	      else
		{
		  rtx last = get_last_insn ();
		  rtx newreg = gen_reg_rtx (	((dest)->mode) );
		  convert_move (newreg, memref,
					((((body)->fld[1].rtx) )->code)  == ZERO_EXTRACT);
		  reorder_insns (((last)->fld[2].rtx) , get_last_insn (),
				 ((insn)->fld[1].rtx) );
		  ((body)->fld[1].rtx)  = newreg;
		}
	    }

	  ((insn)->fld[4].rtint)  = -1;
	}
    }
}

static int max_parm_reg;

static rtx *parm_reg_stack_loc;

int
max_parm_reg_num ()
{
  return max_parm_reg;
}

rtx
get_first_nonparm_insn ()
{
  if (last_parm_insn)
    return ((last_parm_insn)->fld[2].rtx) ;
  return get_insns ();
}

static rtx
parm_stack_loc (reg)
     rtx reg;
{
  if (((reg)->fld[0].rtint)  < max_parm_reg)
    return parm_reg_stack_loc[((reg)->fld[0].rtint) ];
  return 0;
}

static void
assign_parms (fndecl)
     tree fndecl;
{
  register tree parm;
  register rtx entry_parm;
  register rtx stack_parm;
  register int  args_so_far;
  enum machine_mode passed_mode, nominal_mode;

  struct args_size stack_args_size;
  int first_parm_offset = 8 ;
  tree fntype = ((fndecl)->common.type) ;

  int nparmregs
    = list_length (((fndecl)->decl.arguments)  ) + 56 ;

  int vararg
    = ((((fndecl)->decl.arguments)   != 0
	&& ((((fndecl)->decl.arguments)  )->decl.name) 
	&& (! strcmp (((((((fndecl)->decl.arguments)  )->decl.name) )->identifier.pointer) ,
		      "__builtin_va_alist")))
       ||
       (((fntype)->type.values)  != 0
	&& (((tree_last (((fntype)->type.values) ))->list.value) 
	    != void_type_node)));

  stack_args_size.constant = 0;
  stack_args_size.var = 0;

  if ((((((fndecl)->decl.result) )->decl.mode)  == BLKmode
       || 0 )
      && 	((struct_value_incoming_rtx)->code)  == MEM)
    stack_args_size.constant += 	(mode_size[(int)(SImode )]) ;

  parm_reg_stack_loc = (rtx *) oballoc (nparmregs * sizeof (rtx));
  memset (parm_reg_stack_loc,0, nparmregs * sizeof (rtx)) ;

  ((args_so_far) = 0) ;

  for (parm = ((fndecl)->decl.arguments)  ; parm; parm = ((parm)->common.chain) )
    {
      int aggregate
	= (((((parm)->common.type) )->common.code)  == ARRAY_TYPE
	   || ((((parm)->common.type) )->common.code)  == RECORD_TYPE
	   || ((((parm)->common.type) )->common.code)  == UNION_TYPE);
      struct args_size stack_offset;
      rtx stack_offset_rtx;
      enum direction where_pad;

      ((parm)->decl.offset)  = -1;

      if (((parm)->common.type)  == error_mark_node

	  || ((parm)->common.code)  != PARM_DECL
	  || ((parm)->decl.arguments)    == 0 )
	{
	  ((parm)->decl.rtl)  = gen_rtx (MEM, BLKmode, const0_rtx);
	  ((parm)->common.used_attr)  = 1;
	  continue;
	}

      passed_mode = ((((parm)->decl.arguments)   )->type.mode) ;
      nominal_mode = ((((parm)->common.type) )->type.mode) ;

      stack_offset = stack_args_size;
      stack_offset.constant += first_parm_offset;

      where_pad
	= (((passed_mode) == BLKmode	? (	((
				expand_expr (size_in_bytes (((parm)->decl.arguments)   ),
					     0, VOIDmode, 0))->code)  == CONST_INT	&& ((
				expand_expr (size_in_bytes (((parm)->decl.arguments)   ),
					     0, VOIDmode, 0))->fld[0].rtint)  < ((target_flags & 040)  ? 16 : 32)  / 8 )	:  (8  * mode_size[(int)(passed_mode)])  < ((target_flags & 040)  ? 16 : 32) )	? downward : upward) ;

      if (where_pad == downward)
	{
	  if (passed_mode != BLKmode)
	    {
	      if ( (8  * mode_size[(int)(passed_mode)])  % ((target_flags & 040)  ? 16 : 32) )
		stack_offset.constant
		  += ((( (8  * mode_size[(int)(passed_mode)])  + ((target_flags & 040)  ? 16 : 32)  - 1)
		       / ((target_flags & 040)  ? 16 : 32)  * ((target_flags & 040)  ? 16 : 32)  / 8 )
		      - 	(mode_size[(int)(passed_mode)]) );
	    }
	  else
	    {
	      tree sizetree = size_in_bytes (((parm)->decl.arguments)   );
	      tree s1 = convert_units (sizetree, 8 , ((target_flags & 040)  ? 16 : 32) );
	      tree s2 = convert_units (s1, ((target_flags & 040)  ? 16 : 32) , 8 );
	      { tree inc = ( s2);	if (((inc)->common.code)  == INTEGER_CST)	(stack_offset).constant += ((inc)->int_cst.int_cst_low) ;	else if ((stack_offset).var == 0)	(stack_offset).var = inc;	else	(stack_offset).var = genop (PLUS_EXPR, (stack_offset).var, inc); } ;
	      { tree dec = ( sizetree);	if (((dec)->common.code)  == INTEGER_CST)	(stack_offset).constant -= ((dec)->int_cst.int_cst_low) ;	else if ((stack_offset).var == 0)	(stack_offset).var = genop (MINUS_EXPR, integer_zero_node, dec); else	(stack_offset).var = genop (MINUS_EXPR, (stack_offset).var, dec); } ;
	    }
	}

      stack_offset_rtx = ((stack_offset).var == 0 ? gen_rtx (CONST_INT, VOIDmode, (stack_offset).constant)	: plus_constant (expand_expr ((stack_offset).var, 0, VOIDmode, 0),	(stack_offset).constant)) ;

      stack_parm
	= gen_rtx (MEM, passed_mode,
		   memory_address (passed_mode,
				   gen_rtx (PLUS, SImode ,
					    arg_pointer_rtx, stack_offset_rtx)));

      ((stack_parm)->in_struct)  = aggregate;

      entry_parm = 0;
      if (((((((parm)->common.type) )->type.size) )->common.code)  == INTEGER_CST
	  || stack_offset.var != 0)
	{

	  entry_parm
	    = (((target_flags & 020)  && (args_so_far) < 8) ? gen_rtx (REG, ( passed_mode), (args_so_far) / 4) : 0) ;

	}

      {
	int nregs = 0;
	int i;

	nregs = (((target_flags & 020)  && (args_so_far) < 8	&& 8 < ((args_so_far) + (( passed_mode) == BLKmode	? int_size_in_bytes (
					    ((parm)->decl.arguments)   )	: 	(mode_size[(int)( passed_mode)]) ))) ? 2 - (args_so_far) / 4 : 0) ;

	if (((parm)->common.chain)  == 0 && vararg && entry_parm != 0)
	  {
	    if (	((entry_parm)->mode)  == BLKmode)
	      nregs = 	(mode_size[(int)(	((entry_parm)->mode) )])  / 4 ;
	    else
	      nregs = (int_size_in_bytes (((parm)->decl.arguments)   )
		       / 4 );
	  }

	if (nregs > 0)
	  {
	    current_function_pretend_args_size
	      = (((nregs * 4 ) + (((target_flags & 040)  ? 16 : 32)  / 8 ) - 1)
		 / (((target_flags & 040)  ? 16 : 32)  / 8 )
		 * (((target_flags & 040)  ? 16 : 32)  / 8 ));

	    i = nregs;
	    while (--i >= 0)
	      emit_move_insn (gen_rtx (MEM, SImode,
				       plus_constant (((stack_parm)->fld[ 0].rtx) ,
						      i * 	(mode_size[(int)(SImode)]) )),
			      gen_rtx (REG, SImode, ((entry_parm)->fld[0].rtint)  + i));
	    entry_parm = stack_parm;
	  }
      }

      if (entry_parm == 0)
	entry_parm = stack_parm;

      if (entry_parm == stack_parm)
	((parm)->decl.offset)  = stack_offset.constant * 8 ;

      if (entry_parm == stack_parm

	  )
	{
	  tree sizetree = size_in_bytes (((parm)->decl.arguments)   );
	  if (where_pad != none)
	    {
	      tree s1 = convert_units (sizetree, 8 , ((target_flags & 040)  ? 16 : 32) );
	      sizetree = convert_units (s1, ((target_flags & 040)  ? 16 : 32) , 8 );
	    }
	  { tree inc = ( sizetree);	if (((inc)->common.code)  == INTEGER_CST)	(stack_args_size).constant += ((inc)->int_cst.int_cst_low) ;	else if ((stack_args_size).var == 0)	(stack_args_size).var = inc;	else	(stack_args_size).var = genop (PLUS_EXPR, (stack_args_size).var, inc); } ;
	}
      else
	stack_parm = 0;

      if (nominal_mode != BLKmode && nominal_mode != passed_mode
	  && stack_parm != 0)
	{

	  if (	(mode_size[(int)(nominal_mode)])  < 4 )
	    {
	      stack_offset.constant
		+= 	(mode_size[(int)(passed_mode)]) 
		  - 	(mode_size[(int)(nominal_mode)]) ;
	      stack_offset_rtx = ((stack_offset).var == 0 ? gen_rtx (CONST_INT, VOIDmode, (stack_offset).constant)	: plus_constant (expand_expr ((stack_offset).var, 0, VOIDmode, 0),	(stack_offset).constant)) ;
	    }

	  stack_parm
	    = gen_rtx (MEM, nominal_mode,
		       memory_address (nominal_mode,
				       gen_rtx (PLUS, SImode ,
						arg_pointer_rtx,
						stack_offset_rtx)));

	  ((stack_parm)->in_struct)  = aggregate;
	}

      if (nominal_mode == BLKmode)
	{
	  if (	((entry_parm)->code)  == REG)
	    {
	      if (stack_parm == 0)
		stack_parm
		  = assign_stack_local (	((entry_parm)->mode) ,
					int_size_in_bytes (((parm)->common.type) ));

	      move_block_from_reg (((entry_parm)->fld[0].rtint) , stack_parm,
				   int_size_in_bytes (((parm)->common.type) )
				   / 4 );
	    }
	  ((parm)->decl.rtl)  = stack_parm;
	}
      else if (! ((obey_regdecls && ! ((parm)->common.regdecl_attr) 
		   && ! ((fndecl)->common.inline_attr) )
		  || ((parm)->common.addressable_attr) 
		  || ((parm)->common.volatile_attr) 

		  || (flag_float_store
		      && ((((parm)->common.type) )->common.code)  == REAL_TYPE)))
	{
	  register rtx parmreg = gen_reg_rtx (nominal_mode);

	  ((parmreg)->volatil)  = 1;
	  ((parm)->decl.rtl)  = parmreg;

	  if (	((parmreg)->mode)  != 	((entry_parm)->mode) )
	    convert_move (parmreg, entry_parm, 0);
	  else
	    emit_move_insn (parmreg, entry_parm);

	  if (((parmreg)->fld[0].rtint)  >= nparmregs)
	    {
	      rtx *new;
	      nparmregs = ((parmreg)->fld[0].rtint)  + 5;
	      new = (rtx *) oballoc (nparmregs * sizeof (rtx));
	      memcpy ( new,parm_reg_stack_loc, nparmregs * sizeof (rtx)) ;
	      parm_reg_stack_loc = new;
	    }
	  parm_reg_stack_loc[((parmreg)->fld[0].rtint) ] = stack_parm;

	  if (nominal_mode == passed_mode
	      && 	((entry_parm)->code)  == MEM
	      && stack_offset.var == 0)
	    ((get_last_insn ())->fld[6].rtx) 
	      = gen_rtx (EXPR_LIST, REG_EQUIV,
			 entry_parm, ((get_last_insn ())->fld[6].rtx) );

	  if (((((parm)->common.type) )->common.code)  == POINTER_TYPE)
	    mark_reg_pointer (parmreg);
	}
      else
	{

	  if (passed_mode != nominal_mode)
	    entry_parm = convert_to_mode (nominal_mode, entry_parm, 0);

	  if (entry_parm != stack_parm)
	    {
	      if (stack_parm == 0)
		stack_parm = assign_stack_local (	((entry_parm)->mode) ,
						 	(mode_size[(int)(	((entry_parm)->mode) )]) );
	      emit_move_insn (stack_parm, entry_parm);
	    }

	  ((parm)->decl.rtl)  = stack_parm;
	  frame_pointer_needed = 1;
	}
      if (((parm)->common.volatile_attr) )
	((((parm)->decl.rtl) )->volatil)  = 1;
      if (((parm)->common.readonly_attr) )
	((((parm)->decl.rtl) )->unchanging)  = 1;

      ((args_so_far) += (( passed_mode) != BLKmode	? (	(mode_size[(int)( passed_mode)])  + 3) & ~3	: (int_size_in_bytes ( ((parm)->decl.arguments)   ) + 3) & ~3)) ;
    }

  max_parm_reg = max_reg_num ();
  last_parm_insn = get_last_insn ();

  current_function_args_size = stack_args_size.constant;
}

static int max_structure_value_size;

static rtx structure_value;

rtx
get_structure_value_addr (sizex)
     rtx sizex;
{
  register int size;
  if (	((sizex)->code)  != CONST_INT)
    abort ();
  size = ((sizex)->fld[0].rtint) ;

  size = (((size + (16  / 8 ) - 1)
	   / (16  / 8 ))
	  * (16  / 8 ));

  if (size > max_structure_value_size)
    {
      max_structure_value_size = size;
      structure_value = assign_stack_local (BLKmode, size);
      if (	((structure_value)->code)  == MEM)
	structure_value = ((structure_value)->fld[ 0].rtx) ;
    }

  return structure_value;
}

void
uninitialized_vars_warning (block)
     tree block;
{
  register tree decl, sub;
  for (decl = ((block)->bind_stmt.vars) ; decl; decl = ((decl)->common.chain) )
    {
      if (((decl)->common.code)  == VAR_DECL

	  && ((((decl)->common.type) )->common.code)  != RECORD_TYPE
	  && ((((decl)->common.type) )->common.code)  != UNION_TYPE
	  && ((((decl)->common.type) )->common.code)  != ARRAY_TYPE
	  && 	((((decl)->decl.rtl) )->code)  == REG
	  && regno_uninitialized (((((decl)->decl.rtl) )->fld[0].rtint) ))
	warning_with_decl (decl,
			   "`%s' may be used uninitialized in this function");
      if (((decl)->common.code)  == VAR_DECL
	  && 	((((decl)->decl.rtl) )->code)  == REG
	  && regno_clobbered_at_setjmp (((((decl)->decl.rtl) )->fld[0].rtint) ))
	warning_with_decl (decl,
			   "variable `%s' may be clobbered by `longjmp'");
    }
  for (sub = ((block)->stmt.body) ; sub; sub = ((sub)->common.chain) )
    uninitialized_vars_warning (sub);
}

void
setjmp_protect (block)
     tree block;
{
  register tree decl, sub;
  for (decl = ((block)->bind_stmt.vars) ; decl; decl = ((decl)->common.chain) )
    if ((((decl)->common.code)  == VAR_DECL
	 || ((decl)->common.code)  == PARM_DECL)
	&& ((decl)->decl.rtl)  != 0
	&& 	((((decl)->decl.rtl) )->code)  == REG
	&& ! ((decl)->common.regdecl_attr) )
      put_var_into_stack (decl);
  for (sub = ((block)->stmt.body) ; sub; sub = ((sub)->common.chain) )
    setjmp_protect (sub);
}

void
expand_function_start (subr)
     tree subr;
{
  register int i;
  tree tem;

  this_function = subr;
  cse_not_expected = ! optimize;

  frame_pointer_needed = 0  || ! flag_omit_frame_pointer;

  goto_fixup_chain = 0;

  stack_slot_list = 0;

  invalid_stack_slot = 0;

  init_emit (write_symbols);

  init_expr ();

  init_const_rtx_hash_table ();

  current_function_pops_args = ((target_flags & 8)  && ((((subr)->common.type) )->common.code)  != IDENTIFIER_NODE	&& (((((subr)->common.type) )->type.values)  == 0	|| ((tree_last (((((subr)->common.type) )->type.values) ))->list.value)  == void_type_node)) ;

  current_function_name = ((((subr)->decl.name) )->identifier.pointer) ;

  current_function_needs_context
    = (((current_function_decl)->decl.context)  != 0
       && ((((current_function_decl)->decl.context) )->common.code)  == LET_STMT);

  current_function_calls_setjmp = 0;

  current_function_returns_pcc_struct = 0;
  current_function_returns_struct = 0;

  max_structure_value_size = 0;
  structure_value = 0;

  block_stack = 0;
  loop_stack = 0;
  case_stack = 0;
  cond_stack = 0;
  nesting_stack = 0;
  nesting_depth = 0;

  tail_recursion_label = 0;

  frame_offset = 0 ;

  save_expr_regs = 0;

  rtl_expr_chain = 0;

  immediate_size_expand++;

  init_pending_stack_adjust ();
  clear_current_args_size ();
  current_function_pretend_args_size = 0;

  emit_line_note (((subr)->decl.filename) , ((subr)->decl.linenum) );

  emit_note (0, -1 );

  assign_parms (subr);

  if (((((subr)->decl.result) )->decl.mode)  == BLKmode
      || 0 
      || (flag_pcc_struct_return
	  && (((((((subr)->decl.result) )->common.type) )->common.code)  == RECORD_TYPE
	      || ((((((subr)->decl.result) )->common.type) )->common.code)  == UNION_TYPE)))
    {
      register rtx value_address;

      if (flag_pcc_struct_return)
	{
	  int size = int_size_in_bytes (((((subr)->decl.result) )->common.type) );
	  value_address = assemble_static_space (size);
	  current_function_returns_pcc_struct = 1;
	}
      else

	{
	  value_address = gen_reg_rtx (SImode );
	  emit_move_insn (value_address, struct_value_incoming_rtx);
	  current_function_returns_struct = 1;
	}
      ((((subr)->decl.result) )->decl.rtl) 
	= gen_rtx (MEM, ((((subr)->decl.result) )->decl.mode) ,
		   value_address);
    }
  else

    ((((subr)->decl.result) )->decl.rtl) 
      = gen_rtx (REG, ((((((subr)->decl.result) )->common.type) )->type.mode) , 0) ;

  if (	((((((subr)->decl.result) )->decl.rtl) )->code)  == REG)
    ((((((subr)->decl.result) )->decl.rtl) )->integrated)  = 1;

  if ((0)  && ! current_function_returns_pcc_struct)
    return_label = 0;
  else
    return_label = gen_label_rtx ();

  if (obey_regdecls)
    {
      parm_birth_insn = get_last_insn ();
      for (i = 56 ; i < max_parm_reg; i++)
	use_variable (regno_reg_rtx[i]);
    }

  tail_recursion_reentry = get_last_insn ();

  for (tem = get_pending_sizes (); tem; tem = ((tem)->common.chain) )
    expand_expr (((tem)->list.value) , 0, VOIDmode, 0);
}

void
expand_function_end (filename, line)
     char *filename;
     int line;
{
  register int i;
  extern rtx sequence_stack;

  while (sequence_stack)
    end_sequence (0);

  immediate_size_expand--;

  if (current_function_returns_struct)
    {
      rtx value_address = ((((((current_function_decl)->decl.result) )->decl.rtl) )->fld[ 0].rtx) ;
      tree type = ((((current_function_decl)->decl.result) )->common.type) ;
      rtx outgoing
	= hard_function_value (build_pointer_type (type),
			       current_function_decl);

      emit_move_insn (outgoing, value_address);
    }

  if (obey_regdecls)
    {
      rtx tem;
      for (i = 56 ; i < max_parm_reg; i++)
	use_variable (regno_reg_rtx[i]);

      for (tem = save_expr_regs; tem; tem = ((tem)->fld[ 1].rtx) )
	{
	  use_variable (((tem)->fld[ 0].rtx) );
	  use_variable_after (((tem)->fld[ 0].rtx) , parm_birth_insn);
	}
    }

  clear_pending_stack_adjust ();
  do_pending_stack_adjust ();

  emit_note (0, -6 );

  emit_line_note_force (filename, line);

  if ((0)  && ! current_function_returns_pcc_struct)
    emit_jump_insn (gen_return ());
  else

    emit_label (return_label);

  if (current_function_returns_pcc_struct)
    {
      rtx value_address = ((((((current_function_decl)->decl.result) )->decl.rtl) )->fld[ 0].rtx) ;
      tree type = ((((current_function_decl)->decl.result) )->common.type) ;
      rtx outgoing
	= hard_function_value (build_pointer_type (type),
			       current_function_decl);

      emit_move_insn (outgoing, value_address);
      use_variable (outgoing);

      if ((0) )
	{
	  emit_jump_insn (gen_return ());
	  emit_barrier ();
	}

    }

  fixup_gotos (0, 0, 0, get_insns (), 0);
}

