
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

typedef long HARD_REG_SET[((56  + 32  - 1) / 32 ) ];

extern char fixed_regs[56 ];

extern HARD_REG_SET fixed_reg_set;

extern char call_used_regs[56 ];

extern HARD_REG_SET call_used_reg_set;

extern char call_fixed_regs[56 ];

extern HARD_REG_SET call_fixed_reg_set;

extern char global_regs[56 ];

extern int reg_alloc_order[56 ];

extern HARD_REG_SET reg_class_contents[];

extern int reg_class_size[(int) LIM_REG_CLASSES ];

extern enum reg_class reg_class_superclasses[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

extern enum reg_class reg_class_subclasses[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

extern enum reg_class reg_class_subunion[(int) LIM_REG_CLASSES ][(int) LIM_REG_CLASSES ];

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

extern struct _iobuf  *asm_out_file;

extern struct obstack *current_obstack;
extern struct obstack *saveable_obstack;
extern struct obstack permanent_obstack;

extern int xmalloc ();

int const_labelno;

int var_labelno;

static int function_defined;

extern struct _iobuf  *asm_out_file;

static char *compare_constant_1 ();
static void record_constant_1 ();
void assemble_name ();
void output_addressed_constants ();
void output_constant ();
void output_constructor ();
static enum in_section {no_section, in_text, in_data} in_section = no_section;

void
text_section ()
{
  if (in_section != in_text)
    {
      fprintf (asm_out_file, "%s\n", ".text" );
      in_section = in_text;
    }
}

void
data_section ()
{
  if (in_section != in_data)
    {
      if (flag_shared_data)
	{

	  fprintf (asm_out_file, "%s\n", ".data" );

	}
      else
	fprintf (asm_out_file, "%s\n", ".data" );

      in_section = in_data;
    }
}

void
make_function_rtl (decl)
     tree decl;
{
  if (((decl)->decl.rtl)  == 0)
    ((decl)->decl.rtl) 
      = gen_rtx (MEM, ((decl)->decl.mode) ,
		 gen_rtx (SYMBOL_REF, SImode ,
			  ((((decl)->decl.name) )->identifier.pointer) ));

  function_defined = 1;
}

void
make_decl_rtl (decl, asmspec, top_level)
     tree decl;
     tree asmspec;
     int top_level;
{
  register char *name = ((((decl)->decl.name) )->identifier.pointer) ;
  int reg_number = -1;

  if (asmspec != 0)
    {
      int i;
      extern char *reg_names[];

      if (((asmspec)->common.code)  != STRING_CST)
	abort ();
      for (i = 0; i < 56 ; i++)
	if (!strcmp (((asmspec)->string.pointer) ,
		     reg_names[i]))
	  break;

      if (i < 56 )
	reg_number = i;
      else
	{
	  name = (char *) ({ struct obstack *__h = (saveable_obstack);	({ struct obstack *__o = (__h);	int __len = ( (
					 strlen (((asmspec)->string.pointer) ) + 2));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
	  name[0] = '*';
	  strcpy (&name[1], ((asmspec)->string.pointer) );
	  reg_number = -2;
	}
    }

  if (((decl)->decl.rtl)  == 0
      || 	((((decl)->decl.rtl) )->mode)  != ((decl)->decl.mode) )
    {
      ((decl)->decl.rtl)  = 0;

      if (((decl)->common.regdecl_attr)  && reg_number == -1)
	error_with_decl (decl,
			 "register name not specified for `%s'");
      if (((decl)->common.regdecl_attr)  && reg_number == -2)
	error_with_decl (decl,
			 "invalid register name for `%s'");
      else if (reg_number >= 0 && ! ((decl)->common.regdecl_attr) )
	error_with_decl (decl,
			 "register name given for non-register variable `%s'");
      else if (((decl)->common.regdecl_attr)  && ((decl)->common.code)  == FUNCTION_DECL)
	error ("function declared `register'");
      else if (((decl)->common.regdecl_attr)  && ((((decl)->common.type) )->type.mode)  == BLKmode)
	error_with_decl (decl, "data type of `%s' isn't suitable for a register");
      else if (((decl)->common.regdecl_attr) )
	{
	  int nregs;
	  if (pedantic)
	    warning ("ANSI C forbids global register variables");
	  if (((decl)->decl.initial)  != 0)
	    {
	      ((decl)->decl.initial)  = 0;
	      error ("global register variable has initial value");
	    }
	  if (fixed_regs[reg_number] == 0
	      && function_defined)
	    error ("global register variable follows a function definition");
	  ((decl)->decl.rtl)  = gen_rtx (REG, ((decl)->decl.mode) , reg_number);
	  nregs = ((reg_number) >= 16 ? 1	: ((	(mode_size[(int)( ((decl)->decl.mode) )])  + 4  - 1) / 4 )) ;
	  while (nregs > 0)
	    global_regs[reg_number + --nregs] = 1;
	  init_reg_sets_1 ();
	}

      if (((decl)->decl.rtl)  == 0)
	{

	  if (!top_level && !((decl)->common.external_attr)  && asmspec == 0)
	    {
	      char *label;

	      ( (label) = (char *) __builtin_alloca  (strlen (( name)) + 10),	sprintf ((label), "%s.%d", ( name), ( var_labelno))) ;
	      name = ({ struct obstack *__h = (saveable_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( strlen (label)));	((__o->next_free + __len + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, __len + 1) : 0),	memcpy ( __o->next_free, ( label), __len) ,	__o->next_free += __len,	*(__o->next_free)++ = 0;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
	      var_labelno++;
	    }

	  ((decl)->decl.rtl)  = gen_rtx (MEM, ((decl)->decl.mode) ,
				     gen_rtx (SYMBOL_REF, SImode , name));
	  if (((decl)->common.volatile_attr) )
	    ((((decl)->decl.rtl) )->volatil)  = 1;
	  if (((decl)->common.readonly_attr) )
	    ((((decl)->decl.rtl) )->unchanging)  = 1;
	  ((((decl)->decl.rtl) )->in_struct) 
	    = (((((decl)->common.type) )->common.code)  == ARRAY_TYPE
	       || ((((decl)->common.type) )->common.code)  == RECORD_TYPE
	       || ((((decl)->common.type) )->common.code)  == UNION_TYPE);
	}
    }
}

void
assemble_asm (string)
     tree string;
{
  app_enable ();

  fprintf (asm_out_file, "\t%s\n", ((string)->string.pointer) );
}

void
assemble_function (decl)
     tree decl;
{
  rtx x, n;
  char *fnname;

  x = ((decl)->decl.rtl) ;
  if (	((x)->code)  != MEM)
    abort ();
  n = ((x)->fld[ 0].rtx) ;
  if (	((n)->code)  != SYMBOL_REF)
    abort ();
  fnname = ((n)->fld[ 0].rtstr) ;

  app_disable ();

  text_section ();

  if (( floor_log2 (16  / 8 )) == 1)	fprintf (asm_out_file, "\t.even\n");	else if (( floor_log2 (16  / 8 )) != 0)	abort (); ;

  if (((decl)->common.public_attr) )
    do { fputs (".globl ", asm_out_file); assemble_name (asm_out_file,  fnname); fputs ("\n", asm_out_file);} while (0) ;

  do { assemble_name (asm_out_file,  fnname); fputs (":\n", asm_out_file); } while (0) ;

}

void
assemble_integer_zero ()
{
  ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, ( const0_rtx)),	fprintf (asm_out_file, "\n")) ;
}

void
assemble_string (p, size)
     unsigned char *p;
     int size;
{
  register int i;
  int excess = 0;
  int pos = 0;
  int maximum = 2000;

  while (pos < size)
    {
      int thissize = size - pos;
      if (thissize > maximum)
	thissize = maximum;

      fprintf (asm_out_file, "\t.ascii \"");

      for (i = 0; i < thissize; i++)
	{
	  register int c = p[i];
	  if (c == '\"' || c == '\\')
	    (--( asm_out_file)->_cnt >= 0 ?	(int)(*( asm_out_file)->_ptr++ = (unsigned char)('\\')) :	((( asm_out_file)->_flag & 0200 ) && -( asm_out_file)->_cnt < ( asm_out_file)->_bufsiz ?	((*( asm_out_file)->_ptr = (unsigned char)('\\')) != '\n' ?	(int)(*( asm_out_file)->_ptr++) :	_flsbuf(*(unsigned char *)( asm_out_file)->_ptr,  asm_out_file)) :	_flsbuf((unsigned char)('\\'),  asm_out_file))) ;
	  if (c >= ' ' && c < 0177)
	    (--( asm_out_file)->_cnt >= 0 ?	(int)(*( asm_out_file)->_ptr++ = (unsigned char)(c)) :	((( asm_out_file)->_flag & 0200 ) && -( asm_out_file)->_cnt < ( asm_out_file)->_bufsiz ?	((*( asm_out_file)->_ptr = (unsigned char)(c)) != '\n' ?	(int)(*( asm_out_file)->_ptr++) :	_flsbuf(*(unsigned char *)( asm_out_file)->_ptr,  asm_out_file)) :	_flsbuf((unsigned char)(c),  asm_out_file))) ;
	  else
	    {
	      fprintf (asm_out_file, "\\%o", c);

	      if (i < thissize - 1
		  && p[i + 1] >= '0' && p[i + 1] <= '9')
		fprintf (asm_out_file, "\"\n\t.ascii \"");
	    }
	}
      fprintf (asm_out_file, "\"\n");

      pos += thissize;
      p += thissize;
    }
}

void
assemble_variable (decl, top_level, write_symbols, at_end)
     tree decl;
     int top_level;
     enum debugger write_symbols;
     int at_end;
{
  register char *name;
  register int i;

  if (	((((decl)->decl.rtl) )->code)  == REG)
    return;

  if (((decl)->common.external_attr) )
    return;

  if (((decl)->common.code)  == FUNCTION_DECL)
    return;

  if (((decl)->decl.size)  == 0)
    layout_decl (decl, 0);

  if (((decl)->decl.size)  == 0)
    {
      error_with_file_and_line (((decl)->decl.filename) ,
				((decl)->decl.linenum) ,
				"storage size of static var `%s' isn't known",
				((((decl)->decl.name) )->identifier.pointer) );
      return;
    }

  if (((decl)->common.asm_written_attr) )
    return;

  ((decl)->common.asm_written_attr)  = 1;

  if (write_symbols == DBX_DEBUG && top_level)
    dbxout_symbol (decl, 0);

  if (write_symbols == GDB_DEBUG)
    set_current_gdbfile (((decl)->decl.filename) );

  if (! ((((decl)->decl.size) )->common.literal_attr) )
    return;

  app_disable ();

  name = ((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[ 0].rtstr) ;

  if (((decl)->decl.initial)  == 0 || ((decl)->decl.initial)  == error_mark_node)
    {
      int size = (((((decl)->decl.size) )->int_cst.int_cst_low) 
		  * ((decl)->decl.size_unit) 
		  / 8 );
      int rounded = size;

      if (size == 0) rounded = 1;

      rounded = ((rounded + (16  / 8 ) - 1)
		 / (16  / 8 )
		 * (16  / 8 ));
      if (flag_shared_data)
	data_section ();
      if (((decl)->common.public_attr) )
	( fputs (".comm ", (asm_out_file)),	assemble_name ((asm_out_file), ( name)),	fprintf ((asm_out_file), ",%d\n", ( rounded))) ;
      else
	( fputs (".lcomm ", (asm_out_file)),	assemble_name ((asm_out_file), ( name)),	fprintf ((asm_out_file), ",%d\n", ( rounded))) ;
      return;
    }

  if (((decl)->common.public_attr)  && ((decl)->decl.name) )
    do { fputs (".globl ", asm_out_file); assemble_name (asm_out_file,  name); fputs ("\n", asm_out_file);} while (0) ;

  if (((decl)->decl.initial) )
    output_addressed_constants (((decl)->decl.initial) );

  if (((decl)->common.readonly_attr)  && ! ((decl)->common.volatile_attr) )
    text_section ();
  else
    data_section ();

  for (i = 0; ((decl)->decl.align)  >= 8  << (i + 1); i++);
  if (( i) == 1)	fprintf (asm_out_file, "\t.even\n");	else if (( i) != 0)	abort (); ;

  do { assemble_name (asm_out_file,  name); fputs (":\n", asm_out_file); } while (0) ;

  if (((decl)->decl.initial) )
    output_constant (((decl)->decl.initial) , int_size_in_bytes (((decl)->common.type) ));
  else
    fprintf (asm_out_file, "\t.skip %d\n", ( int_size_in_bytes (((decl)->common.type) ))) ;
}

void
assemble_external (decl)
     tree decl;
{
  rtx rtl = ((decl)->decl.rtl) ;

  if (	((rtl)->code)  == MEM && 	((((rtl)->fld[ 0].rtx) )->code)  == SYMBOL_REF)
    {

    }
}

void
assemble_name (file, name)
     struct _iobuf  *file;
     char *name;
{
  if (name[0] == '*')
    fputs (&name[1], file);
  else
    fprintf (file, "_%s",  name) ;
}

rtx
assemble_static_space (size)
     int size;
{
  char name[12];
  char *namestring;
  rtx x;

  int rounded = ((size + (16  / 8 ) - 1)
		 / (16  / 8 )
		 * (16  / 8 ));

  if (flag_shared_data)
    data_section ();
  sprintf (name, "*%s%d",  "LF",  const_labelno) ;
  ++const_labelno;

  namestring = (char *) ({ struct obstack *__h = (saveable_obstack);	({ struct obstack *__o = (__h);	int __len = ( (
				       strlen (name) + 2));	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	__o->next_free += __len;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
  strcpy (namestring, name);
  x = gen_rtx (SYMBOL_REF, SImode , namestring);
  ( fputs (".lcomm ", (asm_out_file)),	assemble_name ((asm_out_file), ( name)),	fprintf ((asm_out_file), ",%d\n", ( rounded))) ;
  return x;
}

static rtx real_constant_chain;

rtx
immed_double_const (i0, i1, mode)
     int i0, i1;
     enum machine_mode mode;
{
  register rtx r;

  if (mode == DImode && i0 == 0 && i1 == 0)
    return const0_rtx;

  for (r = real_constant_chain; r; r = ((r)->fld[ 1].rtx)  )
    if (((r)->fld[ 2].rtint)   == i0 && ((r)->fld[ 3].rtint)   == i1
	&& 	((r)->mode)  == mode)
      return r;

  r = gen_rtx (CONST_DOUBLE, mode, 0, i0, i1);

  ((r)->fld[ 1].rtx)   = real_constant_chain;
  real_constant_chain = r;

  ((r)->fld[ 0].rtx)   = const0_rtx;

  return r;
}

rtx
immed_real_const_1 (d, mode)
     double  d;
     enum machine_mode mode;
{
  union real_extract u;
  register rtx r;
  double  negated;

  u.d = d;

  negated = (- (d)) ;
  if (((negated) == ( d)) )
    return (mode == DFmode ? dconst0_rtx : fconst0_rtx);

  if (sizeof u == 2 * sizeof (int))
    return immed_double_const (u.i[0], u.i[1], mode);

  for (r = real_constant_chain; r; r = ((r)->fld[ 1].rtx)  )
    if (! memcmp (&((r)->fld[ 2].rtint)  , &u, sizeof u) 
	&& 	((r)->mode)  == mode)
      return r;

  r = rtx_alloc (CONST_DOUBLE);
  ((r)->mode = ( mode)) ;
  memcpy ( &((r)->fld[ 2].rtint)  ,&u, sizeof u) ;

  ((r)->fld[ 1].rtx)   = real_constant_chain;
  real_constant_chain = r;

  ((r)->fld[ 0].rtx)   = const0_rtx;

  return r;
}

rtx
immed_real_const (exp)
     tree exp;
{
  return immed_real_const_1 (((exp)->real_cst.real_cst) , ((((exp)->common.type) )->type.mode) );
}

rtx
force_const_double_mem (r)
     rtx r;
{
  if (((r)->fld[ 0].rtx)   == cc0_rtx)
    {
      ((r)->fld[ 1].rtx)   = real_constant_chain;
      real_constant_chain = r;
      ((r)->fld[ 0].rtx)   = const0_rtx;
    }

  if (((r)->fld[ 0].rtx)   == const0_rtx)
    {
      ((r)->fld[ 0].rtx)   = force_const_mem (	((r)->mode) , r);
    }

  if (memory_address_p (	((r)->mode) , ((((r)->fld[ 0].rtx)  )->fld[ 0].rtx) ))
    return ((r)->fld[ 0].rtx)  ;
  return gen_rtx (MEM, 	((r)->mode) , ((((r)->fld[ 0].rtx)  )->fld[ 0].rtx) );
}

void
clear_const_double_mem ()
{
  register rtx r, next;

  for (r = real_constant_chain; r; r = next)
    {
      next = ((r)->fld[ 1].rtx)  ;
      ((r)->fld[ 1].rtx)   = 0;
      ((r)->fld[ 0].rtx)   = cc0_rtx;
    }
  real_constant_chain = 0;
}

struct addr_const
{
  rtx base;
  int offset;
};

static void
decode_addr_const (exp, value)
     tree exp;
     struct addr_const *value;
{
  register tree target = ((exp)->exp.operands[ 0]) ;
  register int offset = 0;
  register rtx x;

  while (1)
    {
      if (((target)->common.code)  == COMPONENT_REF)
	{
	  offset += ((((target)->exp.operands[ 1]) )->decl.offset)  / 8 ;
	  target = ((target)->exp.operands[ 0]) ;
	}
      else if (((target)->common.code)  == ARRAY_REF)
	{
	  if (((((target)->exp.operands[ 1]) )->common.code)  != INTEGER_CST
	      || ((((((target)->common.type) )->type.size) )->common.code)  != INTEGER_CST)
	    abort ();
	  offset += ((((((target)->common.type) )->type.size_unit) 
		      * ((((((target)->common.type) )->type.size) )->int_cst.int_cst_low) 
		      * ((((target)->exp.operands[ 1]) )->int_cst.int_cst_low) )
		     / 8 );
	  target = ((target)->exp.operands[ 0]) ;
	}
      else break;
    }

  if (((target)->common.code)  == VAR_DECL
      || ((target)->common.code)  == FUNCTION_DECL)
    x = ((target)->decl.rtl) ;
  else if (((target)->common.literal_attr) )
    x = ((target)->real_cst.rtl) ;
  else
    abort ();

  if (	((x)->code)  != MEM)
    abort ();
  x = ((x)->fld[ 0].rtx) ;

  value->base = x;
  value->offset = offset;
}

struct constant_descriptor
{
  struct constant_descriptor *next;
  char *label;
  char contents[1];
};

static struct constant_descriptor *const_hash_table[1007 ];

int
const_hash (exp)
     tree exp;
{
  register char *p;
  register int len, hi, i;
  register enum tree_code code = ((exp)->common.code) ;

  if (code == INTEGER_CST)
    {
      p = (char *) &((exp)->int_cst.int_cst_low) ;
      len = 2 * sizeof ((exp)->int_cst.int_cst_low) ;
    }
  else if (code == REAL_CST)
    {
      p = (char *) &((exp)->real_cst.real_cst) ;
      len = sizeof ((exp)->real_cst.real_cst) ;
    }
  else if (code == STRING_CST)
    p = ((exp)->string.pointer) , len = ((exp)->string.length) ;
  else if (code == COMPLEX_CST)
    return const_hash (((exp)->complex.real) ) * 5
      + const_hash (((exp)->complex.imag) );
  else if (code == CONSTRUCTOR)
    {
      register tree link;
      hi = 5;
      for (link = ((exp)->exp.operands[ 1])  ; link; link = ((link)->common.chain) )
	hi = (hi * 603 + const_hash (((link)->list.value) )) % 1007 ;
      return hi;
    }
  else if (code == ADDR_EXPR)
    {
      struct addr_const value;
      decode_addr_const (exp, &value);
      p = (char *) &value;
      len = sizeof value;
    }
  else if (code == PLUS_EXPR || code == MINUS_EXPR)
    return const_hash (((exp)->exp.operands[ 0]) ) * 9
      +  const_hash (((exp)->exp.operands[ 1]) );
  else if (code == NOP_EXPR || code == CONVERT_EXPR)
    return const_hash (((exp)->exp.operands[ 0]) ) * 7 + 2;

  hi = len;
  for (i = 0; i < len; i++)
    hi = ((hi * 613) + (unsigned)(p[i]));

  hi &= (1 << 30 ) - 1;
  hi %= 1007 ;
  return hi;
}

static int
compare_constant (exp, desc)
     tree exp;
     struct constant_descriptor *desc;
{
  return 0 != compare_constant_1 (exp, desc->contents);
}

static char *
compare_constant_1 (exp, p)
     tree exp;
     char *p;
{
  register char *strp;
  register int len;
  register enum tree_code code = ((exp)->common.code) ;

  if (code != (enum tree_code) *p++)
    return 0;

  if (code == INTEGER_CST)
    {
      strp = (char *) &((exp)->int_cst.int_cst_low) ;
      len = 2 * sizeof ((exp)->int_cst.int_cst_low) ;
    }
  else if (code == REAL_CST)
    {
      if (*p++ != ((((exp)->common.type) )->type.sep_unit) )
	return 0;
      strp = (char *) &((exp)->real_cst.real_cst) ;
      len = sizeof ((exp)->real_cst.real_cst) ;
    }
  else if (code == STRING_CST)
    {
      if (flag_writable_strings)
	return 0;
      strp = ((exp)->string.pointer) ;
      len = ((exp)->string.length) ;
      if (memcmp (&((exp)->string.length) , p,
		sizeof ((exp)->string.length) ) )
	return 0;
      p += sizeof ((exp)->string.length) ;
    }
  else if (code == COMPLEX_CST)
    {
      p = compare_constant_1 (((exp)->complex.real) , p);
      if (p == 0) return 0;
      p = compare_constant_1 (((exp)->complex.imag) , p);
      return p;
    }
  else if (code == CONSTRUCTOR)
    {
      register tree link;
      int length = list_length (((exp)->exp.operands[ 1])  );
      if (memcmp (&length, p, sizeof length) )
	return 0;
      p += sizeof length;
      for (link = ((exp)->exp.operands[ 1])  ; link; link = ((link)->common.chain) )
	if ((p = compare_constant_1 (((link)->list.value) , p)) == 0)
	  return 0;
      return p;
    }
  else if (code == ADDR_EXPR)
    {
      struct addr_const value;
      decode_addr_const (exp, &value);
      strp = (char *) &value;
      len = sizeof value;
    }
  else if (code == PLUS_EXPR || code == MINUS_EXPR)
    {
      if (*p++ != (char) code)
	return 0;
      p = compare_constant_1 (((exp)->exp.operands[ 0]) , p);
      if (p == 0) return 0;
      p = compare_constant_1 (((exp)->exp.operands[ 1]) , p);
      return p;
    }
  else if (code == NOP_EXPR || code == CONVERT_EXPR)
    {
      if (*p++ != (char) code)
	return 0;
      p = compare_constant_1 (((exp)->exp.operands[ 0]) , p);
      return p;
    }

  while (--len >= 0)
    if (*p++ != *strp++)
      return 0;

  return p;
}

static struct constant_descriptor *
record_constant (exp)
     tree exp;
{
  struct constant_descriptor *ptr = 0;
  int buf;

  ({ struct obstack *__o = (&permanent_obstack);	int __len = ( sizeof ptr);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, &ptr, __len) ;	__o->next_free += __len;	(void) 0; }) ;
  ({ struct obstack *__o = (&permanent_obstack);	int __len = ( sizeof buf);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, &buf, __len) ;	__o->next_free += __len;	(void) 0; }) ;
  record_constant_1 (exp);
  return (struct constant_descriptor *) ({ struct obstack *__o = (&permanent_obstack);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ;
}

static void
record_constant_1 (exp)
     tree exp;
{
  register char *strp;
  register int len;
  register enum tree_code code = ((exp)->common.code) ;

  ({ struct obstack *__o = (&permanent_obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( (unsigned int) code);	(void) 0; }) ;

  if (code == INTEGER_CST)
    {
      strp = (char *) &((exp)->int_cst.int_cst_low) ;
      len = 2 * sizeof ((exp)->int_cst.int_cst_low) ;
    }
  else if (code == REAL_CST)
    {
      ({ struct obstack *__o = (&permanent_obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( ((((exp)->common.type) )->type.sep_unit) );	(void) 0; }) ;
      strp = (char *) &((exp)->real_cst.real_cst) ;
      len = sizeof ((exp)->real_cst.real_cst) ;
    }
  else if (code == STRING_CST)
    {
      if (flag_writable_strings)
	return;
      strp = ((exp)->string.pointer) ;
      len = ((exp)->string.length) ;
      ({ struct obstack *__o = (&permanent_obstack);	int __len = (
		    sizeof ((exp)->string.length) );	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, (char *) &((exp)->string.length) , __len) ;	__o->next_free += __len;	(void) 0; }) ;
    }
  else if (code == COMPLEX_CST)
    {
      record_constant_1 (((exp)->complex.real) );
      record_constant_1 (((exp)->complex.imag) );
      return;
    }
  else if (code == CONSTRUCTOR)
    {
      register tree link;
      int length = list_length (((exp)->exp.operands[ 1])  );
      ({ struct obstack *__o = (&permanent_obstack);	int __len = ( sizeof length);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, (char *) &length, __len) ;	__o->next_free += __len;	(void) 0; }) ;

      for (link = ((exp)->exp.operands[ 1])  ; link; link = ((link)->common.chain) )
	record_constant_1 (((link)->list.value) );
      return;
    }
  else if (code == ADDR_EXPR)
    {
      struct addr_const value;
      decode_addr_const (exp, &value);
      strp = (char *) &value;
      len = sizeof value;
    }
  else if (code == PLUS_EXPR || code == MINUS_EXPR)
    {
      ({ struct obstack *__o = (&permanent_obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( (int) code);	(void) 0; }) ;
      record_constant_1 (((exp)->exp.operands[ 0]) );
      record_constant_1 (((exp)->exp.operands[ 1]) );
      return;
    }
  else if (code == NOP_EXPR || code == CONVERT_EXPR)
    {
      ({ struct obstack *__o = (&permanent_obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( (int) code);	(void) 0; }) ;
      record_constant_1 (((exp)->exp.operands[ 0]) );
      return;
    }

  ({ struct obstack *__o = (&permanent_obstack);	int __len = ( len);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, strp, __len) ;	__o->next_free += __len;	(void) 0; }) ;
}

static char *
get_or_assign_label (exp)
     tree exp;
{
  register int hash, i;
  register struct constant_descriptor *desc;
  char label[10];

  output_addressed_constants (exp);

  hash = const_hash (exp) % 1007 ;

  for (desc = const_hash_table[hash]; desc; desc = desc->next)
    if (compare_constant (exp, desc))
      return desc->label;

  desc = record_constant (exp);
  desc->next = const_hash_table[hash];
  const_hash_table[hash] = desc;

  if ((((exp)->common.code)  == STRING_CST) && flag_writable_strings)
    data_section ();
  else
    text_section ();

  for (i = 0; ((((exp)->common.type) )->type.align)  >= 8  << (i + 1); i++);
  if (( i) == 1)	fprintf (asm_out_file, "\t.even\n");	else if (( i) != 0)	abort (); ;

  fprintf (asm_out_file, "%s%d:\n",  "LC",  const_labelno) ;

  output_constant (exp,
		   (((exp)->common.code)  == STRING_CST
		    ? ((exp)->string.length) 
		    : int_size_in_bytes (((exp)->common.type) )));

  sprintf (label, "*%s%d",  "LC",  const_labelno) ;

  ++const_labelno;

  desc->label
    = (char *) ({ struct obstack *__h = (&permanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( strlen (label)));	((__o->next_free + __len + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, __len + 1) : 0),	memcpy ( __o->next_free, ( label), __len) ,	__o->next_free += __len,	*(__o->next_free)++ = 0;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;

  return desc->label;
}

rtx
output_constant_def (exp)
     tree exp;
{
  register rtx def;
  int temp_p = allocation_temporary_p ();

  if (((exp)->common.code)  == INTEGER_CST)
    abort ();			 

  if (((exp)->real_cst.rtl) )
    return ((exp)->real_cst.rtl) ;

  if (((exp)->common.permanent_attr) )
    end_temporary_allocation ();

  def = gen_rtx (SYMBOL_REF, SImode , get_or_assign_label (exp));

  ((exp)->real_cst.rtl) 
    = gen_rtx (MEM, ((((exp)->common.type) )->type.mode) , def);
  ((((exp)->real_cst.rtl) )->unchanging)  = 1;

  if (temp_p && ((exp)->common.permanent_attr) )
    resume_temporary_allocation ();

  return ((exp)->real_cst.rtl) ;
}

static struct constant_descriptor *const_rtx_hash_table[61 ];

void
init_const_rtx_hash_table ()
{
  memset (const_rtx_hash_table,0, sizeof const_rtx_hash_table) ;
}

struct rtx_const
{
  enum kind { RTX_DOUBLE, RTX_INT } kind : 16;
  enum machine_mode mode : 16;
  union {
    union real_extract du;
    struct addr_const addr;
  } un;
};

static void
decode_rtx_const (mode, x, value)
     enum machine_mode mode;
     rtx x;
     struct rtx_const *value;
{

  {
    int *p = (int *) value;
    int *end = (int *) (value + 1);
    while (p < end)
      *p++ = 0;
  }

  value->kind = RTX_INT;	 
  value->mode = mode;

  switch (	((x)->code) )
    {
    case CONST_DOUBLE:
      value->kind = RTX_DOUBLE;
      value->mode = 	((x)->mode) ;
      memcpy ( &value->un.du,&((x)->fld[ 2].rtint)  , sizeof value->un.du) ;
      break;

    case CONST_INT:
      value->un.addr.offset = ((x)->fld[0].rtint) ;
      break;

    case SYMBOL_REF:
      value->un.addr.base = x;
      break;

    case LABEL_REF:
      value->un.addr.base = x;
      break;
    case CONST:
      x = ((x)->fld[ 0].rtx) ;
      if (	((x)->code)  == PLUS)
	{
	  value->un.addr.base = ((((x)->fld[ 0].rtx) )->fld[ 0].rtx) ;
	  if (	((((x)->fld[ 1].rtx) )->code)  != CONST_INT)
	    abort ();
	  value->un.addr.offset = ((((x)->fld[ 1].rtx) )->fld[0].rtint) ;
	}
      else if (	((x)->code)  == MINUS)
	{
	  value->un.addr.base = ((x)->fld[ 0].rtx) ;
	  if (	((((x)->fld[ 1].rtx) )->code)  != CONST_INT)
	    abort ();
	  value->un.addr.offset = - ((((x)->fld[ 1].rtx) )->fld[0].rtint) ;
	}
      else
	abort ();
      break;

    default:
      abort ();
    }

  if (value->kind == RTX_INT && value->un.addr.base != 0)
    switch (	((value->un.addr.base)->code) )
      {
      case SYMBOL_REF:
      case LABEL_REF:

	value->un.addr.base = ((value->un.addr.base)->fld[ 0].rtx) ;
      }
}

int
const_hash_rtx (mode, x)
     enum machine_mode mode;
     rtx x;
{
  register int hi, i;

  struct rtx_const value;
  decode_rtx_const (mode, x, &value);

  hi = 0;
  for (i = 0; i < sizeof value / sizeof (int); i++)
    hi += ((int *) &value)[i];

  hi &= (1 << 30 ) - 1;
  hi %= 61 ;
  return hi;
}

static int
compare_constant_rtx (mode, x, desc)
     enum machine_mode mode;
     rtx x;
     struct constant_descriptor *desc;
{
  register int *p = (int *) desc->contents;
  register int *strp;
  register int len;
  struct rtx_const value;

  decode_rtx_const (mode, x, &value);
  strp = (int *) &value;
  len = sizeof value / sizeof (int);

  while (--len >= 0)
    if (*p++ != *strp++)
      return 0;

  return 1;
}

static struct constant_descriptor *
record_constant_rtx (mode, x)
     enum machine_mode mode;
     rtx x;
{
  struct constant_descriptor *ptr = 0;
  int buf;
  struct rtx_const value;

  decode_rtx_const (mode, x, &value);

  ({ struct obstack *__o = (saveable_obstack);	int __len = ( sizeof ptr);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, &ptr, __len) ;	__o->next_free += __len;	(void) 0; }) ;
  ({ struct obstack *__o = (saveable_obstack);	int __len = ( sizeof buf);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, &buf, __len) ;	__o->next_free += __len;	(void) 0; }) ;

  ({ struct obstack *__o = (saveable_obstack);	int __len = ( sizeof value);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, &value, __len) ;	__o->next_free += __len;	(void) 0; }) ;

  return (struct constant_descriptor *) ({ struct obstack *__o = (saveable_obstack);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ;
}

rtx
force_const_mem (mode, x)
     enum machine_mode mode;
     rtx x;
{
  register int hash;
  register struct constant_descriptor *desc;
  char label[10];
  char *found = 0;
  rtx def;

  if (	((x)->code)  == CONST_DOUBLE
      && 	((((x)->fld[ 0].rtx)  )->code)  == MEM)
    return ((x)->fld[ 0].rtx)  ;

  hash = const_hash_rtx (mode, x);

  for (desc = const_rtx_hash_table[hash]; desc; desc = desc->next)
    if (compare_constant_rtx (mode, x, desc))
      {
	found = desc->label;
	break;
      }

  if (found == 0)
    {
      int align;

      desc = record_constant_rtx (mode, x);
      desc->next = const_rtx_hash_table[hash];
      const_rtx_hash_table[hash] = desc;

      text_section ();

      align = (mode == VOIDmode) ? 4  : 	(mode_size[(int)(mode)]) ;
      if (align > 16  / 8 )
	align = 16  / 8 ;

      if (( exact_log2 (align)) == 1)	fprintf (asm_out_file, "\t.even\n");	else if (( exact_log2 (align)) != 0)	abort (); ;

      fprintf (asm_out_file, "%s%d:\n",  "LC",  const_labelno) ;

      if (	((x)->code)  == CONST_DOUBLE)
	{
	  union real_extract u;

	  memcpy ( &u,&((x)->fld[ 2].rtint)  , sizeof u) ;
	  switch (mode)
	    {
	    case DImode:

	      ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, (
			      gen_rtx (CONST_INT, VOIDmode, u.i[0]))),	fprintf (asm_out_file, "\n")) ;
	      ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, (
			      gen_rtx (CONST_INT, VOIDmode, u.i[1]))),	fprintf (asm_out_file, "\n")) ;

	      break;

	    case DFmode:
	      fprintf (asm_out_file, "\t.double 0r%.20e\n", ( u.d)) ;
	      break;

	    case SFmode:
	      fprintf (asm_out_file, "\t.single 0r%.20e\n", ( u.d)) ;
	    }
	}
      else
	switch (mode)
	  {
	  case SImode:
	    ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, ( x)),	fprintf (asm_out_file, "\n")) ;
	    break;

	  case HImode:
	    ( fprintf (asm_out_file, "\t.word "),	output_addr_const (asm_out_file, ( x)),	fprintf (asm_out_file, "\n")) ;
	    break;

	  case QImode:
	    ( fprintf (asm_out_file, "\t.byte "),	output_addr_const (asm_out_file, ( x)),	fprintf (asm_out_file, "\n")) ;
	    break;
	  }

      sprintf (label, "*%s%d",  "LC",  const_labelno) ;

      ++const_labelno;

      desc->label = found
	= (char *) ({ struct obstack *__h = (&permanent_obstack);	({ struct obstack *__o = (__h);	int __len = ( ( strlen (label)));	((__o->next_free + __len + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, __len + 1) : 0),	memcpy ( __o->next_free, ( label), __len) ,	__o->next_free += __len,	*(__o->next_free)++ = 0;	(void) 0; }) ;	({ struct obstack *__o = (__h);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ; }) ;
    }

  def = gen_rtx (MEM, mode, gen_rtx (SYMBOL_REF, SImode , desc->label));

  ((def)->unchanging)  = 1;
  ((((def)->fld[ 0].rtx) )->unchanging)  = 1;

  if (	((x)->code)  == CONST_DOUBLE)
    {
      if (((x)->fld[ 0].rtx)   == cc0_rtx)
	{
	  ((x)->fld[ 1].rtx)   = real_constant_chain;
	  real_constant_chain = x;
	}
      ((x)->fld[ 0].rtx)   = def;
    }

  return def;
}

void
output_addressed_constants (exp)
     tree exp;
{
  switch (((exp)->common.code) )
    {
    case ADDR_EXPR:
      {
	register tree constant = ((exp)->exp.operands[ 0]) ;

	while (((constant)->common.code)  == COMPONENT_REF)
	  {
	    constant = ((constant)->exp.operands[ 0]) ;
	  }

	if (((constant)->common.literal_attr) )

	  output_constant_def (constant);
      }
      break;

    case PLUS_EXPR:
    case MINUS_EXPR:
      output_addressed_constants (((exp)->exp.operands[ 0]) );
      output_addressed_constants (((exp)->exp.operands[ 1]) );
      break;

    case NOP_EXPR:
    case CONVERT_EXPR:
      output_addressed_constants (((exp)->exp.operands[ 0]) );
      break;

    case CONSTRUCTOR:
      {
	register tree link;
	for (link = ((exp)->exp.operands[ 1])  ; link; link = ((link)->common.chain) )
	  output_addressed_constants (((link)->list.value) );
      }
      break;

    case ERROR_MARK:
      break;

    default:
      if (! ((exp)->common.literal_attr) )
	abort ();
    }
}

void
output_constant (exp, size)
     register tree exp;
     register int size;
{
  register enum tree_code code = ((((exp)->common.type) )->common.code) ;
  rtx x;

  if (size == 0)
    return;

  if (((exp)->common.code)  == NOP_EXPR
      && ((exp)->common.type)  == ((((exp)->exp.operands[ 0]) )->common.type) )
    exp = ((exp)->exp.operands[ 0]) ;

  switch (code)
    {
    case INTEGER_TYPE:
    case ENUMERAL_TYPE:
    case POINTER_TYPE:
    case REFERENCE_TYPE:
      while (((exp)->common.code)  == NOP_EXPR || ((exp)->common.code)  == CONVERT_EXPR)
	exp = ((exp)->exp.operands[ 0]) ;

      if (((((exp)->common.type) )->type.mode)  == DImode)
	{
	  if (((exp)->common.code)  == INTEGER_CST)
	    {

	      ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, (
			      gen_rtx (CONST_INT, VOIDmode,
				       ((exp)->int_cst.int_cst_low) ))),	fprintf (asm_out_file, "\n")) ;
	      ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, (
			      gen_rtx (CONST_INT, VOIDmode,
				       ((exp)->int_cst.int_cst_high) ))),	fprintf (asm_out_file, "\n")) ;

	      size -= 8;
	      break;
	    }
	  else
	    error ("8-byte integer constant expression too complicated");

	  break;
	}

      x = expand_expr (exp, 0, VOIDmode, EXPAND_SUM);

      if (size == 1)
	{
	  ( fprintf (asm_out_file, "\t.byte "),	output_addr_const (asm_out_file, ( x)),	fprintf (asm_out_file, "\n")) ;
	  size -= 1;
	}
      else if (size == 2)
	{
	  ( fprintf (asm_out_file, "\t.word "),	output_addr_const (asm_out_file, ( x)),	fprintf (asm_out_file, "\n")) ;
	  size -= 2;
	}
      else if (size == 4)
	{
	  ( fprintf (asm_out_file, "\t.long "),	output_addr_const (asm_out_file, ( x)),	fprintf (asm_out_file, "\n")) ;
	  size -= 4;
	}
      else
	abort ();

      break;

    case REAL_TYPE:
      if (((exp)->common.code)  != REAL_CST)
	error ("initializer for floating value is not a floating constant");

      if (size < 4)
	break;
      else if (size < 8)
	{
	  fprintf (asm_out_file, "\t.single 0r%.20e\n", ( ((exp)->real_cst.real_cst) )) ;
	  size -= 4;
	}
      else
	{
	  fprintf (asm_out_file, "\t.double 0r%.20e\n", ( ((exp)->real_cst.real_cst) )) ;
	  size -= 8;
	}
      break;

    case COMPLEX_TYPE:
      output_constant (((exp)->complex.real) , size / 2);
      output_constant (((exp)->complex.imag) , size / 2);
      size -= (size / 2) * 2;
      break;

    case ARRAY_TYPE:
      if (((exp)->common.code)  == CONSTRUCTOR)
	{
	  output_constructor (exp, size);
	  return;
	}
      else if (((exp)->common.code)  == STRING_CST)
	{
	  int excess = 0;

	  if (size > ((exp)->string.length) )
	    {
	      excess = size - ((exp)->string.length) ;
	      size = ((exp)->string.length) ;
	    }

	  assemble_string (((exp)->string.pointer) , size);
	  size = excess;
	}
      else
	abort ();
      break;

    case RECORD_TYPE:
    case UNION_TYPE:
      if (((exp)->common.code)  == CONSTRUCTOR)
	output_constructor (exp, size);
      else
	abort ();
      return;
    }

  if (size > 0)
    fprintf (asm_out_file, "\t.skip %d\n", ( size)) ;
}

void
output_constructor (exp, size)
     tree exp;
     int size;
{
  register tree link, field = 0;
  register int byte;
  int total_bytes = 0;
  int byte_offset = -1;

  if (((((exp)->common.type) )->common.code)  == RECORD_TYPE
      || ((((exp)->common.type) )->common.code)  == UNION_TYPE)
    field = ((((exp)->common.type) )->type.values) ;

  for (link = ((exp)->exp.operands[ 1])  ;
       link;
       link = ((link)->common.chain) ,
       field = field ? ((field)->common.chain)  : 0)
    {
      tree val = ((link)->list.value) ;

      if (((val)->common.code)  == NOP_EXPR
	  && ((val)->common.type)  == ((((val)->exp.operands[ 0]) )->common.type) )
	val = ((val)->exp.operands[ 0]) ;

      if (field == 0
	  || (((field)->decl.mode)  != BImode))
	{
	  register int fieldsize;

	  if (byte_offset >= 0)
	    {
	      fprintf (asm_out_file, "\t.byte 0x%x\n", ( byte)) ;
	      total_bytes++;
	      byte_offset = -1;
	    }

	  if (field && (total_bytes * 8 ) % ((field)->decl.align)  != 0)
	    {
	      int byte_align = ((field)->decl.align)  / 8 ;
	      int to_byte = (((total_bytes + byte_align - 1) / byte_align)
			     * byte_align);
	      fprintf (asm_out_file, "\t.skip %d\n", ( to_byte - total_bytes)) ;
	      total_bytes = to_byte;
	    }

	  if (field)
	    {
	      if (! ((((field)->decl.size) )->common.literal_attr) )
		abort ();
	      fieldsize = ((((field)->decl.size) )->int_cst.int_cst_low) 
		* ((field)->decl.size_unit) ;
	      fieldsize = (fieldsize + 8  - 1) / 8 ;
	    }
	  else
	    fieldsize = int_size_in_bytes (((((exp)->common.type) )->common.type) );

	  output_constant (val, fieldsize);

	  total_bytes += fieldsize;
	}
      else if (((val)->common.code)  != INTEGER_CST)
	error ("invalid initial value for member `%s'",
	       ((((field)->decl.name) )->identifier.pointer) );
      else
	{

	  int next_offset = ((field)->decl.offset) ;
	  int end_offset
	    = (next_offset
	       + (((((field)->decl.size) )->int_cst.int_cst_low) 
		  * ((field)->decl.size_unit) ));

	  while (next_offset < end_offset)
	    {
	      int this_time;
	      int next_byte = next_offset / 8 ;
	      int next_bit = next_offset % 8 ;
	      if (byte_offset < 0)
		{
		  byte_offset = next_byte;
		  byte = 0;
		}
	      else
		while (next_byte != byte_offset)
		  {
		    fprintf (asm_out_file, "\t.byte 0x%x\n", ( byte)) ;
		    byte_offset++;
		    total_bytes++;
		    byte = 0;
		  }

	      this_time = ((end_offset - next_offset) < (
			       8  - next_bit) ? (end_offset - next_offset) : (
			       8  - next_bit)) ;

	      byte |= (((((val)->int_cst.int_cst_low) 
			 >> (end_offset - next_offset - this_time))
			& ((1 << this_time) - 1))
		       << (8  - this_time - next_bit));

	      next_offset += this_time;
	    }
	}
    }
  if (byte_offset >= 0)
    {
      fprintf (asm_out_file, "\t.byte 0x%x\n", ( byte)) ;
      byte_offset = -1;
      total_bytes++;
    }
  if (total_bytes < size)
    fprintf (asm_out_file, "\t.skip %d\n", ( size - total_bytes)) ;
}

