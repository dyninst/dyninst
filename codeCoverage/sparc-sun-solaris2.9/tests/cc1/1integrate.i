
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

extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

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
  int	temp;			 
  int   alignment_mask;		 
  struct _obstack_chunk *(*chunkfun) ();  
  void (*freefun) ();		 
};

void obstack_init (struct obstack *obstack);

void * obstack_alloc (struct obstack *obstack, int size);

void * obstack_copy (struct obstack *obstack, void *address, int size);
void * obstack_copy0 (struct obstack *obstack, void *address, int size);

void obstack_free (struct obstack *obstack, void *block);

void obstack_blank (struct obstack *obstack, int size);

void obstack_grow (struct obstack *obstack, void *data, int size);
void obstack_grow0 (struct obstack *obstack, void *data, int size);

void obstack_1grow (struct obstack *obstack, int data_char);

void * obstack_finish (struct obstack *obstack);

int obstack_object_size (struct obstack *obstack);

int obstack_room (struct obstack *obstack);
void obstack_1grow_fast (struct obstack *obstack, int data_char);
void obstack_blank_fast (struct obstack *obstack, int size);

void * obstack_base (struct obstack *obstack);
void * obstack_next_free (struct obstack *obstack);
int obstack_alignment_mask (struct obstack *obstack);
int obstack_chunk_size (struct obstack *obstack);

extern int xmalloc ();
extern void free ();

extern struct obstack permanent_obstack, maybepermanent_obstack;
extern struct obstack *rtl_obstack, *saveable_obstack, *current_obstack;

extern tree pushdecl ();

static rtx inline_target;

static rtx inline_fp_rtx;

static rtx *parm_map;

static char *fp_addr_p;

static int fp_delta;

static rtvec orig_asm_operands_vector;

static rtvec copy_asm_operands_vector;

static rtx copy_rtx_and_substitute ();
static rtx copy_address ();

static void copy_parm_decls ();
static void copy_decl_tree ();

static rtx try_fold_cc0 ();

static rtx fold_out_const_cc0 ();

char *
function_cannot_inline_p (fndecl)
     register tree fndecl;
{
  register rtx insn;
  tree last = tree_last (((((fndecl)->common.type) )->type.values) );
  int max_insns = (8 * (8 + list_length (((fndecl)->decl.arguments)  ) + 16*((fndecl)->common.inline_attr) )) ;
  register int ninsns = 0;
  register tree parms;

  if (last && ((last)->list.value)  != void_type_node)
    return "varargs function cannot be inline";

  if (get_max_uid () > 2 * max_insns)
    return "function too large to be inline";

  for (parms = ((fndecl)->decl.arguments)  ; parms; parms = ((parms)->common.chain) )
    {
      if (((((parms)->common.type) )->type.mode)  == BLKmode)
	return "function with large aggregate parameter cannot be inline";
      if (last == (tree) 0   && ((parms)->common.addressable_attr) )
	return "no prototype, and parameter address used; cannot be inline";

      if ((((((parms)->common.type) )->common.code)  == RECORD_TYPE
	   || ((((parms)->common.type) )->common.code)  == UNION_TYPE)
	  && 	((((parms)->decl.rtl) )->code)  == MEM)
	return "address of an aggregate parameter is used; cannot be inline";
    }

  if (get_max_uid () > max_insns)
    {
      for (ninsns = 0, insn = get_first_nonparm_insn (); insn && ninsns < max_insns;
	   insn = ((insn)->fld[2].rtx) )
	{
	  if (	((insn)->code)  == INSN
	      || 	((insn)->code)  == JUMP_INSN
	      || 	((insn)->code)  == CALL_INSN)
	    ninsns++;
	}

      if (ninsns >= max_insns)
	return "function too large to be inline";
    }

  return 0;
}

static rtx *reg_map;

static rtx *label_map;

static rtx *insn_map;

static tree *parmdecl_map;

static int max_parm_reg;

static int first_parm_offset;

extern rtx return_label;

static rtx copy_for_inline ();

void
save_for_inline (fndecl)
     tree fndecl;
{
  extern rtx *regno_reg_rtx;	 
  extern current_function_args_size;

  rtx first_insn, last_insn, insn;
  rtx head, copy;
  tree parms;
  int max_labelno, min_labelno, i, len;
  int max_reg;
  int max_uid;

  if (return_label == 0)
    {
      return_label = gen_label_rtx ();
      emit_label (return_label);
    }

  max_labelno = max_label_num ();
  min_labelno = get_first_label_num ();
  max_parm_reg = max_parm_reg_num ();
  max_reg = max_reg_num ();

  parmdecl_map = (tree *) __builtin_alloca  (max_parm_reg * sizeof (tree));
  memset (parmdecl_map,0, max_parm_reg * sizeof (tree)) ;

  for (parms = ((fndecl)->decl.arguments)  ; parms; parms = ((parms)->common.chain) )
    {
      rtx p = ((parms)->decl.rtl) ;

      if (	((p)->code)  == REG)
	{
	  parmdecl_map[((p)->fld[0].rtint) ] = parms;
	  ((parms)->common.volatile_attr)  = 0;
	}
      else
	((parms)->common.volatile_attr)  = 1;
      ((parms)->common.readonly_attr)  = 1;
    }

  head = gen_inline_header_rtx (0 , 0 , min_labelno, max_labelno,
				max_parm_reg, max_reg,
				current_function_args_size);
  max_uid = ((head)->fld[0].rtint) ;

  preserve_data ();

  insn = get_insns ();
  if (	((insn)->code)  != NOTE)
    abort ();
  first_insn = rtx_alloc (NOTE);
   ((first_insn)->fld[3].rtstr)  =  ((insn)->fld[3].rtstr) ;
  ((first_insn)->fld[4].rtint)  = ((insn)->fld[4].rtint) ;
  ((first_insn)->fld[0].rtint)  = ((insn)->fld[0].rtint) ;
  ((first_insn)->fld[1].rtx)  = 0 ;
  ((first_insn)->fld[2].rtx)  = 0 ;
  last_insn = first_insn;

  reg_map = (rtx *) __builtin_alloca  ((max_reg + 1) * sizeof (rtx));

  len = sizeof (struct rtx_def) + (	(rtx_length[(int)(REG)])  - 1) * sizeof (rtunion);
  for (i = max_reg - 1; i >= 56 ; i--)
    reg_map[i] = (rtx)({ struct obstack *__h = (&maybepermanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( len));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, ( regno_reg_rtx[i]), __len) ;	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
  memcpy (
	 regno_reg_rtx + 56 ,reg_map + 56 ,
	 (max_reg - 56 ) * sizeof (rtx)) ;

  label_map = (rtx *)__builtin_alloca  ((max_labelno - min_labelno) * sizeof (rtx));
  label_map -= min_labelno;

  for (i = min_labelno; i < max_labelno; i++)
    label_map[i] = gen_label_rtx ();

  insn_map = (rtx *) __builtin_alloca  (max_uid * sizeof (rtx));
  memset (insn_map,0, max_uid * sizeof (rtx)) ;

  for (insn = ((insn)->fld[2].rtx) ; insn; insn = ((insn)->fld[2].rtx) )
    {
      orig_asm_operands_vector = 0;
      copy_asm_operands_vector = 0;

      switch (	((insn)->code) )
	{
	case NOTE:
	  if (((insn)->fld[4].rtint)  == -6 )
	    continue;

	  copy = rtx_alloc (NOTE);
	   ((copy)->fld[3].rtstr)  =  ((insn)->fld[3].rtstr) ;
	  ((copy)->fld[4].rtint)  = ((insn)->fld[4].rtint) ;
	  break;

	case INSN:
	case CALL_INSN:
	case JUMP_INSN:
	  copy = rtx_alloc (	((insn)->code) );
	  ((copy)->fld[3].rtx)  = copy_for_inline (((insn)->fld[3].rtx) );
	  ((copy)->fld[4].rtint)  = -1;
	  	((copy)->fld[5].rtx)  = 0 ;
	  ((copy)->fld[6].rtx)  = copy_for_inline (((insn)->fld[6].rtx) );
	  break;

	case CODE_LABEL:
	  copy = label_map[((insn)->fld[3].rtint) ];
	  break;

	case BARRIER:
	  copy = rtx_alloc (BARRIER);
	  break;

	default:
	  abort ();
	}
      ((copy)->fld[0].rtint)  = ((insn)->fld[0].rtint) ;
      insn_map[((insn)->fld[0].rtint) ] = copy;
      ((last_insn)->fld[2].rtx)  = copy;
      ((copy)->fld[1].rtx)  = last_insn;
      last_insn = copy;
    }

  ((last_insn)->fld[2].rtx)  = 0 ;

  ((head)->fld[2].rtx)  = get_first_nonparm_insn ();
  ((head)->fld[3].rtx)  = get_insns ();
  ((fndecl)->decl.saved_insns)  = head;
  ((fndecl)->decl.frame_size)  = get_frame_size ();
  ((fndecl)->common.inline_attr)  = 1;

  parmdecl_map = 0;
  label_map = 0;
  reg_map = 0;
  return_label = 0;

  set_new_first_and_last_insn (first_insn, last_insn);
}

static rtx
copy_for_inline (orig)
     rtx orig;
{
  register rtx x = orig;
  register int i;
  register enum rtx_code code;
  register char *format_ptr;

  if (x == 0)
    return x;

  code = 	((x)->code) ;

  switch (code)
    {
    case QUEUED:
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case PC:
    case CC0:
      return x;

    case ASM_OPERANDS:

      if (orig_asm_operands_vector == ((orig)->fld[ 3].rtvec) )
	{
	  x = rtx_alloc (ASM_OPERANDS);
	  ((x)->fld[ 0].rtstr)  = ((orig)->fld[ 0].rtstr) ;
	  ((x)->fld[ 1].rtstr)  = ((orig)->fld[ 1].rtstr) ;
	  ((x)->fld[ 2].rtint)  = ((orig)->fld[ 2].rtint) ;
	  ((x)->fld[ 3].rtvec)  = copy_asm_operands_vector;
	  ((x)->fld[ 4].rtvec)  = ((orig)->fld[ 4].rtvec) ;
	  return x;
	}
      break;

    case MEM:

      if ( (	((((x)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((x)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST_INT	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST)  )
	return x;
      if (	((((x)->fld[ 0].rtx) )->code)  == PLUS
	  && 	((((((x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG
	  && (((((((x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  == 14 
	      || ((((((x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  == 14 )
	  &&  (	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	|| 	((((((x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST)  )
	if (	((((x)->fld[ 0].rtx) )->code)  == REG
	    && (((((x)->fld[ 0].rtx) )->fld[0].rtint)  == 14 
		|| ((((x)->fld[ 0].rtx) )->fld[0].rtint)  == 14 )
	    &&  (	((((x)->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((x)->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((x)->fld[ 1].rtx) )->code)  == CONST_INT	|| 	((((x)->fld[ 1].rtx) )->code)  == CONST)  )
	return x;
      break;

    case LABEL_REF:
      {
	return gen_rtx (LABEL_REF, 	((orig)->mode) ,
			label_map[((((orig)->fld[ 0].rtx) )->fld[3].rtint) ]);
      }

    case REG:
      if (((x)->fld[0].rtint)  >= 56 )
	return reg_map [((x)->fld[0].rtint) ];
      else
	return x;

    case SET:
      {
	rtx dest = ((x)->fld[0].rtx) ;

	if (	((dest)->code)  == REG
	    && ((dest)->fld[0].rtint)  < max_parm_reg
	    && ((dest)->fld[0].rtint)  >= 56 
	    && parmdecl_map[((dest)->fld[0].rtint) ] != 0)
	  ((parmdecl_map[((dest)->fld[0].rtint) ])->common.readonly_attr)  = 0;
      }
      break;
    }

  x = rtx_alloc (code);
  memcpy ( x,orig, sizeof (int) * (	(rtx_length[(int)(code)])  + 1)) ;

  format_ptr = 	(rtx_format[(int)(code)]) ;

  for (i = 0; i < 	(rtx_length[(int)(code)]) ; i++)
    {
      switch (*format_ptr++)
	{
	case 'e':
	  ((x)->fld[ i].rtx)  = copy_for_inline (((x)->fld[ i].rtx) );
	  break;

	case 'u':

	  return insn_map[((((x)->fld[ i].rtx) )->fld[0].rtint) ];

	case 'E':
	  if (((x)->fld[ i].rtvec)  != 0  && ((x)->fld[ i].rtvec->num_elem)  != 0)
	    {
	      register int j;

	      ((x)->fld[ i].rtvec)  = gen_rtvec_v (((x)->fld[ i].rtvec->num_elem) , &((x)->fld[ i].rtvec->elem[ 0].rtx) );
	      for (j = 0; j < ((x)->fld[ i].rtvec->num_elem) ; j++)
		((x)->fld[ i].rtvec->elem[ j].rtx) 
		  = copy_for_inline (((x)->fld[ i].rtvec->elem[ j].rtx) );
	    }
	  break;
	}
    }

  if (code == ASM_OPERANDS && orig_asm_operands_vector == 0)
    {
      orig_asm_operands_vector = ((orig)->fld[ 3].rtvec) ;
      copy_asm_operands_vector = ((x)->fld[ 3].rtvec) ;
    }

  return x;
}

rtx
expand_inline_function (fndecl, parms, target, ignore, type, structure_value_addr)
     tree fndecl, parms;
     rtx target;
     int ignore;
     tree type;
     rtx structure_value_addr;
{
  tree formal, actual;
  rtx header = ((fndecl)->decl.saved_insns) ;
  rtx insns = ((header)->fld[2].rtx) ;
  rtx insn;
  int max_regno = ((header)->fld[7].rtint)  + 1;
  register int i;
  int min_labelno = ((header)->fld[4].rtint) ;
  int max_labelno = ((header)->fld[5].rtint) ;
  int nargs;
  rtx *arg_vec;
  rtx local_return_label = 0;
  rtx follows_call = 0;
  rtx this_struct_value_rtx = 0;

  if (max_regno < 56 )
    abort ();

  nargs = list_length (((fndecl)->decl.arguments)  );

  first_parm_offset = 8 ;

  if (list_length (parms) != nargs)
    return (rtx)-1;

  for (formal = ((fndecl)->decl.arguments)  ,
       actual = parms;
       formal;
       formal = ((formal)->common.chain) ,
       actual = ((actual)->common.chain) )
    {
      tree arg = ((actual)->list.value) ;
      enum machine_mode mode = ((((formal)->decl.arguments)   )->type.mode) ;
      if (mode != ((((arg)->common.type) )->type.mode) )
	return (rtx)-1;
      if (mode == BLKmode && ((arg)->common.type)  != ((formal)->common.type) )
	return (rtx)-1;
    }

  pushlevel (0);
  expand_start_bindings (0);

  arg_vec = (rtx *)__builtin_alloca  (nargs * sizeof (rtx));

  for (formal = ((fndecl)->decl.arguments)  ,
       actual = parms,
       i = 0;
       formal;
       formal = ((formal)->common.chain) ,
       actual = ((actual)->common.chain) ,
       i++)
    {
      tree arg = ((actual)->list.value) ;  
      enum machine_mode tmode = ((((formal)->decl.arguments)   )->type.mode) ;
      enum machine_mode imode = ((((formal)->common.type) )->type.mode) ;
      rtx copy;

      emit_note (((formal)->decl.filename) , ((formal)->decl.linenum) );

      if (((formal)->common.addressable_attr) )
	{
	  int size = int_size_in_bytes (((formal)->common.type) );
	  copy = assign_stack_local (tmode, size);
	  if (!memory_address_p (((formal)->decl.mode) , ((copy)->fld[ 0].rtx) ))
	    copy = change_address (copy, VOIDmode, copy_rtx (((copy)->fld[ 0].rtx) ));
	  store_expr (arg, copy, 0);
	}
      else if (! ((formal)->common.readonly_attr) 
	       || ((formal)->common.volatile_attr) )
	{

	  copy = gen_reg_rtx (tmode);
	  store_expr (arg, copy, 0);
	}
      else
	{
	  copy = expand_expr (arg, 0, tmode, 0);

	  if (	((copy)->code)  != REG && ! (	((copy)->code)  == LABEL_REF || 	((copy)->code)  == SYMBOL_REF	|| 	((copy)->code)  == CONST_INT	|| 	((copy)->code)  == CONST) )
	    copy = copy_to_reg (copy);
	}

      if (tmode != imode)
	copy = convert_to_mode (imode, copy);
      arg_vec[i] = copy;
    }

  copy_parm_decls (((fndecl)->decl.arguments)  , arg_vec);

  emit_queue ();

  do_pending_stack_adjust ();

  if (structure_value_addr)
    {
      if (	((struct_value_rtx)->code)  == MEM)
	{
	  this_struct_value_rtx = force_reg (SImode , structure_value_addr);
	}
      else
	{
	  this_struct_value_rtx = struct_value_rtx;
	  emit_move_insn (this_struct_value_rtx, structure_value_addr);
	}
    }

  reg_map = (rtx *) __builtin_alloca  (max_regno * sizeof (rtx));
  memset (reg_map,0, max_regno * sizeof (rtx)) ;

  if (((fndecl)->decl.arguments)  )
    {
      tree decl = ((fndecl)->decl.arguments)  ;
      int offset = ((header)->fld[8].rtint) ;

      parm_map =
	(rtx *)__builtin_alloca  ((offset / 4 ) * sizeof (rtx));
      memset (parm_map,0, (offset / 4 ) * sizeof (rtx)) ;
      parm_map -= first_parm_offset / 4 ;

      for (formal = decl, i = 0; formal; formal = ((formal)->common.chain) , i++)
	{

	  if (((formal)->decl.offset)  >= 0)
	    {
	      parm_map[((formal)->decl.offset)  / 32 ] = arg_vec[i];
	    }
	  else
	    {

	      rtx frtx = ((formal)->decl.rtl) ;
	      rtx offset = 0;
	      if (	((frtx)->code)  == MEM)
		{
		  frtx = ((frtx)->fld[ 0].rtx) ;
		  if (	((frtx)->code)  == PLUS)
		    {
		      if (((frtx)->fld[ 0].rtx)  == frame_pointer_rtx
			  && 	((((frtx)->fld[ 1].rtx) )->code)  == CONST_INT)
			offset = ((frtx)->fld[ 1].rtx) ;
		      else if (((frtx)->fld[ 1].rtx)  == frame_pointer_rtx
			       && 	((((frtx)->fld[ 0].rtx) )->code)  == CONST_INT)
			offset = ((frtx)->fld[ 0].rtx) ;
		    }
		  if (offset)
		    parm_map[((offset)->fld[0].rtint)  / 4 ] = arg_vec[i];
		  else abort ();
		}
	      else if (	((frtx)->code)  != REG)
		abort ();
	    }

	  if (	((((formal)->decl.rtl) )->code)  == REG)
	    reg_map[((((formal)->decl.rtl) )->fld[0].rtint) ] = arg_vec[i];
	}

      if (this_struct_value_rtx == 0
	  || 	((struct_value_incoming_rtx)->code)  == REG)
	;
      else if (	((struct_value_incoming_rtx)->code)  == MEM
	       && ((((struct_value_incoming_rtx)->fld[ 0].rtx) )->fld[ 0].rtx)  == frame_pointer_rtx
	       && 	((((((struct_value_incoming_rtx)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT)
	parm_map[((((((struct_value_incoming_rtx)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  / 4 ]
	  = this_struct_value_rtx;
      else
	abort ();
    }
  else
    {
      parm_map = 0 ;
    }

  label_map = (rtx *)__builtin_alloca  ((max_labelno - min_labelno) * sizeof (rtx));
  label_map -= min_labelno;

  for (i = min_labelno; i < max_labelno; i++)
    label_map[i] = gen_label_rtx ();

  insn_map = (rtx *) __builtin_alloca  (((header)->fld[0].rtint)  * sizeof (rtx));
  memset (insn_map,0, ((header)->fld[0].rtint)  * sizeof (rtx)) ;

  if (structure_value_addr != 0 || ((type)->type.mode)  == VOIDmode)
    inline_target = 0;
  else
    {
      enum machine_mode departing_mode = ((type)->type.mode) ;

      enum machine_mode arriving_mode
	= ((((fndecl)->decl.voffset) )->type.mode) ;

      if (target && 	((target)->code)  == REG
	  && 	((target)->mode)  == departing_mode)
	inline_target = target;
      else
	inline_target = target = gen_reg_rtx (departing_mode);

      if (arriving_mode != departing_mode)
	inline_target = gen_rtx (SUBREG, arriving_mode, target, 0);
    }

  fp_delta = get_frame_size ();

  fp_delta = - fp_delta;

  fp_delta -= 0 ;

  inline_fp_rtx
    = copy_to_mode_reg (SImode ,
			plus_constant (frame_pointer_rtx, fp_delta));

  assign_stack_local (VOIDmode, ((fndecl)->decl.frame_size) );

  for (insn = insns; insn; insn = ((insn)->fld[2].rtx) )
    {
      rtx copy, pattern, next = 0;

      orig_asm_operands_vector = 0;
      copy_asm_operands_vector = 0;

      switch (	((insn)->code) )
	{
	case INSN:
	  pattern = ((insn)->fld[3].rtx) ;

	  if (follows_call

	      && ! (	((pattern)->code)  == SET
		    && ((pattern)->fld[0].rtx)  == stack_pointer_rtx))
	    {
	      if (	((pattern)->code)  == SET
		  && rtx_equal_p (((pattern)->fld[1].rtx) , follows_call))

		{
		  copy = emit_insn (gen_rtx (SET, VOIDmode,
					     copy_rtx_and_substitute (((pattern)->fld[0].rtx) ),
					     follows_call));
		  ((copy)->integrated)  = 1;
		  follows_call = 0;
		  break;
		}
	      else if (	((pattern)->code)  == USE
		       && rtx_equal_p (((pattern)->fld[ 0].rtx) , follows_call))

		{
		  copy = emit_insn (gen_rtx (SET, VOIDmode, inline_target,
					     follows_call));
		  ((copy)->integrated)  = 1;
		  follows_call = 0;
		  break;
		}
	      follows_call = 0;
	    }

	  copy = 0;
	  if (	((pattern)->code)  == USE
	      && 	((((pattern)->fld[ 0].rtx) )->code)  == REG
	      && ((((pattern)->fld[ 0].rtx) )->integrated) )
	    break;

	  if (	((pattern)->code)  == SET
	      && ((pattern)->fld[0].rtx)  == cc0_rtx)
	    next = try_fold_cc0 (insn);

	  if (next != 0)
	    {
	      insn = next;
	    }
	  else
	    {
	      copy = emit_insn (copy_rtx_and_substitute (pattern));
	      ((copy)->integrated)  = 1;
	    }
	  break;

	case JUMP_INSN:
	  follows_call = 0;
	  if (	((((insn)->fld[3].rtx) )->code)  == RETURN)
	    {
	      if (local_return_label == 0)
		local_return_label = gen_label_rtx ();
	      emit_jump (local_return_label);
	      break;
	    }
	  copy = emit_jump_insn (copy_rtx_and_substitute (((insn)->fld[3].rtx) ));
	  ((copy)->integrated)  = 1;
	  break;

	case CALL_INSN:

	  copy = emit_call_insn (copy_rtx_and_substitute (((insn)->fld[3].rtx) ));

	  ((copy)->integrated)  = 1;

	  if (	((((insn)->fld[3].rtx) )->code)  == SET)
	    follows_call = ((((insn)->fld[3].rtx) )->fld[0].rtx) ;
	  break;

	case CODE_LABEL:
	  copy = emit_label (label_map[((insn)->fld[3].rtint) ]);
	  follows_call = 0;
	  break;

	case BARRIER:
	  copy = emit_barrier ();
	  break;

	case NOTE:
	  if (((insn)->fld[4].rtint)  != -6 )
	    copy = emit_note ( ((insn)->fld[3].rtstr) , ((insn)->fld[4].rtint) );
	  else
	    copy = 0;
	  break;

	default:
	  abort ();
	  break;
	}

      insn_map[((insn)->fld[0].rtint) ] = copy;
    }

  if (local_return_label)
    emit_label (local_return_label);

  copy_decl_tree (((fndecl)->decl.initial) , 0);

  expand_end_bindings (getdecls (), 1, 1);
  poplevel (1, 1, 0);

  reg_map = 0 ;
  label_map = 0 ;

  if (ignore || ((type)->type.mode)  == VOIDmode)
    return 0;

  if (structure_value_addr)
    {
      if (target)
	return target;
      return gen_rtx (MEM, BLKmode,
		      memory_address (BLKmode, structure_value_addr));
    }

  return target;
}

static void
copy_parm_decls (args, vec)
     tree args;
     rtx *vec;
{
  register tree tail;
  register int i;

  for (tail = args, i = 0; tail; tail = ((tail)->common.chain) , i++)
    {
      register tree decl = pushdecl (build_decl (VAR_DECL, ((tail)->decl.name) ,
						 ((tail)->common.type) ));
      ((decl)->common.used_attr)  = 1;
      ((decl)->decl.rtl)  = vec[i];
    }
}

static void
copy_decl_tree (let, level)
     tree let;
     int level;
{
  tree t;

  pushlevel (0);
  for (t = ((let)->bind_stmt.vars) ; t; t = ((t)->common.chain) )
    {
      tree d = build_decl (((t)->common.code) , ((t)->decl.name) , ((t)->common.type) );
      ((d)->decl.linenum)  = ((t)->decl.linenum) ;
      ((d)->decl.filename)  = ((t)->decl.filename) ;
      if (((t)->decl.rtl)  != 0)
	{
	  if (	((((t)->decl.rtl) )->code)  == MEM
	      &&  (	((((((t)->decl.rtl) )->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((((t)->decl.rtl) )->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((((t)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST_INT	|| 	((((((t)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST)  )

	    ((d)->decl.rtl)  = ((t)->decl.rtl) ;
	  else
	    ((d)->decl.rtl)  = copy_rtx_and_substitute (((t)->decl.rtl) );
	}
      ((d)->common.external_attr)  = ((t)->common.external_attr) ;
      ((d)->common.static_attr)  = ((t)->common.static_attr) ;
      ((d)->common.public_attr)  = ((t)->common.public_attr) ;
      ((d)->common.literal_attr)  = ((t)->common.literal_attr) ;
      ((d)->common.addressable_attr)  = ((t)->common.addressable_attr) ;
      ((d)->common.readonly_attr)  = ((t)->common.readonly_attr) ;
      ((d)->common.volatile_attr)  = ((t)->common.volatile_attr) ;
      ((d)->common.used_attr)  = 1;
      pushdecl (d);
    }

  for (t = ((let)->stmt.body) ; t; t = ((t)->common.chain) )
    copy_decl_tree (t, level + 1);

  poplevel (level > 0, 0, 0);
}

static rtx
copy_rtx_and_substitute (orig)
     register rtx orig;
{
  register rtx copy, temp;
  register int i, j;
  register enum rtx_code  code;
  register enum machine_mode mode;
  register char *format_ptr;
  int regno;

  if (orig == 0)
    return 0;

  code = 	((orig)->code) ;
  mode = 	((orig)->mode) ;

  switch (code)
    {
    case REG:

      regno = ((orig)->fld[0].rtint) ;
      if (regno < 56 )
	{
	  if (((orig)->integrated) )
	    {

	      if (inline_target == 0)
		abort ();
	      if (mode == 	((inline_target)->mode) )
		return inline_target;
	      return gen_rtx (SUBREG, mode, inline_target, 0);
	    }
	  if (regno == 14 )
	    return plus_constant (orig, fp_delta);
	  return orig;
	}
      if (reg_map[regno] == 0 )
	reg_map[regno] = gen_reg_rtx (mode);
      return reg_map[regno];

    case CODE_LABEL:
      return label_map[((orig)->fld[3].rtint) ];

    case LABEL_REF:
      copy = rtx_alloc (LABEL_REF);
      ((copy)->mode = ( mode)) ;
      ((copy)->fld[ 0].rtx)  = label_map[((((orig)->fld[ 0].rtx) )->fld[3].rtint) ];
      return copy;

    case PC:
    case CC0:
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
      return orig;

    case ASM_OPERANDS:

      if (orig_asm_operands_vector == ((orig)->fld[ 3].rtvec) )
	{
	  copy = rtx_alloc (ASM_OPERANDS);
	  ((copy)->fld[ 0].rtstr)  = ((orig)->fld[ 0].rtstr) ;
	  ((copy)->fld[ 1].rtstr)  = ((orig)->fld[ 1].rtstr) ;
	  ((copy)->fld[ 2].rtint)  = ((orig)->fld[ 2].rtint) ;
	  ((copy)->fld[ 3].rtvec)  = copy_asm_operands_vector;
	  ((copy)->fld[ 4].rtvec)  = ((orig)->fld[ 4].rtvec) ;
	  return copy;
	}
      break;

    case CALL:

      if (! (optimize && ! flag_no_function_cse))

	return gen_rtx (CALL, 	((orig)->mode) ,
			gen_rtx (MEM, 	((((orig)->fld[ 0].rtx) )->mode) ,
				 copy_rtx_and_substitute (((((orig)->fld[ 0].rtx) )->fld[ 0].rtx) )),
			copy_rtx_and_substitute (((orig)->fld[ 1].rtx) ));
      break;

    case PLUS:

      if (((orig)->fld[ 0].rtx)  == frame_pointer_rtx
	  || ((orig)->fld[ 1].rtx)  == frame_pointer_rtx
	  || (14  != 14 
	      && (((orig)->fld[ 0].rtx)  == arg_pointer_rtx
		  || ((orig)->fld[ 1].rtx)  == arg_pointer_rtx)))
	{
	  if (((orig)->fld[ 0].rtx)  == frame_pointer_rtx
	      || ((orig)->fld[ 0].rtx)  == arg_pointer_rtx)
	    copy = ((orig)->fld[ 1].rtx) ;
	  else
	    copy = ((orig)->fld[ 0].rtx) ;

	  if (	((copy)->code)  == CONST_INT)
	    {
	      int c = ((copy)->fld[0].rtint) ;

	      if (c > 0)
		{
		  copy = parm_map[c / 4 ];
		  return ((copy)->fld[ 0].rtx) ;
		}
	      return gen_rtx (PLUS, mode,
			      frame_pointer_rtx,
			      gen_rtx (CONST_INT, SImode,
				       c + fp_delta));
	    }
	  copy = copy_rtx_and_substitute (copy);
	  temp = force_reg (mode, gen_rtx (PLUS, mode, frame_pointer_rtx, copy));
	  return plus_constant (temp, fp_delta);
	}
      else if (reg_mentioned_p (frame_pointer_rtx, orig)
	       || (14  != 14 
		   && reg_mentioned_p (arg_pointer_rtx, orig)))
	{

	  if (memory_address_p (mode, orig))
	    {
	      if (	((((orig)->fld[ 0].rtx) )->code)  == CONST_INT)
		{
		  copy = copy_rtx_and_substitute (((orig)->fld[ 1].rtx) );
		  temp = plus_constant (copy, ((((orig)->fld[ 0].rtx) )->fld[0].rtint) );
		}
	      else if (	((((orig)->fld[ 1].rtx) )->code)  == CONST_INT)
		{
		  copy = copy_rtx_and_substitute (((orig)->fld[ 0].rtx) );
		  temp = plus_constant (copy, ((((orig)->fld[ 1].rtx) )->fld[0].rtint) );
		}
	      else
		{
		  temp = gen_rtx (PLUS, 	((orig)->mode) ,
				  copy_rtx_and_substitute (((orig)->fld[ 0].rtx) ),
				  copy_rtx_and_substitute (((orig)->fld[ 1].rtx) ));
		}
	      temp = memory_address (mode, temp);
	    }
	  else
	    temp = gen_rtx (PLUS, 	((orig)->mode) ,
			    copy_rtx_and_substitute (((orig)->fld[ 0].rtx) ),
			    copy_rtx_and_substitute (((orig)->fld[ 1].rtx) ));
	}
      else
	temp = gen_rtx (PLUS, 	((orig)->mode) ,
			copy_rtx_and_substitute (((orig)->fld[ 0].rtx) ),
			copy_rtx_and_substitute (((orig)->fld[ 1].rtx) ));

      return temp;

    case MEM:
      copy = ((orig)->fld[ 0].rtx) ;
      if (copy == frame_pointer_rtx || copy == arg_pointer_rtx)
	return gen_rtx (MEM, mode,
			plus_constant (frame_pointer_rtx, fp_delta));

      if (	((copy)->code)  == PRE_DEC && ((copy)->fld[ 0].rtx)  == stack_pointer_rtx)
	return gen_rtx (MEM, mode, copy_rtx_and_substitute (copy));

      if (! memory_address_p (mode, copy))
	return gen_rtx (MEM, mode, copy_address (copy));

      if (	((copy)->code)  == PLUS)
	{
	  if (((copy)->fld[ 0].rtx)  == frame_pointer_rtx
	      || ((copy)->fld[ 1].rtx)  == frame_pointer_rtx
	      || (14  != 14 
		  && (((copy)->fld[ 0].rtx)  == arg_pointer_rtx
		      || ((copy)->fld[ 1].rtx)  == arg_pointer_rtx)))
	    {
	      rtx reg;
	      if (((copy)->fld[ 0].rtx)  == frame_pointer_rtx
		  || ((copy)->fld[ 0].rtx)  == arg_pointer_rtx)
		reg = ((copy)->fld[ 0].rtx) , copy = ((copy)->fld[ 1].rtx) ;
	      else
		reg = ((copy)->fld[ 1].rtx) , copy = ((copy)->fld[ 0].rtx) ;

	      if (	((copy)->code)  == CONST_INT)
		{
		  int c = ((copy)->fld[0].rtint) ;

		  if (reg == arg_pointer_rtx && c >= first_parm_offset)
		    {
		      int index = c / 4 ;
		      int offset = c % 4 ;

		      while (parm_map[index] == 0)
			{
			  index--;
			  if (index < first_parm_offset / 4 )

			    abort ();
			  offset += 4 ;
			}

		      copy = parm_map[index];

		      if (	(mode_size[(int)(	((copy)->mode) )])  < 4 )
			offset
			  -= (4 
			      - 	(mode_size[(int)(	((copy)->mode) )]) );

		      if ((	((copy)->mode)  != mode
			   && 	((copy)->mode)  != VOIDmode))
			{
			  if (	((copy)->code)  == MEM)
			    return change_address (copy, mode,
						   plus_constant (((copy)->fld[ 0].rtx) ,
								  offset));
			  if (	((copy)->code)  == REG)
			    {

			      if (offset + 	(mode_size[(int)(mode)]) 
				  != 	(mode_size[(int)(	((copy)->mode) )]) )
				abort ();

			      return gen_rtx (SUBREG, mode, copy, 0);
			    }

			  abort ();
			}
		      return copy;
		    }
		  temp = gen_rtx (PLUS, SImode ,
				  frame_pointer_rtx,
				  gen_rtx (CONST_INT, SImode,
					   c + fp_delta));
		  if (! memory_address_p (SImode , temp))
		    return gen_rtx (MEM, mode, plus_constant (inline_fp_rtx, c));
		}
	      copy =  copy_rtx_and_substitute (copy);
	      temp = gen_rtx (PLUS, SImode , frame_pointer_rtx, copy);
	      temp = plus_constant (temp, fp_delta);
	      temp = memory_address (SImode , temp);
	    }
	  else if (reg_mentioned_p (frame_pointer_rtx, copy)
		   || (14  != 14 
		       && reg_mentioned_p (arg_pointer_rtx, copy)))
	    {
	      if (	((((copy)->fld[ 0].rtx) )->code)  == CONST_INT)
		{
		  temp = copy_rtx_and_substitute (((copy)->fld[ 1].rtx) );
		  temp = plus_constant (temp, ((((copy)->fld[ 0].rtx) )->fld[0].rtint) );
		}
	      else if (	((((copy)->fld[ 1].rtx) )->code)  == CONST_INT)
		{
		  temp = copy_rtx_and_substitute (((copy)->fld[ 0].rtx) );
		  temp = plus_constant (temp, ((((copy)->fld[ 1].rtx) )->fld[0].rtint) );
		}
	      else
		{
		  temp = gen_rtx (PLUS, 	((copy)->mode) ,
				  copy_rtx_and_substitute (((copy)->fld[ 0].rtx) ),
				  copy_rtx_and_substitute (((copy)->fld[ 1].rtx) ));
		}
	    }
	  else
	    {
	      if (	((((copy)->fld[ 1].rtx) )->code)  == CONST_INT)
		temp = plus_constant (copy_rtx_and_substitute (((copy)->fld[ 0].rtx) ),
				      ((((copy)->fld[ 1].rtx) )->fld[0].rtint) );
	      else if (	((((copy)->fld[ 0].rtx) )->code)  == CONST_INT)
		temp = plus_constant (copy_rtx_and_substitute (((copy)->fld[ 1].rtx) ),
				      ((((copy)->fld[ 0].rtx) )->fld[0].rtint) );
	      else
		{
		  rtx left = copy_rtx_and_substitute (((copy)->fld[ 0].rtx) );
		  rtx right = copy_rtx_and_substitute (((copy)->fld[ 1].rtx) );

		  temp = gen_rtx (PLUS, 	((copy)->mode) , left, right);
		}
	    }
	}
      else
	temp = copy_rtx_and_substitute (copy);

      return change_address (orig, mode, temp);

    case RETURN:
      abort ();
    }

  copy = rtx_alloc (code);
  ((copy)->mode = ( mode)) ;
  copy->in_struct = orig->in_struct;
  copy->volatil = orig->volatil;
  copy->unchanging = orig->unchanging;

  format_ptr = 	(rtx_format[(int)(	((copy)->code) )]) ;

  for (i = 0; i < 	(rtx_length[(int)(	((copy)->code) )]) ; i++)
    {
      switch (*format_ptr++)
	{
	case '0':
	  break;

	case 'e':
	  ((copy)->fld[ i].rtx)  = copy_rtx_and_substitute (((orig)->fld[ i].rtx) );
	  break;

	case 'u':

	  ((copy)->fld[ i].rtx)  = insn_map[((((orig)->fld[ i].rtx) )->fld[0].rtint) ];
	  break;

	case 'E':
	  ((copy)->fld[ i].rtvec)  = ((orig)->fld[ i].rtvec) ;
	  if (((orig)->fld[ i].rtvec)  != 0  && ((orig)->fld[ i].rtvec->num_elem)  != 0)
	    {
	      ((copy)->fld[ i].rtvec)  = rtvec_alloc (((orig)->fld[ i].rtvec->num_elem) );
	      for (j = 0; j < ((copy)->fld[ i].rtvec->num_elem) ; j++)
		((copy)->fld[ i].rtvec->elem[ j].rtx)  = copy_rtx_and_substitute (((orig)->fld[ i].rtvec->elem[ j].rtx) );
	    }
	  break;

	case 'i':
	  ((copy)->fld[ i].rtint)  = ((orig)->fld[ i].rtint) ;
	  break;

	case 's':
	  ((copy)->fld[ i].rtstr)  = ((orig)->fld[ i].rtstr) ;
	  break;

	default:
	  abort ();
	}
    }

  if (code == ASM_OPERANDS && orig_asm_operands_vector == 0)
    {
      orig_asm_operands_vector = ((orig)->fld[ 3].rtvec) ;
      copy_asm_operands_vector = ((copy)->fld[ 3].rtvec) ;
    }

  return copy;
}

static rtx
copy_address (orig)
     register rtx orig;
{
  register rtx copy;
  register int i, j;
  register enum rtx_code  code;
  register enum machine_mode mode;
  register char *format_ptr;

  if (orig == 0)
    return 0;

  code = 	((orig)->code) ;
  mode = 	((orig)->mode) ;

  switch (code)
    {
    case REG:
      if (((orig)->fld[0].rtint)  != 14 )
	return copy_rtx_and_substitute (orig);
      return plus_constant (frame_pointer_rtx, fp_delta);

    case PLUS:
      if (	((((orig)->fld[ 0].rtx) )->code)  == REG
	  && ((((orig)->fld[ 0].rtx) )->fld[0].rtint)  == 14 )
	return plus_constant (orig, fp_delta);
      break;

    case MEM:
      return copy_to_reg (copy_rtx_and_substitute (orig));

    case CODE_LABEL:
    case LABEL_REF:
      return copy_rtx_and_substitute (orig);

    case PC:
    case CC0:
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
      return orig;
    }

  copy = rtx_alloc (code);
  ((copy)->mode = ( mode)) ;
  copy->in_struct = orig->in_struct;
  copy->volatil = orig->volatil;
  copy->unchanging = orig->unchanging;

  format_ptr = 	(rtx_format[(int)(	((copy)->code) )]) ;

  for (i = 0; i < 	(rtx_length[(int)(	((copy)->code) )]) ; i++)
    {
      switch (*format_ptr++)
	{
	case '0':
	  break;

	case 'e':
	  ((copy)->fld[ i].rtx)  = copy_rtx_and_substitute (((orig)->fld[ i].rtx) );
	  break;

	case 'u':

	  ((copy)->fld[ i].rtx)  = insn_map[((((orig)->fld[ i].rtx) )->fld[0].rtint) ];
	  break;

	case 'E':
	  ((copy)->fld[ i].rtvec)  = ((orig)->fld[ i].rtvec) ;
	  if (((orig)->fld[ i].rtvec)  != 0  && ((orig)->fld[ i].rtvec->num_elem)  != 0)
	    {
	      ((copy)->fld[ i].rtvec)  = rtvec_alloc (((orig)->fld[ i].rtvec->num_elem) );
	      for (j = 0; j < ((copy)->fld[ i].rtvec->num_elem) ; j++)
		((copy)->fld[ i].rtvec->elem[ j].rtx)  = copy_rtx_and_substitute (((orig)->fld[ i].rtvec->elem[ j].rtx) );
	    }
	  break;

	case 'i':
	  ((copy)->fld[ i].rtint)  = ((orig)->fld[ i].rtint) ;
	  break;

	case 's':
	  ((copy)->fld[ i].rtstr)  = ((orig)->fld[ i].rtstr) ;
	  break;

	default:
	  abort ();
	}
    }
  return copy;
}

static rtx
try_fold_cc0 (insn)
     rtx insn;
{
  rtx cnst = copy_rtx_and_substitute (((((insn)->fld[3].rtx) )->fld[1].rtx) );
  rtx pat, copy;

  if ((	((cnst)->code)  == LABEL_REF || 	((cnst)->code)  == SYMBOL_REF	|| 	((cnst)->code)  == CONST_INT	|| 	((cnst)->code)  == CONST) 
      && ((insn)->fld[2].rtx) 
      && 	((pat = ((((insn)->fld[2].rtx) )->fld[3].rtx) )->code)  == SET
      && ((pat)->fld[0].rtx)  == pc_rtx
      && 	((pat = ((pat)->fld[1].rtx) )->code)  == IF_THEN_ELSE
      && 	(rtx_length[(int)(	((((pat)->fld[ 0].rtx) )->code) )])  == 2)
    {
      rtx cnst2;
      rtx cond = ((pat)->fld[ 0].rtx) ;

      if ((((cond)->fld[ 0].rtx)  == cc0_rtx
	   && (	((((cond)->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((cond)->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((cond)->fld[ 1].rtx) )->code)  == CONST_INT	|| 	((((cond)->fld[ 1].rtx) )->code)  == CONST) 
	   && (cnst2 = ((cond)->fld[ 1].rtx) ))
	  || (((cond)->fld[ 1].rtx)  == cc0_rtx
	      && (	((((cond)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((cond)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((cond)->fld[ 0].rtx) )->code)  == CONST_INT	|| 	((((cond)->fld[ 0].rtx) )->code)  == CONST) 
	      && (cnst2 = ((cond)->fld[ 0].rtx) )))
	{
	  copy = fold_out_const_cc0 (cond, ((pat)->fld[ 1].rtx) , ((pat)->fld[ 2].rtx) ,
				     cnst, cnst2);
	  if (copy)
	    {
	      if (	((copy)->code)  == LABEL_REF)
		{

		  rtx tmp = ((insn)->fld[2].rtx) ;
		  while (tmp && 	((tmp)->code)  != CODE_LABEL)
		    tmp = ((tmp)->fld[2].rtx) ;
		  if (! tmp)
		    abort ();
		  if (label_map[((tmp)->fld[3].rtint) ] == ((copy)->fld[ 0].rtx) )
		    {
		      return ((tmp)->fld[1].rtx) ;
		    }
		  else
		    {

		      emit_jump (copy);
		      return ((insn)->fld[2].rtx) ;
		    }
		}
	      else if (copy == pc_rtx)
		{

		  return ((insn)->fld[2].rtx) ;
		}
	      else
		abort ();
	    }
	}
    }
  return 0;
}

static rtx
fold_out_const_cc0 (cond_rtx, then_rtx, else_rtx, cnst1, cnst2)
     rtx cond_rtx, then_rtx, else_rtx;
     rtx cnst1, cnst2;
{
  int value1, value2;
  int int1 = 	((cnst1)->code)  == CONST_INT;
  int int2 = 	((cnst2)->code)  == CONST_INT;
  if (int1)
    value1 = ((cnst1)->fld[0].rtint) ;
  else
    value1 = 1;
  if (int2)
    value2 = ((cnst2)->fld[0].rtint) ;
  else
    value2 = 1;

  switch (	((cond_rtx)->code) )
    {
    case NE:
      if (int1 && int2)
	if (value1 != value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0 || value2 == 0)
	return copy_rtx_and_substitute (then_rtx);
      if (int1 == 0 && int2 == 0)
	if (rtx_equal_p (cnst1, cnst2))
	  return copy_rtx_and_substitute (else_rtx);
      break;
    case EQ:
      if (int1 && int2)
	if (value1 == value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0 || value2 == 0)
	return copy_rtx_and_substitute (else_rtx);
      if (int1 == 0 && int2 == 0)
	if (rtx_equal_p (cnst1, cnst2))
	  return copy_rtx_and_substitute (then_rtx);
      break;
    case GE:
      if (int1 && int2)
	if (value1 >= value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (else_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (then_rtx);
      break;
    case GT:
      if (int1 && int2)
	if (value1 > value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (else_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (then_rtx);
      break;
    case LE:
      if (int1 && int2)
	if (value1 <= value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (then_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (else_rtx);
      break;
    case LT:
      if (int1 && int2)
	if (value1 < value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (then_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (else_rtx);
      break;
    case GEU:
      if (int1 && int2)
	if ((unsigned)value1 >= (unsigned)value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (else_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (then_rtx);
      break;
    case GTU:
      if (int1 && int2)
	if ((unsigned)value1 > (unsigned)value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (else_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (then_rtx);
      break;
    case LEU:
      if (int1 && int2)
	if ((unsigned)value1 <= (unsigned)value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (then_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (else_rtx);
      break;
    case LTU:
      if (int1 && int2)
	if ((unsigned)value1 < (unsigned)value2)
	  return copy_rtx_and_substitute (then_rtx);
	else
	  return copy_rtx_and_substitute (else_rtx);
      if (value1 == 0)
	return copy_rtx_and_substitute (then_rtx);
      if (value2 == 0)
	return copy_rtx_and_substitute (else_rtx);
      break;
    }
  return 0;
}

void
output_inline_function (fndecl)
     tree fndecl;
{
  rtx head = ((fndecl)->decl.saved_insns) ;
  rtx last;

  temporary_allocation ();

  current_function_decl = fndecl;

  expand_function_start (fndecl);

  assign_stack_local (BLKmode, ((fndecl)->decl.frame_size) );

  restore_reg_data (((head)->fld[3].rtx) );

  expand_function_end (((fndecl)->decl.filename) , ((fndecl)->decl.linenum) );

  for (last = head; ((last)->fld[2].rtx) ; last = ((last)->fld[2].rtx) )
    ;

  set_new_first_and_last_insn (((head)->fld[3].rtx) , last);

  rest_of_compilation (fndecl);

  current_function_decl = 0;

  permanent_allocation ();
}

