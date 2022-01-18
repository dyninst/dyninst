static bool IS_ENC_SOP1(uint64_t I);
static bool IS_ENC_SOPC(uint64_t I);
static bool IS_ENC_SOPP(uint64_t I);
static bool IS_ENC_SOPK(uint64_t I);
static bool IS_ENC_SOP2(uint64_t I);
static bool IS_ENC_SMEM(uint64_t I);
static bool IS_ENC_VOP1(uint64_t I);
static bool IS_ENC_VOPC(uint64_t I);
static bool IS_ENC_VOP2(uint64_t I);
static bool IS_ENC_VINTRP(uint64_t I);
static bool IS_ENC_VOP3P(uint64_t I);
static bool IS_ENC_VOP3(uint64_t I);
static bool IS_ENC_DS(uint64_t I);
static bool IS_ENC_MUBUF(uint64_t I);
static bool IS_ENC_MTBUF(uint64_t I);
static bool IS_ENC_MIMG(uint64_t I);
static bool IS_ENC_FLAT(uint64_t I);
static bool IS_ENC_FLAT_GLBL(uint64_t I);
static bool IS_ENC_FLAT_SCRATCH(uint64_t I);
static bool IS_ENC_VOP2_LITERAL(uint64_t I);
static bool IS_ENC_VOP3B(uint64_t I);
static bool IS_ENC_VOP3P_MFMA(uint64_t I);
enum InstructionFamily{
	ENC_SOP1 = -1,
	ENC_SOPC = 0,
	ENC_SOPP = 1,
	ENC_SOPK = 2,
	ENC_SOP2 = 3,
	ENC_SMEM = 4,
	ENC_VOP1 = 5,
	ENC_VOPC = 6,
	ENC_VOP2 = 7,
	ENC_VINTRP = 8,
	ENC_VOP3P = 9,
	ENC_VOP3 = 10,
	ENC_DS = 11,
	ENC_MUBUF = 12,
	ENC_MTBUF = 13,
	ENC_MIMG = 14,
	ENC_FLAT = 16,
	ENC_FLAT_GLBL = 17,
	ENC_FLAT_SCRATCH = 18,
	ENC_VOP2_LITERAL = 19,
	ENC_VOP3B = 20,
	ENC_VOP3P_MFMA = 21,
};
InstructionFamily instr_family;
typedef void (InstructionDecoder_amdgpu_cdna2::*func_ptr)(void);
func_ptr decode_lookup_table [22] = {
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_SOP1),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_SOPC),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_SOPP),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_SOPK),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_SOP2),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_SMEM),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP1),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOPC),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP2),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VINTRP),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3P),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_DS),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_MUBUF),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_MTBUF),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_MIMG),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_FLAT),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_FLAT_GLBL),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_FLAT_SCRATCH),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP2_LITERAL),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3B),
	(&InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3P_MFMA),
};
typedef struct layout_ENC_SOP1{
	uint16_t  ENCODING : 9 ;
	uint8_t  OP : 8 ;
	uint8_t  SDST : 7 ;
	uint8_t  SSRC0 : 8 ;
}layout_ENC_SOP1;
typedef struct layout_ENC_SOPC{
	uint16_t  ENCODING : 9 ;
	uint8_t  OP : 7 ;
	uint8_t  SSRC0 : 8 ;
	uint8_t  SSRC1 : 8 ;
}layout_ENC_SOPC;
typedef struct layout_ENC_SOPP{
	uint16_t  ENCODING : 9 ;
	uint8_t  OP : 7 ;
	uint16_t  SIMM16 : 16 ;
}layout_ENC_SOPP;
typedef struct layout_ENC_SOPK{
	uint8_t  ENCODING : 4 ;
	uint8_t  OP : 5 ;
	uint8_t  SDST : 7 ;
	uint16_t  SIMM16 : 16 ;
}layout_ENC_SOPK;
typedef struct layout_ENC_SOP2{
	uint8_t  ENCODING : 2 ;
	uint8_t  OP : 7 ;
	uint8_t  SDST : 7 ;
	uint8_t  SSRC0 : 8 ;
	uint8_t  SSRC1 : 8 ;
}layout_ENC_SOP2;
typedef struct layout_ENC_SMEM{
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  IMM : 1 ;
	uint8_t  NV : 1 ;
	uint32_t  OFFSET : 21 ;
	uint8_t  OP : 8 ;
	uint8_t  SBASE : 6 ;
	uint8_t  SDATA : 7 ;
	uint8_t  SOFFSET : 7 ;
	uint8_t  SOFFSET_EN : 1 ;
}layout_ENC_SMEM;
typedef struct layout_ENC_VOP1{
	uint8_t  ENCODING : 7 ;
	uint8_t  OP : 8 ;
	uint16_t  SRC0 : 9 ;
	uint8_t  VDST : 8 ;
}layout_ENC_VOP1;
typedef struct layout_ENC_VOPC{
	uint8_t  ENCODING : 7 ;
	uint8_t  OP : 8 ;
	uint16_t  SRC0 : 9 ;
	uint8_t  VSRC1 : 8 ;
}layout_ENC_VOPC;
typedef struct layout_ENC_VOP2{
	uint8_t  ENCODING : 1 ;
	uint8_t  OP : 6 ;
	uint16_t  SRC0 : 9 ;
	uint8_t  VDST : 8 ;
	uint8_t  VSRC1 : 8 ;
}layout_ENC_VOP2;
typedef struct layout_ENC_VINTRP{
	uint8_t  ATTR : 6 ;
	uint8_t  ATTRCHAN : 2 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  OP : 2 ;
	uint8_t  VDST : 8 ;
	uint8_t  VSRC : 8 ;
}layout_ENC_VINTRP;
typedef struct layout_ENC_VOP3P{
	uint8_t  CLAMP : 1 ;
	uint16_t  ENCODING : 9 ;
	uint8_t  NEG : 3 ;
	uint8_t  NEG_HI : 3 ;
	uint8_t  OP : 7 ;
	uint8_t  OP_SEL : 3 ;
	uint8_t  OP_SEL_HI : 2 ;
	uint8_t  OP_SEL_HI_2 : 1 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
	uint8_t  VDST : 8 ;
}layout_ENC_VOP3P;
typedef struct layout_ENC_VOP3{
	uint8_t  ABS : 3 ;
	uint8_t  CLAMP : 1 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  NEG : 3 ;
	uint8_t  OMOD : 2 ;
	uint16_t  OP : 10 ;
	uint8_t  OP_SEL : 4 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
	uint8_t  VDST : 8 ;
}layout_ENC_VOP3;
typedef struct layout_ENC_DS{
	uint8_t  ACC : 1 ;
	uint8_t  ADDR : 8 ;
	uint8_t  DATA0 : 8 ;
	uint8_t  DATA1 : 8 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GDS : 1 ;
	uint8_t  OFFSET0 : 8 ;
	uint8_t  OFFSET1 : 8 ;
	uint8_t  OP : 8 ;
	uint8_t  VDST : 8 ;
}layout_ENC_DS;
typedef struct layout_ENC_MUBUF{
	uint8_t  ACC : 1 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  IDXEN : 1 ;
	uint8_t  LDS : 1 ;
	uint8_t  OFFEN : 1 ;
	uint16_t  OFFSET : 12 ;
	uint8_t  OP : 7 ;
	uint8_t  SCC : 1 ;
	uint8_t  SLC : 1 ;
	uint8_t  SOFFSET : 8 ;
	uint8_t  SRSRC : 5 ;
	uint8_t  VADDR : 8 ;
	uint8_t  VDATA : 8 ;
}layout_ENC_MUBUF;
typedef struct layout_ENC_MTBUF{
	uint8_t  ACC : 1 ;
	uint8_t  DFMT : 4 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  IDXEN : 1 ;
	uint8_t  NFMT : 3 ;
	uint8_t  OFFEN : 1 ;
	uint16_t  OFFSET : 12 ;
	uint8_t  OP : 4 ;
	uint8_t  SCC : 1 ;
	uint8_t  SLC : 1 ;
	uint8_t  SOFFSET : 8 ;
	uint8_t  SRSRC : 5 ;
	uint8_t  VADDR : 8 ;
	uint8_t  VDATA : 8 ;
}layout_ENC_MTBUF;
typedef struct layout_ENC_MIMG{
	uint8_t  A16 : 1 ;
	uint8_t  ACC : 1 ;
	uint8_t  D16 : 1 ;
	uint8_t  DA : 1 ;
	uint8_t  DMASK : 4 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  LWE : 1 ;
	uint8_t  OP : 7 ;
	uint8_t  OPM : 1 ;
	uint8_t  SCC : 1 ;
	uint8_t  SLC : 1 ;
	uint8_t  SRSRC : 5 ;
	uint8_t  SSAMP : 5 ;
	uint8_t  UNORM : 1 ;
	uint8_t  VADDR : 8 ;
	uint8_t  VDATA : 8 ;
}layout_ENC_MIMG;
typedef struct layout_ENC_FLAT{
	uint8_t  ACC : 1 ;
	uint8_t  ADDR : 8 ;
	uint8_t  DATA : 8 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  LDS : 1 ;
	uint16_t  OFFSET : 12 ;
	uint8_t  OP : 7 ;
	uint8_t  SADDR : 7 ;
	uint8_t  SCC : 1 ;
	uint8_t  SEG : 2 ;
	uint8_t  SLC : 1 ;
	uint8_t  VDST : 8 ;
}layout_ENC_FLAT;
typedef struct layout_ENC_FLAT_GLBL{
	uint8_t  ACC : 1 ;
	uint8_t  ADDR : 8 ;
	uint8_t  DATA : 8 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  LDS : 1 ;
	uint16_t  OFFSET : 13 ;
	uint8_t  OP : 7 ;
	uint8_t  SADDR : 7 ;
	uint8_t  SCC : 1 ;
	uint8_t  SEG : 2 ;
	uint8_t  SLC : 1 ;
	uint8_t  VDST : 8 ;
}layout_ENC_FLAT_GLBL;
typedef struct layout_ENC_FLAT_SCRATCH{
	uint8_t  ACC : 1 ;
	uint8_t  ADDR : 8 ;
	uint8_t  DATA : 8 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GLC : 1 ;
	uint8_t  LDS : 1 ;
	uint16_t  OFFSET : 13 ;
	uint8_t  OP : 7 ;
	uint8_t  SADDR : 7 ;
	uint8_t  SCC : 1 ;
	uint8_t  SEG : 2 ;
	uint8_t  SLC : 1 ;
	uint8_t  VDST : 8 ;
}layout_ENC_FLAT_SCRATCH;
typedef struct layout_ENC_VOP2_LITERAL{
	uint8_t  ENCODING : 1 ;
	uint8_t  OP : 6 ;
	uint32_t  SIMM32 : 32 ;
	uint16_t  SRC0 : 9 ;
	uint8_t  VDST : 8 ;
	uint8_t  VSRC1 : 8 ;
}layout_ENC_VOP2_LITERAL;
typedef struct layout_ENC_VOP3B{
	uint8_t  CLAMP : 1 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  NEG : 3 ;
	uint8_t  OMOD : 2 ;
	uint16_t  OP : 10 ;
	uint8_t  SDST : 7 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
	uint8_t  VDST : 8 ;
}layout_ENC_VOP3B;
typedef struct layout_ENC_VOP3P_MFMA{
	uint8_t  ABID : 4 ;
	uint8_t  ACC : 2 ;
	uint8_t  ACC_CD : 1 ;
	uint8_t  BLGP : 3 ;
	uint8_t  CBSZ : 3 ;
	uint16_t  ENCODING : 9 ;
	uint8_t  OP : 7 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
	uint8_t  VDST : 8 ;
}layout_ENC_VOP3P_MFMA;
union insn_layout{
	layout_ENC_SOP1 ENC_SOP1;
	layout_ENC_SOPC ENC_SOPC;
	layout_ENC_SOPP ENC_SOPP;
	layout_ENC_SOPK ENC_SOPK;
	layout_ENC_SOP2 ENC_SOP2;
	layout_ENC_SMEM ENC_SMEM;
	layout_ENC_VOP1 ENC_VOP1;
	layout_ENC_VOPC ENC_VOPC;
	layout_ENC_VOP2 ENC_VOP2;
	layout_ENC_VINTRP ENC_VINTRP;
	layout_ENC_VOP3P ENC_VOP3P;
	layout_ENC_VOP3 ENC_VOP3;
	layout_ENC_DS ENC_DS;
	layout_ENC_MUBUF ENC_MUBUF;
	layout_ENC_MTBUF ENC_MTBUF;
	layout_ENC_MIMG ENC_MIMG;
	layout_ENC_FLAT ENC_FLAT;
	layout_ENC_FLAT_GLBL ENC_FLAT_GLBL;
	layout_ENC_FLAT_SCRATCH ENC_FLAT_SCRATCH;
	layout_ENC_VOP2_LITERAL ENC_VOP2_LITERAL;
	layout_ENC_VOP3B ENC_VOP3B;
	layout_ENC_VOP3P_MFMA ENC_VOP3P_MFMA;
}insn_layout;
void decodeENC_SOP1();
void finalizeENC_SOP1Operands();
void decodeENC_SOPC();
void finalizeENC_SOPCOperands();
void decodeENC_SOPP();
void finalizeENC_SOPPOperands();
void decodeENC_SOPK();
void finalizeENC_SOPKOperands();
void decodeENC_SOP2();
void finalizeENC_SOP2Operands();
void decodeENC_SMEM();
void finalizeENC_SMEMOperands();
void decodeENC_VOP1();
void finalizeENC_VOP1Operands();
void decodeENC_VOPC();
void finalizeENC_VOPCOperands();
void decodeENC_VOP2();
void finalizeENC_VOP2Operands();
void decodeENC_VINTRP();
void finalizeENC_VINTRPOperands();
void decodeENC_VOP3P();
void finalizeENC_VOP3POperands();
void decodeENC_VOP3();
void finalizeENC_VOP3Operands();
void decodeENC_DS();
void finalizeENC_DSOperands();
void decodeENC_MUBUF();
void finalizeENC_MUBUFOperands();
void decodeENC_MTBUF();
void finalizeENC_MTBUFOperands();
void decodeENC_MIMG();
void finalizeENC_MIMGOperands();
void decodeENC_FLAT();
void finalizeENC_FLATOperands();
void decodeENC_FLAT_GLBL();
void finalizeENC_FLAT_GLBLOperands();
void decodeENC_FLAT_SCRATCH();
void finalizeENC_FLAT_SCRATCHOperands();
void decodeENC_VOP2_LITERAL();
void finalizeENC_VOP2_LITERALOperands();
void decodeENC_VOP3B();
void finalizeENC_VOP3BOperands();
void decodeENC_VOP3P_MFMA();
void finalizeENC_VOP3P_MFMAOperands();
