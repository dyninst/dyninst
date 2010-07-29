
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

rtx recog_operand[5 ];

rtx *recog_operand_loc[5 ];

rtx *recog_dup_loc[3 ];

char recog_dup_num[3 ];

extern rtx recog_addr_dummy;

int
recog_1 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L8:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, SImode))
    { recog_operand[0] = x1; return 2; }
 L11:
  if (general_operand (x1, HImode))
    { recog_operand[0] = x1; return 3; }
 L14:
  if (general_operand (x1, QImode))
    { recog_operand[0] = x1; return 4; }
 L23:
  if (general_operand (x1, SFmode))
    { recog_operand[0] = x1; if ((target_flags & 2) ) return 7; }
 L32:
  if (general_operand (x1, DFmode))
    { recog_operand[0] = x1; if ((target_flags & 2) ) return 10; }
 L35:
  if (	((x1)->code)  == COMPARE && 1)
    goto L36;
  if (	((x1)->code)  == ZERO_EXTRACT && 1)
    goto L77;
  if (	((x1)->code)  == SUBREG && 	((x1)->mode)  == SImode && ((x1)->fld[ 1].rtint)  == 0 && 1)
    goto L131;
  if (	((x1)->code)  == AND && 	((x1)->mode)  == SImode && 1)
    goto L137;
 L882:
  if (	((x1)->code)  == ZERO_EXTRACT && 	((x1)->mode)  == SImode && 1)
    goto L883;
  if (	((x1)->code)  != SUBREG)
    goto ret0;
  if (	((x1)->mode)  == QImode && ((x1)->fld[ 1].rtint)  == 0 && 1)
    goto L889;
  if (	((x1)->mode)  == HImode && ((x1)->fld[ 1].rtint)  == 0 && 1)
    goto L896;
  goto ret0;
 L36:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[0] = x2; goto L37; }
 L41:
  if (general_operand (x2, HImode))
    { recog_operand[0] = x2; goto L42; }
 L46:
  if (general_operand (x2, QImode))
    { recog_operand[0] = x2; goto L47; }
 L59:
  if (general_operand (x2, DFmode))
    { recog_operand[0] = x2; goto L60; }
 L72:
  if (general_operand (x2, SFmode))
    { recog_operand[0] = x2; goto L73; }
 L676:
  if (general_operand (x2, QImode))
    { recog_operand[0] = x2; goto L677; }
 L683:
  if (	((x2)->code)  == LSHIFTRT && 	((x2)->mode)  == SImode && 1)
    goto L684;
 L690:
  if (general_operand (x2, QImode))
    { recog_operand[0] = x2; goto L691; }
 L697:
  if (	((x2)->code)  == ASHIFTRT && 	((x2)->mode)  == SImode && 1)
    goto L698;
  goto ret0;
 L37:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; return 11; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L41;
 L42:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 12; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L46;
 L47:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 13; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L59;
 L60:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 16; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L72;
 L73:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 19; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L676;
 L677:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LSHIFTRT && 	((x2)->mode)  == SImode && 1)
    goto L678;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L683;
 L678:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (memory_operand (x3, SImode))
    { recog_operand[1] = x3; goto L679; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L683;
 L679:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 24 && 1)
    if ((	((recog_operand [0])->code)  == CONST_INT
    && (((recog_operand [0])->fld[0].rtint)  & ~0xff) == 0)) return 163;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L683;
 L684:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (memory_operand (x3, SImode))
    { recog_operand[0] = x3; goto L685; }
  goto L690;
 L685:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 24 && 1)
    goto L686;
  goto L690;
 L686:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; if ((	((recog_operand [1])->code)  == CONST_INT
    && (((recog_operand [1])->fld[0].rtint)  & ~0xff) == 0)) return 164; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L690;
 L691:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == ASHIFTRT && 	((x2)->mode)  == SImode && 1)
    goto L692;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L697;
 L692:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (memory_operand (x3, SImode))
    { recog_operand[1] = x3; goto L693; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L697;
 L693:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 24 && 1)
    if ((	((recog_operand [0])->code)  == CONST_INT
    && ((((recog_operand [0])->fld[0].rtint)  + 0x80) & ~0xff) == 0)) return 165;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L697;
 L698:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (memory_operand (x3, SImode))
    { recog_operand[0] = x3; goto L699; }
  goto ret0;
 L699:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 24 && 1)
    goto L700;
  goto ret0;
 L700:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; if ((	((recog_operand [1])->code)  == CONST_INT
    && ((((recog_operand [1])->fld[0].rtint)  + 0x80) & ~0xff) == 0)) return 166; }
  goto ret0;
 L77:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (nonimmediate_operand (x2, QImode))
    { recog_operand[0] = x2; goto L78; }
 L119:
  if (nonimmediate_operand (x2, HImode))
    { recog_operand[0] = x2; goto L120; }
 L85:
  if (nonimmediate_operand (x2, SImode))
    { recog_operand[0] = x2; goto L86; }
  goto L882;
 L78:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const1_rtx && 1)
    goto L79;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L119;
 L79:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == MINUS && 	((x2)->mode)  == SImode && 1)
    goto L80;
 L115:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if (	((recog_operand [1])->code)  == CONST_INT) return 24; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L119;
 L80:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 7 && 1)
    goto L81;
  goto L115;
 L81:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; return 20; }
 L97:
  if (	((x3)->code)  == AND && 	((x3)->mode)  == SImode && 1)
    goto L98;
  goto L115;
 L98:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (general_operand (x4, SImode))
    { recog_operand[1] = x4; goto L99; }
  goto L115;
 L99:
  x4 = ((x3)->fld[ 1].rtx) ;
  if (	((x4)->code)  == CONST_INT && ((x4)->fld[ 0].rtint)  == 7 && 1)
    return 22;
  goto L115;
 L120:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const1_rtx && 1)
    goto L121;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L85;
 L121:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if (	((recog_operand [1])->code)  == CONST_INT) return 25; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L85;
 L86:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const1_rtx && 1)
    goto L87;
  goto L882;
 L87:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == MINUS && 	((x2)->mode)  == SImode && 1)
    goto L88;
 L127:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if (	((recog_operand [1])->code)  == CONST_INT) return 26; }
  goto L882;
 L88:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 31 && 1)
    goto L89;
  goto L127;
 L89:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; return 21; }
 L107:
  if (	((x3)->code)  == AND && 	((x3)->mode)  == SImode && 1)
    goto L108;
  goto L127;
 L108:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (general_operand (x4, SImode))
    { recog_operand[1] = x4; goto L109; }
  goto L127;
 L109:
  x4 = ((x3)->fld[ 1].rtx) ;
  if (	((x4)->code)  == CONST_INT && ((x4)->fld[ 0].rtint)  == 31 && 1)
    return 23;
  goto L127;
 L131:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == LSHIFTRT && 	((x2)->mode)  == QImode && 1)
    goto L132;
  goto ret0;
 L132:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (nonimmediate_operand (x3, QImode))
    { recog_operand[0] = x3; goto L133; }
  goto ret0;
 L133:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == 7 && 1)
    return 27;
  goto ret0;
 L137:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == SIGN_EXTEND && 	((x2)->mode)  == SImode && 1)
    goto L138;
  goto ret0;
 L138:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (	((x3)->code)  == SIGN_EXTEND && 	((x3)->mode)  == HImode && 1)
    goto L139;
  goto ret0;
 L139:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (nonimmediate_operand (x4, QImode))
    { recog_operand[0] = x4; goto L140; }
  goto ret0;
 L140:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((	((recog_operand [1])->code)  == CONST_INT
    && (unsigned) ((recog_operand [1])->fld[0].rtint)  < 0x100
    && exact_log2 (((recog_operand [1])->fld[0].rtint) ) >= 0)) return 28; }
  goto ret0;
 L883:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (memory_operand (x2, QImode))
    { recog_operand[0] = x2; goto L884; }
 L903:
  if (nonimmediate_operand (x2, SImode))
    { recog_operand[0] = x2; goto L904; }
  goto ret0;
 L884:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L885; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L903;
 L885:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT) return 199; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L903;
 L904:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L905; }
  goto ret0;
 L905:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT) return 202; }
  goto ret0;
 L889:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == ZERO_EXTRACT && 	((x2)->mode)  == SImode && 1)
    goto L890;
  goto ret0;
 L890:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (memory_operand (x3, QImode))
    { recog_operand[0] = x3; goto L891; }
 L910:
  if (nonimmediate_operand (x3, SImode))
    { recog_operand[0] = x3; goto L911; }
  goto ret0;
 L891:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; goto L892; }
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L910;
 L892:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[2] = x3; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT) return 200; }
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L910;
 L911:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; goto L912; }
  goto ret0;
 L912:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[2] = x3; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT) return 203; }
  goto ret0;
 L896:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == ZERO_EXTRACT && 	((x2)->mode)  == SImode && 1)
    goto L897;
  goto ret0;
 L897:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (memory_operand (x3, QImode))
    { recog_operand[0] = x3; goto L898; }
 L917:
  if (nonimmediate_operand (x3, SImode))
    { recog_operand[0] = x3; goto L918; }
  goto ret0;
 L898:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; goto L899; }
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L917;
 L899:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[2] = x3; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT) return 201; }
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L917;
 L918:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; goto L919; }
  goto ret0;
 L919:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[2] = x3; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT) return 204; }
  goto ret0;
 ret0: return -1;
}

int
recog_2 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L973:
  x1 = ((x0)->fld[ 1].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
 switch (	((x2)->code) )
  {
  case EQ:
  if (1)
    goto L974;
  break;
  case NE:
  if (1)
    goto L983;
  break;
  case GT:
  if (1)
    goto L992;
  break;
  case GTU:
  if (1)
    goto L1001;
  break;
  case LT:
  if (1)
    goto L1010;
  break;
  case LTU:
  if (1)
    goto L1019;
  break;
  case GE:
  if (1)
    goto L1028;
  break;
  case GEU:
  if (1)
    goto L1037;
  break;
  case LE:
  if (1)
    goto L1046;
  break;
  case LEU:
  if (1)
    goto L1055;
  break;
  }
  goto ret0;
 L974:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L975;
  goto ret0;
 L975:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L976;
  goto ret0;
 L976:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L977;
  if (x2 == pc_rtx && 1)
    goto L1067;
  goto ret0;
 L977:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L978; }
  goto ret0;
 L978:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 215;
  goto ret0;
 L1067:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1068;
  goto ret0;
 L1068:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 225; }
  goto ret0;
 L983:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L984;
  goto ret0;
 L984:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L985;
  goto ret0;
 L985:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L986;
  if (x2 == pc_rtx && 1)
    goto L1076;
  goto ret0;
 L986:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L987; }
  goto ret0;
 L987:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 216;
  goto ret0;
 L1076:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1077;
  goto ret0;
 L1077:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 226; }
  goto ret0;
 L992:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L993;
  goto ret0;
 L993:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L994;
  goto ret0;
 L994:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L995;
  if (x2 == pc_rtx && 1)
    goto L1085;
  goto ret0;
 L995:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L996; }
  goto ret0;
 L996:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 217;
  goto ret0;
 L1085:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1086;
  goto ret0;
 L1086:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 227; }
  goto ret0;
 L1001:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1002;
  goto ret0;
 L1002:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1003;
  goto ret0;
 L1003:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1004;
  if (x2 == pc_rtx && 1)
    goto L1094;
  goto ret0;
 L1004:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1005; }
  goto ret0;
 L1005:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 218;
  goto ret0;
 L1094:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1095;
  goto ret0;
 L1095:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 228; }
  goto ret0;
 L1010:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1011;
  goto ret0;
 L1011:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1012;
  goto ret0;
 L1012:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1013;
  if (x2 == pc_rtx && 1)
    goto L1103;
  goto ret0;
 L1013:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1014; }
  goto ret0;
 L1014:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 219;
  goto ret0;
 L1103:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1104;
  goto ret0;
 L1104:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 229; }
  goto ret0;
 L1019:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1020;
  goto ret0;
 L1020:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1021;
  goto ret0;
 L1021:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1022;
  if (x2 == pc_rtx && 1)
    goto L1112;
  goto ret0;
 L1022:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1023; }
  goto ret0;
 L1023:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 220;
  goto ret0;
 L1112:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1113;
  goto ret0;
 L1113:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 230; }
  goto ret0;
 L1028:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1029;
  goto ret0;
 L1029:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1030;
  goto ret0;
 L1030:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1031;
  if (x2 == pc_rtx && 1)
    goto L1121;
  goto ret0;
 L1031:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1032; }
  goto ret0;
 L1032:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 221;
  goto ret0;
 L1121:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1122;
  goto ret0;
 L1122:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 231; }
  goto ret0;
 L1037:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1038;
  goto ret0;
 L1038:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1039;
  goto ret0;
 L1039:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1040;
  if (x2 == pc_rtx && 1)
    goto L1130;
  goto ret0;
 L1040:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1041; }
  goto ret0;
 L1041:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 222;
  goto ret0;
 L1130:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1131;
  goto ret0;
 L1131:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 232; }
  goto ret0;
 L1046:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1047;
  goto ret0;
 L1047:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1048;
  goto ret0;
 L1048:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1049;
  if (x2 == pc_rtx && 1)
    goto L1139;
  goto ret0;
 L1049:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1050; }
  goto ret0;
 L1050:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 223;
  goto ret0;
 L1139:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1140;
  goto ret0;
 L1140:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 233; }
  goto ret0;
 L1055:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == cc0_rtx && 1)
    goto L1056;
  goto ret0;
 L1056:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (x3 == const0_rtx && 1)
    goto L1057;
  goto ret0;
 L1057:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1058;
  if (x2 == pc_rtx && 1)
    goto L1148;
  goto ret0;
 L1058:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; goto L1059; }
  goto ret0;
 L1059:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (x2 == pc_rtx && 1)
    return 224;
  goto ret0;
 L1148:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1149;
  goto ret0;
 L1149:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x3; return 234; }
  goto ret0;
 ret0: return -1;
}

int
recog_3 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L159:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, QImode))
    { recog_operand[1] = x1; return 34; }
 L178:
 switch (	((x1)->code) )
  {
  case TRUNCATE:
  if (	((x1)->mode)  == QImode && 1)
    goto L179;
  break;
  case FIX:
  if (	((x1)->mode)  == QImode && 1)
    goto L271;
  break;
  case PLUS:
  if (	((x1)->mode)  == QImode && 1)
    goto L327;
  break;
  case MINUS:
  if (	((x1)->mode)  == QImode && 1)
    goto L380;
  break;
  case AND:
  if (	((x1)->mode)  == QImode && 1)
    goto L563;
  break;
  case IOR:
  if (	((x1)->mode)  == QImode && 1)
    goto L590;
  break;
  case XOR:
  if (	((x1)->mode)  == QImode && 1)
    goto L605;
  break;
  case NEG:
  if (	((x1)->mode)  == QImode && 1)
    goto L618;
  break;
  case NOT:
  if (	((x1)->mode)  == QImode && 1)
    goto L662;
  break;
  case ASHIFT:
  if (	((x1)->mode)  == QImode && 1)
    goto L714;
  break;
  case ASHIFTRT:
  if (	((x1)->mode)  == QImode && 1)
    goto L729;
  break;
  case LSHIFT:
  if (	((x1)->mode)  == QImode && 1)
    goto L744;
  break;
  case LSHIFTRT:
  if (	((x1)->mode)  == QImode && 1)
    goto L759;
  break;
  case ROTATE:
  if (	((x1)->mode)  == QImode && 1)
    goto L774;
  break;
  case ROTATERT:
  if (	((x1)->mode)  == QImode && 1)
    goto L789;
  break;
  case EQ:
  if (1)
    goto L923;
  break;
  case NE:
  if (1)
    goto L928;
  break;
  case GT:
  if (1)
    goto L933;
  break;
  case GTU:
  if (1)
    goto L938;
  break;
  case LT:
  if (1)
    goto L943;
  break;
  case LTU:
  if (1)
    goto L948;
  break;
  case GE:
  if (1)
    goto L953;
  break;
  case GEU:
  if (1)
    goto L958;
  break;
  case LE:
  if (1)
    goto L963;
  break;
  case LEU:
  if (1)
    goto L968;
  break;
  }
  goto ret0;
 L179:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; return 40; }
 L183:
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 41; }
  goto ret0;
 L271:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 70; }
 L283:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 73; }
  goto ret0;
 L327:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L328; }
  goto ret0;
 L328:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 82; }
  goto ret0;
 L380:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L381; }
  goto ret0;
 L381:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 94; }
  goto ret0;
 L563:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L564; }
  goto ret0;
 L564:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 134; }
  goto ret0;
 L590:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L591; }
  goto ret0;
 L591:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 139; }
  goto ret0;
 L605:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L606; }
  goto ret0;
 L606:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 142; }
  goto ret0;
 L618:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 145; }
  goto ret0;
 L662:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 160; }
  goto ret0;
 L714:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L715; }
  goto ret0;
 L715:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 169; }
  goto ret0;
 L729:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L730; }
  goto ret0;
 L730:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 172; }
  goto ret0;
 L744:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L745; }
  goto ret0;
 L745:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 175; }
  goto ret0;
 L759:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L760; }
  goto ret0;
 L760:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 178; }
  goto ret0;
 L774:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L775; }
  goto ret0;
 L775:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 181; }
  goto ret0;
 L789:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L790; }
  goto ret0;
 L790:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[2] = x2; return 184; }
  goto ret0;
 L923:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L924;
  goto ret0;
 L924:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 205;
  goto ret0;
 L928:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L929;
  goto ret0;
 L929:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 206;
  goto ret0;
 L933:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L934;
  goto ret0;
 L934:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 207;
  goto ret0;
 L938:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L939;
  goto ret0;
 L939:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 208;
  goto ret0;
 L943:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L944;
  goto ret0;
 L944:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 209;
  goto ret0;
 L948:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L949;
  goto ret0;
 L949:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 210;
  goto ret0;
 L953:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L954;
  goto ret0;
 L954:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 211;
  goto ret0;
 L958:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L959;
  goto ret0;
 L959:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 212;
  goto ret0;
 L963:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L964;
  goto ret0;
 L964:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 213;
  goto ret0;
 L968:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L969;
  goto ret0;
 L969:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (x2 == const0_rtx && 1)
    return 214;
  goto ret0;
 ret0: return -1;
}

int
recog_4 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L152:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, HImode))
    { recog_operand[1] = x1; return 32; }
 L186:
  if (	((x1)->mode)  != HImode)
    goto ret0;
 switch (	((x1)->code) )
  {
  case TRUNCATE:
  if (1)
    goto L187;
  break;
  case ZERO_EXTEND:
  if (1)
    goto L195;
  break;
  case SIGN_EXTEND:
  if (1)
    goto L207;
  break;
  case FIX:
  if (1)
    goto L275;
  break;
  case PLUS:
  if (1)
    goto L316;
  break;
  case MINUS:
  if (1)
    goto L369;
  break;
  case MULT:
  if (1)
    goto L411;
  break;
  case UMULT:
  if (1)
    goto L426;
  break;
  case DIV:
  if (1)
    goto L461;
  break;
  case UDIV:
  if (1)
    goto L476;
  break;
  case MOD:
  if (1)
    goto L511;
  break;
  case UMOD:
  if (1)
    goto L521;
  break;
  case AND:
  if (1)
    goto L558;
  break;
  case IOR:
  if (1)
    goto L585;
  break;
  case XOR:
  if (1)
    goto L600;
  break;
  case NEG:
  if (1)
    goto L614;
  break;
  case NOT:
  if (1)
    goto L658;
  break;
  case ASHIFT:
  if (1)
    goto L709;
  break;
  case ASHIFTRT:
  if (1)
    goto L724;
  break;
  case LSHIFT:
  if (1)
    goto L739;
  break;
  case LSHIFTRT:
  if (1)
    goto L754;
  break;
  case ROTATE:
  if (1)
    goto L769;
  break;
  case ROTATERT:
  if (1)
    goto L784;
  break;
  }
  goto ret0;
 L187:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; return 42; }
  goto ret0;
 L195:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 47; }
  goto ret0;
 L207:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 50; }
  goto ret0;
 L275:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 71; }
 L287:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 74; }
  goto ret0;
 L316:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L317; }
  goto ret0;
 L317:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 80; }
  goto ret0;
 L369:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L370; }
  goto ret0;
 L370:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 92; }
  goto ret0;
 L411:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L412; }
  goto ret0;
 L412:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 102; }
  goto ret0;
 L426:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L427; }
  goto ret0;
 L427:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 105; }
  goto ret0;
 L461:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L462; }
 L466:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L467; }
  goto ret0;
 L462:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 114; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L466;
 L467:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 115; }
  goto ret0;
 L476:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L477; }
 L481:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L482; }
  goto ret0;
 L477:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 117; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L481;
 L482:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 118; }
  goto ret0;
 L511:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L512; }
 L516:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L517; }
  goto ret0;
 L512:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 126; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L516;
 L517:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 127; }
  goto ret0;
 L521:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L522; }
 L526:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L527; }
  goto ret0;
 L522:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 128; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L526;
 L527:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 129; }
  goto ret0;
 L558:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L559; }
  goto ret0;
 L559:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 133; }
  goto ret0;
 L585:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L586; }
  goto ret0;
 L586:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 138; }
  goto ret0;
 L600:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L601; }
  goto ret0;
 L601:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 141; }
  goto ret0;
 L614:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 144; }
  goto ret0;
 L658:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 159; }
  goto ret0;
 L709:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L710; }
  goto ret0;
 L710:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 168; }
  goto ret0;
 L724:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L725; }
  goto ret0;
 L725:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 171; }
  goto ret0;
 L739:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L740; }
  goto ret0;
 L740:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 174; }
  goto ret0;
 L754:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L755; }
  goto ret0;
 L755:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 177; }
  goto ret0;
 L769:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L770; }
  goto ret0;
 L770:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 180; }
  goto ret0;
 L784:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L785; }
  goto ret0;
 L785:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 183; }
  goto ret0;
 ret0: return -1;
}

int
recog_5 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L190:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->mode)  != SImode)
    goto ret0;
 switch (	((x1)->code) )
  {
  case ZERO_EXTEND:
  if (1)
    goto L191;
  break;
  case SIGN_EXTEND:
  if (1)
    goto L203;
  break;
  case FIX:
  if (1)
    goto L279;
  break;
  case PLUS:
  if (1)
    goto L305;
  break;
  case MINUS:
  if (1)
    goto L358;
  break;
  case MULT:
  if (1)
    goto L416;
  break;
  case UMULT:
  if (1)
    goto L431;
  break;
  case DIV:
  if (1)
    goto L471;
  break;
  case UDIV:
  if (1)
    goto L486;
  break;
  case AND:
  if (1)
    goto L553;
  break;
  case IOR:
  if (1)
    goto L580;
  break;
  case XOR:
  if (1)
    goto L595;
  break;
  case NEG:
  if (1)
    goto L610;
  break;
  case NOT:
  if (1)
    goto L654;
  break;
  case ASHIFTRT:
  if (1)
    goto L666;
  break;
  case LSHIFTRT:
  if (1)
    goto L671;
  break;
  case ASHIFT:
  if (1)
    goto L704;
  break;
  case LSHIFT:
  if (1)
    goto L734;
  break;
  case ROTATE:
  if (1)
    goto L764;
  break;
  case ROTATERT:
  if (1)
    goto L779;
  break;
  }
  goto ret0;
 L191:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 46; }
 L199:
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 48; }
  goto ret0;
 L203:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 49; }
 L211:
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; if ((target_flags & 1) ) return 51; }
  goto ret0;
 L279:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 72; }
 L291:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 75; }
 L295:
  if (	((x2)->code)  != FIX)
    goto ret0;
  if (	((x2)->mode)  == SFmode && 1)
    goto L296;
  if (	((x2)->mode)  == DFmode && 1)
    goto L301;
  goto ret0;
 L296:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[1] = x3; if ((target_flags & 0100) ) return 76; }
  goto ret0;
 L301:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[1] = x3; if ((target_flags & 0100) ) return 77; }
  goto ret0;
 L305:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L306; }
  goto ret0;
 L306:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 78; }
 L311:
  if (	((x2)->code)  == SIGN_EXTEND && 	((x2)->mode)  == SImode && 1)
    goto L312;
  goto ret0;
 L312:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, HImode))
    { recog_operand[2] = x3; return 79; }
  goto ret0;
 L358:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L359; }
  goto ret0;
 L359:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 90; }
 L364:
  if (	((x2)->code)  == SIGN_EXTEND && 	((x2)->mode)  == SImode && 1)
    goto L365;
  goto ret0;
 L365:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, HImode))
    { recog_operand[2] = x3; return 91; }
  goto ret0;
 L416:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L417; }
 L421:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L422; }
  goto ret0;
 L417:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 103; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L421;
 L422:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 1) ) return 104; }
  goto ret0;
 L431:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; goto L432; }
 L436:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L437; }
  goto ret0;
 L432:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[2] = x2; return 106; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L436;
 L437:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 1) ) return 107; }
  goto ret0;
 L471:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L472; }
  goto ret0;
 L472:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 1) ) return 116; }
  goto ret0;
 L486:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L487; }
  goto ret0;
 L487:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 1) ) return 119; }
  goto ret0;
 L553:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L554; }
 L568:
  if (	((x2)->code)  == ZERO_EXTEND && 	((x2)->mode)  == SImode && 1)
    goto L569;
  goto ret0;
 L554:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 132; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L568;
 L569:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, HImode))
    { recog_operand[1] = x3; goto L570; }
 L575:
  if (general_operand (x3, QImode))
    { recog_operand[1] = x3; goto L576; }
  goto ret0;
 L570:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if (	((recog_operand [2])->code)  == CONST_INT
   && (unsigned int) ((recog_operand [2])->fld[0].rtint)  < (1 <<  (8  * mode_size[(int)(HImode)]) )) return 135; }
  x2 = ((x1)->fld[ 0].rtx) ;
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L575;
 L576:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if (	((recog_operand [2])->code)  == CONST_INT
   && (unsigned int) ((recog_operand [2])->fld[0].rtint)  < (1 <<  (8  * mode_size[(int)(QImode)]) )) return 136; }
  goto ret0;
 L580:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L581; }
  goto ret0;
 L581:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 137; }
  goto ret0;
 L595:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L596; }
  goto ret0;
 L596:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 140; }
  goto ret0;
 L610:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; return 143; }
  goto ret0;
 L654:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; return 158; }
  goto ret0;
 L666:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (memory_operand (x2, SImode))
    { recog_operand[1] = x2; goto L667; }
 L719:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L720; }
  goto ret0;
 L667:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == CONST_INT && ((x2)->fld[ 0].rtint)  == 24 && 1)
    if (	((((recog_operand [1])->fld[ 0].rtx) )->code)  != POST_INC
   && 	((((recog_operand [1])->fld[ 0].rtx) )->code)  != PRE_DEC) return 161;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L719;
 L720:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 170; }
  goto ret0;
 L671:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (memory_operand (x2, SImode))
    { recog_operand[1] = x2; goto L672; }
 L749:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L750; }
  goto ret0;
 L672:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == CONST_INT && ((x2)->fld[ 0].rtint)  == 24 && 1)
    if (	((((recog_operand [1])->fld[ 0].rtx) )->code)  != POST_INC
   && 	((((recog_operand [1])->fld[ 0].rtx) )->code)  != PRE_DEC) return 162;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L749;
 L750:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 176; }
  goto ret0;
 L704:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L705; }
  goto ret0;
 L705:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 167; }
  goto ret0;
 L734:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L735; }
  goto ret0;
 L735:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 173; }
  goto ret0;
 L764:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L765; }
  goto ret0;
 L765:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 179; }
  goto ret0;
 L779:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L780; }
  goto ret0;
 L780:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 182; }
  goto ret0;
 ret0: return -1;
}

int
recog_6 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L7:
  x1 = ((x0)->fld[ 0].rtx) ;
  if (x1 == cc0_rtx && 1)
    goto L8;
  if (	((x1)->code)  == STRICT_LOW_PART && 1)
    goto L162;
  if (x1 == pc_rtx && 1)
    goto L972;
 L1222:
  if (1)
    { recog_operand[0] = x1; goto L1223; }
 L158:
 switch (	((x1)->mode) )
  {
  case QImode:
  if (general_operand (x1, QImode))
    { recog_operand[0] = x1; goto L159; }
  break;
 L151:
  case HImode:
  if (general_operand (x1, HImode))
    { recog_operand[0] = x1; goto L152; }
  break;
  case SImode:
  if (push_operand (x1, SImode))
    { recog_operand[0] = x1; goto L143; }
 L145:
  if (general_operand (x1, SImode))
    { recog_operand[0] = x1; goto L146; }
 L174:
  if (push_operand (x1, SImode))
    { recog_operand[0] = x1; goto L175; }
 L189:
  if (general_operand (x1, SImode))
    { recog_operand[0] = x1; goto L190; }
 L792:
  if (	((x1)->code)  == ZERO_EXTRACT && 1)
    goto L823;
 L798:
  if (general_operand (x1, SImode))
    { recog_operand[0] = x1; goto L799; }
  break;
  case DImode:
  if (push_operand (x1, DImode))
    { recog_operand[0] = x1; goto L5; }
 L171:
  if (general_operand (x1, DImode))
    { recog_operand[0] = x1; goto L172; }
  break;
  case SFmode:
  if (general_operand (x1, SFmode))
    { recog_operand[0] = x1; goto L166; }
 L221:
  if (register_operand (x1, SFmode))
    { recog_operand[0] = x1; goto L222; }
 L225:
  if (general_operand (x1, SFmode))
    { recog_operand[0] = x1; goto L226; }
 L1245:
  if (register_operand (x1, SFmode))
    { recog_operand[0] = x1; goto L1246; }
  break;
  case DFmode:
  if (push_operand (x1, DFmode))
    { recog_operand[0] = x1; goto L2; }
 L168:
  if (general_operand (x1, DFmode))
    { recog_operand[0] = x1; goto L169; }
 L237:
  if (register_operand (x1, DFmode))
    { recog_operand[0] = x1; goto L238; }
 L241:
  if (general_operand (x1, DFmode))
    { recog_operand[0] = x1; goto L242; }
 L1231:
  if (register_operand (x1, DFmode))
    { recog_operand[0] = x1; goto L1232; }
  break;
  }
  goto ret0;
 L8:
  tem = recog_1 (x0, insn);
  if (tem >= 0) return tem;
  goto L1222;
 L162:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[0] = x2; goto L163; }
 L155:
  if (general_operand (x2, HImode))
    { recog_operand[0] = x2; goto L156; }
  goto L1222;
 L163:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, QImode))
    { recog_operand[1] = x1; return 35; }
 L332:
  if (	((x1)->mode)  != QImode)
    {
      x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
      goto L155;
    }
  if (	((x1)->code)  == PLUS && 1)
    goto L333;
  if (	((x1)->code)  == MINUS && 1)
    goto L386;
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L155;
 L333:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L334;
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L155;
 L334:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 83; }
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L155;
 L386:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L387;
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L155;
 L387:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; return 95; }
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L155;
 L156:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, HImode))
    { recog_operand[1] = x1; return 33; }
 L321:
  if (	((x1)->mode)  != HImode)
    {
      x1 = ((x0)->fld[ 0].rtx) ;
      goto L1222;
    }
  if (	((x1)->code)  == PLUS && 1)
    goto L322;
  if (	((x1)->code)  == MINUS && 1)
    goto L375;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L322:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L323;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L323:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 81; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L375:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L376;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L376:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; return 93; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L972:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->code)  == IF_THEN_ELSE && 1)
    goto L973;
  if (	((x1)->code)  == LABEL_REF && 1)
    goto L1162;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L973:
  tem = recog_2 (x0, insn);
  if (tem >= 0) return tem;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L1162:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[0] = x2; return 239; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1222;
 L1223:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->code)  == CALL && 1)
    goto L1224;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L158;
 L1224:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; goto L1225; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L158;
 L1225:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; return 244; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L158;
 L159:
  tem = recog_3 (x0, insn);
  if (tem >= 0) return tem;
  goto L151;
 L152:
  return recog_4 (x0, insn);
 L143:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, SImode))
    { recog_operand[1] = x1; if (	((recog_operand [1])->code)  == CONST_INT
   && ((recog_operand [1])->fld[0].rtint)  >= -0x8000
   && ((recog_operand [1])->fld[0].rtint)  < 0x8000) return 29; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L145;
 L146:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (x1 == const0_rtx && 1)
    return 30;
 L149:
  if (general_operand (x1, SImode))
    { recog_operand[1] = x1; return 31; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L174;
 L175:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (address_operand (x1, SImode))
    { recog_operand[1] = x1; return 39; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L189;
 L190:
  tem = recog_5 (x0, insn);
  if (tem >= 0) return tem;
  goto L792;
 L823:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (nonimmediate_operand (x2, QImode))
    { recog_operand[0] = x2; goto L824; }
 L793:
  if (nonimmediate_operand (x2, SImode))
    { recog_operand[0] = x2; goto L794; }
  goto L798;
 L824:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L825; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L793;
 L825:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; goto L826; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L793;
 L826:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->code)  == XOR && 	((x1)->mode)  == SImode && 1)
    goto L827;
  if (x1 == const0_rtx && 1)
    if ((target_flags & 1)  && (target_flags & 4) ) return 191;
 L843:
  if (	((x1)->code)  == CONST_INT && ((x1)->fld[ 0].rtint)  == -1 && 1)
    if ((target_flags & 1)  && (target_flags & 4) ) return 192;
 L849:
  if (general_operand (x1, SImode))
    { recog_operand[3] = x1; if ((target_flags & 1)  && (target_flags & 4) ) return 193; }
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L793;
 L827:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == ZERO_EXTRACT && 	((x2)->mode)  == SImode && 1)
    goto L828;
  goto L849;
 L828:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (rtx_equal_p (x3, recog_operand[0]) && 1)
    goto L829;
  goto L849;
 L829:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (rtx_equal_p (x3, recog_operand[1]) && 1)
    goto L830;
  goto L849;
 L830:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (rtx_equal_p (x3, recog_operand[2]) && 1)
    goto L831;
  goto L849;
 L831:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (immediate_operand (x2, VOIDmode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [3])->code)  == CONST_INT
   && (((recog_operand [3])->fld[0].rtint)  == -1
       || (	((recog_operand [1])->code)  == CONST_INT
           && (~ ((recog_operand [3])->fld[0].rtint)  & ((1 << ((recog_operand [1])->fld[0].rtint) )- 1)) == 0))) return 190; }
  goto L849;
 L794:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (immediate_operand (x2, SImode))
    { recog_operand[1] = x2; goto L795; }
 L865:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; goto L866; }
  goto L798;
 L795:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (immediate_operand (x2, SImode))
    { recog_operand[2] = x2; goto L796; }
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L865;
 L796:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, SImode))
    { recog_operand[3] = x1; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [1])->code)  == CONST_INT
   && (((recog_operand [1])->fld[0].rtint)  == 8 || ((recog_operand [1])->fld[0].rtint)  == 16)
   && 	((recog_operand [2])->code)  == CONST_INT
   && ((recog_operand [2])->fld[0].rtint)  % ((recog_operand [1])->fld[0].rtint)  == 0
   && (	((recog_operand [0])->code)  == REG
       || ! mode_dependent_address_p (((recog_operand [0])->fld[ 0].rtx) ))) return 185; }
  x1 = ((x0)->fld[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L865;
 L866:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; goto L867; }
  goto L798;
 L867:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (x1 == const0_rtx && 1)
    if ((target_flags & 1)  && (target_flags & 4) ) return 196;
 L873:
  if (	((x1)->code)  == CONST_INT && ((x1)->fld[ 0].rtint)  == -1 && 1)
    if ((target_flags & 1)  && (target_flags & 4) ) return 197;
 L879:
  if (general_operand (x1, SImode))
    { recog_operand[3] = x1; if ((target_flags & 1)  && (target_flags & 4) ) return 198; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L798;
 L799:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->code)  == ZERO_EXTRACT && 	((x1)->mode)  == SImode && 1)
    goto L818;
  if (	((x1)->code)  == SIGN_EXTRACT && 	((x1)->mode)  == SImode && 1)
    goto L812;
 L1229:
  if (address_operand (x1, QImode))
    { recog_operand[1] = x1; return 246; }
  goto ret0;
 L818:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (nonimmediate_operand (x2, QImode))
    { recog_operand[1] = x2; goto L819; }
 L800:
  if (nonimmediate_operand (x2, SImode))
    { recog_operand[1] = x2; goto L801; }
  goto L1229;
 L819:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; goto L820; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L800;
 L820:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) ) return 189; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L800;
 L801:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (immediate_operand (x2, SImode))
    { recog_operand[2] = x2; goto L802; }
 L860:
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; goto L861; }
  goto L1229;
 L802:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (immediate_operand (x2, SImode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [2])->code)  == CONST_INT
   && (((recog_operand [2])->fld[0].rtint)  == 8 || ((recog_operand [2])->fld[0].rtint)  == 16)
   && 	((recog_operand [3])->code)  == CONST_INT
   && ((recog_operand [3])->fld[0].rtint)  % ((recog_operand [2])->fld[0].rtint)  == 0
   && (	((recog_operand [1])->code)  == REG
       || ! mode_dependent_address_p (((recog_operand [1])->fld[ 0].rtx) ))) return 186; }
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L860;
 L861:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) ) return 195; }
  goto L1229;
 L812:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (nonimmediate_operand (x2, QImode))
    { recog_operand[1] = x2; goto L813; }
 L806:
  if (nonimmediate_operand (x2, SImode))
    { recog_operand[1] = x2; goto L807; }
  goto L1229;
 L813:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; goto L814; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L806;
 L814:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) ) return 188; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L806;
 L807:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (immediate_operand (x2, SImode))
    { recog_operand[2] = x2; goto L808; }
 L854:
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; goto L855; }
  goto L1229;
 L808:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (immediate_operand (x2, SImode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) 
   && 	((recog_operand [2])->code)  == CONST_INT
   && (((recog_operand [2])->fld[0].rtint)  == 8 || ((recog_operand [2])->fld[0].rtint)  == 16)
   && 	((recog_operand [3])->code)  == CONST_INT
   && ((recog_operand [3])->fld[0].rtint)  % ((recog_operand [2])->fld[0].rtint)  == 0
   && (	((recog_operand [1])->code)  == REG
       || ! mode_dependent_address_p (((recog_operand [1])->fld[ 0].rtx) ))) return 187; }
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L854;
 L855:
  x2 = ((x1)->fld[ 2].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[3] = x2; if ((target_flags & 1)  && (target_flags & 4) ) return 194; }
  goto L1229;
 L5:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, DImode))
    { recog_operand[1] = x1; return 1; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L171;
 L172:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, DImode))
    { recog_operand[1] = x1; return 38; }
  goto ret0;
 L166:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, SFmode))
    { recog_operand[1] = x1; return 36; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L221;
 L222:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->code)  == FLOAT_TRUNCATE && 	((x1)->mode)  == SFmode && 1)
    goto L223;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L225;
 L223:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 56; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L225;
 L226:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->mode)  != SFmode)
    {
      x1 = ((x0)->fld[ 0].rtx) ;
      goto L1245;
    }
 switch (	((x1)->code) )
  {
  case FLOAT_TRUNCATE:
  if (1)
    goto L227;
  break;
  case FLOAT:
  if (1)
    goto L231;
  break;
  case FIX:
  if (1)
    goto L267;
  break;
  case PLUS:
  if (1)
    goto L348;
  break;
  case MINUS:
  if (1)
    goto L401;
  break;
  case MULT:
  if (1)
    goto L451;
  break;
  case DIV:
  if (1)
    goto L501;
  break;
  case NEG:
  if (1)
    goto L622;
  break;
  case ABS:
  if (1)
    goto L638;
  break;
  }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L227:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 57; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L231:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 59; }
 L235:
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 60; }
 L247:
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 64; }
 L255:
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 66; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L267:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 69; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L348:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L349; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L349:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 88; }
 L354:
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 89; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L401:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L402; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L402:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 100; }
 L407:
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 101; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L451:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L452; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L452:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 112; }
 L457:
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 113; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L501:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L502; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L502:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 124; }
 L507:
  if (general_operand (x2, SFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 125; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L622:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 147; }
 L626:
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 148; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L638:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 153; }
 L642:
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 154; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1245;
 L1246:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->mode)  != SFmode)
    goto ret0;
  if (	((x1)->code)  == PLUS && 1)
    goto L1247;
  if (	((x1)->code)  == MINUS && 1)
    goto L1268;
  if (	((x1)->code)  == MULT && 1)
    goto L1303;
  goto ret0;
 L1247:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == SFmode && 1)
    goto L1248;
 L1254:
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L1255; }
  goto ret0;
 L1248:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[1] = x3; goto L1249; }
  goto L1254;
 L1249:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1250; }
  goto L1254;
 L1250:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 250; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1254;
 L1255:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == SFmode && 1)
    goto L1256;
  goto ret0;
 L1256:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1257; }
  goto ret0;
 L1257:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 251; }
  goto ret0;
 L1268:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L1269; }
 L1282:
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == SFmode && 1)
    goto L1283;
  goto ret0;
 L1269:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == SFmode && 1)
    goto L1270;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1282;
 L1270:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1271; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1282;
 L1271:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 253; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1282;
 L1283:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, SFmode))
    { recog_operand[1] = x3; goto L1284; }
  goto ret0;
 L1284:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1285; }
  goto ret0;
 L1285:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 255; }
  goto ret0;
 L1303:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == SFmode && 1)
    goto L1304;
 L1310:
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L1311; }
 L1331:
  if (	((x2)->code)  == MINUS && 	((x2)->mode)  == SFmode && 1)
    goto L1332;
 L1338:
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; goto L1339; }
  goto ret0;
 L1304:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, SFmode))
    { recog_operand[1] = x3; goto L1305; }
  goto L1310;
 L1305:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1306; }
  goto L1310;
 L1306:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 258; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1310;
 L1311:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == SFmode && 1)
    goto L1312;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1331;
 L1312:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1313; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1331;
 L1313:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 259; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1331;
 L1332:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, SFmode))
    { recog_operand[1] = x3; goto L1333; }
  goto L1338;
 L1333:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1334; }
  goto L1338;
 L1334:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 262; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1338;
 L1339:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MINUS && 	((x2)->mode)  == SFmode && 1)
    goto L1340;
  goto ret0;
 L1340:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, SFmode))
    { recog_operand[2] = x3; goto L1341; }
  goto ret0;
 L1341:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 263; }
  goto ret0;
 L2:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, DFmode))
    { recog_operand[1] = x1; return 0; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L168;
 L169:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, DFmode))
    { recog_operand[1] = x1; return 37; }
 L214:
  if (	((x1)->code)  == FLOAT_EXTEND && 	((x1)->mode)  == DFmode && 1)
    goto L215;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L237;
 L215:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 53; }
 L219:
  if (general_operand (x2, SFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 54; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L237;
 L238:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->code)  == FLOAT && 	((x1)->mode)  == DFmode && 1)
    goto L239;
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L241;
 L239:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 62; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L241;
 L242:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->mode)  != DFmode)
    {
      x1 = ((x0)->fld[ 0].rtx) ;
      goto L1231;
    }
 switch (	((x1)->code) )
  {
  case FLOAT:
  if (1)
    goto L243;
  break;
  case FIX:
  if (1)
    goto L263;
  break;
  case PLUS:
  if (1)
    goto L338;
  break;
  case MINUS:
  if (1)
    goto L391;
  break;
  case MULT:
  if (1)
    goto L441;
  break;
  case DIV:
  if (1)
    goto L491;
  break;
  case NEG:
  if (1)
    goto L630;
  break;
  case ABS:
  if (1)
    goto L646;
  break;
  }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L243:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 63; }
 L251:
  if (general_operand (x2, HImode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 65; }
 L259:
  if (general_operand (x2, QImode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 67; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L263:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 68; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L338:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L339; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L339:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 85; }
 L344:
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 86; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L391:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L392; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L392:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 97; }
 L397:
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 98; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L441:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L442; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L442:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 109; }
 L447:
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 110; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L491:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L492; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L492:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 121; }
 L497:
  if (general_operand (x2, DFmode))
    { recog_operand[2] = x2; if ((target_flags & 2) ) return 122; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L630:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 150; }
 L634:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 151; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L646:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 156; }
 L650:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; if ((target_flags & 2) ) return 157; }
  x1 = ((x0)->fld[ 0].rtx) ;
  goto L1231;
 L1232:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (	((x1)->mode)  != DFmode)
    goto ret0;
  if (	((x1)->code)  == PLUS && 1)
    goto L1233;
  if (	((x1)->code)  == MINUS && 1)
    goto L1261;
  if (	((x1)->code)  == MULT && 1)
    goto L1289;
  goto ret0;
 L1233:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == DFmode && 1)
    goto L1234;
 L1240:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L1241; }
  goto ret0;
 L1234:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[1] = x3; goto L1235; }
  goto L1240;
 L1235:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1236; }
  goto L1240;
 L1236:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 248; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1240;
 L1241:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == DFmode && 1)
    goto L1242;
  goto ret0;
 L1242:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1243; }
  goto ret0;
 L1243:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 249; }
  goto ret0;
 L1261:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L1262; }
 L1275:
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == DFmode && 1)
    goto L1276;
  goto ret0;
 L1262:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MULT && 	((x2)->mode)  == DFmode && 1)
    goto L1263;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1275;
 L1263:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1264; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1275;
 L1264:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 252; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1275;
 L1276:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, DFmode))
    { recog_operand[1] = x3; goto L1277; }
  goto ret0;
 L1277:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1278; }
  goto ret0;
 L1278:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 254; }
  goto ret0;
 L1289:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == DFmode && 1)
    goto L1290;
 L1296:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L1297; }
 L1317:
  if (	((x2)->code)  == MINUS && 	((x2)->mode)  == DFmode && 1)
    goto L1318;
 L1324:
  if (general_operand (x2, DFmode))
    { recog_operand[1] = x2; goto L1325; }
  goto ret0;
 L1290:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, DFmode))
    { recog_operand[1] = x3; goto L1291; }
  goto L1296;
 L1291:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1292; }
  goto L1296;
 L1292:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 256; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1296;
 L1297:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == DFmode && 1)
    goto L1298;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1317;
 L1298:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1299; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1317;
 L1299:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 257; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1317;
 L1318:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, DFmode))
    { recog_operand[1] = x3; goto L1319; }
  goto L1324;
 L1319:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1320; }
  goto L1324;
 L1320:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, DFmode))
    { recog_operand[3] = x2; if ((target_flags & 0100) ) return 260; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L1324;
 L1325:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MINUS && 	((x2)->mode)  == DFmode && 1)
    goto L1326;
  goto ret0;
 L1326:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (register_operand (x3, DFmode))
    { recog_operand[2] = x3; goto L1327; }
  goto ret0;
 L1327:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[3] = x3; if ((target_flags & 0100) ) return 261; }
  goto ret0;
 ret0: return -1;
}

int
recog_7 (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L17:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  if (x2 == cc0_rtx && 1)
    goto L18;
  if (x2 == pc_rtx && 1)
    goto L1153;
 L530:
  if (general_operand (x2, SImode))
    { recog_operand[0] = x2; goto L531; }
  goto ret0;
 L18:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (general_operand (x2, SFmode))
    { recog_operand[0] = x2; goto L19; }
 L27:
  if (general_operand (x2, DFmode))
    { recog_operand[0] = x2; goto L28; }
 L51:
  if (	((x2)->code)  == COMPARE && 1)
    goto L52;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L19:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == CLOBBER && 1)
    goto L20;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L27;
 L20:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 6; }
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L27;
 L28:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == CLOBBER && 1)
    goto L29;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L51;
 L29:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[1] = x2; if ((target_flags & 0100) ) return 9; }
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  goto L51;
 L52:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[0] = x3; goto L53; }
 L65:
  if (general_operand (x3, SFmode))
    { recog_operand[0] = x3; goto L66; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L53:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, DFmode))
    { recog_operand[1] = x3; goto L54; }
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L65;
 L54:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == CLOBBER && 1)
    goto L55;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L65;
 L55:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 15; }
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 1].rtx) ;
  x3 = ((x2)->fld[ 0].rtx) ;
  goto L65;
 L66:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SFmode))
    { recog_operand[1] = x3; goto L67; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L67:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == CLOBBER && 1)
    goto L68;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L68:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[2] = x2; if ((target_flags & 0100) ) return 18; }
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1153:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == SImode && 1)
    goto L1154;
  if (	((x2)->code)  == IF_THEN_ELSE && 1)
    goto L1167;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1154:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (x3 == pc_rtx && 1)
    goto L1155;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1155:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, HImode))
    { recog_operand[0] = x3; goto L1156; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1156:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == USE && 1)
    goto L1157;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1157:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (	((x2)->code)  == LABEL_REF && 1)
    goto L1158;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1158:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[1] = x3; return 238; }
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1167:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (	((x3)->code)  == NE && 1)
    goto L1168;
  if (	((x3)->code)  == GE && 1)
    goto L1206;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1168:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (	((x4)->code)  == COMPARE && 1)
    goto L1169;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1169:
  x5 = ((x4)->fld[ 0].rtx) ;
  if (	((x5)->code)  != PLUS)
    {  x2 = ((x1)->fld[ 0].rtx) ;
    goto L530; }
  if (	((x5)->mode)  == HImode && 1)
    goto L1170;
  if (	((x5)->mode)  == SImode && 1)
    goto L1189;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1170:
  x6 = ((x5)->fld[ 0].rtx) ;
  if (general_operand (x6, HImode))
    { recog_operand[0] = x6; goto L1171; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1171:
  x6 = ((x5)->fld[ 1].rtx) ;
  if (	((x6)->code)  == CONST_INT && ((x6)->fld[ 0].rtint)  == -1 && 1)
    goto L1172;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1172:
  x5 = ((x4)->fld[ 1].rtx) ;
  if (	((x5)->code)  == CONST_INT && ((x5)->fld[ 0].rtint)  == -1 && 1)
    goto L1173;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1173:
  x4 = ((x3)->fld[ 1].rtx) ;
  if (x4 == const0_rtx && 1)
    goto L1174;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1174:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == LABEL_REF && 1)
    goto L1175;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1175:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[1] = x4; goto L1176; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1176:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (x3 == pc_rtx && 1)
    goto L1177;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1177:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == SET && 1)
    goto L1178;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1178:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L1179;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1179:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == HImode && 1)
    goto L1180;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1180:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (rtx_equal_p (x3, recog_operand[0]) && 1)
    goto L1181;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1181:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == -1 && 1)
    return 240;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1189:
  x6 = ((x5)->fld[ 0].rtx) ;
  if (general_operand (x6, SImode))
    { recog_operand[0] = x6; goto L1190; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1190:
  x6 = ((x5)->fld[ 1].rtx) ;
  if (	((x6)->code)  == CONST_INT && ((x6)->fld[ 0].rtint)  == -1 && 1)
    goto L1191;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1191:
  x5 = ((x4)->fld[ 1].rtx) ;
  if (	((x5)->code)  == CONST_INT && ((x5)->fld[ 0].rtint)  == -1 && 1)
    goto L1192;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1192:
  x4 = ((x3)->fld[ 1].rtx) ;
  if (x4 == const0_rtx && 1)
    goto L1193;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1193:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == LABEL_REF && 1)
    goto L1194;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1194:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[1] = x4; goto L1195; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1195:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (x3 == pc_rtx && 1)
    goto L1196;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1196:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == SET && 1)
    goto L1197;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1197:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L1198;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1198:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == SImode && 1)
    goto L1199;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1199:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (rtx_equal_p (x3, recog_operand[0]) && 1)
    goto L1200;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1200:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == -1 && 1)
    return 241;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1206:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (	((x4)->code)  == PLUS && 	((x4)->mode)  == SImode && 1)
    goto L1207;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1207:
  x5 = ((x4)->fld[ 0].rtx) ;
  if (general_operand (x5, SImode))
    { recog_operand[0] = x5; goto L1208; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1208:
  x5 = ((x4)->fld[ 1].rtx) ;
  if (	((x5)->code)  == CONST_INT && ((x5)->fld[ 0].rtint)  == -1 && 1)
    goto L1209;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1209:
  x4 = ((x3)->fld[ 1].rtx) ;
  if (x4 == const0_rtx && 1)
    goto L1210;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1210:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == LABEL_REF && 1)
    goto L1211;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1211:
  x4 = ((x3)->fld[ 0].rtx) ;
  if (1)
    { recog_operand[1] = x4; goto L1212; }
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1212:
  x3 = ((x2)->fld[ 2].rtx) ;
  if (x3 == pc_rtx && 1)
    goto L1213;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1213:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == SET && 1)
    goto L1214;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1214:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (rtx_equal_p (x2, recog_operand[0]) && 1)
    goto L1215;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1215:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == PLUS && 	((x2)->mode)  == SImode && 1)
    goto L1216;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1216:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (rtx_equal_p (x3, recog_operand[0]) && 1)
    goto L1217;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L1217:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (	((x3)->code)  == CONST_INT && ((x3)->fld[ 0].rtint)  == -1 && 1)
    if (find_reg_note (insn, REG_NONNEG, 0)) return 242;
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  x2 = ((x1)->fld[ 0].rtx) ;
  goto L530;
 L531:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->mode)  != SImode)
    goto ret0;
  if (	((x2)->code)  == DIV && 1)
    goto L532;
  if (	((x2)->code)  == UDIV && 1)
    goto L543;
  goto ret0;
 L532:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; goto L533; }
  goto ret0;
 L533:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[2] = x3; goto L534; }
  goto ret0;
 L534:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == SET && 1)
    goto L535;
  goto ret0;
 L535:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[3] = x2; goto L536; }
  goto ret0;
 L536:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == MOD && 	((x2)->mode)  == SImode && 1)
    goto L537;
  goto ret0;
 L537:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (rtx_equal_p (x3, recog_operand[1]) && 1)
    goto L538;
  goto ret0;
 L538:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (rtx_equal_p (x3, recog_operand[2]) && 1)
    if ((target_flags & 1) ) return 130;
  goto ret0;
 L543:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[1] = x3; goto L544; }
  goto ret0;
 L544:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (general_operand (x3, SImode))
    { recog_operand[2] = x3; goto L545; }
  goto ret0;
 L545:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 1].rtx) ;
  if (	((x1)->code)  == SET && 1)
    goto L546;
  goto ret0;
 L546:
  x2 = ((x1)->fld[ 0].rtx) ;
  if (general_operand (x2, SImode))
    { recog_operand[3] = x2; goto L547; }
  goto ret0;
 L547:
  x2 = ((x1)->fld[ 1].rtx) ;
  if (	((x2)->code)  == UMOD && 	((x2)->mode)  == SImode && 1)
    goto L548;
  goto ret0;
 L548:
  x3 = ((x2)->fld[ 0].rtx) ;
  if (rtx_equal_p (x3, recog_operand[1]) && 1)
    goto L549;
  goto ret0;
 L549:
  x3 = ((x2)->fld[ 1].rtx) ;
  if (rtx_equal_p (x3, recog_operand[2]) && 1)
    if ((target_flags & 1) ) return 131;
  goto ret0;
 ret0: return -1;
}

int
recog (x0, insn)
     register rtx x0;
     rtx insn;
{
  register rtx x1, x2, x3, x4, x5;
  rtx x6, x7, x8, x9, x10, x11;
  int tem;
 L0:
 switch (	((x0)->code) )
  {
  case SET:
  if (1)
    goto L7;
  break;
  case PARALLEL:
  if (((x0)->fld[ 0].rtvec->num_elem)  == 2 && 1)
    goto L16;
  break;
  case CALL:
  if (1)
    goto L1219;
  break;
  case RETURN:
  if (1)
    if (0) return 245;
  break;
  }
  goto ret0;
 L7:
  return recog_6 (x0, insn);
 L16:
  x1 = ((x0)->fld[ 0].rtvec->elem[ 0].rtx) ;
  if (	((x1)->code)  == SET && 1)
    goto L17;
  goto ret0;
 L17:
  return recog_7 (x0, insn);
 L1219:
  x1 = ((x0)->fld[ 0].rtx) ;
  if (general_operand (x1, QImode))
    { recog_operand[0] = x1; goto L1220; }
  goto ret0;
 L1220:
  x1 = ((x0)->fld[ 1].rtx) ;
  if (general_operand (x1, SImode))
    { recog_operand[1] = x1; return 243; }
  goto ret0;
 ret0: return -1;
}

