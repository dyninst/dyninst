enum InstructionFamily{
	SOP2 = 0,
	SOP1 = 1,
	SOPK = 2,
	SOPC = 3,
	SOPP = 4,
	SMEM = 5,
	VOP2 = 6,
	VOP1 = 7,
	VOPC = 8,
	VINTRP = 9,
	DS = 10,
	MTBUF = 11,
	MUBUF = 12,
	VOP3AB = 13,
	VOP3P = 14,
	FLAT = 15
};
typedef void (InstructionDecoder_amdgpu_vega::*func_ptr)(void);
func_ptr decode_lookup_table [16] = {
	(&InstructionDecoder_amdgpu_vega::decodeSOP2),
	(&InstructionDecoder_amdgpu_vega::decodeSOP1),
	(&InstructionDecoder_amdgpu_vega::decodeSOPK),
	(&InstructionDecoder_amdgpu_vega::decodeSOPC),
	(&InstructionDecoder_amdgpu_vega::decodeSOPP),
	(&InstructionDecoder_amdgpu_vega::decodeSMEM),
	(&InstructionDecoder_amdgpu_vega::decodeVOP2),
	(&InstructionDecoder_amdgpu_vega::decodeVOP1),
	(&InstructionDecoder_amdgpu_vega::decodeVOPC),
	(&InstructionDecoder_amdgpu_vega::decodeVINTRP),
	(&InstructionDecoder_amdgpu_vega::decodeDS),
	(&InstructionDecoder_amdgpu_vega::decodeMTBUF),
	(&InstructionDecoder_amdgpu_vega::decodeMUBUF),
	(&InstructionDecoder_amdgpu_vega::decodeVOP3AB),
	(&InstructionDecoder_amdgpu_vega::decodeVOP3P),
	(&InstructionDecoder_amdgpu_vega::decodeFLAT)
};
InstructionFamily instr_family;
typedef struct vop_literal_layout_sdwa{
	unsigned int src0 : 8;
	unsigned int dst_sel : 3;
	unsigned int dst_u : 2;
	unsigned int clmp : 1;
	unsigned int omod : 2;
	unsigned int src0_sel : 3;
	unsigned int src0_next : 1;
	unsigned int src0_neg : 1;
	unsigned int src0_abs : 1;
	unsigned int s0 : 1;
	unsigned int src1_sel : 3;
	unsigned int src_sext : 1;
	unsigned int src1_neg : 1;
	unsigned int s1 : 1;
}vop_literal_layout_sdwa;
typedef struct vop_literal_layout_sdwab{
	unsigned int src0 : 8;
	unsigned int sdst : 7;
	unsigned int sd : 1;
	unsigned int src0_sel : 3;
	unsigned int src0_next : 1;
	unsigned int src0_neg : 1;
	unsigned int src0_abs : 1;
	unsigned int s0 : 1;
	unsigned int src1_sel : 3;
	unsigned int src_sext : 1;
	unsigned int src1_neg : 1;
	unsigned int s1 : 1;
}vop_literal_layout_sdwab;
typedef struct vop_literal_layout_dpp{
	unsigned int src0 : 8;
	unsigned int dpp_ctrl : 9;
	unsigned int bc : 1;
	unsigned int src0_neg : 1;
	unsigned int src0_abs : 1;
	unsigned int src1_neg : 1;
	unsigned int src1_abs : 1;
	unsigned int bank_mask : 4;
	unsigned int row_mask : 4;
}vop_literal_layout_dpp;
typedef union vop_literal_layout {
	vop_literal_layout_sdwa sdwa;
	vop_literal_layout_sdwab sdwab;
	vop_literal_layout_dpp dpp;
} vop_literal_layout;
#define IS_SOP2(I) ((longfield<30,31>(I) == 0x2)&&(longfield<28,29>(I) != 0x3)&&(longfield<23,29>(I) != 0x3d)&&(longfield<23,29>(I) != 0x3e)&&(longfield<23,29>(I) != 0x3f))
#define IS_SOP1(I) ((longfield<23,31>(I) == 0x17d))
#define IS_SOPK(I) ((longfield<30,31>(I) == 0x2)&&(longfield<28,31>(I) == 0xb)&&(longfield<23,27>(I) != 0x1d)&&(longfield<23,27>(I) != 0x1e)&&(longfield<23,27>(I) != 0x1f))
#define IS_SOPC(I) ((longfield<23,31>(I) == 0x17e))
#define IS_SOPP(I) ((longfield<23,31>(I) == 0x17f))
#define IS_SMEM(I) ((longfield<26,31>(I) == 0x30))
#define IS_VOP2(I) ((longfield<31,31>(I) == 0x0)&&(longfield<25,30>(I) != 0x3e)&&(longfield<25,30>(I) != 0x3f))
#define IS_VOP1(I) ((longfield<25,31>(I) == 0x3f))
#define IS_VOPC(I) ((longfield<25,31>(I) == 0x3e))
#define IS_VINTRP(I) ((longfield<26,31>(I) == 0x35))
#define IS_DS(I) ((longfield<26,31>(I) == 0x36))
#define IS_MTBUF(I) ((longfield<26,31>(I) == 0x3a))
#define IS_MUBUF(I) ((longfield<26,31>(I) == 0x38))
#define IS_VOP3AB(I) ((longfield<26,31>(I) == 0x34)&&(longfield<23,25>(I) != 0x7))
#define IS_VOP3P(I) ((longfield<23,31>(I) == 0x1a7))
#define IS_FLAT(I) ((longfield<26,31>(I) == 0x37))
typedef struct layout_sop2{
	unsigned int op : 7;
	unsigned int sdst : 7;
	unsigned int ssrc1 : 8;
	unsigned int ssrc0 : 8;
}layout_sop2;
typedef struct layout_sop1{
	unsigned int sdst : 7;
	unsigned int op : 8;
	unsigned int ssrc0 : 8;
}layout_sop1;
typedef struct layout_sopk{
	unsigned int op : 5;
	unsigned int sdst : 7;
	int simm16 : 16;
}layout_sopk;
typedef struct layout_sopc{
	unsigned int ssrc0 : 8;
	unsigned int ssrc1 : 8;
	unsigned int op : 7;
}layout_sopc;
typedef struct layout_sopp{
	int simm16 : 16;
	unsigned int op : 7;
}layout_sopp;
typedef struct layout_smem{
	unsigned int soffset : 7;
	unsigned int offset : 21;
	unsigned int op : 8;
	unsigned int imm : 1;
	unsigned int glc : 1;
	unsigned int nv : 1;
	unsigned int soe : 1;
	unsigned int sdata : 7;
	unsigned int sbase : 6;
}layout_smem;
typedef struct layout_vop2{
	unsigned int op : 6;
	unsigned int vdst : 8;
	unsigned int vsrc1 : 8;
	unsigned int src0 : 9;
	vop_literal_layout literal;
}layout_vop2;
typedef struct layout_vop1{
	unsigned int src0 : 9;
	unsigned int op : 8;
	unsigned int vdst : 8;
	vop_literal_layout literal;
}layout_vop1;
typedef struct layout_vopc{
	unsigned int src0 : 9;
	unsigned int vsrc1 : 8;
	unsigned int op : 8;
	vop_literal_layout literal;
}layout_vopc;
typedef struct layout_vintrp{
	unsigned int vsrc : 8;
	unsigned int attr_chan : 2;
	unsigned int attr : 6;
	unsigned int op : 2;
	unsigned int vdst : 8;
}layout_vintrp;
typedef struct layout_ds{
	unsigned int offset0 : 8;
	unsigned int offset1 : 8;
	unsigned int gds : 1;
	unsigned int op : 8;
	unsigned int addr : 8;
	unsigned int data0 : 8;
	unsigned int data1 : 8;
	unsigned int vdst : 8;
}layout_ds;
typedef struct layout_mtbuf{
	unsigned int offset : 12;
	unsigned int offen : 1;
	unsigned int idxen : 1;
	unsigned int glc : 1;
	unsigned int op : 4;
	unsigned int dfmt : 4;
	unsigned int nfmt : 3;
	unsigned int vaddr : 8;
	unsigned int vdata : 8;
	unsigned int srsrc : 5;
	unsigned int slc : 1;
	unsigned int tfe : 1;
	unsigned int soffset : 8;
}layout_mtbuf;
typedef struct layout_mubuf{
	unsigned int offset : 12;
	unsigned int offen : 1;
	unsigned int idxen : 1;
	unsigned int glc : 1;
	unsigned int lds : 1;
	unsigned int slc : 1;
	unsigned int op : 7;
	unsigned int vaddr : 8;
	unsigned int vdata : 8;
	unsigned int srsrc : 5;
	unsigned int tfe : 1;
	unsigned int soffset : 8;
}layout_mubuf;
typedef struct layout_vop3ab{
	unsigned int op : 10;
}layout_vop3ab;
typedef struct layout_vop3p{
	unsigned int vdst : 8;
	unsigned int neg_hi : 3;
	unsigned int opsel : 3;
	unsigned int opsel_hi2 : 1;
	unsigned int clmp : 1;
	unsigned int op : 7;
	unsigned int src0 : 9;
	unsigned int src1 : 9;
	unsigned int src2 : 9;
	unsigned int opsel_hi : 2;
	unsigned int neg : 3;
}layout_vop3p;
typedef struct layout_flat{
	unsigned int offset : 13;
	unsigned int lds : 1;
	unsigned int seg : 2;
	unsigned int glc : 1;
	unsigned int slc : 1;
	unsigned int op : 7;
	unsigned int addr : 8;
	unsigned int data : 8;
	unsigned int saddr : 7;
	unsigned int nv : 1;
	unsigned int vdst : 8;
}layout_flat;
union insn_layouts{
	 layout_sop2 sop2;
	 layout_sop1 sop1;
	 layout_sopk sopk;
	 layout_sopc sopc;
	 layout_sopp sopp;
	 layout_smem smem;
	 layout_vop2 vop2;
	 layout_vop1 vop1;
	 layout_vopc vopc;
	 layout_vintrp vintrp;
	 layout_ds ds;
	 layout_mtbuf mtbuf;
	 layout_mubuf mubuf;
	 layout_vop3ab vop3ab;
	 layout_vop3p vop3p;
	 layout_flat flat;
}insn_layout;
void decodeSOP2();
void decodeSOP1();
void decodeSOPK();
void decodeSOPC();
void decodeSOPP();
void decodeSMEM();
void decodeVOP2();
void decodeVOP1();
void decodeVOPC();
void decodeVINTRP();
void decodeDS();
void decodeMTBUF();
void decodeMUBUF();
void decodeVOP3AB();
void decodeVOP3P();
void decodeFLAT();
