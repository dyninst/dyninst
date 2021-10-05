void InstructionDecoder_amdgpu_cdna::decodeENC_SOP1(){
	insn_size = 4;
	layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<8,15>(insn_long);
	layout.SDST = longfield<16,22>(insn_long);
	layout.SSRC0 = longfield<0,7>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOP1_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOP1Operands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_SOPC(){
	insn_size = 4;
	layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.SSRC0 = longfield<0,7>(insn_long);
	layout.SSRC1 = longfield<8,15>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOPC_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOPCOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_SOPP(){
	insn_size = 4;
	layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.SIMM16 = longfield<0,15>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOPP_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOPPOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_SOPK(){
	insn_size = 4;
	layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
	layout.ENCODING = longfield<28,31>(insn_long);
	layout.OP = longfield<23,27>(insn_long);
	layout.SDST = longfield<16,22>(insn_long);
	layout.SIMM16 = longfield<0,15>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOPK_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOPKOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_SOP2(){
	insn_size = 4;
	layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
	layout.ENCODING = longfield<30,31>(insn_long);
	layout.OP = longfield<23,29>(insn_long);
	layout.SDST = longfield<16,22>(insn_long);
	layout.SSRC0 = longfield<0,7>(insn_long);
	layout.SSRC1 = longfield<8,15>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOP2_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOP2Operands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_SMEM(){
	insn_size = 8;
	layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.GLC = longfield<16,16>(insn_long);
	layout.IMM = longfield<17,17>(insn_long);
	layout.NV = longfield<15,15>(insn_long);
	layout.OP = longfield<18,25>(insn_long);
	layout.SBASE = longfield<0,5>(insn_long);
	layout.SDATA = longfield<6,12>(insn_long);
	layout.SOFFSET_EN = longfield<14,14>(insn_long);
	layout.OFFSET = longfield<32,52>(insn_long);
	layout.SOFFSET = longfield<57,63>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SMEM_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SMEMOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_VOP1(){
	insn_size = 4;
	layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
	layout.ENCODING = longfield<25,31>(insn_long);
	layout.OP = longfield<9,16>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VDST = longfield<17,24>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP1_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP1Operands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_VOPC(){
	insn_size = 4;
	layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
	layout.ENCODING = longfield<25,31>(insn_long);
	layout.OP = longfield<17,24>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VSRC1 = longfield<9,16>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOPC_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOPCOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_VOP2(){
	insn_size = 4;
	layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
	layout.ENCODING = longfield<31,31>(insn_long);
	layout.OP = longfield<25,30>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VDST = longfield<17,24>(insn_long);
	layout.VSRC1 = longfield<9,16>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP2_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP2Operands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_VINTRP(){
	insn_size = 4;
	layout_ENC_VINTRP & layout = insn_layout.ENC_VINTRP;
	layout.ATTR = longfield<10,15>(insn_long);
	layout.ATTRCHAN = longfield<8,9>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.OP = longfield<16,17>(insn_long);
	layout.VDST = longfield<18,25>(insn_long);
	layout.VSRC = longfield<0,7>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VINTRP_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VINTRPOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_VOP3P(){
	insn_size = 8;
	layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
	layout.CLAMP = longfield<15,15>(insn_long);
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.NEG_HI = longfield<8,10>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.OP_SEL = longfield<11,13>(insn_long);
	layout.OP_SEL_HI_2 = longfield<14,14>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	layout.NEG = longfield<61,63>(insn_long);
	layout.OP_SEL_HI = longfield<59,60>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP3P_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP3POperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_VOP3(){
	insn_size = 8;
	layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
	layout.ABS = longfield<8,10>(insn_long);
	layout.CLAMP = longfield<15,15>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.OP = longfield<16,25>(insn_long);
	layout.OP_SEL = longfield<11,14>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	layout.NEG = longfield<61,63>(insn_long);
	layout.OMOD = longfield<59,60>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP3_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP3Operands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_DS(){
	insn_size = 8;
	layout_ENC_DS & layout = insn_layout.ENC_DS;
	layout.ACC = longfield<25,25>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.GDS = longfield<16,16>(insn_long);
	layout.OFFSET0 = longfield<0,7>(insn_long);
	layout.OFFSET1 = longfield<8,15>(insn_long);
	layout.OP = longfield<17,24>(insn_long);
	layout.ADDR = longfield<32,39>(insn_long);
	layout.DATA0 = longfield<40,47>(insn_long);
	layout.DATA1 = longfield<48,55>(insn_long);
	layout.VDST = longfield<56,63>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_DS_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_DSOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_MUBUF(){
	insn_size = 8;
	layout_ENC_MUBUF & layout = insn_layout.ENC_MUBUF;
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.IDXEN = longfield<13,13>(insn_long);
	layout.LDS = longfield<16,16>(insn_long);
	layout.NT = longfield<17,17>(insn_long);
	layout.OFFEN = longfield<12,12>(insn_long);
	layout.OFFSET = longfield<0,11>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.SC0 = longfield<14,14>(insn_long);
	layout.SC1 = longfield<15,15>(insn_long);
	layout.ACC = longfield<55,55>(insn_long);
	layout.SOFFSET = longfield<56,63>(insn_long);
	layout.SRSRC = longfield<48,52>(insn_long);
	layout.VADDR = longfield<32,39>(insn_long);
	layout.VDATA = longfield<40,47>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_MUBUF_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_MUBUFOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_MTBUF(){
	insn_size = 8;
	layout_ENC_MTBUF & layout = insn_layout.ENC_MTBUF;
	layout.DFMT = longfield<19,22>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.IDXEN = longfield<13,13>(insn_long);
	layout.NFMT = longfield<23,25>(insn_long);
	layout.OFFEN = longfield<12,12>(insn_long);
	layout.OFFSET = longfield<0,11>(insn_long);
	layout.OP = longfield<15,18>(insn_long);
	layout.SC0 = longfield<14,14>(insn_long);
	layout.ACC = longfield<55,55>(insn_long);
	layout.NT = longfield<54,54>(insn_long);
	layout.SC1 = longfield<53,53>(insn_long);
	layout.SOFFSET = longfield<56,63>(insn_long);
	layout.SRSRC = longfield<48,52>(insn_long);
	layout.VADDR = longfield<32,39>(insn_long);
	layout.VDATA = longfield<40,47>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_MTBUF_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_MTBUFOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_MIMG(){
	insn_size = 8;
	layout_ENC_MIMG & layout = insn_layout.ENC_MIMG;
	layout.A16 = longfield<15,15>(insn_long);
	layout.ACC = longfield<16,16>(insn_long);
	layout.DA = longfield<14,14>(insn_long);
	layout.DMASK = longfield<8,11>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.LWE = longfield<17,17>(insn_long);
	layout.NT = longfield<25,25>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.OPM = longfield<0,0>(insn_long);
	layout.SC0 = longfield<13,13>(insn_long);
	layout.SC1 = longfield<7,7>(insn_long);
	layout.UNORM = longfield<12,12>(insn_long);
	layout.D16 = longfield<63,63>(insn_long);
	layout.SRSRC = longfield<48,52>(insn_long);
	layout.SSAMP = longfield<53,57>(insn_long);
	layout.VADDR = longfield<32,39>(insn_long);
	layout.VDATA = longfield<40,47>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_MIMG_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_MIMGOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeENC_EXP(){
	insn_size = 8;
	//layout_ENC_EXP & layout = insn_layout.ENC_EXP;
}
void InstructionDecoder_amdgpu_cdna::decodeENC_FLAT(){
	insn_size = 8;
	layout_ENC_FLAT & layout = insn_layout.ENC_FLAT;
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.NT = longfield<17,17>(insn_long);
	layout.OFFSET = longfield<0,11>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.SC0 = longfield<16,16>(insn_long);
	layout.SC1 = longfield<25,25>(insn_long);
	layout.SEG = longfield<14,15>(insn_long);
	layout.SVE = longfield<13,13>(insn_long);
	layout.ACC = longfield<55,55>(insn_long);
	layout.ADDR = longfield<32,39>(insn_long);
	layout.DATA = longfield<40,47>(insn_long);
	layout.SADDR = longfield<48,54>(insn_long);
	layout.VDST = longfield<56,63>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_FLAT_insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_FLATOperands();
}
void InstructionDecoder_amdgpu_cdna::decodeVOP3_0_SDST_ENC_VOP3_1_(){
	insn_size = 8;
	layout_VOP3_0_SDST_ENC_VOP3_1_ & layout = insn_layout.VOP3_0_SDST_ENC_VOP3_1_;
	layout.CLAMP = longfield<15,15>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.OP = longfield<16,25>(insn_long);
	layout.SDST = longfield<8,14>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	layout.NEG = longfield<61,63>(insn_long);
	layout.OMOD = longfield<59,60>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::VOP3_0_SDST_ENC_VOP3_1__insn_table[layout.OP];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVOP3_0_SDST_ENC_VOP3_1_Operands();
}
void InstructionDecoder_amdgpu_cdna::mainDecodeOpcode(){
	if(IS_ENC_SOP1(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOP1_insn_table[longfield<8,15>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOP1;
	}
	else 	if(IS_ENC_SOPC(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOPC_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOPC;
	}
	else 	if(IS_ENC_SOPP(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOPP_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOPP;
	}
	else 	if(IS_ENC_SOPK(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOPK_insn_table[longfield<23,27>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOPK;
	}
	else 	if(IS_ENC_SOP2(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SOP2_insn_table[longfield<23,29>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOP2;
	}
	else 	if(IS_ENC_SMEM(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_SMEM_insn_table[longfield<18,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SMEM;
	}
	else 	if(IS_ENC_VOP1(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP1_insn_table[longfield<9,16>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP1;
	}
	else 	if(IS_ENC_VOPC(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOPC_insn_table[longfield<17,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOPC;
	}
	else 	if(IS_ENC_VOP2(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP2_insn_table[longfield<25,30>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP2;
	}
	else 	if(IS_ENC_VINTRP(insn_long)){
		insn_size = 4;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VINTRP_insn_table[longfield<16,17>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VINTRP;
	}
	else 	if(IS_ENC_VOP3P(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP3P_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP3P;
	}
	else 	if(IS_ENC_VOP3(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_VOP3_insn_table[longfield<16,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP3;
	}
	else 	if(IS_ENC_DS(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_DS_insn_table[longfield<17,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_DS;
	}
	else 	if(IS_ENC_MUBUF(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_MUBUF_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_MUBUF;
	}
	else 	if(IS_ENC_MTBUF(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_MTBUF_insn_table[longfield<15,18>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_MTBUF;
	}
	else 	if(IS_ENC_MIMG(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_MIMG_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_MIMG;
	}
	else 	if(IS_ENC_EXP(insn_long)){
		insn_size = 8;
		instr_family = ENC_EXP;
	}
	else 	if(IS_ENC_FLAT(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::ENC_FLAT_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_FLAT;
	}
	else 	if(IS_VOP3_0_SDST_ENC_VOP3_1_(insn_long)){
		insn_size = 8;
		const amdgpu_cdna_insn_entry &insn_entry = amdgpu_cdna_insn_entry::VOP3_0_SDST_ENC_VOP3_1__insn_table[longfield<16,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VOP3_0_SDST_ENC_VOP3_1_;
	}
}
void InstructionDecoder_amdgpu_cdna::mainDecode(){
	if(IS_ENC_SOP1(insn_long)){
		decodeENC_SOP1();
	}
	else 	if(IS_ENC_SOPC(insn_long)){
		decodeENC_SOPC();
	}
	else 	if(IS_ENC_SOPP(insn_long)){
		decodeENC_SOPP();
	}
	else 	if(IS_ENC_SOPK(insn_long)){
		decodeENC_SOPK();
	}
	else 	if(IS_ENC_SOP2(insn_long)){
		decodeENC_SOP2();
	}
	else 	if(IS_ENC_SMEM(insn_long)){
		decodeENC_SMEM();
	}
	else 	if(IS_ENC_VOP1(insn_long)){
		decodeENC_VOP1();
	}
	else 	if(IS_ENC_VOPC(insn_long)){
		decodeENC_VOPC();
	}
	else 	if(IS_ENC_VOP2(insn_long)){
		decodeENC_VOP2();
	}
	else 	if(IS_ENC_VINTRP(insn_long)){
		decodeENC_VINTRP();
	}
	else 	if(IS_ENC_VOP3P(insn_long)){
		decodeENC_VOP3P();
	}
	else 	if(IS_ENC_VOP3(insn_long)){
		decodeENC_VOP3();
	}
	else 	if(IS_ENC_DS(insn_long)){
		decodeENC_DS();
	}
	else 	if(IS_ENC_MUBUF(insn_long)){
		decodeENC_MUBUF();
	}
	else 	if(IS_ENC_MTBUF(insn_long)){
		decodeENC_MTBUF();
	}
	else 	if(IS_ENC_MIMG(insn_long)){
		decodeENC_MIMG();
	}
	else 	if(IS_ENC_EXP(insn_long)){
		decodeENC_EXP();
	}
	else 	if(IS_ENC_FLAT(insn_long)){
		decodeENC_FLAT();
	}
	else 	if(IS_VOP3_0_SDST_ENC_VOP3_1_(insn_long)){
		decodeVOP3_0_SDST_ENC_VOP3_1_();
	}
}
