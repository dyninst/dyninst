
extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

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

static struct _iobuf  *asmfile;

enum typestatus {TYPE_UNSEEN, TYPE_XREF, TYPE_DEFINED};

enum typestatus *typevec;

static int typevec_len;

static int next_type_number;

static int next_block_number;

static int current_sym_code;
static int current_sym_value;
static rtx current_sym_addr;

static int current_sym_nchars;

void dbxout_types ();
void dbxout_tags ();
void dbxout_args ();
void dbxout_symbol ();
static void dbxout_type_name ();
static void dbxout_type ();
static void dbxout_finish_symbol ();
static void dbxout_continue ();

void
dbxout_init (asm_file, input_file_name)
     struct _iobuf  *asm_file;
     char *input_file_name;
{
  asmfile = asm_file;

  typevec_len = 100;
  typevec = (enum typestatus *) xmalloc (typevec_len * sizeof typevec[0]);
  memset (typevec,0, typevec_len * sizeof typevec[0]) ;

  fprintf (asmfile,
	   "\t.stabs \"%s\",%d,0,0,Ltext\nLtext:\n",
	   input_file_name, 0x64		);

  next_type_number = 1;
  next_block_number = 2;

  dbxout_symbol (((integer_type_node)->type.name) , 0);
  dbxout_symbol (((char_type_node)->type.name) , 0);

  dbxout_types (get_permanent_types ());
}

static void
dbxout_continue ()
{

  fprintf (asmfile, "\\\\");

  dbxout_finish_symbol ();
  fprintf (asmfile, ".stabs \"");
  current_sym_nchars = 0;
}

static void
dbxout_type (type, full)
     tree type;
     int full;
{
  register tree tem;

  if (type == error_mark_node)
    type = integer_type_node;
  else if (((type)->type.size)  == 0)
    type = ((type)->type.main_variant) ;

  if (((type)->type.symtab_address)  == 0)
    {
      ((type)->type.symtab_address)  = next_type_number++;

      if (next_type_number == typevec_len)
	{
	  typevec = (enum typestatus *) xrealloc (typevec, typevec_len * 2 * sizeof typevec[0]);
	  memset (typevec + typevec_len,0, typevec_len * sizeof typevec[0]) ;
	  typevec_len *= 2;
	}
    }

  fprintf (asmfile, "%d", ((type)->type.symtab_address) );
  (current_sym_nchars += (3)) ;

  switch (typevec[((type)->type.symtab_address) ])
    {
    case TYPE_UNSEEN:
      break;
    case TYPE_XREF:
      if (! full)
	return;
      break;
    case TYPE_DEFINED:
      return;
    }

  fprintf (asmfile, "=");
  (current_sym_nchars += (1)) ;

  typevec[((type)->type.symtab_address) ] = TYPE_DEFINED;

  switch (((type)->common.code) )
    {
    case VOID_TYPE:

      fprintf (asmfile, "%d", ((type)->type.symtab_address) );
      (current_sym_nchars += (3)) ;
      break;

    case INTEGER_TYPE:
      if (type == char_type_node && ! ((type)->common.unsigned_attr) )

	fprintf (asmfile, "r2;0;127;");
      else
	fprintf (asmfile, "r1;%d;%d;",
		 ((((type)->type.sep) )->int_cst.int_cst_low) ,
		 ((((type)->type.max) )->int_cst.int_cst_low) );
      (current_sym_nchars += (25)) ;
      break;

    case REAL_TYPE:
      fprintf (asmfile, "r1;%d;0;",
	       ((size_in_bytes (type))->int_cst.int_cst_low) );
      (current_sym_nchars += (16)) ;
      break;

    case ARRAY_TYPE:

      fprintf (asmfile, "ar1;0;%d;",
	       (((type)->type.values) 
		? ((((((type)->type.values) )->type.max) )->int_cst.int_cst_low) 
	        : -1));
      (current_sym_nchars += (17)) ;
      dbxout_type (((type)->common.type) , 0);
      break;

    case RECORD_TYPE:
    case UNION_TYPE:
      if ((((type)->type.name)  != 0 && !full)
	  || ((type)->type.size)  == 0)
	{

	  fprintf (asmfile, (((type)->common.code)  == RECORD_TYPE) ? "xs" : "xu");
	  (current_sym_nchars += (3)) ;

	  dbxout_type_name (type);
	  fprintf (asmfile, ":");
	  typevec[((type)->type.symtab_address) ] = TYPE_XREF;
	  break;
	}
      tem = size_in_bytes (type);
      fprintf (asmfile, (((type)->common.code)  == RECORD_TYPE) ? "s%d" : "u%d",
	       ((tem)->int_cst.int_cst_low) );

      if (((type)->type.basetypes)  && use_gdb_dbx_extensions)
	{
	  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('!')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('!')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('!'),  asmfile))) ;
	  (--(
		asmfile)->_cnt >= 0 ?	(int)(*(
		asmfile)->_ptr++ = (unsigned char)((((((type)->type.basetypes) )->common.public_attr)  ? '2' : '0'))) :	(((
		asmfile)->_flag & 0200 ) && -(
		asmfile)->_cnt < (
		asmfile)->_bufsiz ?	((*(
		asmfile)->_ptr = (unsigned char)((((((type)->type.basetypes) )->common.public_attr)  ? '2' : '0'))) != '\n' ?	(int)(*(
		asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)(
		asmfile)->_ptr, 
		asmfile)) :	_flsbuf((unsigned char)((((((type)->type.basetypes) )->common.public_attr)  ? '2' : '0')), 
		asmfile))) ;
	  dbxout_type (((((type)->type.basetypes) )->list.value) , 0);
	  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(',')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(',')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(','),  asmfile))) ;
	  (current_sym_nchars += (3)) ;
	}
      (current_sym_nchars += (11)) ;

      for (tem = ((type)->type.values) ; tem; tem = ((tem)->common.chain) )

	if (((tem)->decl.name)  != 0)
	  {

	    if (tem != ((type)->type.values) )
	      do {if (current_sym_nchars > 80 ) dbxout_continue ();} while (0) ;
	    fprintf (asmfile, "%s:", ((((tem)->decl.name) )->identifier.pointer) );
	    (current_sym_nchars += (2 + ((((tem)->decl.name) )->identifier.length) )) ;
	    if (use_gdb_dbx_extensions)
	      {
		(--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('/')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('/')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('/'),  asmfile))) ;

		(current_sym_nchars += (2)) ;
		if (((tem)->common.code)  == FUNCTION_DECL)
		  {
		    (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(':')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(':')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(':'),  asmfile))) ;
		    (current_sym_nchars += (1)) ;
		    dbxout_type (((tem)->common.type) , 0);  
		    dbxout_args (((((tem)->common.type) )->type.values) );

		    (current_sym_nchars += (3 + strlen (((((((tem)->decl.rtl) )->fld[ 0].rtx) )->fld[ 0].rtstr) ))) ;
		  }
		else
		  dbxout_type (((tem)->common.type) , 0);
	      }
	    else
	      dbxout_type (((tem)->common.type) , 0);

	    if (((tem)->common.code)  == VAR_DECL)
	      {
		if (use_gdb_dbx_extensions)
		  {
		    fprintf (asmfile, ":%s", 
			     ((((((tem)->decl.rtl) )->fld[ 0].rtx) )->fld[ 0].rtstr) );
		    (current_sym_nchars += (2 + strlen (((((((tem)->decl.rtl) )->fld[ 0].rtx) )->fld[ 0].rtstr) ))) ;
		  }
		else
		  {
		    fprintf (asmfile, ",0,0;");
		    (current_sym_nchars += (5)) ;
		  }
	      }
	    else
	      {
		fprintf (asmfile, ",%d,%d;", ((tem)->decl.offset) ,
			 (((((tem)->decl.size) )->int_cst.int_cst_low) 
			  * ((tem)->decl.size_unit) ));
		(current_sym_nchars += (23)) ;
	      }
	  }

      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(';')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(';')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(';'),  asmfile))) ;
      (current_sym_nchars += (1)) ;
      break;

    case ENUMERAL_TYPE:
      if ((((type)->type.name)  != 0 && !full)
	  || ((type)->type.size)  == 0)
	{
	  fprintf (asmfile, "xe");
	  (current_sym_nchars += (3)) ;
	  dbxout_type_name (type);
	  typevec[((type)->type.symtab_address) ] = TYPE_XREF;
	  fprintf (asmfile, ":");
	  return;
	}
      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('e')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('e')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('e'),  asmfile))) ;
      (current_sym_nchars += (1)) ;
      for (tem = ((type)->type.values) ; tem; tem = ((tem)->common.chain) )
	{
	  fprintf (asmfile, "%s:%d,", ((((tem)->list.purpose) )->identifier.pointer) ,
		   ((((tem)->list.value) )->int_cst.int_cst_low) );
	  (current_sym_nchars += (11 + ((((tem)->list.purpose) )->identifier.length) )) ;
	  if (((tem)->common.chain)  != 0)
	    do {if (current_sym_nchars > 80 ) dbxout_continue ();} while (0) ;
	}
      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(';')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(';')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(';'),  asmfile))) ;
      (current_sym_nchars += (1)) ;
      break;

    case POINTER_TYPE:
      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('*')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('*')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('*'),  asmfile))) ;
      (current_sym_nchars += (1)) ;
      dbxout_type (((type)->common.type) , 0);
      break;

    case METHOD_TYPE:
      if (use_gdb_dbx_extensions)
	{
	  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('@')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('@')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('@'),  asmfile))) ;
	  (current_sym_nchars += (1)) ;
	  dbxout_type (((type)->type.max) , 0);
	  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(',')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(',')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(','),  asmfile))) ;
	  (current_sym_nchars += (1)) ;
	  dbxout_type (((type)->common.type) , 0);
	}
      else
	{
	  dbxout_type (((type)->common.type) , 0);
	}
      break;

    case OFFSET_TYPE:
      if (use_gdb_dbx_extensions)
	{
	  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('@')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('@')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('@'),  asmfile))) ;
	  (current_sym_nchars += (1)) ;
	  dbxout_type (((type)->type.max) , 0);
	  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(',')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(',')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(','),  asmfile))) ;
	  (current_sym_nchars += (1)) ;
	  dbxout_type (((type)->common.type) , 0);
	}
      else
	{
	  dbxout_type (((type)->common.type) , 0);
	}
      break;

    case REFERENCE_TYPE:
      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(use_gdb_dbx_extensions ? '&' : '*')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(use_gdb_dbx_extensions ? '&' : '*')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(use_gdb_dbx_extensions ? '&' : '*'),  asmfile))) ;
      (current_sym_nchars += (1)) ;
      dbxout_type (((type)->common.type) , 0);
      break;

    case FUNCTION_TYPE:
      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('f')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('f')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('f'),  asmfile))) ;
      (current_sym_nchars += (1)) ;
      dbxout_type (((type)->common.type) , 0);
      break;

    default:
      abort ();
    }
}

static void
dbxout_type_name (type)
     register tree type;
{
  tree t;
  if (((type)->type.name)  == 0)
    abort ();
  if (((((type)->type.name) )->common.code)  == IDENTIFIER_NODE)
    {
      t = ((type)->type.name) ;
    }
  else if (((((type)->type.name) )->common.code)  == TYPE_DECL)
    {
      t = ((((type)->type.name) )->decl.name) ;
    }
  else
    abort ();

  fprintf (asmfile, "%s", ((t)->identifier.pointer) );
  (current_sym_nchars += (((t)->identifier.length) )) ;
}

void
dbxout_symbol (decl, local)
     tree decl;
     int local;
{
  int letter = 0;
  tree type = ((decl)->common.type) ;

  if (local == 0)
    {
      dbxout_tags (gettags ());
      dbxout_types (get_permanent_types ());
    }

  current_sym_code = 0;
  current_sym_value = 0;
  current_sym_addr = 0;

  current_sym_nchars = 2 + ((((decl)->decl.name) )->identifier.length) ;

  switch (((decl)->common.code) )
    {
    case CONST_DECL:
      break;

    case FUNCTION_DECL:
      if (((decl)->decl.rtl)  == 0)
	return;
      if (((decl)->common.external_attr) )
	break;
      if (	((((decl)->decl.rtl) )->code)  != MEM
	  || 	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  != SYMBOL_REF)
	break;
       ;
      fprintf (asmfile, ".stabs \"%s:%c",
	       ((((decl)->decl.name) )->identifier.pointer) ,
	       ((decl)->common.public_attr)  ? 'F' : 'f');

      current_sym_code = 0x24		;
      current_sym_addr = ((((decl)->decl.rtl) )->fld[ 0].rtx) ;

      if (((((decl)->common.type) )->common.type) )
	dbxout_type (((((decl)->common.type) )->common.type) , 0);
      else
	dbxout_type (void_type_node, 0);
      dbxout_finish_symbol ();
      break;

    case TYPE_DECL:

      if (((decl)->common.asm_written_attr) )
	return;

       ;
      fprintf (asmfile, ".stabs \"%s:t",
	       ((((decl)->decl.name) )->identifier.pointer) );

      current_sym_code = 0x80		;

      dbxout_type (((decl)->common.type) , 1);
      dbxout_finish_symbol ();

      ((decl)->common.asm_written_attr)  = 1;
      break;
    case PARM_DECL:

      abort ();

    case VAR_DECL:
      if (((decl)->decl.rtl)  == 0)
	return;

      if (((decl)->common.external_attr) )
	break;

      if (	((((decl)->decl.rtl) )->code)  == REG
	  && (((((decl)->decl.rtl) )->fld[0].rtint)  < 0
	      || ((((decl)->decl.rtl) )->fld[0].rtint)  >= 56 ))
	break;

      if (	((((decl)->decl.rtl) )->code)  == MEM
	  && 	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  == SYMBOL_REF)
	{
	  if (((decl)->common.public_attr) )
	    {
	      letter = 'G';
	      current_sym_code = 0x20		;
	    }
	  else
	    {
	      current_sym_addr = ((((decl)->decl.rtl) )->fld[ 0].rtx) ;

	      letter = ((decl)->common.permanent_attr)  ? 'S' : 'V';

	      if (!((decl)->decl.initial) )
		current_sym_code = 0x28		;

	      else
		current_sym_code = 0x26		;
	    }
	}
      else if (	((((decl)->decl.rtl) )->code)  == REG)
	{
	  letter = 'r';
	  current_sym_code = 0x40		;
	  current_sym_value = ((((((decl)->decl.rtl) )->fld[0].rtint) ) < 16 ? (((((decl)->decl.rtl) )->fld[0].rtint) ) : (((((decl)->decl.rtl) )->fld[0].rtint) ) + 2) ;
	}
      else if (	((((decl)->decl.rtl) )->code)  == MEM
	       && (	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  == MEM
		   || (	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  == REG
		       && ((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[0].rtint)  != 14 )))

	{
	  if (	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  == REG)
	    {
	      letter = 'r';
	      current_sym_code = 0x40		;
	      current_sym_value = ((((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[0].rtint) ) < 16 ? (((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[0].rtint) ) : (((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[0].rtint) ) + 2) ;
	    }
	  else
	    {
	      current_sym_code = 0x80		;

	      current_sym_value = ((((((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint) ;
	    }

	  type = build_pointer_type (((decl)->common.type) );
	}
      else if (	((((decl)->decl.rtl) )->code)  == MEM
	       && 	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  == REG)
	{
	  current_sym_code = 0x80		;
	  current_sym_value = 0;
	}
      else if (	((((decl)->decl.rtl) )->code)  == MEM
	       && 	((((((decl)->decl.rtl) )->fld[ 0].rtx) )->code)  == PLUS
	       && 	((((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT)
	{
	  current_sym_code = 0x80		;

	  current_sym_value = ((((((((decl)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint) ;
	}
      else

	break;

       ;
      fprintf (asmfile, ".stabs \"%s:",
	       ((((decl)->decl.name) )->identifier.pointer) );
      if (letter) (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(letter)) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(letter)) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(letter),  asmfile))) ;
      dbxout_type (type, 0);
      dbxout_finish_symbol ();
      break;
    }
}

static void
dbxout_finish_symbol ()
{
  fprintf (asmfile, "\",%d,0,0,", current_sym_code);
  if (current_sym_addr)
    output_addr_const (asmfile, current_sym_addr);
  else
    fprintf (asmfile, "%d", current_sym_value);
  (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)('\n')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)('\n')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)('\n'),  asmfile))) ;
}

static void
dbxout_syms (syms)
     tree syms;
{
  while (syms)
    {
      dbxout_symbol (syms, 1);
      syms = ((syms)->common.chain) ;
    }
}

static void
dbxout_parms (parms)
     tree parms;
{
  for (; parms; parms = ((parms)->common.chain) )
    {
      if (((parms)->decl.offset)  >= 0)
	{
	  current_sym_code = 0xa0		;
	  current_sym_value = ((parms)->decl.offset)  / 8 ;
	  current_sym_addr = 0;
	  current_sym_nchars = 2 + strlen (((((parms)->decl.name) )->identifier.pointer) );

	   ;
	  fprintf (asmfile, ".stabs \"%s:p",
		   ((((parms)->decl.name) )->identifier.pointer) );

	  if (	((((parms)->decl.rtl) )->code)  == REG
	      && ((((parms)->decl.rtl) )->fld[0].rtint)  >= 0
	      && ((((parms)->decl.rtl) )->fld[0].rtint)  < 56 )
	    dbxout_type (((parms)->decl.arguments)   , 0);
	  else
	    {

	      if (((parms)->common.type)  != ((parms)->decl.arguments)   )
		current_sym_value += (	(mode_size[(int)(((((parms)->decl.arguments)   )->type.mode) )]) 
				      - 	(mode_size[(int)(	((((parms)->decl.rtl) )->mode) )]) );

	      if (	((((parms)->decl.rtl) )->code)  == MEM
		  && 	((((((parms)->decl.rtl) )->fld[ 0].rtx) )->code)  == PLUS
		  && 	((((((((parms)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT
		  && ((((((((parms)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == current_sym_value)
		dbxout_type (((parms)->common.type) , 0);
	      else
		{
		  current_sym_value = ((parms)->decl.offset)  / 8 ;
		  dbxout_type (((parms)->decl.arguments)   , 0);
		}
	    }
	  dbxout_finish_symbol ();
	}

      else if (	((((parms)->decl.rtl) )->code)  == REG
	       && ((((parms)->decl.rtl) )->fld[0].rtint)  >= 0
	       && ((((parms)->decl.rtl) )->fld[0].rtint)  < 56 )
	{
	  current_sym_code = 0x40		;
	  current_sym_value = ((((((parms)->decl.rtl) )->fld[0].rtint) ) < 16 ? (((((parms)->decl.rtl) )->fld[0].rtint) ) : (((((parms)->decl.rtl) )->fld[0].rtint) ) + 2) ;
	  current_sym_addr = 0;
	  current_sym_nchars = 2 + strlen (((((parms)->decl.name) )->identifier.pointer) );

	   ;
	  fprintf (asmfile, ".stabs \"%s:P",
		   ((((parms)->decl.name) )->identifier.pointer) );

	  dbxout_type (((parms)->decl.arguments)   , 0);
	  dbxout_finish_symbol ();
	}
      else if (	((((parms)->decl.rtl) )->code)  == MEM
	       && ((((parms)->decl.rtl) )->fld[ 0].rtx)  != const0_rtx)
	{
	  current_sym_code = 0x80		;

	  current_sym_value = ((((((((parms)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint) ;
	  current_sym_addr = 0;
	  current_sym_nchars = 2 + strlen (((((parms)->decl.name) )->identifier.pointer) );

	   ;
	  fprintf (asmfile, ".stabs \"%s:p",
		   ((((parms)->decl.name) )->identifier.pointer) );

	  if (((parms)->common.type)  != ((parms)->decl.arguments)   )
	    current_sym_value += (	(mode_size[(int)(((((parms)->decl.arguments)   )->type.mode) )]) 
				  - 	(mode_size[(int)(	((((parms)->decl.rtl) )->mode) )]) );

	  dbxout_type (((parms)->common.type) , 0);
	  dbxout_finish_symbol ();
	}
    }
}

static void
dbxout_reg_parms (parms)
     tree parms;
{
  while (parms)
    {
      if (	((((parms)->decl.rtl) )->code)  == REG
	  && ((((parms)->decl.rtl) )->fld[0].rtint)  >= 0
	  && ((((parms)->decl.rtl) )->fld[0].rtint)  < 56 
	  && ((parms)->decl.offset)  >= 0)
	{
	  current_sym_code = 0x40		;
	  current_sym_value = ((((((parms)->decl.rtl) )->fld[0].rtint) ) < 16 ? (((((parms)->decl.rtl) )->fld[0].rtint) ) : (((((parms)->decl.rtl) )->fld[0].rtint) ) + 2) ;
	  current_sym_addr = 0;
	  current_sym_nchars = 2 + ((((parms)->decl.name) )->identifier.length) ;
	   ;
	  fprintf (asmfile, ".stabs \"%s:r",
		   ((((parms)->decl.name) )->identifier.pointer) );
	  dbxout_type (((parms)->common.type) , 0);
	  dbxout_finish_symbol ();
	}
      else if (	((((parms)->decl.rtl) )->code)  == MEM
	       && 	((((((parms)->decl.rtl) )->fld[ 0].rtx) )->code)  == PLUS
	       && 	((((((((parms)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT)
	{
	  int offset = ((parms)->decl.offset)  / 8 ;

	  if (offset != -1 && ((parms)->common.type)  != ((parms)->decl.arguments)   )
	    offset += (	(mode_size[(int)(((((parms)->decl.arguments)   )->type.mode) )]) 
		       - 	(mode_size[(int)(	((((parms)->decl.rtl) )->mode) )]) );

	  if (((((((((parms)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  != offset)
	    {
	      current_sym_code = 0x80		;
	      current_sym_value = ((((((((parms)->decl.rtl) )->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint) ;
	      current_sym_addr = 0;
	      current_sym_nchars = 2 + ((((parms)->decl.name) )->identifier.length) ;
	       ;
	      fprintf (asmfile, ".stabs \"%s:",
		       ((((parms)->decl.name) )->identifier.pointer) );
	      dbxout_type (((parms)->common.type) , 0);
	      dbxout_finish_symbol ();
	    }
	}
      parms = ((parms)->common.chain) ;
    }
}

void
dbxout_args (args)
     tree args;
{
  while (args)
    {
      (--( asmfile)->_cnt >= 0 ?	(int)(*( asmfile)->_ptr++ = (unsigned char)(',')) :	((( asmfile)->_flag & 0200 ) && -( asmfile)->_cnt < ( asmfile)->_bufsiz ?	((*( asmfile)->_ptr = (unsigned char)(',')) != '\n' ?	(int)(*( asmfile)->_ptr++) :	_flsbuf(*(unsigned char *)( asmfile)->_ptr,  asmfile)) :	_flsbuf((unsigned char)(','),  asmfile))) ;
      dbxout_type (((args)->list.value) , 0);
      (current_sym_nchars += (1)) ;
      args = ((args)->common.chain) ;
    }
}

void
dbxout_types (types)
     register tree types;
{
  while (types)
    {
      if (((types)->type.name) 
	  && ((((types)->type.name) )->common.code)  == TYPE_DECL
	  && ! ((((types)->type.name) )->common.asm_written_attr) )
	dbxout_symbol (((types)->type.name) , 1);
      types = ((types)->common.chain) ;
    }
}

void
dbxout_tags (tags)
     tree tags;
{
  register tree link;
  for (link = tags; link; link = ((link)->common.chain) )
    {
      register tree type = ((((link)->list.value) )->type.main_variant) ;
      if (((link)->list.purpose)  != 0
	  && ! ((link)->common.asm_written_attr) 
	  && ((type)->type.size)  != 0)
	{
	  ((link)->common.asm_written_attr)  = 1;
	  current_sym_code = 0x80		;
	  current_sym_value = 0;
	  current_sym_addr = 0;
	  current_sym_nchars = 2 + ((((link)->list.purpose) )->identifier.length) ;

	   ;
	  fprintf (asmfile, ".stabs \"%s:T",
		   ((((link)->list.purpose) )->identifier.pointer) );
	  dbxout_type (type, 1);
	  dbxout_finish_symbol ();
	}
    }
}

static void
dbxout_block (stmt, depth, args)
     register tree stmt;
     int depth;
     tree args;
{
  int blocknum;

  while (stmt)
    {
      switch (((stmt)->common.code) )
	{
	case COMPOUND_STMT:
	case LOOP_STMT:
	  dbxout_block (((stmt)->stmt.body) , depth, 0);
	  break;

	case IF_STMT:
	  dbxout_block (((stmt)->if_stmt.thenpart) , depth, 0);
	  dbxout_block (((stmt)->if_stmt.elsepart) , depth, 0);
	  break;

	case LET_STMT:
	  dbxout_tags (((stmt)->bind_stmt.type_tags) );
	  dbxout_syms (((stmt)->bind_stmt.vars) );
	  if (args)
	    dbxout_reg_parms (args);

	  if (depth > 0)
	    {
	      char buf[20];
	      blocknum = next_block_number++;
	      sprintf (buf, "*%s%d",  "LBB",  blocknum) ;
	      fprintf (asmfile, ".stabn %d,0,0,", 0xc0		);
	      assemble_name (asmfile, buf);
	      fprintf (asmfile, "\n");
	    }

	  dbxout_block (((stmt)->stmt.body) , depth + 1, 0);

	  if (depth > 0)
	    {
	      char buf[20];
	      sprintf (buf, "*%s%d",  "LBE",  blocknum) ;
	      fprintf (asmfile, ".stabn %d,0,0,", 0xe0		);
	      assemble_name (asmfile, buf);
	      fprintf (asmfile, "\n");
	    }
	}
      stmt = ((stmt)->common.chain) ;
    }
}

void
dbxout_function (decl)
     tree decl;
{
  dbxout_symbol (decl, 0);
  dbxout_parms (((decl)->decl.arguments)  );
  dbxout_block (((decl)->decl.initial) , 0, ((decl)->decl.arguments)  );

  dbxout_types (get_temporary_types ());
}


