
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

extern int recog_memoized ();

extern void insn_extract ();

extern rtx recog_operand[];

extern rtx *recog_operand_loc[];

extern rtx *recog_dup_loc[];

extern char recog_dup_num[];

extern char *insn_template[];

extern char *(*insn_outfun[]) ();

extern int insn_n_operands[];

extern int insn_n_dups[];

extern int insn_n_alternatives[];

extern char *insn_operand_constraint[][5 ];

extern char insn_operand_address_p[][5 ];

extern enum machine_mode insn_operand_mode[][5 ];

extern char insn_operand_strict_low[][5 ];

extern int (*insn_operand_predicate[][5 ]) ();

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

extern double ldexp ();

extern double atof ();

union real_extract 
{
  double  d;
  int i[sizeof (double ) / sizeof (int)];
};

static int inequality_comparisons_p ();
int strict_memory_address_p ();
int memory_address_p ();

int volatile_ok;

rtx recog_addr_dummy;

int which_alternative;

int reload_completed;

void
init_recog ()
{
  volatile_ok = 0;
  recog_addr_dummy = gen_rtx (MEM, VOIDmode, 0);
}

int
recog_memoized (insn)
     rtx insn;
{
  volatile_ok = 1;
  if (((insn)->fld[4].rtint)  < 0)
    ((insn)->fld[4].rtint)  = recog (((insn)->fld[3].rtx) , insn);
  return ((insn)->fld[4].rtint) ;
}

int
next_insn_tests_no_inequality (insn)
     rtx insn;
{
  register rtx next = ((insn)->fld[2].rtx) ;

  return ((	((next)->code)  == JUMP_INSN
	   || 	((next)->code)  == INSN
	   || 	((next)->code)  == CALL_INSN)
	  && ! inequality_comparisons_p (((next)->fld[3].rtx) ));
}

int
next_insns_test_no_inequality (insn)
     rtx insn;
{
  register rtx next = ((insn)->fld[2].rtx) ;

  for (;; next = ((next)->fld[2].rtx) )
    {
      if (	((next)->code)  == CODE_LABEL
	  || 	((next)->code)  == BARRIER)
	return 1;
      if (	((next)->code)  == NOTE)
	continue;
      if (inequality_comparisons_p (((next)->fld[3].rtx) ))
	return 0;
      if (	((((next)->fld[3].rtx) )->code)  == SET
	  && ((((next)->fld[3].rtx) )->fld[0].rtx)  == cc0_rtx)
	return 1;
      if (! reg_mentioned_p (cc0_rtx, ((next)->fld[3].rtx) ))
	return 1;
    }
}

static int
inequality_comparisons_p (x)
     rtx x;
{
  register char *fmt;
  register int len, i;
  register enum rtx_code code = 	((x)->code) ;

  switch (code)
    {
    case REG:
    case PC:
    case CC0:
    case CONST_INT:
    case CONST_DOUBLE:
    case CONST:
    case LABEL_REF:
    case SYMBOL_REF:
      return 0;

    case LT:
    case LTU:
    case GT:
    case GTU:
    case LE:
    case LEU:
    case GE:
    case GEU:
      return (((x)->fld[ 0].rtx)  == cc0_rtx || ((x)->fld[ 1].rtx)  == cc0_rtx);
    }

  len = 	(rtx_length[(int)(code)]) ;
  fmt = 	(rtx_format[(int)(code)]) ;

  for (i = 0; i < len; i++)
    {
      if (fmt[i] == 'e')
	{
	  if (inequality_comparisons_p (((x)->fld[ i].rtx) ))
	    return 1;
	}
      else if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = ((x)->fld[ i].rtvec->num_elem)  - 1; j >= 0; j--)
	    if (inequality_comparisons_p (((x)->fld[ i].rtvec->elem[ j].rtx) ))
	      return 1;
	}
    }
  return 0;
}

int
general_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  register enum rtx_code code = 	((op)->code) ;
  int mode_altering_drug = 0;

  if (mode == VOIDmode)
    mode = 	((op)->mode) ;

  if ((	((op)->code)  == LABEL_REF || 	((op)->code)  == SYMBOL_REF	|| 	((op)->code)  == CONST_INT	|| 	((op)->code)  == CONST) )
    return ((	((op)->mode)  == VOIDmode || 	((op)->mode)  == mode)
	    && 1 );

  if (	((op)->mode)  != mode)
    return 0;

  while (code == SUBREG)
    {
      op = ((op)->fld[0].rtx) ;
      code = 	((op)->code) ;

    }
  if (code == REG)
    return 1;
  if (code == CONST_DOUBLE)
    return 1 ;
  if (code == MEM)
    {
      register rtx y = ((op)->fld[ 0].rtx) ;
      if (! volatile_ok && ((op)->volatil) )
	return 0;
      { { if (( (	(( y)->code)  == LABEL_REF || 	(( y)->code)  == SYMBOL_REF	|| 	(( y)->code)  == CONST_INT	|| 	(( y)->code)  == CONST)  	|| (	(( y)->code)  == REG && (((( y)->fld[0].rtint)  & ~027) != 0) )	|| ((	(( y)->code)  == PRE_DEC || 	(( y)->code)  == POST_INC)	&& (	(((( y)->fld[ 0].rtx) )->code)  == REG) 	&& (((((( y)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) )	|| (	(( y)->code)  == PLUS	&& (	(((( y)->fld[ 0].rtx) )->code)  == REG)  && (((((( y)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) 	&& 	(((( y)->fld[ 1].rtx) )->code)  == CONST_INT	&& ((unsigned) (((( y)->fld[ 1].rtx) )->fld[0].rtint)  + 0x8000) < 0x10000)) ) goto   win; } ;	{ { if (	(( y)->code)  == PLUS && (((	(((( y)->fld[ 0].rtx) )->code)  == REG && (((((( y)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( y)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( y)->fld[ 0].rtx) )->code)  == MULT	&& ((	(((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( y)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( y)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( y)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( y)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( y)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( y)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	(((( y)->fld[ 1].rtx) )->code)  == REG && (((((( y)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	(( y)->code)  == PLUS && (((	(((( y)->fld[ 1].rtx) )->code)  == REG && (((((( y)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( y)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( y)->fld[ 1].rtx) )->code)  == MULT	&& ((	(((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( y)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( y)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( y)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( y)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( y)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( y)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	(((( y)->fld[ 0].rtx) )->code)  == REG && (((((( y)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ;	if (	(( y)->code)  == PLUS)	{ if (	(((( y)->fld[ 1].rtx) )->code)  == CONST_INT	&& (unsigned) (((( y)->fld[ 1].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( y)->fld[ 0].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ; }	if (	(((( y)->fld[ 0].rtx) )->code)  == CONST_INT	&& (unsigned) (((( y)->fld[ 0].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( y)->fld[ 1].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ; } } } ; } ;
    }
  return 0;

 win:
  if (mode_altering_drug)
    return ! mode_dependent_address_p (((op)->fld[ 0].rtx) );
  return 1;
}

int
address_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return memory_address_p (mode, op);
}

int
register_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if (	((op)->mode)  != mode && mode != VOIDmode)
    return 0;

  if (	((op)->code)  == SUBREG)
    {

      if (! reload_completed)
	return general_operand (op, mode);
    }

  while (	((op)->code)  == SUBREG)
    op = ((op)->fld[0].rtx) ;

  return 	((op)->code)  == REG;
}

int
immediate_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return (((	((op)->code)  == LABEL_REF || 	((op)->code)  == SYMBOL_REF	|| 	((op)->code)  == CONST_INT	|| 	((op)->code)  == CONST) 
	   || (	((op)->code)  == CONST_DOUBLE
	       && (	((op)->mode)  == mode || mode == VOIDmode)))
	  && 1 );
}

int
nonimmediate_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return (general_operand (op, mode)
	  && ! (	((op)->code)  == LABEL_REF || 	((op)->code)  == SYMBOL_REF	|| 	((op)->code)  == CONST_INT	|| 	((op)->code)  == CONST)  && 	((op)->code)  != CONST_DOUBLE);
}

int
nonmemory_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  if ((	((op)->code)  == LABEL_REF || 	((op)->code)  == SYMBOL_REF	|| 	((op)->code)  == CONST_INT	|| 	((op)->code)  == CONST)  || 	((op)->code)  == CONST_DOUBLE)
    return ((	((op)->mode)  == VOIDmode || 	((op)->mode)  == mode)
	    && 1 );

  if (	((op)->mode)  != mode && mode != VOIDmode)
    return 0;

  if (	((op)->code)  == SUBREG)
    {

      if (! reload_completed)
	return general_operand (op, mode);
    }

  while (	((op)->code)  == SUBREG)
    op = ((op)->fld[0].rtx) ;

  return 	((op)->code)  == REG;
}

int
push_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (	((op)->code)  != MEM)
    return 0;

  if (	((op)->mode)  != mode)
    return 0;

  op = ((op)->fld[ 0].rtx) ;

  if (	((op)->code)  != PRE_DEC)
    return 0;

  return ((op)->fld[ 0].rtx)  == stack_pointer_rtx;
}

int
memory_address_p (mode, addr)
     enum machine_mode mode;
     register rtx addr;
{
  { { if (( (	(( addr)->code)  == LABEL_REF || 	(( addr)->code)  == SYMBOL_REF	|| 	(( addr)->code)  == CONST_INT	|| 	(( addr)->code)  == CONST)  	|| (	(( addr)->code)  == REG && (((( addr)->fld[0].rtint)  & ~027) != 0) )	|| ((	(( addr)->code)  == PRE_DEC || 	(( addr)->code)  == POST_INC)	&& (	(((( addr)->fld[ 0].rtx) )->code)  == REG) 	&& (((((( addr)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) )	|| (	(( addr)->code)  == PLUS	&& (	(((( addr)->fld[ 0].rtx) )->code)  == REG)  && (((((( addr)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) 	&& 	(((( addr)->fld[ 1].rtx) )->code)  == CONST_INT	&& ((unsigned) (((( addr)->fld[ 1].rtx) )->fld[0].rtint)  + 0x8000) < 0x10000)) ) goto   win; } ;	{ { if (	(( addr)->code)  == PLUS && (((	(((( addr)->fld[ 0].rtx) )->code)  == REG && (((((( addr)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( addr)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( addr)->fld[ 0].rtx) )->code)  == MULT	&& ((	(((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( addr)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( addr)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( addr)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( addr)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( addr)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( addr)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	(((( addr)->fld[ 1].rtx) )->code)  == REG && (((((( addr)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	(( addr)->code)  == PLUS && (((	(((( addr)->fld[ 1].rtx) )->code)  == REG && (((((( addr)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((( addr)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	(((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	(((( addr)->fld[ 1].rtx) )->code)  == MULT	&& ((	(((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && (((((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	(((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	(((((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	(((((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& (((((((((( addr)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	(((((( addr)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& ((((((( addr)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| (((((( addr)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| (((((( addr)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	(((( addr)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	(((( addr)->fld[ 0].rtx) )->code)  == REG && (((((( addr)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ;	if (	(( addr)->code)  == PLUS)	{ if (	(((( addr)->fld[ 1].rtx) )->code)  == CONST_INT	&& (unsigned) (((( addr)->fld[ 1].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( addr)->fld[ 0].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ; }	if (	(((( addr)->fld[ 0].rtx) )->code)  == CONST_INT	&& (unsigned) (((( addr)->fld[ 0].rtx) )->fld[0].rtint)  + 0x80 < 0x100)	{ rtx go_temp = (( addr)->fld[ 1].rtx) ; { if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 0].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 0].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 1].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; }	if (	((go_temp)->code)  == PLUS && (((	((((go_temp)->fld[ 1].rtx) )->code)  == REG && ((((((go_temp)->fld[ 1].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((go_temp)->fld[ 1].rtx) )->code)  == SIGN_EXTEND	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	|| ((target_flags & 1)  && 	((((go_temp)->fld[ 1].rtx) )->code)  == MULT	&& ((	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == REG && ((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )	|| (	((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->code)  == SIGN_EXTEND	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->code)  == REG	&& 	((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->mode)  == HImode	&& ((((((((((go_temp)->fld[ 1].rtx) )->fld[ 0].rtx) )->fld[ 0].rtx) )->fld[0].rtint)  ^ 020) >= 8) )) 	&& 	((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->code)  == CONST_INT	&& (((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 2	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 4	|| ((((((go_temp)->fld[ 1].rtx) )->fld[ 1].rtx) )->fld[0].rtint)  == 8))) )	{ { if (	((((go_temp)->fld[ 0].rtx) )->code)  == LABEL_REF) goto     win;	if (	((((go_temp)->fld[ 0].rtx) )->code)  == REG && ((((((go_temp)->fld[ 0].rtx) )->fld[0].rtint)  & ~027) != 0) ) goto     win; } ; } } ; } } } ; } ;
  return 0;

 win:
  return 1;
}

int
memory_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  int mode_altering_drug = 0;

  if (! reload_completed)

    return 	((op)->code)  == MEM && general_operand (op, mode);

  while (	((op)->code)  == SUBREG)
    {
      op = ((op)->fld[0].rtx) ;
      mode_altering_drug = 1;
    }

  return (	((op)->code)  == MEM && general_operand (op, mode)
	  && ! (mode_altering_drug
		&& mode_dependent_address_p (((op)->fld[ 0].rtx) )));
}

int
indirect_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return (	((op)->mode)  == mode && memory_operand (op, mode)
	  && general_operand (((op)->fld[ 0].rtx) , SImode ));
}

int
asm_noperands (body)
     rtx body;
{
  if (	((body)->code)  == ASM_OPERANDS)
    return ((body)->fld[ 3].rtvec->num_elem) ;
  if (	((body)->code)  == SET && 	((((body)->fld[1].rtx) )->code)  == ASM_OPERANDS)
    return ((((body)->fld[1].rtx) )->fld[ 3].rtvec->num_elem)  + 1;
  else if (	((body)->code)  == PARALLEL
	   && 	((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->code)  == SET
	   && 	((((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->fld[1].rtx) )->code)  == ASM_OPERANDS)
    {

      int i;
      int n_sets;

      for (i = ((body)->fld[ 0].rtvec->num_elem) ; i > 0; i--)
	{
	  if (	((((body)->fld[ 0].rtvec->elem[ i - 1].rtx) )->code)  == SET)
	    break;
	  if (	((((body)->fld[ 0].rtvec->elem[ i - 1].rtx) )->code)  != CLOBBER)
	    return -1;
	}

      n_sets = i;

      for (i = 0; i < n_sets; i++)
	{
	  rtx elt = ((body)->fld[ 0].rtvec->elem[ i].rtx) ;
	  if (	((elt)->code)  != SET)
	    return -1;
	  if (	((((elt)->fld[1].rtx) )->code)  != ASM_OPERANDS)
	    return -1;

	  if (((((elt)->fld[1].rtx) )->fld[ 3].rtvec) 
	      != ((((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->fld[1].rtx) )->fld[ 3].rtvec) )
	    return -1;
	}
      return ((((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->fld[1].rtx) )->fld[ 3].rtvec->num_elem)  + n_sets;
    }
  else if (	((body)->code)  == PARALLEL
	   && 	((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->code)  == ASM_OPERANDS)
    {

      int i;
      int n_sets;

      for (i = ((body)->fld[ 0].rtvec->num_elem)  - 1; i > 0; i--)
	if (	((((body)->fld[ 0].rtvec->elem[ i].rtx) )->code)  != CLOBBER)
	  return -1;

      return ((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->fld[ 3].rtvec->num_elem) ;
    }
  else
    return -1;
}

char *
decode_asm_operands (body, operands, operand_locs, constraints, modes)
     rtx body;
     rtx *operands;
     rtx **operand_locs;
     char **constraints;
     enum machine_mode *modes;
{
  register int i;
  int noperands;
  char *template = 0;

  if (	((body)->code)  == SET && 	((((body)->fld[1].rtx) )->code)  == ASM_OPERANDS)
    {
      rtx asmop = ((body)->fld[1].rtx) ;

      noperands = ((asmop)->fld[ 3].rtvec->num_elem)  + 1;

      for (i = 1; i < noperands; i++)
	{
	  if (operand_locs)
	    operand_locs[i] = &((asmop)->fld[ 3].rtvec->elem[ i - 1].rtx) ;
	  if (operands)
	    operands[i] = ((asmop)->fld[ 3].rtvec->elem[ i - 1].rtx) ;
	  if (constraints)
	    constraints[i] = ((((asmop)->fld[ 4].rtvec->elem[ i - 1].rtx) )->fld[ 0].rtstr) ;
	  if (modes)
	    modes[i] = 	((((asmop)->fld[ 4].rtvec->elem[ i - 1].rtx) )->mode) ;
	}

      if (operands)
	operands[0] = ((body)->fld[0].rtx) ;
      if (operand_locs)
	operand_locs[0] = &((body)->fld[0].rtx) ;
      if (constraints)
	constraints[0] = ((asmop)->fld[ 1].rtstr) ;
      if (modes)
	modes[0] = 	((((body)->fld[0].rtx) )->mode) ;
      template = ((asmop)->fld[ 0].rtstr) ;
    }
  else if (	((body)->code)  == ASM_OPERANDS)
    {
      rtx asmop = body;

      noperands = ((asmop)->fld[ 3].rtvec->num_elem) ;

      for (i = 0; i < noperands; i++)
	{
	  if (operand_locs)
	    operand_locs[i] = &((asmop)->fld[ 3].rtvec->elem[ i].rtx) ;
	  if (operands)
	    operands[i] = ((asmop)->fld[ 3].rtvec->elem[ i].rtx) ;
	  if (constraints)
	    constraints[i] = ((((asmop)->fld[ 4].rtvec->elem[ i].rtx) )->fld[ 0].rtstr) ;
	  if (modes)
	    modes[i] = 	((((asmop)->fld[ 4].rtvec->elem[ i].rtx) )->mode) ;
	}
      template = ((asmop)->fld[ 0].rtstr) ;
    }
  else if (	((body)->code)  == PARALLEL
	   && 	((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->code)  == SET)
    {
      rtx asmop = ((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->fld[1].rtx) ;
      int nparallel = ((body)->fld[ 0].rtvec->num_elem) ;  
      int nin = ((asmop)->fld[ 3].rtvec->num_elem) ;
      int nout = 0;		 

      for (i = 0; i < nparallel; i++)
	{
	  if (	((((body)->fld[ 0].rtvec->elem[ i].rtx) )->code)  == CLOBBER)
	    break;		 
	  if (operands)
	    operands[i] = ((((body)->fld[ 0].rtvec->elem[ i].rtx) )->fld[0].rtx) ;
	  if (operand_locs)
	    operand_locs[i] = &((((body)->fld[ 0].rtvec->elem[ i].rtx) )->fld[0].rtx) ;
	  if (constraints)
	    constraints[i] = ((((((body)->fld[ 0].rtvec->elem[ i].rtx) )->fld[1].rtx) )->fld[ 1].rtstr) ;
	  if (modes)
	    modes[i] = 	((((((body)->fld[ 0].rtvec->elem[ i].rtx) )->fld[0].rtx) )->mode) ;
	  nout++;
	}

      for (i = 0; i < nin; i++)
	{
	  if (operand_locs)
	    operand_locs[i + nout] = &((asmop)->fld[ 3].rtvec->elem[ i].rtx) ;
	  if (operands)
	    operands[i + nout] = ((asmop)->fld[ 3].rtvec->elem[ i].rtx) ;
	  if (constraints)
	    constraints[i + nout] = ((((asmop)->fld[ 4].rtvec->elem[ i].rtx) )->fld[ 0].rtstr) ;
	  if (modes)
	    modes[i + nout] = 	((((asmop)->fld[ 4].rtvec->elem[ i].rtx) )->mode) ;
	}

      template = ((asmop)->fld[ 0].rtstr) ;
    }
  else if (	((body)->code)  == PARALLEL
	   && 	((((body)->fld[ 0].rtvec->elem[ 0].rtx) )->code)  == ASM_OPERANDS)
    {

      rtx asmop = ((body)->fld[ 0].rtvec->elem[ 0].rtx) ;
      int nin = ((asmop)->fld[ 3].rtvec->num_elem) ;

      for (i = 0; i < nin; i++)
	{
	  if (operand_locs)
	    operand_locs[i] = &((asmop)->fld[ 3].rtvec->elem[ i].rtx) ;
	  if (operands)
	    operands[i] = ((asmop)->fld[ 3].rtvec->elem[ i].rtx) ;
	  if (constraints)
	    constraints[i] = ((((asmop)->fld[ 4].rtvec->elem[ i].rtx) )->fld[ 0].rtstr) ;
	  if (modes)
	    modes[i] = 	((((asmop)->fld[ 4].rtvec->elem[ i].rtx) )->mode) ;
	}

      template = ((asmop)->fld[ 0].rtstr) ;
    }

  return template;
}
extern rtx plus_constant ();
extern rtx copy_rtx ();

static rtx *
find_constant_term_loc (p)
     rtx *p;
{
  register rtx *tem;
  register enum rtx_code code = 	((*p)->code) ;

  if (code == CONST_INT || code == SYMBOL_REF || code == LABEL_REF
      || code == CONST)
    return p;

  if (	((*p)->code)  != PLUS)
    return 0;

  if (((*p)->fld[ 0].rtx)  && (	((((*p)->fld[ 0].rtx) )->code)  == LABEL_REF || 	((((*p)->fld[ 0].rtx) )->code)  == SYMBOL_REF	|| 	((((*p)->fld[ 0].rtx) )->code)  == CONST_INT	|| 	((((*p)->fld[ 0].rtx) )->code)  == CONST) 
      && ((*p)->fld[ 1].rtx)  && (	((((*p)->fld[ 1].rtx) )->code)  == LABEL_REF || 	((((*p)->fld[ 1].rtx) )->code)  == SYMBOL_REF	|| 	((((*p)->fld[ 1].rtx) )->code)  == CONST_INT	|| 	((((*p)->fld[ 1].rtx) )->code)  == CONST) )
    return p;

  if (((*p)->fld[ 0].rtx)  != 0)
    {
      tem = find_constant_term_loc (&((*p)->fld[ 0].rtx) );
      if (tem != 0)
	return tem;
    }

  if (((*p)->fld[ 1].rtx)  != 0)
    {
      tem = find_constant_term_loc (&((*p)->fld[ 1].rtx) );
      if (tem != 0)
	return tem;
    }

  return 0;
}

int
offsetable_memref_p (op)
     rtx op;
{
  return ((	((op)->code)  == MEM)
	  && offsetable_address_p (1, 	((op)->mode) , ((op)->fld[ 0].rtx) ));
}

int
offsetable_address_p (strictp, mode, y)
     int strictp;
     enum machine_mode mode;
     register rtx y;
{
  register enum rtx_code ycode = 	((y)->code) ;
  register rtx z;
  rtx y1 = y;
  rtx *y2;
  int (*addressp) () = (strictp ? strict_memory_address_p : memory_address_p);

  if ( (	((y)->code)  == LABEL_REF || 	((y)->code)  == SYMBOL_REF	|| 	((y)->code)  == CONST_INT	|| 	((y)->code)  == CONST)  )
    return 1;

  if ((ycode == PLUS) && (y2 = find_constant_term_loc (&y1)))
    {
      int old = ((y1 = *y2)->fld[0].rtint) ;
      int good;
      ((y1)->fld[0].rtint)  += 	(mode_size[(int)(mode)])  - 1;
      good = (*addressp) (mode, y);
      ((y1)->fld[0].rtint)  = old;
      return good;
    }

  if (ycode == PRE_DEC || ycode == PRE_INC
      || ycode == POST_DEC || ycode == POST_INC)
    return 0;

  z = plus_constant (y, 	(mode_size[(int)(mode)])  - 1);

  return (*addressp) (mode, z);
}

int
mode_dependent_address_p (addr)
     rtx addr;
{
  if (	((addr)->code)  == POST_INC || 	((addr)->code)  == PRE_DEC) goto  win ;
  return 0;
 win:
  return 1;
}

int
mode_independent_operand (op, mode)
     enum machine_mode mode;
     rtx op;
{
  rtx addr;

  if (! general_operand (op, mode))
    return 0;

  if (	((op)->code)  != MEM)
    return 1;

  addr = ((op)->fld[ 0].rtx) ;
  if (	((addr)->code)  == POST_INC || 	((addr)->code)  == PRE_DEC) goto  lose ;
  return 1;
 lose:
  return 0;
}

rtx
adj_offsetable_operand (op, offset)
     rtx op;
     int offset;
{
  register enum rtx_code code = 	((op)->code) ;

  if (code == MEM) 
    {
      register rtx y = ((op)->fld[ 0].rtx) ;

      if ( (	((y)->code)  == LABEL_REF || 	((y)->code)  == SYMBOL_REF	|| 	((y)->code)  == CONST_INT	|| 	((y)->code)  == CONST)  )
	return gen_rtx (MEM, 	((op)->mode) , plus_constant (y, offset));

      if (	((y)->code)  == PLUS)
	{
	  rtx z = y;
	  register rtx *const_loc;

	  op = copy_rtx (op);
	  z = ((op)->fld[ 0].rtx) ;
	  const_loc = find_constant_term_loc (&z);
	  if (const_loc)
	    {
	      *const_loc = plus_constant (*const_loc, offset);
	      return op;
	    }
	}

      return gen_rtx (MEM, 	((op)->mode) , plus_constant (y, offset));
    }
  abort ();
}

struct funny_match
{
  int this, other;
};

int
constrain_operands (insn_code_num)
     int insn_code_num;
{
  char *constraints[5 ];
  register int c;
  int noperands = insn_n_operands[insn_code_num];

  struct funny_match funny_match[5 ];
  int funny_match_index;
  int nalternatives = insn_n_alternatives[insn_code_num];

  if (noperands == 0 || nalternatives == 0)
    return 1;

  for (c = 0; c < noperands; c++)
    constraints[c] = insn_operand_constraint[insn_code_num][c];

  which_alternative = 0;

  while (which_alternative < nalternatives)
    {
      register int opno;
      int lose = 0;
      funny_match_index = 0;

      for (opno = 0; opno < noperands; opno++)
	{
	  register rtx op = recog_operand[opno];
	  register char *p = constraints[opno];
	  int win = 0;
	  int val;

	  while (	((op)->code)  == SUBREG)
	    abort ();

	  if (*p == 0 || *p == ',')
	    win = 1;

	  while (*p && (c = *p++) != ',')
	    switch (c)
	      {
	      case '=':
	      case '+':
	      case '?':
	      case '#':
	      case '!':
	      case '*':
	      case '%':
		break;

	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':

		val = operands_match_p (recog_operand[c - '0'],
					recog_operand[opno]);
		if (val != 0)
		  win = 1;

		if (val == 2)
		  {
		    funny_match[funny_match_index].this = opno;
		    funny_match[funny_match_index++].other = c - '0';
		  }
		break;

	      case 'p':

		win = 1;
		break;

	      case 'g':

		if (GENERAL_REGS == ALL_REGS
		    || 	((op)->code)  != REG
		    || reg_fits_class_p (op, GENERAL_REGS, 0, 	((op)->mode) ))
		  win = 1;
		break;

	      case 'r':
		if (	((op)->code)  == REG
		    && (GENERAL_REGS == ALL_REGS
			|| reg_fits_class_p (op, GENERAL_REGS, 0, 	((op)->mode) )))
		  win = 1;
		break;

	      case 'm':
		if (	((op)->code)  == MEM)
		  win = 1;
		break;

	      case '<':
		if (	((op)->code)  == MEM
		    && (	((((op)->fld[ 0].rtx) )->code)  == PRE_DEC
			|| 	((((op)->fld[ 0].rtx) )->code)  == POST_DEC))
		  win = 1;
		break;

	      case '>':
		if (	((op)->code)  == MEM
		    && (	((((op)->fld[ 0].rtx) )->code)  == PRE_INC
			|| 	((((op)->fld[ 0].rtx) )->code)  == POST_INC))
		  win = 1;
		break;

	      case 'F':
		if (	((op)->code)  == CONST_DOUBLE)
		  win = 1;
		break;

	      case 'G':
	      case 'H':
		if (	((op)->code)  == CONST_DOUBLE
		    && (( c) == 'G' ? ! ((target_flags & 2)  && standard_68881_constant_p (op)) : ( c) == 'H' ? ((target_flags & 0100)  && standard_sun_fpa_constant_p (op)) : 0) )
		  win = 1;
		break;

	      case 's':
		if (	((op)->code)  == CONST_INT)
		  break;
	      case 'i':
		if ((	((op)->code)  == LABEL_REF || 	((op)->code)  == SYMBOL_REF	|| 	((op)->code)  == CONST_INT	|| 	((op)->code)  == CONST) )
		  win = 1;
		break;

	      case 'n':
		if (	((op)->code)  == CONST_INT)
		  win = 1;
		break;

	      case 'I':
	      case 'J':
	      case 'K':
	      case 'L':
	      case 'M':
		if (	((op)->code)  == CONST_INT
		    && (( c) == 'I' ? (((op)->fld[0].rtint) ) > 0 && (((op)->fld[0].rtint) ) <= 8 : ( c) == 'J' ? (((op)->fld[0].rtint) ) >= -0x8000 && (((op)->fld[0].rtint) ) <= 0x7FFF :	( c) == 'K' ? (((op)->fld[0].rtint) ) < -0x80 || (((op)->fld[0].rtint) ) >= 0x80 :	( c) == 'L' ? (((op)->fld[0].rtint) ) < 0 && (((op)->fld[0].rtint) ) >= -8 : 0) )
		  win = 1;
		break;

	      case 'o':
		if (offsetable_memref_p (op))
		  win = 1;
		break;

	      default:
		if (	((op)->code)  == REG
		    && reg_fits_class_p (op, ((c) == 'a' ? ADDR_REGS :	((c) == 'd' ? DATA_REGS :	((c) == 'f' ? ((target_flags & 2)  ? FP_REGS :	NO_REGS) :	((c) == 'x' ? ((target_flags & 0100)  ? FPA_REGS :	NO_REGS) :	((c) == 'y' ? ((target_flags & 0100)  ? LO_FPA_REGS :	NO_REGS) :	NO_REGS))))) ,
					 0, 	((op)->mode) ))
		  win = 1;
	      }

	  constraints[opno] = p;

	  if (! win)
	    lose = 1;
	}

      if (! lose)
	{
	  while (--funny_match_index >= 0)
	    {
	      recog_operand[funny_match[funny_match_index].other]
		= recog_operand[funny_match[funny_match_index].this];
	    }
	  return 1;
	}

      which_alternative++;
    }
  return 0;
}

int
reg_fits_class_p (operand, class, offset, mode)
     rtx operand;
     register enum reg_class class;
     int offset;
     enum machine_mode mode;
{
  register int regno = ((operand)->fld[0].rtint) ;
  if (regno < 56 
      && ((reg_class_contents[(int) class])[(
			    regno + offset) / 32 ] & (1 << ((
			    regno + offset) % 32 ))) )
    {
      register int sr;
      regno += offset;
      for (sr = ((regno) >= 16 ? 1	: ((	(mode_size[(int)( mode)])  + 4  - 1) / 4 ))  - 1;
	   sr > 0; sr--)
	if (! ((reg_class_contents[(int) class])[(
				 regno + sr) / 32 ] & (1 << ((
				 regno + sr) % 32 ))) )
	  break;
      return sr == 0;
    }
  return 0;
}


