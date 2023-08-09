#include <cstddef>

struct amdgpu_gfx90a_insn_entry {
	entryID op;
	const char *mnemonic;
	std::size_t operandCnt;
	const operandFactory *operands;
	static const amdgpu_gfx90a_insn_table main_insn_table ; 
	static const operandFactory operandTable[] ; 
	static const amdgpu_gfx90a_insn_table ENC_DS_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_FLAT_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_FLAT_GLBL_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_FLAT_SCRATCH_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_MIMG_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_MTBUF_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_MUBUF_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_SMEM_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_SOP1_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_SOP2_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_SOPC_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_SOPK_insn_table ; 
	static const amdgpu_gfx90a_insn_table SOPK_INST_LITERAL__insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_SOPP_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP1_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP3_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP2_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP2_LITERAL_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP3B_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP3P_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOP3P_MFMA_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VOPC_insn_table ; 
	static const amdgpu_gfx90a_insn_table ENC_VINTRP_insn_table ; 
};
