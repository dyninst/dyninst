struct aarch64_mask_entry;
struct aarch64_insn_entry;

typedef void (*operandFactory)();
typedef std::vector<operandFactory> operandSpec;
typedef std::vector<aarch64_insn_entry> aarch64_insn_table;
typedef std::map<unsigned int, unsigned int> branchMap;
typedef std::map<unsigned int, aarch64_mask_entry> aarch64_decoder_table;

typedef enum {
  aarch64_op_INVALID,
  aarch64_op_extended,
  aarch64_op_adc,
  aarch64_op_adcs,
  aarch64_op_add_addsub_ext,
  aarch64_op_add_addsub_imm,
  aarch64_op_add_addsub_shift,
  aarch64_op_adds_addsub_ext,
  aarch64_op_adds_addsub_imm,
  aarch64_op_adds_addsub_shift,
  aarch64_op_adr,
  aarch64_op_adrp,
  aarch64_op_and_log_imm,
  aarch64_op_and_log_shift,
  aarch64_op_ands_log_imm,
  aarch64_op_ands_log_shift,
  aarch64_op_asr_asrv,
  aarch64_op_asr_sbfm,
  aarch64_op_asrv,
  aarch64_op_at_sys,
  aarch64_op_b_cond,
  aarch64_op_b_uncond,
  aarch64_op_bfi_bfm,
  aarch64_op_bfm,
  aarch64_op_bfxil_bfm,
  aarch64_op_bic_log_shift,
  aarch64_op_bics,
  aarch64_op_bl,
  aarch64_op_blr,
  aarch64_op_br,
  aarch64_op_brk,
  aarch64_op_cbnz,
  aarch64_op_cbz,
  aarch64_op_ccmn_imm,
  aarch64_op_ccmn_reg,
  aarch64_op_ccmp_imm,
  aarch64_op_ccmp_reg,
  aarch64_op_cinc_csinc,
  aarch64_op_cinv_csinv,
  aarch64_op_clrex,
  aarch64_op_cls_int,
  aarch64_op_clz_int,
  aarch64_op_cmn_adds_addsub_ext,
  aarch64_op_cmn_adds_addsub_imm,
  aarch64_op_cmn_adds_addsub_shift,
  aarch64_op_cmp_subs_addsub_ext,
  aarch64_op_cmp_subs_addsub_imm,
  aarch64_op_cmp_subs_addsub_shift,
  aarch64_op_cneg_csneg,
  aarch64_op_crc32,
  aarch64_op_crc32c,
  aarch64_op_csel,
  aarch64_op_cset_csinc,
  aarch64_op_csetm_csinv,
  aarch64_op_csinc,
  aarch64_op_csinv,
  aarch64_op_csneg,
  aarch64_op_dc_sys,
  aarch64_op_dcps1,
  aarch64_op_dcps2,
  aarch64_op_dcps3,
  aarch64_op_dmb,
  aarch64_op_drps,
  aarch64_op_dsb,
  aarch64_op_eon,
  aarch64_op_eor_log_imm,
  aarch64_op_eor_log_shift,
  aarch64_op_eret,
  aarch64_op_extr,
  aarch64_op_hint,
  aarch64_op_hlt,
  aarch64_op_hvc,
  aarch64_op_ic_sys,
  aarch64_op_isb,
  aarch64_op_ldar,
  aarch64_op_ldarb,
  aarch64_op_ldarh,
  aarch64_op_ldaxp,
  aarch64_op_ldaxr,
  aarch64_op_ldaxrb,
  aarch64_op_ldaxrh,
  aarch64_op_ldnp_gen,
  aarch64_op_ldp_gen,
  aarch64_op_ldpsw,
  aarch64_op_ldr_imm_gen,
  aarch64_op_ldr_lit_gen,
  aarch64_op_ldr_reg_gen,
  aarch64_op_ldrb_imm,
  aarch64_op_ldrb_reg,
  aarch64_op_ldrh_imm,
  aarch64_op_ldrh_reg,
  aarch64_op_ldrsb_imm,
  aarch64_op_ldrsb_reg,
  aarch64_op_ldrsh_imm,
  aarch64_op_ldrsh_reg,
  aarch64_op_ldrsw_imm,
  aarch64_op_ldrsw_lit,
  aarch64_op_ldrsw_reg,
  aarch64_op_ldtr,
  aarch64_op_ldtrb,
  aarch64_op_ldtrh,
  aarch64_op_ldtrsb,
  aarch64_op_ldtrsh,
  aarch64_op_ldtrsw,
  aarch64_op_ldur_gen,
  aarch64_op_ldurb,
  aarch64_op_ldurh,
  aarch64_op_ldursb,
  aarch64_op_ldursh,
  aarch64_op_ldursw,
  aarch64_op_ldxp,
  aarch64_op_ldxr,
  aarch64_op_ldxrb,
  aarch64_op_ldxrh,
  aarch64_op_lsl_lslv,
  aarch64_op_lsl_ubfm,
  aarch64_op_lslv,
  aarch64_op_lsr_lsrv,
  aarch64_op_lsr_ubfm,
  aarch64_op_lsrv,
  aarch64_op_madd,
  aarch64_op_mneg_msub,
  aarch64_op_mov_add_addsub_imm,
  aarch64_op_mov_movn,
  aarch64_op_mov_movz,
  aarch64_op_mov_orr_log_imm,
  aarch64_op_mov_orr_log_shift,
  aarch64_op_movk,
  aarch64_op_movn,
  aarch64_op_movz,
  aarch64_op_mrs,
  aarch64_op_msr_imm,
  aarch64_op_msr_reg,
  aarch64_op_msub,
  aarch64_op_mul_madd,
  aarch64_op_mvn_orn_log_shift,
  aarch64_op_neg_sub_addsub_shift,
  aarch64_op_negs_subs_addsub_shift,
  aarch64_op_ngc_sbc,
  aarch64_op_ngcs_sbcs,
  aarch64_op_nop_hint,
  aarch64_op_orn_log_shift,
  aarch64_op_orr_log_imm,
  aarch64_op_orr_log_shift,
  aarch64_op_prfm_imm,
  aarch64_op_prfm_lit,
  aarch64_op_prfm_reg,
  aarch64_op_prfum,
  aarch64_op_rbit_int,
  aarch64_op_ret,
  aarch64_op_rev,
  aarch64_op_rev16_int,
  aarch64_op_rev32_int,
  aarch64_op_ror_extr,
  aarch64_op_ror_rorv,
  aarch64_op_rorv,
  aarch64_op_sbc,
  aarch64_op_sbcs,
  aarch64_op_sbfiz_sbfm,
  aarch64_op_sbfm,
  aarch64_op_sbfx_sbfm,
  aarch64_op_sdiv,
  aarch64_op_sev_hint,
  aarch64_op_sevl_hint,
  aarch64_op_smaddl,
  aarch64_op_smc,
  aarch64_op_smnegl_smsubl,
  aarch64_op_smsubl,
  aarch64_op_smulh,
  aarch64_op_smull_smaddl,
  aarch64_op_stlr,
  aarch64_op_stlrb,
  aarch64_op_stlrh,
  aarch64_op_stlxp,
  aarch64_op_stlxr,
  aarch64_op_stlxrb,
  aarch64_op_stlxrh,
  aarch64_op_stnp_gen,
  aarch64_op_stp_gen,
  aarch64_op_str_imm_gen,
  aarch64_op_str_reg_gen,
  aarch64_op_strb_imm,
  aarch64_op_strb_reg,
  aarch64_op_strh_imm,
  aarch64_op_strh_reg,
  aarch64_op_sttr,
  aarch64_op_sttrb,
  aarch64_op_sttrh,
  aarch64_op_stur_gen,
  aarch64_op_sturb,
  aarch64_op_sturh,
  aarch64_op_stxp,
  aarch64_op_stxr,
  aarch64_op_stxrb,
  aarch64_op_stxrh,
  aarch64_op_sub_addsub_ext,
  aarch64_op_sub_addsub_imm,
  aarch64_op_sub_addsub_shift,
  aarch64_op_subs_addsub_ext,
  aarch64_op_subs_addsub_imm,
  aarch64_op_subs_addsub_shift,
  aarch64_op_svc,
  aarch64_op_sxtb_sbfm,
  aarch64_op_sxth_sbfm,
  aarch64_op_sxtw_sbfm,
  aarch64_op_sys,
  aarch64_op_sysl,
  aarch64_op_tbnz,
  aarch64_op_tbz,
  aarch64_op_tlbi_sys,
  aarch64_op_tst_ands_log_imm,
  aarch64_op_tst_ands_log_shift,
  aarch64_op_ubfiz_ubfm,
  aarch64_op_ubfm,
  aarch64_op_ubfx_ubfm,
  aarch64_op_udiv,
  aarch64_op_umaddl,
  aarch64_op_umnegl_umsubl,
  aarch64_op_umsubl,
  aarch64_op_umulh,
  aarch64_op_umull_umaddl,
  aarch64_op_unallocated,
  aarch64_op_uxtb_ubfm,
  aarch64_op_uxth_ubfm,
  aarch64_op_wfe_hint,
  aarch64_op_wfi_hint,
  aarch64_op_yield_hint,
} entryID;

struct aarch64_insn_entry
{
	aarch64_insn_entry(entryID o, const char* m, operandSpec ops):
	op(o), mnemonic(m), operands(ops)
	{
	}

	aarch64_insn_entry():
	op(aarch64_op_INVALID), mnemonic("INVALID")
	{
		// TODO: why 5?
		// TODO: Is this needed here?
		operands.reserve(5);
	}

	aarch64_insn_entry(const aarch64_insn_entry& o) :
	op(o.op), mnemonic(o.mnemonic), operands(o.operands)
	{
	}

	const aarch64_insn_entry& operator=(const aarch64_insn_entry& rhs)
	{
		operands.reserve(rhs.operands.size());
		op = rhs.op;
		mnemonic = rhs.mnemonic;
		operands = rhs.operands;
	
		return *this;
	}

	entryID op;
	const char* mnemonic;
	operandSpec operands;
	
	static void buildInsnTable();
	static bool built_insn_table;
	
	static aarch64_insn_table main_insn_table;
};

struct aarch64_mask_entry
{		
	aarch64_mask_entry(unsigned int m, branchMap bm, int tabIndex):
	mask(m), nodeBranches(bm), insnTableIndex(tabIndex)
	{
	}
	
	aarch64_mask_entry():
	mask(0), nodeBranches(branchMap()), insnTableIndex(-1)
	{
	}
	
	aarch64_mask_entry(const aarch64_mask_entry& e):
	mask(e.mask), nodeBranches(e.nodeBranches), insnTableIndex(e.insnTableIndex)
	{
	}
	
	const aarch64_mask_entry& operator=(const aarch64_mask_entry& rhs)
	{
		mask = rhs.mask;
		nodeBranches = rhs.nodeBranches;
		insnTableIndex = rhs.insnTableIndex;
		
		return *this;
	}
	
	unsigned int mask;
	branchMap nodeBranches;
	int insnTableIndex;
	
	static void buildDecoderTable();
	static bool built_decoder_table;
	static aarch64_decoder_table main_decoder_table;
};

aarch64_decoder_table aarch64_mask_entry::main_decoder_table;
aarch64_insn_table aarch64_insn_entry::main_insn_table;

void aarch64_mask_entry::buildDecoderTable()
{
	main_decoder_table[0]=aarch64_mask_entry(0x18000000, map_list_of(0,1)(1,2)(2,3)(3,4),-1);
	main_decoder_table[1]=aarch64_mask_entry(0x0, branchMap(),0);
	main_decoder_table[2]=aarch64_mask_entry(0x27000000, map_list_of(0,5)(2,6)(3,7)(8,8)(10,9)(11,10),-1);
	main_decoder_table[5]=aarch64_mask_entry(0x80e08000, map_list_of(0,11)(1,12)(4,13)(5,14)(9,15)(13,16)(16,17)(17,18)(18,19)(19,20)(20,21)(21,22)(22,23)(23,24)(25,25)(29,26),-1);
	main_decoder_table[11]=aarch64_mask_entry(0x40000000, map_list_of(0,27)(1,28),-1);
	main_decoder_table[27]=aarch64_mask_entry(0x0, branchMap(),192);
	main_decoder_table[28]=aarch64_mask_entry(0x0, branchMap(),193);
	main_decoder_table[12]=aarch64_mask_entry(0x40000000, map_list_of(0,29)(1,30),-1);
	main_decoder_table[29]=aarch64_mask_entry(0x0, branchMap(),174);
	main_decoder_table[30]=aarch64_mask_entry(0x0, branchMap(),175);
	main_decoder_table[13]=aarch64_mask_entry(0x40000000, map_list_of(0,31)(1,32),-1);
	main_decoder_table[31]=aarch64_mask_entry(0x0, branchMap(),111);
	main_decoder_table[32]=aarch64_mask_entry(0x0, branchMap(),112);
	main_decoder_table[14]=aarch64_mask_entry(0x40000000, map_list_of(0,33)(1,34),-1);
	main_decoder_table[33]=aarch64_mask_entry(0x0, branchMap(),78);
	main_decoder_table[34]=aarch64_mask_entry(0x0, branchMap(),79);
	main_decoder_table[15]=aarch64_mask_entry(0x40000000, map_list_of(0,35)(1,36),-1);
	main_decoder_table[35]=aarch64_mask_entry(0x0, branchMap(),170);
	main_decoder_table[36]=aarch64_mask_entry(0x0, branchMap(),171);
	main_decoder_table[16]=aarch64_mask_entry(0x40000000, map_list_of(0,37)(1,38),-1);
	main_decoder_table[37]=aarch64_mask_entry(0x0, branchMap(),74);
	main_decoder_table[38]=aarch64_mask_entry(0x0, branchMap(),75);
	main_decoder_table[17]=aarch64_mask_entry(0x0, branchMap(),191);
	main_decoder_table[18]=aarch64_mask_entry(0x0, branchMap(),173);
	main_decoder_table[19]=aarch64_mask_entry(0x0, branchMap(),190);
	main_decoder_table[20]=aarch64_mask_entry(0x0, branchMap(),172);
	main_decoder_table[21]=aarch64_mask_entry(0x0, branchMap(),110);
	main_decoder_table[22]=aarch64_mask_entry(0x0, branchMap(),77);
	main_decoder_table[23]=aarch64_mask_entry(0x0, branchMap(),109);
	main_decoder_table[24]=aarch64_mask_entry(0x0, branchMap(),76);
	main_decoder_table[25]=aarch64_mask_entry(0x0, branchMap(),169);
	main_decoder_table[26]=aarch64_mask_entry(0x0, branchMap(),73);
	main_decoder_table[6]=aarch64_mask_entry(0x40200000, map_list_of(0,39)(1,40)(2,41)(3,42),-1);
	main_decoder_table[39]=aarch64_mask_entry(0x0, branchMap(),12);
	main_decoder_table[40]=aarch64_mask_entry(0x0, branchMap(),24);
	main_decoder_table[41]=aarch64_mask_entry(0x0, branchMap(),65);
	main_decoder_table[42]=aarch64_mask_entry(0x0, branchMap(),63);
	main_decoder_table[7]=aarch64_mask_entry(0x40200000, map_list_of(0,43)(1,44)(2,45)(3,46),-1);
	main_decoder_table[43]=aarch64_mask_entry(0x0, branchMap(),5);
	main_decoder_table[44]=aarch64_mask_entry(0x0, branchMap(),3);
	main_decoder_table[45]=aarch64_mask_entry(0x3e0, map_list_of(0,47)(31,48),-1);
	main_decoder_table[47]=aarch64_mask_entry(0x0, branchMap(),196);
	main_decoder_table[48]=aarch64_mask_entry(0x0, branchMap(),135);
	main_decoder_table[46]=aarch64_mask_entry(0x0, branchMap(),194);
	main_decoder_table[8]=aarch64_mask_entry(0x40c00000, map_list_of(0,49)(1,50)(2,51)(3,52)(7,53),-1);
	main_decoder_table[49]=aarch64_mask_entry(0x0, branchMap(),176);
	main_decoder_table[50]=aarch64_mask_entry(0x0, branchMap(),80);
	main_decoder_table[51]=aarch64_mask_entry(0x0, branchMap(),177);
	main_decoder_table[52]=aarch64_mask_entry(0x0, branchMap(),81);
	main_decoder_table[53]=aarch64_mask_entry(0x0, branchMap(),82);
	main_decoder_table[9]=aarch64_mask_entry(0x40200000, map_list_of(0,54)(1,55)(2,56)(3,57),-1);
	main_decoder_table[54]=aarch64_mask_entry(0x3e0, map_list_of(0,58)(31,59),-1);
	main_decoder_table[58]=aarch64_mask_entry(0x0, branchMap(),142);
	main_decoder_table[59]=aarch64_mask_entry(0x0, branchMap(),125);
	main_decoder_table[55]=aarch64_mask_entry(0x3e0, map_list_of(0,60)(31,61),-1);
	main_decoder_table[60]=aarch64_mask_entry(0x0, branchMap(),140);
	main_decoder_table[61]=aarch64_mask_entry(0x0, branchMap(),134);
	main_decoder_table[56]=aarch64_mask_entry(0x1f, map_list_of(0,62)(31,63),-1);
	main_decoder_table[62]=aarch64_mask_entry(0x0, branchMap(),14);
	main_decoder_table[63]=aarch64_mask_entry(0x0, branchMap(),210);
	main_decoder_table[57]=aarch64_mask_entry(0x0, branchMap(),25);
	main_decoder_table[10]=aarch64_mask_entry(0x40200000, map_list_of(0,64)(1,65)(2,66)(3,67),-1);
	main_decoder_table[64]=aarch64_mask_entry(0x1f, map_list_of(0,68)(31,69),-1);
	main_decoder_table[68]=aarch64_mask_entry(0x0, branchMap(),8);
	main_decoder_table[69]=aarch64_mask_entry(0x0, branchMap(),43);
	main_decoder_table[65]=aarch64_mask_entry(0xc00000, map_list_of(0,70),-1);
	main_decoder_table[70]=aarch64_mask_entry(0x1f, map_list_of(0,71)(31,72),-1);
	main_decoder_table[71]=aarch64_mask_entry(0x0, branchMap(),6);
	main_decoder_table[72]=aarch64_mask_entry(0x0, branchMap(),41);
	main_decoder_table[66]=aarch64_mask_entry(0x3ff, map_list_of(0,73)(31,74)(992,75),-1);
	main_decoder_table[73]=aarch64_mask_entry(0x0, branchMap(),199);
	main_decoder_table[74]=aarch64_mask_entry(0x0, branchMap(),46);
	main_decoder_table[75]=aarch64_mask_entry(0x0, branchMap(),136);
	main_decoder_table[67]=aarch64_mask_entry(0xc00000, map_list_of(0,76),-1);
	main_decoder_table[76]=aarch64_mask_entry(0x1f, map_list_of(0,77)(31,78),-1);
	main_decoder_table[77]=aarch64_mask_entry(0x0, branchMap(),197);
	main_decoder_table[78]=aarch64_mask_entry(0x0, branchMap(),44);
	main_decoder_table[3]=aarch64_mask_entry(0x4000000, map_list_of(0,79)(1,80),-1);
	main_decoder_table[79]=aarch64_mask_entry(0x3000000, map_list_of(0,81)(1,82)(2,83)(3,84),-1);
	main_decoder_table[81]=aarch64_mask_entry(0x80000000, map_list_of(0,85)(1,86),-1);
	main_decoder_table[85]=aarch64_mask_entry(0x0, branchMap(),9);
	main_decoder_table[86]=aarch64_mask_entry(0x0, branchMap(),10);
	main_decoder_table[82]=aarch64_mask_entry(0x60000000, map_list_of(0,87)(1,88)(2,89)(3,90),-1);
	main_decoder_table[87]=aarch64_mask_entry(0x0, branchMap(),4);
	main_decoder_table[88]=aarch64_mask_entry(0x1f, map_list_of(0,91)(31,92),-1);
	main_decoder_table[91]=aarch64_mask_entry(0x0, branchMap(),7);
	main_decoder_table[92]=aarch64_mask_entry(0x0, branchMap(),42);
	main_decoder_table[89]=aarch64_mask_entry(0x0, branchMap(),195);
	main_decoder_table[90]=aarch64_mask_entry(0x1f, map_list_of(0,93)(31,94),-1);
	main_decoder_table[93]=aarch64_mask_entry(0x0, branchMap(),198);
	main_decoder_table[94]=aarch64_mask_entry(0x0, branchMap(),45);
	main_decoder_table[83]=aarch64_mask_entry(0x60800000, map_list_of(0,95)(1,96)(2,97)(4,98)(5,99)(6,100)(7,101),-1);
	main_decoder_table[95]=aarch64_mask_entry(0x0, branchMap(),11);
	main_decoder_table[96]=aarch64_mask_entry(0x0, branchMap(),122);
	main_decoder_table[97]=aarch64_mask_entry(0x3e0, map_list_of(0,102)(31,103),-1);
	main_decoder_table[102]=aarch64_mask_entry(0x0, branchMap(),141);
	main_decoder_table[103]=aarch64_mask_entry(0x0, branchMap(),124);
	main_decoder_table[98]=aarch64_mask_entry(0x0, branchMap(),64);
	main_decoder_table[99]=aarch64_mask_entry(0x0, branchMap(),123);
	main_decoder_table[100]=aarch64_mask_entry(0x1f, map_list_of(0,104)(31,105),-1);
	main_decoder_table[104]=aarch64_mask_entry(0x0, branchMap(),13);
	main_decoder_table[105]=aarch64_mask_entry(0x0, branchMap(),209);
	main_decoder_table[101]=aarch64_mask_entry(0x0, branchMap(),126);
	main_decoder_table[84]=aarch64_mask_entry(0x60800000, map_list_of(0,106)(1,107)(2,108)(4,109),-1);
	main_decoder_table[106]=aarch64_mask_entry(0x80402000, map_list_of(0,110)(1,111)(7,112),-1);
	main_decoder_table[110]=aarch64_mask_entry(0x1c00, map_list_of(0,113)(7,114),-1);
	main_decoder_table[113]=aarch64_mask_entry(0x0, branchMap(),157);
	main_decoder_table[114]=aarch64_mask_entry(0x0, branchMap(),201);
	main_decoder_table[111]=aarch64_mask_entry(0x5c00, map_list_of(7,115)(15,116),-1);
	main_decoder_table[115]=aarch64_mask_entry(0x0, branchMap(),202);
	main_decoder_table[116]=aarch64_mask_entry(0x0, branchMap(),16);
	main_decoder_table[112]=aarch64_mask_entry(0x0, branchMap(),203);
	main_decoder_table[107]=aarch64_mask_entry(0x200000, map_list_of(0,117),-1);
	main_decoder_table[117]=aarch64_mask_entry(0x0, branchMap(),67);
	main_decoder_table[108]=aarch64_mask_entry(0x0, branchMap(),21);
	main_decoder_table[109]=aarch64_mask_entry(0x5c00, map_list_of(0,118)(7,119)(15,120),-1);
	main_decoder_table[118]=aarch64_mask_entry(0x0, branchMap(),114);
	main_decoder_table[119]=aarch64_mask_entry(0x807fa000, map_list_of(0,121)(1,122),-1);
	main_decoder_table[121]=aarch64_mask_entry(0x0, branchMap(),220);
	main_decoder_table[122]=aarch64_mask_entry(0x0, branchMap(),221);
	main_decoder_table[120]=aarch64_mask_entry(0x0, branchMap(),117);
	main_decoder_table[80]=aarch64_mask_entry(0x60000000, map_list_of(0,123)(1,124)(2,125),-1);
	main_decoder_table[123]=aarch64_mask_entry(0x80000000, map_list_of(0,126)(1,127),-1);
	main_decoder_table[126]=aarch64_mask_entry(0x0, branchMap(),20);
	main_decoder_table[127]=aarch64_mask_entry(0x0, branchMap(),26);
	main_decoder_table[124]=aarch64_mask_entry(0x3000000, map_list_of(0,128)(1,129)(2,130)(3,131),-1);
	main_decoder_table[128]=aarch64_mask_entry(0x0, branchMap(),31);
	main_decoder_table[129]=aarch64_mask_entry(0x0, branchMap(),30);
	main_decoder_table[130]=aarch64_mask_entry(0x0, branchMap(),207);
	main_decoder_table[131]=aarch64_mask_entry(0x0, branchMap(),206);
	main_decoder_table[125]=aarch64_mask_entry(0x83000000, map_list_of(0,132)(4,133)(5,134)(6,135),-1);
	main_decoder_table[132]=aarch64_mask_entry(0x0, branchMap(),19);
	main_decoder_table[133]=aarch64_mask_entry(0xe0001f, map_list_of(1,136)(2,137)(3,138)(32,139)(64,140)(161,141)(162,142)(163,143),-1);
	main_decoder_table[136]=aarch64_mask_entry(0x0, branchMap(),200);
	main_decoder_table[137]=aarch64_mask_entry(0x0, branchMap(),70);
	main_decoder_table[138]=aarch64_mask_entry(0x0, branchMap(),164);
	main_decoder_table[139]=aarch64_mask_entry(0x0, branchMap(),29);
	main_decoder_table[140]=aarch64_mask_entry(0x0, branchMap(),69);
	main_decoder_table[141]=aarch64_mask_entry(0x0, branchMap(),57);
	main_decoder_table[142]=aarch64_mask_entry(0x0, branchMap(),58);
	main_decoder_table[143]=aarch64_mask_entry(0x0, branchMap(),59);
	main_decoder_table[134]=aarch64_mask_entry(0xf00000, map_list_of(0,144)(1,145)(2,146)(3,147),-1);
	main_decoder_table[144]=aarch64_mask_entry(0x80000, map_list_of(0,148)(1,149),-1);
	main_decoder_table[148]=aarch64_mask_entry(0xf01f, map_list_of(95,150)(127,151)(159,152),-1);
	main_decoder_table[150]=aarch64_mask_entry(0x70000, map_list_of(3,153),-1);
	main_decoder_table[153]=aarch64_mask_entry(0x20, map_list_of(0,154)(1,155),-1);
	main_decoder_table[154]=aarch64_mask_entry(0xc0, map_list_of(0,156)(1,157)(2,158),-1);
	main_decoder_table[156]=aarch64_mask_entry(0x0, branchMap(),68);
	main_decoder_table[157]=aarch64_mask_entry(0x0, branchMap(),222);
	main_decoder_table[158]=aarch64_mask_entry(0x0, branchMap(),161);
	main_decoder_table[155]=aarch64_mask_entry(0xfc0, map_list_of(0,159)(1,160)(2,161),-1);
	main_decoder_table[159]=aarch64_mask_entry(0x0, branchMap(),224);
	main_decoder_table[160]=aarch64_mask_entry(0x0, branchMap(),223);
	main_decoder_table[161]=aarch64_mask_entry(0x0, branchMap(),162);
	main_decoder_table[151]=aarch64_mask_entry(0x700e0, map_list_of(26,162)(28,163)(29,164)(30,165),-1);
	main_decoder_table[162]=aarch64_mask_entry(0x0, branchMap(),38);
	main_decoder_table[163]=aarch64_mask_entry(0x0, branchMap(),62);
	main_decoder_table[164]=aarch64_mask_entry(0x0, branchMap(),60);
	main_decoder_table[165]=aarch64_mask_entry(0x0, branchMap(),72);
	main_decoder_table[152]=aarch64_mask_entry(0x0, branchMap(),130);
	main_decoder_table[149]=aarch64_mask_entry(0xf800, map_list_of(0,166)(14,167)(15,168)(16,169),-1);
	main_decoder_table[166]=aarch64_mask_entry(0x0, branchMap(),204);
	main_decoder_table[167]=aarch64_mask_entry(0x0, branchMap(),56);
	main_decoder_table[168]=aarch64_mask_entry(0x0, branchMap(),18);
	main_decoder_table[169]=aarch64_mask_entry(0x0, branchMap(),208);
	main_decoder_table[145]=aarch64_mask_entry(0x0, branchMap(),131);
	main_decoder_table[146]=aarch64_mask_entry(0x0, branchMap(),205);
	main_decoder_table[147]=aarch64_mask_entry(0x0, branchMap(),129);
	main_decoder_table[135]=aarch64_mask_entry(0xfffc1f, map_list_of(63488,170)(129024,171)(194560,172)(325632,173)(391168,174),-1);
	main_decoder_table[170]=aarch64_mask_entry(0x0, branchMap(),28);
	main_decoder_table[171]=aarch64_mask_entry(0x0, branchMap(),27);
	main_decoder_table[172]=aarch64_mask_entry(0x0, branchMap(),148);
	main_decoder_table[173]=aarch64_mask_entry(0x0, branchMap(),66);
	main_decoder_table[174]=aarch64_mask_entry(0x0, branchMap(),61);
	main_decoder_table[4]=aarch64_mask_entry(0x27000000, map_list_of(0,175)(2,176)(3,177)(8,178)(9,179)(10,180),-1);
	main_decoder_table[175]=aarch64_mask_entry(0x80000000, map_list_of(0,181)(1,182),-1);
	main_decoder_table[181]=aarch64_mask_entry(0x0, branchMap(),84);
	main_decoder_table[182]=aarch64_mask_entry(0x40000000, map_list_of(0,183)(1,184),-1);
	main_decoder_table[183]=aarch64_mask_entry(0x0, branchMap(),95);
	main_decoder_table[184]=aarch64_mask_entry(0x0, branchMap(),144);
	main_decoder_table[176]=aarch64_mask_entry(0x40e00000, map_list_of(0,185)(4,186)(6,187)(8,188)(12,189)(14,190),-1);
	main_decoder_table[185]=aarch64_mask_entry(0x0, branchMap(),1);
	main_decoder_table[186]=aarch64_mask_entry(0xc00, map_list_of(0,191)(1,192),-1);
	main_decoder_table[191]=aarch64_mask_entry(0x0, branchMap(),50);
	main_decoder_table[192]=aarch64_mask_entry(0x1f03e0, map_list_of(0,193)(1023,194),-1);
	main_decoder_table[193]=aarch64_mask_entry(0x0, branchMap(),36);
	main_decoder_table[194]=aarch64_mask_entry(0x0, branchMap(),51);
	main_decoder_table[187]=aarch64_mask_entry(0xf000, map_list_of(0,195)(2,196)(4,197)(5,198),-1);
	main_decoder_table[195]=aarch64_mask_entry(0xc00, map_list_of(2,199)(3,200),-1);
	main_decoder_table[199]=aarch64_mask_entry(0x0, branchMap(),214);
	main_decoder_table[200]=aarch64_mask_entry(0x0, branchMap(),160);
	main_decoder_table[196]=aarch64_mask_entry(0xc00, map_list_of(0,201)(1,202)(2,203)(3,204),-1);
	main_decoder_table[201]=aarch64_mask_entry(0x0, branchMap(),113);
	main_decoder_table[202]=aarch64_mask_entry(0x0, branchMap(),116);
	main_decoder_table[203]=aarch64_mask_entry(0x0, branchMap(),15);
	main_decoder_table[204]=aarch64_mask_entry(0x0, branchMap(),153);
	main_decoder_table[197]=aarch64_mask_entry(0x0, branchMap(),48);
	main_decoder_table[198]=aarch64_mask_entry(0x0, branchMap(),49);
	main_decoder_table[188]=aarch64_mask_entry(0xfc00, map_list_of(0,205),-1);
	main_decoder_table[205]=aarch64_mask_entry(0x3e0, map_list_of(0,206)(31,207),-1);
	main_decoder_table[206]=aarch64_mask_entry(0x0, branchMap(),155);
	main_decoder_table[207]=aarch64_mask_entry(0x0, branchMap(),137);
	main_decoder_table[189]=aarch64_mask_entry(0xc00, map_list_of(0,208)(1,209),-1);
	main_decoder_table[208]=aarch64_mask_entry(0x1f03e0, map_list_of(0,210)(1023,211),-1);
	main_decoder_table[210]=aarch64_mask_entry(0x0, branchMap(),37);
	main_decoder_table[211]=aarch64_mask_entry(0x0, branchMap(),52);
	main_decoder_table[209]=aarch64_mask_entry(0x0, branchMap(),47);
	main_decoder_table[190]=aarch64_mask_entry(0x1ff800, map_list_of(0,212)(1,213)(2,214),-1);
	main_decoder_table[212]=aarch64_mask_entry(0x400, map_list_of(0,215)(1,216),-1);
	main_decoder_table[215]=aarch64_mask_entry(0x0, branchMap(),147);
	main_decoder_table[216]=aarch64_mask_entry(0x0, branchMap(),150);
	main_decoder_table[213]=aarch64_mask_entry(0x80000000, map_list_of(0,217)(1,218),-1);
	main_decoder_table[217]=aarch64_mask_entry(0x0, branchMap(),149);
	main_decoder_table[218]=aarch64_mask_entry(0x0, branchMap(),151);
	main_decoder_table[214]=aarch64_mask_entry(0x400, map_list_of(0,219)(1,220),-1);
	main_decoder_table[219]=aarch64_mask_entry(0x0, branchMap(),40);
	main_decoder_table[220]=aarch64_mask_entry(0x0, branchMap(),39);
	main_decoder_table[177]=aarch64_mask_entry(0x40e08000, map_list_of(0,221)(1,222)(2,223)(3,224)(4,225)(10,226)(11,227)(12,228),-1);
	main_decoder_table[221]=aarch64_mask_entry(0x7c00, map_list_of(0,229)(31,230),-1);
	main_decoder_table[229]=aarch64_mask_entry(0x0, branchMap(),119);
	main_decoder_table[230]=aarch64_mask_entry(0x0, branchMap(),133);
	main_decoder_table[222]=aarch64_mask_entry(0x7c00, map_list_of(0,231)(31,232),-1);
	main_decoder_table[231]=aarch64_mask_entry(0x0, branchMap(),132);
	main_decoder_table[232]=aarch64_mask_entry(0x0, branchMap(),120);
	main_decoder_table[223]=aarch64_mask_entry(0x80000000, map_list_of(1,233),-1);
	main_decoder_table[233]=aarch64_mask_entry(0x7c00, map_list_of(0,234)(31,235),-1);
	main_decoder_table[234]=aarch64_mask_entry(0x0, branchMap(),163);
	main_decoder_table[235]=aarch64_mask_entry(0x0, branchMap(),168);
	main_decoder_table[224]=aarch64_mask_entry(0x80000000, map_list_of(1,236),-1);
	main_decoder_table[236]=aarch64_mask_entry(0x7c00, map_list_of(0,237)(31,238),-1);
	main_decoder_table[237]=aarch64_mask_entry(0x0, branchMap(),166);
	main_decoder_table[238]=aarch64_mask_entry(0x0, branchMap(),165);
	main_decoder_table[225]=aarch64_mask_entry(0x0, branchMap(),167);
	main_decoder_table[226]=aarch64_mask_entry(0x80000000, map_list_of(1,239),-1);
	main_decoder_table[239]=aarch64_mask_entry(0x7c00, map_list_of(0,240)(31,241),-1);
	main_decoder_table[240]=aarch64_mask_entry(0x0, branchMap(),215);
	main_decoder_table[241]=aarch64_mask_entry(0x0, branchMap(),219);
	main_decoder_table[227]=aarch64_mask_entry(0x80000000, map_list_of(1,242),-1);
	main_decoder_table[242]=aarch64_mask_entry(0x7c00, map_list_of(0,243)(31,244),-1);
	main_decoder_table[243]=aarch64_mask_entry(0x0, branchMap(),217);
	main_decoder_table[244]=aarch64_mask_entry(0x0, branchMap(),216);
	main_decoder_table[228]=aarch64_mask_entry(0x0, branchMap(),218);
	main_decoder_table[178]=aarch64_mask_entry(0x80a00c00, map_list_of(0,245)(1,246)(2,247)(6,248)(8,249)(9,250)(10,251)(14,252)(16,253)(17,254)(18,255)(22,256)(24,257)(25,258)(26,259)(30,260),-1);
	main_decoder_table[245]=aarch64_mask_entry(0x40400000, map_list_of(0,261)(1,262)(2,263)(3,264),-1);
	main_decoder_table[261]=aarch64_mask_entry(0x0, branchMap(),188);
	main_decoder_table[262]=aarch64_mask_entry(0x0, branchMap(),104);
	main_decoder_table[263]=aarch64_mask_entry(0x0, branchMap(),189);
	main_decoder_table[264]=aarch64_mask_entry(0x0, branchMap(),105);
	main_decoder_table[246]=aarch64_mask_entry(0x40400000, map_list_of(0,265)(1,266)(2,267)(3,268),-1);
	main_decoder_table[265]=aarch64_mask_entry(0x0, branchMap(),180);
	main_decoder_table[266]=aarch64_mask_entry(0x0, branchMap(),86);
	main_decoder_table[267]=aarch64_mask_entry(0x0, branchMap(),182);
	main_decoder_table[268]=aarch64_mask_entry(0x0, branchMap(),88);
	main_decoder_table[247]=aarch64_mask_entry(0x40400000, map_list_of(0,269)(1,270)(2,271)(3,272),-1);
	main_decoder_table[269]=aarch64_mask_entry(0x0, branchMap(),185);
	main_decoder_table[270]=aarch64_mask_entry(0x0, branchMap(),98);
	main_decoder_table[271]=aarch64_mask_entry(0x0, branchMap(),186);
	main_decoder_table[272]=aarch64_mask_entry(0x0, branchMap(),99);
	main_decoder_table[248]=aarch64_mask_entry(0x40400000, map_list_of(0,273)(1,274)(2,275)(3,276),-1);
	main_decoder_table[273]=aarch64_mask_entry(0x0, branchMap(),181);
	main_decoder_table[274]=aarch64_mask_entry(0x0, branchMap(),87);
	main_decoder_table[275]=aarch64_mask_entry(0x0, branchMap(),183);
	main_decoder_table[276]=aarch64_mask_entry(0x0, branchMap(),89);
	main_decoder_table[249]=aarch64_mask_entry(0x40000000, map_list_of(0,277)(1,278),-1);
	main_decoder_table[277]=aarch64_mask_entry(0x0, branchMap(),106);
	main_decoder_table[278]=aarch64_mask_entry(0x0, branchMap(),107);
	main_decoder_table[250]=aarch64_mask_entry(0x40000000, map_list_of(0,279)(1,280),-1);
	main_decoder_table[279]=aarch64_mask_entry(0x0, branchMap(),90);
	main_decoder_table[280]=aarch64_mask_entry(0x0, branchMap(),92);
	main_decoder_table[251]=aarch64_mask_entry(0x40000000, map_list_of(0,281)(1,282),-1);
	main_decoder_table[281]=aarch64_mask_entry(0x0, branchMap(),100);
	main_decoder_table[282]=aarch64_mask_entry(0x0, branchMap(),101);
	main_decoder_table[252]=aarch64_mask_entry(0x40000000, map_list_of(0,283)(1,284),-1);
	main_decoder_table[283]=aarch64_mask_entry(0x0, branchMap(),91);
	main_decoder_table[284]=aarch64_mask_entry(0x0, branchMap(),93);
	main_decoder_table[253]=aarch64_mask_entry(0x400000, map_list_of(0,285)(1,286),-1);
	main_decoder_table[285]=aarch64_mask_entry(0x0, branchMap(),187);
	main_decoder_table[286]=aarch64_mask_entry(0x0, branchMap(),103);
	main_decoder_table[254]=aarch64_mask_entry(0x400000, map_list_of(0,287)(1,288),-1);
	main_decoder_table[287]=aarch64_mask_entry(0x0, branchMap(),178);
	main_decoder_table[288]=aarch64_mask_entry(0x0, branchMap(),83);
	main_decoder_table[255]=aarch64_mask_entry(0x400000, map_list_of(0,289)(1,290),-1);
	main_decoder_table[289]=aarch64_mask_entry(0x0, branchMap(),184);
	main_decoder_table[290]=aarch64_mask_entry(0x0, branchMap(),97);
	main_decoder_table[256]=aarch64_mask_entry(0x400000, map_list_of(0,291)(1,292),-1);
	main_decoder_table[291]=aarch64_mask_entry(0x0, branchMap(),179);
	main_decoder_table[292]=aarch64_mask_entry(0x0, branchMap(),85);
	main_decoder_table[257]=aarch64_mask_entry(0x40400000, map_list_of(0,293)(2,294),-1);
	main_decoder_table[293]=aarch64_mask_entry(0x0, branchMap(),108);
	main_decoder_table[294]=aarch64_mask_entry(0x0, branchMap(),146);
	main_decoder_table[258]=aarch64_mask_entry(0x0, branchMap(),94);
	main_decoder_table[259]=aarch64_mask_entry(0x0, branchMap(),102);
	main_decoder_table[260]=aarch64_mask_entry(0x40400000, map_list_of(0,295)(2,296),-1);
	main_decoder_table[295]=aarch64_mask_entry(0x0, branchMap(),96);
	main_decoder_table[296]=aarch64_mask_entry(0x0, branchMap(),145);
	main_decoder_table[179]=aarch64_mask_entry(0x0, branchMap(),143);
	main_decoder_table[180]=aarch64_mask_entry(0x40e00c00, map_list_of(0,297)(8,298)(10,299)(32,300)(40,301)(42,302),-1);
	main_decoder_table[297]=aarch64_mask_entry(0x0, branchMap(),2);
	main_decoder_table[298]=aarch64_mask_entry(0x0, branchMap(),33);
	main_decoder_table[299]=aarch64_mask_entry(0x0, branchMap(),32);
	main_decoder_table[300]=aarch64_mask_entry(0xf000, map_list_of(0,303),-1);
	main_decoder_table[303]=aarch64_mask_entry(0x3e0, map_list_of(0,304)(31,305),-1);
	main_decoder_table[304]=aarch64_mask_entry(0x0, branchMap(),156);
	main_decoder_table[305]=aarch64_mask_entry(0x0, branchMap(),138);
	main_decoder_table[301]=aarch64_mask_entry(0x0, branchMap(),35);
	main_decoder_table[302]=aarch64_mask_entry(0x0, branchMap(),34);
}

void aarch64_insn_entry::buildInsnTable()
{
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_unallocated, 	"unallocated	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adc, 	"adc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adcs, 	"adcs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_addsub_ext, 	"add	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_addsub_imm, 	"add	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_add_addsub_shift, 	"add	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adds_addsub_ext, 	"adds	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adds_addsub_imm, 	"adds	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adds_addsub_shift, 	"adds	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adr, 	"adr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_adrp, 	"adrp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_and_log_imm, 	"and	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_and_log_shift, 	"and	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ands_log_imm, 	"ands	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ands_log_shift, 	"ands	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_asr_asrv, 	"asr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_asr_sbfm, 	"asr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_asrv, 	"asrv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_at_sys, 	"at	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_b_cond, 	"b	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_b_uncond, 	"b	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bfi_bfm, 	"bfi	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bfm, 	"bfm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bfxil_bfm, 	"bfxil	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bic_log_shift, 	"bic	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bics, 	"bics	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_bl, 	"bl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_blr, 	"blr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_br, 	"br	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_brk, 	"brk	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cbnz, 	"cbnz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cbz, 	"cbz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmn_imm, 	"ccmn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmn_reg, 	"ccmn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmp_imm, 	"ccmp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ccmp_reg, 	"ccmp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cinc_csinc, 	"cinc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cinv_csinv, 	"cinv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_clrex, 	"clrex	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cls_int, 	"cls	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_clz_int, 	"clz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmn_adds_addsub_ext, 	"cmn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmn_adds_addsub_imm, 	"cmn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmn_adds_addsub_shift, 	"cmn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmp_subs_addsub_ext, 	"cmp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmp_subs_addsub_imm, 	"cmp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cmp_subs_addsub_shift, 	"cmp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cneg_csneg, 	"cneg	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_crc32, 	"crc32	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_crc32c, 	"crc32c	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csel, 	"csel	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_cset_csinc, 	"cset	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csetm_csinv, 	"csetm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csinc, 	"csinc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csinv, 	"csinv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_csneg, 	"csneg	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dc_sys, 	"dc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dcps1, 	"dcps1	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dcps2, 	"dcps2	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dcps3, 	"dcps3	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dmb, 	"dmb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_drps, 	"drps	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_dsb, 	"dsb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eon, 	"eon	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eor_log_imm, 	"eor	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eor_log_shift, 	"eor	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_eret, 	"eret	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_extr, 	"extr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_hint, 	"hint	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_hlt, 	"hlt	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_hvc, 	"hvc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ic_sys, 	"ic	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_isb, 	"isb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldar, 	"ldar	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldarb, 	"ldarb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldarh, 	"ldarh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxp, 	"ldaxp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxr, 	"ldaxr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxrb, 	"ldaxrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldaxrh, 	"ldaxrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldnp_gen, 	"ldnp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldp_gen, 	"ldp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldpsw, 	"ldpsw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_imm_gen, 	"ldr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_lit_gen, 	"ldr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldr_reg_gen, 	"ldr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrb_imm, 	"ldrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrb_reg, 	"ldrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrh_imm, 	"ldrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrh_reg, 	"ldrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsb_imm, 	"ldrsb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsb_reg, 	"ldrsb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsh_imm, 	"ldrsh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsh_reg, 	"ldrsh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_imm, 	"ldrsw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_lit, 	"ldrsw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldrsw_reg, 	"ldrsw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtr, 	"ldtr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrb, 	"ldtrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrh, 	"ldtrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrsb, 	"ldtrsb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrsh, 	"ldtrsh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldtrsw, 	"ldtrsw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldur_gen, 	"ldur	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldurb, 	"ldurb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldurh, 	"ldurh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldursb, 	"ldursb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldursh, 	"ldursh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldursw, 	"ldursw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxp, 	"ldxp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxr, 	"ldxr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxrb, 	"ldxrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ldxrh, 	"ldxrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsl_lslv, 	"lsl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsl_ubfm, 	"lsl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lslv, 	"lslv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsr_lsrv, 	"lsr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsr_ubfm, 	"lsr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_lsrv, 	"lsrv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_madd, 	"madd	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mneg_msub, 	"mneg	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_add_addsub_imm, 	"mov	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_movn, 	"mov	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_movz, 	"mov	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_orr_log_imm, 	"mov	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mov_orr_log_shift, 	"mov	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movk, 	"movk	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movn, 	"movn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_movz, 	"movz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mrs, 	"mrs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_msr_imm, 	"msr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_msr_reg, 	"msr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_msub, 	"msub	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mul_madd, 	"mul	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_mvn_orn_log_shift, 	"mvn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_neg_sub_addsub_shift, 	"neg	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_negs_subs_addsub_shift, 	"negs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ngc_sbc, 	"ngc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ngcs_sbcs, 	"ngcs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_nop_hint, 	"nop	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orn_log_shift, 	"orn	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orr_log_imm, 	"orr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_orr_log_shift, 	"orr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfm_imm, 	"prfm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfm_lit, 	"prfm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfm_reg, 	"prfm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_prfum, 	"prfum	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rbit_int, 	"rbit	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ret, 	"ret	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev, 	"rev	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev16_int, 	"rev16	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rev32_int, 	"rev32	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ror_extr, 	"ror	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ror_rorv, 	"ror	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_rorv, 	"rorv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbc, 	"sbc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbcs, 	"sbcs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbfiz_sbfm, 	"sbfiz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbfm, 	"sbfm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sbfx_sbfm, 	"sbfx	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sdiv, 	"sdiv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sev_hint, 	"sev	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sevl_hint, 	"sevl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smaddl, 	"smaddl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smc, 	"smc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smnegl_smsubl, 	"smnegl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smsubl, 	"smsubl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smulh, 	"smulh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_smull_smaddl, 	"smull	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlr, 	"stlr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlrb, 	"stlrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlrh, 	"stlrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxp, 	"stlxp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxr, 	"stlxr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxrb, 	"stlxrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stlxrh, 	"stlxrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stnp_gen, 	"stnp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stp_gen, 	"stp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_imm_gen, 	"str	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_str_reg_gen, 	"str	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strb_imm, 	"strb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strb_reg, 	"strb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strh_imm, 	"strh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_strh_reg, 	"strh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sttr, 	"sttr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sttrb, 	"sttrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sttrh, 	"sttrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stur_gen, 	"stur	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sturb, 	"sturb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sturh, 	"sturh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxp, 	"stxp	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxr, 	"stxr	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxrb, 	"stxrb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_stxrh, 	"stxrh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_addsub_ext, 	"sub	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_addsub_imm, 	"sub	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sub_addsub_shift, 	"sub	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subs_addsub_ext, 	"subs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subs_addsub_imm, 	"subs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_subs_addsub_shift, 	"subs	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_svc, 	"svc	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxtb_sbfm, 	"sxtb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxth_sbfm, 	"sxth	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sxtw_sbfm, 	"sxtw	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sys, 	"sys	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_sysl, 	"sysl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tbnz, 	"tbnz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tbz, 	"tbz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tlbi_sys, 	"tlbi	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tst_ands_log_imm, 	"tst	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_tst_ands_log_shift, 	"tst	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ubfiz_ubfm, 	"ubfiz	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ubfm, 	"ubfm	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_ubfx_ubfm, 	"ubfx	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_udiv, 	"udiv	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umaddl, 	"umaddl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umnegl_umsubl, 	"umnegl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umsubl, 	"umsubl	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umulh, 	"umulh	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_umull_umaddl, 	"umull	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uxtb_ubfm, 	"uxtb	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_uxth_ubfm, 	"uxth	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_wfe_hint, 	"wfe	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_wfi_hint, 	"wfi	", operandSpec() ));
	main_insn_table.push_back(aarch64_insn_entry(aarch64_op_yield_hint, 	"yield	", operandSpec() ));
}
