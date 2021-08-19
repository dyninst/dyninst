struct amdgpu_cdna_insn_entry {
	entryID op;
	const char *mnemonic;
	std::size_t operandCnt;
	const operandFactory *operands;
	static const amdgpu_cdna_insn_table main_insn_table;
	static const operandFactory operandTable[];
	static const amdgpu_cdna_insn_table ENC_SOP2_insn_table;
	static const amdgpu_cdna_insn_table ENC_SOP1_insn_table;
	static const amdgpu_cdna_insn_table ENC_SOPK_insn_table;
	static const amdgpu_cdna_insn_table ENC_SOPC_insn_table;
	static const amdgpu_cdna_insn_table ENC_SOPP_insn_table;
	static const amdgpu_cdna_insn_table ENC_SMEM_insn_table;
	static const amdgpu_cdna_insn_table ENC_VOP2_insn_table;
	static const amdgpu_cdna_insn_table ENC_VOP1_insn_table;
	static const amdgpu_cdna_insn_table ENC_VOPC_insn_table;
	static const amdgpu_cdna_insn_table ENC_VINTRP_insn_table;
	static const amdgpu_cdna_insn_table ENC_DS_insn_table;
	static const amdgpu_cdna_insn_table ENC_MTBUF_insn_table;
	static const amdgpu_cdna_insn_table ENC_MUBUF_insn_table;
	static const amdgpu_cdna_insn_table ENC_VOP3AB_insn_table;
	static const amdgpu_cdna_insn_table ENC_VOP3P_insn_table;
	static const amdgpu_cdna_insn_table ENC_VOP3_insn_table;
	static const amdgpu_cdna_insn_table ENC_FLAT_insn_table;
	static const amdgpu_cdna_insn_table ENC_MIMG_insn_table;
	static const amdgpu_cdna_insn_table ENC_EXP_insn_table;
	static const amdgpu_cdna_insn_table VOP3_0_SDST_ENC_VOP3_1__insn_table;
};
