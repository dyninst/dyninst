
/*
 * Define sparc instruction information.
 *
 */
struct fmt1 {
    unsigned op:2;
    signed disp30:30;
};

struct fmt2 {
    unsigned op:2;
    unsigned anneal:1;
    unsigned cond:4;
    unsigned op2:3;
    signed disp22:22;
};

struct fmt2a {
    unsigned op:2;
    unsigned rd:5;
    unsigned op2:3;
    signed imm22:22;
};

struct fmt3 {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    unsigned unused:8;
    unsigned rs2:5;
};

struct fmt3i {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    signed simm13:13;
};

union instructUnion {
    struct fmt1	call;
    struct fmt2	branch;
    struct fmt2a sethi;
    struct fmt3 rest;
    struct fmt3i resti;
    unsigned int raw;
};

typedef union instructUnion instruction;


/*
 * Define the operation codes
 *
 */
#define SetCC		16
#define ADDop3		0
#define ANDop3		1
#define ORop3		2
#define SUBop3		4
#define SUBop3cc	SetCC|SUBop3
#define SMULop3		11
#define SDIVop3		15
#define XNORop3		SetCC|7
#define SAVEop3		60
#define RESTOREop3	61
#define JMPLop3		56

#define FP_OP3_LOW	0x20
#define LDFop3		0x20
#define LDFSRop3	0x21
#define LDDFop3		0x23
#define STFop3		0x24
#define STFSRop3	0x25
#define STDFQop3	0x26
#define STDFop3		0x27
#define FP_OP3_HIGH	0x27


/* op = 11 */
#define STop	3
#define LDop3	0
#define STop3	4


/* mask bits for various parts of the instruction format */
#define OPmask		0xc0000000
#define OP2mask		0x00e00000
#define OP3mask		0x01f80000
#define RDmask		0x3e000000

#define DISP30mask	0x3fffffff

/* op = 01 -- mask for and match for call instruction */
#define	CALLop		1
#define CALLmask	OPmask
#define CALLmatch	0x40000000

/* (op==10) && (op3 == 111000) 
 */
#define RESTop		2
#define JMPLmask	(OPmask|OP3mask)
#define JMPLmatch	0x81c00000

#define FMT2op		0
#define LOADop		3

/*
 * added this on 8/18 (jkh) to tell a jmpl from a call indirect.
 *
 */
#define CALLImask	(OPmask|RDmask|OP3mask)
#define CALLImatch	0x9fc00000

/* (op=10) && (op3==111001) trap instructions */
#define TRAPmask	(OPmask|OP3mask)
#define TRAPmatch	0x81c10000

/* (op == 00) && (op2 ^ 2) mask for conditional branching instructions */
#define BICCop2		2

#define BEcond		1
#define BLEcond		2
#define BAcond		8
#define BNEcond		9

#define BRNCHmask	(OPmask|OP2mask)
#define BRNCHmatch	0x1<<23

/* really jmpl %i7+8,%g0 */
/* changed this to jmpl %i7+??,%g0 since g++ sometimes returns +0xc not +0x8 
 * jkh - 7/12/93
 */
#define RETmask         0xfffff000
#define RETmatch	0x81c7e000

/* retl - leaf return really jmpl %o7+8,%g0 */
/* changed this to jmpl %i7+??,%g0 since g++ sometimes returns +0xc not +0x8
 * jkh - 7/12/93
 */
#define RETLmask        0xfffff000
#define RETLmatch	0x81c3e000

/* noop insn */
#define NOOPop2		4
#define SETHIop2	4

/* If these bits are non-zero an op2 instruction is a non-annuled branch */
#define ANNUL_BIT	0x40000000

#define LOW(x)	((x)%1024)
#define HIGH(x)	((x)/1024)

#define isInsn(insn, mask, match)	(((insn).raw & mask) == match)

#define IS_DELAYED_INST(insn)	\
	(insn.call.op == CALLop || \
	 isInsn(insn, JMPLmask, JMPLmatch) || \
	 isInsn(insn, BRNCHmask, BRNCHmatch) || \
	 isInsn(insn, TRAPmask, TRAPmatch))

/* catch small ints that are invalid instructions */
/*
 * insn.call.op checks for CALL or Format 3 insns
 * op2 == {2,4,6,7} checks for valid format 2 instructions.
 *    See SPARC Arch manual v8 p. 44.
 *
 */
#define IS_VALID_INSN(insn)     \
        ((insn.call.op) || ((insn.branch.op2 == 2) ||   \
                           (insn.branch.op2 == 4) ||    \
                           (insn.branch.op2 == 6) ||    \
                           (insn.branch.op2 == 7)))

