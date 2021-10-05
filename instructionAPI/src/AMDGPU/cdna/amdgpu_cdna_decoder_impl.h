static bool IS_ENC_SOP1(uint64_t I){ return  ((I & 0xFF800000) == 0xBE800000);}
static bool IS_ENC_SOPC(uint64_t I){ return  ((I & 0xFF800000) == 0xBF000000);}
static bool IS_ENC_SOPP(uint64_t I){ return  ((I & 0xFF800000) == 0xBF800000);}
static bool IS_ENC_SOPK(uint64_t I){ return  ((I & 0xF0000000) == 0xB0000000);}
static bool IS_ENC_SOP2(uint64_t I){ return  ((I & 0xC0000000) == 0x80000000);}
static bool IS_ENC_SMEM(uint64_t I){ return  ((I & 0xFC000000) == 0xC0000000);}
static bool IS_ENC_VOP1(uint64_t I){ return  ((I & 0xFE000000) == 0x7E000000);}
static bool IS_ENC_VOPC(uint64_t I){ return  ((I & 0xFE000000) == 0x7C000000);}
static bool IS_ENC_VOP2(uint64_t I){ return  ((I & 0x80000000) == 0x0);}
static bool IS_ENC_VINTRP(uint64_t I){ return  ((I & 0xFC000000) == 0xD4000000);}
static bool IS_ENC_VOP3P(uint64_t I){ return  ((I & 0xFF800000) == 0xD3800000);}
static bool IS_ENC_VOP3(uint64_t I){ return  ((I & 0xFC000000) == 0xD0000000);}
static bool IS_ENC_DS(uint64_t I){ return  ((I & 0xFC000000) == 0xD8000000);}
static bool IS_ENC_MUBUF(uint64_t I){ return  ((I & 0xFC000000) == 0xE0000000);}
static bool IS_ENC_MTBUF(uint64_t I){ return  ((I & 0xFC000000) == 0xE8000000);}
static bool IS_ENC_MIMG(uint64_t I){ return  ((I & 0xFC000000) == 0xF0000000);}
static bool IS_ENC_EXP(uint64_t I){ return  ((I & 0xFC000000) == 0xC4000000);}
static bool IS_ENC_FLAT(uint64_t I){ return  ((I & 0xFC000000) == 0xDC000000);}
static bool IS_VOP3_0_SDST_ENC_VOP3_1_(uint64_t I){ return  ((I & 0x0) == 0x0);}
enum InstructionFamily{
	ENC_SOP1 = 0,
	ENC_SOPC = 1,
	ENC_SOPP = 2,
	ENC_SOPK = 3,
	ENC_SOP2 = 4,
	ENC_SMEM = 5,
	ENC_VOP1 = 6,
	ENC_VOPC = 7,
	ENC_VOP2 = 8,
	ENC_VINTRP = 9,
	ENC_VOP3P = 10,
	ENC_VOP3 = 11,
	ENC_DS = 12,
	ENC_MUBUF = 13,
	ENC_MTBUF = 14,
	ENC_MIMG = 15,
	ENC_EXP = 16,
	ENC_FLAT = 17,
	VOP3_0_SDST_ENC_VOP3_1_ = 18,
};
InstructionFamily instr_family;
typedef void (InstructionDecoder_amdgpu_cdna::*func_ptr)(void);
func_ptr decode_lookup_table [19] = {
	(&InstructionDecoder_amdgpu_cdna::decodeENC_SOP1),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_SOPC),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_SOPP),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_SOPK),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_SOP2),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_SMEM),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_VOP1),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_VOPC),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_VOP2),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_VINTRP),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_VOP3P),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_VOP3),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_DS),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_MUBUF),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_MTBUF),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_MIMG),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_EXP),
	(&InstructionDecoder_amdgpu_cdna::decodeENC_FLAT),
	(&InstructionDecoder_amdgpu_cdna::decodeVOP3_0_SDST_ENC_VOP3_1_),
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
	uint8_t  OP : 8 ;
	uint8_t  SBASE : 6 ;
	uint8_t  SDATA : 7 ;
	uint8_t  SOFFSET_EN : 1 ;
	uint32_t  OFFSET : 21 ;
	uint8_t  SOFFSET : 7 ;
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
	uint8_t  NEG_HI : 3 ;
	uint8_t  OP : 7 ;
	uint8_t  OP_SEL : 3 ;
	uint8_t  OP_SEL_HI_2 : 1 ;
	uint8_t  VDST : 8 ;
	uint8_t  NEG : 3 ;
	uint8_t  OP_SEL_HI : 2 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
}layout_ENC_VOP3P;
typedef struct layout_ENC_VOP3{
	uint8_t  ABS : 3 ;
	uint8_t  CLAMP : 1 ;
	uint8_t  ENCODING : 6 ;
	uint16_t  OP : 10 ;
	uint8_t  OP_SEL : 4 ;
	uint8_t  VDST : 8 ;
	uint8_t  NEG : 3 ;
	uint8_t  OMOD : 2 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
}layout_ENC_VOP3;
typedef struct layout_ENC_DS{
	uint8_t  ACC : 1 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  GDS : 1 ;
	uint8_t  OFFSET0 : 8 ;
	uint8_t  OFFSET1 : 8 ;
	uint8_t  OP : 8 ;
	uint8_t  ADDR : 8 ;
	uint8_t  DATA0 : 8 ;
	uint8_t  DATA1 : 8 ;
	uint8_t  VDST : 8 ;
}layout_ENC_DS;
typedef struct layout_ENC_MUBUF{
	uint8_t  ENCODING : 6 ;
	uint8_t  IDXEN : 1 ;
	uint8_t  LDS : 1 ;
	uint8_t  NT : 1 ;
	uint8_t  OFFEN : 1 ;
	uint16_t  OFFSET : 12 ;
	uint8_t  OP : 7 ;
	uint8_t  SC0 : 1 ;
	uint8_t  SC1 : 1 ;
	uint8_t  ACC : 1 ;
	uint8_t  SOFFSET : 8 ;
	uint8_t  SRSRC : 5 ;
	uint8_t  VADDR : 8 ;
	uint8_t  VDATA : 8 ;
}layout_ENC_MUBUF;
typedef struct layout_ENC_MTBUF{
	uint8_t  DFMT : 4 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  IDXEN : 1 ;
	uint8_t  NFMT : 3 ;
	uint8_t  OFFEN : 1 ;
	uint16_t  OFFSET : 12 ;
	uint8_t  OP : 4 ;
	uint8_t  SC0 : 1 ;
	uint8_t  ACC : 1 ;
	uint8_t  NT : 1 ;
	uint8_t  SC1 : 1 ;
	uint8_t  SOFFSET : 8 ;
	uint8_t  SRSRC : 5 ;
	uint8_t  VADDR : 8 ;
	uint8_t  VDATA : 8 ;
}layout_ENC_MTBUF;
typedef struct layout_ENC_MIMG{
	uint8_t  A16 : 1 ;
	uint8_t  ACC : 1 ;
	uint8_t  DA : 1 ;
	uint8_t  DMASK : 4 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  LWE : 1 ;
	uint8_t  NT : 1 ;
	uint8_t  OP : 7 ;
	uint8_t  OPM : 1 ;
	uint8_t  SC0 : 1 ;
	uint8_t  SC1 : 1 ;
	uint8_t  UNORM : 1 ;
	uint8_t  D16 : 1 ;
	uint8_t  SRSRC : 5 ;
	uint8_t  SSAMP : 5 ;
	uint8_t  VADDR : 8 ;
	uint8_t  VDATA : 8 ;
}layout_ENC_MIMG;
typedef struct layout_ENC_EXP{
	uint8_t  COMPR : 1 ;
	uint8_t  DONE : 1 ;
	uint8_t  EN : 4 ;
	uint8_t  ENCODING : 6 ;
	uint8_t  TGT : 6 ;
	uint8_t  VM : 1 ;
	uint8_t  VSRC0 : 8 ;
	uint8_t  VSRC1 : 8 ;
	uint8_t  VSRC2 : 8 ;
	uint8_t  VSRC3 : 8 ;
}layout_ENC_EXP;
typedef struct layout_ENC_FLAT{
	uint8_t  ENCODING : 6 ;
	uint8_t  NT : 1 ;
	uint16_t  OFFSET : 12 ;
	uint8_t  OP : 7 ;
	uint8_t  SC0 : 1 ;
	uint8_t  SC1 : 1 ;
	uint8_t  SEG : 2 ;
	uint8_t  SVE : 1 ;
	uint8_t  ACC : 1 ;
	uint8_t  ADDR : 8 ;
	uint8_t  DATA : 8 ;
	uint8_t  SADDR : 7 ;
	uint8_t  VDST : 8 ;
}layout_ENC_FLAT;
typedef struct layout_VOP3_0_SDST_ENC_VOP3_1_{
	uint8_t  CLAMP : 1 ;
	uint8_t  ENCODING : 6 ;
	uint16_t  OP : 10 ;
	uint8_t  SDST : 7 ;
	uint8_t  VDST : 8 ;
	uint8_t  NEG : 3 ;
	uint8_t  OMOD : 2 ;
	uint16_t  SRC0 : 9 ;
	uint16_t  SRC1 : 9 ;
	uint16_t  SRC2 : 9 ;
}layout_VOP3_0_SDST_ENC_VOP3_1_;
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
	layout_ENC_EXP ENC_EXP;
	layout_ENC_FLAT ENC_FLAT;
	layout_VOP3_0_SDST_ENC_VOP3_1_ VOP3_0_SDST_ENC_VOP3_1_;
}insn_layout;
void decodeENC_SOP1();
void decodeENC_SOPC();
void decodeENC_SOPP();
void decodeENC_SOPK();
void decodeENC_SOP2();
void decodeENC_SMEM();
void decodeENC_VOP1();
void decodeENC_VOPC();
void decodeENC_VOP2();
void decodeENC_VINTRP();
void decodeENC_VOP3P();
void decodeENC_VOP3();
void decodeENC_DS();
void decodeENC_MUBUF();
void decodeENC_MTBUF();
void decodeENC_MIMG();
void decodeENC_EXP();
void decodeENC_FLAT();
void decodeVOP3_0_SDST_ENC_VOP3_1_();
