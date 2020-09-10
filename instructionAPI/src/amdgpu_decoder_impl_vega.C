void InstructionDecoder_amdgpu::decodeSOP2(InstructionDecoder::buffer & b){
	unsigned insn_size = 4;
	layout_sop2 & layout = insn_layout.sop2;
	layout.ssrc0     = longfield<0,7>(insn_long);
	layout.ssrc1     = longfield<8,15>(insn_long);
	layout.sdst      = longfield<16,22>(insn_long);
	layout.op        = longfield<23,29>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sop2_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = sop2 , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeSOP1(InstructionDecoder::buffer & b){
	unsigned insn_size = 4;
	layout_sop1 & layout = insn_layout.sop1;
	layout.ssrc0     = longfield<0,7>(insn_long);
	layout.op        = longfield<8,15>(insn_long);
	layout.sdst      = longfield<16,22>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sop1_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = sop1 , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeSOPK(InstructionDecoder::buffer & b){
	unsigned insn_size = 4;
	layout_sopk & layout = insn_layout.sopk;
	layout.simm16    = longfield<0,15>(insn_long);
	layout.sdst      = longfield<16,22>(insn_long);
	layout.op        = longfield<23,27>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopk_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = sopk , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeSOPC(InstructionDecoder::buffer & b){
	unsigned insn_size = 4;
	layout_sopc & layout = insn_layout.sopc;
	layout.ssrc0     = longfield<0,7>(insn_long);
	layout.ssrc1     = longfield<8,15>(insn_long);
	layout.op        = longfield<16,22>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopc_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = sopc , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeSOPP(InstructionDecoder::buffer & b){
	unsigned insn_size = 4;
	layout_sopp & layout = insn_layout.sopp;
	layout.simm16    = longfield<0,15>(insn_long);
	layout.op        = longfield<16,22>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::sopp_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = sopp , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeSMEM(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
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
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = smem , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeVOP2(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
	layout_vop2 & layout = insn_layout.vop2;
	layout.src0      = longfield<0,8>(insn_long);
	layout.vsrc1     = longfield<9,16>(insn_long);
	layout.vdst      = longfield<17,24>(insn_long);
	layout.op        = longfield<25,30>(insn_long);
	layout.literal   = longfield<32,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop2_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = vop2 , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeVOP1(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
	layout_vop1 & layout = insn_layout.vop1;
	layout.src0      = longfield<0,9>(insn_long);
	layout.op        = longfield<9,16>(insn_long);
	layout.vdst      = longfield<17,24>(insn_long);
	layout.literal   = longfield<32,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop1_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = vop1 , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeVOPC(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
	layout_vopc & layout = insn_layout.vopc;
	layout.src0      = longfield<0,8>(insn_long);
	layout.vsrc1     = longfield<9,16>(insn_long);
	layout.op        = longfield<17,24>(insn_long);
	layout.literal   = longfield<32,63>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vopc_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = vopc , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeVINTRP(InstructionDecoder::buffer & b){
	unsigned insn_size = 4;
	layout_vintrp & layout = insn_layout.vintrp;
	layout.vsrc      = longfield<0,7>(insn_long);
	layout.attr_chan = longfield<8,9>(insn_long);
	layout.attr      = longfield<10,15>(insn_long);
	layout.op        = longfield<16,17>(insn_long);
	layout.vdst      = longfield<18,25>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vintrp_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = vintrp , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeDS(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
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
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = ds , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeMTBUF(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
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
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = mtbuf , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeMUBUF(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
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
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = mubuf , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeVOP3AB(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
	layout_vop3ab & layout = insn_layout.vop3ab;
	layout.op        = longfield<16,25>(insn_long);
	const amdgpu_insn_entry &insn_entry = amdgpu_insn_entry::vop3ab_insn_table[layout.op];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = vop3ab , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeVOP3P(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
	layout_vop3p & layout = insn_layout.vop3p;
	layout.vdst      = longfield<0,8>(insn_long);
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
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = vop3p , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::decodeFLAT(InstructionDecoder::buffer & b){
	unsigned insn_size = 8;
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
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
	cout << "type = flat , op = " << std::hex << layout.op << endl;
	decodeOperands(this->insn_in_progress.get(),insn_entry);
}
void InstructionDecoder_amdgpu::mainDecode(InstructionDecoder::buffer &b){
	if(IS_SOP2(insn_long)){
		decodeSOP2(b);
	}
	else if(IS_SOP1(insn_long)){
		decodeSOP1(b);
	}
	else if(IS_SOPK(insn_long)){
		decodeSOPK(b);
	}
	else if(IS_SOPC(insn_long)){
		decodeSOPC(b);
	}
	else if(IS_SOPP(insn_long)){
		decodeSOPP(b);
	}
	else if(IS_SMEM(insn_long)){
		decodeSMEM(b);
	}
	else if(IS_VOP2(insn_long)){
		decodeVOP2(b);
	}
	else if(IS_VOP1(insn_long)){
		decodeVOP1(b);
	}
	else if(IS_VOPC(insn_long)){
		decodeVOPC(b);
	}
	else if(IS_VINTRP(insn_long)){
		decodeVINTRP(b);
	}
	else if(IS_DS(insn_long)){
		decodeDS(b);
	}
	else if(IS_MTBUF(insn_long)){
		decodeMTBUF(b);
	}
	else if(IS_MUBUF(insn_long)){
		decodeMUBUF(b);
	}
	else if(IS_VOP3AB(insn_long)){
		decodeVOP3AB(b);
	}
	else if(IS_VOP3P(insn_long)){
		decodeVOP3P(b);
	}
	else if(IS_FLAT(insn_long)){
		decodeFLAT(b);
	}
	else{
		assert(0);
	}

}
