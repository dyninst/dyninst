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
  CC_NOOVmode, CCFPmode, CCFPEmode ,
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
  int  rtwint;
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
extern rtx plus_constant_wide		 (rtx, int ) ;
extern rtx plus_constant_for_output_wide (rtx, int ) ;
extern rtx gen_rtx ();
extern rtvec gen_rtvec ();
extern rtx read_rtx ();
extern char *xrealloc ();
extern char *xmalloc			(unsigned) ;
extern char *oballoc			(int) ;
extern char *permalloc			(int) ;
extern void free			(void *) ;
extern rtx rtx_alloc			(enum rtx_code ) ;
extern rtvec rtvec_alloc		(int) ;
extern rtx find_reg_note		(rtx, enum reg_note, rtx) ;
extern rtx find_regno_note		(rtx, enum reg_note, int) ;
extern int  get_integer_term	(rtx) ;
extern rtx get_related_value		(rtx) ;
extern rtx single_set			(rtx) ;
extern rtx find_last_value		(rtx, rtx *, rtx) ;
extern rtx copy_rtx			(rtx) ;
extern rtx copy_rtx_if_shared		(rtx) ;
extern rtx copy_most_rtx		(rtx, rtx) ;
extern rtx replace_rtx			(rtx, rtx, rtx) ;
extern rtvec gen_rtvec_v		(int, rtx *) ;
extern rtx gen_reg_rtx			(enum machine_mode) ;
extern rtx gen_label_rtx		(void) ;
extern rtx gen_inline_header_rtx	(rtx, rtx, int, int, int, int, int, int, rtx, int, int, rtvec, rtx) ;
extern rtx gen_lowpart_common		(enum machine_mode, rtx) ;
extern rtx gen_lowpart			(enum machine_mode, rtx) ;
extern rtx gen_lowpart_if_possible	(enum machine_mode, rtx) ;
extern rtx gen_highpart			(enum machine_mode, rtx) ;
extern rtx gen_realpart			(enum machine_mode, rtx) ;
extern rtx gen_imagpart			(enum machine_mode, rtx) ;
extern rtx operand_subword		(rtx, int, int, enum machine_mode) ;
extern rtx operand_subword_force	(rtx, int, enum machine_mode) ;
extern int subreg_lowpart_p		(rtx) ;
extern rtx make_safe_from		(rtx, rtx) ;
extern rtx memory_address		(enum machine_mode, rtx) ;
extern rtx get_insns			(void) ;
extern rtx get_last_insn		(void) ;
extern rtx get_last_insn_anywhere	(void) ;
extern void start_sequence		(void) ;
extern void push_to_sequence		(rtx) ;
extern void end_sequence		(void) ;
extern rtx gen_sequence			(void) ;
extern rtx immed_double_const		(int , int , enum machine_mode) ;
extern rtx force_const_mem		(enum machine_mode, rtx) ;
extern rtx force_reg			(enum machine_mode, rtx) ;
extern rtx get_pool_constant		(rtx) ;
extern enum machine_mode get_pool_mode	(rtx) ;
extern int get_pool_offset		(rtx) ;
extern rtx simplify_subtraction		(rtx) ;
extern rtx assign_stack_local		(enum machine_mode, int, int) ;
extern rtx assign_stack_temp		(enum machine_mode, int, int) ;
extern rtx protect_from_queue		(rtx, int) ;
extern void emit_queue			(void) ;
extern rtx emit_move_insn		(rtx, rtx) ;
extern rtx emit_insn_before		(rtx, rtx) ;
extern rtx emit_jump_insn_before	(rtx, rtx) ;
extern rtx emit_call_insn_before	(rtx, rtx) ;
extern rtx emit_barrier_before		(rtx) ;
extern rtx emit_note_before		(int, rtx) ;
extern rtx emit_insn_after		(rtx, rtx) ;
extern rtx emit_jump_insn_after		(rtx, rtx) ;
extern rtx emit_barrier_after		(rtx) ;
extern rtx emit_label_after		(rtx, rtx) ;
extern rtx emit_note_after		(int, rtx) ;
extern rtx emit_line_note_after		(char *, int, rtx) ;
extern rtx emit_insn			(rtx) ;
extern rtx emit_insns			(rtx) ;
extern rtx emit_insns_before		(rtx, rtx) ;
extern rtx emit_jump_insn		(rtx) ;
extern rtx emit_call_insn		(rtx) ;
extern rtx emit_label			(rtx) ;
extern rtx emit_barrier			(void) ;
extern rtx emit_line_note		(char *, int) ;
extern rtx emit_note			(char *, int) ;
extern rtx emit_line_note_force		(char *, int) ;
extern rtx make_insn_raw		(rtx) ;
extern rtx previous_insn		(rtx) ;
extern rtx next_insn			(rtx) ;
extern rtx prev_nonnote_insn		(rtx) ;
extern rtx next_nonnote_insn		(rtx) ;
extern rtx prev_real_insn		(rtx) ;
extern rtx next_real_insn		(rtx) ;
extern rtx prev_active_insn		(rtx) ;
extern rtx next_active_insn		(rtx) ;
extern rtx prev_label			(rtx) ;
extern rtx next_label			(rtx) ;
extern rtx next_cc0_user		(rtx) ;
extern rtx prev_cc0_setter		(rtx) ;
extern rtx reg_set_last			(rtx, rtx) ;
extern rtx next_nondeleted_insn		(rtx) ;
extern enum rtx_code reverse_condition	(enum rtx_code) ;
extern enum rtx_code swap_condition	(enum rtx_code) ;
extern enum rtx_code unsigned_condition	(enum rtx_code) ;
extern enum rtx_code signed_condition	(enum rtx_code) ;
extern rtx find_equiv_reg		(rtx, rtx, enum reg_class, int, short *, int, enum machine_mode) ;
extern rtx squeeze_notes		(rtx, rtx) ;
extern rtx delete_insn			(rtx) ;
extern void delete_jump			(rtx) ;
extern rtx get_label_before		(rtx) ;
extern rtx get_label_after		(rtx) ;
extern rtx follow_jumps			(rtx) ;
extern rtx adj_offsettable_operand	(rtx, int) ;
extern rtx try_split			(rtx, rtx, int) ;
extern rtx split_insns			(rtx, rtx) ;
extern rtx simplify_unary_operation	(enum rtx_code, enum machine_mode, rtx, enum machine_mode) ;
extern rtx simplify_binary_operation	(enum rtx_code, enum machine_mode, rtx, rtx) ;
extern rtx simplify_ternary_operation	(enum rtx_code, enum machine_mode, enum machine_mode, rtx, rtx, rtx) ;
extern rtx simplify_relational_operation (enum rtx_code, enum machine_mode, rtx, rtx) ;
extern rtx nonlocal_label_rtx_list	(void) ;
extern rtx gen_move_insn		(rtx, rtx) ;
extern rtx gen_jump			(rtx) ;
extern rtx gen_beq			(rtx) ;
extern rtx gen_bge			(rtx) ;
extern rtx gen_ble			(rtx) ;
extern rtx eliminate_constant_term	(rtx, rtx *) ;
extern rtx expand_complex_abs		(enum machine_mode, rtx, rtx, int) ;
extern int max_parallel;
extern int asm_noperands		(rtx) ;
extern char *decode_asm_operands	(rtx, rtx *, rtx **, char **, enum machine_mode *) ;
extern enum reg_class reg_preferred_class (int) ;
extern enum reg_class reg_alternate_class (int) ;
extern rtx get_first_nonparm_insn	(void) ;
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
extern rtx find_next_ref		(rtx, rtx) ;
extern rtx *find_single_use		(rtx, rtx, rtx *) ;
extern rtx expand_expr ();
extern rtx immed_real_const_1();
extern rtx output_constant_def ();
extern rtx immed_real_const ();
extern rtx immed_real_const_1 ();
extern int reload_completed;
extern int reload_in_progress;
extern int cse_not_expected;
extern rtx *regno_reg_rtx;
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
  int  int_cst_low;
  int  int_cst_high;
};
extern double ldexp ();
extern double (atof) ();
extern double  real_value_truncate ();
extern double  dconst0;
extern double  dconst1;
extern double  dconst2;
extern double  dconstm1;
union real_extract 
{
  double  d;
  int  i[sizeof (double ) / sizeof (int )];
};
double  real_value_from_int_cst ();
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
extern char *oballoc			(int) ;
extern char *permalloc			(int) ;
extern char *savealloc			(int) ;
extern char *xmalloc			(unsigned) ;
extern void free			(void *) ;
extern tree make_node			(enum tree_code) ;
extern tree copy_node			(tree) ;
extern tree copy_list			(tree) ;
extern tree make_tree_vec		(int) ;
extern tree get_identifier		(char *) ;
extern tree build ();
extern tree build_nt ();
extern tree build_parse_node ();
extern tree build_int_2_wide		(int , int ) ;
extern tree build_real			(tree, double ) ;
extern tree build_real_from_int_cst 	(tree, tree) ;
extern tree build_complex		(tree, tree) ;
extern tree build_string		(int, char *) ;
extern tree build1			(enum tree_code, tree, tree) ;
extern tree build_tree_list		(tree, tree) ;
extern tree build_decl_list		(tree, tree) ;
extern tree build_decl			(enum tree_code, tree, tree) ;
extern tree build_block			(tree, tree, tree, tree, tree) ;
extern tree make_signed_type		(int) ;
extern tree make_unsigned_type		(int) ;
extern tree signed_or_unsigned_type 	(int, tree) ;
extern void fixup_unsigned_type		(tree) ;
extern tree build_pointer_type		(tree) ;
extern tree build_reference_type 	(tree) ;
extern tree build_index_type		(tree) ;
extern tree build_index_2_type		(tree, tree) ;
extern tree build_array_type		(tree, tree) ;
extern tree build_function_type		(tree, tree) ;
extern tree build_method_type		(tree, tree) ;
extern tree build_offset_type		(tree, tree) ;
extern tree build_complex_type		(tree) ;
extern tree array_type_nelts		(tree) ;
extern tree build_binary_op ();
extern tree build_indirect_ref ();
extern tree build_unary_op		(enum tree_code, tree, int) ;
extern tree make_tree ();
extern tree build_type_variant		(tree, int, int) ;
extern tree build_type_copy		(tree) ;
extern void layout_type			(tree) ;
extern tree type_hash_canon		(int, tree) ;
extern void layout_decl			(tree, unsigned) ;
extern tree fold			(tree) ;
extern tree non_lvalue			(tree) ;
extern tree convert			(tree, tree) ;
extern tree size_in_bytes		(tree) ;
extern int int_size_in_bytes		(tree) ;
extern tree size_binop			(enum tree_code, tree, tree) ;
extern tree size_int			(unsigned) ;
extern tree round_up			(tree, int) ;
extern tree get_pending_sizes		(void) ;
extern tree sizetype;
extern tree chainon			(tree, tree) ;
extern tree tree_cons			(tree, tree, tree) ;
extern tree perm_tree_cons		(tree, tree, tree) ;
extern tree temp_tree_cons		(tree, tree, tree) ;
extern tree saveable_tree_cons		(tree, tree, tree) ;
extern tree decl_tree_cons		(tree, tree, tree) ;
extern tree tree_last			(tree) ;
extern tree nreverse			(tree) ;
extern int list_length			(tree) ;
extern int integer_zerop		(tree) ;
extern int integer_onep			(tree) ;
extern int integer_all_onesp		(tree) ;
extern int integer_pow2p		(tree) ;
extern int staticp			(tree) ;
extern int lvalue_or_else		(tree, char *) ;
extern tree save_expr			(tree) ;
extern tree variable_size		(tree) ;
extern tree stabilize_reference		(tree) ;
extern tree get_unwidened		(tree, tree) ;
extern tree get_narrower		(tree, int *) ;
extern tree type_for_mode		(enum machine_mode, int) ;
extern tree type_for_size		(unsigned, int) ;
extern tree unsigned_type		(tree) ;
extern tree signed_type			(tree) ;
extern tree maybe_build_cleanup		(tree) ;
extern tree get_inner_reference		(tree, int *, int *, tree *, enum machine_mode *, int *, int *) ;
extern tree decl_function_context 	(tree) ;
extern tree decl_type_context		(tree) ;
extern char *function_cannot_inline_p 	(tree) ;
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
extern char *perm_calloc			(int, long) ;
extern tree expand_start_stmt_expr		(void) ;
extern tree expand_end_stmt_expr		(tree) ;
extern void expand_expr_stmt			(tree) ;
extern void clear_last_expr			(void) ;
extern void expand_label			(tree) ;
extern void expand_goto				(tree) ;
extern void expand_asm				(tree) ;
extern void expand_start_cond			(tree, int) ;
extern void expand_end_cond			(void) ;
extern void expand_start_else			(void) ;
extern void expand_start_elseif			(tree) ;
extern struct nesting *expand_start_loop 	(int) ;
extern struct nesting *expand_start_loop_continue_elsewhere 	(int) ;
extern void expand_loop_continue_here		(void) ;
extern void expand_end_loop			(void) ;
extern int expand_continue_loop			(struct nesting *) ;
extern int expand_exit_loop			(struct nesting *) ;
extern int expand_exit_loop_if_false		(struct nesting *, tree) ;
extern int expand_exit_something		(void) ;
extern void expand_null_return			(void) ;
extern void expand_return			(tree) ;
extern void expand_start_bindings		(int) ;
extern void expand_end_bindings			(tree, int, int) ;
extern tree last_cleanup_this_contour		(void) ;
extern void expand_start_case			(int, tree, tree, char *) ;
extern void expand_end_case			(tree) ;
extern int pushcase				(tree, tree, tree *) ;
extern int pushcase_range			(tree, tree, tree, tree *) ;
extern tree invert_truthvalue			(tree) ;
extern void init_lex				(void) ;
extern void init_decl_processing		(void) ;
extern void lang_init				(void) ;
extern void lang_finish				(void) ;
extern int yyparse				(void) ;
extern int lang_decode_option			(char *) ;
extern void pushlevel				(int) ;
extern tree poplevel				(int, int, int) ;
extern void set_block				(tree) ;
extern tree pushdecl				(tree) ;
extern tree getdecls				(void) ;
extern tree gettags				(void) ;
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
struct var_refs_queue
  {
    rtx modified;
    enum machine_mode promoted_mode;
    int unsignedp;
    struct var_refs_queue *next;
  };
struct sequence_stack
{
  rtx first, last;
  struct sequence_stack *next;
};
extern struct sequence_stack *sequence_stack;
struct function
{
  struct function *next;
  char *name;
  tree decl;
  int pops_args;
  int returns_struct;
  int returns_pcc_struct;
  int needs_context;
  int calls_setjmp;
  int calls_longjmp;
  int calls_alloca;
  int has_nonlocal_label;
  rtx nonlocal_goto_handler_slot;
  rtx nonlocal_goto_stack_level;
  tree nonlocal_labels;
  int args_size;
  int pretend_args_size;
  rtx arg_offset_rtx;
  int max_parm_reg;
  rtx *parm_reg_stack_loc;
  int outgoing_args_size;
  rtx return_rtx;
  rtx cleanup_label;
  rtx return_label;
  rtx save_expr_regs;
  rtx stack_slot_list;
  rtx parm_birth_insn;
  int frame_offset;
  rtx tail_recursion_label;
  rtx tail_recursion_reentry;
  rtx internal_arg_pointer;
  rtx arg_pointer_save_area;
  tree rtl_expr_chain;
  rtx last_parm_insn;
  tree context_display;
  tree trampoline_list;
  int function_call_count;
  struct temp_slot *temp_slots;
  int temp_slot_level;
  struct var_refs_queue *fixup_var_refs_queue;
  struct nesting *block_stack;
  struct nesting *stack_block_stack;
  struct nesting *cond_stack;
  struct nesting *loop_stack;
  struct nesting *case_stack;
  struct nesting *nesting_stack;
  int nesting_depth;
  int block_start_count;
  tree last_expr_type;
  rtx last_expr_value;
  int expr_stmts_for_value;
  char *emit_filename;
  int emit_lineno;
  struct goto_fixup *goto_fixup_chain;
  int pending_stack_adjust;
  int inhibit_defer_pop;
  tree cleanups_this_call;
  rtx saveregs_value;
  rtx forced_labels;
  int reg_rtx_no;
  int first_label_num;
  rtx first_insn;
  rtx last_insn;
  struct sequence_stack *sequence_stack;
  int cur_insn_uid;
  int last_linenum;
  char *last_filename;
  char *regno_pointer_flag;
  int regno_pointer_flag_length;
  rtx *regno_reg_rtx;
  tree permanent_type_chain;
  tree temporary_type_chain;
  tree permanent_type_end;
  tree temporary_type_end;
  tree pending_sizes;
  int immediate_size_expand;
  int all_types_permanent;
  struct momentary_level *momentary_stack;
  char *maybepermanent_firstobj;
  char *temporary_firstobj;
  char *momentary_firstobj;
  struct obstack *current_obstack;
  struct obstack *function_obstack;
  struct obstack *function_maybepermanent_obstack;
  struct obstack *expression_obstack;
  struct obstack *saveable_obstack;
  struct obstack *rtl_obstack;
  int uses_const_pool;
  int uses_pic_offset_table;
  rtx epilogue_delay_list;
  struct constant_descriptor **const_rtx_hash_table;
  struct pool_sym **const_rtx_sym_hash_table;
  struct pool_constant *first_pool, *last_pool;
  int pool_offset;
};
extern tree inline_function_decl;
extern rtx return_label;
extern rtx stack_slot_list;
struct function *find_function_data ();
extern struct function *outer_function_chain;
tree *identify_blocks ();
extern rtx gen_cmpsi            (rtx, rtx) ;
extern rtx gen_cmpsf            (rtx, rtx) ;
extern rtx gen_cmpdf            (rtx, rtx) ;
extern rtx gen_cmptf            (rtx, rtx) ;
extern rtx gen_seq_special      (rtx, rtx, rtx) ;
extern rtx gen_sne_special      (rtx, rtx, rtx) ;
extern rtx gen_seq              (rtx) ;
extern rtx gen_sne              (rtx) ;
extern rtx gen_sgt              (rtx) ;
extern rtx gen_slt              (rtx) ;
extern rtx gen_sge              (rtx) ;
extern rtx gen_sle              (rtx) ;
extern rtx gen_sgtu             (rtx) ;
extern rtx gen_sltu             (rtx) ;
extern rtx gen_sgeu             (rtx) ;
extern rtx gen_sleu             (rtx) ;
extern rtx gen_beq              (rtx) ;
extern rtx gen_bne              (rtx) ;
extern rtx gen_bgt              (rtx) ;
extern rtx gen_bgtu             (rtx) ;
extern rtx gen_blt              (rtx) ;
extern rtx gen_bltu             (rtx) ;
extern rtx gen_bge              (rtx) ;
extern rtx gen_bgeu             (rtx) ;
extern rtx gen_ble              (rtx) ;
extern rtx gen_bleu             (rtx) ;
extern rtx gen_movsi            (rtx, rtx) ;
extern rtx gen_reload_insi      (rtx, rtx, rtx) ;
extern rtx gen_movhi            (rtx, rtx) ;
extern rtx gen_movqi            (rtx, rtx) ;
extern rtx gen_movstrsi         (rtx, rtx, rtx, rtx) ;
extern rtx gen_movtf            (rtx, rtx) ;
extern rtx gen_movdf            (rtx, rtx) ;
extern rtx gen_movdi            (rtx, rtx) ;
extern rtx gen_movsf            (rtx, rtx) ;
extern rtx gen_zero_extendhisi2 (rtx, rtx) ;
extern rtx gen_zero_extendqihi2 (rtx, rtx) ;
extern rtx gen_zero_extendqisi2 (rtx, rtx) ;
extern rtx gen_extendhisi2      (rtx, rtx) ;
extern rtx gen_extendqihi2      (rtx, rtx) ;
extern rtx gen_extendqisi2      (rtx, rtx) ;
extern rtx gen_extendsfdf2      (rtx, rtx) ;
extern rtx gen_extendsftf2      (rtx, rtx) ;
extern rtx gen_extenddftf2      (rtx, rtx) ;
extern rtx gen_truncdfsf2       (rtx, rtx) ;
extern rtx gen_trunctfsf2       (rtx, rtx) ;
extern rtx gen_trunctfdf2       (rtx, rtx) ;
extern rtx gen_floatsisf2       (rtx, rtx) ;
extern rtx gen_floatsidf2       (rtx, rtx) ;
extern rtx gen_floatsitf2       (rtx, rtx) ;
extern rtx gen_fix_truncsfsi2   (rtx, rtx) ;
extern rtx gen_fix_truncdfsi2   (rtx, rtx) ;
extern rtx gen_fix_trunctfsi2   (rtx, rtx) ;
extern rtx gen_adddi3           (rtx, rtx, rtx) ;
extern rtx gen_addsi3           (rtx, rtx, rtx) ;
extern rtx gen_subdi3           (rtx, rtx, rtx) ;
extern rtx gen_subsi3           (rtx, rtx, rtx) ;
extern rtx gen_mulsi3           (rtx, rtx, rtx) ;
extern rtx gen_mulsidi3         (rtx, rtx, rtx) ;
extern rtx gen_umulsidi3        (rtx, rtx, rtx) ;
extern rtx gen_divsi3           (rtx, rtx, rtx) ;
extern rtx gen_udivsi3          (rtx, rtx, rtx) ;
extern rtx gen_anddi3           (rtx, rtx, rtx) ;
extern rtx gen_andsi3           (rtx, rtx, rtx) ;
extern rtx gen_iordi3           (rtx, rtx, rtx) ;
extern rtx gen_iorsi3           (rtx, rtx, rtx) ;
extern rtx gen_xordi3           (rtx, rtx, rtx) ;
extern rtx gen_xorsi3           (rtx, rtx, rtx) ;
extern rtx gen_negdi2           (rtx, rtx) ;
extern rtx gen_negsi2           (rtx, rtx) ;
extern rtx gen_one_cmpldi2      (rtx, rtx) ;
extern rtx gen_one_cmplsi2      (rtx, rtx) ;
extern rtx gen_addtf3           (rtx, rtx, rtx) ;
extern rtx gen_adddf3           (rtx, rtx, rtx) ;
extern rtx gen_addsf3           (rtx, rtx, rtx) ;
extern rtx gen_subtf3           (rtx, rtx, rtx) ;
extern rtx gen_subdf3           (rtx, rtx, rtx) ;
extern rtx gen_subsf3           (rtx, rtx, rtx) ;
extern rtx gen_multf3           (rtx, rtx, rtx) ;
extern rtx gen_muldf3           (rtx, rtx, rtx) ;
extern rtx gen_mulsf3           (rtx, rtx, rtx) ;
extern rtx gen_divtf3           (rtx, rtx, rtx) ;
extern rtx gen_divdf3           (rtx, rtx, rtx) ;
extern rtx gen_divsf3           (rtx, rtx, rtx) ;
extern rtx gen_negtf2           (rtx, rtx) ;
extern rtx gen_negdf2           (rtx, rtx) ;
extern rtx gen_negsf2           (rtx, rtx) ;
extern rtx gen_abstf2           (rtx, rtx) ;
extern rtx gen_absdf2           (rtx, rtx) ;
extern rtx gen_abssf2           (rtx, rtx) ;
extern rtx gen_sqrttf2          (rtx, rtx) ;
extern rtx gen_sqrtdf2          (rtx, rtx) ;
extern rtx gen_sqrtsf2          (rtx, rtx) ;
extern rtx gen_ashldi3          (rtx, rtx, rtx) ;
extern rtx gen_ashlsi3          (rtx, rtx, rtx) ;
extern rtx gen_ashrsi3          (rtx, rtx, rtx) ;
extern rtx gen_lshrsi3          (rtx, rtx, rtx) ;
extern rtx gen_jump             (rtx) ;
extern rtx gen_tablejump        (rtx, rtx) ;
extern rtx gen_pic_tablejump    (rtx, rtx) ;
extern rtx gen_return           (void) ;
extern rtx gen_nop              (void) ;
extern rtx gen_indirect_jump    (rtx) ;
extern rtx gen_nonlocal_goto    (rtx, rtx, rtx, rtx) ;
extern rtx gen_call ();
extern rtx gen_call_value ();
enum insn_code {
  CODE_FOR_cmpsi = 0,
  CODE_FOR_cmpsf = 1,
  CODE_FOR_cmpdf = 2,
  CODE_FOR_cmptf = 3,
  CODE_FOR_seq_special = 4,
  CODE_FOR_sne_special = 5,
  CODE_FOR_seq = 6,
  CODE_FOR_sne = 7,
  CODE_FOR_sgt = 8,
  CODE_FOR_slt = 9,
  CODE_FOR_sge = 10,
  CODE_FOR_sle = 11,
  CODE_FOR_sgtu = 12,
  CODE_FOR_sltu = 13,
  CODE_FOR_sgeu = 14,
  CODE_FOR_sleu = 15,
  CODE_FOR_beq = 45,
  CODE_FOR_bne = 46,
  CODE_FOR_bgt = 47,
  CODE_FOR_bgtu = 48,
  CODE_FOR_blt = 49,
  CODE_FOR_bltu = 50,
  CODE_FOR_bge = 51,
  CODE_FOR_bgeu = 52,
  CODE_FOR_ble = 53,
  CODE_FOR_bleu = 54,
  CODE_FOR_movsi = 57,
  CODE_FOR_reload_insi = 58,
  CODE_FOR_movhi = 69,
  CODE_FOR_movqi = 73,
  CODE_FOR_movstrsi = 77,
  CODE_FOR_movtf = 80,
  CODE_FOR_movdf = 84,
  CODE_FOR_movdi = 87,
  CODE_FOR_movsf = 90,
  CODE_FOR_zero_extendhisi2 = 93,
  CODE_FOR_zero_extendqihi2 = 95,
  CODE_FOR_zero_extendqisi2 = 97,
  CODE_FOR_extendhisi2 = 101,
  CODE_FOR_extendqihi2 = 103,
  CODE_FOR_extendqisi2 = 105,
  CODE_FOR_extendsfdf2 = 108,
  CODE_FOR_extendsftf2 = 109,
  CODE_FOR_extenddftf2 = 110,
  CODE_FOR_truncdfsf2 = 111,
  CODE_FOR_trunctfsf2 = 112,
  CODE_FOR_trunctfdf2 = 113,
  CODE_FOR_floatsisf2 = 114,
  CODE_FOR_floatsidf2 = 115,
  CODE_FOR_floatsitf2 = 116,
  CODE_FOR_fix_truncsfsi2 = 117,
  CODE_FOR_fix_truncdfsi2 = 118,
  CODE_FOR_fix_trunctfsi2 = 119,
  CODE_FOR_adddi3 = 120,
  CODE_FOR_addsi3 = 121,
  CODE_FOR_subdi3 = 124,
  CODE_FOR_subsi3 = 125,
  CODE_FOR_mulsi3 = 128,
  CODE_FOR_mulsidi3 = 130,
  CODE_FOR_umulsidi3 = 131,
  CODE_FOR_divsi3 = 132,
  CODE_FOR_udivsi3 = 134,
  CODE_FOR_anddi3 = 136,
  CODE_FOR_andsi3 = 138,
  CODE_FOR_iordi3 = 142,
  CODE_FOR_iorsi3 = 144,
  CODE_FOR_xordi3 = 148,
  CODE_FOR_xorsi3 = 150,
  CODE_FOR_negdi2 = 161,
  CODE_FOR_negsi2 = 162,
  CODE_FOR_one_cmpldi2 = 165,
  CODE_FOR_one_cmplsi2 = 167,
  CODE_FOR_addtf3 = 170,
  CODE_FOR_adddf3 = 171,
  CODE_FOR_addsf3 = 172,
  CODE_FOR_subtf3 = 173,
  CODE_FOR_subdf3 = 174,
  CODE_FOR_subsf3 = 175,
  CODE_FOR_multf3 = 176,
  CODE_FOR_muldf3 = 177,
  CODE_FOR_mulsf3 = 178,
  CODE_FOR_divtf3 = 179,
  CODE_FOR_divdf3 = 180,
  CODE_FOR_divsf3 = 181,
  CODE_FOR_negtf2 = 182,
  CODE_FOR_negdf2 = 183,
  CODE_FOR_negsf2 = 184,
  CODE_FOR_abstf2 = 185,
  CODE_FOR_absdf2 = 186,
  CODE_FOR_abssf2 = 187,
  CODE_FOR_sqrttf2 = 188,
  CODE_FOR_sqrtdf2 = 189,
  CODE_FOR_sqrtsf2 = 190,
  CODE_FOR_ashldi3 = 191,
  CODE_FOR_ashlsi3 = 193,
  CODE_FOR_ashrsi3 = 194,
  CODE_FOR_lshrsi3 = 195,
  CODE_FOR_jump = 196,
  CODE_FOR_tablejump = 197,
  CODE_FOR_pic_tablejump = 198,
  CODE_FOR_call = 201,
  CODE_FOR_call_value = 204,
  CODE_FOR_return = 206,
  CODE_FOR_nop = 207,
  CODE_FOR_indirect_jump = 208,
  CODE_FOR_nonlocal_goto = 209,
  CODE_FOR_nothing };
enum expand_modifier {EXPAND_NORMAL, EXPAND_SUM,
		      EXPAND_CONST_ADDRESS, EXPAND_INITIALIZER};
extern rtx forced_labels;
extern rtx save_expr_regs;
extern int current_function_calls_alloca;
extern int current_function_outgoing_args_size;
extern rtx current_function_arg_offset_rtx;
extern int current_function_uses_const_pool;
extern int current_function_uses_pic_offset_table;
extern rtx current_function_internal_arg_pointer;
extern int inhibit_defer_pop;
extern int function_call_count;
extern rtx nonlocal_goto_handler_slot;
extern rtx nonlocal_goto_stack_level;
extern tree nonlocal_labels;
extern int pending_stack_adjust;
extern tree cleanups_this_call;
struct args_size
{
  int constant;
  tree var;
};
enum direction {none, upward, downward};   
typedef struct optab
{
  enum rtx_code code;
  struct {
    enum insn_code insn_code;
    rtx libfunc;
  } handlers [(int) MAX_MACHINE_MODE ];
} * optab;
extern rtx (*const insn_gen_function[]) ();
extern optab add_optab;
extern optab sub_optab;
extern optab smul_optab;	 
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
extern optab ior_optab;		 
extern optab xor_optab;		 
extern optab ashl_optab;	 
extern optab ashr_optab;	 
extern optab lshl_optab;	 
extern optab lshr_optab;	 
extern optab rotl_optab;	 
extern optab rotr_optab;	 
extern optab smin_optab;	 
extern optab smax_optab;	 
extern optab umin_optab;	 
extern optab umax_optab;	 
extern optab mov_optab;		 
extern optab movstrict_optab;	 
extern optab cmp_optab;		 
extern optab tst_optab;		 
extern optab neg_optab;		 
extern optab abs_optab;		 
extern optab one_cmpl_optab;	 
extern optab ffs_optab;		 
extern optab sqrt_optab;	 
extern optab sin_optab;		 
extern optab cos_optab;		 
extern optab strlen_optab;	 
enum optab_methods
{
  OPTAB_DIRECT,
  OPTAB_LIB,
  OPTAB_WIDEN,
  OPTAB_LIB_WIDEN,
  OPTAB_MUST_WIDEN
};
extern rtx extendsfdf2_libfunc;
extern rtx extendsfxf2_libfunc;
extern rtx extendsftf2_libfunc;
extern rtx extenddfxf2_libfunc;
extern rtx extenddftf2_libfunc;
extern rtx truncdfsf2_libfunc;
extern rtx truncxfsf2_libfunc;
extern rtx trunctfsf2_libfunc;
extern rtx truncxfdf2_libfunc;
extern rtx trunctfdf2_libfunc;
extern rtx memcpy_libfunc;
extern rtx bcopy_libfunc;
extern rtx memcmp_libfunc;
extern rtx bcmp_libfunc;
extern rtx memset_libfunc;
extern rtx bzero_libfunc;
extern rtx eqsf2_libfunc;
extern rtx nesf2_libfunc;
extern rtx gtsf2_libfunc;
extern rtx gesf2_libfunc;
extern rtx ltsf2_libfunc;
extern rtx lesf2_libfunc;
extern rtx eqdf2_libfunc;
extern rtx nedf2_libfunc;
extern rtx gtdf2_libfunc;
extern rtx gedf2_libfunc;
extern rtx ltdf2_libfunc;
extern rtx ledf2_libfunc;
extern rtx eqxf2_libfunc;
extern rtx nexf2_libfunc;
extern rtx gtxf2_libfunc;
extern rtx gexf2_libfunc;
extern rtx ltxf2_libfunc;
extern rtx lexf2_libfunc;
extern rtx eqtf2_libfunc;
extern rtx netf2_libfunc;
extern rtx gttf2_libfunc;
extern rtx getf2_libfunc;
extern rtx lttf2_libfunc;
extern rtx letf2_libfunc;
extern rtx floatsisf_libfunc;
extern rtx floatdisf_libfunc;
extern rtx floattisf_libfunc;
extern rtx floatsidf_libfunc;
extern rtx floatdidf_libfunc;
extern rtx floattidf_libfunc;
extern rtx floatsixf_libfunc;
extern rtx floatdixf_libfunc;
extern rtx floattixf_libfunc;
extern rtx floatsitf_libfunc;
extern rtx floatditf_libfunc;
extern rtx floattitf_libfunc;
extern rtx fixsfsi_libfunc;
extern rtx fixsfdi_libfunc;
extern rtx fixsfti_libfunc;
extern rtx fixdfsi_libfunc;
extern rtx fixdfdi_libfunc;
extern rtx fixdfti_libfunc;
extern rtx fixxfsi_libfunc;
extern rtx fixxfdi_libfunc;
extern rtx fixxfti_libfunc;
extern rtx fixtfsi_libfunc;
extern rtx fixtfdi_libfunc;
extern rtx fixtfti_libfunc;
extern rtx fixunssfsi_libfunc;
extern rtx fixunssfdi_libfunc;
extern rtx fixunssfti_libfunc;
extern rtx fixunsdfsi_libfunc;
extern rtx fixunsdfdi_libfunc;
extern rtx fixunsdfti_libfunc;
extern rtx fixunsxfsi_libfunc;
extern rtx fixunsxfdi_libfunc;
extern rtx fixunsxfti_libfunc;
extern rtx fixunstfsi_libfunc;
extern rtx fixunstfdi_libfunc;
extern rtx fixunstfti_libfunc;
typedef rtx (*rtxfun) ();
extern rtxfun bcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE) ];
extern enum insn_code setcc_gen_code[((int)LAST_AND_UNUSED_RTX_CODE) ];
extern rtx expand_binop ();
extern rtx sign_expand_binop ();
extern rtx expand_unop ();
extern rtx expand_complex_abs ();
extern rtx negate_rtx ();
extern rtx expand_and ();
extern rtx emit_store_flag ();
extern rtx label_rtx ();
extern rtx get_condition ();
extern enum insn_code can_extend_p ();
extern void init_fixtab ();
extern void init_floattab ();
extern void expand_fix ();
extern void expand_float ();
extern rtx gen_add2_insn ();
extern rtx gen_sub2_insn ();
extern rtx gen_move_insn ();
extern void emit_clr_insn ();
extern void emit_0_to_1_insn ();
extern void emit_cmp_insn ();
extern rtx compare_from_rtx ();
extern void convert_move ();
extern rtx convert_to_mode ();
extern void emit_library_call ();
extern rtx force_operand ();
extern rtx expr_size ();
extern rtx lookup_static_chain ();
extern rtx eliminate_constant_term ();
extern rtx memory_address ();
extern rtx memory_address_noforce ();
extern rtx change_address ();
extern rtx validize_mem ();
extern rtx fix_lexical_addr ();
extern rtx trampoline_address ();
extern rtx assemble_trampoline_template ();
extern int rtx_equal_p ();
extern rtx stabilize ();
extern rtx copy_all_regs ();
extern rtx copy_to_reg ();
extern rtx copy_addr_to_reg ();
extern rtx copy_to_mode_reg ();
extern rtx copy_to_suggested_reg ();
extern rtx force_reg ();
extern rtx force_not_mem ();
extern void adjust_stack ();
extern void anti_adjust_stack ();
enum save_level {SAVE_BLOCK, SAVE_FUNCTION, SAVE_NONLOCAL};
extern void emit_stack_save ();
extern void emit_stack_restore ();
extern rtx allocate_dynamic_stack_space ();
extern rtx function_value ();
extern rtx hard_function_value ();
extern rtx hard_libcall_value ();
extern void copy_function_value ();
extern rtx round_push ();
extern rtx push_block ();
extern rtx store_expr ();
extern rtx prepare_call_address ();
extern rtx expand_call ();
extern void emit_call_1 ();
extern void emit_block_move ();
extern void emit_push_insn ();
extern void use_regs ();
extern void move_block_to_reg ();
extern rtx store_bit_field ();
extern rtx extract_bit_field ();
extern rtx expand_shift ();
extern rtx expand_mult ();
extern rtx expand_divmod ();
extern rtx expand_mult_add ();
extern rtx expand_stmt_expr ();
extern rtx emit_no_conflict_block ();
extern void emit_libcall_block ();
extern void jumpifnot ();
extern void jumpif ();
extern void do_jump ();
extern rtx assemble_static_space ();
extern void locate_and_pad_parm ();
extern rtx expand_inline_function ();
extern rtx (*lang_expand_expr) ();
extern int recog_memoized ();
extern int validate_change ();
extern int apply_change_group ();
extern int num_validated_changes ();
extern void cancel_changes ();
extern int volatile_ok;
extern void insn_extract ();
extern rtx recog_operand[];
extern rtx *recog_operand_loc[];
extern rtx *recog_dup_loc[];
extern char recog_dup_num[];
extern char *const insn_template[];
extern char *(*const insn_outfun[]) ();
extern const int insn_n_operands[];
extern const int insn_n_dups[];
extern const int insn_n_alternatives[];
extern char *const insn_operand_constraint[][10 ];
extern const enum machine_mode insn_operand_mode[][10 ];
extern const char insn_operand_strict_low[][10 ];
extern int (*const insn_operand_predicate[][10 ]) ();
extern char * insn_name[];
extern void output_asm_insn ();
extern void asm_fprintf ();
extern void output_addr_const ();
extern void assemble_name ();
extern rtx alter_subreg ();
extern int which_alternative;
extern rtx final_sequence;
extern int current_function_pops_args;
extern int current_function_returns_struct;
extern int current_function_returns_pcc_struct;
extern int current_function_needs_context;
extern int current_function_calls_setjmp;
extern int current_function_calls_longjmp;
extern int current_function_calls_alloca;
extern int current_function_has_nonlocal_label;
extern int current_function_contains_functions;
extern int current_function_returns_pointer;
extern int current_function_args_size;
extern int current_function_pretend_args_size;
extern int current_function_outgoing_args_size;
extern int current_function_varargs;
extern int  current_function_args_info;
extern char *current_function_name;
extern rtx current_function_return_rtx;
extern rtx current_function_epilogue_delay_list;
extern int flag_pic;
extern int current_function_uses_pic_offset_table;
extern int current_function_uses_const_pool;
enum type_class
{
  no_type_class = -1,
  void_type_class, integer_type_class, char_type_class,
  enumeral_type_class, boolean_type_class,
  pointer_type_class, reference_type_class, offset_type_class,
  real_type_class, complex_type_class,
  function_type_class, method_type_class,
  record_type_class, union_type_class,
  array_type_class, string_type_class, set_type_class, file_type_class,
  lang_type_class
};
int cse_not_expected;
int do_preexpand_calls = 1;
int pending_stack_adjust;
int inhibit_defer_pop;
tree cleanups_this_call;
static rtx saveregs_value;
rtx store_expr ();
static void store_constructor ();
static rtx store_field ();
static rtx expand_builtin ();
static rtx compare ();
static rtx do_store_flag ();
static void preexpand_calls ();
static rtx expand_increment ();
static void init_queue ();
void do_pending_stack_adjust ();
static void do_jump_for_compare ();
static void do_jump_by_parts_equality ();
static void do_jump_by_parts_equality_rtx ();
static void do_jump_by_parts_greater ();
static char direct_load[(int) MAX_MACHINE_MODE ];
static char direct_store[(int) MAX_MACHINE_MODE ];
static enum insn_code movstr_optab[(int) MAX_MACHINE_MODE ];
void
init_expr_once ()
{
  rtx insn, pat;
  enum machine_mode mode;
  rtx mem = gen_rtx (MEM, VOIDmode, stack_pointer_rtx);
  rtx mem1 = gen_rtx (MEM, VOIDmode, frame_pointer_rtx);
  start_sequence ();
  insn = emit_insn (gen_rtx (SET, 0, 0));
  pat = ((insn)->fld[3].rtx) ;
  for (mode = VOIDmode; (int) mode < (int) MAX_MACHINE_MODE ;
       mode = (enum machine_mode) ((int) mode + 1))
    {
      int regno;
      rtx reg;
      int num_clobbers;
      direct_load[(int) mode] = direct_store[(int) mode] = 0;
      ((mem)->mode = ( mode)) ;
      ((mem1)->mode = ( mode)) ;
      if (mode != VOIDmode && mode != BLKmode)
	for (regno = 0; regno < 64 
	     && (direct_load[(int) mode] == 0 || direct_store[(int) mode] == 0);
	     regno++)
	  {
	    if (! ((hard_regno_mode_ok[regno] & (1<<(int)( mode))) != 0) )
	      continue;
	    reg = gen_rtx (REG, mode, regno);
	    ((pat)->fld[1].rtx)  = mem;
	    ((pat)->fld[0].rtx)  = reg;
	    if (recog (pat, insn, &num_clobbers) >= 0)
	      direct_load[(int) mode] = 1;
	    ((pat)->fld[1].rtx)  = mem1;
	    ((pat)->fld[0].rtx)  = reg;
	    if (recog (pat, insn, &num_clobbers) >= 0)
	      direct_load[(int) mode] = 1;
	    ((pat)->fld[1].rtx)  = reg;
	    ((pat)->fld[0].rtx)  = mem;
	    if (recog (pat, insn, &num_clobbers) >= 0)
	      direct_store[(int) mode] = 1;
	    ((pat)->fld[1].rtx)  = reg;
	    ((pat)->fld[0].rtx)  = mem1;
	    if (recog (pat, insn, &num_clobbers) >= 0)
	      direct_store[(int) mode] = 1;
	  }
      movstr_optab[(int) mode] = CODE_FOR_nothing;
    }
  end_sequence ();
  if (1 )
    movstr_optab[(int) SImode] = CODE_FOR_movstrsi;
}
void
init_expr ()
{
  init_queue ();
  pending_stack_adjust = 0;
  inhibit_defer_pop = 0;
  cleanups_this_call = 0;
  saveregs_value = 0;
  forced_labels = 0;
}
void
save_expr_status (p)
     struct function *p;
{
  emit_queue ();
  p->pending_stack_adjust = pending_stack_adjust;
  p->inhibit_defer_pop = inhibit_defer_pop;
  p->cleanups_this_call = cleanups_this_call;
  p->saveregs_value = saveregs_value;
  p->forced_labels = forced_labels;
  pending_stack_adjust = 0;
  inhibit_defer_pop = 0;
  cleanups_this_call = 0;
  saveregs_value = 0;
  forced_labels = 0;
}
void
restore_expr_status (p)
     struct function *p;
{
  pending_stack_adjust = p->pending_stack_adjust;
  inhibit_defer_pop = p->inhibit_defer_pop;
  cleanups_this_call = p->cleanups_this_call;
  saveregs_value = p->saveregs_value;
  forced_labels = p->forced_labels;
}
static rtx pending_chain;
static rtx
enqueue_insn (var, body)
     rtx var, body;
{
  pending_chain = gen_rtx (QUEUED, 	((var)->mode) ,
			   var, (rtx) 0 , (rtx) 0 , body, pending_chain);
  return pending_chain;
}
rtx
protect_from_queue (x, modify)
     register rtx x;
     int modify;
{
  register enum rtx_code  code = 	((x)->code) ;
  if (code != QUEUED)
    {
      if (code == MEM && 	((x)->mode)  != BLKmode
	  && 	((((x)->fld[ 0].rtx) )->code)  == QUEUED && !modify)
	{
	  register rtx y = ((x)->fld[ 0].rtx) ;
	  ((x)->fld[ 0].rtx)  = ((y)->fld[ 0].rtx)  ;
	  if (((y)->fld[ 1].rtx)  )
	    {
	      register rtx temp = gen_reg_rtx (	((x)->mode) );
	      emit_insn_before (gen_move_insn (temp, x),
				((y)->fld[ 1].rtx)  );
	      return temp;
	    }
	  return x;
	}
      if (code == MEM)
	((x)->fld[ 0].rtx)  = protect_from_queue (((x)->fld[ 0].rtx) , 0);
      else if (code == PLUS || code == MULT)
	{
	  ((x)->fld[ 0].rtx)  = protect_from_queue (((x)->fld[ 0].rtx) , 0);
	  ((x)->fld[ 1].rtx)  = protect_from_queue (((x)->fld[ 1].rtx) , 0);
	}
      return x;
    }
  if (((x)->fld[ 1].rtx)   == 0)
    return ((x)->fld[ 0].rtx)  ;
  if (((x)->fld[ 2].rtx)   != 0)
    return ((x)->fld[ 2].rtx)  ;
  ((x)->fld[ 2].rtx)   = gen_reg_rtx (	((((x)->fld[ 0].rtx)  )->mode) );
  emit_insn_before (gen_move_insn (((x)->fld[ 2].rtx)  , ((x)->fld[ 0].rtx)  ),
		    ((x)->fld[ 1].rtx)  );
  return ((x)->fld[ 2].rtx)  ;
}
static int
queued_subexp_p (x)
     rtx x;
{
  register enum rtx_code code = 	((x)->code) ;
  switch (code)
    {
    case QUEUED:
      return 1;
    case MEM:
      return queued_subexp_p (((x)->fld[ 0].rtx) );
    case MULT:
    case PLUS:
    case MINUS:
      return queued_subexp_p (((x)->fld[ 0].rtx) )
	|| queued_subexp_p (((x)->fld[ 1].rtx) );
    }
  return 0;
}
void
emit_queue ()
{
  register rtx p;
  while (p = pending_chain)
    {
      ((p)->fld[ 1].rtx)   = emit_insn (((p)->fld[ 3].rtx)  );
      pending_chain = ((p)->fld[ 4].rtx)  ;
    }
}
static void
init_queue ()
{
  if (pending_chain)
    abort ();
}
void
convert_move (to, from, unsignedp)
     register rtx to, from;
     int unsignedp;
{
  enum machine_mode to_mode = 	((to)->mode) ;
  enum machine_mode from_mode = 	((from)->mode) ;
  int to_real = 	(mode_class[(int)(to_mode)])  == MODE_FLOAT;
  int from_real = 	(mode_class[(int)(from_mode)])  == MODE_FLOAT;
  enum insn_code code;
  rtx libcall;
  enum rtx_code equiv_code = (unsignedp ? ZERO_EXTEND : SIGN_EXTEND);
  to = protect_from_queue (to, 1);
  from = protect_from_queue (from, 0);
  if (to_real != from_real)
    abort ();
  if (	((from)->code)  == SUBREG && ((from)->in_struct) 
      && (	(mode_size[(int)(	((((from)->fld[0].rtx) )->mode) )]) 
	  >= 	(mode_size[(int)(to_mode)]) )
      && ((from)->unchanging)  == unsignedp)
    from = gen_lowpart (to_mode, from), from_mode = to_mode;
  if (	((to)->code)  == SUBREG && ((to)->in_struct) )
    abort ();
  if (to_mode == from_mode
      || (from_mode == VOIDmode && (	((from)->code)  == LABEL_REF || 	((from)->code)  == SYMBOL_REF	|| 	((from)->code)  == CONST_INT || 	((from)->code)  == CONST_DOUBLE	|| 	((from)->code)  == CONST || 	((from)->code)  == HIGH) ))
    {
      emit_move_insn (to, from);
      return;
    }
  if (to_real)
    {
      if (1  && from_mode == SFmode && to_mode == DFmode)
	{
	  emit_unop_insn (CODE_FOR_extendsfdf2, to, from, UNKNOWN);
	  return;
	}
      if (1  && from_mode == SFmode && to_mode == TFmode)
	{
	  emit_unop_insn (CODE_FOR_extendsftf2, to, from, UNKNOWN);
	  return;
	}
      if (1  && from_mode == DFmode && to_mode == TFmode)
	{
	  emit_unop_insn (CODE_FOR_extenddftf2, to, from, UNKNOWN);
	  return;
	}
      if (1  && from_mode == DFmode && to_mode == SFmode)
	{
	  emit_unop_insn (CODE_FOR_truncdfsf2, to, from, UNKNOWN);
	  return;
	}
      if (1  && from_mode == TFmode && to_mode == SFmode)
	{
	  emit_unop_insn (CODE_FOR_trunctfsf2, to, from, UNKNOWN);
	  return;
	}
      if (1  && from_mode == TFmode && to_mode == DFmode)
	{
	  emit_unop_insn (CODE_FOR_trunctfdf2, to, from, UNKNOWN);
	  return;
	}
      libcall = (rtx) 0;
      switch (from_mode)
	{
	case SFmode:
	  switch (to_mode)
	    {
	    case DFmode:
	      libcall = extendsfdf2_libfunc;
	      break;
	    case XFmode:
	      libcall = extendsfxf2_libfunc;
	      break;
	    case TFmode:
	      libcall = extendsftf2_libfunc;
	      break;
	    }
	  break;
	case DFmode:
	  switch (to_mode)
	    {
	    case SFmode:
	      libcall = truncdfsf2_libfunc;
	      break;
	    case XFmode:
	      libcall = extenddfxf2_libfunc;
	      break;
	    case TFmode:
	      libcall = extenddftf2_libfunc;
	      break;
	    }
	  break;
	case XFmode:
	  switch (to_mode)
	    {
	    case SFmode:
	      libcall = truncxfsf2_libfunc;
	      break;
	    case DFmode:
	      libcall = truncxfdf2_libfunc;
	      break;
	    }
	  break;
	case TFmode:
	  switch (to_mode)
	    {
	    case SFmode:
	      libcall = trunctfsf2_libfunc;
	      break;
	    case DFmode:
	      libcall = trunctfdf2_libfunc;
	      break;
	    }
	  break;
	}
      if (libcall == (rtx) 0)
	abort ();
      emit_library_call (libcall, 1, to_mode, 1, from, from_mode);
      emit_move_insn (to, hard_libcall_value (to_mode));
      return;
    }
  if ( (8  * mode_size[(int)(from_mode)])  <  (8  * mode_size[(int)(to_mode)]) 
      &&  (8  * mode_size[(int)(to_mode)])  > 32 )
    {
      rtx insns;
      rtx lowpart;
      rtx fill_value;
      rtx lowfrom;
      int i;
      enum machine_mode lowpart_mode;
      int nwords = (((	(mode_size[(int)(to_mode)]) ) + ( 4 ) - 1) / ( 4 )) ;
      if ((code = can_extend_p (to_mode, from_mode, unsignedp))
	  != CODE_FOR_nothing)
	{
	  if (optimize > 0 && 	((from)->code)  == SUBREG)
	    from = force_reg (from_mode, from);
	  emit_unop_insn (code, to, from, equiv_code);
	  return;
	}
      else if ( (8  * mode_size[(int)(from_mode)])  < 32 
	       && ((code = can_extend_p (to_mode, word_mode, unsignedp))
		   != CODE_FOR_nothing))
	{
	  convert_move (gen_lowpart (word_mode, to), from, unsignedp);
	  emit_unop_insn (code, to,
			  gen_lowpart (word_mode, to), equiv_code);
	  return;
	}
      start_sequence ();
      if ( (8  * mode_size[(int)(from_mode)])  < 32 )
	lowpart_mode = word_mode;
      else
	lowpart_mode = from_mode;
      lowfrom = convert_to_mode (lowpart_mode, from, unsignedp);
      lowpart = gen_lowpart (lowpart_mode, to);
      emit_move_insn (lowpart, lowfrom);
      if (unsignedp)
	fill_value = const0_rtx;
      else
	{
	  if (1 
	      && insn_operand_mode[(int) CODE_FOR_slt][0] == word_mode
	      && 1  == -1)
	    {
	      emit_cmp_insn (lowfrom, const0_rtx, NE, (rtx) 0 ,
			     lowpart_mode, 0, 0);
	      fill_value = gen_reg_rtx (word_mode);
	      emit_insn (gen_slt (fill_value));
	    }
	  else
	    {
	      fill_value
		= expand_shift (RSHIFT_EXPR, lowpart_mode, lowfrom,
				size_int ( (8  * mode_size[(int)(lowpart_mode)])  - 1),
				(rtx) 0 , 0);
	      fill_value = convert_to_mode (word_mode, fill_value, 1);
	    }
	}
      for (i = 	(mode_size[(int)(lowpart_mode)])  / 4 ; i < nwords; i++)
	{
	  int index = (1  ? nwords - i - 1 : i);
	  rtx subword = operand_subword (to, index, 1, to_mode);
	  if (subword == 0)
	    abort ();
	  if (fill_value != subword)
	    emit_move_insn (subword, fill_value);
	}
      insns = get_insns ();
      end_sequence ();
      emit_no_conflict_block (insns, to, from, (rtx) 0 ,
			      gen_rtx (equiv_code, to_mode, from));
      return;
    }
  if ( (8  * mode_size[(int)(from_mode)])  > 32 )
    {
      convert_move (to, gen_lowpart (word_mode, from), 0);
      return;
    }
  if (to_mode == PSImode)
    {
      if (from_mode != SImode)
	from = convert_to_mode (SImode, from, unsignedp);
      abort ();
    }
  if (from_mode == PSImode)
    {
      if (to_mode != SImode)
	{
	  from = convert_to_mode (SImode, from, unsignedp);
	  from_mode = SImode;
	}
      else
	{
	  abort ();
	}
    }
  if ( (8  * mode_size[(int)(to_mode)])  <  (8  * mode_size[(int)(from_mode)]) 
      && 1 
      && ((	((from)->code)  == MEM
	   && ! ((from)->volatil) 
	   && direct_load[(int) to_mode]
	   && ! mode_dependent_address_p (((from)->fld[ 0].rtx) ))
	  || 	((from)->code)  == REG
	  || 	((from)->code)  == SUBREG))
    {
      emit_move_insn (to, gen_lowpart (to_mode, from));
      return;
    }
  if ( (8  * mode_size[(int)(to_mode)])  >  (8  * mode_size[(int)(from_mode)]) )
    {
      if ((code = can_extend_p (to_mode, from_mode, unsignedp))
	  != CODE_FOR_nothing)
	{
	  if (optimize > 0 && 	((from)->code)  == SUBREG)
	    from = force_reg (from_mode, from);
	  emit_unop_insn (code, to, from, equiv_code);
	  return;
	}
      else
	{
	  enum machine_mode intermediate;
	  for (intermediate = from_mode; intermediate != VOIDmode;
	       intermediate = (mode_wider_mode[(int)(intermediate)]) )
	    if ((can_extend_p (to_mode, intermediate, unsignedp)
		 != CODE_FOR_nothing)
		&& (can_extend_p (intermediate, from_mode, unsignedp)
		    != CODE_FOR_nothing))
	      {
		convert_move (to, convert_to_mode (intermediate, from,
						   unsignedp), unsignedp);
		return;
	      }
	  abort ();
	}
    }
  if (from_mode == DImode && to_mode == SImode)
    {
      convert_move (to, force_reg (from_mode, from), unsignedp);
      return;
    }
  if (from_mode == DImode && to_mode == HImode)
    {
      convert_move (to, force_reg (from_mode, from), unsignedp);
      return;
    }
  if (from_mode == DImode && to_mode == QImode)
    {
      convert_move (to, force_reg (from_mode, from), unsignedp);
      return;
    }
  if (from_mode == SImode && to_mode == HImode)
    {
      convert_move (to, force_reg (from_mode, from), unsignedp);
      return;
    }
  if (from_mode == SImode && to_mode == QImode)
    {
      convert_move (to, force_reg (from_mode, from), unsignedp);
      return;
    }
  if (from_mode == HImode && to_mode == QImode)
    {
      convert_move (to, force_reg (from_mode, from), unsignedp);
      return;
    }
  if ( (8  * mode_size[(int)(to_mode)])  <  (8  * mode_size[(int)(from_mode)]) )
    {
      rtx temp = force_reg (to_mode, gen_lowpart (to_mode, from));
      emit_move_insn (to, temp);
      return;
    }
  abort ();
}
rtx
convert_to_mode (mode, x, unsignedp)
     enum machine_mode mode;
     rtx x;
     int unsignedp;
{
  register rtx temp;
  if (	((x)->code)  == SUBREG && ((x)->in_struct) 
      && 	(mode_size[(int)(	((((x)->fld[0].rtx) )->mode) )])  >= 	(mode_size[(int)(mode)]) 
      && ((x)->unchanging)  == unsignedp)
    x = gen_lowpart (mode, x);
  if (mode == 	((x)->mode) )
    return x;
  if (unsignedp && 	(mode_class[(int)(mode)])  == MODE_INT
      &&  (8  * mode_size[(int)(mode)])  == 2 * 32  
      && 	((x)->code)  == CONST_INT && ((x)->fld[0].rtwint)  < 0)
    return immed_double_const (((x)->fld[0].rtwint) , (int ) 0, mode);
  if (	((x)->code)  == CONST_INT
      || (	(mode_class[(int)(mode)])  == MODE_INT
	  && 	(mode_class[(int)(	((x)->mode) )])  == MODE_INT
	  && (	((x)->code)  == CONST_DOUBLE
	      || (	(mode_size[(int)(mode)])  <= 	(mode_size[(int)(	((x)->mode) )]) 
		  && ((	((x)->code)  == MEM && ! ((x)->volatil) )
		      && direct_load[(int) mode]
		      || 	((x)->code)  == REG)))))
    return gen_lowpart (mode, x);
  temp = gen_reg_rtx (mode);
  convert_move (temp, x, unsignedp);
  return temp;
}
struct move_by_pieces
{
  rtx to;
  rtx to_addr;
  int autinc_to;
  int explicit_inc_to;
  rtx from;
  rtx from_addr;
  int autinc_from;
  int explicit_inc_from;
  int len;
  int offset;
  int reverse;
};
static void move_by_pieces_1 ();
static int move_by_pieces_ninsns ();
static void
move_by_pieces (to, from, len, align)
     rtx to, from;
     int len, align;
{
  struct move_by_pieces data;
  rtx to_addr = ((to)->fld[ 0].rtx) , from_addr = ((from)->fld[ 0].rtx) ;
  int max_size = 8  + 1;
  data.offset = 0;
  data.to_addr = to_addr;
  data.from_addr = from_addr;
  data.to = to;
  data.from = from;
  data.autinc_to
    = (	((to_addr)->code)  == PRE_INC || 	((to_addr)->code)  == PRE_DEC
       || 	((to_addr)->code)  == POST_INC || 	((to_addr)->code)  == POST_DEC);
  data.autinc_from
    = (	((from_addr)->code)  == PRE_INC || 	((from_addr)->code)  == PRE_DEC
       || 	((from_addr)->code)  == POST_INC
       || 	((from_addr)->code)  == POST_DEC);
  data.explicit_inc_from = 0;
  data.explicit_inc_to = 0;
  data.reverse
    = (	((to_addr)->code)  == PRE_DEC || 	((to_addr)->code)  == POST_DEC);
  if (data.reverse) data.offset = len;
  data.len = len;
  if (!(data.autinc_from && data.autinc_to)
      && move_by_pieces_ninsns (len, align) > 2)
    {
      if (!data.autinc_from && (	((from_addr)->code)  == LABEL_REF || 	((from_addr)->code)  == SYMBOL_REF	|| 	((from_addr)->code)  == CONST_INT || 	((from_addr)->code)  == CONST_DOUBLE	|| 	((from_addr)->code)  == CONST || 	((from_addr)->code)  == HIGH) )
	data.from_addr = copy_addr_to_reg (from_addr);
      if (!data.autinc_to && (	((to_addr)->code)  == LABEL_REF || 	((to_addr)->code)  == SYMBOL_REF	|| 	((to_addr)->code)  == CONST_INT || 	((to_addr)->code)  == CONST_DOUBLE	|| 	((to_addr)->code)  == CONST || 	((to_addr)->code)  == HIGH) )
	data.to_addr = copy_addr_to_reg (to_addr);
    }
  if (! (1  || 0 )
      || align > 8  || align >= 64  / 8 )
    align = 8 ;
  while (max_size > 1)
    {
      enum machine_mode mode = VOIDmode, tmode;
      enum insn_code icode;
      for (tmode = class_narrowest_mode[(int)(MODE_INT)] ;
	   tmode != VOIDmode; tmode = (mode_wider_mode[(int)(tmode)]) )
	if (	(mode_size[(int)(tmode)])  < max_size)
	  mode = tmode;
      if (mode == VOIDmode)
	break;
      icode = mov_optab->handlers[(int) mode].insn_code;
      if (icode != CODE_FOR_nothing
	  && align >= ((64  / 8 ) < (
			   	(mode_size[(int)(mode)]) ) ? (64  / 8 ) : (			   	(mode_size[(int)(mode)]) )) )
	move_by_pieces_1 ((*insn_gen_function[(int) (icode)]) , mode, &data);
      max_size = 	(mode_size[(int)(mode)]) ;
    }
  if (data.len != 0)
    abort ();
}
static int
move_by_pieces_ninsns (l, align)
     unsigned int l;
     int align;
{
  register int n_insns = 0;
  int max_size = 8  + 1;
  if (! (1  || 0 )
      || align > 8  || align >= 64  / 8 )
    align = 8 ;
  while (max_size > 1)
    {
      enum machine_mode mode = VOIDmode, tmode;
      enum insn_code icode;
      for (tmode = class_narrowest_mode[(int)(MODE_INT)] ;
	   tmode != VOIDmode; tmode = (mode_wider_mode[(int)(tmode)]) )
	if (	(mode_size[(int)(tmode)])  < max_size)
	  mode = tmode;
      if (mode == VOIDmode)
	break;
      icode = mov_optab->handlers[(int) mode].insn_code;
      if (icode != CODE_FOR_nothing
	  && align >= ((64  / 8 ) < (
			   	(mode_size[(int)(mode)]) ) ? (64  / 8 ) : (			   	(mode_size[(int)(mode)]) )) )
	n_insns += l / 	(mode_size[(int)(mode)]) , l %= 	(mode_size[(int)(mode)]) ;
      max_size = 	(mode_size[(int)(mode)]) ;
    }
  return n_insns;
}
static void
move_by_pieces_1 (genfun, mode, data)
     rtx (*genfun) ();
     enum machine_mode mode;
     struct move_by_pieces *data;
{
  register int size = 	(mode_size[(int)(mode)]) ;
  register rtx to1, from1;
  while (data->len >= size)
    {
      if (data->reverse) data->offset -= size;
      to1 = (data->autinc_to
	     ? gen_rtx (MEM, mode, data->to_addr)
	     : change_address (data->to, mode,
			       plus_constant_wide (data->to_addr, (int ) ( data->offset)) ));
      from1 =
	(data->autinc_from
	 ? gen_rtx (MEM, mode, data->from_addr)
	 : change_address (data->from, mode,
			   plus_constant_wide (data->from_addr, (int ) ( data->offset)) ));
      emit_insn ((*genfun) (to1, from1));
      if (! data->reverse) data->offset += size;
      data->len -= size;
    }
}
void
emit_block_move (x, y, size, align)
     rtx x, y;
     rtx size;
     int align;
{
  if (	((x)->mode)  != BLKmode)
    abort ();
  if (	((y)->mode)  != BLKmode)
    abort ();
  x = protect_from_queue (x, 1);
  y = protect_from_queue (y, 0);
  size = protect_from_queue (size, 0);
  if (	((x)->code)  != MEM)
    abort ();
  if (	((y)->code)  != MEM)
    abort ();
  if (size == 0)
    abort ();
  if (	((size)->code)  == CONST_INT
      && (move_by_pieces_ninsns (((size)->fld[0].rtwint) , align) < 2 ))
    move_by_pieces (x, y, ((size)->fld[0].rtwint) , align);
  else
    {
      rtx opalign = gen_rtx (CONST_INT, VOIDmode, (align)) ;
      enum machine_mode mode;
      for (mode = class_narrowest_mode[(int)(MODE_INT)] ; mode != VOIDmode;
	   mode = (mode_wider_mode[(int)(mode)]) )
	{
	  enum insn_code code = movstr_optab[(int) mode];
	  if (code != CODE_FOR_nothing
	      && (unsigned) ((size)->fld[0].rtwint)  <= (( (8  * mode_size[(int)(mode)])  >= 32  ) ?(int ) ~0 : (((int ) 1 <<  (8  * mode_size[(int)(mode)]) ) - 1)) 
	      && (insn_operand_predicate[(int) code][0] == 0
		  || (*insn_operand_predicate[(int) code][0]) (x, BLKmode))
	      && (insn_operand_predicate[(int) code][1] == 0
		  || (*insn_operand_predicate[(int) code][1]) (y, BLKmode))
	      && (insn_operand_predicate[(int) code][3] == 0
		  || (*insn_operand_predicate[(int) code][3]) (opalign,
							       VOIDmode)))
	    {
	      rtx op2;
	      rtx last = get_last_insn ();
	      rtx pat;
	      op2 = convert_to_mode (mode, size, 1);
	      if (insn_operand_predicate[(int) code][2] != 0
		  && ! (*insn_operand_predicate[(int) code][2]) (op2, mode))
		op2 = copy_to_mode_reg (mode, op2);
	      pat = (*insn_gen_function[(int) ((int) code)])  (x, y, op2, opalign);
	      if (pat)
		{
		  emit_insn (pat);
		  return;
		}
	      else
		delete_insns_since (last);
	    }
	}
      emit_library_call (memcpy_libfunc, 0,
			 VOIDmode, 3, ((x)->fld[ 0].rtx) , SImode ,
			 ((y)->fld[ 0].rtx) , SImode ,
			 convert_to_mode (SImode , size, 1), SImode );
    }
}
void
move_block_to_reg (regno, x, nregs, mode)
     int regno;
     rtx x;
     int nregs;
     enum machine_mode mode;
{
  int i;
  rtx pat, last;
  if ((	((x)->code)  == LABEL_REF || 	((x)->code)  == SYMBOL_REF	|| 	((x)->code)  == CONST_INT || 	((x)->code)  == CONST_DOUBLE	|| 	((x)->code)  == CONST || 	((x)->code)  == HIGH)  && ! (	((x)->code)  != CONST_DOUBLE || 	((x)->mode)  == VOIDmode) )
    x = validize_mem (force_const_mem (mode, x));
  for (i = 0; i < nregs; i++)
    emit_move_insn (gen_rtx (REG, word_mode, regno + i),
		    operand_subword_force (x, i, mode));
}
void
move_block_from_reg (regno, x, nregs)
     int regno;
     rtx x;
     int nregs;
{
  int i;
  rtx pat, last;
  for (i = 0; i < nregs; i++)
    {
      rtx tem = operand_subword (x, i, 1, BLKmode);
      if (tem == 0)
	abort ();
      emit_move_insn (tem, gen_rtx (REG, word_mode, regno + i));
    }
}
void
use_regs (regno, nregs)
     int regno;
     int nregs;
{
  int i;
  for (i = 0; i < nregs; i++)
    emit_insn (gen_rtx (USE, VOIDmode, gen_rtx (REG, word_mode, regno + i)));
}
static rtx
group_insns (prev)
     rtx prev;
{
  rtx insn_first;
  rtx insn_last;
  if (prev)
    insn_first = ((prev)->fld[2].rtx) ;
  else
    insn_first = get_insns ();
  insn_last = get_last_insn ();
  ((insn_last)->fld[6].rtx)  = gen_rtx (INSN_LIST, REG_RETVAL, insn_first,
				   ((insn_last)->fld[6].rtx) );
  ((insn_first)->fld[6].rtx)  = gen_rtx (INSN_LIST, REG_LIBCALL, insn_last,
				    ((insn_first)->fld[6].rtx) );
}
void
clear_storage (object, size)
     rtx object;
     int size;
{
  if (	((object)->mode)  == BLKmode)
    {
      emit_library_call (memset_libfunc, 0,
			 VOIDmode, 3,
			 ((object)->fld[ 0].rtx) , SImode , const0_rtx, SImode ,
			 gen_rtx (CONST_INT, VOIDmode, (size)) , SImode );
    }
  else
    emit_move_insn (object, const0_rtx);
}
rtx
emit_move_insn (x, y)
     rtx x, y;
{
  enum machine_mode mode = 	((x)->mode) ;
  enum machine_mode submode;
  enum mode_class class = 	(mode_class[(int)(mode)]) ;
  int i;
  x = protect_from_queue (x, 1);
  y = protect_from_queue (y, 0);
  if (mode == BLKmode || (	((y)->mode)  != mode && 	((y)->mode)  != VOIDmode))
    abort ();
  if ((	((y)->code)  == LABEL_REF || 	((y)->code)  == SYMBOL_REF	|| 	((y)->code)  == CONST_INT || 	((y)->code)  == CONST_DOUBLE	|| 	((y)->code)  == CONST || 	((y)->code)  == HIGH)  && ! (	((y)->code)  != CONST_DOUBLE || 	((y)->mode)  == VOIDmode) )
    y = force_const_mem (mode, y);
  if (	((x)->code)  == MEM
      && ((! memory_address_p (	((x)->mode) , ((x)->fld[ 0].rtx) )
	   && ! push_operand (x, 	((x)->mode) ))
	  || (flag_force_addr
	      && ((	((((x)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((x)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST_INT || 	((((x)->fld[ 0].rtx) )->code)  == CONST_DOUBLE	|| 	((((x)->fld[ 0].rtx) )->code)  == CONST || 	((((x)->fld[ 0].rtx) )->code)  == HIGH) ) )))
    x = change_address (x, VOIDmode, ((x)->fld[ 0].rtx) );
  if (	((y)->code)  == MEM
      && (! memory_address_p (	((y)->mode) , ((y)->fld[ 0].rtx) )
	  || (flag_force_addr
	      && ((	((((y)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((y)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((y)->fld[ 0].rtx) )->code)  == CONST_INT || 	((((y)->fld[ 0].rtx) )->code)  == CONST_DOUBLE	|| 	((((y)->fld[ 0].rtx) )->code)  == CONST || 	((((y)->fld[ 0].rtx) )->code)  == HIGH) ) )))
    y = change_address (y, VOIDmode, ((y)->fld[ 0].rtx) );
  if (mode == BLKmode)
    abort ();
  if (class == MODE_COMPLEX_FLOAT || class == MODE_COMPLEX_INT)
    submode = mode_for_size ((mode_unit_size[(int)(mode)])  * 8 ,
			     (class == MODE_COMPLEX_INT
			      ? MODE_INT : MODE_FLOAT),
			     0);
  if (mov_optab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
    return
      emit_insn ((*insn_gen_function[(int) (mov_optab->handlers[(int) mode].insn_code)])  (x, y));
  else if ((class == MODE_COMPLEX_FLOAT || class == MODE_COMPLEX_INT)
	   && submode != BLKmode
	   && (mov_optab->handlers[(int) submode].insn_code
	       != CODE_FOR_nothing))
    {
      int stack = push_operand (x, 	((x)->mode) );
      rtx prev = get_last_insn ();
      if (	((x)->code)  == REG)
	emit_insn (gen_rtx (CLOBBER, VOIDmode, x));
      emit_insn ((*insn_gen_function[(int) (mov_optab->handlers[(int) submode].insn_code)]) 
		 ((stack ? change_address (x, submode, (rtx) 0)
		   : gen_highpart (submode, x)),
		  gen_highpart (submode, y)));
      emit_insn ((*insn_gen_function[(int) (mov_optab->handlers[(int) submode].insn_code)]) 
		 ((stack ? change_address (x, submode, (rtx) 0)
		   : gen_lowpart (submode, x)),
		  gen_lowpart (submode, y)));
      group_insns (prev);
      return get_last_insn ();
    }
  else if (	(mode_size[(int)(mode)])  > 4 )
    {
      rtx last_insn = 0;
      rtx prev_insn = get_last_insn ();
      for (i = 0;
	   i < (	(mode_size[(int)(mode)])   + (4  - 1)) / 4 ;
	   i++)
	{
	  rtx xpart = operand_subword (x, i, 1, mode);
	  rtx ypart = operand_subword (y, i, 1, mode);
	  if (ypart == 0 && (	((y)->code)  == LABEL_REF || 	((y)->code)  == SYMBOL_REF	|| 	((y)->code)  == CONST_INT || 	((y)->code)  == CONST_DOUBLE	|| 	((y)->code)  == CONST || 	((y)->code)  == HIGH) )
	    {
	      y = force_const_mem (mode, y);
	      ypart = operand_subword (y, i, 1, mode);
	    }
	  else if (ypart == 0)
	    ypart = operand_subword_force (y, i, mode);
	  if (xpart == 0 || ypart == 0)
	    abort ();
	  last_insn = emit_move_insn (xpart, ypart);
	}
      group_insns (prev_insn);
      return last_insn;
    }
  else
    abort ();
}
rtx
push_block (size, extra, below)
     rtx size;
     int extra, below;
{
  register rtx temp;
  if ((	((size)->code)  == LABEL_REF || 	((size)->code)  == SYMBOL_REF	|| 	((size)->code)  == CONST_INT || 	((size)->code)  == CONST_DOUBLE	|| 	((size)->code)  == CONST || 	((size)->code)  == HIGH) )
    anti_adjust_stack (plus_constant_wide (size, (int ) ( extra)) );
  else if (	((size)->code)  == REG && extra == 0)
    anti_adjust_stack (size);
  else
    {
      rtx temp = copy_to_mode_reg (SImode , size);
      if (extra != 0)
	temp = expand_binop (SImode , add_optab, temp, gen_rtx (CONST_INT, VOIDmode, (extra)) ,
			     temp, 0, OPTAB_LIB_WIDEN);
      anti_adjust_stack (temp);
    }
  temp = virtual_outgoing_args_rtx;
  if (extra != 0 && below)
    temp = plus_constant_wide (temp, (int ) ( extra)) ;
  return memory_address (class_narrowest_mode[(int)(MODE_INT)] , temp);
}
rtx
gen_push_operand ()
{
  return gen_rtx (PRE_DEC , SImode , stack_pointer_rtx);
}
void
emit_push_insn (x, mode, type, size, align, partial, reg, extra,
		args_addr, args_so_far)
     register rtx x;
     enum machine_mode mode;
     tree type;
     rtx size;
     int align;
     int partial;
     rtx reg;
     int extra;
     rtx args_addr;
     rtx args_so_far;
{
  rtx xinner;
  enum direction stack_direction
    = downward;
  enum direction where_pad = (((mode) == BLKmode	? (( type) && ((enum tree_code) ((( type)->type.size) )->common.code)  == INTEGER_CST	&& int_size_in_bytes ( type) < 32  / 8 )	:  (8  * mode_size[(int)(mode)])  < 32 )	? downward : upward) ;
  if (PRE_DEC  == POST_INC || PRE_DEC  == POST_DEC)
    if (where_pad != none)
      where_pad = (where_pad == downward ? upward : downward);
  xinner = x = protect_from_queue (x, 0);
  if (mode == BLKmode)
    {
      register rtx temp;
      int used = partial * 4 ;
      int offset = used % (32  / 8 );
      int skip;
      if (size == 0)
	abort ();
      used -= offset;
      if (partial != 0)
	xinner = change_address (xinner, BLKmode,
				 plus_constant_wide (((xinner)->fld[ 0].rtx) , (int ) ( used)) );
      skip = used;
	{
	  if (partial != 0)
	    {
	      if (	((size)->code)  == CONST_INT)
		size = gen_rtx (CONST_INT, VOIDmode, (((size)->fld[0].rtwint)  - used)) ;
	      else
		size = expand_binop (	((size)->mode) , sub_optab, size,
				     gen_rtx (CONST_INT, VOIDmode, (used)) , (rtx) 0 , 0,
				     OPTAB_LIB_WIDEN);
	    }
	  if (! args_addr)
	    {
	      temp = push_block (size, extra, where_pad == downward);
	      extra = 0;
	    }
	  else if (	((args_so_far)->code)  == CONST_INT)
	    temp = memory_address (BLKmode,
				   plus_constant_wide (args_addr, (int ) (
						  skip + ((args_so_far)->fld[0].rtwint) )) );
	  else
	    temp = memory_address (BLKmode,
				   plus_constant_wide (gen_rtx (PLUS, SImode ,
							   args_addr, args_so_far), (int ) (
						  skip)) );
	  if (	((size)->code)  == CONST_INT
	      && (move_by_pieces_ninsns ((unsigned) ((size)->fld[0].rtwint) , align)
		  < 2 ))
	    {
	      move_by_pieces (gen_rtx (MEM, BLKmode, temp), xinner,
			      ((size)->fld[0].rtwint) , align);
	      goto ret;
	    }
	  if (1 )
	    {
	      emit_insn (gen_movstrsi (gen_rtx (MEM, BLKmode, temp),
				       xinner, size, gen_rtx (CONST_INT, VOIDmode, (align)) ));
	      goto ret;
	    }
	  (inhibit_defer_pop += 1) ;
	  emit_library_call (memcpy_libfunc, 0,
			     VOIDmode, 3, temp, SImode , ((xinner)->fld[ 0].rtx) , SImode ,
			     size, SImode );
	  (inhibit_defer_pop -= 1) ;
	}
    }
  else if (partial > 0)
    {
      int size = 	(mode_size[(int)(mode)])  / 4 ;
      int i;
      int not_stack;
      int offset = partial % (32  / 32 );
      int args_offset = ((args_so_far)->fld[0].rtwint) ;
      int skip;
      if (extra && args_addr == 0
	  && where_pad != none && where_pad != stack_direction)
	anti_adjust_stack (gen_rtx (CONST_INT, VOIDmode, (extra)) );
      if (args_addr == 0)
	offset = 0;
      not_stack = partial - offset;
      skip = not_stack;
      if ((	((x)->code)  == LABEL_REF || 	((x)->code)  == SYMBOL_REF	|| 	((x)->code)  == CONST_INT || 	((x)->code)  == CONST_DOUBLE	|| 	((x)->code)  == CONST || 	((x)->code)  == HIGH)  && ! (	((x)->code)  != CONST_DOUBLE || 	((x)->mode)  == VOIDmode) )
	x = validize_mem (force_const_mem (mode, x));
      if ((	((x)->code)  == REG && ((x)->fld[0].rtint)  < 64 
	   && 	(mode_class[(int)(	((x)->mode) )])  != MODE_INT))
	x = copy_to_reg (x);
      for (i = not_stack; i < size; i++)
	if (i >= not_stack + offset)
	  emit_push_insn (operand_subword_force (x, i, mode),
			  word_mode, (tree) 0  , (rtx) 0 , align, 0, (rtx) 0 ,
			  0, args_addr,
			  gen_rtx (CONST_INT, VOIDmode, (args_offset + ((i - not_stack + skip)
						  * 4 ))) );
    }
  else
    {
      rtx addr;
      if (extra && args_addr == 0
	  && where_pad != none && where_pad != stack_direction)
	anti_adjust_stack (gen_rtx (CONST_INT, VOIDmode, (extra)) );
	if (	((args_so_far)->code)  == CONST_INT)
	  addr
	    = memory_address (mode,
			      plus_constant_wide (args_addr, (int ) ( ((args_so_far)->fld[0].rtwint) )) );
      else
	addr = memory_address (mode, gen_rtx (PLUS, SImode , args_addr,
					      args_so_far));
      emit_move_insn (gen_rtx (MEM, mode, addr), x);
    }
 ret:
  if (partial > 0)
    move_block_to_reg (((reg)->fld[0].rtint) , x, partial, mode);
  if (extra && args_addr == 0 && where_pad == stack_direction)
    anti_adjust_stack (gen_rtx (CONST_INT, VOIDmode, (extra)) );
}
typedef char *va_list;
void
emit_library_call (va_alist)
     int va_alist;
{
  va_list p;
  struct args_size args_size;
  register int argnum;
  enum machine_mode outmode;
  int nargs;
  rtx fun;
  rtx orgfun;
  int inc;
  int count;
  rtx argblock = 0;
  int  args_so_far;
  struct arg { rtx value; enum machine_mode mode; rtx reg; int partial;
	       struct args_size offset; struct args_size size; };
  struct arg *argvec;
  int old_inhibit_defer_pop = inhibit_defer_pop;
  int no_queue = 0;
  rtx use_insns;
  p = (char *) &va_alist;
  orgfun = fun = (( rtx *)(p += sizeof( rtx)))[-1] ;
  no_queue = (( int *)(p += sizeof( int)))[-1] ;
  outmode = (( enum machine_mode *)(p += sizeof( enum machine_mode)))[-1] ;
  nargs = (( int *)(p += sizeof( int)))[-1] ;
  argvec = (struct arg *) alloca (nargs * sizeof (struct arg));
  ((args_so_far) = 0) ;
  args_size.constant = 0;
  args_size.var = 0;
  for (count = 0; count < nargs; count++)
    {
      rtx val = (( rtx *)(p += sizeof( rtx)))[-1] ;
      enum machine_mode mode = (( enum machine_mode *)(p += sizeof( enum machine_mode)))[-1] ;
      if (mode == BLKmode
	  || (	((val)->mode)  != mode && 	((val)->mode)  != VOIDmode))
	abort ();
      if (	((val)->code)  != REG && 	((val)->code)  != MEM
	  && ! ((	((val)->code)  == LABEL_REF || 	((val)->code)  == SYMBOL_REF	|| 	((val)->code)  == CONST_INT || 	((val)->code)  == CONST_DOUBLE	|| 	((val)->code)  == CONST || 	((val)->code)  == HIGH)  && (	((val)->code)  != CONST_DOUBLE || 	((val)->mode)  == VOIDmode) ))
	val = force_operand (val, (rtx) 0 );
      argvec[count].value = val;
      argvec[count].mode = mode;
      if ((( (tree) 0   && (((enum tree_code) ( (tree) 0  )->common.code)  == RECORD_TYPE	|| ((enum tree_code) ( (tree) 0  )->common.code)  == UNION_TYPE))	|| ( mode == TFmode)) )
	abort ();
      argvec[count].reg = (((target_flags & 32)  && (mode_unit_size[(int)(( ( mode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far)))  < 6 	&& (( (tree) 0  )==0 || ! (((tree)( (tree) 0  ))->common.addressable_flag) )	&& (( (tree) 0  )==0 || ( mode) != BLKmode	|| (((( (tree) 0  ))->type.align)  % 32  == 0))	? gen_rtx (REG, ( mode),	((8)  + ((target_flags & 32)  && (mode_unit_size[(int)(( ( mode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far))) ))	: 0) ;
      if (argvec[count].reg && 	((argvec[count].reg)->code)  == EXPR_LIST)
	abort ();
      argvec[count].partial
	= ((((target_flags & 32)  && (mode_unit_size[(int)(( ( mode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far)))  < 6 	&& (( (tree) 0  )==0 || ! (((tree)( (tree) 0  ))->common.addressable_flag) )	&& (( (tree) 0  )==0 || ( mode) != BLKmode	|| (((( (tree) 0  ))->type.align)  % 32  == 0))	&& (((target_flags & 32)  && (mode_unit_size[(int)(( ( mode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far))) 	+ (( mode) == BLKmode	? ((int_size_in_bytes ( (tree) 0  ) + 4  - 1) / 4 ) 	: ((	(mode_size[(int)( mode)])  + 4  - 1) / 4 ) )) - 6  > 0)	? (6  - ((target_flags & 32)  && (mode_unit_size[(int)(( ( mode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far))) )	: 0) ;
      locate_and_pad_parm (mode, (tree) 0  ,
			   argvec[count].reg && argvec[count].partial == 0,
			   (tree) 0  , &args_size, &argvec[count].offset,
			   &argvec[count].size);
      if (argvec[count].size.var)
	abort ();
      if (argvec[count].reg == 0 || argvec[count].partial != 0
	  || 1
	  )
	args_size.constant += argvec[count].size.constant;
      if (argvec[count].reg == 0 || argvec[count].partial != 0)
	abort ();
      ((args_so_far) = (((target_flags & 32)  && (mode_unit_size[(int)(( ( mode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far))) 	+ (( mode) != BLKmode	? ((	(mode_size[(int)( mode)])  + 4  - 1) / 4 ) 	: ((int_size_in_bytes ( (tree)0) + 4  - 1) / 4 ) ))) ;
    }
   ;
  assemble_external_libcall (fun);
  args_size.constant = (((args_size.constant + ((64  / 8 )  - 1))
			 / (64  / 8 ) ) * (64  / 8 ) );
  args_size.constant = ((args_size.constant) > (
			    (6  * 4 ) ) ? (args_size.constant) : (			    (6  * 4 ) )) ;
  if (args_size.constant > current_function_outgoing_args_size)
    current_function_outgoing_args_size = args_size.constant;
  args_size.constant = 0;
  argblock = push_block (gen_rtx (CONST_INT, VOIDmode, (args_size.constant)) , 0, 0);
  inc = 1;
  argnum = 0;
  for (count = 0; count < nargs; count++, argnum += inc)
    {
      register enum machine_mode mode = argvec[argnum].mode;
      register rtx val = argvec[argnum].value;
      rtx reg = argvec[argnum].reg;
      int partial = argvec[argnum].partial;
      if (! (reg != 0 && partial == 0))
	emit_push_insn (val, mode, (tree) 0  , (rtx) 0 , 0, partial, reg, 0,
			argblock, gen_rtx (CONST_INT, VOIDmode, (argvec[count].offset.constant)) );
      (inhibit_defer_pop += 1) ;
    }
  argnum = 0;
  for (count = 0; count < nargs; count++, argnum += inc)
    {
      register enum machine_mode mode = argvec[argnum].mode;
      register rtx val = argvec[argnum].value;
      rtx reg = argvec[argnum].reg;
      int partial = argvec[argnum].partial;
      if (reg != 0 && partial == 0)
	emit_move_insn (reg, val);
      (inhibit_defer_pop += 1) ;
    }
  if (! no_queue)
    emit_queue ();
  start_sequence ();
  for (count = 0; count < nargs; count++)
    if (argvec[count].reg != 0)
      emit_insn (gen_rtx (USE, VOIDmode, argvec[count].reg));
  use_insns = get_insns ();
  end_sequence ();
  fun = prepare_call_address (fun, (tree) 0  , &use_insns);
  (inhibit_defer_pop += 1) ;
  emit_call_1 (fun, get_identifier (((orgfun)->fld[ 0].rtstr) ), args_size.constant, 0,
	       (((target_flags & 32)  && (mode_unit_size[(int)(( ( VOIDmode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far)))  < 6 	&& (( void_type_node)==0 || ! (((tree)( void_type_node))->common.addressable_flag) )	&& (( void_type_node)==0 || ( VOIDmode) != BLKmode	|| (((( void_type_node))->type.align)  % 32  == 0))	? gen_rtx (REG, ( VOIDmode),	((8)  + ((target_flags & 32)  && (mode_unit_size[(int)(( ( VOIDmode)))])  > 4	? (((args_so_far)) + ! (((args_so_far)) & 1)) : ((args_so_far))) ))	: 0) ,
	       outmode != VOIDmode ? hard_libcall_value (outmode) : (rtx) 0 ,
	       old_inhibit_defer_pop + 1, use_insns, no_queue);
  (inhibit_defer_pop -= 1) ;
}
rtx
expand_assignment (to, from, want_value, suggest_reg)
     tree to, from;
     int want_value;
     int suggest_reg;
{
  register rtx to_rtx = 0;
  rtx result;
  if (((enum tree_code) (to)->common.code)  == ERROR_MARK)
    return expand_expr (from, (rtx) 0 , VOIDmode, 0);
  if (((enum tree_code) (to)->common.code)  == COMPONENT_REF
      || ((enum tree_code) (to)->common.code)  == BIT_FIELD_REF
      || (((enum tree_code) (to)->common.code)  == ARRAY_REF
	  && ((enum tree_code) (((to)->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	  && ((enum tree_code) (((((to)->common.type) )->type.size) )->common.code)  == INTEGER_CST))
    {
      enum machine_mode mode1;
      int bitsize;
      int bitpos;
      tree offset;
      int unsignedp;
      int volatilep = 0;
      tree tem = get_inner_reference (to, &bitsize, &bitpos, &offset,
				      &mode1, &unsignedp, &volatilep);
      if (mode1 == VOIDmode && want_value)
	tem = stabilize_reference (tem);
      to_rtx = expand_expr (tem, (rtx) 0 , VOIDmode, 0);
      if (offset != 0)
	{
	  rtx offset_rtx = expand_expr (offset, (rtx) 0 , VOIDmode, 0);
	  if (	((to_rtx)->code)  != MEM)
	    abort ();
	  to_rtx = change_address (to_rtx, VOIDmode,
				   gen_rtx (PLUS, SImode , ((to_rtx)->fld[ 0].rtx) ,
					    force_reg (SImode , offset_rtx)));
	}
      if (volatilep)
	{
	  if (	((to_rtx)->code)  == MEM)
	    ((to_rtx)->volatil)  = 1;
	}
      result = store_field (to_rtx, bitsize, bitpos, mode1, from,
			    (want_value
			     ? (enum machine_mode) ((((to)->common.type) )->type.mode) 
			     : VOIDmode),
			    unsignedp,
			    ((((tem)->common.type) )->type.align)  / 8 ,
			    int_size_in_bytes (((tem)->common.type) ));
      preserve_temp_slots (result);
      free_temp_slots ();
      return result;
    }
  if (to_rtx == 0)
    to_rtx = expand_expr (to, (rtx) 0 , VOIDmode, 0);
  if (((enum tree_code) (to)->common.code)  == RESULT_DECL && ((enum tree_code) (from)->common.code)  == INDIRECT_REF
      && current_function_returns_struct
      && !current_function_returns_pcc_struct)
    {
      rtx from_rtx = expand_expr (from, (rtx) 0 , VOIDmode, 0);
      rtx size = expr_size (from);
      emit_library_call (memcpy_libfunc, 0,
			 VOIDmode, 3, ((to_rtx)->fld[ 0].rtx) , SImode ,
			 ((from_rtx)->fld[ 0].rtx) , SImode ,
			 size, SImode );
      preserve_temp_slots (to_rtx);
      free_temp_slots ();
      return to_rtx;
    }
  result = store_expr (from, to_rtx, want_value);
  preserve_temp_slots (result);
  free_temp_slots ();
  return result;
}
rtx
store_expr (exp, target, suggest_reg)
     register tree exp;
     register rtx target;
     int suggest_reg;
{
  register rtx temp;
  int dont_return_target = 0;
  if (((enum tree_code) (exp)->common.code)  == COMPOUND_EXPR)
    {
      expand_expr (((exp)->exp.operands[ 0]) , const0_rtx, VOIDmode, 0);
      emit_queue ();
      return store_expr (((exp)->exp.operands[ 1]) , target, suggest_reg);
    }
  else if (((enum tree_code) (exp)->common.code)  == COND_EXPR && 	((target)->mode)  == BLKmode)
    {
      rtx lab1 = gen_label_rtx (), lab2 = gen_label_rtx ();
      emit_queue ();
      target = protect_from_queue (target, 1);
      (inhibit_defer_pop += 1) ;
      jumpifnot (((exp)->exp.operands[ 0]) , lab1);
      store_expr (((exp)->exp.operands[ 1]) , target, suggest_reg);
      emit_queue ();
      emit_jump_insn (gen_jump (lab2));
      emit_barrier ();
      emit_label (lab1);
      store_expr (((exp)->exp.operands[ 2]) , target, suggest_reg);
      emit_queue ();
      emit_label (lab2);
      (inhibit_defer_pop -= 1) ;
      return target;
    }
  else if (suggest_reg && 	((target)->code)  == MEM
	   && 	((target)->mode)  != BLKmode)
    {
      temp = expand_expr (exp, cse_not_expected ? (rtx) 0  : target,
			  	((target)->mode) , 0);
      if (	((temp)->mode)  != BLKmode && 	((temp)->mode)  != VOIDmode)
	temp = copy_to_reg (temp);
      dont_return_target = 1;
    }
  else if (queued_subexp_p (target))
    {
      if (	((target)->mode)  != BLKmode && 	((target)->mode)  != VOIDmode)
	{
	  temp = gen_reg_rtx (	((target)->mode) );
	  temp = expand_expr (exp, temp, 	((target)->mode) , 0);
	}
      else
	temp = expand_expr (exp, (rtx) 0 , 	((target)->mode) , 0);
      dont_return_target = 1;
    }
  else if (	((target)->code)  == SUBREG && ((target)->in_struct) )
    {
      temp = expand_expr (exp, (rtx) 0 , VOIDmode, 0);
      convert_move (((target)->fld[0].rtx) , temp,
		    ((target)->unchanging) );
      return temp;
    }
  else
    {
      temp = expand_expr (exp, target, 	((target)->mode) , 0);
      if (!(target && 	((target)->code)  == REG
	    && ((target)->fld[0].rtint)  < 64 )
	  && (	((temp)->code)  == LABEL_REF || 	((temp)->code)  == SYMBOL_REF	|| 	((temp)->code)  == CONST_INT || 	((temp)->code)  == CONST_DOUBLE	|| 	((temp)->code)  == CONST || 	((temp)->code)  == HIGH) )
	dont_return_target = 1;
    }
  if (temp != target && ((enum tree_code) (exp)->common.code)  != ERROR_MARK)
    {
      target = protect_from_queue (target, 1);
      if (	((temp)->mode)  != 	((target)->mode) 
	  && 	((temp)->mode)  != VOIDmode)
	{
	  int unsignedp = ((((exp)->common.type) )->common.unsigned_flag) ;
	  if (dont_return_target)
	    {
	      temp = convert_to_mode (	((target)->mode) , temp, unsignedp);
	      emit_move_insn (target, temp);
	    }
	  else
	    convert_move (target, temp, unsignedp);
	}
      else if (	((temp)->mode)  == BLKmode && ((enum tree_code) (exp)->common.code)  == STRING_CST)
	{
	  rtx size;
	  size = expr_size (exp);
	  if (	((size)->code)  == CONST_INT
	      && ((size)->fld[0].rtwint)  < ((exp)->string.length) )
	    emit_block_move (target, temp, size,
			     ((((exp)->common.type) )->type.align)  / 8 );
	  else
	    {
	      tree copy_size
		= fold (build (MIN_EXPR, sizetype,
			       size_binop (CEIL_DIV_EXPR,
					   ((((exp)->common.type) )->type.size) ,
					   size_int (8 )),
			       convert (sizetype,
					build_int_2_wide ((int ) (((exp)->string.length) ), (int ) ( 0)) )));
	      rtx copy_size_rtx = expand_expr (copy_size, (rtx) 0 ,
					       VOIDmode, 0);
	      rtx label = 0;
	      emit_block_move (target, temp, copy_size_rtx,
			       ((((exp)->common.type) )->type.align)  / 8 );
	      if (	((copy_size_rtx)->code)  == CONST_INT)
		{
		  temp = plus_constant_wide (((target)->fld[ 0].rtx) , (int ) (
					((exp)->string.length) )) ;
		  size = plus_constant_wide (size, (int ) (
					- ((exp)->string.length) )) ;
		}
	      else
		{
		  enum machine_mode size_mode = SImode ;
		  temp = force_reg (SImode , ((target)->fld[ 0].rtx) );
		  temp = expand_binop (size_mode, add_optab, temp,
				       copy_size_rtx, (rtx) 0 , 0,
				       OPTAB_LIB_WIDEN);
		  size = expand_binop (size_mode, sub_optab, size,
				       copy_size_rtx, (rtx) 0 , 0,
				       OPTAB_LIB_WIDEN);
		  emit_cmp_insn (size, const0_rtx, LT, (rtx) 0 ,
				 	((size)->mode) , 0, 0);
		  label = gen_label_rtx ();
		  emit_jump_insn (gen_blt (label));
		}
	      if (size != const0_rtx)
		{
		  emit_library_call (memset_libfunc, 0, VOIDmode, 3,
				     temp, SImode , const0_rtx, SImode , size, SImode );
		}
	      if (label)
		emit_label (label);
	    }
	}
      else if (	((temp)->mode)  == BLKmode)
	emit_block_move (target, temp, expr_size (exp),
			 ((((exp)->common.type) )->type.align)  / 8 );
      else
	emit_move_insn (target, temp);
    }
  if (dont_return_target)
    return temp;
  return target;
}
static void
store_constructor (exp, target)
     tree exp;
     rtx target;
{
  tree type = ((exp)->common.type) ;
  if (((enum tree_code) (type)->common.code)  == RECORD_TYPE || ((enum tree_code) (type)->common.code)  == UNION_TYPE)
    {
      register tree elt;
      if (((enum tree_code) (type)->common.code)  == UNION_TYPE)
	emit_insn (gen_rtx (CLOBBER, VOIDmode, target));
      else if (	((target)->code)  == REG && ((exp)->common.static_flag) )
	emit_move_insn (target, const0_rtx);
      else if (list_length (((exp)->exp.operands[ 1])  )
	       != list_length (((type)->type.values) ))
	clear_storage (target, int_size_in_bytes (type));
      else
	emit_insn (gen_rtx (CLOBBER, VOIDmode, target));
      for (elt = ((exp)->exp.operands[ 1])  ; elt; elt = ((elt)->common.chain) )
	{
	  register tree field = ((elt)->list.purpose) ;
	  register enum machine_mode mode;
	  int bitsize;
	  int bitpos;
	  int unsignedp;
	  if (field == 0)
	    continue;
	  bitsize = ((((field)->decl.size) )->int_cst.int_cst_low) ;
	  unsignedp = ((field)->common.unsigned_flag) ;
	  mode = ((field)->decl.mode) ;
	  if (((field)->decl.bit_field_flag) )
	    mode = VOIDmode;
	  if (((enum tree_code) (((field)->decl.arguments) )->common.code)  != INTEGER_CST)
	    abort ();
	  bitpos = ((((field)->decl.arguments) )->int_cst.int_cst_low) ;
	  store_field (target, bitsize, bitpos, mode, ((elt)->list.value) ,
		       VOIDmode, 0,
		       ((type)->type.align)  / 8 ,
		       int_size_in_bytes (type));
	}
    }
  else if (((enum tree_code) (type)->common.code)  == ARRAY_TYPE)
    {
      register tree elt;
      register int i;
      tree domain = ((type)->type.values) ;
      int  minelt = ((((domain)->type.minval) )->int_cst.int_cst_low) ;
      int  maxelt = ((((domain)->type.maxval) )->int_cst.int_cst_low) ;
      tree elttype = ((type)->common.type) ;
      if (list_length (((exp)->exp.operands[ 1])  ) < maxelt - minelt + 1
	  || (	((target)->code)  == REG && ((exp)->common.static_flag) ))
	clear_storage (target, maxelt - minelt + 1);
      else
	emit_insn (gen_rtx (CLOBBER, VOIDmode, target));
      for (elt = ((exp)->exp.operands[ 1])  , i = 0;
	   elt;
	   elt = ((elt)->common.chain) , i++)
	{
	  register enum machine_mode mode;
	  int bitsize;
	  int bitpos;
	  int unsignedp;
	  mode = ((elttype)->type.mode) ;
	  bitsize =  (8  * mode_size[(int)(mode)]) ;
	  unsignedp = ((elttype)->common.unsigned_flag) ;
	  bitpos = (i * ((((elttype)->type.size) )->int_cst.int_cst_low) );
	  store_field (target, bitsize, bitpos, mode, ((elt)->list.value) ,
		       VOIDmode, 0,
		       ((type)->type.align)  / 8 ,
		       int_size_in_bytes (type));
	}
    }
  else
    abort ();
}
static rtx
store_field (target, bitsize, bitpos, mode, exp, value_mode,
	     unsignedp, align, total_size)
     rtx target;
     int bitsize, bitpos;
     enum machine_mode mode;
     tree exp;
     enum machine_mode value_mode;
     int unsignedp;
     int align;
     int total_size;
{
  int  width_mask = 0;
  if (bitsize < 32  )
    width_mask = ((int ) 1 << bitsize) - 1;
  if (mode == BLKmode
      && (	((target)->code)  == REG || 	((target)->code)  == SUBREG))
    {
      rtx object = assign_stack_temp (	((target)->mode) ,
				      	(mode_size[(int)(	((target)->mode) )]) , 0);
      rtx blk_object = copy_rtx (object);
      ((blk_object)->mode = ( BLKmode)) ;
      if (bitsize !=  (8  * mode_size[(int)(	((target)->mode) )]) )
	emit_move_insn (object, target);
      store_field (blk_object, bitsize, bitpos, mode, exp, VOIDmode, 0,
		   align, total_size);
      emit_move_insn (target, object);
      return target;
    }
  if (mode == VOIDmode
      || (mode != BLKmode && ! direct_store[(int) mode])
      || 	((target)->code)  == REG
      || 	((target)->code)  == SUBREG)
    {
      rtx temp = expand_expr (exp, (rtx) 0 , VOIDmode, 0);
      store_bit_field (target, bitsize, bitpos, mode, temp, align, total_size);
      if (value_mode != VOIDmode)
	{
	  if (width_mask != 0
	      && ! (	((target)->code)  == MEM && ((target)->volatil) ))
	    {
	      tree count;
	      enum machine_mode tmode;
	      if (unsignedp)
		return expand_and (temp, gen_rtx (CONST_INT, VOIDmode, (width_mask)) , (rtx) 0 );
	      tmode = 	((temp)->mode) ;
	      if (tmode == VOIDmode)
		tmode = value_mode;
	      count = build_int_2_wide ((int ) ( (8  * mode_size[(int)(tmode)])  - bitsize), (int ) ( 0)) ;
	      temp = expand_shift (LSHIFT_EXPR, tmode, temp, count, 0, 0);
	      return expand_shift (RSHIFT_EXPR, tmode, temp, count, 0, 0);
	    }
	  return extract_bit_field (target, bitsize, bitpos, unsignedp,
				    (rtx) 0 , value_mode, 0, align,
				    total_size);
	}
      return const0_rtx;
    }
  else
    {
      rtx addr = ((target)->fld[ 0].rtx) ;
      rtx to_rtx;
      if (value_mode != VOIDmode && 	((addr)->code)  != REG
	  && ! ((	((addr)->code)  == LABEL_REF || 	((addr)->code)  == SYMBOL_REF	|| 	((addr)->code)  == CONST_INT || 	((addr)->code)  == CONST_DOUBLE	|| 	((addr)->code)  == CONST || 	((addr)->code)  == HIGH) ) 
	  && ! (	((addr)->code)  == PLUS
		&& 	((((addr)->fld[ 1].rtx) )->code)  == CONST_INT
		&& (((addr)->fld[ 0].rtx)  == virtual_incoming_args_rtx
		    || ((addr)->fld[ 0].rtx)  == virtual_stack_vars_rtx)))
	addr = copy_to_reg (addr);
      to_rtx = change_address (target, mode,
			       plus_constant_wide (addr, (int ) ( (bitpos / 8 ))) );
      ((to_rtx)->in_struct)  = 1;
      return store_expr (exp, to_rtx, value_mode != VOIDmode);
    }
}
tree
get_inner_reference (exp, pbitsize, pbitpos, poffset, pmode, punsignedp, pvolatilep)
     tree exp;
     int *pbitsize;
     int *pbitpos;
     tree *poffset;
     enum machine_mode *pmode;
     int *punsignedp;
     int *pvolatilep;
{
  tree size_tree = 0;
  enum machine_mode mode = VOIDmode;
  tree offset = 0;
  if (((enum tree_code) (exp)->common.code)  == COMPONENT_REF)
    {
      size_tree = ((((exp)->exp.operands[ 1]) )->decl.size) ;
      if (! ((((exp)->exp.operands[ 1]) )->decl.bit_field_flag) )
	mode = ((((exp)->exp.operands[ 1]) )->decl.mode) ;
      *punsignedp = ((((exp)->exp.operands[ 1]) )->common.unsigned_flag) ;
    }
  else if (((enum tree_code) (exp)->common.code)  == BIT_FIELD_REF)
    {
      size_tree = ((exp)->exp.operands[ 1]) ;
      *punsignedp = ((exp)->common.unsigned_flag) ;
    }
  else
    {
      mode = ((((exp)->common.type) )->type.mode) ;
      *pbitsize =  (8  * mode_size[(int)(mode)]) ;
      *punsignedp = ((((exp)->common.type) )->common.unsigned_flag) ;
    }
  if (size_tree)
    {
      if (((enum tree_code) (size_tree)->common.code)  != INTEGER_CST)
	mode = BLKmode, *pbitsize = -1;
      else
	*pbitsize = ((size_tree)->int_cst.int_cst_low) ;
    }
  *pbitpos = 0;
  while (1)
    {
      if (((enum tree_code) (exp)->common.code)  == INDIRECT_REF && flag_volatile)
 	*pvolatilep = 1;
      if (((enum tree_code) (exp)->common.code)  == COMPONENT_REF || ((enum tree_code) (exp)->common.code)  == BIT_FIELD_REF)
	{
	  tree pos = (((enum tree_code) (exp)->common.code)  == COMPONENT_REF
		      ? ((((exp)->exp.operands[ 1]) )->decl.arguments) 
		      : ((exp)->exp.operands[ 2]) );
	  if (((enum tree_code) (pos)->common.code)  == PLUS_EXPR)
	    {
	      tree constant, var;
	      if (((enum tree_code) (((pos)->exp.operands[ 0]) )->common.code)  == INTEGER_CST)
		{
		  constant = ((pos)->exp.operands[ 0]) ;
		  var = ((pos)->exp.operands[ 1]) ;
		}
	      else if (((enum tree_code) (((pos)->exp.operands[ 1]) )->common.code)  == INTEGER_CST)
		{
		  constant = ((pos)->exp.operands[ 1]) ;
		  var = ((pos)->exp.operands[ 0]) ;
		}
	      else
		abort ();
	      *pbitpos += ((constant)->int_cst.int_cst_low) ;
	      if (offset)
		offset = size_binop (PLUS_EXPR, offset,
				     size_binop (FLOOR_DIV_EXPR, var,
						 size_int (8 )));
	      else
		offset = size_binop (FLOOR_DIV_EXPR, var,
				     size_int (8 ));
	    }
	  else if (((enum tree_code) (pos)->common.code)  == INTEGER_CST)
	    *pbitpos += ((pos)->int_cst.int_cst_low) ;
	  else
	    {
	      if (offset)
		offset = size_binop (PLUS_EXPR, offset,
				     size_binop (FLOOR_DIV_EXPR, pos,
						 size_int (8 )));
	      else
		offset = size_binop (FLOOR_DIV_EXPR, pos,
				     size_int (8 ));
	    }
	}
      else if (((enum tree_code) (exp)->common.code)  == ARRAY_REF
	       && ((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	       && ((enum tree_code) (((((exp)->common.type) )->type.size) )->common.code)  == INTEGER_CST)
	{
	  *pbitpos += (((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) 
		       * ((((((exp)->common.type) )->type.size) )->int_cst.int_cst_low) );
	}
      else if (((enum tree_code) (exp)->common.code)  != NON_LVALUE_EXPR
	       && ! ((((enum tree_code) (exp)->common.code)  == NOP_EXPR
		      || ((enum tree_code) (exp)->common.code)  == CONVERT_EXPR)
		     && (((((exp)->common.type) )->type.mode) 
			 == ((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )))
	break;
      if (((exp)->common.volatile_flag) )
	*pvolatilep = 1;
      exp = ((exp)->exp.operands[ 0]) ;
    }
  if (mode == VOIDmode && *pbitpos % *pbitsize == 0)
    {
      mode = mode_for_size (*pbitsize, MODE_INT, 0);
      if (mode == BLKmode)
	mode = VOIDmode;
    }
  *pmode = mode;
  *poffset = offset;
  return exp;
}
rtx
force_operand (value, target)
     rtx value, target;
{
  register optab binoptab = 0;
  rtx tmp;
  register rtx op2;
  register rtx subtarget = (target != 0 && 	((target)->code)  == REG ? target : 0);
  if (	((value)->code)  == PLUS)
    binoptab = add_optab;
  else if (	((value)->code)  == MINUS)
    binoptab = sub_optab;
  else if (	((value)->code)  == MULT)
    {
      op2 = ((value)->fld[ 1].rtx) ;
      if (!(	((op2)->code)  == LABEL_REF || 	((op2)->code)  == SYMBOL_REF	|| 	((op2)->code)  == CONST_INT || 	((op2)->code)  == CONST_DOUBLE	|| 	((op2)->code)  == CONST || 	((op2)->code)  == HIGH) 
	  && !(	((op2)->code)  == REG && op2 != subtarget))
	subtarget = 0;
      tmp = force_operand (((value)->fld[ 0].rtx) , subtarget);
      return expand_mult (	((value)->mode) , tmp,
			  force_operand (op2, (rtx) 0 ),
			  target, 0);
    }
  if (binoptab)
    {
      op2 = ((value)->fld[ 1].rtx) ;
      if (!(	((op2)->code)  == LABEL_REF || 	((op2)->code)  == SYMBOL_REF	|| 	((op2)->code)  == CONST_INT || 	((op2)->code)  == CONST_DOUBLE	|| 	((op2)->code)  == CONST || 	((op2)->code)  == HIGH) 
	  && !(	((op2)->code)  == REG && op2 != subtarget))
	subtarget = 0;
      if (binoptab == sub_optab && 	((op2)->code)  == CONST_INT)
	{
	  binoptab = add_optab;
	  op2 = negate_rtx (	((value)->mode) , op2);
	}
      if (binoptab == add_optab && 	((op2)->code)  == CONST_INT
	  && 	((((value)->fld[ 0].rtx) )->code)  == PLUS
	  && 	((((((value)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG
	  && ((((((value)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  >= (64 ) 
	  && ((((((value)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  <= (((64 ) ) + 3) )
	{
	  rtx temp = expand_binop (	((value)->mode) , binoptab,
				   ((((value)->fld[ 0].rtx) )->fld[ 0].rtx) , op2,
				   subtarget, 0, OPTAB_LIB_WIDEN);
	  return expand_binop (	((value)->mode) , binoptab, temp,
			       force_operand (((((value)->fld[ 0].rtx) )->fld[ 1].rtx) , 0),
			       target, 0, OPTAB_LIB_WIDEN);
	}
      tmp = force_operand (((value)->fld[ 0].rtx) , subtarget);
      return expand_binop (	((value)->mode) , binoptab, tmp,
			   force_operand (op2, (rtx) 0 ),
			   target, 0, OPTAB_LIB_WIDEN);
    }
  return value;
}
static tree
save_noncopied_parts (lhs, list)
     tree lhs;
     tree list;
{
  tree tail;
  tree parts = 0;
  for (tail = list; tail; tail = ((tail)->common.chain) )
    if (((enum tree_code) (((tail)->list.value) )->common.code)  == TREE_LIST)
      parts = chainon (parts, save_noncopied_parts (lhs, ((tail)->list.value) ));
    else
      {
	tree part = ((tail)->list.value) ;
	tree part_type = ((part)->common.type) ;
	tree to_be_saved = build (COMPONENT_REF, part_type, lhs, part);
	rtx target = assign_stack_temp (((part_type)->type.mode) ,
					int_size_in_bytes (part_type), 0);
	if (! memory_address_p (((part_type)->type.mode) , ((target)->fld[ 0].rtx) ))
	  target = change_address (target, ((part_type)->type.mode) , (rtx) 0 );
	parts = tree_cons (to_be_saved,
			   build (RTL_EXPR, part_type, (tree) 0  ,
				  (tree) target),
			   parts);
	store_expr (((parts)->list.purpose) , (*(struct rtx_def **) &(((parts)->list.value) )->exp.operands[1]) , 0);
      }
  return parts;
}
static tree
init_noncopied_parts (lhs, list)
     tree lhs;
     tree list;
{
  tree tail;
  tree parts = 0;
  for (tail = list; tail; tail = ((tail)->common.chain) )
    if (((enum tree_code) (((tail)->list.value) )->common.code)  == TREE_LIST)
      parts = chainon (parts, init_noncopied_parts (lhs, ((tail)->list.value) ));
    else
      {
	tree part = ((tail)->list.value) ;
	tree part_type = ((part)->common.type) ;
	tree to_be_initialized = build (COMPONENT_REF, part_type, lhs, part);
	parts = tree_cons (((tail)->list.purpose) , to_be_initialized, parts);
      }
  return parts;
}
static int
safe_from_p (x, exp)
     rtx x;
     tree exp;
{
  rtx exp_rtl = 0;
  int i, nops;
  if (x == 0)
    return 1;
  if (	((x)->code)  == SUBREG)
    {
      x = ((x)->fld[0].rtx) ;
      if (	((x)->code)  == REG && ((x)->fld[0].rtint)  < 64 )
	return 0;
    }
  if (	((x)->code)  == MEM
      && (((x)->fld[ 0].rtx)  == virtual_outgoing_args_rtx
	  || (	((((x)->fld[ 0].rtx) )->code)  == PLUS
	      && ((((x)->fld[ 0].rtx) )->fld[ 0].rtx)  == virtual_outgoing_args_rtx)))
    return 1;
  switch ((*tree_code_type[(int) (((enum tree_code) (exp)->common.code) )]) )
    {
    case 'd':
      exp_rtl = ((exp)->decl.rtl) ;
      break;
    case 'c':
      return 1;
    case 'x':
      if (((enum tree_code) (exp)->common.code)  == TREE_LIST)
	return ((((exp)->list.value)  == 0
		 || safe_from_p (x, ((exp)->list.value) ))
		&& (((exp)->common.chain)  == 0
		    || safe_from_p (x, ((exp)->common.chain) )));
      else
	return 0;
    case '1':
      return safe_from_p (x, ((exp)->exp.operands[ 0]) );
    case '2':
    case '<':
      return (safe_from_p (x, ((exp)->exp.operands[ 0]) )
	      && safe_from_p (x, ((exp)->exp.operands[ 1]) ));
    case 'e':
    case 'r':
      switch (((enum tree_code) (exp)->common.code) )
	{
	case ADDR_EXPR:
	  return staticp (((exp)->exp.operands[ 0]) );
	case INDIRECT_REF:
	  if (	((x)->code)  == MEM)
	    return 0;
	  break;
	case CALL_EXPR:
	  exp_rtl = (*(struct rtx_def **) &(exp)->exp.operands[2]) ;
	  if (exp_rtl == 0)
	    {
	      if ((	((x)->code)  == REG && ((x)->fld[0].rtint)  < 64 )
		  || 	((x)->code)  == MEM)
		return 0;
	    }
	  break;
	case RTL_EXPR:
	  exp_rtl = (*(struct rtx_def **) &(exp)->exp.operands[1]) ;
	  if (exp_rtl == 0)
	    return 0;
	  break;
	case WITH_CLEANUP_EXPR:
	  exp_rtl = (*(struct rtx_def **) &(exp)->exp.operands[1]) ;
	  break;
	case SAVE_EXPR:
	  exp_rtl = (*(struct rtx_def **) &(exp)->exp.operands[2]) ;
	  break;
	case BIND_EXPR:
	  return safe_from_p (x, ((exp)->exp.operands[ 1]) );
	case METHOD_CALL_EXPR:
	  abort ();
	}
      if (exp_rtl)
	break;
      nops = tree_code_length[(int) ((enum tree_code) (exp)->common.code) ];
      for (i = 0; i < nops; i++)
	if (((exp)->exp.operands[ i])  != 0
	    && ! safe_from_p (x, ((exp)->exp.operands[ i]) ))
	  return 0;
    }
  if (exp_rtl)
    {
      if (	((exp_rtl)->code)  == SUBREG)
	{
	  exp_rtl = ((exp_rtl)->fld[0].rtx) ;
	  if (	((exp_rtl)->code)  == REG
	      && ((exp_rtl)->fld[0].rtint)  < 64 )
	    return 0;
	}
      return ! (rtx_equal_p (x, exp_rtl)
		|| (	((x)->code)  == MEM && 	((exp_rtl)->code)  == MEM
		    && ! ((exp)->common.readonly_flag) ));
    }
  return 1;
}
static int
fixed_type_p (exp)
     tree exp;
{
  if (((enum tree_code) (exp)->common.code)  == PARM_DECL
      || ((enum tree_code) (exp)->common.code)  == VAR_DECL
      || ((enum tree_code) (exp)->common.code)  == CALL_EXPR || ((enum tree_code) (exp)->common.code)  == TARGET_EXPR
      || ((enum tree_code) (exp)->common.code)  == COMPONENT_REF
      || ((enum tree_code) (exp)->common.code)  == ARRAY_REF)
    return 1;
  return 0;
}
rtx
expand_expr (exp, target, tmode, modifier)
     register tree exp;
     rtx target;
     enum machine_mode tmode;
     enum expand_modifier modifier;
{
  register rtx op0, op1, temp;
  tree type = ((exp)->common.type) ;
  int unsignedp = ((type)->common.unsigned_flag) ;
  register enum machine_mode mode = ((type)->type.mode) ;
  register enum tree_code code = ((enum tree_code) (exp)->common.code) ;
  optab this_optab;
  rtx subtarget = (target != 0 && 	((target)->code)  == REG ? target : 0);
  rtx original_target = target;
  int ignore = target == const0_rtx;
  tree context;
  if (subtarget && ((subtarget)->fld[0].rtint)  < 64 )
    subtarget = 0;
  if (preserve_subexpressions_p ())
    subtarget = 0;
  if (ignore) target = 0, original_target = 0;
  if (! cse_not_expected && mode != BLKmode && target
      && (	((target)->code)  != REG || ((target)->fld[0].rtint)  < 64 ))
    target = subtarget;
  if (ignore && ((exp)->common.volatile_flag) 
      && mode != VOIDmode && mode != BLKmode)
    {
      target = gen_reg_rtx (mode);
      temp = expand_expr (exp, target, VOIDmode, modifier);
      if (temp != target)
	emit_move_insn (target, temp);
      return target;
    }
  switch (code)
    {
    case LABEL_DECL:
      {
	tree function = decl_function_context (exp);
	if (function != current_function_decl && function != 0)
	  {
	    struct function *p = find_function_data (function);
	    push_obstacks (p->function_obstack,
			   p->function_maybepermanent_obstack);
	    p->forced_labels = gen_rtx (EXPR_LIST, VOIDmode,
					label_rtx (exp), p->forced_labels);
	    pop_obstacks ();
	  }
	else if (modifier == EXPAND_INITIALIZER)
	  forced_labels = gen_rtx (EXPR_LIST, VOIDmode,
				   label_rtx (exp), forced_labels);
	temp = gen_rtx (MEM, SImode ,
			gen_rtx (LABEL_REF, SImode , label_rtx (exp)));
	if (function != current_function_decl && function != 0)
	  ((((temp)->fld[ 0].rtx) )->volatil)  = 1;
	return temp;
      }
    case PARM_DECL:
      if (((exp)->decl.rtl)  == 0)
	{
	  error_with_decl (exp, "prior parameter's size depends on `%s'");
	  return (const_tiny_rtx[0][(int) (mode)]) ;
	}
    case FUNCTION_DECL:
    case VAR_DECL:
    case RESULT_DECL:
      if (((exp)->decl.rtl)  == 0)
	abort ();
      ((exp)->common.used_flag)  = 1;
      context = decl_function_context (exp);
      if (context != 0 && context != current_function_decl
	  && context != inline_function_decl
	  && ! (	((((exp)->decl.rtl) )->code)  == MEM
		&& (	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST_INT || 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST_DOUBLE	|| 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST || 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == HIGH) ))
	{
	  rtx addr;
	  ((exp)->decl.nonlocal_flag)  = 1;
	  mark_addressable (exp);
	  if (	((((exp)->decl.rtl) )->code)  != MEM)
	    abort ();
	  addr = ((((exp)->decl.rtl) )->fld[ 0].rtx) ;
	  if (	((addr)->code)  == MEM)
	    addr = gen_rtx (MEM, SImode , fix_lexical_addr (((addr)->fld[ 0].rtx) , exp));
	  else
	    addr = fix_lexical_addr (addr, exp);
	  return change_address (((exp)->decl.rtl) , mode, addr);
	}
      if (	((((exp)->decl.rtl) )->code)  == MEM
	  && 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == REG)
	return change_address (((exp)->decl.rtl) , 	((((exp)->decl.rtl) )->mode) ,
			       ((((exp)->decl.rtl) )->fld[ 0].rtx) );
      if (	((((exp)->decl.rtl) )->code)  == MEM
	  && modifier != EXPAND_CONST_ADDRESS
	  && modifier != EXPAND_SUM
	  && modifier != EXPAND_INITIALIZER)
	{
	  if (!memory_address_p (((exp)->decl.mode) , ((((exp)->decl.rtl) )->fld[ 0].rtx) )
	      || (flag_force_addr
		  && ((	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST_INT || 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST_DOUBLE	|| 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == CONST || 	((((((exp)->decl.rtl) )->fld[ 0].rtx) )->code)  == HIGH) ) ))
	    return change_address (((exp)->decl.rtl) , VOIDmode,
				   copy_rtx (((((exp)->decl.rtl) )->fld[ 0].rtx) ));
	}
      if (	((((exp)->decl.rtl) )->code)  == REG
	  && 	((((exp)->decl.rtl) )->mode)  != mode)
	{
	  enum machine_mode decl_mode = ((exp)->decl.mode) ;
	   ;
	  if (decl_mode != 	((((exp)->decl.rtl) )->mode) )
	    abort ();
	  temp = gen_rtx (SUBREG, mode, ((exp)->decl.rtl) , 0);
	  ((temp)->in_struct)  = 1;
	  ((temp)->unchanging)  = unsignedp;
	  return temp;
	}
      return ((exp)->decl.rtl) ;
    case INTEGER_CST:
      return immed_double_const (((exp)->int_cst.int_cst_low) ,
				 ((exp)->int_cst.int_cst_high) ,
				 mode);
    case CONST_DECL:
      return expand_expr (((exp)->decl.initial) , target, VOIDmode, 0);
    case REAL_CST:
      return immed_real_const (exp);
    case COMPLEX_CST:
    case STRING_CST:
      if (! ((exp)->real_cst.rtl) )
	output_constant_def (exp);
      if (	((((exp)->real_cst.rtl) )->code)  == MEM
	  && modifier != EXPAND_CONST_ADDRESS
	  && modifier != EXPAND_INITIALIZER
	  && modifier != EXPAND_SUM
	  && !memory_address_p (mode, ((((exp)->real_cst.rtl) )->fld[ 0].rtx) ))
	return change_address (((exp)->real_cst.rtl) , VOIDmode,
			       copy_rtx (((((exp)->real_cst.rtl) )->fld[ 0].rtx) ));
      return ((exp)->real_cst.rtl) ;
    case SAVE_EXPR:
      context = decl_function_context (exp);
      if (context == current_function_decl || context == inline_function_decl)
	context = 0;
      if (context)
	{
	  temp = (*(struct rtx_def **) &(exp)->exp.operands[2]) ;
	  if (temp && 	((temp)->code)  == REG)
	    {
	      put_var_into_stack (exp);
	      temp = (*(struct rtx_def **) &(exp)->exp.operands[2]) ;
	    }
	  if (temp == 0 || 	((temp)->code)  != MEM)
	    abort ();
	  return change_address (temp, mode,
				 fix_lexical_addr (((temp)->fld[ 0].rtx) , exp));
	}
      if ((*(struct rtx_def **) &(exp)->exp.operands[2])  == 0)
	{
	  if (mode == BLKmode)
	    temp
	      = assign_stack_temp (mode,
				   int_size_in_bytes (((exp)->common.type) ), 0);
	  else
	    {
	      enum machine_mode var_mode = mode;
	      if (((enum tree_code) (type)->common.code)  == INTEGER_TYPE
		  || ((enum tree_code) (type)->common.code)  == ENUMERAL_TYPE
		  || ((enum tree_code) (type)->common.code)  == BOOLEAN_TYPE
		  || ((enum tree_code) (type)->common.code)  == CHAR_TYPE
		  || ((enum tree_code) (type)->common.code)  == REAL_TYPE
		  || ((enum tree_code) (type)->common.code)  == POINTER_TYPE
		  || ((enum tree_code) (type)->common.code)  == OFFSET_TYPE)
		{
		   ;
		}
	      temp = gen_reg_rtx (var_mode);
	    }
	  (*(struct rtx_def **) &(exp)->exp.operands[2])  = temp;
	  store_expr (((exp)->exp.operands[ 0]) , temp, 0);
	  if (!optimize && 	((temp)->code)  == REG)
	    save_expr_regs = gen_rtx (EXPR_LIST, VOIDmode, temp,
				      save_expr_regs);
	}
      if (	(((*(struct rtx_def **) &(exp)->exp.operands[2]) )->code)  == REG
	  && 	(((*(struct rtx_def **) &(exp)->exp.operands[2]) )->mode)  != mode)
	{
	  temp = gen_rtx (SUBREG, mode, (*(struct rtx_def **) &(exp)->exp.operands[2]) , 0);
	  ((temp)->in_struct)  = 1;
	  ((temp)->unchanging)  = unsignedp;
	  return temp;
	}
      return (*(struct rtx_def **) &(exp)->exp.operands[2]) ;
    case EXIT_EXPR:
      {
	rtx label = gen_label_rtx ();
	do_jump (((exp)->exp.operands[ 0]) , label, (rtx) 0 );
	expand_exit_loop (((void * )0) );
	emit_label (label);
      }
      return const0_rtx;
    case LOOP_EXPR:
      expand_start_loop (1);
      expand_expr_stmt (((exp)->exp.operands[ 0]) );
      expand_end_loop ();
      return const0_rtx;
    case BIND_EXPR:
      {
	tree vars = ((exp)->exp.operands[ 0]) ;
	int vars_need_expansion = 0;
	expand_start_bindings (0);
	if (((exp)->exp.operands[ 2])  != 0
	    && ! ((((exp)->exp.operands[ 2]) )->common.used_flag) )
	  insert_block (((exp)->exp.operands[ 2]) );
	while (vars)
	  {
	    if (((vars)->decl.rtl)  == 0)
	      {
		vars_need_expansion = 1;
		expand_decl (vars);
	      }
	    expand_decl_init (vars);
	    vars = ((vars)->common.chain) ;
	  }
	temp = expand_expr (((exp)->exp.operands[ 1]) , target, tmode, modifier);
	expand_end_bindings (((exp)->exp.operands[ 0]) , 0, 0);
	return temp;
      }
    case RTL_EXPR:
      if ((*(struct rtx_def **) &(exp)->exp.operands[0])  == const0_rtx)
	abort ();
      emit_insns ((*(struct rtx_def **) &(exp)->exp.operands[0]) );
      (*(struct rtx_def **) &(exp)->exp.operands[0])  = const0_rtx;
      return (*(struct rtx_def **) &(exp)->exp.operands[1]) ;
    case CONSTRUCTOR:
      if (((exp)->common.static_flag)  && (mode == BLKmode || ((exp)->common.addressable_flag) ))
	{
	  rtx constructor = output_constant_def (exp);
	  if (modifier != EXPAND_CONST_ADDRESS
	      && modifier != EXPAND_INITIALIZER
	      && modifier != EXPAND_SUM
	      && !memory_address_p (	((constructor)->mode) ,
				    ((constructor)->fld[ 0].rtx) ))
	    constructor = change_address (constructor, VOIDmode,
					  ((constructor)->fld[ 0].rtx) );
	  return constructor;
	}
      if (ignore)
	{
	  tree elt;
	  for (elt = ((exp)->exp.operands[ 1])  ; elt; elt = ((elt)->common.chain) )
	    expand_expr (((elt)->list.value) , const0_rtx, VOIDmode, 0);
	  return const0_rtx;
	}
      else
	{
	  if (target == 0 || ! safe_from_p (target, exp))
	    {
	      if (mode != BLKmode && ! ((exp)->common.addressable_flag) )
		target = gen_reg_rtx (mode);
	      else
		{
		  rtx safe_target = assign_stack_temp (mode, int_size_in_bytes (type), 0);
		  if (target)
		    ((safe_target)->in_struct)  = ((target)->in_struct) ;
		  target = safe_target;
		}
	    }
	  store_constructor (exp, target);
	  return target;
	}
    case INDIRECT_REF:
      {
	tree exp1 = ((exp)->exp.operands[ 0]) ;
	tree exp2;
	if (((enum tree_code) (exp1)->common.code)  == SAVE_EXPR
	    && (*(struct rtx_def **) &(exp1)->exp.operands[2])  == 0
	    && ((enum tree_code) (exp2 = ((exp1)->exp.operands[ 0]) )->common.code)  != ERROR_MARK
	    && ((((exp1)->common.type) )->type.mode)  == SImode 
	    && ((((exp2)->common.type) )->type.mode)  == SImode )
	  {
	    temp = expand_expr (((exp1)->exp.operands[ 0]) , (rtx) 0 ,
				VOIDmode, EXPAND_SUM);
	    op0 = memory_address (mode, temp);
	    op0 = copy_all_regs (op0);
	    (*(struct rtx_def **) &(exp1)->exp.operands[2])  = op0;
	  }
	else
	  {
	    op0 = expand_expr (exp1, (rtx) 0 , VOIDmode, EXPAND_SUM);
	    op0 = memory_address (mode, op0);
	  }
	temp = gen_rtx (MEM, mode, op0);
	if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == PLUS_EXPR
	    || (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == SAVE_EXPR
		&& ((enum tree_code) (((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.code)  == PLUS_EXPR)
	    || ((enum tree_code) (((exp)->common.type) )->common.code)  == ARRAY_TYPE
	    || ((enum tree_code) (((exp)->common.type) )->common.code)  == RECORD_TYPE
	    || ((enum tree_code) (((exp)->common.type) )->common.code)  == UNION_TYPE
	    || (((enum tree_code) (exp1)->common.code)  == ADDR_EXPR
		&& (exp2 = ((exp1)->exp.operands[ 0]) )
		&& (((enum tree_code) (((exp2)->common.type) )->common.code)  == ARRAY_TYPE
		    || ((enum tree_code) (((exp2)->common.type) )->common.code)  == RECORD_TYPE
		    || ((enum tree_code) (((exp2)->common.type) )->common.code)  == UNION_TYPE)))
	  ((temp)->in_struct)  = 1;
	((temp)->volatil)  = ((exp)->common.volatile_flag)  || flag_volatile;
	return temp;
      }
    case ARRAY_REF:
      if (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  != INTEGER_CST
	  || ((enum tree_code) (((type)->type.size) )->common.code)  != INTEGER_CST)
	{
	  tree variant_type = build_type_variant (type,
						  ((exp)->common.readonly_flag) ,
						  ((exp)->common.volatile_flag) );
	  tree array_adr = build1 (ADDR_EXPR, build_pointer_type (variant_type),
				   ((exp)->exp.operands[ 0]) );
	  tree index = ((exp)->exp.operands[ 1]) ;
	  tree elt;
	  if (((((index)->common.type) )->type.precision)  != 32 )
	    index = convert (type_for_size (32 , 0), index);
	  ((array_adr)->common.side_effects_flag)  = 0;
	  elt = build1 (INDIRECT_REF, type,
			fold (build (PLUS_EXPR, ((variant_type)->type.pointer_to) ,
				     array_adr,
				     fold (build (MULT_EXPR,
						  ((variant_type)->type.pointer_to) ,
						  index, size_in_bytes (type))))));
	  ((elt)->common.side_effects_flag)  = ((exp)->common.side_effects_flag) ;
	  ((elt)->common.volatile_flag)  = ((exp)->common.volatile_flag) ;
	  ((elt)->common.readonly_flag)  = ((exp)->common.readonly_flag) ;
	  return expand_expr (elt, target, tmode, modifier);
	}
      {
	int i;
	tree arg0 = ((exp)->exp.operands[ 0]) ;
	tree arg1 = ((exp)->exp.operands[ 1]) ;
	if (((enum tree_code) (arg0)->common.code)  == STRING_CST
	    && ((enum tree_code) (arg1)->common.code)  == INTEGER_CST
	    && !((arg1)->int_cst.int_cst_high) 
	    && (i = ((arg1)->int_cst.int_cst_low) ) < ((arg0)->string.length) )
	  {
	    if (((((arg0)->common.type) )->common.type)  == integer_type_node)
	      {
		exp = build_int_2_wide ((int ) (((int *)((arg0)->string.pointer) )[i]), (int ) ( 0)) ;
		((exp)->common.type)  = integer_type_node;
		return expand_expr (exp, target, tmode, modifier);
	      }
	    if (((((arg0)->common.type) )->common.type)  == char_type_node)
	      {
		exp = build_int_2_wide ((int ) (((arg0)->string.pointer) [i]), (int ) ( 0)) ;
		((exp)->common.type)  = integer_type_node;
		return expand_expr (convert (((((arg0)->common.type) )->common.type) , exp), target, tmode, modifier);
	      }
	  }
      }
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == CONSTRUCTOR
	  && ! ((((exp)->exp.operands[ 0]) )->common.side_effects_flag) )
	{
	  tree index = fold (((exp)->exp.operands[ 1]) );
	  if (((enum tree_code) (index)->common.code)  == INTEGER_CST
	      && ((index)->int_cst.int_cst_high)  == 0)
	    {
	      int i = ((index)->int_cst.int_cst_low) ;
	      tree elem = ((((exp)->exp.operands[ 0]) )->exp.operands[ 1])  ;
	      while (elem && i--)
		elem = ((elem)->common.chain) ;
	      if (elem)
		return expand_expr (fold (((elem)->list.value) ), target,
				    tmode, modifier);
	    }
	}
      else if (((((exp)->exp.operands[ 0]) )->common.readonly_flag) 
	       && ! ((((exp)->exp.operands[ 0]) )->common.side_effects_flag) 
	       && ((enum tree_code) (((((exp)->exp.operands[ 0]) )->common.type) )->common.code)  == ARRAY_TYPE
	       && ((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == VAR_DECL
	       && ((((exp)->exp.operands[ 0]) )->decl.initial) 
	       && optimize >= 1
	       && (((enum tree_code) (((((exp)->exp.operands[ 0]) )->decl.initial) )->common.code) 
		   != ERROR_MARK))
	{
	  tree index = fold (((exp)->exp.operands[ 1]) );
	  if (((enum tree_code) (index)->common.code)  == INTEGER_CST
	      && ((index)->int_cst.int_cst_high)  == 0)
	    {
	      int i = ((index)->int_cst.int_cst_low) ;
	      tree init = ((((exp)->exp.operands[ 0]) )->decl.initial) ;
	      if (((enum tree_code) (init)->common.code)  == CONSTRUCTOR)
		{
		  tree elem = ((init)->exp.operands[ 1])  ;
		  while (elem && i--)
		    elem = ((elem)->common.chain) ;
		  if (elem)
		    return expand_expr (fold (((elem)->list.value) ), target,
					tmode, modifier);
		}
	      else if (((enum tree_code) (init)->common.code)  == STRING_CST
		       && i < ((init)->string.length) )
		{
		  temp = gen_rtx (CONST_INT, VOIDmode, (((init)->string.pointer) [i])) ;
		  return convert_to_mode (mode, temp, 0);
		}
	    }
	}
    case COMPONENT_REF:
    case BIT_FIELD_REF:
      if (code != ARRAY_REF
	  && ((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == CONSTRUCTOR)
	{
	  tree elt;
	  for (elt = ((((exp)->exp.operands[ 0]) )->exp.operands[ 1])  ; elt;
	       elt = ((elt)->common.chain) )
	    if (((elt)->list.purpose)  == ((exp)->exp.operands[ 1]) )
	      return expand_expr (((elt)->list.value) , target, tmode, modifier);
	}
      {
	enum machine_mode mode1;
	int bitsize;
	int bitpos;
	tree offset;
	int volatilep = 0;
	tree tem = get_inner_reference (exp, &bitsize, &bitpos, &offset,
					&mode1, &unsignedp, &volatilep);
	op0 = expand_expr (tem, (rtx) 0 , VOIDmode, EXPAND_SUM);
	if ((	((op0)->code)  == LABEL_REF || 	((op0)->code)  == SYMBOL_REF	|| 	((op0)->code)  == CONST_INT || 	((op0)->code)  == CONST_DOUBLE	|| 	((op0)->code)  == CONST || 	((op0)->code)  == HIGH) )
	  {
	    enum machine_mode mode = ((((tem)->common.type) )->type.mode) ;
	    if ((	((op0)->code)  != CONST_DOUBLE || 	((op0)->mode)  == VOIDmode) )
	      op0 = force_reg (mode, op0);
	    else
	      op0 = validize_mem (force_const_mem (mode, op0));
	  }
	if (offset != 0)
	  {
	    rtx offset_rtx = expand_expr (offset, (rtx) 0 , VOIDmode, 0);
	    if (	((op0)->code)  != MEM)
	      abort ();
	    op0 = change_address (op0, VOIDmode,
				  gen_rtx (PLUS, SImode , ((op0)->fld[ 0].rtx) ,
					   force_reg (SImode , offset_rtx)));
	  }
	if (	((op0)->code)  == MEM && volatilep && ! ((op0)->volatil) )
	  {
	    op0 = copy_rtx (op0);
	    ((op0)->volatil)  = 1;
	  }
	if (mode1 == VOIDmode
	    || (mode1 != BLKmode && ! direct_load[(int) mode1]
		&& modifier != EXPAND_CONST_ADDRESS
		&& modifier != EXPAND_SUM && modifier != EXPAND_INITIALIZER)
	    || 	((op0)->code)  == REG || 	((op0)->code)  == SUBREG)
	  {
	    enum machine_mode ext_mode = mode;
	    if (ext_mode == BLKmode)
	      ext_mode = mode_for_size (bitsize, MODE_INT, 1);
	    if (ext_mode == BLKmode)
	      abort ();
	    op0 = extract_bit_field (validize_mem (op0), bitsize, bitpos,
				     unsignedp, target, ext_mode, ext_mode,
				     ((((tem)->common.type) )->type.align)  / 8 ,
				     int_size_in_bytes (((tem)->common.type) ));
	    if (mode == BLKmode)
	      {
		rtx new = assign_stack_temp (ext_mode,
					     bitsize / 8 , 0);
		emit_move_insn (new, op0);
		op0 = copy_rtx (new);
		((op0)->mode = ( BLKmode)) ;
	      }
	    return op0;
	  }
	if (modifier == EXPAND_CONST_ADDRESS
	    || modifier == EXPAND_SUM || modifier == EXPAND_INITIALIZER)
	  op0 = gen_rtx (MEM, mode1, plus_constant_wide (((op0)->fld[ 0].rtx) , (int ) (
						    (bitpos / 8 ))) );
	else
	  op0 = change_address (op0, mode1,
				plus_constant_wide (((op0)->fld[ 0].rtx) , (int ) (
					       (bitpos / 8 ))) );
	((op0)->in_struct)  = 1;
	((op0)->volatil)  |= volatilep;
	if (mode == mode1 || mode1 == BLKmode || mode1 == tmode)
	  return op0;
	if (target == 0)
	  target = gen_reg_rtx (tmode != VOIDmode ? tmode : mode);
	convert_move (target, op0, unsignedp);
	return target;
      }
    case OFFSET_REF:
      {
	tree base = build_unary_op (ADDR_EXPR, ((exp)->exp.operands[ 0]) , 0);
	tree addr = build (PLUS_EXPR, type, base, ((exp)->exp.operands[ 1]) );
	op0 = expand_expr (addr, (rtx) 0 , VOIDmode, EXPAND_SUM);
	temp = gen_rtx (MEM, mode, memory_address (mode, op0));
	((temp)->in_struct)  = 1;
	((temp)->volatil)  = ((exp)->common.volatile_flag)  || flag_volatile;
	return temp;
      }
    case BUFFER_REF:
      abort ();
    case IN_EXPR:
      preexpand_calls (exp);
      {
	tree set = ((exp)->exp.operands[ 0]) ;
	tree index = ((exp)->exp.operands[ 1]) ;
	tree set_type = ((set)->common.type) ;
	tree set_low_bound = ((((set_type)->type.values) )->type.minval) ;
	tree set_high_bound = ((((set_type)->type.values) )->type.maxval) ;
	rtx index_val;
	rtx lo_r;
	rtx hi_r;
	rtx rlow;
	rtx diff, quo, rem, addr, bit, result;
	rtx setval, setaddr;
	enum machine_mode index_mode = ((((index)->common.type) )->type.mode) ;
	if (target == 0)
	  target = gen_reg_rtx (((((exp)->common.type) )->type.mode) );
	if (tree_int_cst_lt (set_high_bound, set_low_bound))
	  return const0_rtx;
	index_val = expand_expr (index, 0, VOIDmode, 0);
	lo_r = expand_expr (set_low_bound, 0, VOIDmode, 0);
	hi_r = expand_expr (set_high_bound, 0, VOIDmode, 0);
	setval = expand_expr (set, 0, VOIDmode, 0);
	setaddr = ((setval)->fld[ 0].rtx) ; 
	if (	((index_val)->code)  == CONST_INT
	    && 	((lo_r)->code)  == CONST_INT)
	  {
	    if (((index_val)->fld[0].rtwint)  < ((lo_r)->fld[0].rtwint) )
	      return const0_rtx;
	  }
	if (	((index_val)->code)  == CONST_INT
	    && 	((hi_r)->code)  == CONST_INT)
	  {
	    if (((hi_r)->fld[0].rtwint)  < ((index_val)->fld[0].rtwint) )
	      return const0_rtx;
	  }
	op0 = gen_label_rtx ();
	op1 = gen_label_rtx ();
	if (! (	((index_val)->code)  == CONST_INT
	       && 	((lo_r)->code)  == CONST_INT))
	  {
	    emit_cmp_insn (index_val, lo_r, LT, 0, 	((index_val)->mode) , 0, 0);
	    emit_jump_insn (gen_blt (op1));
	  }
	if (! (	((index_val)->code)  == CONST_INT
	       && 	((hi_r)->code)  == CONST_INT))
	  {
	    emit_cmp_insn (index_val, hi_r, GT, 0, 	((index_val)->mode) , 0, 0);
	    emit_jump_insn (gen_bgt (op1));
	  }
	if (	((lo_r)->code)  == CONST_INT)
	  rlow = gen_rtx (CONST_INT, VOIDmode,
			  ((lo_r)->fld[0].rtwint)  & ~ (1 << 8 ));
	else
	  rlow = expand_binop (index_mode, and_optab,
			       lo_r, gen_rtx (CONST_INT, VOIDmode,
					      ~ (1 << 8 )),
			       0, 0, OPTAB_LIB_WIDEN);
	diff = expand_binop (index_mode, sub_optab,
			     index_val, rlow, 0, 0, OPTAB_LIB_WIDEN);
	quo = expand_divmod (0, TRUNC_DIV_EXPR, index_mode, diff,
			     gen_rtx (CONST_INT, VOIDmode, 8 ),
			     0, 0);
	rem = expand_divmod (1, TRUNC_MOD_EXPR, index_mode, index_val,
			     gen_rtx (CONST_INT, VOIDmode, 8 ),
			     0, 0);
	addr = memory_address (byte_mode,
			       expand_binop (index_mode, add_optab,
					     diff, setaddr));
	bit = expand_shift (RSHIFT_EXPR, byte_mode,
			    gen_rtx (MEM, byte_mode, addr), rem, 0, 1);
	result = expand_binop (SImode, and_optab, bit, const1_rtx, target,
			       1, OPTAB_LIB_WIDEN);
	emit_move_insn (target, result);
	emit_jump (op0);
	emit_label (op1);
	emit_move_insn (target, const0_rtx);
	emit_label (op0);
	return target;
      }
    case WITH_CLEANUP_EXPR:
      if ((*(struct rtx_def **) &(exp)->exp.operands[1])  == 0)
	{
	  (*(struct rtx_def **) &(exp)->exp.operands[1]) 
	    = expand_expr (((exp)->exp.operands[ 0]) , target, tmode, modifier);
	  cleanups_this_call
	    = tree_cons ((tree) 0  , ((exp)->exp.operands[ 2]) , cleanups_this_call);
	  ((exp)->exp.operands[ 2])  = 0;
	}
      return (*(struct rtx_def **) &(exp)->exp.operands[1]) ;
    case CALL_EXPR:
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == ADDR_EXPR
	  && ((enum tree_code) (((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.code)  == FUNCTION_DECL
	  && ((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->decl.bit_field_flag) )
	return expand_builtin (exp, target, subtarget, tmode, ignore);
      if ((*(struct rtx_def **) &(exp)->exp.operands[2])  != 0)
	return (*(struct rtx_def **) &(exp)->exp.operands[2]) ;
      return expand_call (exp, target, ignore);
    case NON_LVALUE_EXPR:
    case NOP_EXPR:
    case CONVERT_EXPR:
    case REFERENCE_EXPR:
      if (((enum tree_code) (type)->common.code)  == VOID_TYPE || ignore)
	{
	  expand_expr (((exp)->exp.operands[ 0]) , const0_rtx, VOIDmode, modifier);
	  return const0_rtx;
	}
      if (mode == ((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )
	return expand_expr (((exp)->exp.operands[ 0]) , target, VOIDmode, modifier);
      if (((enum tree_code) (type)->common.code)  == UNION_TYPE)
	{
	  tree valtype = ((((exp)->exp.operands[ 0]) )->common.type) ;
	  if (target == 0)
	    {
	      if (mode == BLKmode)
		{
		  if (((type)->type.size)  == 0
		      || ((enum tree_code) (((type)->type.size) )->common.code)  != INTEGER_CST)
		    abort ();
		  target = assign_stack_temp (BLKmode,
					      (((((type)->type.size) )->int_cst.int_cst_low) 
					       + 8  - 1)
					      / 8 , 0);
		}
	      else
		target = gen_reg_rtx (mode);
	    }
	  if (	((target)->code)  == MEM)
	    store_expr (((exp)->exp.operands[ 0]) ,
			change_address (target, ((valtype)->type.mode) , 0), 0);
	  else if (	((target)->code)  == REG)
	    store_field (target,  (8  * mode_size[(int)(((valtype)->type.mode) )]) , 0,
			 ((valtype)->type.mode) , ((exp)->exp.operands[ 0]) ,
			 VOIDmode, 0, 1,
			 int_size_in_bytes (((((exp)->exp.operands[ 0]) )->common.type) ));
	  else
	    abort ();
	  return target;
	}
      op0 = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 , mode, 0);
      if (	((op0)->mode)  == mode || 	((op0)->mode)  == VOIDmode)
	return op0;
      if (modifier == EXPAND_INITIALIZER)
	return gen_rtx (unsignedp ? ZERO_EXTEND : SIGN_EXTEND, mode, op0);
      if (flag_force_mem && 	((op0)->code)  == MEM)
	op0 = copy_to_reg (op0);
      if (target == 0)
	return convert_to_mode (mode, op0, ((((((exp)->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) );
      else
	convert_move (target, op0, ((((((exp)->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) );
      return target;
    case PLUS_EXPR:
    plus_expr:
      this_optab = add_optab;
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == PLUS_EXPR
	  && ((enum tree_code) (((((exp)->exp.operands[ 0]) )->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	  && ((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == RTL_EXPR
	  && ((*(struct rtx_def **) &(((exp)->exp.operands[ 1]) )->exp.operands[1])  == frame_pointer_rtx
	      || (*(struct rtx_def **) &(((exp)->exp.operands[ 1]) )->exp.operands[1])  == stack_pointer_rtx
	      || (*(struct rtx_def **) &(((exp)->exp.operands[ 1]) )->exp.operands[1])  == arg_pointer_rtx))
	{
	  tree t = ((exp)->exp.operands[ 1]) ;
	  ((exp)->exp.operands[ 1])  = ((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) ;
	  ((((exp)->exp.operands[ 0]) )->exp.operands[ 0])  = t;
	}
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == INTEGER_CST
	  &&  (8  * mode_size[(int)(mode)])  <= 32  
	  && (modifier == EXPAND_SUM || modifier == EXPAND_INITIALIZER
	      || mode == SImode ))
	{
	  op1 = expand_expr (((exp)->exp.operands[ 1]) , subtarget, VOIDmode,
			     EXPAND_SUM);
	  op1 = plus_constant_wide (op1, (int ) ( ((((exp)->exp.operands[ 0]) )->int_cst.int_cst_low) )) ;
	  if (modifier != EXPAND_SUM && modifier != EXPAND_INITIALIZER)
	    op1 = force_operand (op1, target);
	  return op1;
	}
      else if (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	       &&  (8  * mode_size[(int)(mode)])  <= 32 
	       && (modifier == EXPAND_SUM || modifier == EXPAND_INITIALIZER
		   || mode == SImode ))
	{
	  op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode,
			     EXPAND_SUM);
	  op0 = plus_constant_wide (op0, (int ) ( ((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) )) ;
	  if (modifier != EXPAND_SUM && modifier != EXPAND_INITIALIZER)
	    op0 = force_operand (op0, target);
	  return op0;
	}
      if ((modifier != EXPAND_SUM && modifier != EXPAND_INITIALIZER)
	  || mode != SImode ) goto binop;
      preexpand_calls (exp);
      if (! safe_from_p (subtarget, ((exp)->exp.operands[ 1]) ))
	subtarget = 0;
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, modifier);
      op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, modifier);
      if (	((op0)->code)  == PLUS
	  && (	((((op0)->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((op0)->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((op0)->fld[ 1].rtx) )->code)  == CONST_INT || 	((((op0)->fld[ 1].rtx) )->code)  == CONST_DOUBLE	|| 	((((op0)->fld[ 1].rtx) )->code)  == CONST || 	((((op0)->fld[ 1].rtx) )->code)  == HIGH) )
	{
	  temp = op0;
	  op0 = op1;
	  op1 = temp;
	}
      if (	((op1)->code)  == PLUS
	  && (	((((op1)->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((op1)->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((op1)->fld[ 1].rtx) )->code)  == CONST_INT || 	((((op1)->fld[ 1].rtx) )->code)  == CONST_DOUBLE	|| 	((((op1)->fld[ 1].rtx) )->code)  == CONST || 	((((op1)->fld[ 1].rtx) )->code)  == HIGH) )
	{
	  rtx constant_term = const0_rtx;
	  temp = simplify_binary_operation (PLUS, mode, ((op1)->fld[ 0].rtx) , op0);
	  if (temp != 0)
	    op0 = temp;
	  else if (	((op0)->code)  == MULT)
	    op0 = gen_rtx (PLUS, mode, op0, ((op1)->fld[ 0].rtx) );
	  else
	    op0 = gen_rtx (PLUS, mode, ((op1)->fld[ 0].rtx) , op0);
	  op0 = eliminate_constant_term (op0, &constant_term);
	  temp = simplify_binary_operation (PLUS, mode, constant_term,
					    ((op1)->fld[ 1].rtx) );
	  if (temp != 0)
	    op1 = temp;
	  else
	    op1 = gen_rtx (PLUS, mode, constant_term, ((op1)->fld[ 1].rtx) );
	}
      if ((	((op0)->code)  == LABEL_REF || 	((op0)->code)  == SYMBOL_REF	|| 	((op0)->code)  == CONST_INT || 	((op0)->code)  == CONST_DOUBLE	|| 	((op0)->code)  == CONST || 	((op0)->code)  == HIGH)  || 	((op1)->code)  == MULT)
	temp = op1, op1 = op0, op0 = temp;
      temp = simplify_binary_operation (PLUS, mode, op0, op1);
      return temp ? temp : gen_rtx (PLUS, mode, op0, op1);
    case MINUS_EXPR:
      if ((modifier == EXPAND_SUM || modifier == EXPAND_INITIALIZER)
	  && really_constant_p (((exp)->exp.operands[ 0]) )
	  && really_constant_p (((exp)->exp.operands[ 1]) ))
	{
	  rtx op0 = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 ,
				 VOIDmode, modifier);
	  rtx op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 ,
				 VOIDmode, modifier);
	  return gen_rtx (MINUS, mode, op0, op1);
	}
      if (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST)
	{
	  exp = build (PLUS_EXPR, type, ((exp)->exp.operands[ 0]) ,
		       fold (build1 (NEGATE_EXPR, type,
				     ((exp)->exp.operands[ 1]) )));
	  goto plus_expr;
	}
      this_optab = sub_optab;
      goto binop;
    case MULT_EXPR:
      preexpand_calls (exp);
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == INTEGER_CST)
	{
	  register tree t1 = ((exp)->exp.operands[ 0]) ;
	  ((exp)->exp.operands[ 0])  = ((exp)->exp.operands[ 1]) ;
	  ((exp)->exp.operands[ 1])  = t1;
	}
      if (modifier == EXPAND_SUM && mode == SImode 
	  && ((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	  &&  (8  * mode_size[(int)(mode)])  <= 32  )
	{
	  op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, EXPAND_SUM);
	  if (	((op0)->code)  == PLUS
	      && 	((((op0)->fld[ 1].rtx) )->code)  == CONST_INT)
	    return gen_rtx (PLUS, mode,
			    gen_rtx (MULT, mode, ((op0)->fld[ 0].rtx) ,
				     gen_rtx (CONST_INT, VOIDmode, (((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) )) ),
			    gen_rtx (CONST_INT, VOIDmode, (((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) 
				     * ((((op0)->fld[ 1].rtx) )->fld[0].rtwint) )) );
	  if (	((op0)->code)  != REG)
	    op0 = force_operand (op0, (rtx) 0 );
	  if (	((op0)->code)  != REG)
	    op0 = copy_to_mode_reg (mode, op0);
	  return gen_rtx (MULT, mode, op0,
			  gen_rtx (CONST_INT, VOIDmode, (((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) )) );
	}
      if (! safe_from_p (subtarget, ((exp)->exp.operands[ 1]) ))
	subtarget = 0;
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == NOP_EXPR
	  && ((enum tree_code) (type)->common.code)  == INTEGER_TYPE
	  && (((((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.type) )->type.precision) 
	      < ((((((exp)->exp.operands[ 0]) )->common.type) )->type.precision) )
	  && ((((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	       && int_fits_type_p (((exp)->exp.operands[ 1]) ,
				   ((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.type) )
	       && (( (8  * mode_size[(int)(((((((exp)->exp.operands[ 1]) )->common.type) )->type.mode) )]) 
		    > 32  )
		   || exact_log2_wide ((int ) (((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) ))  < 0))
	      ||
	      (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == NOP_EXPR
	       && (((((((((exp)->exp.operands[ 1]) )->exp.operands[ 0]) )->common.type) )->type.precision) 
		   ==
		   ((((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.type) )->type.precision) )
	       && (((((((((exp)->exp.operands[ 1]) )->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) 
		   ==
		   ((((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) ))))
	{
	  enum machine_mode innermode
	    = ((((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.type) )->type.mode) ;
	  this_optab = (((((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) 
			? umul_widen_optab : smul_widen_optab);
	  if (mode == (mode_wider_mode[(int)(innermode)]) 
	      && this_optab->handlers[(int) mode].insn_code != CODE_FOR_nothing)
	    {
	      op0 = expand_expr (((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) ,
				 (rtx) 0 , VOIDmode, 0);
	      if (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST)
		op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 ,
				   VOIDmode, 0);
	      else
		op1 = expand_expr (((((exp)->exp.operands[ 1]) )->exp.operands[ 0]) ,
				   (rtx) 0 , VOIDmode, 0);
	      goto binop2;
	    }
	}
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
      return expand_mult (mode, op0, op1, target, unsignedp);
    case TRUNC_DIV_EXPR:
    case FLOOR_DIV_EXPR:
    case CEIL_DIV_EXPR:
    case ROUND_DIV_EXPR:
    case EXACT_DIV_EXPR:
      preexpand_calls (exp);
      if (! safe_from_p (subtarget, ((exp)->exp.operands[ 1]) ))
	subtarget = 0;
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
      return expand_divmod (0, code, mode, op0, op1, target, unsignedp);
    case RDIV_EXPR:
      this_optab = flodiv_optab;
      goto binop;
    case TRUNC_MOD_EXPR:
    case FLOOR_MOD_EXPR:
    case CEIL_MOD_EXPR:
    case ROUND_MOD_EXPR:
      preexpand_calls (exp);
      if (! safe_from_p (subtarget, ((exp)->exp.operands[ 1]) ))
	subtarget = 0;
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
      return expand_divmod (1, code, mode, op0, op1, target, unsignedp);
    case FIX_ROUND_EXPR:
    case FIX_FLOOR_EXPR:
    case FIX_CEIL_EXPR:
      abort ();			 
    case FIX_TRUNC_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 , VOIDmode, 0);
      if (target == 0)
	target = gen_reg_rtx (mode);
      expand_fix (target, op0, unsignedp);
      return target;
    case FLOAT_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 , VOIDmode, 0);
      if (target == 0)
	target = gen_reg_rtx (mode);
      if (	((op0)->mode)  == VOIDmode)
	op0 = copy_to_mode_reg (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ,
				op0);
      expand_float (target, op0,
		    ((((((exp)->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) );
      return target;
    case NEGATE_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , target, VOIDmode, 0);
      temp = expand_unop (mode, neg_optab, op0, target, 0);
      if (temp == 0)
	abort ();
      return temp;
    case ABS_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      {
	enum machine_mode opmode
	  = ((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ;
	if (	(mode_class[(int)(opmode)])  == MODE_COMPLEX_INT
	    || 	(mode_class[(int)(opmode)])  == MODE_COMPLEX_FLOAT)
	  return expand_complex_abs (opmode, op0, target, unsignedp);
      }
      if (((type)->common.unsigned_flag) )
	return op0;
      temp = expand_unop (mode, abs_optab, op0, target, 0);
      if (temp != 0)
	return temp;
      if (	(mode_class[(int)(mode)])  == MODE_INT && 1  >= 2)
	{
	  rtx extended = expand_shift (RSHIFT_EXPR, mode, op0,
				       size_int ( (8  * mode_size[(int)(mode)])  - 1),
				       (rtx) 0 , 0);
	  temp = expand_binop (mode, xor_optab, extended, op0, target, 0,
			       OPTAB_LIB_WIDEN);
	  if (temp != 0)
	    temp = expand_binop (mode, sub_optab, temp, extended, target, 0,
				 OPTAB_LIB_WIDEN);
	  if (temp != 0)
	    return temp;
	}
      target = original_target;
      temp = gen_label_rtx ();
      if (target == 0 || ! safe_from_p (target, ((exp)->exp.operands[ 0]) )
	  || (	((target)->code)  == REG
	      && ((target)->fld[0].rtint)  < 64 ))
	target = gen_reg_rtx (mode);
      emit_move_insn (target, op0);
      emit_cmp_insn (target,
		     expand_expr (convert (type, integer_zero_node),
				  (rtx) 0 , VOIDmode, 0),
		     GE, (rtx) 0 , mode, 0, 0);
      (inhibit_defer_pop += 1) ;
      emit_jump_insn (gen_bge (temp));
      op0 = expand_unop (mode, neg_optab, target, target, 0);
      if (op0 != target)
	emit_move_insn (target, op0);
      emit_label (temp);
      (inhibit_defer_pop -= 1) ;
      return target;
    case MAX_EXPR:
    case MIN_EXPR:
      target = original_target;
      if (target == 0 || ! safe_from_p (target, ((exp)->exp.operands[ 1]) )
	  || (	((target)->code)  == REG
	      && ((target)->fld[0].rtint)  < 64 ))
	target = gen_reg_rtx (mode);
      op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
      op0 = expand_expr (((exp)->exp.operands[ 0]) , target, VOIDmode, 0);
      this_optab = (((type)->common.unsigned_flag) 
		    ? (code == MIN_EXPR ? umin_optab : umax_optab)
		    : (code == MIN_EXPR ? smin_optab : smax_optab));
      temp = expand_binop (mode, this_optab, op0, op1, target, unsignedp,
			   OPTAB_WIDEN);
      if (temp != 0)
	return temp;
      if (target != op0)
	emit_move_insn (target, op0);
      op0 = gen_label_rtx ();
      if (code == MAX_EXPR)
	temp = (((((((exp)->exp.operands[ 1]) )->common.type) )->common.unsigned_flag) 
		? compare_from_rtx (target, op1, GEU, 1, mode, (rtx) 0 , 0)
		: compare_from_rtx (target, op1, GE, 0, mode, (rtx) 0 , 0));
      else
	temp = (((((((exp)->exp.operands[ 1]) )->common.type) )->common.unsigned_flag) 
		? compare_from_rtx (target, op1, LEU, 1, mode, (rtx) 0 , 0)
		: compare_from_rtx (target, op1, LE, 0, mode, (rtx) 0 , 0));
      if (temp == const0_rtx)
	emit_move_insn (target, op1);
      else if (temp != const_true_rtx)
	{
	  if (bcc_gen_fctn[(int) 	((temp)->code) ] != 0)
	    emit_jump_insn ((*bcc_gen_fctn[(int) 	((temp)->code) ]) (op0));
	  else
	    abort ();
	  emit_move_insn (target, op1);
	}
      emit_label (op0);
      return target;
    case BIT_NOT_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      temp = expand_unop (mode, one_cmpl_optab, op0, target, 1);
      if (temp == 0)
	abort ();
      return temp;
    case FFS_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      temp = expand_unop (mode, ffs_optab, op0, target, 1);
      if (temp == 0)
	abort ();
      return temp;
    case TRUTH_AND_EXPR:
    case BIT_AND_EXPR:
      this_optab = and_optab;
      goto binop;
    case TRUTH_OR_EXPR:
    case BIT_IOR_EXPR:
      this_optab = ior_optab;
      goto binop;
    case BIT_XOR_EXPR:
      this_optab = xor_optab;
      goto binop;
    case LSHIFT_EXPR:
    case RSHIFT_EXPR:
    case LROTATE_EXPR:
    case RROTATE_EXPR:
      preexpand_calls (exp);
      if (! safe_from_p (subtarget, ((exp)->exp.operands[ 1]) ))
	subtarget = 0;
      op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      return expand_shift (code, mode, op0, ((exp)->exp.operands[ 1]) , target,
			   unsignedp);
    case LT_EXPR:
    case LE_EXPR:
    case GT_EXPR:
    case GE_EXPR:
    case EQ_EXPR:
    case NE_EXPR:
      preexpand_calls (exp);
      temp = do_store_flag (exp, target, tmode != VOIDmode ? tmode : mode, 0);
      if (temp != 0)
	return temp;
      if (code == NE_EXPR && integer_zerop (((exp)->exp.operands[ 1]) )
	  && original_target
	  && 	((original_target)->code)  == REG
	  && (	((original_target)->mode) 
	      == ((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	{
	  temp = expand_expr (((exp)->exp.operands[ 0]) , original_target, VOIDmode, 0);
	  if (temp != original_target)
	    temp = copy_to_reg (temp);
	  op1 = gen_label_rtx ();
	  emit_cmp_insn (temp, const0_rtx, EQ, (rtx) 0 ,
			 	((temp)->mode) , unsignedp, 0);
	  emit_jump_insn (gen_beq (op1));
	  emit_move_insn (temp, const1_rtx);
	  emit_label (op1);
	  return temp;
	}
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
      if (target == 0 || ! safe_from_p (target, exp)
	  || (!optimize && 	((target)->code)  == REG
	      && ((target)->fld[0].rtint)  < 64 ))
	target = gen_reg_rtx (tmode != VOIDmode ? tmode : mode);
      emit_clr_insn (target);
      op1 = gen_label_rtx ();
      jumpifnot (exp, op1);
      emit_0_to_1_insn (target);
      emit_label (op1);
      return target;
    case TRUTH_NOT_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , target, VOIDmode, 0);
      temp = expand_binop (mode, xor_optab, op0, const1_rtx,
			   target, 1, OPTAB_LIB_WIDEN);
      if (temp == 0)
	abort ();
      return temp;
    case COMPOUND_EXPR:
      expand_expr (((exp)->exp.operands[ 0]) , const0_rtx, VOIDmode, 0);
      emit_queue ();
      return expand_expr (((exp)->exp.operands[ 1]) ,
			  (ignore ? const0_rtx : target),
			  VOIDmode, 0);
    case COND_EXPR:
      {
	tree singleton = 0;
	tree binary_op = 0, unary_op = 0;
	tree old_cleanups = cleanups_this_call;
	cleanups_this_call = 0;
	if (integer_onep (((exp)->exp.operands[ 1]) )
	    && integer_zerop (((exp)->exp.operands[ 2]) )
	    && (*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code) )])  == '<')
	  {
	    op0 = expand_expr (((exp)->exp.operands[ 0]) , target, mode, modifier);
	    if (	((op0)->mode)  == mode)
	      return op0;
	    if (target == 0)
	      target = gen_reg_rtx (mode);
	    convert_move (target, op0, unsignedp);
	    return target;
	  }
	if (mode == VOIDmode || ignore)
	  temp = 0;
	else if (original_target
		 && safe_from_p (original_target, ((exp)->exp.operands[ 0]) ))
	  temp = original_target;
	else if (mode == BLKmode)
	  {
	    if (((type)->type.size)  == 0
		|| ((enum tree_code) (((type)->type.size) )->common.code)  != INTEGER_CST)
	      abort ();
	    temp = assign_stack_temp (BLKmode,
				      (((((type)->type.size) )->int_cst.int_cst_low) 
				       + 8  - 1)
				      / 8 , 0);
	  }
	else
	  temp = gen_reg_rtx (mode);
	if ((*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code) )])  == '2'
	    && operand_equal_p (((exp)->exp.operands[ 2]) ,
				((((exp)->exp.operands[ 1]) )->exp.operands[ 0]) , 0))
	  singleton = ((exp)->exp.operands[ 2]) , binary_op = ((exp)->exp.operands[ 1]) ;
	else if ((*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 2]) )->common.code) )])  == '2'
		 && operand_equal_p (((exp)->exp.operands[ 1]) ,
				     ((((exp)->exp.operands[ 2]) )->exp.operands[ 0]) , 0))
	  singleton = ((exp)->exp.operands[ 1]) , binary_op = ((exp)->exp.operands[ 2]) ;
	else if ((*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code) )])  == '1'
		 && operand_equal_p (((exp)->exp.operands[ 2]) ,
				     ((((exp)->exp.operands[ 1]) )->exp.operands[ 0]) , 0))
	  singleton = ((exp)->exp.operands[ 2]) , unary_op = ((exp)->exp.operands[ 1]) ;
	else if ((*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 2]) )->common.code) )])  == '1'
		 && operand_equal_p (((exp)->exp.operands[ 1]) ,
				     ((((exp)->exp.operands[ 2]) )->exp.operands[ 0]) , 0))
	  singleton = ((exp)->exp.operands[ 1]) , unary_op = ((exp)->exp.operands[ 2]) ;
	if (singleton && binary_op
	    && ! ((((exp)->exp.operands[ 0]) )->common.side_effects_flag) 
	    && (((enum tree_code) (binary_op)->common.code)  == PLUS_EXPR
		|| ((enum tree_code) (binary_op)->common.code)  == MINUS_EXPR
		|| ((enum tree_code) (binary_op)->common.code)  == BIT_IOR_EXPR
		|| ((enum tree_code) (binary_op)->common.code)  == BIT_XOR_EXPR
		|| ((enum tree_code) (binary_op)->common.code)  == BIT_AND_EXPR)
	    && integer_onep (((binary_op)->exp.operands[ 1]) )
	    && (*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code) )])  == '<')
	  {
	    rtx result;
	    optab boptab = (((enum tree_code) (binary_op)->common.code)  == PLUS_EXPR ? add_optab
			    : ((enum tree_code) (binary_op)->common.code)  == MINUS_EXPR ? sub_optab
			    : ((enum tree_code) (binary_op)->common.code)  == BIT_IOR_EXPR ? ior_optab
			    : ((enum tree_code) (binary_op)->common.code)  == BIT_XOR_EXPR ? xor_optab
			    : and_optab);
	    if (singleton == ((exp)->exp.operands[ 1]) )
	      ((exp)->exp.operands[ 0]) 
		= invert_truthvalue (((exp)->exp.operands[ 0]) );
	    result = do_store_flag (((exp)->exp.operands[ 0]) ,
				    (safe_from_p (temp, singleton)
				     ? temp : (rtx) 0 ),
				    mode, 1  <= 1);
	    if (result)
	      {
		op1 = expand_expr (singleton, (rtx) 0 , VOIDmode, 0);
		return expand_binop (mode, boptab, op1, result, temp,
				     unsignedp, OPTAB_LIB_WIDEN);
	      }
	    else if (singleton == ((exp)->exp.operands[ 1]) )
	      ((exp)->exp.operands[ 0]) 
		= invert_truthvalue (((exp)->exp.operands[ 0]) );
	  }
	(inhibit_defer_pop += 1) ;
	op0 = gen_label_rtx ();
	if (singleton && ! ((((exp)->exp.operands[ 0]) )->common.side_effects_flag) )
	  {
	    if (temp != 0)
	      {
		if ((binary_op
		     && ! safe_from_p (temp, ((binary_op)->exp.operands[ 1]) ))
		    || (	((temp)->code)  == REG
			&& ((temp)->fld[0].rtint)  < 64 ))
		  temp = gen_reg_rtx (mode);
		store_expr (singleton, temp, 0);
	      }
	    else
	      expand_expr (singleton,
			   ignore ? const1_rtx : (rtx) 0 , VOIDmode, 0);
	    if (cleanups_this_call)
	      {
		sorry ("aggregate value in COND_EXPR");
		cleanups_this_call = 0;
	      }
	    if (singleton == ((exp)->exp.operands[ 1]) )
	      jumpif (((exp)->exp.operands[ 0]) , op0);
	    else
	      jumpifnot (((exp)->exp.operands[ 0]) , op0);
	    if (binary_op && temp == 0)
	      expand_expr (((binary_op)->exp.operands[ 1]) ,
			   ignore ? const0_rtx : (rtx) 0 , VOIDmode, 0);
	    else if (binary_op)
	      store_expr (build (((enum tree_code) (binary_op)->common.code) , type,
				 make_tree (type, temp),
				 ((binary_op)->exp.operands[ 1]) ),
			  temp, 0);
	    else
	      store_expr (build1 (((enum tree_code) (unary_op)->common.code) , type,
				  make_tree (type, temp)),
			  temp, 0);
	    op1 = op0;
	  }
	else if (temp
		 && (*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code) )])  == '<'
		 && integer_zerop (((((exp)->exp.operands[ 0]) )->exp.operands[ 1]) )
		 && operand_equal_p (((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) ,
				     ((exp)->exp.operands[ 1]) , 0)
		 && ! ((((exp)->exp.operands[ 0]) )->common.side_effects_flag) 
		 && safe_from_p (temp, ((exp)->exp.operands[ 2]) ))
	  {
	    if (	((temp)->code)  == REG && ((temp)->fld[0].rtint)  < 64 )
	      temp = gen_reg_rtx (mode);
	    store_expr (((exp)->exp.operands[ 1]) , temp, 0);
	    jumpif (((exp)->exp.operands[ 0]) , op0);
	    store_expr (((exp)->exp.operands[ 2]) , temp, 0);
	    op1 = op0;
	  }
	else if (temp
		 && (*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code) )])  == '<'
		 && integer_zerop (((((exp)->exp.operands[ 0]) )->exp.operands[ 1]) )
		 && operand_equal_p (((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) ,
				     ((exp)->exp.operands[ 2]) , 0)
		 && ! ((((exp)->exp.operands[ 0]) )->common.side_effects_flag) 
		 && safe_from_p (temp, ((exp)->exp.operands[ 1]) ))
	  {
	    if (	((temp)->code)  == REG && ((temp)->fld[0].rtint)  < 64 )
	      temp = gen_reg_rtx (mode);
	    store_expr (((exp)->exp.operands[ 2]) , temp, 0);
	    jumpifnot (((exp)->exp.operands[ 0]) , op0);
	    store_expr (((exp)->exp.operands[ 1]) , temp, 0);
	    op1 = op0;
	  }
	else
	  {
	    op1 = gen_label_rtx ();
	    jumpifnot (((exp)->exp.operands[ 0]) , op0);
	    if (temp != 0)
	      store_expr (((exp)->exp.operands[ 1]) , temp, 0);
	    else
	      expand_expr (((exp)->exp.operands[ 1]) ,
			   ignore ? const0_rtx : (rtx) 0 , VOIDmode, 0);
	    if (cleanups_this_call)
	      {
		sorry ("aggregate value in COND_EXPR");
		cleanups_this_call = 0;
	      }
	    emit_queue ();
	    emit_jump_insn (gen_jump (op1));
	    emit_barrier ();
	    emit_label (op0);
	    if (temp != 0)
	      store_expr (((exp)->exp.operands[ 2]) , temp, 0);
	    else
	      expand_expr (((exp)->exp.operands[ 2]) ,
			   ignore ? const0_rtx : (rtx) 0 , VOIDmode, 0);
	  }
	if (cleanups_this_call)
	  {
	    sorry ("aggregate value in COND_EXPR");
	    cleanups_this_call = 0;
	  }
	emit_queue ();
	emit_label (op1);
	(inhibit_defer_pop -= 1) ;
	cleanups_this_call = old_cleanups;
	return temp;
      }
    case TARGET_EXPR:
      {
	tree slot = ((exp)->exp.operands[ 0]) ;
	tree exp1;
	if (((enum tree_code) (slot)->common.code)  != VAR_DECL)
	  abort ();
	if (target == 0)
	  {
	    if (((slot)->decl.rtl)  != 0)
	      {
		target = ((slot)->decl.rtl) ;
		if (((exp)->exp.operands[ 1])  == (tree) 0  )
		  return target;
	      }
	    else
	      {
		target = assign_stack_temp (mode, int_size_in_bytes (type), 0);
		preserve_temp_slots (target);
		((slot)->decl.rtl)  = target;
	      }
	  }
	else
	  {
	    ((slot)->decl.rtl)  = target;
	  }
	exp1 = ((exp)->exp.operands[ 1]) ;
	((exp)->exp.operands[ 1])  = (tree) 0  ;
	return expand_expr (exp1, target, tmode, modifier);
      }
    case INIT_EXPR:
      {
	tree lhs = ((exp)->exp.operands[ 0]) ;
	tree rhs = ((exp)->exp.operands[ 1]) ;
	tree noncopied_parts = 0;
	tree lhs_type = ((lhs)->common.type) ;
	temp = expand_assignment (lhs, rhs, ! ignore, original_target != 0);
	if (((lhs_type)->type.noncopied_parts)  != 0 && !fixed_type_p (rhs))
	  noncopied_parts = init_noncopied_parts (stabilize_reference (lhs),
						  ((lhs_type)->type.noncopied_parts) );
	while (noncopied_parts != 0)
	  {
	    expand_assignment (((noncopied_parts)->list.value) ,
			       ((noncopied_parts)->list.purpose) , 0, 0);
	    noncopied_parts = ((noncopied_parts)->common.chain) ;
	  }
	return temp;
      }
    case MODIFY_EXPR:
      {
	tree lhs = ((exp)->exp.operands[ 0]) ;
	tree rhs = ((exp)->exp.operands[ 1]) ;
	tree noncopied_parts = 0;
	tree lhs_type = ((lhs)->common.type) ;
	temp = 0;
	if (((enum tree_code) (lhs)->common.code)  != VAR_DECL
	    && ((enum tree_code) (lhs)->common.code)  != RESULT_DECL
	    && ((enum tree_code) (lhs)->common.code)  != PARM_DECL)
	  preexpand_calls (exp);
	if (ignore
	    && ((enum tree_code) (lhs)->common.code)  == COMPONENT_REF
	    && (((enum tree_code) (rhs)->common.code)  == BIT_IOR_EXPR
		|| ((enum tree_code) (rhs)->common.code)  == BIT_AND_EXPR)
	    && ((rhs)->exp.operands[ 0])  == lhs
	    && ((enum tree_code) (((rhs)->exp.operands[ 1]) )->common.code)  == COMPONENT_REF
	    && ((((((lhs)->exp.operands[ 1]) )->decl.size) )->int_cst.int_cst_low)  == 1
	    && ((((((((rhs)->exp.operands[ 1]) )->exp.operands[ 1]) )->decl.size) )->int_cst.int_cst_low)  == 1)
	  {
	    rtx label = gen_label_rtx ();
	    do_jump (((rhs)->exp.operands[ 1]) ,
		     ((enum tree_code) (rhs)->common.code)  == BIT_IOR_EXPR ? label : 0,
		     ((enum tree_code) (rhs)->common.code)  == BIT_AND_EXPR ? label : 0);
	    expand_assignment (lhs, convert (((rhs)->common.type) ,
					     (((enum tree_code) (rhs)->common.code)  == BIT_IOR_EXPR
					      ? integer_one_node
					      : integer_zero_node)),
			       0, 0);
	    do_pending_stack_adjust ();
	    emit_label (label);
	    return const0_rtx;
	  }
	if (((lhs_type)->type.noncopied_parts)  != 0
	    && ! (fixed_type_p (lhs) && fixed_type_p (rhs)))
	  noncopied_parts = save_noncopied_parts (stabilize_reference (lhs),
						  ((lhs_type)->type.noncopied_parts) );
	temp = expand_assignment (lhs, rhs, ! ignore, original_target != 0);
	while (noncopied_parts != 0)
	  {
	    expand_assignment (((noncopied_parts)->list.purpose) ,
			       ((noncopied_parts)->list.value) , 0, 0);
	    noncopied_parts = ((noncopied_parts)->common.chain) ;
	  }
	return temp;
      }
    case PREINCREMENT_EXPR:
    case PREDECREMENT_EXPR:
      return expand_increment (exp, 0);
    case POSTINCREMENT_EXPR:
    case POSTDECREMENT_EXPR:
      return expand_increment (exp, ! ignore);
    case ADDR_EXPR:
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == FUNCTION_DECL
	  && decl_function_context (((exp)->exp.operands[ 0]) ) != 0)
	{
	  op0 = trampoline_address (((exp)->exp.operands[ 0]) );
	  op0 = force_operand (op0, target);
	}
      else
	{
	  op0 = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 , VOIDmode,
			     (modifier == EXPAND_INITIALIZER
			      ? modifier : EXPAND_CONST_ADDRESS));
	  if (	((op0)->code)  != MEM)
	    abort ();
	  if (modifier == EXPAND_SUM || modifier == EXPAND_INITIALIZER)
	    return ((op0)->fld[ 0].rtx) ;
	  op0 = force_operand (((op0)->fld[ 0].rtx) , target);
	}
      if (flag_force_addr && 	((op0)->code)  != REG)
	return force_reg (SImode , op0);
      return op0;
    case ENTRY_VALUE_EXPR:
      abort ();
    case COMPLEX_EXPR:
      {
	enum machine_mode mode = ((((((exp)->common.type) )->common.type) )->type.mode) ;
	rtx prev;
	op0 = expand_expr (((exp)->exp.operands[ 0]) , 0, VOIDmode, 0);
	op1 = expand_expr (((exp)->exp.operands[ 1]) , 0, VOIDmode, 0);
	if (! target)
	  target = gen_reg_rtx (((((exp)->common.type) )->type.mode) );
	prev = get_last_insn ();
	if (	((target)->code)  == REG)
	  emit_insn (gen_rtx (CLOBBER, VOIDmode, target));
	emit_move_insn (gen_realpart (mode, target), op0);
	emit_move_insn (gen_imagpart (mode, target), op1);
	group_insns (prev);
	return target;
      }
    case REALPART_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , 0, VOIDmode, 0);
      return gen_realpart (mode, op0);
    case IMAGPART_EXPR:
      op0 = expand_expr (((exp)->exp.operands[ 0]) , 0, VOIDmode, 0);
      return gen_imagpart (mode, op0);
    case CONJ_EXPR:
      {
	enum machine_mode mode = ((((((exp)->common.type) )->common.type) )->type.mode) ;
	rtx imag_t;
	rtx prev;
	op0  = expand_expr (((exp)->exp.operands[ 0]) , 0, VOIDmode, 0);
	if (! target)
	  target = gen_reg_rtx (((((exp)->common.type) )->type.mode) );
	prev = get_last_insn ();
	if (	((target)->code)  == REG)
	  emit_insn (gen_rtx (CLOBBER, VOIDmode, target));
	emit_move_insn (gen_realpart (mode, target), gen_realpart (mode, op0));
	imag_t = gen_imagpart (mode, target);
	temp   = expand_unop (mode, neg_optab,
			      gen_imagpart (mode, op0), imag_t, 0);
	if (temp != imag_t)
	  emit_move_insn (imag_t, temp);
	group_insns (prev);
	return target;
      }
    case ERROR_MARK:
      return const0_rtx;
    default:
      return (*lang_expand_expr) (exp, target, tmode, modifier);
    }
 binop:
  preexpand_calls (exp);
  if (! safe_from_p (subtarget, ((exp)->exp.operands[ 1]) ))
    subtarget = 0;
  op0 = expand_expr (((exp)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
  op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
 binop2:
  temp = expand_binop (mode, this_optab, op0, op1, target,
		       unsignedp, OPTAB_LIB_WIDEN);
  if (temp == 0)
    abort ();
  return temp;
}
static int
get_pointer_alignment (exp, max_align)
     tree exp;
     unsigned max_align;
{
  unsigned align, inner;
  if (((enum tree_code) (((exp)->common.type) )->common.code)  != POINTER_TYPE)
    return 0;
  align = ((((((exp)->common.type) )->common.type) )->type.align) ;
  align = ((align) < ( max_align) ? (align) : ( max_align)) ;
  while (1)
    {
      switch (((enum tree_code) (exp)->common.code) )
	{
	case NOP_EXPR:
	case CONVERT_EXPR:
	case NON_LVALUE_EXPR:
	  exp = ((exp)->exp.operands[ 0]) ;
	  if (((enum tree_code) (((exp)->common.type) )->common.code)  != POINTER_TYPE)
	    return align;
	  inner = ((((((exp)->common.type) )->common.type) )->type.align) ;
	  inner = ((inner) < ( max_align) ? (inner) : ( max_align)) ;
	  align = ((align) > ( inner) ? (align) : ( inner)) ;
	  break;
	case PLUS_EXPR:
	  if (((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  != INTEGER_CST)
	    return align;
	  while (((((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low)  * 8 )
		  & (max_align - 1))
		 != 0)
	    max_align >>= 1;
	  exp = ((exp)->exp.operands[ 0]) ;
	  break;
	case ADDR_EXPR:
	  exp = ((exp)->exp.operands[ 0]) ;
	  if (((enum tree_code) (exp)->common.code)  == FUNCTION_DECL)
	    align = ((align) > ( 32 ) ? (align) : ( 32 )) ;
	  else if ((*tree_code_type[(int) (((enum tree_code) (exp)->common.code) )])  == 'd')
	    align = ((align) > ( ((exp)->decl.frame_size) ) ? (align) : ( ((exp)->decl.frame_size) )) ;
	  else if ((*tree_code_type[(int) (((enum tree_code) (exp)->common.code) )])  == 'c')
	    align = ((((enum tree_code) (exp)->common.code)  == STRING_CST	&& ( align) < 64 )	? 64  : ( align)) ;
	  return ((align) < ( max_align) ? (align) : ( max_align)) ;
	default:
	  return align;
	}
    }
}
static tree
string_constant (arg, ptr_offset)
     tree arg;
     tree *ptr_offset;
{
  while ((((enum tree_code) (arg)->common.code)  == NOP_EXPR	|| ((enum tree_code) (arg)->common.code)  == CONVERT_EXPR	|| ((enum tree_code) (arg)->common.code)  == NON_LVALUE_EXPR)	&& (((((arg)->common.type) )->type.mode) 	== ((((((arg)->exp.operands[ 0]) )->common.type) )->type.mode) ))	(arg) = ((arg)->exp.operands[ 0]) ; ;
  if (((enum tree_code) (arg)->common.code)  == ADDR_EXPR
      && ((enum tree_code) (((arg)->exp.operands[ 0]) )->common.code)  == STRING_CST)
    {
      *ptr_offset = integer_zero_node;
      return ((arg)->exp.operands[ 0]) ;
    }
  else if (((enum tree_code) (arg)->common.code)  == PLUS_EXPR)
    {
      tree arg0 = ((arg)->exp.operands[ 0]) ;
      tree arg1 = ((arg)->exp.operands[ 1]) ;
      while ((((enum tree_code) (arg0)->common.code)  == NOP_EXPR	|| ((enum tree_code) (arg0)->common.code)  == CONVERT_EXPR	|| ((enum tree_code) (arg0)->common.code)  == NON_LVALUE_EXPR)	&& (((((arg0)->common.type) )->type.mode) 	== ((((((arg0)->exp.operands[ 0]) )->common.type) )->type.mode) ))	(arg0) = ((arg0)->exp.operands[ 0]) ; ;
      while ((((enum tree_code) (arg1)->common.code)  == NOP_EXPR	|| ((enum tree_code) (arg1)->common.code)  == CONVERT_EXPR	|| ((enum tree_code) (arg1)->common.code)  == NON_LVALUE_EXPR)	&& (((((arg1)->common.type) )->type.mode) 	== ((((((arg1)->exp.operands[ 0]) )->common.type) )->type.mode) ))	(arg1) = ((arg1)->exp.operands[ 0]) ; ;
      if (((enum tree_code) (arg0)->common.code)  == ADDR_EXPR
	  && ((enum tree_code) (((arg0)->exp.operands[ 0]) )->common.code)  == STRING_CST)
	{
	  *ptr_offset = arg1;
	  return ((arg0)->exp.operands[ 0]) ;
	}
      else if (((enum tree_code) (arg1)->common.code)  == ADDR_EXPR
	       && ((enum tree_code) (((arg1)->exp.operands[ 0]) )->common.code)  == STRING_CST)
	{
	  *ptr_offset = arg0;
	  return ((arg1)->exp.operands[ 0]) ;
	}
    }
  return 0;
}
static tree
c_strlen (src)
     tree src;
{
  tree offset_node;
  int offset, max;
  char *ptr;
  src = string_constant (src, &offset_node);
  if (src == 0)
    return 0;
  max = ((src)->string.length) ;
  ptr = ((src)->string.pointer) ;
  if (offset_node && ((enum tree_code) (offset_node)->common.code)  != INTEGER_CST)
    {
      int i;
      for (i = 0; i < max; i++)
	if (ptr[i] == 0)
	  return 0;
      return size_binop (MINUS_EXPR, size_int (max), offset_node);
    }
  if (offset_node == 0)
    offset = 0;
  else
    {
      if (((offset_node)->int_cst.int_cst_high)  != 0)
	return 0;
      offset = ((offset_node)->int_cst.int_cst_low) ;
    }
  if (offset < 0 || offset > max)
    {
      warning ("offset outside bounds of constant string");
      return 0;
    }
  return size_int (strlen (ptr + offset));
}
static rtx
expand_builtin (exp, target, subtarget, mode, ignore)
     tree exp;
     rtx target;
     rtx subtarget;
     enum machine_mode mode;
     int ignore;
{
  tree fndecl = ((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) ;
  tree arglist = ((exp)->exp.operands[ 1]) ;
  rtx op0;
  rtx lab1, insns;
  enum machine_mode value_mode = ((((exp)->common.type) )->type.mode) ;
  optab builtin_optab;
  switch (((enum built_in_function) (fndecl)->decl.frame_size) )
    {
    case BUILT_IN_ABS:
    case BUILT_IN_LABS:
    case BUILT_IN_FABS:
      abort ();
    case BUILT_IN_SIN:
    case BUILT_IN_COS:
    case BUILT_IN_FSQRT:
      if (! optimize)
	break;
      if (arglist == 0
	  || ((enum tree_code) (((((arglist)->list.value) )->common.type) )->common.code)  != REAL_TYPE)
	return (const_tiny_rtx[0][(int) (((((exp)->common.type) )->type.mode) )]) ;
      if (((enum tree_code) (((arglist)->list.value) )->common.code)  != VAR_DECL
	  && ((enum tree_code) (((arglist)->list.value) )->common.code)  != PARM_DECL)
	{
	  exp = copy_node (exp);
	  arglist = copy_node (arglist);
	  ((exp)->exp.operands[ 1])  = arglist;
	  ((arglist)->list.value)  = save_expr (((arglist)->list.value) );
	}
      op0 = expand_expr (((arglist)->list.value) , subtarget, VOIDmode, 0);
      target = gen_reg_rtx (((((exp)->common.type) )->type.mode) );
      emit_queue ();
      start_sequence ();
      switch (((enum built_in_function) (fndecl)->decl.frame_size) )
	{
	case BUILT_IN_SIN:
	  builtin_optab = sin_optab; break;
	case BUILT_IN_COS:
	  builtin_optab = cos_optab; break;
	case BUILT_IN_FSQRT:
	  builtin_optab = sqrt_optab; break;
	default:
	  abort ();
	}
      target = expand_unop (((((((arglist)->list.value) )->common.type) )->type.mode) ,
			    builtin_optab, op0, target, 0);
      if (target == 0)
	{
	  end_sequence ();
	  break;
        }
      if (! flag_fast_math)
	{
	  if (1   != 1 )
	    abort ();
	  lab1 = gen_label_rtx ();
	  emit_cmp_insn (target, target, EQ, 0, 	((target)->mode) , 0, 0);
	  emit_jump_insn (gen_beq (lab1));
	  (inhibit_defer_pop += 1) ;
	  expand_call (exp, target, 0);
	  (inhibit_defer_pop -= 1) ;
	  emit_label (lab1);
	}
      insns = get_insns ();
      end_sequence ();
      emit_insns (insns);
      return target;
    case BUILT_IN_SAVEREGS:
      if (saveregs_value != 0)
	return saveregs_value;
      {
	rtx temp;
	rtx seq;
	rtx valreg, saved_valreg;
	start_sequence ();
	temp = (emit_insn (gen_rtx (USE, VOIDmode, gen_rtx (REG, TImode, 24))),	emit_insn (gen_rtx (USE, VOIDmode, gen_rtx (REG, DImode, 28))),	expand_call (exp, target, ignore)) ;
	seq = get_insns ();
	end_sequence ();
	saveregs_value = temp;
	if (in_sequence_p ())
	  {
	    error ("`va_start' used within `({...})'");
	    return temp;
	  }
	emit_insns_before (seq, ((get_insns ())->fld[2].rtx) );
	return temp;
      }
    case BUILT_IN_ARGS_INFO:
      {
	int nwords = sizeof (int ) / sizeof (int);
	int i;
	int *word_ptr = (int *) &current_function_args_info;
	tree type, elts, result;
	if (sizeof (int ) % sizeof (int) != 0)
	  fatal ("CUMULATIVE_ARGS type defined badly; see %s, line %d",
		 "expr.c", 5625);
	if (arglist != 0)
	  {
	    tree arg = ((arglist)->list.value) ;
	    if (((enum tree_code) (arg)->common.code)  != INTEGER_CST)
	      error ("argument of __builtin_args_info must be constant");
	    else
	      {
		int wordnum = ((arg)->int_cst.int_cst_low) ;
		if (wordnum < 0 || wordnum >= nwords)
		  error ("argument of __builtin_args_info out of range");
		else
		  return gen_rtx (CONST_INT, VOIDmode, (word_ptr[wordnum])) ;
	      }
	  }
	else
	  error ("missing argument in __builtin_args_info");
	return const0_rtx;
      }
    case BUILT_IN_NEXT_ARG:
      {
	tree fntype = ((current_function_decl)->common.type) ;
	if (!(((fntype)->type.values)  != 0
	      && (((tree_last (((fntype)->type.values) ))->list.value) 
		  != void_type_node)))
	  {
	    error ("`va_start' used in function with fixed args");
	    return const0_rtx;
	  }
      }
      return expand_binop (SImode , add_optab,
			   current_function_internal_arg_pointer,
			   current_function_arg_offset_rtx,
			   (rtx) 0 , 0, OPTAB_LIB_WIDEN);
    case BUILT_IN_CLASSIFY_TYPE:
      if (arglist != 0)
	{
	  tree type = ((((arglist)->list.value) )->common.type) ;
	  enum tree_code code = ((enum tree_code) (type)->common.code) ;
	  if (code == VOID_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (void_type_class)) ;
	  if (code == INTEGER_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (integer_type_class)) ;
	  if (code == CHAR_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (char_type_class)) ;
	  if (code == ENUMERAL_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (enumeral_type_class)) ;
	  if (code == BOOLEAN_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (boolean_type_class)) ;
	  if (code == POINTER_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (pointer_type_class)) ;
	  if (code == REFERENCE_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (reference_type_class)) ;
	  if (code == OFFSET_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (offset_type_class)) ;
	  if (code == REAL_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (real_type_class)) ;
	  if (code == COMPLEX_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (complex_type_class)) ;
	  if (code == FUNCTION_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (function_type_class)) ;
	  if (code == METHOD_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (method_type_class)) ;
	  if (code == RECORD_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (record_type_class)) ;
	  if (code == UNION_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (union_type_class)) ;
	  if (code == ARRAY_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (array_type_class)) ;
	  if (code == STRING_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (string_type_class)) ;
	  if (code == SET_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (set_type_class)) ;
	  if (code == FILE_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (file_type_class)) ;
	  if (code == LANG_TYPE)
	    return gen_rtx (CONST_INT, VOIDmode, (lang_type_class)) ;
	}
      return gen_rtx (CONST_INT, VOIDmode, (no_type_class)) ;
    case BUILT_IN_CONSTANT_P:
      if (arglist == 0)
	return const0_rtx;
      else
	return ((*tree_code_type[(int) (((enum tree_code) (((arglist)->list.value) )->common.code) )])  == 'c'
		? const1_rtx : const0_rtx);
    case BUILT_IN_FRAME_ADDRESS:
    case BUILT_IN_RETURN_ADDRESS:
      if (arglist == 0)
	return const0_rtx;
      else if (((enum tree_code) (((arglist)->list.value) )->common.code)  != INTEGER_CST)
	{
	  error ("invalid arg to __builtin_return_address");
	  return const0_rtx;
	}
      else if (tree_int_cst_lt (((arglist)->list.value) , integer_zero_node))
	{
	  error ("invalid arg to __builtin_return_address");
	  return const0_rtx;
	}
      else
	{
	  int count = ((((arglist)->list.value) )->int_cst.int_cst_low) ; 
	  rtx tem = frame_pointer_rtx;
	  int i;
	  for (i = 0; i < count; i++)
	    {
	      tem = memory_address (SImode , tem);
	      tem = copy_to_reg (gen_rtx (MEM, SImode , tem));
	    }
	  if (((enum built_in_function) (fndecl)->decl.frame_size)  == BUILT_IN_FRAME_ADDRESS)
	    return tem;
	  tem = memory_address (SImode ,
				plus_constant_wide (tem, (int ) ( 	(mode_size[(int)(SImode )]) )) );
	  return copy_to_reg (gen_rtx (MEM, SImode , tem));
	}
    case BUILT_IN_ALLOCA:
      if (arglist == 0
	  || ((enum tree_code) (((((arglist)->list.value) )->common.type) )->common.code)  != INTEGER_TYPE)
	return const0_rtx;
      current_function_calls_alloca = 1;
      op0 = expand_expr (((arglist)->list.value) , (rtx) 0 , VOIDmode, 0);
      target = allocate_dynamic_stack_space (op0, target, 8 );
      if (nonlocal_goto_handler_slot != 0)
	emit_stack_save (SAVE_NONLOCAL, &nonlocal_goto_stack_level, (rtx) 0 );
      return target;
    case BUILT_IN_FFS:
      if (!optimize)
	break;
      if (arglist == 0
	  || ((enum tree_code) (((((arglist)->list.value) )->common.type) )->common.code)  != INTEGER_TYPE)
	return const0_rtx;
      op0 = expand_expr (((arglist)->list.value) , subtarget, VOIDmode, 0);
      target = expand_unop (((((((arglist)->list.value) )->common.type) )->type.mode) ,
			    ffs_optab, op0, target, 1);
      if (target == 0)
	abort ();
      return target;
    case BUILT_IN_STRLEN:
      if (!optimize)
	break;
      if (arglist == 0
	  || ((enum tree_code) (((((arglist)->list.value) )->common.type) )->common.code)  != POINTER_TYPE)
	return const0_rtx;
      else
	{
	  tree src = ((arglist)->list.value) ;
	  tree len = c_strlen (src);
	  int align
	    = get_pointer_alignment (src, 64 ) / 8 ;
	  rtx result, src_rtx, char_rtx;
	  enum machine_mode insn_mode = value_mode, char_mode;
	  enum insn_code icode;
	  if (len != 0)
	    return expand_expr (len, target, mode, 0);
	  if (align == 0)
	    break;
	  while (insn_mode != VOIDmode)
	    {
	      icode = strlen_optab->handlers[(int) insn_mode].insn_code;
	      if (icode != CODE_FOR_nothing)
		break;
	      insn_mode = (mode_wider_mode[(int)(insn_mode)]) ;
	    }
	  if (insn_mode == VOIDmode)
	    break;
	  result = target;
	  if (! (result != 0
		 && 	((result)->code)  == REG
		 && 	((result)->mode)  == insn_mode
		 && ((result)->fld[0].rtint)  >= 64 ))
	    result = gen_reg_rtx (insn_mode);
	  if (! (*insn_operand_predicate[(int)icode][0]) (result, insn_mode))
	    result = gen_reg_rtx (insn_mode);
	  src_rtx = memory_address (BLKmode,
				    expand_expr (src, (rtx) 0 , SImode ,
						 EXPAND_NORMAL));
	  if (! (*insn_operand_predicate[(int)icode][1]) (src_rtx, SImode ))
	    src_rtx = copy_to_mode_reg (SImode , src_rtx);
	  char_rtx = const0_rtx;
	  char_mode = insn_operand_mode[(int)icode][2];
	  if (! (*insn_operand_predicate[(int)icode][2]) (char_rtx, char_mode))
	    char_rtx = copy_to_mode_reg (char_mode, char_rtx);
	  emit_insn ((*insn_gen_function[(int) (icode)])  (result,
				      gen_rtx (MEM, BLKmode, src_rtx),
				      char_rtx, gen_rtx (CONST_INT, VOIDmode, (align)) ));
	  if (	((result)->mode)  == value_mode)
	    return result;
	  else if (target != 0)
	    {
	      convert_move (target, result, 0);
	      return target;
	    }
	  else
	    return convert_to_mode (value_mode, result, 0);
	}
    case BUILT_IN_STRCPY:
      if (!optimize)
	break;
      if (arglist == 0
	  || ((enum tree_code) (((((arglist)->list.value) )->common.type) )->common.code)  != POINTER_TYPE
	  || ((arglist)->common.chain)  == 0
	  || ((enum tree_code) (((((((arglist)->common.chain) )->list.value) )->common.type) )->common.code)  != POINTER_TYPE)
	return const0_rtx;
      else
	{
	  tree len = c_strlen (((((arglist)->common.chain) )->list.value) );
	  if (len == 0)
	    break;
	  len = size_binop (PLUS_EXPR, len, integer_one_node);
	  chainon (arglist, build_tree_list ((tree) 0  , len));
	}
    case BUILT_IN_MEMCPY:
      if (!optimize)
	break;
      if (arglist == 0
	  || ((enum tree_code) (((((arglist)->list.value) )->common.type) )->common.code)  != POINTER_TYPE
	  || ((arglist)->common.chain)  == 0
	  || ((enum tree_code) (((((((arglist)->common.chain) )->list.value) )->common.type) )->common.code)  != POINTER_TYPE
	  || ((((arglist)->common.chain) )->common.chain)  == 0
	  || ((enum tree_code) (((((((((arglist)->common.chain) )->common.chain) )->list.value) )->common.type) )->common.code)  != INTEGER_TYPE)
	return const0_rtx;
      else
	{
	  tree dest = ((arglist)->list.value) ;
	  tree src = ((((arglist)->common.chain) )->list.value) ;
	  tree len = ((((((arglist)->common.chain) )->common.chain) )->list.value) ;
	  int src_align
	    = get_pointer_alignment (src, 64 ) / 8 ;
	  int dest_align
	    = get_pointer_alignment (dest, 64 ) / 8 ;
	  rtx dest_rtx;
	  if (src_align == 0 || dest_align == 0)
	    {
	      if (((enum built_in_function) (fndecl)->decl.frame_size)  == BUILT_IN_STRCPY)
		((((arglist)->common.chain) )->common.chain)  = 0;
	      break;
	    }
	  dest_rtx = expand_expr (dest, (rtx) 0 , SImode , EXPAND_NORMAL);
	  emit_block_move (gen_rtx (MEM, BLKmode,
				    memory_address (BLKmode, dest_rtx)),
			   gen_rtx (MEM, BLKmode,
				    memory_address (BLKmode,
						    expand_expr (src, (rtx) 0 ,
								 SImode ,
								 EXPAND_NORMAL))),
			   expand_expr (len, (rtx) 0 , VOIDmode, 0),
			   ((src_align) < ( dest_align) ? (src_align) : ( dest_align)) );
	  return dest_rtx;
	}
    case BUILT_IN_STRCMP:
    case BUILT_IN_MEMCMP:
      break;
    default:			 
      error ("built-in function %s not currently supported",
	     ((((fndecl)->decl.name) )->identifier.pointer) );
    }
  return expand_call (exp, target, ignore);
}
static rtx
expand_increment (exp, post)
     register tree exp;
     int post;
{
  register rtx op0, op1;
  register rtx temp, value;
  register tree incremented = ((exp)->exp.operands[ 0]) ;
  optab this_optab = add_optab;
  int icode;
  enum machine_mode mode = ((((exp)->common.type) )->type.mode) ;
  int op0_is_copy = 0;
  if (((enum tree_code) (incremented)->common.code)  == BIT_FIELD_REF
      || (((enum tree_code) (incremented)->common.code)  == COMPONENT_REF
	  && (((enum tree_code) (((incremented)->exp.operands[ 0]) )->common.code)  != INDIRECT_REF
	      || ((((incremented)->exp.operands[ 1]) )->decl.bit_field_flag) )))
    incremented = stabilize_reference (incremented);
  temp = get_last_insn ();
  op0 = expand_expr (incremented, (rtx) 0 , VOIDmode, 0);
  if (	((op0)->code)  == SUBREG && ((op0)->in_struct) )
    ((op0)->fld[0].rtx)  = copy_to_reg (((op0)->fld[0].rtx) );
  op0_is_copy = ((	((op0)->code)  == SUBREG || 	((op0)->code)  == REG)
		 && temp != get_last_insn ());
  op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
  if (((enum tree_code) (exp)->common.code)  == POSTDECREMENT_EXPR
      || ((enum tree_code) (exp)->common.code)  == PREDECREMENT_EXPR)
    this_optab = sub_optab;
  if (op0_is_copy)
    {
      tree newexp = build ((this_optab == add_optab
			    ? PLUS_EXPR : MINUS_EXPR),
			   ((exp)->common.type) ,
			   incremented,
			   ((exp)->exp.operands[ 1]) );
      temp = expand_assignment (incremented, newexp, ! post, 0);
      return post ? op0 : temp;
    }
  if (this_optab == sub_optab
      && 	((op1)->code)  == CONST_INT)
    {
      op1 = gen_rtx (CONST_INT, VOIDmode, (- ((op1)->fld[0].rtwint) )) ;
      this_optab = add_optab;
    }
  if (post)
    {
      icode = (int) this_optab->handlers[(int) mode].insn_code;
      if (icode != (int) CODE_FOR_nothing
	  && (*insn_operand_predicate[icode][0]) (op0, mode)
	  && (*insn_operand_predicate[icode][1]) (op0, mode))
	{
	  if (! (*insn_operand_predicate[icode][2]) (op1, mode))
	    op1 = force_reg (mode, op1);
	  return enqueue_insn (op0, (*insn_gen_function[(int) (icode)])  (op0, op0, op1));
	}
    }
  if (post)
    temp = value = copy_to_reg (op0);
  else
    temp = copy_rtx (value = op0);
  op1 = expand_binop (mode, this_optab, value, op1, op0,
		      ((((exp)->common.type) )->common.unsigned_flag) , OPTAB_LIB_WIDEN);
  if (op1 != op0)
    emit_move_insn (op0, op1);
  return temp;
}
static void
preexpand_calls (exp)
     tree exp;
{
  register int nops, i;
  int type = (*tree_code_type[(int) (((enum tree_code) (exp)->common.code) )]) ;
  if (! do_preexpand_calls)
    return;
  if (type != 'e' && type != '<' && type != '1' && type != '2' && type != 'r')
    return;
  switch (((enum tree_code) (exp)->common.code) )
    {
    case CALL_EXPR:
      if ((*(struct rtx_def **) &(exp)->exp.operands[2])  != 0)
	return;
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  != ADDR_EXPR
	  || ((enum tree_code) (((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->common.code)  != FUNCTION_DECL
	  || ! ((((((exp)->exp.operands[ 0]) )->exp.operands[ 0]) )->decl.bit_field_flag) )
	(*(struct rtx_def **) &(exp)->exp.operands[2])  = expand_call (exp, (rtx) 0 , 0);
      return;
    case COMPOUND_EXPR:
    case COND_EXPR:
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
      do_pending_stack_adjust ();
      return;
    case BLOCK:
    case RTL_EXPR:
    case WITH_CLEANUP_EXPR:
      return;
    case SAVE_EXPR:
      if ((*(struct rtx_def **) &(exp)->exp.operands[2])  != 0)
	return;
    }
  nops = tree_code_length[(int) ((enum tree_code) (exp)->common.code) ];
  for (i = 0; i < nops; i++)
    if (((exp)->exp.operands[ i])  != 0)
      {
	type = (*tree_code_type[(int) (((enum tree_code) (((exp)->exp.operands[ i]) )->common.code) )]) ;
	if (type == 'e' || type == '<' || type == '1' || type == '2'
	    || type == 'r')
	  preexpand_calls (((exp)->exp.operands[ i]) );
      }
}
void
init_pending_stack_adjust ()
{
  pending_stack_adjust = 0;
}
void
clear_pending_stack_adjust ()
{
  if (! flag_omit_frame_pointer && (get_frame_size () != 0	|| current_function_calls_alloca || current_function_outgoing_args_size) 
      && ! (((current_function_decl)->decl.inline_flag)  && ! flag_no_inline)
      && ! flag_inline_functions)
    pending_stack_adjust = 0;
}
void
do_pending_stack_adjust ()
{
  if (inhibit_defer_pop == 0)
    {
      if (pending_stack_adjust != 0)
	adjust_stack (gen_rtx (CONST_INT, VOIDmode, (pending_stack_adjust)) );
      pending_stack_adjust = 0;
    }
}
void
expand_cleanups_to (old_cleanups)
     tree old_cleanups;
{
  while (cleanups_this_call != old_cleanups)
    {
      expand_expr (((cleanups_this_call)->list.value) , (rtx) 0 , VOIDmode, 0);
      cleanups_this_call = ((cleanups_this_call)->common.chain) ;
    }
}
void
jumpifnot (exp, label)
     tree exp;
     rtx label;
{
  do_jump (exp, label, (rtx) 0 );
}
void
jumpif (exp, label)
     tree exp;
     rtx label;
{
  do_jump (exp, (rtx) 0 , label);
}
void
do_jump (exp, if_false_label, if_true_label)
     tree exp;
     rtx if_false_label, if_true_label;
{
  register enum tree_code code = ((enum tree_code) (exp)->common.code) ;
  rtx drop_through_label = 0;
  rtx temp;
  rtx comparison = 0;
  int i;
  tree type;
  emit_queue ();
  switch (code)
    {
    case ERROR_MARK:
      break;
    case INTEGER_CST:
      temp = integer_zerop (exp) ? if_false_label : if_true_label;
      if (temp)
	emit_jump (temp);
      break;
    case NOP_EXPR:
      if (((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == COMPONENT_REF
	  || ((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == BIT_FIELD_REF
	  || ((enum tree_code) (((exp)->exp.operands[ 0]) )->common.code)  == ARRAY_REF)
	goto normal;
    case CONVERT_EXPR:
      if ((((((exp)->common.type) )->type.precision) 
	   < ((((((exp)->exp.operands[ 0]) )->common.type) )->type.precision) ))
	goto normal;
    case NON_LVALUE_EXPR:
    case REFERENCE_EXPR:
    case ABS_EXPR:
    case NEGATE_EXPR:
    case LROTATE_EXPR:
    case RROTATE_EXPR:
      do_jump (((exp)->exp.operands[ 0]) , if_false_label, if_true_label);
      break;
    case MINUS_EXPR:
      comparison = compare (build (NE_EXPR, ((exp)->common.type) ,
				   ((exp)->exp.operands[ 0]) ,
				   ((exp)->exp.operands[ 1]) ),
			    NE, NE);
      break;
    case BIT_AND_EXPR:
      if (! 1 
	  && ((enum tree_code) (((exp)->exp.operands[ 1]) )->common.code)  == INTEGER_CST
	  && ((((exp)->common.type) )->type.precision)  <= 32  
	  && (i = floor_log2_wide ((int ) (((((exp)->exp.operands[ 1]) )->int_cst.int_cst_low) )) ) >= 0
	  && (type = type_for_size (i + 1, 1)) != 0
	  && ((type)->type.precision)  < ((((exp)->common.type) )->type.precision) 
	  && (cmp_optab->handlers[(int) ((type)->type.mode) ].insn_code
	      != CODE_FOR_nothing))
	{
	  do_jump (convert (type, exp), if_false_label, if_true_label);
	  break;
	}
      goto normal;
    case TRUTH_NOT_EXPR:
      do_jump (((exp)->exp.operands[ 0]) , if_true_label, if_false_label);
      break;
    case TRUTH_ANDIF_EXPR:
      if (if_false_label == 0)
	if_false_label = drop_through_label = gen_label_rtx ();
      do_jump (((exp)->exp.operands[ 0]) , if_false_label, (rtx) 0 );
      do_jump (((exp)->exp.operands[ 1]) , if_false_label, if_true_label);
      break;
    case TRUTH_ORIF_EXPR:
      if (if_true_label == 0)
	if_true_label = drop_through_label = gen_label_rtx ();
      do_jump (((exp)->exp.operands[ 0]) , (rtx) 0 , if_true_label);
      do_jump (((exp)->exp.operands[ 1]) , if_false_label, if_true_label);
      break;
    case COMPOUND_EXPR:
      expand_expr (((exp)->exp.operands[ 0]) , const0_rtx, VOIDmode, 0);
      free_temp_slots ();
      emit_queue ();
      do_pending_stack_adjust ();
      do_jump (((exp)->exp.operands[ 1]) , if_false_label, if_true_label);
      break;
    case COMPONENT_REF:
    case BIT_FIELD_REF:
    case ARRAY_REF:
      {
	int bitsize, bitpos, unsignedp;
	enum machine_mode mode;
	tree type;
	tree offset;
	int volatilep = 0;
	get_inner_reference (exp, &bitsize, &bitpos, &offset,
			     &mode, &unsignedp, &volatilep);
	type = type_for_size (bitsize, unsignedp);
	if (! 1 
	    && type != 0 && bitsize >= 0
	    && ((type)->type.precision)  < ((((exp)->common.type) )->type.precision) 
	    && (cmp_optab->handlers[(int) ((type)->type.mode) ].insn_code
		!= CODE_FOR_nothing))
	  {
	    do_jump (convert (type, exp), if_false_label, if_true_label);
	    break;
	  }
	goto normal;
      }
    case COND_EXPR:
      if (integer_onep (((exp)->exp.operands[ 1]) )
	  && integer_zerop (((exp)->exp.operands[ 2]) ))
	do_jump (((exp)->exp.operands[ 0]) , if_false_label, if_true_label);
      else if (integer_zerop (((exp)->exp.operands[ 1]) )
	       && integer_onep (((exp)->exp.operands[ 2]) ))
	do_jump (((exp)->exp.operands[ 0]) , if_true_label, if_false_label);
      else
	{
	  register rtx label1 = gen_label_rtx ();
	  drop_through_label = gen_label_rtx ();
	  do_jump (((exp)->exp.operands[ 0]) , label1, (rtx) 0 );
	  do_jump (((exp)->exp.operands[ 1]) ,
		   if_false_label ? if_false_label : drop_through_label,
		   if_true_label ? if_true_label : drop_through_label);
	  do_pending_stack_adjust ();
	  emit_label (label1);
	  do_jump (((exp)->exp.operands[ 2]) ,
		   if_false_label ? if_false_label : drop_through_label,
		   if_true_label ? if_true_label : drop_through_label);
	}
      break;
    case EQ_EXPR:
      if (integer_zerop (((exp)->exp.operands[ 1]) ))
	do_jump (((exp)->exp.operands[ 0]) , if_true_label, if_false_label);
      else if ((	(mode_class[(int)(((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )]) 
		== MODE_INT)
	       && 
	       !can_compare_p (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	do_jump_by_parts_equality (exp, if_false_label, if_true_label);
      else
	comparison = compare (exp, EQ, EQ);
      break;
    case NE_EXPR:
      if (integer_zerop (((exp)->exp.operands[ 1]) ))
	do_jump (((exp)->exp.operands[ 0]) , if_false_label, if_true_label);
      else if ((	(mode_class[(int)(((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )]) 
		== MODE_INT)
	       && 
	       !can_compare_p (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	do_jump_by_parts_equality (exp, if_true_label, if_false_label);
      else
	comparison = compare (exp, NE, NE);
      break;
    case LT_EXPR:
      if ((	(mode_class[(int)(((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )]) 
	   == MODE_INT)
	  && !can_compare_p (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	do_jump_by_parts_greater (exp, 1, if_false_label, if_true_label);
      else
	comparison = compare (exp, LT, LTU);
      break;
    case LE_EXPR:
      if ((	(mode_class[(int)(((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )]) 
	   == MODE_INT)
	  && !can_compare_p (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	do_jump_by_parts_greater (exp, 0, if_true_label, if_false_label);
      else
	comparison = compare (exp, LE, LEU);
      break;
    case GT_EXPR:
      if ((	(mode_class[(int)(((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )]) 
	   == MODE_INT)
	  && !can_compare_p (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	do_jump_by_parts_greater (exp, 0, if_false_label, if_true_label);
      else
	comparison = compare (exp, GT, GTU);
      break;
    case GE_EXPR:
      if ((	(mode_class[(int)(((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) )]) 
	   == MODE_INT)
	  && !can_compare_p (((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ))
	do_jump_by_parts_greater (exp, 1, if_true_label, if_false_label);
      else
	comparison = compare (exp, GE, GEU);
      break;
    default:
    normal:
      temp = expand_expr (exp, (rtx) 0 , VOIDmode, 0);
      do_pending_stack_adjust ();
      if (	((temp)->code)  == CONST_INT)
	comparison = (temp == const0_rtx ? const0_rtx : const_true_rtx);
      else if (	((temp)->code)  == LABEL_REF)
	comparison = const_true_rtx;
      else if (	(mode_class[(int)(	((temp)->mode) )])  == MODE_INT
	       && !can_compare_p (	((temp)->mode) ))
	do_jump_by_parts_equality_rtx (temp, if_true_label, if_false_label);
      else if (	((temp)->mode)  != VOIDmode)
	comparison = compare_from_rtx (temp, (const_tiny_rtx[0][(int) (	((temp)->mode) )]) ,
				       NE, ((((exp)->common.type) )->common.unsigned_flag) ,
				       	((temp)->mode) , (rtx) 0 , 0);
      else
	abort ();
    }
  emit_queue ();
  if (comparison == const_true_rtx)
    {
      if (if_true_label)
	emit_jump (if_true_label);
    }
  else if (comparison == const0_rtx)
    {
      if (if_false_label)
	emit_jump (if_false_label);
    }
  else if (comparison)
    do_jump_for_compare (comparison, if_false_label, if_true_label);
  free_temp_slots ();
  if (drop_through_label)
    {
      do_pending_stack_adjust ();
      emit_label (drop_through_label);
    }
}
static void
do_jump_by_parts_greater (exp, swap, if_false_label, if_true_label)
     tree exp;
     int swap;
     rtx if_false_label, if_true_label;
{
  rtx op0 = expand_expr (((exp)->exp.operands[ swap]) , (rtx) 0 , VOIDmode, 0);
  rtx op1 = expand_expr (((exp)->exp.operands[ !swap]) , (rtx) 0 , VOIDmode, 0);
  enum machine_mode mode = ((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ;
  int nwords = (	(mode_size[(int)(mode)])  / 4 );
  rtx drop_through_label = 0;
  int unsignedp = ((((((exp)->exp.operands[ 0]) )->common.type) )->common.unsigned_flag) ;
  int i;
  if (! if_true_label || ! if_false_label)
    drop_through_label = gen_label_rtx ();
  if (! if_true_label)
    if_true_label = drop_through_label;
  if (! if_false_label)
    if_false_label = drop_through_label;
  for (i = 0; i < nwords; i++)
    {
      rtx comp;
      rtx op0_word, op1_word;
      if (1 )
	{
	  op0_word = operand_subword_force (op0, i, mode);
	  op1_word = operand_subword_force (op1, i, mode);
	}
      else
	{
	  op0_word = operand_subword_force (op0, nwords - 1 - i, mode);
	  op1_word = operand_subword_force (op1, nwords - 1 - i, mode);
	}
      comp = compare_from_rtx (op0_word, op1_word,
			       (unsignedp || i > 0) ? GTU : GT,
			       unsignedp, word_mode, (rtx) 0 , 0);
      if (comp == const_true_rtx)
	emit_jump (if_true_label);
      else if (comp != const0_rtx)
	do_jump_for_compare (comp, (rtx) 0 , if_true_label);
      comp = compare_from_rtx (op0_word, op1_word, NE, unsignedp, word_mode,
			       (rtx) 0 , 0);
      if (comp == const_true_rtx)
	emit_jump (if_false_label);
      else if (comp != const0_rtx)
	do_jump_for_compare (comp, (rtx) 0 , if_false_label);
    }
  if (if_false_label)
    emit_jump (if_false_label);
  if (drop_through_label)
    emit_label (drop_through_label);
}
static void
do_jump_by_parts_equality (exp, if_false_label, if_true_label)
     tree exp;
     rtx if_false_label, if_true_label;
{
  rtx op0 = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 , VOIDmode, 0);
  rtx op1 = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
  enum machine_mode mode = ((((((exp)->exp.operands[ 0]) )->common.type) )->type.mode) ;
  int nwords = (	(mode_size[(int)(mode)])  / 4 );
  int i;
  rtx drop_through_label = 0;
  if (! if_false_label)
    drop_through_label = if_false_label = gen_label_rtx ();
  for (i = 0; i < nwords; i++)
    {
      rtx comp = compare_from_rtx (operand_subword_force (op0, i, mode),
				   operand_subword_force (op1, i, mode),
				   EQ, ((((exp)->common.type) )->common.unsigned_flag) ,
				   word_mode, (rtx) 0 , 0);
      if (comp == const_true_rtx)
	emit_jump (if_false_label);
      else if (comp != const0_rtx)
	do_jump_for_compare (comp, if_false_label, (rtx) 0 );
    }
  if (if_true_label)
    emit_jump (if_true_label);
  if (drop_through_label)
    emit_label (drop_through_label);
}
static void
do_jump_by_parts_equality_rtx (op0, if_false_label, if_true_label)
     rtx op0;
     rtx if_false_label, if_true_label;
{
  int nwords = 	(mode_size[(int)(	((op0)->mode) )])  / 4 ;
  int i;
  rtx drop_through_label = 0;
  if (! if_false_label)
    drop_through_label = if_false_label = gen_label_rtx ();
  for (i = 0; i < nwords; i++)
    {
      rtx comp = compare_from_rtx (operand_subword_force (op0, i,
							  	((op0)->mode) ),
				   const0_rtx, EQ, 1, word_mode, (rtx) 0 , 0);
      if (comp == const_true_rtx)
	emit_jump (if_false_label);
      else if (comp != const0_rtx)
	do_jump_for_compare (comp, if_false_label, (rtx) 0 );
    }
  if (if_true_label)
    emit_jump (if_true_label);
  if (drop_through_label)
    emit_label (drop_through_label);
}
static void
do_jump_for_compare (comparison, if_false_label, if_true_label)
     rtx comparison, if_false_label, if_true_label;
{
  if (if_true_label)
    {
      if (bcc_gen_fctn[(int) 	((comparison)->code) ] != 0)
	emit_jump_insn ((*bcc_gen_fctn[(int) 	((comparison)->code) ]) (if_true_label));
      else
	abort ();
      if (if_false_label)
	emit_jump (if_false_label);
    }
  else if (if_false_label)
    {
      rtx insn;
      rtx prev = ((get_last_insn ())->fld[1].rtx) ;
      rtx branch = 0;
      if (bcc_gen_fctn[(int) 	((comparison)->code) ] != 0)
	emit_jump_insn ((*bcc_gen_fctn[(int) 	((comparison)->code) ]) (if_false_label));
      else
	abort ();
      if (prev == 0)
	insn = get_insns ();
      else
	insn = ((prev)->fld[2].rtx) ;
      for (insn = ((insn)->fld[2].rtx) ; insn; insn = ((insn)->fld[2].rtx) )
	if (	((insn)->code)  == JUMP_INSN)
	  {
	    if (branch)
	      abort ();
	    branch = insn;
	  }
      if (branch != get_last_insn ())
	abort ();
      if (! invert_jump (branch, if_false_label))
	{
	  if_true_label = gen_label_rtx ();
	  redirect_jump (branch, if_true_label);
	  emit_jump (if_false_label);
	  emit_label (if_true_label);
	}
    }
}
static rtx
compare (exp, signed_code, unsigned_code)
     register tree exp;
     enum rtx_code signed_code, unsigned_code;
{
  register rtx op0
    = expand_expr (((exp)->exp.operands[ 0]) , (rtx) 0 , VOIDmode, 0);
  register rtx op1
    = expand_expr (((exp)->exp.operands[ 1]) , (rtx) 0 , VOIDmode, 0);
  register tree type = ((((exp)->exp.operands[ 0]) )->common.type) ;
  register enum machine_mode mode = ((type)->type.mode) ;
  int unsignedp = ((type)->common.unsigned_flag) ;
  enum rtx_code code = unsignedp ? unsigned_code : signed_code;
  return compare_from_rtx (op0, op1, code, unsignedp, mode,
			   ((mode == BLKmode)
			    ? expr_size (((exp)->exp.operands[ 0]) ) : (rtx) 0 ),
			   ((((exp)->common.type) )->type.align)  / 8 );
}
rtx
compare_from_rtx (op0, op1, code, unsignedp, mode, size, align)
     register rtx op0, op1;
     enum rtx_code code;
     int unsignedp;
     enum machine_mode mode;
     rtx size;
     int align;
{
  if (	((op0)->code)  == CONST_INT || 	((op0)->code)  == CONST_DOUBLE)
    {
      rtx tem = op0;
      op0 = op1;
      op1 = tem;
      code = swap_condition (code);
    }
  if (flag_force_mem)
    {
      op0 = force_not_mem (op0);
      op1 = force_not_mem (op1);
    }
  do_pending_stack_adjust ();
  if (	((op0)->code)  == CONST_INT && 	((op1)->code)  == CONST_INT)
    return simplify_relational_operation (code, mode, op0, op1);
  emit_cmp_insn (op0, op1, code, size, mode, unsignedp, align);
  return gen_rtx (code, VOIDmode, cc0_rtx, const0_rtx);
}
static rtx
do_store_flag (exp, target, mode, only_cheap)
     tree exp;
     rtx target;
     enum machine_mode mode;
     int only_cheap;
{
  enum rtx_code code;
  tree arg0, arg1, type;
  tree tem;
  enum machine_mode operand_mode;
  int invert = 0;
  int unsignedp;
  rtx op0, op1;
  enum insn_code icode;
  rtx subtarget = target;
  rtx result, label, pattern, jump_pat;
  if (((enum tree_code) (exp)->common.code)  == TRUTH_NOT_EXPR)
    invert = 1, exp = ((exp)->exp.operands[ 0]) ;
  arg0 = ((exp)->exp.operands[ 0]) ;
  arg1 = ((exp)->exp.operands[ 1]) ;
  type = ((arg0)->common.type) ;
  operand_mode = ((type)->type.mode) ;
  unsignedp = ((type)->common.unsigned_flag) ;
  if (operand_mode == BLKmode)
    return 0;
  while ((((enum tree_code) (arg0)->common.code)  == NOP_EXPR	|| ((enum tree_code) (arg0)->common.code)  == CONVERT_EXPR	|| ((enum tree_code) (arg0)->common.code)  == NON_LVALUE_EXPR)	&& (((((arg0)->common.type) )->type.mode) 	== ((((((arg0)->exp.operands[ 0]) )->common.type) )->type.mode) ))	(arg0) = ((arg0)->exp.operands[ 0]) ; ;
  while ((((enum tree_code) (arg1)->common.code)  == NOP_EXPR	|| ((enum tree_code) (arg1)->common.code)  == CONVERT_EXPR	|| ((enum tree_code) (arg1)->common.code)  == NON_LVALUE_EXPR)	&& (((((arg1)->common.type) )->type.mode) 	== ((((((arg1)->exp.operands[ 0]) )->common.type) )->type.mode) ))	(arg1) = ((arg1)->exp.operands[ 0]) ; ;
  switch (((enum tree_code) (exp)->common.code) )
    {
    case EQ_EXPR:
      code = EQ;
      break;
    case NE_EXPR:
      code = NE;
      break;
    case LT_EXPR:
      if (integer_onep (arg1))
	arg1 = integer_zero_node, code = unsignedp ? LEU : LE;
      else
	code = unsignedp ? LTU : LT;
      break;
    case LE_EXPR:
      if (integer_all_onesp (arg1))
	arg1 = integer_zero_node, code = unsignedp ? LTU : LT;
      else
	code = unsignedp ? LEU : LE;
      break;
    case GT_EXPR:
      if (integer_all_onesp (arg1))
	arg1 = integer_zero_node, code = unsignedp ? GEU : GE;
      else
	code = unsignedp ? GTU : GT;
      break;
    case GE_EXPR:
      if (integer_onep (arg1))
	arg1 = integer_zero_node, code = unsignedp ? GTU : GT;
      else
	code = unsignedp ? GEU : GE;
      break;
    default:
      abort ();
    }
  if (((enum tree_code) (arg0)->common.code)  == REAL_CST || ((enum tree_code) (arg0)->common.code)  == INTEGER_CST)
    {
      tem = arg0; arg0 = arg1; arg1 = tem;
      code = swap_condition (code);
    }
  if ((code == NE || code == EQ)
      && ((enum tree_code) (arg0)->common.code)  == BIT_AND_EXPR && integer_zerop (arg1)
      && integer_pow2p (((arg0)->exp.operands[ 1]) )
      && ((type)->type.precision)  <= 32  )
    {
      int bitnum = exact_log2_wide ((int ) (((expand_expr (((arg0)->exp.operands[ 1]) ,
						    (rtx) 0 , VOIDmode, 0))->fld[0].rtwint) )) ;
      if (subtarget == 0 || 	((subtarget)->code)  != REG
	  || 	((subtarget)->mode)  != operand_mode
	  || ! safe_from_p (subtarget, ((arg0)->exp.operands[ 0]) ))
	subtarget = 0;
      op0 = expand_expr (((arg0)->exp.operands[ 0]) , subtarget, VOIDmode, 0);
      if (bitnum != 0)
	op0 = expand_shift (RSHIFT_EXPR, 	((op0)->mode) , op0,
			    size_int (bitnum), target, 1);
      if (	((op0)->mode)  != mode)
	op0 = convert_to_mode (mode, op0, 1);
      if (bitnum != ((type)->type.precision)  - 1)
	op0 = expand_and (op0, const1_rtx, target);
      if ((code == EQ && ! invert) || (code == NE && invert))
	op0 = expand_binop (mode, xor_optab, op0, const1_rtx, target, 0,
			    OPTAB_LIB_WIDEN);
      return op0;
    }
  if (! can_compare_p (operand_mode))
    return 0;
  icode = setcc_gen_code[(int) code];
  if (icode == CODE_FOR_nothing
      || (only_cheap && insn_operand_mode[(int) icode][0] != mode))
    {
      if ((code == LT && integer_zerop (arg1))
	  || (! only_cheap && code == GE && integer_zerop (arg1)))
	;
      else if (1  >= 0
	       && ! only_cheap && (code == NE || code == EQ)
	       && ((enum tree_code) (type)->common.code)  != REAL_TYPE
	       && ((abs_optab->handlers[(int) operand_mode].insn_code
		    != CODE_FOR_nothing)
		   || (ffs_optab->handlers[(int) operand_mode].insn_code
		       != CODE_FOR_nothing)))
	;
      else
	return 0;
    }
  preexpand_calls (exp);
  if (subtarget == 0 || 	((subtarget)->code)  != REG
      || 	((subtarget)->mode)  != operand_mode
      || ! safe_from_p (subtarget, arg1))
    subtarget = 0;
  op0 = expand_expr (arg0, subtarget, VOIDmode, 0);
  op1 = expand_expr (arg1, (rtx) 0 , VOIDmode, 0);
  if (target == 0)
    target = gen_reg_rtx (mode);
  result = emit_store_flag (target, code,
			    queued_subexp_p (op0) ? copy_rtx (op0) : op0,
			    queued_subexp_p (op1) ? copy_rtx (op1) : op1,
			    operand_mode, unsignedp, 1);
  if (result)
    {
      if (invert)
	result = expand_binop (mode, xor_optab, result, const1_rtx,
			       result, 0, OPTAB_LIB_WIDEN);
      return result;
    }
  if (target == 0 || 	((target)->code)  != REG
      || reg_mentioned_p (target, op0) || reg_mentioned_p (target, op1))
    target = gen_reg_rtx (	((target)->mode) );
  emit_move_insn (target, invert ? const0_rtx : const1_rtx);
  result = compare_from_rtx (op0, op1, code, unsignedp,
			     operand_mode, (rtx) 0 , 0);
  if (	((result)->code)  == CONST_INT)
    return (((result == const0_rtx && ! invert)
	     || (result != const0_rtx && invert))
	    ? const0_rtx : const1_rtx);
  label = gen_label_rtx ();
  if (bcc_gen_fctn[(int) code] == 0)
    abort ();
  emit_jump_insn ((*bcc_gen_fctn[(int) code]) (label));
  emit_move_insn (target, invert ? const1_rtx : const0_rtx);
  emit_label (label);
  return target;
}
void
do_tablejump (index, mode, range, table_label, default_label)
     rtx index, range, table_label, default_label;
     enum machine_mode mode;
{
  register rtx temp, vector;
  emit_cmp_insn (range, index, LTU, (rtx) 0 , mode, 0, 0);
  emit_jump_insn (gen_bltu (default_label));
  if (mode != SImode )
    index = convert_to_mode (SImode , index, 1);
  index = memory_address_noforce
    (SImode ,
     gen_rtx (PLUS, SImode ,
	      gen_rtx (MULT, SImode , index,
		       gen_rtx (CONST_INT, VOIDmode, (	(mode_size[(int)(SImode )]) )) ),
	      gen_rtx (LABEL_REF, SImode , table_label)));
  temp = gen_reg_rtx (SImode );
  vector = gen_rtx (MEM, SImode , index);
  ((vector)->unchanging)  = 1;
  convert_move (temp, vector, 0);
  emit_jump_insn (gen_tablejump (temp, table_label));
  if (! flag_pic)
    emit_barrier ();
}
