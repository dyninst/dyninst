void InstructionDecoder_amdgpu_vega::decodeSOP2(){
	unsigned insn_size_ = 4;
	layout_sop2 & layout = insn_layout.sop2;
	layout.ssrc0     = longfield<0,7>(insn_long);
	layout.ssrc1     = longfield<8,15>(insn_long);
	layout.sdst      = longfield<16,22>(insn_long);
	layout.op        = longfield<23,29>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sop2_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeSOP2Operands();
}
void InstructionDecoder_amdgpu_vega::decodeSOP1(){
	unsigned insn_size_ = 4;
	layout_sop1 & layout = insn_layout.sop1;
	layout.ssrc0     = longfield<0,7>(insn_long);
	layout.op        = longfield<8,15>(insn_long);
	layout.sdst      = longfield<16,22>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sop1_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeSOP1Operands();
}
void InstructionDecoder_amdgpu_vega::decodeSOPK(){
	unsigned insn_size_ = 4;
	layout_sopk & layout = insn_layout.sopk;
	layout.simm16    = longfield<0,15>(insn_long);
	layout.sdst      = longfield<16,22>(insn_long);
	layout.op        = longfield<23,27>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopk_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeSOPKOperands();
}
void InstructionDecoder_amdgpu_vega::decodeSOPC(){
	unsigned insn_size_ = 4;
	layout_sopc & layout = insn_layout.sopc;
	layout.ssrc0     = longfield<0,7>(insn_long);
	layout.ssrc1     = longfield<8,15>(insn_long);
	layout.op        = longfield<16,22>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopc_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeSOPCOperands();
}
void InstructionDecoder_amdgpu_vega::decodeSOPP(){
	unsigned insn_size_ = 4;
	layout_sopp & layout = insn_layout.sopp;
	layout.simm16    = longfield<0,15>(insn_long);
	layout.op        = longfield<16,22>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopp_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeSOPPOperands();
}
void InstructionDecoder_amdgpu_vega::decodeSMEM(){
	unsigned insn_size_ = 8;
	layout_smem & layout = insn_layout.smem;
	layout.sbase     = longfield<0,5>(insn_long);
	layout.sdata     = longfield<6,12>(insn_long);
	layout.soe       = longfield<14,14>(insn_long);
	layout.nv        = longfield<15,15>(insn_long);
	layout.glc       = longfield<16,16>(insn_long);
	layout.imm       = longfield<17,17>(insn_long);
	layout.op        = longfield<18,25>(insn_long);
	layout.offset    = longfield<32,52>(insn_long);
	layout.soffset   = longfield<57,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::smem_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeSMEMOperands();
}
void InstructionDecoder_amdgpu_vega::decodeVOP2(){
	unsigned insn_size_ = 4;
	layout_vop2 & layout = insn_layout.vop2;
	layout.src0      = longfield<0,8>(insn_long);
	layout.vsrc1     = longfield<9,16>(insn_long);
	layout.vdst      = longfield<17,24>(insn_long);
	layout.op        = longfield<25,30>(insn_long);
	if (layout.src0 == 249){
		vop_literal_layout_sdwa &vop_literal = layout.literal.sdwa;
		vop_literal.src0      = longfield<32,39>(insn_long);
		vop_literal.dst_sel   = longfield<40,42>(insn_long);
		vop_literal.dst_u     = longfield<43,44>(insn_long);
		vop_literal.clmp      = longfield<45,45>(insn_long);
		vop_literal.omod      = longfield<46,47>(insn_long);
		vop_literal.src0_sel  = longfield<48,50>(insn_long);
		vop_literal.src0_next = longfield<51,51>(insn_long);
		vop_literal.src0_neg  = longfield<52,52>(insn_long);
		vop_literal.src0_abs  = longfield<53,53>(insn_long);
		vop_literal.s0        = longfield<55,55>(insn_long);
		vop_literal.src1_sel  = longfield<56,58>(insn_long);
		vop_literal.src_sext  = longfield<59,59>(insn_long);
		vop_literal.src1_neg  = longfield<60,60>(insn_long);
		vop_literal.s1        = longfield<63,63>(insn_long);
	}
	if (layout.src0 == 250){
		vop_literal_layout_dpp &vop_literal = layout.literal.dpp;
		vop_literal.src0      = longfield<32,39>(insn_long);
		vop_literal.dpp_ctrl  = longfield<40,48>(insn_long);
		vop_literal.bc        = longfield<51,51>(insn_long);
		vop_literal.src0_neg  = longfield<52,52>(insn_long);
		vop_literal.src0_abs  = longfield<53,53>(insn_long);
		vop_literal.src1_neg  = longfield<54,54>(insn_long);
		vop_literal.src1_abs  = longfield<55,55>(insn_long);
		vop_literal.bank_mask = longfield<56,59>(insn_long);
		vop_literal.row_mask  = longfield<60,63>(insn_long);
	}
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop2_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVOP2Operands();
}
void InstructionDecoder_amdgpu_vega::decodeVOP1(){
	unsigned insn_size_ = 4;
	layout_vop1 & layout = insn_layout.vop1;
	layout.src0      = longfield<0,8>(insn_long);
	layout.op        = longfield<9,16>(insn_long);
	layout.vdst      = longfield<17,24>(insn_long);
	if (layout.src0 == 249){
		vop_literal_layout_sdwa &vop_literal = layout.literal.sdwa;
		vop_literal.src0      = longfield<32,39>(insn_long);
		vop_literal.dst_sel   = longfield<40,42>(insn_long);
		vop_literal.dst_u     = longfield<43,44>(insn_long);
		vop_literal.clmp      = longfield<45,45>(insn_long);
		vop_literal.omod      = longfield<46,47>(insn_long);
		vop_literal.src0_sel  = longfield<48,50>(insn_long);
		vop_literal.src0_next = longfield<51,51>(insn_long);
		vop_literal.src0_neg  = longfield<52,52>(insn_long);
		vop_literal.src0_abs  = longfield<53,53>(insn_long);
		vop_literal.s0        = longfield<55,55>(insn_long);
		vop_literal.src1_sel  = longfield<56,58>(insn_long);
		vop_literal.src_sext  = longfield<59,59>(insn_long);
		vop_literal.src1_neg  = longfield<60,60>(insn_long);
		vop_literal.s1        = longfield<63,63>(insn_long);
	}
	if (layout.src0 == 250){
		vop_literal_layout_dpp &vop_literal = layout.literal.dpp;
		vop_literal.src0      = longfield<32,39>(insn_long);
		vop_literal.dpp_ctrl  = longfield<40,48>(insn_long);
		vop_literal.bc        = longfield<51,51>(insn_long);
		vop_literal.src0_neg  = longfield<52,52>(insn_long);
		vop_literal.src0_abs  = longfield<53,53>(insn_long);
		vop_literal.src1_neg  = longfield<54,54>(insn_long);
		vop_literal.src1_abs  = longfield<55,55>(insn_long);
		vop_literal.bank_mask = longfield<56,59>(insn_long);
		vop_literal.row_mask  = longfield<60,63>(insn_long);
	}
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop1_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVOP1Operands();
}
void InstructionDecoder_amdgpu_vega::decodeVOPC(){
	unsigned insn_size_ = 4;
	layout_vopc & layout = insn_layout.vopc;
	layout.src0      = longfield<0,8>(insn_long);
	layout.vsrc1     = longfield<9,16>(insn_long);
	layout.op        = longfield<17,24>(insn_long);
	if (layout.src0 == 249){
		vop_literal_layout_sdwab &vop_literal = layout.literal.sdwab;
		vop_literal.src0      = longfield<32,39>(insn_long);
		vop_literal.sdst      = longfield<40,46>(insn_long);
		vop_literal.sd        = longfield<47,47>(insn_long);
		vop_literal.src0_sel  = longfield<48,50>(insn_long);
		vop_literal.src0_next = longfield<51,51>(insn_long);
		vop_literal.src0_neg  = longfield<52,52>(insn_long);
		vop_literal.src0_abs  = longfield<53,53>(insn_long);
		vop_literal.s0        = longfield<55,55>(insn_long);
		vop_literal.src1_sel  = longfield<56,58>(insn_long);
		vop_literal.src_sext  = longfield<59,59>(insn_long);
		vop_literal.src1_neg  = longfield<60,60>(insn_long);
		vop_literal.s1        = longfield<63,63>(insn_long);
	}
	if (layout.src0 == 250){
		vop_literal_layout_dpp &vop_literal = layout.literal.dpp;
		vop_literal.src0      = longfield<32,39>(insn_long);
		vop_literal.dpp_ctrl  = longfield<40,48>(insn_long);
		vop_literal.bc        = longfield<51,51>(insn_long);
		vop_literal.src0_neg  = longfield<52,52>(insn_long);
		vop_literal.src0_abs  = longfield<53,53>(insn_long);
		vop_literal.src1_neg  = longfield<54,54>(insn_long);
		vop_literal.src1_abs  = longfield<55,55>(insn_long);
		vop_literal.bank_mask = longfield<56,59>(insn_long);
		vop_literal.row_mask  = longfield<60,63>(insn_long);
	}
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vopc_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVOPCOperands();
}
void InstructionDecoder_amdgpu_vega::decodeVINTRP(){
	unsigned insn_size_ = 4;
	layout_vintrp & layout = insn_layout.vintrp;
	layout.vsrc      = longfield<0,7>(insn_long);
	layout.attr_chan = longfield<8,9>(insn_long);
	layout.attr      = longfield<10,15>(insn_long);
	layout.op        = longfield<16,17>(insn_long);
	layout.vdst      = longfield<18,25>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vintrp_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVINTRPOperands();
}
void InstructionDecoder_amdgpu_vega::decodeDS(){
	unsigned insn_size_ = 8;
	layout_ds & layout = insn_layout.ds;
	layout.offset0   = longfield<0,7>(insn_long);
	layout.offset1   = longfield<8,15>(insn_long);
	layout.gds       = longfield<16,16>(insn_long);
	layout.op        = longfield<17,24>(insn_long);
	layout.addr      = longfield<32,39>(insn_long);
	layout.data0     = longfield<40,47>(insn_long);
	layout.data1     = longfield<48,55>(insn_long);
	layout.vdst      = longfield<56,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::ds_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeDSOperands();
}
void InstructionDecoder_amdgpu_vega::decodeMTBUF(){
	unsigned insn_size_ = 8;
	layout_mtbuf & layout = insn_layout.mtbuf;
	layout.offset    = longfield<0,11>(insn_long);
	layout.offen     = longfield<12,12>(insn_long);
	layout.idxen     = longfield<13,13>(insn_long);
	layout.glc       = longfield<14,14>(insn_long);
	layout.op        = longfield<15,18>(insn_long);
	layout.dfmt      = longfield<19,22>(insn_long);
	layout.nfmt      = longfield<23,25>(insn_long);
	layout.vaddr     = longfield<32,39>(insn_long);
	layout.vdata     = longfield<40,47>(insn_long);
	layout.srsrc     = longfield<48,52>(insn_long);
	layout.slc       = longfield<54,54>(insn_long);
	layout.tfe       = longfield<55,55>(insn_long);
	layout.soffset   = longfield<56,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::mtbuf_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeMTBUFOperands();
}
void InstructionDecoder_amdgpu_vega::decodeMUBUF(){
	unsigned insn_size_ = 8;
	layout_mubuf & layout = insn_layout.mubuf;
	layout.offset    = longfield<0,11>(insn_long);
	layout.offen     = longfield<12,12>(insn_long);
	layout.idxen     = longfield<13,13>(insn_long);
	layout.glc       = longfield<14,14>(insn_long);
	layout.lds       = longfield<16,16>(insn_long);
	layout.slc       = longfield<17,17>(insn_long);
	layout.op        = longfield<18,24>(insn_long);
	layout.vaddr     = longfield<32,39>(insn_long);
	layout.vdata     = longfield<40,47>(insn_long);
	layout.srsrc     = longfield<48,52>(insn_long);
	layout.tfe       = longfield<55,55>(insn_long);
	layout.soffset   = longfield<56,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::mubuf_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeMUBUFOperands();
}
void InstructionDecoder_amdgpu_vega::decodeVOP3AB(){
	unsigned insn_size_ = 8;
	layout_vop3ab & layout = insn_layout.vop3ab;
	layout.op        = longfield<16,25>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop3ab_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVOP3ABOperands();
}
void InstructionDecoder_amdgpu_vega::decodeVOP3P(){
	unsigned insn_size_ = 8;
	layout_vop3p & layout = insn_layout.vop3p;
	layout.vdst      = longfield<0,7>(insn_long);
	layout.neg_hi    = longfield<8,10>(insn_long);
	layout.opsel     = longfield<11,13>(insn_long);
	layout.opsel_hi2 = longfield<14,14>(insn_long);
	layout.clmp      = longfield<15,15>(insn_long);
	layout.op        = longfield<16,22>(insn_long);
	layout.src0      = longfield<32,40>(insn_long);
	layout.src1      = longfield<41,49>(insn_long);
	layout.src2      = longfield<50,58>(insn_long);
	layout.opsel_hi  = longfield<59,60>(insn_long);
	layout.neg       = longfield<61,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop3p_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeVOP3POperands();
}
void InstructionDecoder_amdgpu_vega::decodeFLAT(){
	unsigned insn_size_ = 8;
	layout_flat & layout = insn_layout.flat;
	layout.offset    = longfield<0,12>(insn_long);
	layout.lds       = longfield<13,13>(insn_long);
	layout.seg       = longfield<14,15>(insn_long);
	layout.glc       = longfield<16,16>(insn_long);
	layout.slc       = longfield<17,17>(insn_long);
	layout.op        = longfield<18,24>(insn_long);
	layout.addr      = longfield<32,39>(insn_long);
	layout.data      = longfield<40,47>(insn_long);
	layout.saddr     = longfield<48,54>(insn_long);
	layout.nv        = longfield<55,55>(insn_long);
	layout.vdst      = longfield<56,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::flat_insn_table[layout.op];
	decodeOperands(insn_entry);
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeFLATOperands();
}
void InstructionDecoder_amdgpu_vega::mainDecode(InstructionDecoder::buffer &b){
	if(IS_SOP2(insn_long)){
		setUseImm<0,7,255>(b,4);
		setUseImm<8,15,255>(b,4);
		decodeSOP2();
	}
	else if(IS_SOP1(insn_long)){
		setUseImm<0,7,255>(b,4);
		decodeSOP1();
	}
	else if(IS_SOPK(insn_long)){
		decodeSOPK();
	}
	else if(IS_SOPC(insn_long)){
		decodeSOPC();
	}
	else if(IS_SOPP(insn_long)){
		decodeSOPP();
	}
	else if(IS_SMEM(insn_long)){
		decodeSMEM();
	}
	else if(IS_VOP2(insn_long)){
		setUseImm<0,8,249>(b,4);
		setUseImm<0,8,250>(b,4);
		setUseImm<0,8,255>(b,4);
		decodeVOP2();
	}
	else if(IS_VOP1(insn_long)){
		setUseImm<0,8,249>(b,4);
		setUseImm<0,8,250>(b,4);
		setUseImm<0,8,255>(b,4);
		decodeVOP1();
	}
	else if(IS_VOPC(insn_long)){
		setUseImm<0,8,255>(b,4);
		decodeVOPC();
	}
	else if(IS_VINTRP(insn_long)){
		decodeVINTRP();
	}
	else if(IS_DS(insn_long)){
		decodeDS();
	}
	else if(IS_MTBUF(insn_long)){
		decodeMTBUF();
	}
	else if(IS_MUBUF(insn_long)){
		decodeMUBUF();
	}
	else if(IS_VOP3AB(insn_long)){
		decodeVOP3AB();
	}
	else if(IS_VOP3P(insn_long)){
		decodeVOP3P();
	}
	else if(IS_FLAT(insn_long)){
		decodeFLAT();
	}
	else{
		assert(0);
	}

}
void InstructionDecoder_amdgpu_vega::mainDecodeOpcode(InstructionDecoder::buffer &b){
	if(IS_SOP2(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sop2_insn_table[longfield<23,29>(insn_long)];
		setUseImm<0,7,255>(b,4);
		setUseImm<8,15,255>(b,4);
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = SOP2;
	}
	else if(IS_SOP1(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sop1_insn_table[longfield<8,15>(insn_long)];
		setUseImm<0,7,255>(b,4);
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = SOP1;
	}
	else if(IS_SOPK(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopk_insn_table[longfield<23,27>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = SOPK;
	}
	else if(IS_SOPC(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopc_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = SOPC;
	}
	else if(IS_SOPP(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopp_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = SOPP;
	}
	else if(IS_SMEM(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::smem_insn_table[longfield<18,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = SMEM;
	}
	else if(IS_VOP2(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop2_insn_table[longfield<25,30>(insn_long)];
		setUseImm<0,8,249>(b,4);
		setUseImm<0,8,250>(b,4);
		setUseImm<0,8,255>(b,4);
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VOP2;
	}
	else if(IS_VOP1(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop1_insn_table[longfield<9,16>(insn_long)];
		setUseImm<0,8,249>(b,4);
		setUseImm<0,8,250>(b,4);
		setUseImm<0,8,255>(b,4);
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VOP1;
	}
	else if(IS_VOPC(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vopc_insn_table[longfield<17,24>(insn_long)];
		setUseImm<0,8,255>(b,4);
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VOPC;
	}
	else if(IS_VINTRP(insn_long)){
		unsigned insn_size_ = 4;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vintrp_insn_table[longfield<16,17>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VINTRP;
	}
	else if(IS_DS(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::ds_insn_table[longfield<17,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = DS;
	}
	else if(IS_MTBUF(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::mtbuf_insn_table[longfield<15,18>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = MTBUF;
	}
	else if(IS_MUBUF(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::mubuf_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = MUBUF;
	}
	else if(IS_VOP3AB(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop3ab_insn_table[longfield<16,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VOP3AB;
	}
	else if(IS_VOP3P(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop3p_insn_table[longfield<11,13>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = VOP3P;
	}
	else if(IS_FLAT(insn_long)){
		unsigned insn_size_ = 8;
		const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::flat_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size_+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = FLAT;
	}
	else{
		assert(0);
	}

}
