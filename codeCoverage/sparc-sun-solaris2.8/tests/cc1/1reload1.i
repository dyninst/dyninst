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
  int rtwint;
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
extern rtx plus_constant_wide		 ();
extern rtx plus_constant_for_output_wide ();
extern rtx gen_rtx ();
extern rtvec gen_rtvec ();
extern rtx read_rtx			();
extern char *xrealloc ();
extern char *xmalloc			();
extern char *oballoc			();
extern char *permalloc			();
extern void free			();
extern rtx rtx_alloc			();
extern rtvec rtvec_alloc		();
extern rtx find_reg_note		();
extern rtx find_regno_note		();
extern int get_integer_term	();
extern rtx get_related_value		();
extern rtx single_set			();
extern rtx find_last_value		();
extern rtx copy_rtx			();
extern rtx copy_rtx_if_shared		();
extern rtx copy_most_rtx		();
extern rtx replace_rtx			();
extern rtvec gen_rtvec_v		();
extern rtx gen_reg_rtx			();
extern rtx gen_label_rtx		();
extern rtx gen_inline_header_rtx	();
extern rtx gen_lowpart_common		();
extern rtx gen_lowpart			();
extern rtx gen_lowpart_if_possible	();
extern rtx gen_highpart			();
extern rtx gen_realpart			();
extern rtx gen_imagpart			();
extern rtx operand_subword		();
extern rtx operand_subword_force	();
extern int subreg_lowpart_p		();
extern rtx make_safe_from		();
extern rtx memory_address		();
extern rtx get_insns			();
extern rtx get_last_insn		();
extern rtx get_last_insn_anywhere	();
extern void start_sequence		();
extern void push_to_sequence		();
extern void end_sequence		();
extern rtx gen_sequence			();
extern rtx immed_double_const		();
extern rtx force_const_mem		();
extern rtx force_reg			();
extern rtx get_pool_constant		();
extern enum machine_mode get_pool_mode	();
extern int get_pool_offset		();
extern rtx simplify_subtraction		();
extern rtx assign_stack_local		();
extern rtx assign_stack_temp		();
extern rtx protect_from_queue		();
extern void emit_queue			();
extern rtx emit_move_insn		();
extern rtx emit_insn_before		();
extern rtx emit_jump_insn_before	();
extern rtx emit_call_insn_before	();
extern rtx emit_barrier_before		();
extern rtx emit_note_before		();
extern rtx emit_insn_after		();
extern rtx emit_jump_insn_after		();
extern rtx emit_barrier_after		();
extern rtx emit_label_after		();
extern rtx emit_note_after		();
extern rtx emit_line_note_after		();
extern rtx emit_insn			();
extern rtx emit_insns			();
extern rtx emit_insns_before		();
extern rtx emit_jump_insn		();
extern rtx emit_call_insn		();
extern rtx emit_label			();
extern rtx emit_barrier			();
extern rtx emit_line_note		();
extern rtx emit_note			();
extern rtx emit_line_note_force		();
extern rtx make_insn_raw		();
extern rtx previous_insn		();
extern rtx next_insn			();
extern rtx prev_nonnote_insn		();
extern rtx next_nonnote_insn		();
extern rtx prev_real_insn		();
extern rtx next_real_insn		();
extern rtx prev_active_insn		();
extern rtx next_active_insn		();
extern rtx prev_label			();
extern rtx next_label			();
extern rtx next_cc0_user		();
extern rtx prev_cc0_setter		();
extern rtx reg_set_last			();
extern rtx next_nondeleted_insn		();
extern enum rtx_code reverse_condition	();
extern enum rtx_code swap_condition	();
extern enum rtx_code unsigned_condition	();
extern enum rtx_code signed_condition	();
extern rtx find_equiv_reg		();
extern rtx squeeze_notes		();
extern rtx delete_insn			();
extern void delete_jump			();
extern rtx get_label_before		();
extern rtx get_label_after		();
extern rtx follow_jumps			();
extern rtx adj_offsettable_operand	();
extern rtx try_split			();
extern rtx split_insns			();
extern rtx simplify_unary_operation	();
extern rtx simplify_binary_operation	();
extern rtx simplify_ternary_operation	();
extern rtx simplify_relational_operation ();
extern rtx nonlocal_label_rtx_list	();
extern rtx gen_move_insn		();
extern rtx gen_jump			();
extern rtx gen_beq			();
extern rtx gen_bge			();
extern rtx gen_ble			();
extern rtx eliminate_constant_term	();
extern rtx expand_complex_abs		();
extern int max_parallel;
extern int asm_noperands		();
extern char *decode_asm_operands	();
extern enum reg_class reg_preferred_class ();
extern enum reg_class reg_alternate_class ();
extern rtx get_first_nonparm_insn	();
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
extern rtx find_next_ref		();
extern rtx *find_single_use		();
extern rtx expand_expr ();
extern rtx immed_real_const_1();
extern rtx output_constant_def ();
extern rtx immed_real_const ();
extern rtx immed_real_const_1 ();
extern int reload_completed;
extern int reload_in_progress;
extern int cse_not_expected;
extern rtx *regno_reg_rtx;
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
  long temp;		
  int   alignment_mask;		
  struct _obstack_chunk *(*chunkfun) (); 
  void (*freefun) ();		
  char *extra_arg;		
  unsigned use_extra_arg:1;	
  unsigned maybe_empty_object:1;
};
extern void _obstack_newchunk ();
extern void _obstack_free ();
extern void _obstack_begin ();
extern void _obstack_begin_1 ();
extern rtx gen_tstsi            ();
extern rtx gen_tsthi            ();
extern rtx gen_tstqi            ();
extern rtx gen_tstdf            ();
extern rtx gen_tstsf            ();
extern rtx gen_cmpsi            ();
extern rtx gen_cmphi            ();
extern rtx gen_cmpqi            ();
extern rtx gen_cmpdf            ();
extern rtx gen_cmpsf            ();
extern rtx gen_sltu             ();
extern rtx gen_sgeu             ();
extern rtx gen_movdf            ();
extern rtx gen_movsf            ();
extern rtx gen_movdi            ();
extern rtx gen_movsi            ();
extern rtx gen_movhi            ();
extern rtx gen_movstricthi      ();
extern rtx gen_movqi            ();
extern rtx gen_movstrictqi      ();
extern rtx gen_movstrhi         ();
extern rtx gen_movstrhi1        ();
extern rtx gen_truncsiqi2       ();
extern rtx gen_truncsihi2       ();
extern rtx gen_trunchiqi2       ();
extern rtx gen_extendhisi2      ();
extern rtx gen_extendqihi2      ();
extern rtx gen_extendqisi2      ();
extern rtx gen_extendsfdf2      ();
extern rtx gen_truncdfsf2       ();
extern rtx gen_zero_extendhisi2 ();
extern rtx gen_zero_extendqihi2 ();
extern rtx gen_zero_extendqisi2 ();
extern rtx gen_floatsisf2       ();
extern rtx gen_floatsidf2       ();
extern rtx gen_floathisf2       ();
extern rtx gen_floathidf2       ();
extern rtx gen_floatqisf2       ();
extern rtx gen_floatqidf2       ();
extern rtx gen_fix_truncsfqi2   ();
extern rtx gen_fix_truncsfhi2   ();
extern rtx gen_fix_truncsfsi2   ();
extern rtx gen_fix_truncdfqi2   ();
extern rtx gen_fix_truncdfhi2   ();
extern rtx gen_fix_truncdfsi2   ();
extern rtx gen_adddf3           ();
extern rtx gen_addsf3           ();
extern rtx gen_addsi3           ();
extern rtx gen_addhi3           ();
extern rtx gen_addqi3           ();
extern rtx gen_adddi3           ();
extern rtx gen_subdf3           ();
extern rtx gen_subsf3           ();
extern rtx gen_subsi3           ();
extern rtx gen_subhi3           ();
extern rtx gen_subqi3           ();
extern rtx gen_subdi3           ();
extern rtx gen_muldf3           ();
extern rtx gen_mulsf3           ();
extern rtx gen_mulsi3           ();
extern rtx gen_mulhi3           ();
extern rtx gen_mulqi3           ();
extern rtx gen_mulsidi3         ();
extern rtx gen_divdf3           ();
extern rtx gen_divsf3           ();
extern rtx gen_divsi3           ();
extern rtx gen_divhi3           ();
extern rtx gen_divqi3           ();
extern rtx gen_andsi3           ();
extern rtx gen_andhi3           ();
extern rtx gen_andqi3           ();
extern rtx gen_iorsi3           ();
extern rtx gen_iorhi3           ();
extern rtx gen_iorqi3           ();
extern rtx gen_xorsi3           ();
extern rtx gen_xorhi3           ();
extern rtx gen_xorqi3           ();
extern rtx gen_negdf2           ();
extern rtx gen_negsf2           ();
extern rtx gen_negsi2           ();
extern rtx gen_neghi2           ();
extern rtx gen_negqi2           ();
extern rtx gen_one_cmplsi2      ();
extern rtx gen_one_cmplhi2      ();
extern rtx gen_one_cmplqi2      ();
extern rtx gen_ashrsi3          ();
extern rtx gen_ashlsi3          ();
extern rtx gen_ashrdi3          ();
extern rtx gen_ashldi3          ();
extern rtx gen_rotrsi3          ();
extern rtx gen_rotlsi3          ();
extern rtx gen_extv             ();
extern rtx gen_extzv            ();
extern rtx gen_insv             ();
extern rtx gen_jump             ();
extern rtx gen_beq              ();
extern rtx gen_bne              ();
extern rtx gen_bgt              ();
extern rtx gen_bgtu             ();
extern rtx gen_blt              ();
extern rtx gen_bltu             ();
extern rtx gen_bge              ();
extern rtx gen_bgeu             ();
extern rtx gen_ble              ();
extern rtx gen_bleu             ();
extern rtx gen_return           ();
extern rtx gen_nop              ();
extern rtx gen_indirect_jump    ();
extern rtx gen_casesi           ();
extern rtx gen_casesi1          ();
extern rtx gen_call_pop ();
extern rtx gen_call_value_pop ();
enum insn_code {
  CODE_FOR_tstsi = 0,
  CODE_FOR_tsthi = 1,
  CODE_FOR_tstqi = 2,
  CODE_FOR_tstdf = 3,
  CODE_FOR_tstsf = 4,
  CODE_FOR_cmpsi = 5,
  CODE_FOR_cmphi = 6,
  CODE_FOR_cmpqi = 7,
  CODE_FOR_cmpdf = 8,
  CODE_FOR_cmpsf = 9,
  CODE_FOR_sltu = 13,
  CODE_FOR_sgeu = 14,
  CODE_FOR_movdf = 15,
  CODE_FOR_movsf = 16,
  CODE_FOR_movdi = 17,
  CODE_FOR_movsi = 18,
  CODE_FOR_movhi = 19,
  CODE_FOR_movstricthi = 20,
  CODE_FOR_movqi = 21,
  CODE_FOR_movstrictqi = 22,
  CODE_FOR_movstrhi = 23,
  CODE_FOR_movstrhi1 = 24,
  CODE_FOR_truncsiqi2 = 25,
  CODE_FOR_truncsihi2 = 26,
  CODE_FOR_trunchiqi2 = 27,
  CODE_FOR_extendhisi2 = 28,
  CODE_FOR_extendqihi2 = 29,
  CODE_FOR_extendqisi2 = 30,
  CODE_FOR_extendsfdf2 = 31,
  CODE_FOR_truncdfsf2 = 32,
  CODE_FOR_zero_extendhisi2 = 33,
  CODE_FOR_zero_extendqihi2 = 34,
  CODE_FOR_zero_extendqisi2 = 35,
  CODE_FOR_floatsisf2 = 36,
  CODE_FOR_floatsidf2 = 37,
  CODE_FOR_floathisf2 = 38,
  CODE_FOR_floathidf2 = 39,
  CODE_FOR_floatqisf2 = 40,
  CODE_FOR_floatqidf2 = 41,
  CODE_FOR_fix_truncsfqi2 = 42,
  CODE_FOR_fix_truncsfhi2 = 43,
  CODE_FOR_fix_truncsfsi2 = 44,
  CODE_FOR_fix_truncdfqi2 = 45,
  CODE_FOR_fix_truncdfhi2 = 46,
  CODE_FOR_fix_truncdfsi2 = 47,
  CODE_FOR_adddf3 = 48,
  CODE_FOR_addsf3 = 49,
  CODE_FOR_addsi3 = 50,
  CODE_FOR_addhi3 = 51,
  CODE_FOR_addqi3 = 52,
  CODE_FOR_adddi3 = 53,
  CODE_FOR_subdf3 = 54,
  CODE_FOR_subsf3 = 55,
  CODE_FOR_subsi3 = 56,
  CODE_FOR_subhi3 = 57,
  CODE_FOR_subqi3 = 58,
  CODE_FOR_subdi3 = 59,
  CODE_FOR_muldf3 = 60,
  CODE_FOR_mulsf3 = 61,
  CODE_FOR_mulsi3 = 62,
  CODE_FOR_mulhi3 = 63,
  CODE_FOR_mulqi3 = 64,
  CODE_FOR_mulsidi3 = 65,
  CODE_FOR_divdf3 = 68,
  CODE_FOR_divsf3 = 69,
  CODE_FOR_divsi3 = 70,
  CODE_FOR_divhi3 = 71,
  CODE_FOR_divqi3 = 72,
  CODE_FOR_andsi3 = 73,
  CODE_FOR_andhi3 = 74,
  CODE_FOR_andqi3 = 75,
  CODE_FOR_iorsi3 = 82,
  CODE_FOR_iorhi3 = 83,
  CODE_FOR_iorqi3 = 84,
  CODE_FOR_xorsi3 = 85,
  CODE_FOR_xorhi3 = 86,
  CODE_FOR_xorqi3 = 87,
  CODE_FOR_negdf2 = 88,
  CODE_FOR_negsf2 = 89,
  CODE_FOR_negsi2 = 90,
  CODE_FOR_neghi2 = 91,
  CODE_FOR_negqi2 = 92,
  CODE_FOR_one_cmplsi2 = 93,
  CODE_FOR_one_cmplhi2 = 94,
  CODE_FOR_one_cmplqi2 = 95,
  CODE_FOR_ashrsi3 = 96,
  CODE_FOR_ashlsi3 = 99,
  CODE_FOR_ashrdi3 = 100,
  CODE_FOR_ashldi3 = 101,
  CODE_FOR_rotrsi3 = 103,
  CODE_FOR_rotlsi3 = 104,
  CODE_FOR_extv = 116,
  CODE_FOR_extzv = 117,
  CODE_FOR_insv = 118,
  CODE_FOR_jump = 120,
  CODE_FOR_beq = 121,
  CODE_FOR_bne = 122,
  CODE_FOR_bgt = 123,
  CODE_FOR_bgtu = 124,
  CODE_FOR_blt = 125,
  CODE_FOR_bltu = 126,
  CODE_FOR_bge = 127,
  CODE_FOR_bgeu = 128,
  CODE_FOR_ble = 129,
  CODE_FOR_bleu = 130,
  CODE_FOR_call_pop = 143,
  CODE_FOR_call_value_pop = 144,
  CODE_FOR_return = 147,
  CODE_FOR_nop = 148,
  CODE_FOR_indirect_jump = 149,
  CODE_FOR_casesi = 150,
  CODE_FOR_casesi1 = 151,
  CODE_FOR_nothing };
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
extern int pending_stack_adjust;
enum direction {none, upward, downward};  
typedef struct optab
{
  enum rtx_code code;
  struct {
    enum insn_code insn_code;
    rtx libfunc;
  } handlers [(int) MAX_MACHINE_MODE];
} * optab;
extern rtx (* insn_gen_function[]) ();
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
extern rtxfun bcc_gen_fctn[((int)LAST_AND_UNUSED_RTX_CODE)];
extern enum insn_code setcc_gen_code[((int)LAST_AND_UNUSED_RTX_CODE)];
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
extern int max_regno;
extern int max_scratch;
extern int *reg_n_refs;
extern short *reg_n_sets;
extern short *reg_n_deaths;
extern int *reg_n_calls_crossed;
extern int *reg_live_length;
extern short *reg_renumber;
extern char regs_ever_live[64];
extern char *reg_names[64];
extern int *regno_first_uid;
extern int *regno_last_uid;
extern char *regno_pointer_flag;
extern rtx regs_may_share;
extern rtx *regno_reg_rtx;
extern int caller_save_needed;
typedef int HARD_REG_SET[ ((64 + 32 - 1)	  / 32)];
extern char fixed_regs[64];
extern HARD_REG_SET fixed_reg_set;
extern char call_used_regs[64];
extern HARD_REG_SET call_used_reg_set;
extern char call_fixed_regs[64];
extern HARD_REG_SET call_fixed_reg_set;
extern char global_regs[64];
extern int reg_alloc_order[64];
extern HARD_REG_SET reg_class_contents[];
extern int reg_class_size[(int) LIM_REG_CLASSES];
extern enum reg_class reg_class_superclasses[(int) LIM_REG_CLASSES][(int) LIM_REG_CLASSES];
extern enum reg_class reg_class_subclasses[(int) LIM_REG_CLASSES][(int) LIM_REG_CLASSES];
extern enum reg_class reg_class_subunion[(int) LIM_REG_CLASSES][(int) LIM_REG_CLASSES];
extern enum reg_class reg_class_superunion[(int) LIM_REG_CLASSES][(int) LIM_REG_CLASSES];
extern int n_non_fixed_regs;
extern char *reg_names[64];
extern rtx reload_in[(2 * 10 * (1 + 1))];
extern rtx reload_out[(2 * 10 * (1 + 1))];
extern rtx reload_in_reg[(2 * 10 * (1 + 1))];
extern enum reg_class reload_reg_class[(2 * 10 * (1 + 1))];
extern enum machine_mode reload_inmode[(2 * 10 * (1 + 1))];
extern enum machine_mode reload_outmode[(2 * 10 * (1 + 1))];
extern char reload_strict_low[(2 * 10 * (1 + 1))];
extern char reload_optional[(2 * 10 * (1 + 1))];
extern int reload_inc[(2 * 10 * (1 + 1))];
extern int reload_needed_for_multiple[(2 * 10 * (1 + 1))];
extern rtx reload_needed_for[(2 * 10 * (1 + 1))];
extern int reload_secondary_reload[(2 * 10 * (1 + 1))];
extern int reload_secondary_p[(2 * 10 * (1 + 1))];
extern enum insn_code reload_secondary_icode[(2 * 10 * (1 + 1))];
extern int n_reloads;
extern rtx reload_reg_rtx[(2 * 10 * (1 + 1))];
enum reload_when_needed
{
  RELOAD_FOR_INPUT_RELOAD_ADDRESS,
  RELOAD_FOR_OUTPUT_RELOAD_ADDRESS,
  RELOAD_FOR_OPERAND_ADDRESS,
  RELOAD_FOR_INPUT,
  RELOAD_FOR_OUTPUT,
  RELOAD_OTHER
};
extern enum reload_when_needed reload_when_needed[(2 * 10 * (1 + 1))];
extern rtx *reg_equiv_constant;
extern rtx *reg_equiv_memory_loc;
extern rtx *reg_equiv_address;
extern rtx *reg_equiv_mem;
extern int n_earlyclobbers;
extern rtx reload_earlyclobbers[10];
extern int reload_first_uid;
extern char indirect_symref_ok;
extern char double_reg_address_ok;
extern enum insn_code reload_in_optab[];
extern enum insn_code reload_out_optab[];
extern void init_reload ();
extern void find_reloads ();
extern void subst_reloads ();
extern rtx get_secondary_mem ();
extern rtx eliminate_regs ();
extern rtx gen_input_reload ();
extern rtx find_replacement ();
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
extern char * insn_template[];
extern char *(* insn_outfun[]) ();
extern  int insn_n_operands[];
extern  int insn_n_dups[];
extern  int insn_n_alternatives[];
extern char * insn_operand_constraint[][10];
extern  enum machine_mode insn_operand_mode[][10];
extern  char insn_operand_strict_low[][10];
extern int (* insn_operand_predicate[][10]) ();
extern char * insn_name[];
typedef int *regset;
extern int regset_bytes;
extern int regset_size;
extern int n_basic_blocks;
extern rtx *basic_block_head;
extern rtx *basic_block_end;
extern regset *basic_block_live_at_start;
extern short *reg_basic_block;
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
extern int current_function_args_info;
extern char *current_function_name;
extern rtx current_function_return_rtx;
extern rtx current_function_epilogue_delay_list;
extern int flag_pic;
extern int current_function_uses_pic_offset_table;
extern int current_function_uses_const_pool;
extern FILE *asm_out_file;
static rtx *reg_last_reload_reg;
static char *reg_has_output_reload;
static HARD_REG_SET reg_is_output_reload;
rtx *reg_equiv_constant;
rtx *reg_equiv_memory_loc;
rtx *reg_equiv_address;
rtx *reg_equiv_mem;
static int *reg_max_ref_width;
static rtx *reg_equiv_init;
static int reg_reloaded_contents[64];
static rtx reg_reloaded_insn[64];
static int n_spills;
static rtx spill_reg_rtx[64];
static rtx spill_reg_store[64];
static short spill_reg_order[64];
HARD_REG_SET forbidden_regs;
static HARD_REG_SET bad_spill_regs;
static short spill_regs[64];
static short potential_reload_regs[64];
static char regs_explicitly_used[64];
static HARD_REG_SET counted_for_groups;
static HARD_REG_SET counted_for_nongroups;
static char spill_indirect_levels;
char indirect_symref_ok;
char double_reg_address_ok;
static rtx spill_stack_slot[64];
static int spill_stack_slot_width[64];
char *basic_block_needs[(int) LIM_REG_CLASSES];
int reload_first_uid;
int caller_save_needed;
int reload_in_progress = 0;
enum insn_code reload_in_optab[(int) MAX_MACHINE_MODE];
enum insn_code reload_out_optab[(int) MAX_MACHINE_MODE];
struct obstack reload_obstack;
char *reload_firstobj;
extern rtx forced_labels;
static struct elim_table
{
  int from;			
  int to;			
  int initial_offset;		
  int can_eliminate;		
  int can_eliminate_previous;	
  int offset;			
  int max_offset;		
  int previous_offset;		
  int ref_outside_mem;		
  rtx from_rtx;			
  rtx to_rtx;			
} reg_eliminate[] =
  {{ 15, 30},	 { 29, 0}};
static int num_not_at_initial_offset;
static int num_eliminable;
static char *offsets_known_at;
static int (*offsets_at)[(sizeof reg_eliminate / sizeof reg_eliminate[0])];
static int num_labels;
void mark_home_live ();
static void count_possible_groups ();
static int possible_group_p ();
static void scan_paradoxical_subregs ();
static void reload_as_needed ();
static int modes_equiv_for_class_p ();
static void alter_reg ();
static void delete_dead_insn ();
static void spill_failure ();
static int new_spill_reg();
static void set_label_offsets ();
static int eliminate_regs_in_insn ();
static void mark_not_eliminable ();
static int spill_hard_reg ();
static void choose_reload_regs ();
static void emit_reload_insns ();
static void delete_output_reload ();
static void forget_old_reloads_1 ();
static void order_regs_for_reload ();
static rtx inc_for_reload ();
static int constraint_accepts_reg_p ();
static int count_occurrences ();
extern void remove_death ();
extern rtx adj_offsettable_operand ();
extern rtx form_sum ();
void
init_reload ()
{
  register int i;
  register rtx tem
    = gen_rtx (MEM, DImode,
	       gen_rtx (PLUS, DImode,
			gen_rtx (REG, DImode, (((64)) + 3) + 1),
			gen_rtx (CONST_INT, VOIDmode, (4))));
  spill_indirect_levels = 0;
  while (memory_address_p (QImode, tem))
    {
      spill_indirect_levels++;
      tem = gen_rtx (MEM, DImode, tem);
    }
  tem = gen_rtx (MEM, DImode, gen_rtx (SYMBOL_REF, DImode, "foo"));
  indirect_symref_ok = memory_address_p (QImode, tem);
  for (i = 0; i < 64; i++)
    {
      tem = gen_rtx (PLUS, DImode,
		     gen_rtx (REG, DImode, 15),
		     gen_rtx (REG, DImode, i));
      tem = plus_constant_wide (tem, (int) ( 4));
      if (memory_address_p (QImode, tem))
	{
	  double_reg_address_ok = 1;
	  break;
	}
    }
  gcc_obstack_init (&reload_obstack);
  reload_firstobj = (char *)  (( ((&reload_obstack))->temp = ( ( 0)),							  ((((&reload_obstack))->chunk_limit - ((&reload_obstack))->next_free < ((&reload_obstack))->temp)			   ? (_obstack_newchunk (((&reload_obstack)), ((&reload_obstack))->temp), 0) : 0),			  ((&reload_obstack))->next_free += ((&reload_obstack))->temp), ( (((&reload_obstack))->next_free == ((&reload_obstack))->object_base					   ? ((((&reload_obstack))->maybe_empty_object = 1), 0)					   : 0),								  ((&reload_obstack))->temp = ((((&reload_obstack))->object_base) - (char *)0),				  ((&reload_obstack))->next_free							    = (((((((&reload_obstack))->next_free) - (char *)0)+((&reload_obstack))->alignment_mask)			    & ~ (((&reload_obstack))->alignment_mask)) + (char *)0),				  ((((&reload_obstack))->next_free - (char *)((&reload_obstack))->chunk					    > ((&reload_obstack))->chunk_limit - (char *)((&reload_obstack))->chunk)				   ? (((&reload_obstack))->next_free = ((&reload_obstack))->chunk_limit) : 0),				  ((&reload_obstack))->object_base = ((&reload_obstack))->next_free,					  ((((&reload_obstack))->temp) + (char *)0)));
  for (i = 0; i < (int) MAX_MACHINE_MODE; i++)
    reload_in_optab[i] = reload_out_optab[i] = CODE_FOR_nothing;
}
int
reload (first, global, dumpfile)
     rtx first;
     int global;
     FILE *dumpfile;
{
  register int class;
  register int i;
  register rtx insn;
  register struct elim_table *ep;
  int something_changed;
  int something_needs_reloads;
  int something_needs_elimination;
  int new_basic_block_needs;
  enum reg_class caller_save_spill_class = NO_REGS;
  int caller_save_group_size = 1;
  int failure = 0;
  int this_block;
  init_recog ();
  reload_first_uid = get_max_uid ();
  for (i = 0; i < (int) LIM_REG_CLASSES; i++)
    basic_block_needs[i] = 0;
  clear_secondary_mem ();
  bcopy (regs_ever_live, regs_explicitly_used, sizeof regs_ever_live);
  bzero (spill_stack_slot, sizeof spill_stack_slot);
  bzero (spill_stack_slot_width, sizeof spill_stack_slot_width);
  init_save_areas ();
  for (i = 64; i < max_regno; i++)
    mark_home_live (i);
  emit_note (((char *)0), -1);
  reg_equiv_constant = (rtx *) alloca (max_regno * sizeof (rtx));
  bzero (reg_equiv_constant, max_regno * sizeof (rtx));
  reg_equiv_memory_loc = (rtx *) alloca (max_regno * sizeof (rtx));
  bzero (reg_equiv_memory_loc, max_regno * sizeof (rtx));
  reg_equiv_mem = (rtx *) alloca (max_regno * sizeof (rtx));
  bzero (reg_equiv_mem, max_regno * sizeof (rtx));
  reg_equiv_init = (rtx *) alloca (max_regno * sizeof (rtx));
  bzero (reg_equiv_init, max_regno * sizeof (rtx));
  reg_equiv_address = (rtx *) alloca (max_regno * sizeof (rtx));
  bzero (reg_equiv_address, max_regno * sizeof (rtx));
  reg_max_ref_width = (int *) alloca (max_regno * sizeof (int));
  bzero (reg_max_ref_width, max_regno * sizeof (int));
  for (insn = first; insn; insn = ((insn)->fld[2].rtx))
    {
      rtx set = single_set (insn);
      if (set != 0 && ((((set)->fld[0].rtx))->code) == REG)
	{
	  rtx note = find_reg_note (insn, REG_EQUIV, (rtx) 0);
	  if (note
	      )
	    {
	      rtx x = ((note)->fld[ 0].rtx);
	      i = ((((set)->fld[0].rtx))->fld[0].rtint);
	      if (i > (((64)) + 3))
		{
		  if (((x)->code) == MEM)
		    reg_equiv_memory_loc[i] = x;
		  else if (  (((x)->code) == LABEL_REF || ((x)->code) == SYMBOL_REF		   || ((x)->code) == CONST_INT || ((x)->code) == CONST_DOUBLE		   || ((x)->code) == CONST || ((x)->code) == HIGH))
		    {
		      if (  ((mode_class[(int)(((x)->mode))]) != MODE_FLOAT	   || (x) == (const_tiny_rtx[0][(int) (((x)->mode))])))
			reg_equiv_constant[i] = x;
		      else
			reg_equiv_memory_loc[i]
			  = force_const_mem (((((set)->fld[0].rtx))->mode), x);
		    }
		  else
		    continue;
		  if (((x)->code) != MEM
		      || rtx_equal_p (((set)->fld[1].rtx), x))
		    reg_equiv_init[i] = insn;
		}
	    }
	}
      else if (set && ((((set)->fld[0].rtx))->code) == MEM
	       && ((((set)->fld[1].rtx))->code) == REG
	       && reg_equiv_memory_loc[((((set)->fld[1].rtx))->fld[0].rtint)]
	       && rtx_equal_p (((set)->fld[0].rtx),
			       reg_equiv_memory_loc[((((set)->fld[1].rtx))->fld[0].rtint)]))
	reg_equiv_init[((((set)->fld[1].rtx))->fld[0].rtint)] = insn;
      if ((rtx_class[(int)(((insn)->code))]) == 'i')
	scan_paradoxical_subregs (((insn)->fld[3].rtx));
    }
  frame_pointer_needed = (! flag_omit_frame_pointer
			  || (current_function_calls_alloca
			      && 1)
			  || 0);
  num_eliminable = 0;
  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
    {
      ep->can_eliminate = ep->can_eliminate_previous
	= (((ep->from) == 29 ? ! alpha_need_gp () : 1)
	   && (ep->from != 15 || ! frame_pointer_needed));
    }
  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
    {
      num_eliminable += ep->can_eliminate;
      ep->from_rtx = gen_rtx (REG, DImode, ep->from);
      ep->to_rtx = gen_rtx (REG, DImode, ep->to);
    }
  num_labels = max_label_num () - get_first_label_num ();
  offsets_known_at = (char *) alloca (num_labels);
  offsets_at
    = (int (*)[(sizeof reg_eliminate / sizeof reg_eliminate[0])])
      alloca (num_labels * (sizeof reg_eliminate / sizeof reg_eliminate[0]) * sizeof (int));
  offsets_known_at -= get_first_label_num ();
  offsets_at -= get_first_label_num ();
  for (i = (((64)) + 3) + 1; i < max_regno; i++)
    alter_reg (i, -1);
  assign_stack_local (BLKmode, 0, 0);
  for (insn = first; insn && num_eliminable; insn = ((insn)->fld[2].rtx))
    if (((insn)->code) == INSN || ((insn)->code) == JUMP_INSN
	|| ((insn)->code) == CALL_INSN)
      note_stores (((insn)->fld[3].rtx), mark_not_eliminable);
  order_regs_for_reload ();
  n_spills = 0;
  for (i = 0; i < 64; i++)
    spill_reg_order[i] = -1;
  do { register int *scan_tp_ = (forbidden_regs), *scan_fp_ = ( bad_spill_regs);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
    if (! ep->can_eliminate)
      {
	spill_hard_reg (ep->from, global, dumpfile, 1);
	regs_ever_live[ep->from] = 1;
      }
  if (global)
    for (i = 0; i < (int) LIM_REG_CLASSES; i++)
      {
	basic_block_needs[i] = (char *)alloca (n_basic_blocks);
	bzero (basic_block_needs[i], n_basic_blocks);
      }
  reload_in_progress = 1;
  something_changed = 1;
  something_needs_reloads = 0;
  something_needs_elimination = 0;
  while (something_changed)
    {
      rtx after_call = 0;
      int max_needs[(int) LIM_REG_CLASSES];
      int group_size[(int) LIM_REG_CLASSES];
      int max_groups[(int) LIM_REG_CLASSES];
      int max_nongroups[(int) LIM_REG_CLASSES];
      enum machine_mode group_mode[(int) LIM_REG_CLASSES];
      rtx max_needs_insn[(int) LIM_REG_CLASSES];
      rtx max_groups_insn[(int) LIM_REG_CLASSES];
      rtx max_nongroups_insn[(int) LIM_REG_CLASSES];
      rtx x;
      int starting_frame_size = get_frame_size ();
      static char *reg_class_names[] =  {"NO_REGS", "GENERAL_REGS", "FLOAT_REGS", "ALL_REGS" };
      something_changed = 0;
      bzero (max_needs, sizeof max_needs);
      bzero (max_groups, sizeof max_groups);
      bzero (max_nongroups, sizeof max_nongroups);
      bzero (max_needs_insn, sizeof max_needs_insn);
      bzero (max_groups_insn, sizeof max_groups_insn);
      bzero (max_nongroups_insn, sizeof max_nongroups_insn);
      bzero (group_size, sizeof group_size);
      for (i = 0; i < (int) LIM_REG_CLASSES; i++)
	group_mode[i] = VOIDmode;
      this_block = 0;
      new_basic_block_needs = 0;
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	{
	  { if ((ep->from) == 15 && ( ep->to) == 30)	    ( ep->initial_offset) = (get_frame_size () + current_function_outgoing_args_size 		+ current_function_pretend_args_size					+ alpha_sa_size () + 15) & ~ 15;			};
	  ep->previous_offset = ep->offset
	    = ep->max_offset = ep->initial_offset;
	}
      num_not_at_initial_offset = 0;
      bzero (&offsets_known_at[get_first_label_num ()], num_labels);
      for (x = forced_labels; x; x = ((x)->fld[ 1].rtx))
	if (((x)->fld[ 0].rtx))
	  set_label_offsets (((x)->fld[ 0].rtx), (rtx) 0, 1);
      for (i = 64; i < max_regno; i++)
	if (reg_renumber[i] < 0 && reg_equiv_memory_loc[i])
	  {
	    rtx x = eliminate_regs (reg_equiv_memory_loc[i], 0, (rtx) 0);
	    if (strict_memory_address_p (((regno_reg_rtx[i])->mode),
					 ((x)->fld[ 0].rtx)))
	      reg_equiv_mem[i] = x, reg_equiv_address[i] = 0;
	    else if (  (((((x)->fld[ 0].rtx))->code) == LABEL_REF || ((((x)->fld[ 0].rtx))->code) == SYMBOL_REF		   || ((((x)->fld[ 0].rtx))->code) == CONST_INT || ((((x)->fld[ 0].rtx))->code) == CONST_DOUBLE		   || ((((x)->fld[ 0].rtx))->code) == CONST || ((((x)->fld[ 0].rtx))->code) == HIGH)
		     || (((((x)->fld[ 0].rtx))->code) == PLUS
			 && ((((((x)->fld[ 0].rtx))->fld[ 0].rtx))->code) == REG
			 && (((((((x)->fld[ 0].rtx))->fld[ 0].rtx))->fld[0].rtint)
			     < 64)
			 &&   (((((((x)->fld[ 0].rtx))->fld[ 1].rtx))->code) == LABEL_REF || ((((((x)->fld[ 0].rtx))->fld[ 1].rtx))->code) == SYMBOL_REF		   || ((((((x)->fld[ 0].rtx))->fld[ 1].rtx))->code) == CONST_INT || ((((((x)->fld[ 0].rtx))->fld[ 1].rtx))->code) == CONST_DOUBLE		   || ((((((x)->fld[ 0].rtx))->fld[ 1].rtx))->code) == CONST || ((((((x)->fld[ 0].rtx))->fld[ 1].rtx))->code) == HIGH)))
	      reg_equiv_address[i] = ((x)->fld[ 0].rtx), reg_equiv_mem[i] = 0;
	    else
	      {
		reg_equiv_memory_loc[i] = 0;
		reg_equiv_init[i] = 0;
		alter_reg (i, -1);
		something_changed = 1;
	      }
	  }
      if (something_changed)
	continue;
      if (caller_save_group_size > 1)
	{
	  group_mode[(int) caller_save_spill_class] = DImode;
	  group_size[(int) caller_save_spill_class] = caller_save_group_size;
	}
      for (insn = first; insn; insn = ((insn)->fld[2].rtx))
	{
	  if (global && this_block + 1 < n_basic_blocks
	      && insn == basic_block_head[this_block+1])
	    ++this_block;
	  if (((insn)->code) == CODE_LABEL || ((insn)->code) == JUMP_INSN
	      || ((rtx_class[(int)(((insn)->code))]) == 'i'
		  && ((insn)->fld[6].rtx) != 0))
	    set_label_offsets (insn, insn, 0);
	  if ((rtx_class[(int)(((insn)->code))]) == 'i')
	    {
	      rtx avoid_return_reg = 0;
	      rtx old_body = ((insn)->fld[3].rtx);
	      int old_code = ((insn)->fld[4].rtint);
 	      rtx old_notes = ((insn)->fld[6].rtx);
	      int did_elimination = 0;
	      int insn_needs[(int) LIM_REG_CLASSES];
	      int insn_groups[(int) LIM_REG_CLASSES];
	      int insn_total_groups = 0;
	      int insn_needs_for_inputs[(int) LIM_REG_CLASSES];
	      int insn_groups_for_inputs[(int) LIM_REG_CLASSES];
	      int insn_total_groups_for_inputs = 0;
	      int insn_needs_for_outputs[(int) LIM_REG_CLASSES];
	      int insn_groups_for_outputs[(int) LIM_REG_CLASSES];
	      int insn_total_groups_for_outputs = 0;
	      int insn_needs_for_operands[(int) LIM_REG_CLASSES];
	      int insn_groups_for_operands[(int) LIM_REG_CLASSES];
	      int insn_total_groups_for_operands = 0;
	      if (num_eliminable)
		did_elimination = eliminate_regs_in_insn (insn, 0);
	      find_reloads (insn, 0, spill_indirect_levels, global,
			    spill_reg_order);
	      ((insn)->mode = ( (did_elimination ? QImode
			       : n_reloads ? HImode
			       : VOIDmode)));
	      if (did_elimination)
		{
		  ( (&reload_obstack)->temp = (char *)( reload_firstobj) - (char *) (&reload_obstack)->chunk,			  (((&reload_obstack)->temp > 0 && (&reload_obstack)->temp < (&reload_obstack)->chunk_limit - (char *) (&reload_obstack)->chunk)   ? (int) ((&reload_obstack)->next_free = (&reload_obstack)->object_base					    = (&reload_obstack)->temp + (char *) (&reload_obstack)->chunk)				   : (_obstack_free ((&reload_obstack), (&reload_obstack)->temp + (char *) (&reload_obstack)->chunk), 0)));
		  ((insn)->fld[3].rtx) = old_body;
		  ((insn)->fld[4].rtint) = old_code;
 		  ((insn)->fld[6].rtx) = old_notes;
		  something_needs_elimination = 1;
		}
	      if (n_reloads == 0
		  && ! (((insn)->code) == CALL_INSN
			&& caller_save_spill_class != NO_REGS))
		continue;
	      something_needs_reloads = 1;
	      for (i = 0; i < (int) LIM_REG_CLASSES; i++)
		{
		  insn_needs[i] = 0, insn_groups[i] = 0;
		  insn_needs_for_inputs[i] = 0, insn_groups_for_inputs[i] = 0;
		  insn_needs_for_outputs[i] = 0, insn_groups_for_outputs[i] = 0;
		  insn_needs_for_operands[i] = 0, insn_groups_for_operands[i] = 0;
		}
	      for (i = 0; i < n_reloads; i++)
		{
		  register enum reg_class *p;
		  enum reg_class class = reload_reg_class[i];
		  int size;
		  enum machine_mode mode;
		  int *this_groups;
		  int *this_needs;
		  int *this_total_groups;
		  if (reload_reg_rtx[i] != 0
		      || reload_optional[i] != 0
		      || (reload_out[i] == 0 && reload_in[i] == 0
			  && ! reload_secondary_p[i]))
  		    continue;
		  if (global && ! basic_block_needs[(int) class][this_block])
		    {
		      basic_block_needs[(int) class][this_block] = 1;
		      new_basic_block_needs = 1;
		    }
		  switch (reload_when_needed[i])
		    {
		    case RELOAD_OTHER:
		    case RELOAD_FOR_OUTPUT:
		    case RELOAD_FOR_INPUT:
		      this_needs = insn_needs;
		      this_groups = insn_groups;
		      this_total_groups = &insn_total_groups;
		      break;
		    case RELOAD_FOR_INPUT_RELOAD_ADDRESS:
		      this_needs = insn_needs_for_inputs;
		      this_groups = insn_groups_for_inputs;
		      this_total_groups = &insn_total_groups_for_inputs;
		      break;
		    case RELOAD_FOR_OUTPUT_RELOAD_ADDRESS:
		      this_needs = insn_needs_for_outputs;
		      this_groups = insn_groups_for_outputs;
		      this_total_groups = &insn_total_groups_for_outputs;
		      break;
		    case RELOAD_FOR_OPERAND_ADDRESS:
		      this_needs = insn_needs_for_operands;
		      this_groups = insn_groups_for_operands;
		      this_total_groups = &insn_total_groups_for_operands;
		      break;
		    }
		  mode = reload_inmode[i];
		  if ((mode_size[(int)(reload_outmode[i])]) > (mode_size[(int)(mode)]))
		    mode = reload_outmode[i];
		  size =  (((mode_size[(int)( mode)]) + 8 - 1) / 8);
		  if (size > 1)
		    {
		      enum machine_mode other_mode, allocate_mode;
		      this_groups[(int) class]++;
		      p = reg_class_superclasses[(int) class];
		      while (*p != LIM_REG_CLASSES)
			this_groups[(int) *p++]++;
		      (*this_total_groups)++;
		      if (group_size[(int) class] < size)
			{
			  other_mode = group_mode[(int) class];
			  allocate_mode = mode;
			  group_size[(int) class] = size;
			  group_mode[(int) class] = mode;
			}
		      else
			{
			  other_mode = mode;
			  allocate_mode = group_mode[(int) class];
			}
		      if (other_mode != VOIDmode
			  && other_mode != allocate_mode
			  && ! modes_equiv_for_class_p (allocate_mode,
							other_mode,
							class))
			abort ();
		    }
		  else if (size == 1)
		    {
		      this_needs[(int) class] += 1;
		      p = reg_class_superclasses[(int) class];
		      while (*p != LIM_REG_CLASSES)
			this_needs[(int) *p++] += 1;
		    }
		  else
		    abort ();
		}
	      for (i = 0; i < (int) LIM_REG_CLASSES; i++)
		{
		  int this_max;
		  this_max = insn_needs_for_inputs[i];
		  if (insn_needs_for_outputs[i] > this_max)
		    this_max = insn_needs_for_outputs[i];
		  if (insn_needs_for_operands[i] > this_max)
		    this_max = insn_needs_for_operands[i];
		  insn_needs[i] += this_max;
		  this_max = insn_groups_for_inputs[i];
		  if (insn_groups_for_outputs[i] > this_max)
		    this_max = insn_groups_for_outputs[i];
		  if (insn_groups_for_operands[i] > this_max)
		    this_max = insn_groups_for_operands[i];
		  insn_groups[i] += this_max;
		}
	      insn_total_groups += ((insn_total_groups_for_inputs) > (
					((insn_total_groups_for_outputs) > (
					     insn_total_groups_for_operands) ? (insn_total_groups_for_outputs) : (
					     insn_total_groups_for_operands))) ? (insn_total_groups_for_inputs) : (
					((insn_total_groups_for_outputs) > (
					     insn_total_groups_for_operands) ? (insn_total_groups_for_outputs) : (
					     insn_total_groups_for_operands))));
	      if (((insn)->code) == CALL_INSN
		  && caller_save_spill_class != NO_REGS)
		{
		  int *caller_save_needs
		    = (caller_save_group_size > 1 ? insn_groups : insn_needs);
		  if (caller_save_needs[(int) caller_save_spill_class] == 0)
		    {
		      register enum reg_class *p
			= reg_class_superclasses[(int) caller_save_spill_class];
		      caller_save_needs[(int) caller_save_spill_class]++;
		      while (*p != LIM_REG_CLASSES)
			caller_save_needs[(int) *p++] += 1;
		    }
		  if (caller_save_group_size > 1)
		    insn_total_groups = ((insn_total_groups) > ( 1) ? (insn_total_groups) : ( 1));
                if (global
                    && ! (basic_block_needs[(int) caller_save_spill_class]
                          [this_block]))
                  {
                    basic_block_needs[(int) caller_save_spill_class]
                      [this_block] = 1;
                    new_basic_block_needs = 1;
                  }
		}
	      for (i = 0; i < (int) LIM_REG_CLASSES; i++)
		{
		  if (max_needs[i] < insn_needs[i])
		    {
		      max_needs[i] = insn_needs[i];
		      max_needs_insn[i] = insn;
		    }
		  if (max_groups[i] < insn_groups[i])
		    {
		      max_groups[i] = insn_groups[i];
		      max_groups_insn[i] = insn;
		    }
		  if (insn_total_groups > 0)
		    if (max_nongroups[i] < insn_needs[i])
		      {
			max_nongroups[i] = insn_needs[i];
			max_nongroups_insn[i] = insn;
		      }
		}
	    }
	}
      if (starting_frame_size != get_frame_size ())
	something_changed = 1;
      if (dumpfile)
	for (i = 0; i < (int) LIM_REG_CLASSES; i++)
	  {
	    if (max_needs[i] > 0)
	      fprintf (dumpfile,
			 ";; Need %d reg%s of class %s (for insn %d).\n",
		       max_needs[i], max_needs[i] == 1 ? "" : "s",
		       reg_class_names[i], ((max_needs_insn[i])->fld[0].rtint));
	    if (max_nongroups[i] > 0)
	      fprintf (dumpfile,
		       ";; Need %d nongroup reg%s of class %s (for insn %d).\n",
		       max_nongroups[i], max_nongroups[i] == 1 ? "" : "s",
		       reg_class_names[i], ((max_nongroups_insn[i])->fld[0].rtint));
	    if (max_groups[i] > 0)
	      fprintf (dumpfile,
		       ";; Need %d group%s (%smode) of class %s (for insn %d).\n",
		       max_groups[i], max_groups[i] == 1 ? "" : "s",
		       mode_name[(int) group_mode[i]],
		       reg_class_names[i], ((max_groups_insn[i])->fld[0].rtint));
	  }
      if (caller_save_needed
	  && ! setup_save_areas (&something_changed)
	  && caller_save_spill_class  == NO_REGS)
	{
	  caller_save_spill_class
	    = double_reg_address_ok ? NO_REGS : GENERAL_REGS;
	  caller_save_group_size
	    =  (((mode_size[(int)( DImode)]) + 8 - 1) / 8);
	  something_changed = 1;
	}
      do { register int *scan_tp_ = (counted_for_groups);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
      do { register int *scan_tp_ = (counted_for_nongroups);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
      for (i = 0; i < n_spills; i++)
	{
	  register enum reg_class *p;
	  class = (int) ((spill_regs[i]) >= 32 ? FLOAT_REGS : GENERAL_REGS);
	  if (reg_class_size[class] == 1 && max_nongroups[class] > 0)
	    {
	      max_needs[class]--;
	      p = reg_class_superclasses[class];
	      while (*p != LIM_REG_CLASSES)
		max_needs[(int) *p++]--;
	        ((counted_for_nongroups)[( spill_regs[i]) / ((unsigned) 32)]	   |= (int) 1 << (( spill_regs[i]) % ((unsigned) 32)));
	      max_nongroups[class]--;
	      p = reg_class_superclasses[class];
	      while (*p != LIM_REG_CLASSES)
		{
		  if (max_nongroups[(int) *p] > 0)
		      ((counted_for_nongroups)[( spill_regs[i]) / ((unsigned) 32)]	   |= (int) 1 << (( spill_regs[i]) % ((unsigned) 32)));
		  max_nongroups[(int) *p++]--;
		}
	    }
	}
      count_possible_groups (group_size, group_mode, max_groups);
      for (i = 0; i < n_spills; i++)
	{
	  register enum reg_class *p;
	  class = (int) ((spill_regs[i]) >= 32 ? FLOAT_REGS : GENERAL_REGS);
	  if (!   ((counted_for_nongroups)[( spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << (( spill_regs[i]) % ((unsigned) 32)))))
	    {
	      max_needs[class]--;
	      p = reg_class_superclasses[class];
	      while (*p != LIM_REG_CLASSES)
		max_needs[(int) *p++]--;
	      if (!   ((counted_for_groups)[( spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << (( spill_regs[i]) % ((unsigned) 32)))))
		{
		  if (max_nongroups[class] > 0)
		      ((counted_for_nongroups)[( spill_regs[i]) / ((unsigned) 32)]	   |= (int) 1 << (( spill_regs[i]) % ((unsigned) 32)));
		  max_nongroups[class]--;
		  p = reg_class_superclasses[class];
		  while (*p != LIM_REG_CLASSES)
		    {
		      if (max_nongroups[(int) *p] > 0)
			  ((counted_for_nongroups)[(
					  spill_regs[i]) / ((unsigned) 32)]	   |= (int) 1 << ((
					  spill_regs[i]) % ((unsigned) 32)));
		      max_nongroups[(int) *p++]--;
		    }
		}
	    }
	}
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	if ((ep->from == 15 && 0)
	    || ! ((ep->from) == 29 ? ! alpha_need_gp () : 1)
	    )
	  ep->can_eliminate = 0;
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	{
	  struct elim_table *op;
	  register int new_to = -1;
	  if (! ep->can_eliminate && ep->can_eliminate_previous)
	    {
	      for (op = reg_eliminate;
		   op < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; op++)
		if (op->from == ep->from && op->can_eliminate)
		  {
		    new_to = op->to;
		    break;
		  }
	      for (op = reg_eliminate;
		   op < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; op++)
		if (op->from == new_to && op->to == ep->to)
		  op->can_eliminate = 0;
	    }
	}
      frame_pointer_needed = 1;
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	{
	  if (ep->can_eliminate && ep->from == 15)
	    frame_pointer_needed = 0;
	  if (! ep->can_eliminate && ep->can_eliminate_previous)
	    {
	      ep->can_eliminate_previous = 0;
	      spill_hard_reg (ep->from, global, dumpfile, 1);
	      regs_ever_live[ep->from] = 1;
	      something_changed = 1;
	      num_eliminable--;
	    }
	}
      for (i = 0; i < (int) LIM_REG_CLASSES; i++)
	if (max_needs[i] > 0 || max_groups[i] > 0 || max_nongroups[i] > 0)
	  break;
      if (i == (int) LIM_REG_CLASSES && !new_basic_block_needs && ! something_changed)
	break;
      if (new_basic_block_needs)
	for (i = 0; i < n_spills; i++)
	  something_changed
	    |= spill_hard_reg (spill_regs[i], global, dumpfile, 0);
      for (class = 0; class < (int) LIM_REG_CLASSES; class++)
	{
	  while (max_groups[class] > 0)
	    {
	      count_possible_groups (group_size, group_mode, max_groups);
	      if (group_size[class] == 2)
		{
		  for (i = 0; i < 64; i++)
		    {
		      int j = potential_reload_regs[i];
		      int other;
		      if (j >= 0 && !   ((bad_spill_regs)[( j) / ((unsigned) 32)]	   & ((int) 1 << (( j) % ((unsigned) 32))))
			  &&
			  ((j > 0 && (other = j - 1, spill_reg_order[other] >= 0)
			    &&   ((reg_class_contents[class])[( j) / ((unsigned) 32)]	   & ((int) 1 << (( j) % ((unsigned) 32))))
			    &&   ((reg_class_contents[class])[( other) / ((unsigned) 32)]	   & ((int) 1 << (( other) % ((unsigned) 32))))
			    && 1
			    && !   ((counted_for_nongroups)[(
						    other) / ((unsigned) 32)]	   & ((int) 1 << ((
						    other) % ((unsigned) 32))))
			    && !   ((counted_for_groups)[( other) / ((unsigned) 32)]	   & ((int) 1 << (( other) % ((unsigned) 32)))))
			   ||
			   (j < 64 - 1
			    && (other = j + 1, spill_reg_order[other] >= 0)
			    &&   ((reg_class_contents[class])[( j) / ((unsigned) 32)]	   & ((int) 1 << (( j) % ((unsigned) 32))))
			    &&   ((reg_class_contents[class])[( other) / ((unsigned) 32)]	   & ((int) 1 << (( other) % ((unsigned) 32))))
			    && 1
			    && !   ((counted_for_nongroups)[(
						    other) / ((unsigned) 32)]	   & ((int) 1 << ((
						    other) % ((unsigned) 32))))
			    && !   ((counted_for_groups)[(
						    other) / ((unsigned) 32)]	   & ((int) 1 << ((
						    other) % ((unsigned) 32)))))))
			{
			  register enum reg_class *p;
			  max_groups[class]--;
			  p = reg_class_superclasses[class];
			  while (*p != LIM_REG_CLASSES)
			    max_groups[(int) *p++]--;
			    ((counted_for_groups)[( j) / ((unsigned) 32)]	   |= (int) 1 << (( j) % ((unsigned) 32)));
			    ((counted_for_groups)[( other) / ((unsigned) 32)]	   |= (int) 1 << (( other) % ((unsigned) 32)));
			  break;
			}
		    }
		  if (i == 64)
		    for (i = 0; i < 64; i++)
		      {
			int j = potential_reload_regs[i];
			if (j >= 0 && j + 1 < 64
			    && spill_reg_order[j] < 0 && spill_reg_order[j + 1] < 0
			    &&   ((reg_class_contents[class])[( j) / ((unsigned) 32)]	   & ((int) 1 << (( j) % ((unsigned) 32))))
			    &&   ((reg_class_contents[class])[( j + 1) / ((unsigned) 32)]	   & ((int) 1 << (( j + 1) % ((unsigned) 32))))
			    && 1
			    && !   ((counted_for_nongroups)[(
						    j + 1) / ((unsigned) 32)]	   & ((int) 1 << ((
						    j + 1) % ((unsigned) 32)))))
			  break;
		      }
		  if (i >= 64)
		    {
		      spill_failure (max_groups_insn[class]);
		      failure = 1;
		      goto failed;
		    }
		  else
		    something_changed
		      |= new_spill_reg (i, class, max_needs, ((char *)0),
					global, dumpfile);
		}
	      else
		{
		  for (i = 0; i < 64; i++)
		    {
		      int j = potential_reload_regs[i];
		      int k;
		      if (j >= 0
			  && j + group_size[class] <= 64
			  && 1)
			{
			  for (k = 0; k < group_size[class]; k++)
			    if (! (spill_reg_order[j + k] < 0
				   && !   ((bad_spill_regs)[( j + k) / ((unsigned) 32)]	   & ((int) 1 << (( j + k) % ((unsigned) 32))))
				   &&   ((reg_class_contents[class])[( j + k) / ((unsigned) 32)]	   & ((int) 1 << (( j + k) % ((unsigned) 32))))))
			      break;
			  if (k == group_size[class])
			    {
			      register enum reg_class *p;
			      for (k = 0; k < group_size[class]; k++)
				{
				  int idx;
				    ((counted_for_groups)[( j + k) / ((unsigned) 32)]	   |= (int) 1 << (( j + k) % ((unsigned) 32)));
				  for (idx = 0; idx < 64; idx++)
				    if (potential_reload_regs[idx] == j + k)
				      break;
				  something_changed
				    |= new_spill_reg (idx, class,
						      max_needs, ((char *)0),
						      global, dumpfile);
				}
			      max_groups[class]--;
			      p = reg_class_superclasses[class];
			      while (*p != LIM_REG_CLASSES)
				max_groups[(int) *p++]--;
			      break;
			    }
			}
		    }
		  if (i >= 64)
		    {
		      spill_failure (max_groups_insn[class]);
		      failure = 1;
		      goto failed;
		    }
		}
	    }
	  while (max_needs[class] > 0 || max_nongroups[class] > 0)
	    {
	      for (i = 0; i < 64; i++)
		if (potential_reload_regs[i] >= 0
		    &&   ((reg_class_contents[class])[(
					  potential_reload_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << ((
					  potential_reload_regs[i]) % ((unsigned) 32))))
		    && (max_nongroups[class] == 0
			|| possible_group_p (potential_reload_regs[i], max_groups)))
		  break;
	      if (i >= 64
		  && (asm_noperands (max_needs[class] > 0
				     ? max_needs_insn[class]
				     : max_nongroups_insn[class])
		      < 0))
		for (i = 0; i < 64; i++)
		  if (potential_reload_regs[i] >= 0
		      &&   ((reg_class_contents[class])[(
					    potential_reload_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << ((
					    potential_reload_regs[i]) % ((unsigned) 32)))))
		    break;
	      if (i >= 64)
		{
		  spill_failure (max_needs[class] > 0 ? max_needs_insn[class]
				 : max_nongroups_insn[class]);
		  failure = 1;
		  goto failed;
		}
	      else
		something_changed
		  |= new_spill_reg (i, class, max_needs, max_nongroups,
				    global, dumpfile);
	    }
	}
    }
  if (global)
    for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
      if (ep->can_eliminate)
	mark_elimination (ep->from, ep->to);
  if (caller_save_needed)
    save_call_clobbered_regs (num_eliminable ? QImode
			      : caller_save_spill_class != NO_REGS ? HImode
			      : VOIDmode);
  for (i = 64; i < max_regno; i++)
    if (reg_renumber[i] < 0 && reg_equiv_init[i] != 0
	&& ((reg_equiv_init[i])->code) != NOTE)
      {
	if (reg_set_p (regno_reg_rtx[i], ((reg_equiv_init[i])->fld[3].rtx)))
	  delete_dead_insn (reg_equiv_init[i]);
	else
	  {
	    ((reg_equiv_init[i])->code = ( NOTE));
	    ((reg_equiv_init[i])->fld[3].rtstr) = 0;
	    ((reg_equiv_init[i])->fld[4].rtint) = -1;
	  }
      }
  if (something_needs_reloads || something_needs_elimination
      || (caller_save_needed && num_eliminable)
      || caller_save_spill_class != NO_REGS)
    reload_as_needed (first, global);
  if (! frame_pointer_needed)
    for (i = 0; i < n_basic_blocks; i++)
      basic_block_live_at_start[i][15 / 32]
	&= ~ ((int) 1 << (15 % 32));
  reload_in_progress = 0;
 failed:
  for (i = 64; i < max_regno; i++)
    {
      rtx addr = 0;
      int in_struct = 0;
      if (reg_equiv_mem[i])
	{
	  addr = ((reg_equiv_mem[i])->fld[ 0].rtx);
	  in_struct = ((reg_equiv_mem[i])->in_struct);
	}
      if (reg_equiv_address[i])
	addr = reg_equiv_address[i];
      if (addr)
	{
	  if (reg_renumber[i] < 0)
	    {
	      rtx reg = regno_reg_rtx[i];
	      ((reg)->fld[ 0].rtx) = addr;
	      ((reg)->volatil) = 0;
	      ((reg)->in_struct) = in_struct;
	      ((reg)->code = ( MEM));
	    }
	  else if (reg_equiv_mem[i])
	    ((reg_equiv_mem[i])->fld[ 0].rtx) = addr;
	}
    }
  reg_equiv_constant = 0;
  reg_equiv_memory_loc = 0;
  return failure;
}
static int
possible_group_p (regno, max_groups)
     int regno;
     int *max_groups;
{
  int i;
  int class = (int) NO_REGS;
  for (i = 0; i < (int) (int) LIM_REG_CLASSES; i++)
    if (max_groups[i] > 0)
      {
	class = i;
	break;
      }
  if (class == (int) NO_REGS)
    return 1;
  for (i = 0; i < 64 - 1; i++)
    {
      if (i == regno || i + 1 == regno)
	continue;
      if (! (  ((reg_class_contents[class])[( i) / ((unsigned) 32)]	   & ((int) 1 << (( i) % ((unsigned) 32))))
	     &&   ((reg_class_contents[class])[( i + 1) / ((unsigned) 32)]	   & ((int) 1 << (( i + 1) % ((unsigned) 32))))))
	continue;
      if (spill_reg_order[i] < 0 && spill_reg_order[i + 1] < 0
	  && !   ((bad_spill_regs)[( i) / ((unsigned) 32)]	   & ((int) 1 << (( i) % ((unsigned) 32))))
	  && !   ((bad_spill_regs)[( i + 1) / ((unsigned) 32)]	   & ((int) 1 << (( i + 1) % ((unsigned) 32)))))
	return 1;
      if (spill_reg_order[i] < 0
	  && !   ((bad_spill_regs)[( i) / ((unsigned) 32)]	   & ((int) 1 << (( i) % ((unsigned) 32))))
	  && spill_reg_order[i + 1] >= 0
	  && !   ((counted_for_groups)[( i + 1) / ((unsigned) 32)]	   & ((int) 1 << (( i + 1) % ((unsigned) 32))))
	  && !   ((counted_for_nongroups)[( i + 1) / ((unsigned) 32)]	   & ((int) 1 << (( i + 1) % ((unsigned) 32)))))
	return 1;
      if (spill_reg_order[i + 1] < 0
	  && !   ((bad_spill_regs)[( i + 1) / ((unsigned) 32)]	   & ((int) 1 << (( i + 1) % ((unsigned) 32))))
	  && spill_reg_order[i] >= 0
	  && !   ((counted_for_groups)[( i) / ((unsigned) 32)]	   & ((int) 1 << (( i) % ((unsigned) 32))))
	  && !   ((counted_for_nongroups)[( i) / ((unsigned) 32)]	   & ((int) 1 << (( i) % ((unsigned) 32)))))
	return 1;
    }
  return 0;
}
static void
count_possible_groups (group_size, group_mode, max_groups)
     int *group_size, *max_groups;
     enum machine_mode *group_mode;
{
  int i;
  for (i = 0; i < (int) LIM_REG_CLASSES; i++)
    if (group_size[i] > 1)
      {
	char regmask[64];
	int j;
	bzero (regmask, sizeof regmask);
	for (j = 0; j < n_spills; j++)
	  if (  ((reg_class_contents[i])[( spill_regs[j]) / ((unsigned) 32)]	   & ((int) 1 << (( spill_regs[j]) % ((unsigned) 32))))
	      && !   ((counted_for_groups)[( spill_regs[j]) / ((unsigned) 32)]	   & ((int) 1 << (( spill_regs[j]) % ((unsigned) 32))))
	      && !   ((counted_for_nongroups)[(
				      spill_regs[j]) / ((unsigned) 32)]	   & ((int) 1 << ((
				      spill_regs[j]) % ((unsigned) 32)))))
	    regmask[spill_regs[j]] = 1;
	for (j = 0; j < 64 && max_groups[i] > 0; j++)
	  if (regmask[j] && j + group_size[i] <= 64
	      && 1)
	    {
	      int k;
	      for (k = 1; k < group_size[i]; k++)
		if (! regmask[j + k])
		  break;
	      if (k == group_size[i])
		{
		  register enum reg_class *p;
		  max_groups[i]--;
		  p = reg_class_superclasses[i];
		  while (*p != LIM_REG_CLASSES)
		    max_groups[(int) *p++]--;
		  for (k = 0; k < group_size[i]; k++)
		      ((counted_for_groups)[( j + k) / ((unsigned) 32)]	   |= (int) 1 << (( j + k) % ((unsigned) 32)));
		}
	      j += k - 1;
	    }
      }
}
static int
modes_equiv_for_class_p (allocate_mode, other_mode, class)
     enum machine_mode allocate_mode, other_mode;
     enum reg_class class;
{
  register int regno;
  for (regno = 0; regno < 64; regno++)
    {
      if (  ((reg_class_contents[(int) class])[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	  && 1
	  && ! 1)
	return 0;
    }
  return 1;
}
static void
spill_failure (insn)
     rtx insn;
{
  if (asm_noperands (((insn)->fld[3].rtx)) >= 0)
    error_for_asm (insn, "`asm' needs too many reloads");
  else
    abort ();
}
static int
new_spill_reg (i, class, max_needs, max_nongroups, global, dumpfile)
     int i;
     int class;
     int *max_needs;
     int *max_nongroups;
     int global;
     FILE *dumpfile;
{
  register enum reg_class *p;
  int val;
  int regno = potential_reload_regs[i];
  if (i >= 64)
    abort ();	
  if (fixed_regs[regno] ||   ((forbidden_regs)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
    fatal ("fixed or forbidden register was spilled.\nThis may be due to a compiler bug or to impossible asm statements."  );
  potential_reload_regs[i] = -1;
  spill_regs[n_spills] = regno;
  spill_reg_order[regno] = n_spills;
  if (dumpfile)
    fprintf (dumpfile, "Spilling reg %d.\n", spill_regs[n_spills]);
  max_needs[class]--;
  p = reg_class_superclasses[class];
  while (*p != LIM_REG_CLASSES)
    max_needs[(int) *p++]--;
  if (max_nongroups && max_nongroups[class] > 0)
    {
        ((counted_for_nongroups)[( regno) / ((unsigned) 32)]	   |= (int) 1 << (( regno) % ((unsigned) 32)));
      max_nongroups[class]--;
      p = reg_class_superclasses[class];
      while (*p != LIM_REG_CLASSES)
	max_nongroups[(int) *p++]--;
    }
  val = spill_hard_reg (spill_regs[n_spills], global, dumpfile, 0);
  if (num_eliminable && ! regs_ever_live[spill_regs[n_spills]])
    val = 1;
  regs_ever_live[spill_regs[n_spills]] = 1;
  n_spills++;
  return val;
}
static void
delete_dead_insn (insn)
     rtx insn;
{
  rtx prev = prev_real_insn (insn);
  rtx prev_dest;
  if (prev && ((((prev)->fld[3].rtx))->code) == SET
      && (prev_dest = ((((prev)->fld[3].rtx))->fld[0].rtx), ((prev_dest)->code) == REG)
      && reg_mentioned_p (prev_dest, ((insn)->fld[3].rtx))
      && find_regno_note (insn, REG_DEAD, ((prev_dest)->fld[0].rtint)))
    delete_dead_insn (prev);
  ((insn)->code = ( NOTE));
  ((insn)->fld[4].rtint) = -1;
  ((insn)->fld[3].rtstr) = 0;
}
static void
alter_reg (i, from_reg)
     register int i;
     int from_reg;
{
  if (regno_reg_rtx[i] == 0)
    return;
  if (((regno_reg_rtx[i])->code) != REG)
    return;
  ((regno_reg_rtx[i])->fld[0].rtint)
    = reg_renumber[i] >= 0 ? reg_renumber[i] : i;
  if (reg_renumber[i] < 0
      && reg_n_refs[i] > 0
      && reg_equiv_constant[i] == 0
      && reg_equiv_memory_loc[i] == 0)
    {
      register rtx x;
      int inherent_size =   (mode_size[(int)(((regno_reg_rtx[i])->mode))]);
      int total_size = ((inherent_size) > ( reg_max_ref_width[i]) ? (inherent_size) : ( reg_max_ref_width[i]));
      int adjust = 0;
      if (from_reg == -1)
	{
	  x = assign_stack_local (((regno_reg_rtx[i])->mode), total_size, -1);
	}
      else if (spill_stack_slot[from_reg] != 0
	       && spill_stack_slot_width[from_reg] >= total_size
	       && ((mode_size[(int)(((spill_stack_slot[from_reg])->mode))])
		   >= inherent_size))
	x = spill_stack_slot[from_reg];
      else
	{
	  enum machine_mode mode = ((regno_reg_rtx[i])->mode);
	  if (spill_stack_slot[from_reg])
	    {
	      if ((mode_size[(int)(((spill_stack_slot[from_reg])->mode))])
		  > inherent_size)
		mode = ((spill_stack_slot[from_reg])->mode);
	      if (spill_stack_slot_width[from_reg] > total_size)
		total_size = spill_stack_slot_width[from_reg];
	    }
	  x = assign_stack_local (mode, total_size, -1);
	  spill_stack_slot[from_reg] = x;
	  spill_stack_slot_width[from_reg] = total_size;
	}
      if (adjust != 0 || ((x)->mode) != ((regno_reg_rtx[i])->mode))
	{
	  x = gen_rtx (MEM, ((regno_reg_rtx[i])->mode),
		       plus_constant_wide (((x)->fld[ 0].rtx), (int) ( adjust)));
	  ((x)->unchanging) = ((regno_reg_rtx[i])->unchanging);
	}
      reg_equiv_memory_loc[i] = x;
    }
}
void
mark_home_live (regno)
     int regno;
{
  register int i, lim;
  i = reg_renumber[regno];
  if (i < 0)
    return;
  lim = i +   (((mode_size[(int)( ((regno_reg_rtx[regno])->mode))]) + 8 - 1) / 8);
  while (i < lim)
    regs_ever_live[i++] = 1;
}
static void
set_label_offsets (x, insn, initial_p)
     rtx x;
     rtx insn;
     int initial_p;
{
  enum rtx_code code = ((x)->code);
  rtx tem;
  int i;
  struct elim_table *p;
  switch (code)
    {
    case LABEL_REF:
      if (((x)->volatil))
	return;
      x = ((x)->fld[ 0].rtx);
    case CODE_LABEL:
      if (! offsets_known_at[((x)->fld[3].rtint)])
	{
	  for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
	    offsets_at[((x)->fld[3].rtint)][i]
	      = (initial_p ? reg_eliminate[i].initial_offset
		 : reg_eliminate[i].offset);
	  offsets_known_at[((x)->fld[3].rtint)] = 1;
	}
      else if (x == insn
	       && (tem = prev_nonnote_insn (insn)) != 0
	       && ((tem)->code) == BARRIER)
	{
	  num_not_at_initial_offset = 0;
	  for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
	    {
	      reg_eliminate[i].offset = reg_eliminate[i].previous_offset
		= offsets_at[((x)->fld[3].rtint)][i];
	      if (reg_eliminate[i].can_eliminate
		  && (reg_eliminate[i].offset
		      != reg_eliminate[i].initial_offset))
		num_not_at_initial_offset++;
	    }
	}
      else
	for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
	  if (offsets_at[((x)->fld[3].rtint)][i]
	      != (initial_p ? reg_eliminate[i].initial_offset
		  : reg_eliminate[i].offset))
	    reg_eliminate[i].can_eliminate = 0;
      return;
    case JUMP_INSN:
      set_label_offsets (((insn)->fld[3].rtx), insn, initial_p);
    case INSN:
    case CALL_INSN:
      for (tem = ((x)->fld[6].rtx); tem; tem = ((tem)->fld[ 1].rtx))
	if (((enum reg_note) ((tem)->mode)) == REG_LABEL)
	  set_label_offsets (((tem)->fld[ 0].rtx), insn, 1);
      return;
    case ADDR_VEC:
    case ADDR_DIFF_VEC:
      for (i = 0; i < ((x)->fld[ code == ADDR_DIFF_VEC].rtvec->num_elem); i++)
	set_label_offsets (((x)->fld[ code == ADDR_DIFF_VEC].rtvec->elem[ i].rtx),
			   insn, initial_p);
      return;
    case SET:
      if (((x)->fld[0].rtx) != pc_rtx)
	return;
      switch (((((x)->fld[1].rtx))->code))
	{
	case PC:
	case RETURN:
	  return;
	case LABEL_REF:
	  set_label_offsets (((((x)->fld[1].rtx))->fld[ 0].rtx), insn, initial_p);
	  return;
	case IF_THEN_ELSE:
	  tem = ((((x)->fld[1].rtx))->fld[ 1].rtx);
	  if (((tem)->code) == LABEL_REF)
	    set_label_offsets (((tem)->fld[ 0].rtx), insn, initial_p);
	  else if (((tem)->code) != PC && ((tem)->code) != RETURN)
	    break;
	  tem = ((((x)->fld[1].rtx))->fld[ 2].rtx);
	  if (((tem)->code) == LABEL_REF)
	    set_label_offsets (((tem)->fld[ 0].rtx), insn, initial_p);
	  else if (((tem)->code) != PC && ((tem)->code) != RETURN)
	    break;
	  return;
	}
      for (p = reg_eliminate; p < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; p++)
	if (p->offset != p->initial_offset)
	  p->can_eliminate = 0;
    }
}
static struct rtvec_def *old_asm_operands_vec, *new_asm_operands_vec;
rtx
eliminate_regs (x, mem_mode, insn)
     rtx x;
     enum machine_mode mem_mode;
     rtx insn;
{
  enum rtx_code code = ((x)->code);
  struct elim_table *ep;
  int regno;
  rtx new;
  int i, j;
  char *fmt;
  int copied = 0;
  switch (code)
    {
    case CONST_INT:
    case CONST_DOUBLE:
    case CONST:
    case SYMBOL_REF:
    case CODE_LABEL:
    case PC:
    case CC0:
    case ASM_INPUT:
    case ADDR_VEC:
    case ADDR_DIFF_VEC:
    case RETURN:
      return x;
    case REG:
      regno = ((x)->fld[0].rtint);
      if (regno < 64)
	{
	  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])];
	       ep++)
	    if (ep->from_rtx == x && ep->can_eliminate)
	      {
		if (! mem_mode)
		  ep->ref_outside_mem = 1;
		return plus_constant_wide (ep->to_rtx, (int) ( ep->previous_offset));
	      }
	}
      else if (reg_equiv_memory_loc && reg_equiv_memory_loc[regno]
	       && (reg_equiv_address[regno] || num_not_at_initial_offset))
	{
	  new = eliminate_regs (reg_equiv_memory_loc[regno],
				mem_mode, (rtx) 0);
	  if (new != reg_equiv_memory_loc[regno])
	    return copy_rtx (new);
	}
      return x;
    case PLUS:
      if (((((x)->fld[ 0].rtx))->code) == REG
	  && ((((x)->fld[ 0].rtx))->fld[0].rtint) < 64
	  &&   (((((x)->fld[ 1].rtx))->code) == LABEL_REF || ((((x)->fld[ 1].rtx))->code) == SYMBOL_REF		   || ((((x)->fld[ 1].rtx))->code) == CONST_INT || ((((x)->fld[ 1].rtx))->code) == CONST_DOUBLE		   || ((((x)->fld[ 1].rtx))->code) == CONST || ((((x)->fld[ 1].rtx))->code) == HIGH))
	{
	  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])];
	       ep++)
	    if (ep->from_rtx == ((x)->fld[ 0].rtx) && ep->can_eliminate)
	      {
		if (! mem_mode)
		  ep->ref_outside_mem = 1;
		if (mem_mode != 0 && ((((x)->fld[ 1].rtx))->code) == CONST_INT
		    && ((((x)->fld[ 1].rtx))->fld[0].rtwint) == - ep->previous_offset)
		  return ep->to_rtx;
		else
		  return gen_rtx (PLUS, DImode, ep->to_rtx,
				  plus_constant_wide (((x)->fld[ 1].rtx), (int) (
						 ep->previous_offset)));
	      }
	  return x;
	}
      {
	rtx new0 = eliminate_regs (((x)->fld[ 0].rtx), mem_mode, (rtx) 0);
	rtx new1 = eliminate_regs (((x)->fld[ 1].rtx), mem_mode, (rtx) 0);
	if (new0 != ((x)->fld[ 0].rtx) || new1 != ((x)->fld[ 1].rtx))
	  {
	    if (((new0)->code) == PLUS && ((new1)->code) == REG
		&& ((new1)->fld[0].rtint) >= 64
		&& reg_renumber[((new1)->fld[0].rtint)] < 0
		&& reg_equiv_constant != 0
		&& reg_equiv_constant[((new1)->fld[0].rtint)] != 0)
	      new1 = reg_equiv_constant[((new1)->fld[0].rtint)];
	    else if (((new1)->code) == PLUS && ((new0)->code) == REG
		     && ((new0)->fld[0].rtint) >= 64
		     && reg_renumber[((new0)->fld[0].rtint)] < 0
		     && reg_equiv_constant[((new0)->fld[0].rtint)] != 0)
	      new0 = reg_equiv_constant[((new0)->fld[0].rtint)];
	    new = form_sum (new0, new1);
	    if (! mem_mode && ((new)->code) != PLUS)
	      return gen_rtx (PLUS, ((x)->mode), new, const0_rtx);
	    else
	      return new;
	  }
      }
      return x;
    case EXPR_LIST:
      if (((x)->fld[ 0].rtx))
	{
	  new = eliminate_regs (((x)->fld[ 0].rtx), mem_mode, (rtx) 0);
	  if (new != ((x)->fld[ 0].rtx))
	    x = gen_rtx (EXPR_LIST, ((enum reg_note) ((x)->mode)), new, ((x)->fld[ 1].rtx));
	}
    case INSN_LIST:
      if (((x)->fld[ 1].rtx))
	{
	  new = eliminate_regs (((x)->fld[ 1].rtx), mem_mode, (rtx) 0);
	  if (new != ((x)->fld[ 1].rtx))
	    return gen_rtx (INSN_LIST, ((x)->mode), ((x)->fld[ 0].rtx), new);
	}
      return x;
    case CALL:
    case COMPARE:
    case MINUS:
    case MULT:
    case DIV:      case UDIV:
    case MOD:      case UMOD:
    case AND:      case IOR:      case XOR:
    case LSHIFT:   case ASHIFT:   case ROTATE:
    case ASHIFTRT: case LSHIFTRT: case ROTATERT:
    case NE:       case EQ:
    case GE:       case GT:       case GEU:    case GTU:
    case LE:       case LT:       case LEU:    case LTU:
      {
	rtx new0 = eliminate_regs (((x)->fld[ 0].rtx), mem_mode, (rtx) 0);
	rtx new1
	  = ((x)->fld[ 1].rtx) ? eliminate_regs (((x)->fld[ 1].rtx), mem_mode, (rtx) 0) : 0;
	if (new0 != ((x)->fld[ 0].rtx) || new1 != ((x)->fld[ 1].rtx))
	  return gen_rtx (code, ((x)->mode), new0, new1);
      }
      return x;
    case PRE_INC:
    case POST_INC:
    case PRE_DEC:
    case POST_DEC:
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	if (ep->to_rtx == ((x)->fld[ 0].rtx))
	  {
	    if (code == PRE_DEC || code == POST_DEC)
	      ep->offset += (mode_size[(int)(mem_mode)]);
	    else
	      ep->offset -= (mode_size[(int)(mem_mode)]);
	  }
    case USE:
    case STRICT_LOW_PART:
    case NEG:          case NOT:
    case SIGN_EXTEND:  case ZERO_EXTEND:
    case TRUNCATE:     case FLOAT_EXTEND: case FLOAT_TRUNCATE:
    case FLOAT:        case FIX:
    case UNSIGNED_FIX: case UNSIGNED_FLOAT:
    case ABS:
    case SQRT:
    case FFS:
      new = eliminate_regs (((x)->fld[ 0].rtx), mem_mode, (rtx) 0);
      if (new != ((x)->fld[ 0].rtx))
	return gen_rtx (code, ((x)->mode), new);
      return x;
    case SUBREG:
      if (((((x)->fld[0].rtx))->code) == REG
	  && ((mode_size[(int)(((x)->mode))])
	      <= (mode_size[(int)(((((x)->fld[0].rtx))->mode))]))
	  && reg_equiv_memory_loc != 0
	  && reg_equiv_memory_loc[((((x)->fld[0].rtx))->fld[0].rtint)] != 0)
	{
	  new = eliminate_regs (reg_equiv_memory_loc[((((x)->fld[0].rtx))->fld[0].rtint)],
				mem_mode, (rtx) 0);
	  if (new == reg_equiv_memory_loc[((((x)->fld[0].rtx))->fld[0].rtint)])
	    new = ((x)->fld[ 0].rtx);
	  else
	    new = copy_rtx (new);
	}
      else
	new = eliminate_regs (((x)->fld[0].rtx), mem_mode, (rtx) 0);
      if (new != ((x)->fld[ 0].rtx))
	{
	  if (((new)->code) == MEM
	      && ((mode_size[(int)(((x)->mode))])
		  <= (mode_size[(int)(((new)->mode))])))
	    {
	      int offset = ((x)->fld[1].rtint) * 8;
	      enum machine_mode mode = ((x)->mode);
	      ((new)->mode = ( mode));
	      ((new)->fld[ 0].rtx) = plus_constant_wide (((new)->fld[ 0].rtx), (int) ( offset));
	      return new;
	    }
	  else
	    return gen_rtx (SUBREG, ((x)->mode), new, ((x)->fld[1].rtint));
	}
      return x;
    case CLOBBER:
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	if (ep->to_rtx == ((x)->fld[ 0].rtx))
	  ep->can_eliminate = 0;
      return x;
    case ASM_OPERANDS:
      {
	rtx *temp_vec;
	if ((((x))->fld[ 3].rtvec) != old_asm_operands_vec)
	  {
	    old_asm_operands_vec = (((x))->fld[ 3].rtvec);
	    temp_vec = (rtx *) alloca (((x)->fld[ 3].rtvec->num_elem) * sizeof (rtx));
	    for (i = 0; i < (((x))->fld[ 3].rtvec->num_elem); i++)
	      temp_vec[i] = eliminate_regs ((((x))->fld[ 3].rtvec->elem[ ( i)].rtx),
					    mem_mode, (rtx) 0);
	    for (i = 0; i < (((x))->fld[ 3].rtvec->num_elem); i++)
	      if (temp_vec[i] != (((x))->fld[ 3].rtvec->elem[ ( i)].rtx))
		break;
	    if (i == (((x))->fld[ 3].rtvec->num_elem))
	      new_asm_operands_vec = old_asm_operands_vec;
	    else
	      new_asm_operands_vec
		= gen_rtvec_v ((((x))->fld[ 3].rtvec->num_elem), temp_vec);
	  }
	if (new_asm_operands_vec == old_asm_operands_vec)
	  return x;
	new = gen_rtx (ASM_OPERANDS, VOIDmode, (((x))->fld[ 0].rtstr),
		       (((x))->fld[ 1].rtstr),
		       (((x))->fld[ 2].rtint), new_asm_operands_vec,
		       (((x))->fld[ 4].rtvec),
		       (((x))->fld[ 5].rtstr),
		       (((x))->fld[ 6].rtint));
	new->volatil = x->volatil;
	return new;
      }
    case SET:
      if (((((x)->fld[0].rtx))->code) == REG)
	{
	  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])];
	       ep++)
	    if (ep->to_rtx == ((x)->fld[0].rtx)
		&& ((x)->fld[0].rtx) != frame_pointer_rtx)
	      {
		rtx src = ((x)->fld[1].rtx);
		if (((src)->code) == PLUS
		    && ((src)->fld[ 0].rtx) == ((x)->fld[0].rtx)
		    && ((((src)->fld[ 1].rtx))->code) == CONST_INT)
		  ep->offset -= ((((src)->fld[ 1].rtx))->fld[0].rtwint);
		else
		  ep->can_eliminate = 0;
	      }
	  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])];
	       ep++)
	    if (ep->from_rtx == ((x)->fld[0].rtx) && ep->can_eliminate)
	      ep->can_eliminate = 0;
	}
      {
	rtx new0 = eliminate_regs (((x)->fld[0].rtx), 0, (rtx) 0);
	rtx new1 = eliminate_regs (((x)->fld[1].rtx), 0, (rtx) 0);
	if (((((x)->fld[0].rtx))->code) == REG && ((new0)->code) == MEM
	    && insn != 0)
	  emit_insn_after (gen_rtx (CLOBBER, VOIDmode, ((x)->fld[0].rtx)), insn);
	if (new0 != ((x)->fld[0].rtx) || new1 != ((x)->fld[1].rtx))
	  return gen_rtx (SET, VOIDmode, new0, new1);
      }
      return x;
    case MEM:
      new = eliminate_regs (((x)->fld[ 0].rtx), ((x)->mode), (rtx) 0);
      if (new != ((x)->fld[ 0].rtx))
	{
	  new = gen_rtx (MEM, ((x)->mode), new);
	  new->volatil = x->volatil;
	  new->unchanging = x->unchanging;
	  new->in_struct = x->in_struct;
	  return new;
	}
      else
	return x;
    }
  fmt = (rtx_format[(int)(code)]);
  for (i = 0; i < (rtx_length[(int)(code)]); i++, fmt++)
    {
      if (*fmt == 'e')
	{
	  new = eliminate_regs (((x)->fld[ i].rtx), mem_mode, (rtx) 0);
	  if (new != ((x)->fld[ i].rtx) && ! copied)
	    {
	      rtx new_x = rtx_alloc (code);
	      bcopy (x, new_x, (sizeof (*new_x) - sizeof (new_x->fld)
				+ (sizeof (new_x->fld[0])
				   * (rtx_length[(int)(code)]))));
	      x = new_x;
	      copied = 1;
	    }
	  ((x)->fld[ i].rtx) = new;
	}
      else if (*fmt == 'E')
	{
	  int copied_vec = 0;
	  for (j = 0; j < ((x)->fld[ i].rtvec->num_elem); j++)
	    {
	      new = eliminate_regs (((x)->fld[ i].rtvec->elem[ j].rtx), mem_mode, insn);
	      if (new != ((x)->fld[ i].rtvec->elem[ j].rtx) && ! copied_vec)
		{
		  rtvec new_v = gen_rtvec_v (((x)->fld[ i].rtvec->num_elem),
					     &((x)->fld[ i].rtvec->elem[ 0].rtx));
		  if (! copied)
		    {
		      rtx new_x = rtx_alloc (code);
		      bcopy (x, new_x, (sizeof (*new_x) - sizeof (new_x->fld)
					+ (sizeof (new_x->fld[0])
					   * (rtx_length[(int)(code)]))));
		      x = new_x;
		      copied = 1;
		    }
		  ((x)->fld[ i].rtvec) = new_v;
		  copied_vec = 1;
		}
	      ((x)->fld[ i].rtvec->elem[ j].rtx) = new;
	    }
	}
    }
  return x;
}
static int
eliminate_regs_in_insn (insn, replace)
     rtx insn;
     int replace;
{
  rtx old_body = ((insn)->fld[3].rtx);
  rtx new_body;
  int val = 0;
  struct elim_table *ep;
  if (! replace)
    push_obstacks (&reload_obstack, &reload_obstack);
  if (((old_body)->code) == SET && ((((old_body)->fld[0].rtx))->code) == REG
      && ((((old_body)->fld[0].rtx))->fld[0].rtint) < 64)
    {
      for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
	if (ep->from_rtx == ((old_body)->fld[0].rtx) && ep->can_eliminate)
	  {
	    if (replace)
	      delete_dead_insn (insn);
	    val = 1;
	    goto done;
	  }
      if (((((old_body)->fld[1].rtx))->code) == PLUS
	  && ((((((old_body)->fld[1].rtx))->fld[ 0].rtx))->code) == REG
	  && ((((((old_body)->fld[1].rtx))->fld[ 1].rtx))->code) == CONST_INT)
	for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])];
	     ep++)
	  if (ep->from_rtx == ((((old_body)->fld[1].rtx))->fld[ 0].rtx)
	      && ep->can_eliminate
	      && ep->offset == - ((((((old_body)->fld[1].rtx))->fld[ 1].rtx))->fld[0].rtwint))
	    {
	      ((insn)->fld[3].rtx) = gen_rtx (SET, VOIDmode,
					((old_body)->fld[0].rtx), ep->to_rtx);
	      ((insn)->fld[4].rtint) = -1;
	      val = 1;
	      goto done;
	    }
    }
  old_asm_operands_vec = 0;
  new_body = eliminate_regs (old_body, 0, replace ? insn : (rtx) 0);
  if (new_body != old_body)
    {
      if (! replace && asm_noperands (old_body) < 0)
	new_body = copy_rtx (new_body);
      if ((((old_body)->code) == SET && ((((old_body)->fld[1].rtx))->code) == REG
	   && (((new_body)->code) != SET
	       || ((((new_body)->fld[1].rtx))->code) != REG))
	  ||
	  (((old_body)->code) == SET
	   && ((((old_body)->fld[1].rtx))->code) == PLUS))
	{
	  if (! validate_change (insn, &((insn)->fld[3].rtx), new_body, 0))
	    ((insn)->fld[3].rtx) = new_body;
	}
      else
	((insn)->fld[3].rtx) = new_body;
      if (replace && ((insn)->fld[6].rtx))
	((insn)->fld[6].rtx) = eliminate_regs (((insn)->fld[6].rtx), 0, (rtx) 0);
      val = 1;
    }
  num_not_at_initial_offset = 0;
  for (ep = reg_eliminate; ep < &reg_eliminate[(sizeof reg_eliminate / sizeof reg_eliminate[0])]; ep++)
    {
      if (ep->previous_offset != ep->offset && ep->ref_outside_mem)
	ep->can_eliminate = 0;
      ep->ref_outside_mem = 0;
      if (ep->previous_offset != ep->offset)
	val = 1;
      ep->previous_offset = ep->offset;
      if (ep->can_eliminate && ep->offset != ep->initial_offset)
	num_not_at_initial_offset++;
      ep->max_offset = ((ep->max_offset) > ( ep->offset) ? (ep->max_offset) : ( ep->offset));
    }
 done:
  if (! replace)
    pop_obstacks ();
  return val;
}
static void
mark_not_eliminable (dest, x)
     rtx dest;
     rtx x;
{
  register int i;
  if (((dest)->code) == SUBREG)
    dest = ((dest)->fld[0].rtx);
  if (dest == frame_pointer_rtx)
    return;
  for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
    if (reg_eliminate[i].can_eliminate && dest == reg_eliminate[i].to_rtx
	&& (((x)->code) != SET
	    || ((((x)->fld[1].rtx))->code) != PLUS
	    || ((((x)->fld[1].rtx))->fld[ 0].rtx) != dest
	    || ((((((x)->fld[1].rtx))->fld[ 1].rtx))->code) != CONST_INT))
      {
	reg_eliminate[i].can_eliminate_previous
	  = reg_eliminate[i].can_eliminate = 0;
	num_eliminable--;
      }
}
static int
spill_hard_reg (regno, global, dumpfile, cant_eliminate)
     register int regno;
     int global;
     FILE *dumpfile;
     int cant_eliminate;
{
  int something_changed = 0;
  register int i;
    ((forbidden_regs)[( regno) / ((unsigned) 32)]	   |= (int) 1 << (( regno) % ((unsigned) 32)));
  for (i = 64; i < max_regno; i++)
    if (reg_renumber[i] >= 0
	&& reg_renumber[i] <= regno
	&& (reg_renumber[i]
	    +   (((mode_size[(int)(
				((regno_reg_rtx[i])->mode))]) + 8 - 1) / 8)
	    > regno))
      {
	enum reg_class class = ((regno) >= 32 ? FLOAT_REGS : GENERAL_REGS);
	if (! cant_eliminate
	    && basic_block_needs[0]
	    && reg_basic_block[i] >= 0
	    && basic_block_needs[(int) class][reg_basic_block[i]] == 0)
	  {
	    enum reg_class *p;
	    for (p = reg_class_superclasses[(int) class];
		 *p != LIM_REG_CLASSES; p++)
	      if (basic_block_needs[(int) *p][reg_basic_block[i]] > 0)
		break;
	    if (*p == LIM_REG_CLASSES)
	      continue;
	  }
	reg_renumber[i] = -1;
	something_changed = 1;
	if (global)
	    retry_global_alloc (i, forbidden_regs);
	alter_reg (i, regno);
	if (dumpfile)
	  {
	    if (reg_renumber[i] == -1)
	      fprintf (dumpfile, " Register %d now on stack.\n\n", i);
	    else
	      fprintf (dumpfile, " Register %d now in %d.\n\n",
		       i, reg_renumber[i]);
	  }
      }
  return something_changed;
}
static void
scan_paradoxical_subregs (x)
     register rtx x;
{
  register int i;
  register char *fmt;
  register enum rtx_code code = ((x)->code);
  switch (code)
    {
    case CONST_INT:
    case CONST:
    case SYMBOL_REF:
    case LABEL_REF:
    case CONST_DOUBLE:
    case CC0:
    case PC:
    case REG:
    case USE:
    case CLOBBER:
      return;
    case SUBREG:
      if (((((x)->fld[0].rtx))->code) == REG
	  && (mode_size[(int)(((x)->mode))]) > (mode_size[(int)(((((x)->fld[0].rtx))->mode))]))
	reg_max_ref_width[((((x)->fld[0].rtx))->fld[0].rtint)]
	  = (mode_size[(int)(((x)->mode))]);
      return;
    }
  fmt = (rtx_format[(int)(code)]);
  for (i = (rtx_length[(int)(code)]) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	scan_paradoxical_subregs (((x)->fld[ i].rtx));
      else if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = ((x)->fld[ i].rtvec->num_elem) - 1; j >=0; j--)
	    scan_paradoxical_subregs (((x)->fld[ i].rtvec->elem[ j].rtx));
	}
    }
}
struct hard_reg_n_uses { int regno; int uses; };
static int
hard_reg_use_compare (p1, p2)
     struct hard_reg_n_uses *p1, *p2;
{
  int tem = p1->uses - p2->uses;
  if (tem != 0) return tem;
  return p1->regno - p2->regno;
}
static void
order_regs_for_reload ()
{
  register int i;
  register int o = 0;
  int large = 0;
  struct hard_reg_n_uses hard_reg_n_uses[64];
  do { register int *scan_tp_ = (bad_spill_regs);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  for (i = 0; i < 64; i++)
    potential_reload_regs[i] = -1;
  for (i = 0; i < 64; i++)
    {
      hard_reg_n_uses[i].uses = 0;
      hard_reg_n_uses[i].regno = i;
    }
  for (i = 64; i < max_regno; i++)
    {
      int regno = reg_renumber[i];
      if (regno >= 0)
	{
	  int lim = regno +   (((mode_size[(int)( ((regno_reg_rtx[i])->mode))]) + 8 - 1) / 8);
	  while (regno < lim)
	    hard_reg_n_uses[regno++].uses += reg_n_refs[i];
	}
      large += reg_n_refs[i];
    }
  for (i = 0; i < 64; i++)
    {
      if (fixed_regs[i])
	{
	  hard_reg_n_uses[i].uses += 2 * large + 2;
	    ((bad_spill_regs)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	}
      else if (regs_explicitly_used[i])
	{
	  hard_reg_n_uses[i].uses += large + 1;
	    ((bad_spill_regs)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	}
    }
  hard_reg_n_uses[15].uses += 2 * large + 2;
    ((bad_spill_regs)[( 15) / ((unsigned) 32)]	   |= (int) 1 << (( 15) % ((unsigned) 32)));
  for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
    {
      hard_reg_n_uses[reg_eliminate[i].from].uses += 2 * large + 2;
        ((bad_spill_regs)[( reg_eliminate[i].from) / ((unsigned) 32)]	   |= (int) 1 << (( reg_eliminate[i].from) % ((unsigned) 32)));
    }
  for (i = 0; i < 64; i++)
    {
      int regno = reg_alloc_order[i];
      if (hard_reg_n_uses[regno].uses == 0)
	potential_reload_regs[o++] = regno;
    }
  qsort (hard_reg_n_uses, 64,
	 sizeof hard_reg_n_uses[0], hard_reg_use_compare);
  for (i = 0; i < 64; i++)
    if (hard_reg_n_uses[i].uses != 0)
      potential_reload_regs[o++] = hard_reg_n_uses[i].regno;
}
static void
reload_as_needed (first, live_known)
     rtx first;
     int live_known;
{
  register rtx insn;
  register int i;
  int this_block = 0;
  rtx x;
  rtx after_call = 0;
  bzero (spill_reg_rtx, sizeof spill_reg_rtx);
  reg_last_reload_reg = (rtx *) alloca (max_regno * sizeof (rtx));
  bzero (reg_last_reload_reg, max_regno * sizeof (rtx));
  reg_has_output_reload = (char *) alloca (max_regno);
  for (i = 0; i < n_spills; i++)
    {
      reg_reloaded_contents[i] = -1;
      reg_reloaded_insn[i] = 0;
    }
  for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
    {
      { if ((reg_eliminate[i].from) == 15 && ( reg_eliminate[i].to) == 30)	    (
				  reg_eliminate[i].initial_offset) = (get_frame_size () + current_function_outgoing_args_size 		+ current_function_pretend_args_size					+ alpha_sa_size () + 15) & ~ 15;			};
      reg_eliminate[i].previous_offset
	= reg_eliminate[i].offset = reg_eliminate[i].initial_offset;
    }
  num_not_at_initial_offset = 0;
  for (insn = first; insn;)
    {
      register rtx next = ((insn)->fld[2].rtx);
      if (live_known && this_block + 1 < n_basic_blocks
	  && insn == basic_block_head[this_block+1])
	++this_block;
      if (((insn)->code) == CODE_LABEL)
	{
	  num_not_at_initial_offset = 0;
	  for (i = 0; i < (sizeof reg_eliminate / sizeof reg_eliminate[0]); i++)
	    {
	      reg_eliminate[i].offset = reg_eliminate[i].previous_offset
		= offsets_at[((insn)->fld[3].rtint)][i];
	      if (reg_eliminate[i].can_eliminate
		  && (reg_eliminate[i].offset
		      != reg_eliminate[i].initial_offset))
		num_not_at_initial_offset++;
	    }
	}
      else if ((rtx_class[(int)(((insn)->code))]) == 'i')
	{
	  rtx avoid_return_reg = 0;
	  if ((((((insn)->fld[3].rtx))->code) == USE
	       || ((((insn)->fld[3].rtx))->code) == CLOBBER)
	      && ((((((insn)->fld[3].rtx))->fld[ 0].rtx))->code) == MEM)
	    ((((((insn)->fld[3].rtx))->fld[ 0].rtx))->fld[ 0].rtx)
	      = eliminate_regs (((((((insn)->fld[3].rtx))->fld[ 0].rtx))->fld[ 0].rtx),
				((((((insn)->fld[3].rtx))->fld[ 0].rtx))->mode), (rtx) 0);
	  if (num_eliminable && ((insn)->mode) == QImode)
	    {
	      eliminate_regs_in_insn (insn, 1);
	      if (((insn)->code) == NOTE)
		{
		  insn = next;
		  continue;
		}
	    }
	  if (((insn)->mode) == VOIDmode)
	    n_reloads = 0;
	  else
	    {
	      bzero (reg_has_output_reload, max_regno);
	      do { register int *scan_tp_ = (reg_is_output_reload);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
	      find_reloads (insn, 1, spill_indirect_levels, live_known,
			    spill_reg_order);
	    }
	  if (n_reloads > 0)
	    {
	      rtx prev = ((insn)->fld[1].rtx), next = ((insn)->fld[2].rtx);
	      rtx p;
	      int class;
	      for (class = 0; class < (int) LIM_REG_CLASSES; class++)
		if (basic_block_needs[class] != 0
		    && basic_block_needs[class][this_block] == 0)
		  for (i = 0; i < n_reloads; i++)
		    if (class == (int) reload_reg_class[i])
		      {
			if (reload_optional[i])
			  {
			    reload_in[i] = reload_out[i] = 0;
			    reload_secondary_p[i] = 0;
			  }
			else if (reload_reg_rtx[i] == 0
				 && (reload_in[i] != 0 || reload_out[i] != 0
				     || reload_secondary_p[i] != 0))
			  abort ();
		      }
	      choose_reload_regs (insn, avoid_return_reg);
	      emit_reload_insns (insn);
	      subst_reloads ();
	      if (asm_noperands (((insn)->fld[3].rtx)) >= 0)
		for (p = ((prev)->fld[2].rtx); p != next; p = ((p)->fld[2].rtx))
		  if (p != insn && (rtx_class[(int)(((p)->code))]) == 'i'
		      && (recog_memoized (p) < 0
			  || (insn_extract (p),
			      ! constrain_operands (((p)->fld[4].rtint), 1))))
		    {
		      error_for_asm (insn,
				     "`asm' operand requires impossible reload");
		      ((p)->code = ( NOTE));
		      ((p)->fld[3].rtstr) = 0;
		      ((p)->fld[4].rtint) = -1;
		    }
	    }
	  note_stores (((insn)->fld[3].rtx), forget_old_reloads_1);
	  for (x = ((insn)->fld[2].rtx); x != next; x = ((x)->fld[2].rtx))
	    if (((x)->code) == INSN && ((((x)->fld[3].rtx))->code) == CLOBBER)
	      note_stores (((x)->fld[3].rtx), forget_old_reloads_1);
	}
      if (((insn)->code) == CODE_LABEL)
	for (i = 0; i < n_spills; i++)
	  {
	    reg_reloaded_contents[i] = -1;
	    reg_reloaded_insn[i] = 0;
	  }
      if (((insn)->code) == CODE_LABEL || ((insn)->code) == CALL_INSN)
	for (i = 0; i < n_spills; i++)
	  if (call_used_regs[spill_regs[i]])
	    {
	      reg_reloaded_contents[i] = -1;
	      reg_reloaded_insn[i] = 0;
	    }
      insn = next;
    }
}
static void
forget_old_reloads_1 (x)
     rtx x;
{
  register int regno;
  int nr;
  int offset = 0;
  while (((x)->code) == SUBREG)
    {
      offset += ((x)->fld[1].rtint);
      x = ((x)->fld[0].rtx);
    }
  if (((x)->code) != REG)
    return;
  regno = ((x)->fld[0].rtint) + offset;
  if (regno >= 64)
    nr = 1;
  else
    {
      int i;
      nr =   (((mode_size[(int)( ((x)->mode))]) + 8 - 1) / 8);
      for (i = 0; i < nr; i++)
	if (spill_reg_order[regno + i] >= 0
	    && (n_reloads == 0
		|| !   ((reg_is_output_reload)[( regno + i) / ((unsigned) 32)]	   & ((int) 1 << (( regno + i) % ((unsigned) 32))))))
	  {
	    reg_reloaded_contents[spill_reg_order[regno + i]] = -1;
	    reg_reloaded_insn[spill_reg_order[regno + i]] = 0;
	  }
    }
  while (nr-- > 0)
    if (n_reloads == 0 || reg_has_output_reload[regno + nr] == 0)
      reg_last_reload_reg[regno + nr] = 0;
}
static enum machine_mode reload_mode[(2 * 10 * (1 + 1))];
static int reload_nregs[(2 * 10 * (1 + 1))];
static int
reload_reg_class_lower (p1, p2)
     short *p1, *p2;
{
  register int r1 = *p1, r2 = *p2;
  register int t;
  t = reload_optional[r1] - reload_optional[r2];
  if (t != 0)
    return t;
  t = ((reg_class_size[(int) reload_reg_class[r2]] == 1)
       - (reg_class_size[(int) reload_reg_class[r1]] == 1));
  if (t != 0)
    return t;
  t = reload_nregs[r2] - reload_nregs[r1];
  if (t != 0)
    return t;
  t = (int) reload_reg_class[r1] - (int) reload_reg_class[r2];
  if (t != 0)
    return t;
  return r1 - r2;
}
static HARD_REG_SET reload_reg_used;
static HARD_REG_SET reload_reg_used_in_input_addr;
static HARD_REG_SET reload_reg_used_in_output_addr;
static HARD_REG_SET reload_reg_used_in_op_addr;
static HARD_REG_SET reload_reg_used_in_input;
static HARD_REG_SET reload_reg_used_in_output;
static HARD_REG_SET reload_reg_used_at_all;
static void
mark_reload_reg_in_use (regno, when_needed, mode)
     int regno;
     enum reload_when_needed when_needed;
     enum machine_mode mode;
{
  int nregs =   (((mode_size[(int)( mode)]) + 8 - 1) / 8);
  int i;
  for (i = regno; i < nregs + regno; i++)
    {
      switch (when_needed)
	{
	case RELOAD_OTHER:
	    ((reload_reg_used)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	  break;
	case RELOAD_FOR_INPUT_RELOAD_ADDRESS:
	    ((reload_reg_used_in_input_addr)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	  break;
	case RELOAD_FOR_OUTPUT_RELOAD_ADDRESS:
	    ((reload_reg_used_in_output_addr)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	  break;
	case RELOAD_FOR_OPERAND_ADDRESS:
	    ((reload_reg_used_in_op_addr)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	  break;
	case RELOAD_FOR_INPUT:
	    ((reload_reg_used_in_input)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	  break;
	case RELOAD_FOR_OUTPUT:
	    ((reload_reg_used_in_output)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
	  break;
	}
        ((reload_reg_used_at_all)[( i) / ((unsigned) 32)]	   |= (int) 1 << (( i) % ((unsigned) 32)));
    }
}
static int
reload_reg_free_p (regno, when_needed)
     int regno;
     enum reload_when_needed when_needed;
{
  if (  ((reload_reg_used)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
    return 0;
  switch (when_needed)
    {
    case RELOAD_OTHER:
      return !   ((reload_reg_used_at_all)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))));
    case RELOAD_FOR_INPUT:
      return (!   ((reload_reg_used_in_input)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_op_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_input_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))));
    case RELOAD_FOR_INPUT_RELOAD_ADDRESS:
      return (!   ((reload_reg_used_in_input_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_input)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))));
    case RELOAD_FOR_OUTPUT_RELOAD_ADDRESS:
      return (!   ((reload_reg_used_in_output_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_output)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))));
    case RELOAD_FOR_OPERAND_ADDRESS:
      return (!   ((reload_reg_used_in_op_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_input)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_output)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))));
    case RELOAD_FOR_OUTPUT:
      return (!   ((reload_reg_used_in_op_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_output_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	      && !   ((reload_reg_used_in_output)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))));
    }
  abort ();
}
static int
reload_reg_free_before_p (regno, when_needed)
     int regno;
     enum reload_when_needed when_needed;
{
  switch (when_needed)
    {
    case RELOAD_OTHER:
      return 1;
    case RELOAD_FOR_OUTPUT_RELOAD_ADDRESS:
      if (  ((reload_reg_used_in_op_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	return 0;
    case RELOAD_FOR_OUTPUT:
      if (  ((reload_reg_used_in_input)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	return 0;
    case RELOAD_FOR_OPERAND_ADDRESS:
      if (  ((reload_reg_used_in_input_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	return 0;
    case RELOAD_FOR_INPUT_RELOAD_ADDRESS:
    case RELOAD_FOR_INPUT:
      return 1;
    }
  abort ();
}
static int
reload_reg_reaches_end_p (regno, when_needed)
     int regno;
     enum reload_when_needed when_needed;
{
  switch (when_needed)
    {
    case RELOAD_OTHER:
      return 1;
    case RELOAD_FOR_INPUT_RELOAD_ADDRESS:
    case RELOAD_FOR_INPUT:
      if (  ((reload_reg_used_in_op_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
	  ||   ((reload_reg_used_in_output)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	return 0;
    case RELOAD_FOR_OPERAND_ADDRESS:
      if (  ((reload_reg_used_in_output_addr)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	return 0;
    case RELOAD_FOR_OUTPUT:
    case RELOAD_FOR_OUTPUT_RELOAD_ADDRESS:
      return 1;
    }
  abort ();
}
short reload_order[(2 * 10 * (1 + 1))];
char reload_inherited[(2 * 10 * (1 + 1))];
rtx reload_inheritance_insn[(2 * 10 * (1 + 1))];
rtx reload_override_in[(2 * 10 * (1 + 1))];
int reload_spill_index[(2 * 10 * (1 + 1))];
static last_spill_reg = 0;
static int
allocate_reload_reg (r, insn, last_reload, noerror)
     int r;
     rtx insn;
     int last_reload;
     int noerror;
{
  int i;
  int pass;
  int count;
  rtx new;
  int regno;
  int force_group = reload_nregs[r] > 1 && ! last_reload;
  for (pass = 0; pass < 2; pass++)
    {
      for (count = 0, i = last_spill_reg; count < n_spills; count++)
	{
	  int class = (int) reload_reg_class[r];
	  i = (i + 1) % n_spills;
	  if (reload_reg_free_p (spill_regs[i], reload_when_needed[r])
	      &&   ((reg_class_contents[class])[( spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << (( spill_regs[i]) % ((unsigned) 32))))
	      && 1
	      && (pass ||   ((reload_reg_used_at_all)[(
					     spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << ((
					     spill_regs[i]) % ((unsigned) 32))))))
	    {
	      int nr =   (((mode_size[(int)( reload_mode[r])]) + 8 - 1) / 8);
	      if (force_group)
		nr =  (((mode_size[(int)( reload_mode[r])]) + 8 - 1) / 8);
	      if (nr == 1)
		{
		  if (force_group)
		    continue;
		  break;
		}
	      if (!   ((counted_for_nongroups)[( spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << (( spill_regs[i]) % ((unsigned) 32)))))
		while (nr > 1)
		  {
		    regno = spill_regs[i] + nr - 1;
		    if (!(  ((reg_class_contents[class])[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32))))
			  && spill_reg_order[regno] >= 0
			  && reload_reg_free_p (regno, reload_when_needed[r])
			  && !   ((counted_for_nongroups)[(
						  regno) / ((unsigned) 32)]	   & ((int) 1 << ((
						  regno) % ((unsigned) 32))))))
		      break;
		    nr--;
		  }
	      if (nr == 1)
		break;
	    }
	}
      if (count < n_spills)
	break;
    }
  if (count == n_spills)
    {
      if (noerror)
	return 0;
      goto failure;
    }
  last_spill_reg = i;
  mark_reload_reg_in_use (spill_regs[i], reload_when_needed[r],
			  reload_mode[r]);
  new = spill_reg_rtx[i];
  if (new == 0 || ((new)->mode) != reload_mode[r])
    spill_reg_rtx[i] = new = gen_rtx (REG, reload_mode[r], spill_regs[i]);
  reload_reg_rtx[r] = new;
  reload_spill_index[r] = i;
  regno = true_regnum (new);
  if (1)
    {
      enum machine_mode test_mode = VOIDmode;
      if (reload_in[r])
	test_mode = ((reload_in[r])->mode);
      if (! (reload_in[r] != 0 && test_mode != VOIDmode
	     && ! 1))
	if (! (reload_out[r] != 0
	       && ! 1))
	  return 1;
    }
  if (noerror)
    return 0;
 failure:
  if (asm_noperands (((insn)->fld[3].rtx)) < 0)
    abort ();
  error_for_asm (insn,
		 "`asm' operand constraint incompatible with operand size");
  reload_in[r] = 0;
  reload_out[r] = 0;
  reload_reg_rtx[r] = 0;
  reload_optional[r] = 1;
  reload_secondary_p[r] = 1;
  return 1;
}
static void
choose_reload_regs (insn, avoid_return_reg)
     rtx insn;
     rtx avoid_return_reg;
{
  register int i, j;
  int max_group_size = 1;
  enum reg_class group_class = NO_REGS;
  int inheritance;
  rtx save_reload_reg_rtx[(2 * 10 * (1 + 1))];
  char save_reload_inherited[(2 * 10 * (1 + 1))];
  rtx save_reload_inheritance_insn[(2 * 10 * (1 + 1))];
  rtx save_reload_override_in[(2 * 10 * (1 + 1))];
  int save_reload_spill_index[(2 * 10 * (1 + 1))];
  HARD_REG_SET save_reload_reg_used;
  HARD_REG_SET save_reload_reg_used_in_input_addr;
  HARD_REG_SET save_reload_reg_used_in_output_addr;
  HARD_REG_SET save_reload_reg_used_in_op_addr;
  HARD_REG_SET save_reload_reg_used_in_input;
  HARD_REG_SET save_reload_reg_used_in_output;
  HARD_REG_SET save_reload_reg_used_at_all;
  bzero (reload_inherited, (2 * 10 * (1 + 1)));
  bzero (reload_inheritance_insn, (2 * 10 * (1 + 1)) * sizeof (rtx));
  bzero (reload_override_in, (2 * 10 * (1 + 1)) * sizeof (rtx));
  do { register int *scan_tp_ = (reload_reg_used);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  do { register int *scan_tp_ = (reload_reg_used_at_all);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  do { register int *scan_tp_ = (reload_reg_used_in_input_addr);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  do { register int *scan_tp_ = (reload_reg_used_in_output_addr);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  do { register int *scan_tp_ = (reload_reg_used_in_op_addr);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  do { register int *scan_tp_ = (reload_reg_used_in_output);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  do { register int *scan_tp_ = (reload_reg_used_in_input);			     register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = 0; } while (0);
  for (j = 0; j < n_reloads; j++)
    if (reload_when_needed[j] == RELOAD_OTHER
	&& ! reload_needed_for_multiple[j])
      {
	if (reload_in[j] == 0)
	  {
	    for (i = 0; i < n_earlyclobbers; i++)
	      if (rtx_equal_p (reload_out[j], reload_earlyclobbers[i]))
		break;
	    if (i == n_earlyclobbers)
	      reload_when_needed[j] = RELOAD_FOR_OUTPUT;
	  }
	if (reload_out[j] == 0)
	  reload_when_needed[j] = RELOAD_FOR_INPUT;
	if (reload_secondary_reload[j] >= 0
	    && ! reload_needed_for_multiple[reload_secondary_reload[j]])
	  reload_when_needed[reload_secondary_reload[j]]
	    = reload_when_needed[j];
      }
  for (j = 0; j < n_reloads; j++)
    {
      reload_order[j] = j;
      reload_spill_index[j] = -1;
      reload_mode[j]
	= (reload_strict_low[j] && reload_out[j]
	   ? ((((reload_out[j])->fld[0].rtx))->mode)
	   : (reload_inmode[j] == VOIDmode
	      || ((mode_size[(int)(reload_outmode[j])])
		  > (mode_size[(int)(reload_inmode[j])])))
	   ? reload_outmode[j] : reload_inmode[j]);
      reload_nregs[j] =  (((mode_size[(int)( reload_mode[j])]) + 8 - 1) / 8);
      if (reload_nregs[j] > 1)
	{
	  max_group_size = ((reload_nregs[j]) > ( max_group_size) ? (reload_nregs[j]) : ( max_group_size));
	  group_class = reg_class_superunion[(int)reload_reg_class[j]][(int)group_class];
	}
      if (reload_reg_rtx[j])
	mark_reload_reg_in_use (((reload_reg_rtx[j])->fld[0].rtint),
				reload_when_needed[j], reload_mode[j]);
    }
  if (n_reloads > 1)
    qsort (reload_order, n_reloads, sizeof (short), reload_reg_class_lower);
  bcopy (reload_reg_rtx, save_reload_reg_rtx, sizeof reload_reg_rtx);
  bcopy (reload_inherited, save_reload_inherited, sizeof reload_inherited);
  bcopy (reload_inheritance_insn, save_reload_inheritance_insn,
	 sizeof reload_inheritance_insn);
  bcopy (reload_override_in, save_reload_override_in,
	 sizeof reload_override_in);
  bcopy (reload_spill_index, save_reload_spill_index,
	 sizeof reload_spill_index);
  do { register int *scan_tp_ = (save_reload_reg_used), *scan_fp_ = ( reload_reg_used);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  do { register int *scan_tp_ = (save_reload_reg_used_at_all), *scan_fp_ = ( reload_reg_used_at_all);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  do { register int *scan_tp_ = (save_reload_reg_used_in_output), *scan_fp_ = (
		     reload_reg_used_in_output);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  do { register int *scan_tp_ = (save_reload_reg_used_in_input), *scan_fp_ = (
		     reload_reg_used_in_input);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  do { register int *scan_tp_ = (save_reload_reg_used_in_input_addr), *scan_fp_ = (
		     reload_reg_used_in_input_addr);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  do { register int *scan_tp_ = (save_reload_reg_used_in_output_addr), *scan_fp_ = (
		     reload_reg_used_in_output_addr);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  do { register int *scan_tp_ = (save_reload_reg_used_in_op_addr), *scan_fp_ = (
		     reload_reg_used_in_op_addr);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
  for (inheritance = optimize > 0; inheritance >= 0; inheritance--)
    {
      for (j = 0; j < n_reloads; j++)
	{
	  register int r = reload_order[j];
	  if (reload_out[r] == 0 && reload_in[r] == 0 && ! reload_secondary_p[r])
	    continue;
	  if (reload_in[r] != 0 && reload_reg_rtx[r] != 0
	      && (rtx_equal_p (reload_in[r], reload_reg_rtx[r])
		  || rtx_equal_p (reload_out[r], reload_reg_rtx[r])))
	    continue;
	  if (inheritance)
	    {
	      register int regno = -1;
	      enum machine_mode mode;
	      if (reload_in[r] == 0)
		;
	      else if (((reload_in[r])->code) == REG)
		{
		  regno = ((reload_in[r])->fld[0].rtint);
		  mode = ((reload_in[r])->mode);
		}
	      else if (((reload_in_reg[r])->code) == REG)
		{
		  regno = ((reload_in_reg[r])->fld[0].rtint);
		  mode = ((reload_in_reg[r])->mode);
		}
	      if (regno >= 0 && reg_last_reload_reg[regno] != 0)
		{
		  i = spill_reg_order[((reg_last_reload_reg[regno])->fld[0].rtint)];
		  if (reg_reloaded_contents[i] == regno
		      && ((mode_size[(int)(((reg_last_reload_reg[regno])->mode))])
			  >= (mode_size[(int)(mode)]))
		      && 1
		      &&   ((reg_class_contents[(int) reload_reg_class[r]])[(
					    spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << ((
					    spill_regs[i]) % ((unsigned) 32))))
		      && (reload_nregs[r] == max_group_size
			  || !   ((reg_class_contents[(int) group_class])[(
						  spill_regs[i]) / ((unsigned) 32)]	   & ((int) 1 << ((
						  spill_regs[i]) % ((unsigned) 32)))))
		      && reload_reg_free_p (spill_regs[i], reload_when_needed[r])
		      && reload_reg_free_before_p (spill_regs[i],
						   reload_when_needed[r]))
		    {
		      int nr
			=   (((mode_size[(int)( reload_mode[r])]) + 8 - 1) / 8);
		      int k;
		      for (k = 1; k < nr; k++)
			if (reg_reloaded_contents[spill_reg_order[spill_regs[i] + k]]
			    != regno)
			  break;
		      if (k == nr)
			{
			  mark_reload_reg_in_use (spill_regs[i],
						  reload_when_needed[r],
						  reload_mode[r]);
			  reload_reg_rtx[r] = reg_last_reload_reg[regno];
			  reload_inherited[r] = 1;
			  reload_inheritance_insn[r] = reg_reloaded_insn[i];
			  reload_spill_index[r] = i;
			}
		    }
		}
	    }
	  if (inheritance
	      && reload_in[r] != 0
	      && ! reload_inherited[r]
	      && reload_out[r] == 0
	      && (  (((reload_in[r])->code) == LABEL_REF || ((reload_in[r])->code) == SYMBOL_REF		   || ((reload_in[r])->code) == CONST_INT || ((reload_in[r])->code) == CONST_DOUBLE		   || ((reload_in[r])->code) == CONST || ((reload_in[r])->code) == HIGH)
		  || ((reload_in[r])->code) == PLUS
		  || ((reload_in[r])->code) == REG
		  || ((reload_in[r])->code) == MEM)
	      && (reload_nregs[r] == max_group_size
		  || ! reg_classes_intersect_p (reload_reg_class[r], group_class)))
	    {
	      register rtx equiv
		= find_equiv_reg (reload_in[r], insn, reload_reg_class[r],
				  -1, ((char *)0), 0, reload_mode[r]);
	      int regno;
	      if (equiv != 0)
		{
		  if (((equiv)->code) == REG)
		    regno = ((equiv)->fld[0].rtint);
		  else if (((equiv)->code) == SUBREG)
		    {
		      regno = ((((equiv)->fld[0].rtx))->fld[0].rtint);
		      if (regno < 64)
			regno += ((equiv)->fld[1].rtint);
		    }
		  else
		    abort ();
		}
	      if (equiv != 0
		  && ((spill_reg_order[regno] >= 0
		       && ! reload_reg_free_before_p (regno,
						      reload_when_needed[r]))
		      || !   ((reg_class_contents[(int) reload_reg_class[r]])[(
					      regno) / ((unsigned) 32)]	   & ((int) 1 << ((
					      regno) % ((unsigned) 32))))))
		equiv = 0;
	      if (equiv != 0 &&   ((reload_reg_used_at_all)[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
		equiv = 0;
	      if (equiv != 0 && ! 1)
		equiv = 0;
	      if (equiv != 0)
		for (i = 0; i < n_earlyclobbers; i++)
		  if (reg_overlap_mentioned_for_reload_p (equiv,
							  reload_earlyclobbers[i]))
		    {
		      reload_override_in[r] = equiv;
		      equiv = 0;
		      break;
		    }
	      if (equiv != 0 && regno_clobbered_p (regno, insn))
		{
		  reload_override_in[r] = equiv;
		  equiv = 0;
		}
	      if (equiv != 0 && regno != 15)
		{
		  reload_reg_rtx[r] = equiv;
		  reload_inherited[r] = 1;
		  i = spill_reg_order[regno];
		  if (i >= 0)
		    mark_reload_reg_in_use (regno, reload_when_needed[r],
					    reload_mode[r]);
		}
	    }
	  if (reload_reg_rtx[r] != 0 || reload_optional[r] != 0)
	    continue;
	}
      for (j = 0; j < n_reloads; j++)
	{
	  register int r = reload_order[j];
	  if (reload_out[r] == 0 && reload_in[r] == 0 && ! reload_secondary_p[r])
	    continue;
	  if (reload_reg_rtx[r] != 0 || reload_optional[r])
	    continue;
	  if (! allocate_reload_reg (r, insn, j == n_reloads - 1, inheritance))
	    break;
	}
      if (j == n_reloads)
	break;
    fail:
      bcopy (save_reload_reg_rtx, reload_reg_rtx, sizeof reload_reg_rtx);
      bcopy (save_reload_inherited, reload_inherited, sizeof reload_inherited);
      bcopy (save_reload_inheritance_insn, reload_inheritance_insn,
	     sizeof reload_inheritance_insn);
      bcopy (save_reload_override_in, reload_override_in,
	     sizeof reload_override_in);
      bcopy (save_reload_spill_index, reload_spill_index,
	     sizeof reload_spill_index);
      do { register int *scan_tp_ = (reload_reg_used), *scan_fp_ = ( save_reload_reg_used);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
      do { register int *scan_tp_ = (reload_reg_used_at_all), *scan_fp_ = ( save_reload_reg_used_at_all);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
      do { register int *scan_tp_ = (reload_reg_used_in_input), *scan_fp_ = (
			 save_reload_reg_used_in_input);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
      do { register int *scan_tp_ = (reload_reg_used_in_output), *scan_fp_ = (
			 save_reload_reg_used_in_output);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
      do { register int *scan_tp_ = (reload_reg_used_in_input_addr), *scan_fp_ = (
			 save_reload_reg_used_in_input_addr);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
      do { register int *scan_tp_ = (reload_reg_used_in_output_addr), *scan_fp_ = (
			 save_reload_reg_used_in_output_addr);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
      do { register int *scan_tp_ = (reload_reg_used_in_op_addr), *scan_fp_ = (
			 save_reload_reg_used_in_op_addr);      register int i;						     for (i = 0; i <  ((64 + 32 - 1)	  / 32); i++)			       *scan_tp_++ = *scan_fp_++; } while (0);
    }
  for (j = 0; j < n_reloads; j++)
    {
      register int r = reload_order[j];
      if (reload_inherited[r] && reload_reg_rtx[r] != 0
	  && ! reload_reg_free_before_p (true_regnum (reload_reg_rtx[r]),
					 reload_when_needed[r]))
	reload_inherited[r] = 0;
      if (reload_override_in[r]
	  && (((reload_override_in[r])->code) == REG
	      || ((reload_override_in[r])->code) == SUBREG))
	{
	  int regno = true_regnum (reload_override_in[r]);
	  if (spill_reg_order[regno] >= 0
	      && ! reload_reg_free_before_p (regno, reload_when_needed[r]))
	    reload_override_in[r] = 0;
	}
    }
  for (j = 0; j < n_reloads; j++)
    if (reload_override_in[j])
      reload_in[j] = reload_override_in[j];
  for (j = 0; j < n_reloads; j++)
    if ((reload_optional[j] && ! reload_inherited[j])
	|| (reload_in[j] == 0 && reload_out[j] == 0
	    && ! reload_secondary_p[j]))
      reload_reg_rtx[j] = 0;
  for (j = 0; j < n_reloads; j++)
    {
      register int r = reload_order[j];
      i = reload_spill_index[r];
      if (reload_out[r] != 0 && ((reload_out[r])->code) == REG
	  && reload_reg_rtx[r] != 0)
	{
	  register int nregno = ((reload_out[r])->fld[0].rtint);
	  int nr = 1;
	  if (nregno < 64)
	    nr =   (((mode_size[(int)( reload_mode[r])]) + 8 - 1) / 8);
	  while (--nr >= 0)
	    reg_has_output_reload[nregno + nr] = 1;
	  if (i >= 0)
	    {
	      nr =   (((mode_size[(int)( reload_mode[r])]) + 8 - 1) / 8);
	      while (--nr >= 0)
		  ((reg_is_output_reload)[( spill_regs[i] + nr) / ((unsigned) 32)]	   |= (int) 1 << (( spill_regs[i] + nr) % ((unsigned) 32)));
	    }
	  if (reload_when_needed[r] != RELOAD_OTHER
	      && reload_when_needed[r] != RELOAD_FOR_OUTPUT)
	    abort ();
	}
    }
}
static void
emit_reload_insns (insn)
     rtx insn;
{
  register int j;
  rtx following_insn = ((insn)->fld[2].rtx);
  rtx before_insn = insn;
  rtx first_output_reload_insn = ((insn)->fld[2].rtx);
  rtx first_other_reload_insn = insn;
  rtx first_operand_address_reload_insn = insn;
  int special;
  rtx new_spill_reg_store[64];
  if (((insn)->code) == CALL_INSN && ((((insn)->fld[1].rtx))->code) == INSN
      && ((((((insn)->fld[1].rtx))->fld[3].rtx))->code) == USE)
    while (((((before_insn)->fld[1].rtx))->code) == INSN
	   && ((((((before_insn)->fld[1].rtx))->fld[3].rtx))->code) == USE)
      first_other_reload_insn = first_operand_address_reload_insn
	= before_insn = ((before_insn)->fld[1].rtx);
  for (j = 0; j < n_reloads; j++)
    {
      register rtx old;
      rtx oldequiv_reg = 0;
      rtx this_reload_insn = 0;
      rtx store_insn = 0;
      old = reload_in[j];
      if (old != 0 && ! reload_inherited[j]
	  && ! rtx_equal_p (reload_reg_rtx[j], old)
	  && reload_reg_rtx[j] != 0)
	{
	  register rtx reloadreg = reload_reg_rtx[j];
	  rtx oldequiv = 0;
	  enum machine_mode mode;
	  rtx where;
	  rtx reload_insn;
	  mode = ((old)->mode);
	  if (mode == VOIDmode)
	    mode = reload_inmode[j];
	  if (reload_strict_low[j])
	    mode = ((((reload_in[j])->fld[0].rtx))->mode);
	  if (reload_secondary_reload[j] >= 0
	      && reload_secondary_icode[j] == CODE_FOR_nothing
	      && optimize)
	    oldequiv
	      = find_equiv_reg (old, insn,
				reload_reg_class[reload_secondary_reload[j]],
				-1, ((char *)0), 0, mode);
	  if (oldequiv == 0 && optimize
	      && (((old)->code) == MEM
		  || (((old)->code) == REG
		      && ((old)->fld[0].rtint) >= 64
		      && reg_renumber[((old)->fld[0].rtint)] < 0)))
	    oldequiv = find_equiv_reg (old, insn, GENERAL_REGS,
				       -1, ((char *)0), 0, mode);
	  if (oldequiv)
	    {
	      int regno = true_regnum (oldequiv);
	      if (spill_reg_order[regno] >= 0
		  && (! reload_reg_free_p (regno, reload_when_needed[j])
		      || ! reload_reg_free_before_p (regno,
						     reload_when_needed[j])))
		oldequiv = 0;
	      if (spill_reg_order[regno] < 0)
		{
		  int k;
		  for (k = 0; k < n_reloads; k++)
		    if (reload_reg_rtx[k] != 0 && k != j
			&& reg_overlap_mentioned_for_reload_p (reload_reg_rtx[k],
							       oldequiv))
		      {
			oldequiv = 0;
			break;
		      }
		}
	    }
	  if (oldequiv == 0)
	    oldequiv = old;
	  else if (((oldequiv)->code) == REG)
	    oldequiv_reg = oldequiv;
	  else if (((oldequiv)->code) == SUBREG)
	    oldequiv_reg = ((oldequiv)->fld[0].rtx);
	  if (((reloadreg)->mode) != mode)
	    reloadreg = gen_rtx (REG, mode, ((reloadreg)->fld[0].rtint));
	  while (((oldequiv)->code) == SUBREG && ((oldequiv)->mode) != mode)
	    oldequiv = ((oldequiv)->fld[0].rtx);
	  if (((oldequiv)->mode) != VOIDmode
	      && mode != ((oldequiv)->mode))
	    oldequiv = gen_rtx (SUBREG, mode, oldequiv, 0);
	  switch (reload_when_needed[j])
	    {
	    case RELOAD_FOR_INPUT:
	    case RELOAD_OTHER:
	      where = first_operand_address_reload_insn;
	      break;
	    case RELOAD_FOR_INPUT_RELOAD_ADDRESS:
	      where = first_other_reload_insn;
	      break;
	    case RELOAD_FOR_OUTPUT_RELOAD_ADDRESS:
	      where = first_output_reload_insn;
	      break;
	    case RELOAD_FOR_OPERAND_ADDRESS:
	      where = before_insn;
	    }
	  special = 0;
	  if (((oldequiv)->code) == POST_INC
	      || ((oldequiv)->code) == POST_DEC
	      || ((oldequiv)->code) == PRE_INC
	      || ((oldequiv)->code) == PRE_DEC)
	    {
	      if (reload_secondary_reload[j] >= 0)
		abort ();
	      special = 1;
	      this_reload_insn
		= inc_for_reload (reloadreg, oldequiv, reload_inc[j], where);
	    }
	  else if (optimize && ((old)->code) == REG
		   && ((old)->fld[0].rtint) >= 64
		   && dead_or_set_p (insn, old)
		   && (reload_when_needed[j] == RELOAD_OTHER
		       || reload_when_needed[j] == RELOAD_FOR_INPUT
		       || reload_when_needed[j] == RELOAD_FOR_INPUT_RELOAD_ADDRESS))
	    {
	      rtx temp = ((insn)->fld[1].rtx);
	      while (temp && ((temp)->code) == NOTE)
		temp = ((temp)->fld[1].rtx);
	      if (temp
		  && ((temp)->code) == INSN
		  && ((((temp)->fld[3].rtx))->code) == SET
		  && ((((temp)->fld[3].rtx))->fld[0].rtx) == old
		  && asm_noperands (((temp)->fld[3].rtx)) < 0
		  && constraint_accepts_reg_p (insn_operand_constraint[recog_memoized (temp)][0],
					       reloadreg)
		  && count_occurrences (((insn)->fld[3].rtx), old) == 1
		  && ! reg_mentioned_p (old, ((((temp)->fld[3].rtx))->fld[1].rtx)))
		{
		  ((((temp)->fld[3].rtx))->fld[0].rtx) = reloadreg;
		  if (reg_n_deaths[((old)->fld[0].rtint)] == 1
		      && reg_n_sets[((old)->fld[0].rtint)] == 1)
		    {
		      reg_renumber[((old)->fld[0].rtint)] = ((reload_reg_rtx[j])->fld[0].rtint);
		      alter_reg (((old)->fld[0].rtint), -1);
		    }
		  special = 1;
		}
	    }
	  if (! special)
	    {
	      rtx second_reload_reg = 0;
	      enum insn_code icode;
	      if (reload_secondary_reload[j] >= 0)
		{
		  int secondary_reload = reload_secondary_reload[j];
		  rtx real_oldequiv = oldequiv;
		  rtx real_old = old;
		  if (((oldequiv)->code) == REG
		      && ((oldequiv)->fld[0].rtint) >= 64
		      && reg_equiv_mem[((oldequiv)->fld[0].rtint)] != 0)
		    real_oldequiv = reg_equiv_mem[((oldequiv)->fld[0].rtint)];
		  if (((old)->code) == REG
		      && ((old)->fld[0].rtint) >= 64
		      && reg_equiv_mem[((old)->fld[0].rtint)] != 0)
		    real_old = reg_equiv_mem[((old)->fld[0].rtint)];
		  second_reload_reg = reload_reg_rtx[secondary_reload];
		  icode = reload_secondary_icode[j];
		  if ((old != oldequiv && ! rtx_equal_p (old, oldequiv))
		      || (reload_in[j] != 0 && reload_out[j] != 0))
		    {
		      enum reg_class new_class
			= ((((( real_oldequiv)->code) == MEM 						   || ((( real_oldequiv)->code) == REG && (( real_oldequiv)->fld[0].rtint) >= 64)	   || ((( real_oldequiv)->code) == SUBREG						       && ((((( real_oldequiv)->fld[0].rtx))->code) == MEM					   || ((((( real_oldequiv)->fld[0].rtx))->code) == REG				       && (((( real_oldequiv)->fld[0].rtx))->fld[0].rtint) >= 64))))	  && (((reload_reg_class[j]) == FLOAT_REGS						       && ((
							mode) == SImode || (
							mode) == HImode || (
							mode) == QImode))	      || (((
							mode) == QImode || (
							mode) == HImode)				  && unaligned_memory_operand ( real_oldequiv, 
							mode))))			 ? GENERAL_REGS : NO_REGS);
		      if (new_class == NO_REGS)
			second_reload_reg = 0;
		      else
			{
			  enum insn_code new_icode;
			  enum machine_mode new_mode;
			  if (!   ((reg_class_contents[(int) new_class])[(
						   ((second_reload_reg)->fld[0].rtint)) / ((unsigned) 32)]	   & ((int) 1 << ((
						   ((second_reload_reg)->fld[0].rtint)) % ((unsigned) 32)))))
			    oldequiv = old, real_oldequiv = real_old;
			  else
			    {
			      new_icode = reload_in_optab[(int) mode];
			      if (new_icode != CODE_FOR_nothing
				  && ((insn_operand_predicate[(int) new_icode][0]
				       && ! ((*insn_operand_predicate[(int) new_icode][0])
					     (reloadreg, mode)))
				      || (insn_operand_predicate[(int) new_icode][1]
					  && ! ((*insn_operand_predicate[(int) new_icode][1])
						(real_oldequiv, mode)))))
				new_icode = CODE_FOR_nothing;
			      if (new_icode == CODE_FOR_nothing)
				new_mode = mode;
			      else
				new_mode = insn_operand_mode[new_icode][2];
			      if (((second_reload_reg)->mode) != new_mode)
				{
				  if (!1)
				    oldequiv = old, real_oldequiv = real_old;
				  else
				    second_reload_reg
				      = gen_rtx (REG, new_mode,
						 ((second_reload_reg)->fld[0].rtint));
				}
			    }
			}
		    }
		  if (second_reload_reg)
		    {
		      if (icode != CODE_FOR_nothing)
			{
			  reload_insn = emit_insn_before ((*insn_gen_function[(int) (icode)])
							  (reloadreg,
							   real_oldequiv,
							   second_reload_reg),
							  where);
			  if (this_reload_insn == 0)
			    this_reload_insn = reload_insn;
			  special = 1;
			}
		      else
			{
			  enum insn_code tertiary_icode
			    = reload_secondary_icode[secondary_reload];
			  if (tertiary_icode != CODE_FOR_nothing)
			    {
			      rtx third_reload_reg
			        = reload_reg_rtx[reload_secondary_reload[secondary_reload]];
			      reload_insn
				= emit_insn_before (((*insn_gen_function[(int) (tertiary_icode)])
						     (second_reload_reg,
						      real_oldequiv,
						      third_reload_reg)),
						    where);
			      if (this_reload_insn == 0)
				this_reload_insn = reload_insn;
			    }
			  else
			    {
			      reload_insn
				= gen_input_reload (second_reload_reg,
						    oldequiv, where);
			      if (this_reload_insn == 0)
				this_reload_insn = reload_insn;
			      oldequiv = second_reload_reg;
			    }
			}
		    }
		}
	      if (! special)
		{
		  reload_insn = gen_input_reload (reloadreg, oldequiv, where);
		  if (this_reload_insn == 0)
		    this_reload_insn = reload_insn;
		}
	    }
	  if (this_reload_insn)
	    switch (reload_when_needed[j])
	      {
	      case RELOAD_FOR_INPUT:
	      case RELOAD_OTHER:
		if (first_other_reload_insn == first_operand_address_reload_insn)
		  first_other_reload_insn = this_reload_insn;
		break;
	      case RELOAD_FOR_OPERAND_ADDRESS:
		if (first_operand_address_reload_insn == before_insn)
		  first_operand_address_reload_insn = this_reload_insn;
		if (first_other_reload_insn == before_insn)
		  first_other_reload_insn = this_reload_insn;
	      }
	}
      if (optimize && reload_inherited[j] && reload_spill_index[j] >= 0
	  && (reload_when_needed[j] == RELOAD_OTHER
	      || reload_when_needed[j] == RELOAD_FOR_INPUT
	      || reload_when_needed[j] == RELOAD_FOR_INPUT_RELOAD_ADDRESS)
	  && ((reload_in[j])->code) == REG
	  && spill_reg_store[reload_spill_index[j]] != 0
	  && dead_or_set_p (insn, reload_in[j])
	  && count_occurrences (((insn)->fld[3].rtx), reload_in[j]) == 1)
	delete_output_reload (insn, j,
			      spill_reg_store[reload_spill_index[j]]);
      old = reload_out[j];
      if (old != 0
	  && reload_reg_rtx[j] != old
	  && reload_reg_rtx[j] != 0)
	{
	  register rtx reloadreg = reload_reg_rtx[j];
	  register rtx second_reloadreg = 0;
	  rtx prev_insn = ((first_output_reload_insn)->fld[1].rtx);
	  rtx note, p;
	  enum machine_mode mode;
	  int special = 0;
	  if ((((old)->code) == REG || ((old)->code) == SCRATCH)
	      && (note = find_reg_note (insn, REG_UNUSED, old)) != 0)
	    {
	      ((note)->fld[ 0].rtx) = reload_reg_rtx[j];
	      continue;
	    }
	  else if (((old)->code) == SCRATCH)
	    continue;
	  if (((insn)->code) == JUMP_INSN)
	    abort ();
	  mode = ((old)->mode);
	  if (mode == VOIDmode)
	    abort ();		
	  if (reload_strict_low[j])
	    {
	      mode = ((((reload_out[j])->fld[0].rtx))->mode);
	      while (((old)->code) == SUBREG && ((old)->mode) != mode)
		old = ((old)->fld[0].rtx);
	      if (((old)->mode) != VOIDmode
		  && mode != ((old)->mode))
		old = gen_rtx (SUBREG, mode, old, 0);
	    }
	  if (((reloadreg)->mode) != mode)
	    reloadreg = gen_rtx (REG, mode, ((reloadreg)->fld[0].rtint));
	  if (reload_secondary_reload[j] >= 0)
	    {
	      rtx real_old = old;
	      if (((old)->code) == REG && ((old)->fld[0].rtint) >= 64
		  && reg_equiv_mem[((old)->fld[0].rtint)] != 0)
		real_old = reg_equiv_mem[((old)->fld[0].rtint)];
	      if((((((( real_old)->code) == MEM 						   || ((( real_old)->code) == REG && (( real_old)->fld[0].rtint) >= 64)	   || ((( real_old)->code) == SUBREG						       && ((((( real_old)->fld[0].rtx))->code) == MEM					   || ((((( real_old)->fld[0].rtx))->code) == REG				       && (((( real_old)->fld[0].rtx))->fld[0].rtint) >= 64))))   && (((
						 mode) == HImode || (
						 mode) == QImode				       || ((
						 mode) == SImode && (reload_reg_class[j]) == FLOAT_REGS))))		 ? GENERAL_REGS : NO_REGS)
		  != NO_REGS))
		{
		  second_reloadreg = reloadreg;
		  reloadreg = reload_reg_rtx[reload_secondary_reload[j]];
		  if (reload_secondary_icode[j] != CODE_FOR_nothing)
		    {
		      emit_insn_before (((*insn_gen_function[(int) (reload_secondary_icode[j])])
					 (real_old, second_reloadreg,
					  reloadreg)),
					first_output_reload_insn);
		      special = 1;
		    }
		  else
		    {
		      int secondary_reload = reload_secondary_reload[j];
		      enum insn_code tertiary_icode
			= reload_secondary_icode[secondary_reload];
		      rtx pat;
		      if (((reloadreg)->mode) != mode)
			reloadreg = gen_rtx (REG, mode, ((reloadreg)->fld[0].rtint));
		      if (tertiary_icode != CODE_FOR_nothing)
			{
			  rtx third_reloadreg
			    = reload_reg_rtx[reload_secondary_reload[secondary_reload]];
			  pat = ((*insn_gen_function[(int) (tertiary_icode)])
				 (reloadreg, second_reloadreg, third_reloadreg));
			}
		      else if (((reloadreg)->code) == REG
			       && ((reloadreg)->fld[0].rtint) < 64
			       && ((((((reloadreg)->fld[0].rtint)) >= 32 ? FLOAT_REGS : GENERAL_REGS)) != (
					   ((((second_reloadreg)->fld[0].rtint)) >= 32 ? FLOAT_REGS : GENERAL_REGS))))
			{
			  rtx loc = get_secondary_mem (reloadreg,
						       ((second_reloadreg)->mode));
			  rtx tmp_reloadreg;
			  if (((loc)->mode) != ((second_reloadreg)->mode))
			    second_reloadreg = gen_rtx (REG, ((loc)->mode),
							((second_reloadreg)->fld[0].rtint));
			  if (((loc)->mode) != ((reloadreg)->mode))
			    tmp_reloadreg = gen_rtx (REG, ((loc)->mode),
						     ((reloadreg)->fld[0].rtint));
			  else
			    tmp_reloadreg = reloadreg;
			  emit_insn_before (gen_move_insn (loc, second_reloadreg),
					    first_output_reload_insn);
			  pat = gen_move_insn (tmp_reloadreg, loc);
			}
		      else
			pat = gen_move_insn (reloadreg, second_reloadreg);
		      emit_insn_before (pat, first_output_reload_insn);
		    }
		}
	    }
	  if (! special)
	    {
	      if (((old)->code) == REG && ((old)->fld[0].rtint) < 64
		  && ((((((old)->fld[0].rtint)) >= 32 ? FLOAT_REGS : GENERAL_REGS)) != (
					      ((((reloadreg)->fld[0].rtint)) >= 32 ? FLOAT_REGS : GENERAL_REGS))))
		{
		  rtx loc = get_secondary_mem (old, ((reloadreg)->mode));
		  if (((loc)->mode) != ((reloadreg)->mode))
		    reloadreg = gen_rtx (REG, ((loc)->mode),
					 ((reloadreg)->fld[0].rtint));
		  if (((loc)->mode) != ((old)->mode))
		    old = gen_rtx (REG, ((loc)->mode), ((old)->fld[0].rtint));
		  emit_insn_before (gen_move_insn (loc, reloadreg),
				    first_output_reload_insn);
		  emit_insn_before (gen_move_insn (old, loc),
				    first_output_reload_insn);
		}
	      else
		emit_insn_before (gen_move_insn (old, reloadreg),
				  first_output_reload_insn);
	    }
	  for (p = ((prev_insn)->fld[2].rtx); p != first_output_reload_insn;
	       p = ((p)->fld[2].rtx))
	    if ((rtx_class[(int)(((p)->code))]) == 'i')
	      {
		note_stores (((p)->fld[3].rtx), forget_old_reloads_1);
		if (reg_mentioned_p (reload_reg_rtx[j], ((p)->fld[3].rtx)))
		  store_insn = p;
	      }
	  first_output_reload_insn = ((prev_insn)->fld[2].rtx);
	}
      if (reload_spill_index[j] >= 0)
	new_spill_reg_store[reload_spill_index[j]] = store_insn;
    }
  for (j = 0; j < n_reloads; j++)
    {
      register int r = reload_order[j];
      register int i = reload_spill_index[r];
      if (i >= 0 && reload_reg_rtx[r] != 0)
	{
	  int nr
	    =   (((mode_size[(int)( ((reload_reg_rtx[r])->mode))]) + 8 - 1) / 8);
	  int k;
	  for (k = 0; k < nr; k++)
	    {
	      reg_reloaded_contents[spill_reg_order[spill_regs[i] + k]] = -1;
	      reg_reloaded_insn[spill_reg_order[spill_regs[i] + k]] = 0;
	    }
	  if (reload_out[r] != 0 && ((reload_out[r])->code) == REG)
	    {
	      register int nregno = ((reload_out[r])->fld[0].rtint);
	      spill_reg_store[i] = new_spill_reg_store[i];
	      reg_last_reload_reg[nregno] = reload_reg_rtx[r];
	      for (k = 0; k < nr; k++)
		{
		  reg_reloaded_contents[spill_reg_order[spill_regs[i] + k]]
		    = nregno;
		  reg_reloaded_insn[spill_reg_order[spill_regs[i] + k]] = insn;
		}
	    }
	  else if (reload_out[r] == 0
		   && reload_in[r] != 0
		   && (((reload_in[r])->code) == REG
		       || ((reload_in_reg[r])->code) == REG))
	    {
	      register int nregno;
	      if (((reload_in[r])->code) == REG)
		nregno = ((reload_in[r])->fld[0].rtint);
	      else
		nregno = ((reload_in_reg[r])->fld[0].rtint);
	      if (!reg_has_output_reload[nregno]
		  && reload_reg_reaches_end_p (spill_regs[i],
					       reload_when_needed[r]))
		{
		  reg_last_reload_reg[nregno] = reload_reg_rtx[r];
		  if (! reload_inherited[r])
		    spill_reg_store[i] = 0;
		  for (k = 0; k < nr; k++)
		    {
		      reg_reloaded_contents[spill_reg_order[spill_regs[i] + k]]
			= nregno;
		      reg_reloaded_insn[spill_reg_order[spill_regs[i] + k]]
			= insn;
		    }
		}
	    }
	}
      if (i < 0 && reload_out[r] != 0 && ((reload_out[r])->code) == REG)
	{
	  register int nregno = ((reload_out[r])->fld[0].rtint);
	  reg_last_reload_reg[nregno] = 0;
	}
    }
}
rtx
gen_input_reload (reloadreg, in, before_insn)
     rtx reloadreg;
     rtx in;
     rtx before_insn;
{
  register rtx prev_insn = ((before_insn)->fld[1].rtx);
  if (((in)->code) == PLUS
      && ((((in)->fld[ 0].rtx))->code) == REG
      && (((((in)->fld[ 1].rtx))->code) == REG
	  ||   (((((in)->fld[ 1].rtx))->code) == LABEL_REF || ((((in)->fld[ 1].rtx))->code) == SYMBOL_REF		   || ((((in)->fld[ 1].rtx))->code) == CONST_INT || ((((in)->fld[ 1].rtx))->code) == CONST_DOUBLE		   || ((((in)->fld[ 1].rtx))->code) == CONST || ((((in)->fld[ 1].rtx))->code) == HIGH)
	  || ((((in)->fld[ 1].rtx))->code) == MEM))
    {
      rtx op0, op1, tem, insn;
      int code;
      op0 = find_replacement (&((in)->fld[ 0].rtx));
      op1 = find_replacement (&((in)->fld[ 1].rtx));
      if (((((in)->fld[ 1].rtx))->code) == REG
	  && ((reloadreg)->fld[0].rtint) == ((((in)->fld[ 1].rtx))->fld[0].rtint))
	tem = op0, op0 = op1, op1 = tem;
      if (op0 != ((in)->fld[ 0].rtx) || op1 != ((in)->fld[ 1].rtx))
	in = gen_rtx (PLUS, ((in)->mode), op0, op1);
      insn = emit_insn_before (gen_rtx (SET, VOIDmode, reloadreg, in),
				   before_insn);
      code = recog_memoized (insn);
      if (code >= 0)
	{
	  insn_extract (insn);
	  if (constrain_operands (code, 1))
	    return insn;
	}
      if (((insn)->fld[1].rtx))
	((((insn)->fld[1].rtx))->fld[2].rtx) = ((insn)->fld[2].rtx);
      if (((insn)->fld[2].rtx))
	((((insn)->fld[2].rtx))->fld[1].rtx) = ((insn)->fld[1].rtx);
      if (  (((op1)->code) == LABEL_REF || ((op1)->code) == SYMBOL_REF		   || ((op1)->code) == CONST_INT || ((op1)->code) == CONST_DOUBLE		   || ((op1)->code) == CONST || ((op1)->code) == HIGH) || ((op1)->code) == MEM
	  || (((op1)->code) == REG
	      && ((op1)->fld[0].rtint) >= 64))
	tem = op0, op0 = op1, op1 = tem;
      emit_insn_before (gen_move_insn (reloadreg, op0), before_insn);
      if (rtx_equal_p (op0, op1))
	op1 = reloadreg;
      emit_insn_before (gen_add2_insn (reloadreg, op1), before_insn);
    }
  else if (((in)->code) == REG && ((in)->fld[0].rtint) < 64
	   && ((((((in)->fld[0].rtint)) >= 32 ? FLOAT_REGS : GENERAL_REGS)) != (
				       ((((reloadreg)->fld[0].rtint)) >= 32 ? FLOAT_REGS : GENERAL_REGS))))
    {
      rtx loc = get_secondary_mem (in, ((reloadreg)->mode));
      if (((loc)->mode) != ((reloadreg)->mode))
	reloadreg = gen_rtx (REG, ((loc)->mode), ((reloadreg)->fld[0].rtint));
      if (((loc)->mode) != ((in)->mode))
	in = gen_rtx (REG, ((loc)->mode), ((in)->fld[0].rtint));
      emit_insn_before (gen_move_insn (loc, in), before_insn);
      emit_insn_before (gen_move_insn (reloadreg, loc), before_insn);
    }
  else if ((rtx_class[(int)(((in)->code))]) == 'o' || ((in)->code) == SUBREG)
    emit_insn_before (gen_move_insn (reloadreg, in), before_insn);
  else
    emit_insn_before (gen_rtx (SET, VOIDmode, reloadreg, in), before_insn);
  return ((prev_insn)->fld[2].rtx);
}
static void
delete_output_reload (insn, j, output_reload_insn)
     rtx insn;
     int j;
     rtx output_reload_insn;
{
  register rtx i1;
  rtx reg = reload_in[j];
  while (((reg)->code) == SUBREG)
    reg = ((reg)->fld[0].rtx);
  for (i1 = ((output_reload_insn)->fld[2].rtx);
       i1 != insn; i1 = ((i1)->fld[2].rtx))
    {
      if (((i1)->code) == CODE_LABEL || ((i1)->code) == JUMP_INSN)
	return;
      if ((((i1)->code) == INSN || ((i1)->code) == CALL_INSN)
	  && reg_mentioned_p (reg, ((i1)->fld[3].rtx)))
	return;
    }
  if (reload_out[j] == reload_in[j])
    delete_insn (output_reload_insn);
  else if (reg_n_deaths[((reg)->fld[0].rtint)] == 1
	   && reg_basic_block[((reg)->fld[0].rtint)] >= 0
	   && find_regno_note (insn, REG_DEAD, ((reg)->fld[0].rtint)))
    {
      rtx i2;
      for (i2 = ((insn)->fld[1].rtx); i2; i2 = ((i2)->fld[1].rtx))
	{
	  rtx set = single_set (i2);
	  if (set != 0 && ((set)->fld[0].rtx) == reg)
	    continue;
	  if (((i2)->code) == CODE_LABEL
	      || ((i2)->code) == JUMP_INSN)
	    break;
	  if ((((i2)->code) == INSN || ((i2)->code) == CALL_INSN)
	      && reg_mentioned_p (reg, ((i2)->fld[3].rtx)))
	    return;
	}
      for (i2 = ((insn)->fld[1].rtx); i2; i2 = ((i2)->fld[1].rtx))
	{
	  rtx set = single_set (i2);
	  if (set != 0 && ((set)->fld[0].rtx) == reg)
	    delete_insn (i2);
	  if (((i2)->code) == CODE_LABEL
	      || ((i2)->code) == JUMP_INSN)
	    break;
	}
      reg_renumber[((reg)->fld[0].rtint)] = ((reload_reg_rtx[j])->fld[0].rtint);
      alter_reg (((reg)->fld[0].rtint), -1);
    }
}
static rtx
inc_for_reload (reloadreg, value, inc_amount, insn)
     rtx reloadreg;
     rtx value;
     int inc_amount;
     rtx insn;
{
  rtx incloc = ((value)->fld[ 0].rtx);
  int post = (((value)->code) == POST_DEC || ((value)->code) == POST_INC);
  rtx prev = ((insn)->fld[1].rtx);
  rtx inc;
  rtx add_insn;
  int code;
  if (((incloc)->code) == REG)
    reg_last_reload_reg[((incloc)->fld[0].rtint)] = 0;
  if (((value)->code) == PRE_DEC || ((value)->code) == POST_DEC)
    inc_amount = - inc_amount;
  inc = gen_rtx (CONST_INT, VOIDmode, (inc_amount));
  if (post)
    emit_insn_before (gen_move_insn (reloadreg, incloc), insn);
  add_insn = emit_insn_before (gen_rtx (SET, VOIDmode, incloc,
					gen_rtx (PLUS, ((incloc)->mode),
						 incloc, inc)), insn);
  code = recog_memoized (add_insn);
  if (code >= 0)
    {
      insn_extract (add_insn);
      if (constrain_operands (code, 1))
	{
	  if (! post)
	    emit_insn_before (gen_move_insn (reloadreg, incloc), insn);
	  return ((prev)->fld[2].rtx);
	}
    }
  if (((add_insn)->fld[1].rtx))
    ((((add_insn)->fld[1].rtx))->fld[2].rtx) = ((add_insn)->fld[2].rtx);
  if (((add_insn)->fld[2].rtx))
    ((((add_insn)->fld[2].rtx))->fld[1].rtx) = ((add_insn)->fld[1].rtx);
  if (! post)
    {
      emit_insn_before (gen_move_insn (reloadreg, incloc), insn);
      emit_insn_before (gen_add2_insn (reloadreg, inc), insn);
      emit_insn_before (gen_move_insn (incloc, reloadreg), insn);
    }
  else
    {
      emit_insn_before (gen_add2_insn (reloadreg, inc), insn);
      emit_insn_before (gen_move_insn (incloc, reloadreg), insn);
      emit_insn_before (gen_add2_insn (reloadreg, gen_rtx (CONST_INT, VOIDmode, (-inc_amount))),
			insn);
    }
  return ((prev)->fld[2].rtx);
}
static int
constraint_accepts_reg_p (string, reg)
     char *string;
     rtx reg;
{
  int value = 0;
  int regno = true_regnum (reg);
  int c;
  value = 0;
  while (1)
    switch (c = *string++)
      {
      case 0:
	return value;
      case ',':
	if (value == 0)
	  return 0;
	value = 0;
	break;
      case 'g':
      case 'r':
	if (  ((reg_class_contents[(int) GENERAL_REGS])[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	  value = 1;
	break;
      default:
	{
	  enum reg_class class =  ((c) == 'f' ? FLOAT_REGS : NO_REGS);
	  if (  ((reg_class_contents[(int) class])[( regno) / ((unsigned) 32)]	   & ((int) 1 << (( regno) % ((unsigned) 32)))))
	    value = 1;
	}
      }
}
static int
count_occurrences (x, find)
     register rtx x, find;
{
  register int i, j;
  register enum rtx_code code;
  register char *format_ptr;
  int count;
  if (x == find)
    return 1;
  if (x == 0)
    return 0;
  code = ((x)->code);
  switch (code)
    {
    case REG:
    case QUEUED:
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case CODE_LABEL:
    case PC:
    case CC0:
      return 0;
    case SET:
      if (((x)->fld[0].rtx) == find)
	return count_occurrences (((x)->fld[1].rtx), find);
      break;
    }
  format_ptr = (rtx_format[(int)(code)]);
  count = 0;
  for (i = 0; i < (rtx_length[(int)(code)]); i++)
    {
      switch (*format_ptr++)
	{
	case 'e':
	  count += count_occurrences (((x)->fld[ i].rtx), find);
	  break;
	case 'E':
	  if (((x)->fld[ i].rtvec) != 0L)
	    {
	      for (j = 0; j < ((x)->fld[ i].rtvec->num_elem); j++)
		count += count_occurrences (((x)->fld[ i].rtvec->elem[ j].rtx), find);
	    }
	  break;
	}
    }
  return count;
}
