
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

rtx
plus_constant (x, c)
     register rtx x;
     register int c;
{
  register enum rtx_code  code = 	((x)->code) ;
  register enum machine_mode mode = 	((x)->mode) ;
  int all_constant = 0;

  if (c == 0)
    return x;

  if (code == CONST_INT)
    return gen_rtx (CONST_INT, VOIDmode, (((x)->fld[0].rtint)  + c));

  if (code == CONST)
    {
      x = ((x)->fld[ 0].rtx) ;
      all_constant = 1;
    }
  else if (code == SYMBOL_REF || code == LABEL_REF)
    all_constant = 1;

  if (	((x)->code)  == PLUS)
    {
      if (	((((x)->fld[ 0].rtx) )->code)  == CONST_INT)
	{
	  c += ((((x)->fld[ 0].rtx) )->fld[0].rtint) ;
	  x = ((x)->fld[ 1].rtx) ;
	}
      else if (	((((x)->fld[ 1].rtx) )->code)  == CONST_INT)
	{
	  c += ((((x)->fld[ 1].rtx) )->fld[0].rtint) ;
	  x = ((x)->fld[ 0].rtx) ;
	}
      else if ((	((((x)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((x)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST_INT	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST) )
	{
	  return gen_rtx (PLUS, mode,
			  plus_constant (((x)->fld[ 0].rtx) , c),
			  ((x)->fld[ 1].rtx) );
	}
      else if ((	((((x)->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((x)->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((x)->fld[ 1].rtx) )->code)  == CONST_INT	|| 	((((x)->fld[ 1].rtx) )->code)  == CONST) )
	{
	  return gen_rtx (PLUS, mode,
			  ((x)->fld[ 0].rtx) ,
			  plus_constant (((x)->fld[ 1].rtx) , c));
	}

    }
  if (c != 0)
    x = gen_rtx (PLUS, mode, x, gen_rtx (CONST_INT, VOIDmode, c));

  if (	((x)->code)  == SYMBOL_REF || 	((x)->code)  == LABEL_REF)
    return x;
  else if (all_constant)
    return gen_rtx (CONST, mode, x);
  else
    return x;
}

rtx
eliminate_constant_term (x, constptr)
     rtx x;
     int *constptr;
{
  int c;
  register rtx x0, x1;

  if (	((x)->code)  != PLUS)
    return x;

  if (	((((x)->fld[ 0].rtx) )->code)  == CONST_INT)
    {
      *constptr += ((((x)->fld[ 0].rtx) )->fld[0].rtint) ;
      return eliminate_constant_term (((x)->fld[ 1].rtx) , constptr);
    }

  if (	((((x)->fld[ 1].rtx) )->code)  == CONST_INT)
    {
      *constptr += ((((x)->fld[ 1].rtx) )->fld[0].rtint) ;
      return eliminate_constant_term (((x)->fld[ 0].rtx) , constptr);
    }

  c = 0;
  x0 = eliminate_constant_term (((x)->fld[ 0].rtx) , &c);
  x1 = eliminate_constant_term (((x)->fld[ 1].rtx) , &c);
  if (x1 != ((x)->fld[ 1].rtx)  || x0 != ((x)->fld[ 0].rtx) )
    {
      *constptr += c;
      return gen_rtx (PLUS, 	((x)->mode) , x0, x1);
    }
  return x;
}

rtx
expr_size (exp)
     tree exp;
{
  return expand_expr (size_in_bytes (((exp)->common.type) ), 0, SImode, 0);
}

rtx
lookup_static_chain ()
{
  abort ();
}

static rtx
break_out_memory_refs (x)
     register rtx x;
{
  if (	((x)->code)  == MEM || 	((x)->code)  == CONST
      || 	((x)->code)  == SYMBOL_REF)
    {
      register rtx temp = force_reg (SImode , x);
      mark_reg_pointer (temp);
      x = temp;
    }
  else if (	((x)->code)  == PLUS || 	((x)->code)  == MINUS
	   || 	((x)->code)  == MULT)
    {
      register rtx op0 = break_out_memory_refs (((x)->fld[ 0].rtx) );
      register rtx op1 = break_out_memory_refs (((x)->fld[ 1].rtx) );
      if (op0 != ((x)->fld[ 0].rtx)  || op1 != ((x)->fld[ 1].rtx) )
	x = gen_rtx (	((x)->code) , SImode , op0, op1);
    }
  return x;
}

rtx
copy_all_regs (x)
     register rtx x;
{
  if (	((x)->code)  == REG)
    {
      if (((x)->fld[0].rtint)  != 14 )
	x = copy_to_reg (x);
    }
  else if (	((x)->code)  == MEM)
    x = copy_to_reg (x);
  else if (	((x)->code)  == PLUS || 	((x)->code)  == MINUS
	   || 	((x)->code)  == MULT)
    {
      register rtx op0 = copy_all_regs (((x)->fld[ 0].rtx) );
      register rtx op1 = copy_all_regs (((x)->fld[ 1].rtx) );
      if (op0 != ((x)->fld[ 0].rtx)  || op1 != ((x)->fld[ 1].rtx) )
	x = gen_rtx (	((x)->code) , SImode , op0, op1);
    }
  return x;
}

rtx
memory_address (mode, x)
     enum machine_mode mode;
     register rtx x;
{
  register rtx oldx;

  if (! cse_not_expected && (	((x)->code)  == LABEL_REF || 	((x)->code)  == SYMBOL_REF	|| 	((x)->code)  == CONST_INT	|| 	((x)->code)  == CONST) )
    return force_reg (SImode , x);

  if (	((x)->code)  == QUEUED
      && 	((((x)->fld[ 0].rtx)  )->code)  == REG)
    return x;

  oldx = x;
  if (! cse_not_expected && 	((x)->code)  != REG)
    x = break_out_memory_refs (x);

  { { if (( (	(( x)->code)  == LABEL_REF || 	(( x)->code)  == SYMBOL_REF	|| 	(( x)->code)  == CONST_INT	|| 	(( x)->code)  == CONST)  	|| (	(( x)->code)  == REG && (((( x)->fld[0].rtint)  & ~027) != 0) )	|| ((	(( x)->code)  == PRE_DEC || 	(( x)->code)  == POST_INC)	&& (	(((( x)->fld[ 0].rtx) )->code)  == REG) 	&& (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) )	|| (	(( x)->code)  == PLUS	&& (	(((( x)->fld[ 0].rtx) )->code)  == REG)  && (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) 	&& 	(((( x)->fld[ 1].rtx) )->code)  == CONST_INT	&& ((unsigned) (((( x)->fld[ 1].rtx) )->fld[0].rtint)  + 0x8000) < 0x10000)) ) goto   win; } ;	{ { if (	(( x)->code)  == PLUS && (((	(((( x)->fld[ 0].rtx) )->code)  == REG && (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( x)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( x)->fld[ 0].rtx) )->code)  == MULT	&& ((	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( x)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	(((( x)->fld[ 1].rtx) )->code)  == REG && (((((( x)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	(( x)->code)  == PLUS && (((	(((( x)->fld[ 1].rtx) )->code)  == REG && (((((( x)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( x)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( x)->fld[ 1].rtx) )->code)  == MULT	&& ((	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( x)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	(((( x)->fld[ 0].rtx) )->code)  == REG && (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ;	if (	(( x)->code)  == PLUS)	{ if (	(((( x)->fld[ 1].rtx) )->code)  == CONST_INT	&& (unsigned) (((( x)->fld[ 1].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( x)->fld[ 0].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ; }	if (	(((( x)->fld[ 0].rtx) )->code)  == CONST_INT	&& (unsigned) (((( x)->fld[ 0].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( x)->fld[ 1].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ; } } } ; } ;

  if (memory_address_p (mode, oldx))
    goto win2;

  { register int ch = (x) != ( oldx);	if (	((x)->code)  == PLUS)	{ if (	((((x)->fld[ 0].rtx) )->code)  == MULT)	ch = 1, ((x)->fld[ 0].rtx)  = force_operand (((x)->fld[ 0].rtx) , 0);	if (	((((x)->fld[ 1].rtx) )->code)  == MULT)	ch = 1, ((x)->fld[ 1].rtx)  = force_operand (((x)->fld[ 1].rtx) , 0);	if (ch && 	((((x)->fld[ 1].rtx) )->code)  == REG	&& 	((((x)->fld[ 0].rtx) )->code)  == REG)	return x;	if (ch) { { { if (( (	(( x)->code)  == LABEL_REF || 	(( x)->code)  == SYMBOL_REF	|| 	(( x)->code)  == CONST_INT	|| 	(( x)->code)  == CONST)  	|| (	(( x)->code)  == REG && (((( x)->fld[0].rtint)  & ~027) != 0) )	|| ((	(( x)->code)  == PRE_DEC || 	(( x)->code)  == POST_INC)	&& (	(((( x)->fld[ 0].rtx) )->code)  == REG) 	&& (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) )	|| (	(( x)->code)  == PLUS	&& (	(((( x)->fld[ 0].rtx) )->code)  == REG)  && (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) 	&& 	(((( x)->fld[ 1].rtx) )->code)  == CONST_INT	&& ((unsigned) (((( x)->fld[ 1].rtx) )->fld[0].rtint)  + 0x8000) < 0x10000)) ) goto    win; } ;	{ { if (	(( x)->code)  == PLUS && (((	(((( x)->fld[ 0].rtx) )->code)  == REG && (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( x)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( x)->fld[ 0].rtx) )->code)  == MULT	&& ((	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( x)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( x)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( x)->fld[ 1].rtx) )->code)  == LABEL_REF) goto      win;	if (	(((( x)->fld[ 1].rtx) )->code)  == REG && (((((( x)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto      win; } ; }	if (	(( x)->code)  == PLUS && (((	(((( x)->fld[ 1].rtx) )->code)  == REG && (((((( x)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( x)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( x)->fld[ 1].rtx) )->code)  == MULT	&& ((	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( x)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( x)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( x)->fld[ 0].rtx) )->code)  == LABEL_REF) goto      win;	if (	(((( x)->fld[ 0].rtx) )->code)  == REG && (((((( x)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto      win; } ; } } ;	if (	(( x)->code)  == PLUS)	{ if (	(((( x)->fld[ 1].rtx) )->code)  == CONST_INT	&& (unsigned) (((( x)->fld[ 1].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( x)->fld[ 0].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto      win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto      win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto      win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto      win; } ; } } ; }	if (	(((( x)->fld[ 0].rtx) )->code)  == CONST_INT	&& (unsigned) (((( x)->fld[ 0].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( x)->fld[ 1].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto      win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto      win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto      win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto      win; } ; } } ; } } } ; } ; }	if (	((((x)->fld[ 0].rtx) )->code)  == REG	|| (	((((x)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((x)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((x)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode))	{ register rtx temp = gen_reg_rtx (SImode );	register rtx val = force_operand (((x)->fld[ 1].rtx) , 0);	emit_move_insn (temp, val);	((x)->fld[ 1].rtx)  = temp;	return x; }	else if (	((((x)->fld[ 1].rtx) )->code)  == REG	|| (	((((x)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((x)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((x)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode))	{ register rtx temp = gen_reg_rtx (SImode );	register rtx val = force_operand (((x)->fld[ 0].rtx) , 0);	emit_move_insn (temp, val);	((x)->fld[ 0].rtx)  = temp;	return x; }}} ;

  if (	((x)->code)  == PLUS)
    {
      int constant_term = 0;
      rtx y = eliminate_constant_term (x, &constant_term);
      if (constant_term == 0
	  || ! memory_address_p (mode, y))
	return force_operand (x, 0);

      y = plus_constant (copy_to_reg (y), constant_term);
      if (! memory_address_p (mode, y))
	return force_operand (x, 0);
      return y;
    }
  if (	((x)->code)  == MULT || 	((x)->code)  == MINUS)
    return force_operand (x, 0);

  return force_reg (SImode , x);

 win2:
  x = oldx;
 win:
  if (flag_force_addr && optimize && 	((x)->code)  != REG

      && ! (	((x)->code)  == PLUS
	    && (((x)->fld[ 0].rtx)  == frame_pointer_rtx
		|| ((x)->fld[ 0].rtx)  == arg_pointer_rtx)))
    return force_reg (SImode , x);
  return x;
}

rtx
memory_address_noforce (mode, x)
     enum machine_mode mode;
     rtx x;
{
  int ambient_force_addr = flag_force_addr;
  rtx val;

  flag_force_addr = 0;
  val = memory_address (mode, x);
  flag_force_addr = ambient_force_addr;
  return val;
}

rtx
stabilize (x)
     rtx x;
{
  register rtx addr;
  if (	((x)->code)  != MEM)
    return x;
  addr = ((x)->fld[ 0].rtx) ;
  if (rtx_unstable_p (addr))
    {
      rtx temp = copy_all_regs (addr);
      rtx mem;
      if (	((temp)->code)  != REG)
	temp = copy_to_reg (temp);
      mem = gen_rtx (MEM, 	((x)->mode) , temp);

      if (	((addr)->code)  == PLUS || ((x)->in_struct) )
	((mem)->in_struct)  = 1;
      return mem;
    }
  return x;
}

rtx
copy_to_reg (x)
     rtx x;
{
  register rtx temp = gen_reg_rtx (	((x)->mode) );
  emit_move_insn (temp, x);
  return temp;
}

rtx
copy_addr_to_reg (x)
     rtx x;
{
  register rtx temp = gen_reg_rtx (SImode );
  emit_move_insn (temp, x);
  return temp;
}

rtx
copy_to_mode_reg (mode, x)
     enum machine_mode mode;
     rtx x;
{
  register rtx temp = gen_reg_rtx (mode);
  if (	((x)->mode)  != mode && 	((x)->mode)  != VOIDmode)
    abort ();
  emit_move_insn (temp, x);
  return temp;
}

rtx
force_reg (mode, x)
     enum machine_mode mode;
     rtx x;
{
  register rtx temp, insn;

  if (	((x)->code)  == REG)
    return x;
  temp = gen_reg_rtx (mode);
  insn = emit_move_insn (temp, x);

  if ((	((x)->code)  == LABEL_REF || 	((x)->code)  == SYMBOL_REF	|| 	((x)->code)  == CONST_INT	|| 	((x)->code)  == CONST) )
    ((insn)->fld[6].rtx)  = gen_rtx (EXPR_LIST, REG_EQUIV, x, ((insn)->fld[6].rtx) );
  return temp;
}

rtx
force_not_mem (x)
     rtx x;
{
  register rtx temp;
  if (	((x)->code)  != MEM)
    return x;
  temp = gen_reg_rtx (	((x)->mode) );
  emit_move_insn (temp, x);
  return temp;
}

rtx
copy_to_suggested_reg (x, target)
     rtx x, target;
{
  register rtx temp;
  if (target && 	((target)->code)  == REG)
    temp = target;
  else
    temp = gen_reg_rtx (	((x)->mode) );
  emit_move_insn (temp, x);
  return temp;
}

void
adjust_stack (adjust)
     rtx adjust;
{
  adjust = protect_from_queue (adjust, 0);

  emit_insn (gen_add2_insn (stack_pointer_rtx, adjust));

}

void
anti_adjust_stack (adjust)
     rtx adjust;
{
  adjust = protect_from_queue (adjust, 0);

  emit_insn (gen_sub2_insn (stack_pointer_rtx, adjust));

}

rtx
round_push (size)
     rtx size;
{

  int align = 16  / 8 ;
  if (align == 1)
    ;
  if (	((size)->code)  == CONST_INT)
    {
      int new = (((size)->fld[0].rtint)  + align - 1) / align * align;
      if (((size)->fld[0].rtint)  != new)
	size = gen_rtx (CONST_INT, VOIDmode, new);
    }
  else
    {
      size = expand_divmod (0, CEIL_DIV_EXPR, SImode , size,
			    gen_rtx (CONST_INT, VOIDmode, align),
			    0, 1);
      size = expand_mult (SImode , size,
			  gen_rtx (CONST_INT, VOIDmode, align),
			  0, 1);
    }

  return size;
}

rtx
hard_function_value (valtype, func)
     tree valtype;
     tree func;
{
  return gen_rtx (REG, ((valtype)->type.mode) , 0) ;
}

rtx
hard_libcall_value (mode)
     enum machine_mode mode;
{
  return  gen_rtx (REG, mode, 0) ;
}

