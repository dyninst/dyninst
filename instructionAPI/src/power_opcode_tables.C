/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
std::once_flag built_tables;

std::vector<power_entry> power_entry::main_opcode_table;
power_table power_entry::extended_op_0;
power_table power_entry::extended_op_4;
power_table power_entry::extended_op_4_1409;
power_table power_entry::extended_op_4_1538;
power_table power_entry::extended_op_4_1921;
power_table power_entry::extended_op_19;
power_table power_entry::extended_op_30;
power_table power_entry::extended_op_31;
power_table power_entry::extended_op_57;
power_table power_entry::extended_op_58;
power_table power_entry::extended_op_59;
power_table power_entry::extended_op_60;
power_table power_entry::extended_op_60_specials;
power_table power_entry::extended_op_60_347;
power_table power_entry::extended_op_60_475;
power_table power_entry::extended_op_61;
power_table power_entry::extended_op_63;
power_table power_entry::extended_op_63_583;
power_table power_entry::extended_op_63_804;
power_table power_entry::extended_op_63_836;
power_entry invalid_entry;

void power_entry::buildTables()
{
    std::call_once(built_tables, [&]() {
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_0), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_tdi, "tdi", NULL, list_of(fn(TO))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_twi, "twi", NULL, list_of(fn(TO))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_4), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_INVALID, "INVALID", NULL, operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_mulli, "mulli", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_subfic, "subfic", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_dozi, "dozi", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_cmpli, "cmpli", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_cmpi, "cmpi", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addic, "addic", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addic_rc, "addic.", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addi, "addi", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_addis, "addis", NULL, list_of(fn(RT))(fn(RA))(fn(SI))));
    main_opcode_table.push_back(power_entry(power_op_bc, "bc", NULL, list_of(fn(BO))(fn(BI))(fn(BD))(fn(AA))(fn(LK))));
    main_opcode_table.push_back(power_entry(power_op_svcs, "sc", NULL, list_of(fn(syscall))));
    main_opcode_table.push_back(power_entry(power_op_b, "b", NULL, list_of(fn(LL))(fn(AA))(fn(LK))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_19), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_rlwimi, "rlwimi", NULL,
                                list_of(fn(RA))(fn(RS))(fn(SH))(fn(ME))(fn(MB))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_rlwinm, "rlwinm", NULL,
                                list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(ME))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_rlmi, "rlmi", NULL,
                                list_of(fn(RA))(fn(RS))(fn(RB))(fn(MB))(fn(ME))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_rlwnm, "rlwnm", NULL,
                                list_of(fn(RA))(fn(RS))(fn(RB))(fn(MB))(fn(ME))(fn(Rc))));
    main_opcode_table.push_back(power_entry(power_op_ori, "ori", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_oris, "oris", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_xori, "xori", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_xoris, "xoris", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_andi_rc, "andi.", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_andis_rc, "andis.", NULL, list_of(fn(RA))(fn(RS))(fn(UI))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_30), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_31), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_lwz, "lwz", NULL, list_of(fn(RT))(fn(L<u32>))));
    main_opcode_table.push_back(power_entry(power_op_lwzu, "lwzu", NULL, list_of(fn(RT))(fn(LU<u32>))));
    main_opcode_table.push_back(power_entry(power_op_lbz, "lbz", NULL, list_of(fn(RT))(fn(L<u8>))));
    main_opcode_table.push_back(power_entry(power_op_lbzu, "lbzu", NULL, list_of(fn(RT))(fn(LU<u8>))));
    main_opcode_table.push_back(power_entry(power_op_stw, "stw", NULL, list_of(fn(RS))(fn(ST<u32>))));
    main_opcode_table.push_back(power_entry(power_op_stwu, "stwu", NULL, list_of(fn(RS))(fn(STU<u32>))));
    main_opcode_table.push_back(power_entry(power_op_stb, "stb", NULL, list_of(fn(RS))(fn(ST<u8>))));
    main_opcode_table.push_back(power_entry(power_op_stbu, "stbu", NULL, list_of(fn(RS))(fn(STU<u8>))));
    main_opcode_table.push_back(power_entry(power_op_lhz, "lhz", NULL, list_of(fn(RT))(fn(L<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lhzu, "lhzu", NULL, list_of(fn(RT))(fn(LU<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lha, "lha", NULL, list_of(fn(RT))(fn(L<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lhau, "lhau", NULL, list_of(fn(RT))(fn(LU<u16>))));
    main_opcode_table.push_back(power_entry(power_op_sth, "sth", NULL, list_of(fn(RS))(fn(ST<u16>))));
    main_opcode_table.push_back(power_entry(power_op_sthu, "sthu", NULL, list_of(fn(RS))(fn(STU<u16>))));
    main_opcode_table.push_back(power_entry(power_op_lmw, "lmw", NULL, list_of(fn(RT))(fn(L<u32>))));
    main_opcode_table.push_back(power_entry(power_op_stmw, "stmw", NULL, list_of(fn(RT))(fn(ST<u32>))));
    main_opcode_table.push_back(power_entry(power_op_lfs, "lfs", NULL, list_of(fn(FRT))(fn(L<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfsu, "lfsu", NULL, list_of(fn(FRT))(fn(LU<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfd, "lfd", NULL, list_of(fn(FRT))(fn(L<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfdu, "lfdu", NULL, list_of(fn(FRT))(fn(LU<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfs, "stfs", NULL, list_of(fn(FRS))(fn(ST<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfsu, "stfsu", NULL, list_of(fn(FRS))(fn(STU<sp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfd, "stfd", NULL, list_of(fn(FRS))(fn(ST<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_stfdu, "stfdu", NULL, list_of(fn(FRS))(fn(STU<dp_float>))));
    main_opcode_table.push_back(power_entry(power_op_lfq, "lfq", NULL, list_of(fn(FRT2))(fn(L<dbl128>))));
   	//opcode57: revised, previous:lfqu
	 	//main_opcode_table.push_back(power_entry(power_op_lfqu, "lfqu", NULL, list_of(fn(FRT2))(fn(LU<dbl128>))));i
		//new extended opcode 57	
		main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_57), operandSpec()));
		
		main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_58), operandSpec()));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_59), operandSpec()));
    //opcode60 revised, previous:stfq
    //new extended opcode 60
		main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_60), operandSpec()));

    //opcode61 revised, previous:stfqu
		//main_opcode_table.push_back(power_entry(power_op_stfqu, "stfqu", NULL, list_of(fn(FRS2))(fn(STU<dbl128>))));
    //new extended opcode 61 
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_61), operandSpec()));
		
		main_opcode_table.push_back(power_entry(power_op_stdu, "stdu", NULL, list_of(fn(RS))(fn(STU<u64>))));
    main_opcode_table.push_back(power_entry(power_op_extended, "extended", fn(extended_op_63), operandSpec()));

        extended_op_0[1] = power_entry(power_op_qvfxxmadds, "qvfxxmadds", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
        extended_op_0[3] = power_entry(power_op_qvfxxcpnmadds, "qvfxxcpnmadds", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
        extended_op_0[5] = power_entry(power_op_fpsel, "fpsel", NULL,
                                       list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[8] = power_entry(power_op_fpmul, "fpmul", NULL,
                                       list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRCP)));
        extended_op_0[10] = power_entry(power_op_fxpmul, "fxpmul", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRCP)));
        extended_op_0[12] = power_entry(power_op_fpadd, "fpadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP)));
        extended_op_0[13] = power_entry(power_op_fpsub, "fpsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP)));
        extended_op_0[14] = power_entry(power_op_fpre, "fpre", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
        extended_op_0[15] = power_entry(power_op_fprsqrte, "fprsqrte", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
        extended_op_0[16] = power_entry(power_op_fpmadd, "fpmadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[17] = power_entry(power_op_fxmadd, "fxmadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[18] = power_entry(power_op_fxcpmadd, "fxcpmadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
        extended_op_0[19] = power_entry(power_op_fxcsmadd, "fxcsmadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));

        extended_op_0[9] = power_entry(power_op_fxmul, "fxmul", NULL,
                                       list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRCP)));
        extended_op_0[11] = power_entry(power_op_fxsmul, "fxsmul", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRCP)));
        extended_op_0[20] = power_entry(power_op_fpnmadd, "fpnmadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[21] = power_entry(power_op_fxnmadd, "fxnmadd", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[24] = power_entry(power_op_fpmsub, "fpmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[25] = power_entry(power_op_fxmsub, "fxmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[26] = power_entry(power_op_fxcpmsub, "fxcpmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
        extended_op_0[28] = power_entry(power_op_fpnmsub, "fpnmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[29] = power_entry(power_op_fxnmsub, "fxnmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAP))(fn(FRBP))(fn(FRCP)));
        extended_op_0[30] = power_entry(power_op_fxcpnmsub, "fxcpnmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
        extended_op_0[31] = power_entry(power_op_fxcsnmsub, "fxcsnmsub", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
       extended_op_0[22] = power_entry(power_op_fxcpnmadd, "fxcpnmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
       extended_op_0[23] = power_entry(power_op_fxcsnmadd, "fxcsnmadd", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
       extended_op_0[27] = power_entry(power_op_fxcsmsub, "fxcsmsub", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
       extended_op_0[32] = power_entry(power_op_fpmr, "fpmr", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[96] = power_entry(power_op_fpabs, "fpabs", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[160] = power_entry(power_op_fpneg, "fpneg", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[192] = power_entry(power_op_fprsp, "fprsp", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[224] = power_entry(power_op_fpnabs, "fpnabs", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[288] = power_entry(power_op_fsmr, "fsmr", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[320] = power_entry(power_op_fscmp, "fscmp", NULL, list_of(fn(setFPMode))(fn(BF))(fn(FRAS))(fn(FRBS)));
       extended_op_0[352] = power_entry(power_op_fsabs, "fsabs", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[416] = power_entry(power_op_fsneg, "fsneg", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[480] = power_entry(power_op_fsnabs, "fsnabs", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRBS)));
       extended_op_0[544] = power_entry(power_op_fxmr, "fxmr", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[576] = power_entry(power_op_fpctiw, "fpctiw", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[704] = power_entry(power_op_fpctiwz, "fpctiwz", NULL, list_of(fn(setFPMode))(fn(FRTP))(fn(FRBP)));
       extended_op_0[800] = power_entry(power_op_fsmfp, "fpmr", NULL, list_of(fn(setFPMode))(fn(FRTS))(fn(FRB)));
       extended_op_0[846] = power_entry(power_op_qvfcfids, "qvfcfids", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_0[928] = power_entry(power_op_fsmtp, "fpmr", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRBS)));
       extended_op_0[974] = power_entry(power_op_qvfcfidus, "qvfcfidus", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
  
/* deleted in Version 3.0B
			 extended_op_4[0] = power_entry(power_op_qvfcmpeq, "qvfcmpeq", NULL,
  							 list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[1]   = power_entry(power_op_qvfxxmadd, "qvfxxmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
        extended_op_4[3] = power_entry(power_op_qvfxxcpnmadd, "qvfxxcpnmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
        extended_op_4[4] = power_entry(power_op_qvflogical, "qvflogical", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QTT)));
        extended_op_4[5] = power_entry(power_op_qvfaligni, "qvfaligni", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QVD)));
        extended_op_4[6] = power_entry(power_op_qvfperm, "qvfperm", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
        extended_op_4[8] = power_entry(power_op_qvfcpsgn, "qvfcpsgn", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
        extended_op_4[9] = power_entry(power_op_qvfxmadd, "qvfxmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[11] = power_entry(power_op_qvfxxnpmadd, "qvfxxnpmadd", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
       extended_op_4[12] = power_entry(power_op_qvfrsp, "qvfrsp", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       //extended_op_4[14] = power_entry(power_op_qvfctiw, "qvfctiw", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
			 
       extended_op_4[15] = power_entry(power_op_qvfctiwz, "qvfctiwz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
       extended_op_4[17] = power_entry(power_op_qvfxmul, "qvfxmul", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QFRCP)));
       extended_op_4[20] = power_entry(power_op_qvfsub, "qvfsub", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[21] = power_entry(power_op_qvfadd, "qvfadd", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
       extended_op_4[23] = power_entry(power_op_qvfsel, "qvfsel", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB))(fn(QFRCP)));
*/
			 
			 
			extended_op_4[0] = power_entry(power_op_vaddubm, "vaddubm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1] = power_entry(power_op_vmul10cuq, "vmul10cuq", NULL, list_of(fn(VRT))(fn(VRA)));
extended_op_4[2] = power_entry(power_op_vmaxub, "vmaxub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[4] = power_entry(power_op_vrlb, "vrlb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[6] = power_entry(power_op_vcmpequb, "vcmpequb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[7] = power_entry(power_op_vcmpneb, "vcmpneb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[10] = power_entry(power_op_vaddfp, "vaddfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[8] = power_entry(power_op_vmuloub, "vmuloub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[12] = power_entry(power_op_vmrghb, "vmrghb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));

extended_op_4[14] = power_entry(power_op_vpkuhum, "vpkuhum", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));

        extended_op_4[24] = power_entry(power_op_fxcpnpma, "fxcpnpma", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
        extended_op_4[25] = power_entry(power_op_fxcsnpma, "fxcsnpma", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
        extended_op_4[26] = power_entry(power_op_fxcpnsma, "fxcpnsma", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRA))(fn(FRBP))(fn(FRCP)));
        extended_op_4[28] = power_entry(power_op_fxcxnpma, "fxcxnpma", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
        extended_op_4[29] = power_entry(power_op_fxcxnsma, "fxcxnsma", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
        extended_op_4[30] = power_entry(power_op_fxcxma, "fxcxma", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
        extended_op_4[31] = power_entry(power_op_fxcxnms, "fxcxnms", NULL,
                                        list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));

extended_op_4[27] = power_entry(power_op_fxcsnsma, "fxcsnsma", NULL,
list_of(fn(setFPMode))(fn(FRTP))(fn(FRAS))(fn(FRBP))(fn(FRCP)));
extended_op_4[32] = power_entry(power_op_vmhaddshs, "vmhaddshs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[33] = power_entry(power_op_vmhraddshs, "vmhraddshs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[34] = power_entry(power_op_vmladduhm, "vmladduhm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[35] = power_entry(power_op_vmsumudm, "vmsumudm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[36] = power_entry(power_op_vmsumubm, "vmsumubm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[37] = power_entry(power_op_vmsummbm, "vmsummbm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[38] = power_entry(power_op_vmsumuhm, "vmsumuhm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[39] = power_entry(power_op_vmsumuhs, "vmsumuhs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[40] = power_entry(power_op_vmsumshm, "vmsumshm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[41] = power_entry(power_op_vmsumshs, "vmsumshs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[42] = power_entry(power_op_vsel, "vsel", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[43] = power_entry(power_op_vperm, "vperm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[44] = power_entry(power_op_vsldoi, "vsldoi", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(SHB)));
extended_op_4[45] = power_entry(power_op_vpermxor, "vpermxor", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[46] = power_entry(power_op_vmaddfp, "vmaddfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[47] = power_entry(power_op_vnmsubfp, "vnmsubfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[48] = power_entry(power_op_maddhd,"maddhd",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(RC)));
extended_op_4[49] = power_entry(power_op_maddhdu,"maddhdu",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(RC)));
extended_op_4[51] = power_entry(power_op_maddld,"maddld",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(RC)));
extended_op_4[59] = power_entry(power_op_vpermr, "vpermr", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[60] = power_entry(power_op_vaddeuqm, "vaddeuqm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[61] = power_entry(power_op_vaddecuq, "vaddecuq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[62] = power_entry(power_op_vsubeuqm, "vsubeuqm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[63] = power_entry(power_op_vsubecuq, "vsubecuq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(VRC)));
extended_op_4[64] = power_entry(power_op_vadduhm, "vadduhm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[65] = power_entry(power_op_vmul10ecuq, "vmul10ecuq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[66] = power_entry(power_op_vmaxuh, "vmaxuh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[68] = power_entry(power_op_vrlh, "vrlh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[70] = power_entry(power_op_vcmpequh, "vcmpequh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[71] = power_entry(power_op_vcmpneh, "vcmpneh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[72] = power_entry(power_op_vmulouh, "vmulouh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[74] = power_entry(power_op_vsubfp, "vsubfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[76] = power_entry(power_op_vmrghh, "vmrghh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[78] = power_entry(power_op_vpkuwum, "vpkuwum", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[96] = power_entry(power_op_qvfcmplt, "qvfcmplt", NULL,
                                       list_of(fn(setFPMode))(fn(QFRTP))(fn(QFRA))(fn(QRB)));
extended_op_4[128] = power_entry(power_op_vadduwm, "vadduwm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[130] = power_entry(power_op_vmaxuw, "vmaxuw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[132] = power_entry(power_op_vrlw, "vrlw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[133] = power_entry(power_op_vrlwmi, "vrlwmi", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[134] = power_entry(power_op_vcmpequw, "vcmpequw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[135] = power_entry(power_op_vcmpnew, "vcmpnew", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[136] = power_entry(power_op_vmulouw, "vmulouw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[137] = power_entry(power_op_vmuluwm, "vmuluwm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[140] = power_entry(power_op_vmrghw, "vmrghw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[142] = power_entry(power_op_vpkuhus, "vpkuhus", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[143] = power_entry(power_op_qvfctiwuz, "qvfctiwuz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[192] = power_entry(power_op_vaddudm, "vaddudm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[194] = power_entry(power_op_vmaxud, "vmaxud", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[196] = power_entry(power_op_vrld, "vrld", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[197] = power_entry(power_op_vrldmi, "vrldmi", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[198] = power_entry(power_op_vcmpeqfp, "vcmpeqfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[199] = power_entry(power_op_vcmpequd, "vcmpequd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[206] = power_entry(power_op_vpkuwus, "vpkuwus", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[256] = power_entry(power_op_vadduqm, "vadduqm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[258] = power_entry(power_op_vmaxsb, "vmaxsb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[260] = power_entry(power_op_vslb, "vslb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[263] = power_entry(power_op_vcmpnezb, "vcmpnezb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[264] = power_entry(power_op_vmulosb, "vmulosb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[266] = power_entry(power_op_vrefp, "vrefp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[268] = power_entry(power_op_vmrglb, "vmrglb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[270] = power_entry(power_op_vpkshus, "vpkshus", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[320] = power_entry(power_op_vaddcuq, "vaddcuq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[322] = power_entry(power_op_vmaxsh, "vmaxsh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[324] = power_entry(power_op_vslh, "vslh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[327] = power_entry(power_op_vcmpnezh, "vcmpnezh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[328] = power_entry(power_op_vmulosh, "vmulosh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[330] = power_entry(power_op_vrsqrtefp, "vrsqrtefp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[332] = power_entry(power_op_vmrglh, "vmrglh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[334] = power_entry(power_op_vpkswus, "vpkswus", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[384] = power_entry(power_op_vaddcuw, "vaddcuw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[386] = power_entry(power_op_vmaxsw, "vmaxsw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[388] = power_entry(power_op_vslw, "vslw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[389] = power_entry(power_op_vrlwnm, "vrlwnm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[391] = power_entry(power_op_vcmpnezw, "vcmpnezw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[392] = power_entry(power_op_vmulosw, "vmulosw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[394] = power_entry(power_op_vexptefp, "vexptefp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[396] = power_entry(power_op_vmrglw, "vmrglw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[398] = power_entry(power_op_vpkshss, "vpkshss", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[424] = power_entry(power_op_qvfriz, "qvfriz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[450] = power_entry(power_op_vmaxsd, "vmaxsd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[452] = power_entry(power_op_vsl, "vsl", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[453] = power_entry(power_op_vrldnm, "vrldnm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[454] = power_entry(power_op_vcmpgefp, "vcmpgefp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[456] = power_entry(power_op_qvfrip, "qvfrip", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[458] = power_entry(power_op_vlogefp, "vlogefp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[462] = power_entry(power_op_vpkswss, "vpkswss", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[512] = power_entry(power_op_vaddubs, "vaddubs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[513] = power_entry(power_op_vmul10uq, "vmul10uq", NULL, list_of(fn(VRT))(fn(VRA)));
extended_op_4[514] = power_entry(power_op_vminub, "vminub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[516] = power_entry(power_op_vsrb, "vsrb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[518] = power_entry(power_op_vcmpgtub, "vcmpgtub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[520] = power_entry(power_op_vmuleub, "vmuleub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[524] = power_entry(power_op_vspltb, "vspltb", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[525] = power_entry(power_op_vextractub, "vextractub", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[526] = power_entry(power_op_vupkhsb, "vupkhsb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[552] = power_entry(power_op_vrfin, "vrfin", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[576] = power_entry(power_op_vadduhs, "vadduhs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[577] = power_entry(power_op_vmul10euq, "vmul10euq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[578] = power_entry(power_op_vminuh, "vminuh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[580] = power_entry(power_op_vsrh, "vsrh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[582] = power_entry(power_op_vcmpgtuh, "vcmpgtuh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[584] = power_entry(power_op_vmuleuh, "vmuleuh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[586] = power_entry(power_op_vrfiz, "vrfiz", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[588] = power_entry(power_op_vsplth, "vsplth", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[589] = power_entry(power_op_vextractuh, "vextractuh", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[590] = power_entry(power_op_vupkhsh, "vupkhsh", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[642] = power_entry(power_op_vminuw, "vminuw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[640] = power_entry(power_op_vadduws, "vadduws", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[644] = power_entry(power_op_vsrw, "vsrw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[646] = power_entry(power_op_vcmpgtuw, "vcmpgtuw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[648] = power_entry(power_op_vmuleuw, "vmuleuw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[650] = power_entry(power_op_vrfip, "vrfip", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[652] = power_entry(power_op_vspltw, "vspltw", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[653] = power_entry(power_op_vextractuw, "vextractuw", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[654] = power_entry(power_op_vupklsb, "vupklsb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[706] = power_entry(power_op_vminud, "vminud", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[708] = power_entry(power_op_vsr, "vsr", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[710] = power_entry(power_op_vcmpgtfp, "vcmpgtfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[711] = power_entry(power_op_vcmpgtud, "vcmpgtud", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[714] = power_entry(power_op_vrfim, "vrfim", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[717] = power_entry(power_op_vextractd, "vextractd", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[718] = power_entry(power_op_vupklsh, "vupklsh", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[768] = power_entry(power_op_vaddsbs, "vaddsbs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[770] = power_entry(power_op_vminsb, "vminsb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[772] = power_entry(power_op_vsrab, "vsrab", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[774] = power_entry(power_op_vcmpgtsb, "vcmpgtsb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[776] = power_entry(power_op_vmulesb, "vmulesb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[778] = power_entry(power_op_vcfsx, "vcfsx", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[780] = power_entry(power_op_vspltisb, "vspltisb", NULL, list_of(fn(VRT))(fn(SIM)));
extended_op_4[781] = power_entry(power_op_vinsertb, "vinsertb", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[782] = power_entry(power_op_vpkpx, "vpkpx", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[814] = power_entry(power_op_qvfctid, "qvfctid", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[815] = power_entry(power_op_qvfctidz, "qvfctidz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[832] = power_entry(power_op_vaddshs, "vaddshs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[833] = power_entry(power_op_bcdcpsgn, "bcdcpsgn", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[834] = power_entry(power_op_vminsh, "vminsh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[836] = power_entry(power_op_vsraw, "vsraw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[838] = power_entry(power_op_vcmpgtsh, "vcmpgtsh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[840] = power_entry(power_op_vmulesh, "vmulesh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[842] = power_entry(power_op_vcfux, "vcfux", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[844] = power_entry(power_op_vspltish, "vspltish", NULL, list_of(fn(VRT))(fn(SIM)));
extended_op_4[845] = power_entry(power_op_vinserth, "vinserth", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[846] = power_entry(power_op_vupkhpx, "vupkhpx", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[896] = power_entry(power_op_vaddsws, "vaddsws", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[898] = power_entry(power_op_vminsw, "vminsw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[900] = power_entry(power_op_vsraw, "vsraw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[902] = power_entry(power_op_vcmpgtsw, "vcmpgtsw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[904] = power_entry(power_op_vmulesw, "vmulesw", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[906] = power_entry(power_op_vctuxs, "vctuxs", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[908] = power_entry(power_op_vspltisw, "vspltisw", NULL, list_of(fn(VRT))(fn(SIM)));
extended_op_4[909] = power_entry(power_op_vinsertw, "vinsertw", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[942] = power_entry(power_op_qvfctidu, "qvfctidu", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[943] = power_entry(power_op_qvfctiduz, "qvfctiduz", NULL, list_of(fn(setFPMode))(fn(QFRTP))(fn(QRB)));
extended_op_4[962] = power_entry(power_op_vminsd, "vminsd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[964] = power_entry(power_op_vsrad, "vsrad", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[966] = power_entry(power_op_vcmpbfp, "vcmpbfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[967] = power_entry(power_op_vcmpgtsd, "vcmpgtsd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[970] = power_entry(power_op_vctsxs, "vctsxs", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[973] = power_entry(power_op_vinsertd, "vinsertd", NULL, list_of(fn(VRT))(fn(UIM))(fn(VRB)));
extended_op_4[974] = power_entry(power_op_vupklpx,  "vupklpx", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1024] = power_entry(power_op_vsububm, "vsububm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1025] =	power_entry(power_op_bcdadd, "bcdadd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1026] = power_entry(power_op_vavgub, "vavgub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1027] = power_entry(power_op_vabsdub, "vabsdub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1028] = power_entry(power_op_vand, "vand", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1030] = power_entry(power_op_vcmpequb, "vcmpequb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1031] = power_entry(power_op_vcmpneb, "vcmpneb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1032] = power_entry(power_op_vpmsumb, "vpmsumb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1034] = power_entry(power_op_vmaxfp, "vmaxfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1036] = power_entry(power_op_vslo, "vslo", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1088] = power_entry(power_op_vsubuhm, "vsubuhm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1089] =	power_entry(power_op_bcdsub, "bcdsub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1090] = power_entry(power_op_vavguh, "vavguh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1091] = power_entry(power_op_vabsduh, "vabsduh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1092] = power_entry(power_op_vandc, "vandc", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1094] = power_entry(power_op_vcmpequh, "vcmpequh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1095] = power_entry(power_op_vcmpneh, "vcmpneh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1096] = power_entry(power_op_vpmsumh, "vpmsumh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1098] = power_entry(power_op_vminfp, "vminfp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1100] = power_entry(power_op_vsro, "vsro", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1102] = power_entry(power_op_vpkudum, "vpkudum", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1152] = power_entry(power_op_vsubuwm, "vsubuwm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1153] = power_entry(power_op_vsubudm, "vsubudm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1154] = power_entry(power_op_vavguw, "vavguw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1155] = power_entry(power_op_vabsduw, "vabsduw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1156] = power_entry(power_op_vor, "vor", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1158] = power_entry(power_op_vcmpequw, "vcmpequw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1159] = power_entry(power_op_vcmpnew, "vcmpnew", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1160] = power_entry(power_op_vpmsumw, "vpmsumw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1216] = power_entry(power_op_vsubudm, "vsubudm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1217] = power_entry(power_op_bcds, "bcds", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1220] = power_entry(power_op_vxor, "vxor", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1223] = power_entry(power_op_vcmpequh, "vcmpequh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1224] = power_entry(power_op_vpmsumd, "vpmsumd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1230] = power_entry(power_op_vpkudus, "vpkudus", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1280] = power_entry(power_op_vsubuqm, "vsubuqm", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1281] = power_entry(power_op_bcdtrunc, "bcdtrunc", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1282] = power_entry(power_op_vavgsb, "vavgsb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1284] = power_entry(power_op_vnor, "vnor", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1287] = power_entry(power_op_vcmpnezb, "vcmpnezb", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1288] = power_entry(power_op_vcipher, "vcipher", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1289] = power_entry(power_op_vcipherlast, "vcipherlast", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1292] = power_entry(power_op_vgbbd, "vgbbd", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1344] = power_entry(power_op_vsubcuq, "vsubcuq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1345] = power_entry(power_op_bcdutrunc, "bcdutrunc", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1346] = power_entry(power_op_vavgsh, "vavgsh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1348] = power_entry(power_op_vorc, "vorc", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1351] = power_entry(power_op_vcmpnezh, "vcmpnezh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1352] = power_entry(power_op_vncipher, "vncipher", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1353] = power_entry(power_op_vncipherlast, "vncipherlast", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1356] = power_entry(power_op_vbpermq, "vbpermq", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));	
extended_op_4[1358] = power_entry(power_op_vpksdus, "vpksdus", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1408] = power_entry(power_op_vsubcuw, "vsubcuw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));

//extended_op_4[1409] = power_entry(power_op_extended, "extended", fn(extended_op_4_1409), operandSpec());
extended_op_4_1409[0] = power_entry(power_op_bcdctsq, "bcdctsq", NULL, list_of(fn(VRT))(fn(VRB))(fn(PS)));
extended_op_4_1409[2] = power_entry(power_op_bcdcfsq, "bcdcfsq", NULL, list_of(fn(VRT))(fn(VRB))(fn(PS)));
extended_op_4_1409[4] = power_entry(power_op_bcdctz, "bcdctz", NULL, list_of(fn(VRT))(fn(VRB))(fn(PS)));
extended_op_4_1409[5] = power_entry(power_op_bcdctn, "bcdctn", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1409[6] = power_entry(power_op_bcdcfz, "bcdcfz", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1409[7] = power_entry(power_op_bcdcfn, "bcdcfn", NULL, list_of(fn(VRT))(fn(VRB))(fn(PS)));
extended_op_4_1409[31] = power_entry(power_op_bcdsetsgn, "bcdsetsgn", NULL, list_of(fn(VRT))(fn(VRB))(fn(PS)));


//


extended_op_4[1410] = power_entry(power_op_vavgsw, "vavgsw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1412] = power_entry(power_op_vnand, "vnand", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1415] = power_entry(power_op_vcmpnezw, "vcmpnezw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1473] = power_entry(power_op_bcdsr, "bcdsr", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1476] = power_entry(power_op_vsld, "vsld", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1480] = power_entry(power_op_vsubcuq, "vsubcuq", NULL, list_of(fn(VRT))(fn(VRA)));	
extended_op_4[1484] = power_entry(power_op_vbpermd, "vbpermd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));	
extended_op_4[1486] = power_entry(power_op_vpksdss, "vpksdss", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1536] =	power_entry(power_op_vsububs, "vsububs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1537] =	power_entry(power_op_bcdadd, "bcdadd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));


//extended_op_4[1538] = power_entry(power_op_extended, "extended", fn(extended_op_4_1538), operandSpec()));
extended_op_4_1538[0] = power_entry(power_op_vclzlsbb, "vclzlsbb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[1] = power_entry(power_op_vctzlsbb, "vctzlsbb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[6] = power_entry(power_op_vnegw, "vnegw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[7] = power_entry(power_op_vnegd, "vnegd", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[8] = power_entry(power_op_vprtybw, "vprtybw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[9] = power_entry(power_op_vprtybd, "vprtybd", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[10] = power_entry(power_op_vprtybq, "vprtybq", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[16] = power_entry(power_op_vextsb2w, "vextsb2w", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[17] = power_entry(power_op_vextsh2w, "vextsh2w", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[24] = power_entry(power_op_vextsb2d, "vextsb2d", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[25] = power_entry(power_op_vextsh2d, "vextsh2d", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[26] = power_entry(power_op_vextsw2d, "vextsw2d", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[28] = power_entry(power_op_vctzb, "vctzb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[29] = power_entry(power_op_vctzh, "vctzh", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[30] = power_entry(power_op_vctzw, "vctzw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4_1538[31] = power_entry(power_op_vctzd, "vctzd", NULL, list_of(fn(VRT))(fn(VRB)));

extended_op_4[1540] = power_entry(power_op_mfvscr, "mfvscr", NULL, list_of((fn(VRT))));
extended_op_4[1542] = power_entry(power_op_vcmpgtub, "vcmpgtub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1544] = power_entry(power_op_vsum4ubs, "vsum4ubs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1549] = power_entry(power_op_vextublx, "vextublx", NULL, list_of(fn(RT))(fn(RA))(fn(VRB)));
extended_op_4[1600] = power_entry(power_op_vsubuhs, "vsubuhs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1601] =	power_entry(power_op_bcdsub, "bcdsub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1604] = power_entry(power_op_mtvscr, "mtvscr", NULL, list_of((fn(VRB))));
extended_op_4[1606] = power_entry(power_op_vcmpgtuh, "vcmpgtuh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1608] = power_entry(power_op_vsum4shs, "vsum4shs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1613] = power_entry(power_op_vextuhlx, "vextuhlx", NULL, list_of(fn(RT))(fn(RA))(fn(VRB)));
extended_op_4[1614] = power_entry(power_op_vupkhsw, "vupkhsw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1664] = power_entry(power_op_vsubuws, "vsubuws", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
//extended_op_4[1666] = power_entry(power_op_vshasigmaw, "vshasigmaw", NULL, list_of(fn(VRT))(fn(VRA))(fn(ST))(fn(SIX)));
extended_op_4[1668] = power_entry(power_op_veqv, "veqv", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1670] = power_entry(power_op_vcmpgtuw, "vcmpgtuw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1672] = power_entry(power_op_vsum2sws, "vsum2sws", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1676] = power_entry(power_op_vmrgow, "vmrgow", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1677] = power_entry(power_op_vextuwlx, "vextuwlx", NULL, list_of(fn(RT))(fn(RA))(fn(VRB)));
extended_op_4[1729] = power_entry(power_op_bcds, "bcds", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
//extended_op_4[1730] = power_entry(power_op_vshasigmad, "vshasigmad", NULL, list_of(fn(VRT))(fn(VRA))(fn(ST))(fn(SIX)));
extended_op_4[1732] = power_entry(power_op_vsrd, "vsrd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1735] = power_entry(power_op_vcmpgtub, "vcmpgtub", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1742] = power_entry(power_op_vupklsw, "vupklsw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1792] = power_entry(power_op_vsubsbs, "vsubsbs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1793] = power_entry(power_op_bcdtrunc, "bcdtrunc", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1794] = power_entry(power_op_vclzb, "vclzb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1795] = power_entry(power_op_vpopcntb, "vpopcntb", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1796] = power_entry(power_op_vsrv, "vsrv", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1798] = power_entry(power_op_vcmpequh, "vcmpequh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1800] = power_entry(power_op_vsum4sbs, "vsum4sbs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1805] = power_entry(power_op_vextubrx, "vextubrx", NULL, list_of(fn(RT))(fn(RA))(fn(VRB)));
extended_op_4[1856] = power_entry(power_op_vsubshs, "vsubshs", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1858] = power_entry(power_op_vclzh, "vclzh", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1859] = power_entry(power_op_vpopcnth, "vpopcnth", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1860] = power_entry(power_op_vslv, "vslv", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1862] = power_entry(power_op_vcmpgtsh, "vcmpgtsh", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1869] = power_entry(power_op_vextuhrx, "vextuhrx", NULL, list_of(fn(RT))(fn(RA))(fn(VRB)));
extended_op_4[1920] = power_entry(power_op_vsubsws, "vsubsws", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));

//4-1921 shares the same third level opcode table with 4-1409
//extended_op_4[1921] = power_entry(power_op_extended, "extended", fn(extended_op_4_1409), operandSpec()));

extended_op_4[1922] = power_entry(power_op_vclzw, "vclzw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1923] = power_entry(power_op_vpopcntw, "vpopcntw", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1928] = power_entry(power_op_vsumsws, "vsumsws", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1932] = power_entry(power_op_vmrgew, "vmrgew", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_4[1933] = power_entry(power_op_vextuwrx, "vextuwrx", NULL, list_of(fn(RT))(fn(RA))(fn(VRB)));
extended_op_4[1956] = power_entry(power_op_vcmpgtsw, "vcmpgtsw", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));
extended_op_4[1985] = power_entry(power_op_bcdsr, "bcdsr", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(PS)));
extended_op_4[1986] = power_entry(power_op_vclzd, "vclzd", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1987] = power_entry(power_op_vpopcntd, "vpopcntd", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_4[1991] = power_entry(power_op_vcmpgtsd, "vcmpgtsd", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(Rc)));



			extended_op_19[0] = power_entry(power_op_mcrf, "mcrf", NULL, list_of(fn(BF))(fn(BFA)));
extended_op_19[16] = power_entry(power_op_bclr, "bclr", NULL, list_of(fn(BO))(fn(BI))(fn(LK)));
extended_op_19[18] = power_entry(power_op_rfid, "rfid", NULL, operandSpec());
extended_op_19[33] = power_entry(power_op_crnor, "crnor", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[50] = power_entry(power_op_rfi, "rfi", NULL, operandSpec());
extended_op_19[82] = power_entry(power_op_rfsvc, "rfsvc", NULL, list_of(fn(LK)));
extended_op_19[129] = power_entry(power_op_crandc, "crandc", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[150] = power_entry(power_op_isync, "isync", NULL, operandSpec());
extended_op_19[193] = power_entry(power_op_crxor, "crxor", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[225] = power_entry(power_op_crnand, "crnand", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[257] = power_entry(power_op_crand, "crand", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[289] = power_entry(power_op_creqv, "creqv", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[417] = power_entry(power_op_crorc, "crorc", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[449] = power_entry(power_op_cror, "cror", NULL, list_of(fn(BT))(fn(BA))(fn(BB)));
extended_op_19[528] = power_entry(power_op_bcctr, "bcctr", NULL, list_of(fn(BO))(fn(BI))(fn(LK)));

        power_entry::extended_op_30[1] = power_entry(power_op_rldicr, "rldicr", NULL,
                                                     list_of(fn(RA))(fn(RS))(fn(SH))(fn(ME))(fn(Rc)));
        extended_op_30[2] = power_entry(power_op_rldic, "rldic", NULL, list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(Rc)));
        extended_op_30[3] = power_entry(power_op_rldimi, "rldimi", NULL,
                                        list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(Rc)));
        extended_op_30[8] = power_entry(power_op_rldcl, "rldcl", NULL, list_of(fn(RA))(fn(RS))(fn(RB))(fn(MB))(fn(Rc)));
        extended_op_30[9] = power_entry(power_op_rldcr, "rldcr", NULL, list_of(fn(RA))(fn(RS))(fn(RB))(fn(ME))(fn(Rc)));
        extended_op_30[0] = power_entry(power_op_rldicl, "rldicl", NULL,
                                        list_of(fn(RA))(fn(RS))(fn(SH))(fn(MB))(fn(Rc)));

    power_entry::extended_op_31[0] = power_entry(power_op_cmp, "cmp", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(RB)));
extended_op_31[4] = power_entry(power_op_tw, "tw", NULL, list_of(fn(TO))(fn(RA))(fn(RB)));
extended_op_31[6] = power_entry(power_op_lvsl, "lvsl", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[7] = power_entry(power_op_lvebx, "lvebx", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[8] = power_entry(power_op_subfc, "subfc", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[9] = power_entry(power_op_mulhdu, "mulhdu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[10] = power_entry(power_op_addc, "addc", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[11] = power_entry(power_op_mulhwu, "mulhwu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[12] = power_entry(power_op_lxsiwzx, "lxsiwzx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[15] = power_entry(power_op_isel,"isel",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(BC)));
extended_op_31[19] = power_entry(power_op_mfcr, "mfcr", NULL, list_of(fn(RT))(fn(Rc)));
extended_op_31[19] = power_entry(power_op_mfocrf,"mfocrf",NULL,list_of(fn(RT))(fn(FXM)));
extended_op_31[20] = power_entry(power_op_lwarx, "lwarx", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[21] = power_entry(power_op_ldx, "ldx", NULL, list_of(fn(RT))(fn(LX<u64>)));
extended_op_31[22] = power_entry(power_op_icbt,"icbt",NULL,list_of(fn(CT))(fn(RA))(fn(RB)));
extended_op_31[23] = power_entry(power_op_lwzx, "lwzx", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[24] = power_entry(power_op_slw, "slw", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[26] = power_entry(power_op_cntlzw, "cntlzw", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[27] = power_entry(power_op_sld, "sld", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[28] = power_entry(power_op_and, "and", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[29] = power_entry(power_op_maskg, "maskg", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[30] = power_entry(power_op_wait,"wait",NULL,list_of(fn(WC)));
extended_op_31[32] = power_entry(power_op_cmpl, "cmpl", NULL, list_of(fn(BF))(fn(L))(fn(RA))(fn(RB)));
extended_op_31[38] = power_entry(power_op_lvsr, "lvsr", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[39] = power_entry(power_op_lvehx, "lvehx", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[40] = power_entry(power_op_subf, "subf", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[51] = power_entry(power_op_mfvsrd,"mfvsrd",NULL,list_of(fn(XS))(fn(RA)));
extended_op_31[52] = power_entry(power_op_lbarx,"lbarx",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(EH)));
extended_op_31[53] = power_entry(power_op_ldux, "ldux", NULL, list_of(fn(RT))(fn(LUX<u64>)));
extended_op_31[54] = power_entry(power_op_dcbst, "dcbst", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[55] = power_entry(power_op_lwzux, "lwzux", NULL, list_of(fn(RT))(fn(LUX<u32>)));
extended_op_31[58] = power_entry(power_op_cntlzd, "cntlzd", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[60] = power_entry(power_op_andc, "andc", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
//extended_op_31[62] = power_entry(power_op_wait, "wait", NULL, list_of(fn(WC)));
extended_op_31[68] = power_entry(power_op_td, "td", NULL, list_of(fn(TO))(fn(RA))(fn(RB)));
extended_op_31[70] = power_entry(power_op_qvlpcrdx, "qvlpcrdx", NULL, list_of(fn(QFRTP))(fn(LX<dbl128>)));
extended_op_31[71] = power_entry(power_op_lvewx, "lvewx", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[73] = power_entry(power_op_mulhd, "mulhd", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[74] = power_entry(power_op_addg6s,"addg6s",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[75] = power_entry(power_op_mulhw, "mulhw", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[76] = power_entry(power_op_lxsiwax,"lxsiwax",NULL,list_of(fn(XT))(fn(RA))(fn(RB)));
//extended_op_31[78] = power_entry(power_op_lxsiwax, "lxsiwax", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[83] = power_entry(power_op_mfmsr, "mfmsr", NULL, list_of(fn(RT)));
extended_op_31[84] = power_entry(power_op_ldarx, "ldarx", NULL, list_of(fn(RT))(fn(LX<u64>)));
extended_op_31[86] = power_entry(power_op_dcbf, "dcbf", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[87] = power_entry(power_op_lbzx, "lbzx", NULL, list_of(fn(RT))(fn(LX<u8>)));
extended_op_31[103] = power_entry(power_op_lvx, "lvx", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[104] = power_entry(power_op_neg, "neg", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[107] = power_entry(power_op_mul, "mul", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[115] = power_entry(power_op_mfvsrwz,"mfvsrwz",NULL,list_of(fn(XS))(fn(RA)));
extended_op_31[116] = power_entry(power_op_lharx,"lharx",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(EH)));
extended_op_31[118] = power_entry(power_op_clf, "clf", NULL, list_of(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[119] = power_entry(power_op_lbzux, "lbzux", NULL, list_of(fn(RT))(fn(LUX<u8>)));
extended_op_31[122] = power_entry(power_op_popcntb, "popcntb", NULL, list_of(fn(RS))(fn(RA)));extended_op_31[124] = power_entry(power_op_nor, "nor", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[128] = power_entry(power_op_setb,"setb",NULL,list_of(fn(RT))(fn(BFA)));
extended_op_31[133] = power_entry(power_op_qvstfcsxi, "qvstfcsxi", NULL,  list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[135] = power_entry(power_op_stvebx, "stvebx", NULL, list_of(fn(VRS))(fn(RA))(fn(RB)));
extended_op_31[136] = power_entry(power_op_subfe, "subfe", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[138] = power_entry(power_op_adde, "adde", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[140] = power_entry(power_op_stxsiwx, "stxsiwx", NULL, list_of(fn(VRS))(fn(RA))(fn(RB)));
extended_op_31[142] = power_entry(power_op_msgsndp,"msgsndp",NULL,list_of(fn(RB)));
extended_op_31[144] = power_entry(power_op_mtcrf, "mtcrf", NULL, list_of(fn(RS))(fn(FXM))(fn(Rc)));
extended_op_31[146] = power_entry(power_op_mtmsr,"mtmsr",NULL,list_of(fn(RS))(fn(L)));
extended_op_31[149] = power_entry(power_op_stdx, "stdx", NULL, list_of(fn(RS))(fn(STX<u64>)));
extended_op_31[150] = power_entry(power_op_stwcx_rc, "stwcx.", NULL, list_of(fn(RS))(fn(STX<u32>)));
extended_op_31[151] = power_entry(power_op_stwx, "stwx", NULL, list_of(fn(RS))(fn(STX<u32>)));
extended_op_31[152] = power_entry(power_op_slq, "slq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[153] = power_entry(power_op_sle, "sle", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[154] = power_entry(power_op_prtyw,"prtyw",NULL,list_of(fn(RS))(fn(RA)));
extended_op_31[165] = power_entry(power_op_qvstfcsuxi, "qvstfcsuxi", NULL, list_of(fn(QFRSP))(fn(STUX<sp_float>)));
extended_op_31[167] = power_entry(power_op_stvehx, "stvehx", NULL, list_of(fn(VRS))(fn(RA))(fn(RB)));
extended_op_31[170] = power_entry(power_op_addex, "addex", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(CY)));
extended_op_31[174] = power_entry(power_op_msgclrp,"msgclrp",NULL,list_of(fn(RB)));
extended_op_31[178] = power_entry(power_op_mtmsrd,"mtmsrd",NULL,list_of(fn(RS))(fn(L)));
extended_op_31[179] = power_entry(power_op_mtvsrd,"mtvsrd",NULL,list_of(fn(XT))(fn(RA)));
extended_op_31[181] = power_entry(power_op_stdux, "stdux", NULL, list_of(fn(RS))(fn(STUX<u64>)));
extended_op_31[182] = power_entry(power_op_stqcx,"stqcx",NULL,list_of(fn(RSP))(fn(RA))(fn(RB)));
extended_op_31[183] = power_entry(power_op_stwux, "stwux", NULL, list_of(fn(RS))(fn(STUX<u32>)));
extended_op_31[184] = power_entry(power_op_sliq, "sliq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[186] = power_entry(power_op_prtyd,"prtyd",NULL,list_of(fn(RS))(fn(RA)));
extended_op_31[192] = power_entry(power_op_cmprb,"cmprb",NULL,list_of(fn(BF))(fn(L))(fn(RA))(fn(RB)));
extended_op_31[197] = power_entry(power_op_qvstfcdxi, "qvstfcdxi", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[199] = power_entry(power_op_stvewx, "stvewx", NULL, list_of(fn(VRS))(fn(RA))(fn(RB)));
extended_op_31[200] = power_entry(power_op_subfze,"subfze",NULL,list_of(fn(RT))(fn(RA))(fn(OE))(fn(RC)));
extended_op_31[202] = power_entry(power_op_addze, "addze", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[206] = power_entry(power_op_msgsnd,"msgsnd",NULL,list_of(fn(RB)));
extended_op_31[211] = power_entry(power_op_mtvsrwa,"mtvsrwa",NULL,list_of(fn(XT))(fn(RA)));
extended_op_31[214] = power_entry(power_op_stdcx_rc, "stdcx.", NULL, list_of(fn(RS))(fn(STX<u64>)));
extended_op_31[215] = power_entry(power_op_stbx, "stbx", NULL, list_of(fn(RS))(fn(STX<u8>)));
extended_op_31[216] = power_entry(power_op_sllq, "sllq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[217] = power_entry(power_op_sleq, "sleq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[224] = power_entry(power_op_cmpeqb,"cmpeqb",NULL,list_of(fn(BF))(fn(RA))(fn(RB)));
extended_op_31[229] = power_entry(power_op_qvstfcduxi, "qvstfcduxi", NULL, list_of(fn(QFRSP))(fn(STUX<dbl128>)));
extended_op_31[231] = power_entry(power_op_stvx, "stvx", NULL, list_of(fn(VRS))(fn(RA))(fn(RB)));
extended_op_31[232] = power_entry(power_op_subfme, "subfme", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[233] = power_entry(power_op_mulld, "mulld", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[234] = power_entry(power_op_addme, "addme", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[235] = power_entry(power_op_mullw, "mullw", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[238] = power_entry(power_op_msgclr,"msgclr",NULL,list_of(fn(RB)));
extended_op_31[243] = power_entry(power_op_mtvsrwz,"mtvsrwz",NULL,list_of(fn(XT))(fn(RA)));
extended_op_31[246] = power_entry(power_op_dcbtst, "dcbtst", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[247] = power_entry(power_op_stbux, "stbux", NULL, list_of(fn(RS))(fn(STUX<u8>)));
extended_op_31[248] = power_entry(power_op_slliq, "slliq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[252] = power_entry(power_op_bpermd,"bpermd",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[264] = power_entry(power_op_doz, "doz", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[265] = power_entry(power_op_modud,"modud",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[266] = power_entry(power_op_add, "add", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[267] = power_entry(power_op_moduw,"moduw",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[268] = power_entry(power_op_lxvx, "lxvx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[269] = power_entry(power_op_lxvl, "lxvl", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[270] = power_entry(power_op_lfxsx, "lfxsx", NULL, list_of(fn(FRTP))(fn(LX<dp_float>)));
extended_op_31[274] = power_entry(power_op_tlbiel,"tlbiel",NULL,list_of(fn(RS))(fn(RIC))(fn(PRS))(fn(R))(fn(RB)));
extended_op_31[276] = power_entry(power_op_lqarx,"lqarx",NULL,list_of(fn(RTP))(fn(RA))(fn(RB))(fn(EH)));
extended_op_31[277] = power_entry(power_op_lscbx, "lscbx", NULL, list_of(fn(RT))(fn(LX<u8>))(fn(Rc)));
extended_op_31[278] = power_entry(power_op_dcbt, "dcbt", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[279] = power_entry(power_op_lhzx, "lhzx", NULL, list_of(fn(RT))(fn(LX<u16>)));
extended_op_31[282] = power_entry(power_op_cdtbcd,"cdtbcd",NULL,list_of(fn(RS))(fn(RA)));
extended_op_31[284] = power_entry(power_op_eqv, "eqv", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[300] = power_entry(power_op_lxvx, "lxvx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[301] = power_entry(power_op_lxvll, "lxvll", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[302] = power_entry(power_op_mfbhrbe,"mfbhrbe",NULL,list_of(fn(RT))(fn(BHRBE)));
extended_op_31[306] = power_entry(power_op_tlbie, "tlbie", NULL, list_of(fn(RB)));
extended_op_31[307] = power_entry(power_op_mfvsrld,"mfvsrld",NULL,list_of(fn(XS))(fn(RA)));
extended_op_31[310] = power_entry(power_op_eciwx, "eciwx", NULL, list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[314] = power_entry(power_op_cbcdtd,"cbcdtd",NULL,list_of(fn(RS))(fn(RA)));
extended_op_31[316] = power_entry(power_op_xor, "xor", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[311] = power_entry(power_op_lhzux, "lhzux", NULL, list_of(fn(RT))(fn(LUX<u16>)));
extended_op_31[332] = power_entry(power_op_lxvdsx, "lxvdsx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[334] = power_entry(power_op_lfxdx, "lfxdx", NULL, list_of(fn(FRTP))(fn(LX<dbl128>)));
extended_op_31[338] = power_entry(power_op_slbsync,"slbsync",NULL,operandSpec());
extended_op_31[339] = power_entry(power_op_mfspr, "mfspr", NULL, list_of(fn(RT))(fn(spr))(fn(Rc)));
extended_op_31[341] = power_entry(power_op_lwax, "lwax", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[343] = power_entry(power_op_lhax, "lhax", NULL, list_of(fn(RT))(fn(LX<u16>)));
extended_op_31[359] = power_entry(power_op_lvxl, "lvxl", NULL, list_of(fn(VRT))(fn(RA))(fn(RB)));
extended_op_31[360] = power_entry(power_op_abs, "abs", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[363] = power_entry(power_op_divs, "divs", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[364] = power_entry(power_op_lxvwsx, "lxvwsx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[366] = power_entry(power_op_lfxdux, "lfxdux", NULL, list_of(fn(FRTP))(fn(LUX<dbl128>)));
// xop 371 is mftb (Move from time base). It is phased-out and equivalent to mfspr Rx, 268
extended_op_31[371] = power_entry(power_op_mfspr, "mfspr", NULL, list_of(fn(RT))(fn(spr))(fn(Rc)));extended_op_31[373] = power_entry(power_op_lwaux, "lwaux", NULL, list_of(fn(RT))(fn(LUX<u32>)));
extended_op_31[375] = power_entry(power_op_lhaux, "lhaux", NULL, list_of(fn(RT))(fn(LUX<u16>)));
extended_op_31[378] = power_entry(power_op_popcntw, "popcntw", NULL, list_of(fn(RS))(fn(RA)));
extended_op_31[393] = power_entry(power_op_divdeu,"divdeu",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[395] = power_entry(power_op_divweu,"divweu",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(RC)));
extended_op_31[396] = power_entry(power_op_stxvx, "stxvx", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[397] = power_entry(power_op_stxvl, "stxvl", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[398] = power_entry(power_op_lfpsx, "lfpsx", NULL, list_of(fn(FRTP))(fn(LX<sp_float>)));
extended_op_31[402] = power_entry(power_op_slbmte,"slbmte",NULL,list_of(fn(RS))(fn(RB)));
extended_op_31[403] = power_entry(power_op_mtvsrws,"mtvsrws",NULL,list_of(fn(XT))(fn(RA)));
extended_op_31[407] = power_entry(power_op_sthx, "sthx", NULL, list_of(fn(RS))(fn(STX<u16>)));
extended_op_31[412] = power_entry(power_op_orc, "orc", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[413] = power_entry(power_op_sradi, "sradi", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[425] = power_entry(power_op_divde,"divde",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[427] = power_entry(power_op_divwe,"divwe",NULL,list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(RC)));
extended_op_31[429] = power_entry(power_op_stxvll, "stxvll", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[430] = power_entry(power_op_clrbhrb,"clrbhrb",NULL,operandSpec());
extended_op_31[434] = power_entry(power_op_slbie,"slbie",NULL,list_of(fn(RB)));
extended_op_31[435] = power_entry(power_op_mtvsrdd,"mtvsrdd",NULL,list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[438] = power_entry(power_op_ecowx, "ecowx", NULL, list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[439] = power_entry(power_op_sthux, "sthux", NULL, list_of(fn(RS))(fn(STUX<u16>)));
extended_op_31[444] = power_entry(power_op_or, "or", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[445] = power_entry(power_op_extswsli,"extswsli",NULL,list_of(fn(RS))(fn(RA))(fn(SH))(fn(RC)));
extended_op_31[457] = power_entry(power_op_divdu, "divdu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[459] = power_entry(power_op_divwu, "divwu", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[466] = power_entry(power_op_slbieg,"slbieg",NULL,list_of(fn(RS))(fn(RB)));
extended_op_31[467] = power_entry(power_op_mtspr, "mtspr", NULL, list_of(fn(RS))(fn(spr))(fn(Rc)));
extended_op_31[470] = power_entry(power_op_dcbi, "dcbi", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[476] = power_entry(power_op_nand, "nand", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[487] = power_entry(power_op_stvxl, "stvxl", NULL, list_of(fn(VRS))(fn(RA))(fn(RB)));
extended_op_31[488] = power_entry(power_op_nabs, "nabs", NULL, list_of(fn(RT))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[489] = power_entry(power_op_divd, "divd", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[491] = power_entry(power_op_divw, "divw", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(OE))(fn(Rc)));
extended_op_31[494] = power_entry(power_op_lfpdux, "lfpdux", NULL, list_of(fn(FRTP))(fn(LUX<dbl128>)));
extended_op_31[498] = power_entry(power_op_slbia,"slbia",NULL,list_of(fn(IH)));
extended_op_31[502] = power_entry(power_op_cli, "cli", NULL, list_of(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[506] = power_entry(power_op_popcntd, "popcntd", NULL, list_of(fn(RS))(fn(RA)));
extended_op_31[508] = power_entry(power_op_cmpb,"cmpb",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[512] = power_entry(power_op_mcrxr, "mcrxr", NULL, list_of(fn(BF)));
extended_op_31[518] = power_entry(power_op_qvlpclsx, "qvlpclsx", NULL, list_of(fn(QFRTP))(fn(LX<sp_float>)));
extended_op_31[519] = power_entry(power_op_qvlfsx, "qvlfsx", NULL,  list_of(fn(QFRTP))(fn(LX<sp_float>)));
extended_op_31[524] = power_entry(power_op_lxsspx, "lxsspx", NULL,  list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[526] = power_entry(power_op_stfpiwx, "stfpiwx", NULL, list_of(fn(FRSP))(fn(STUX<u64>)));
extended_op_31[531] = power_entry(power_op_clcs, "clcs", NULL, list_of(fn(RT))(fn(RA))(fn(Rc)));
extended_op_31[532] = power_entry(power_op_ldbrx,"ldbrx",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[533] = power_entry(power_op_lswx, "lswx", NULL, list_of(fn(RT))(fn(LX<u8>)));
extended_op_31[534] = power_entry(power_op_lwbrx, "lwbrx", NULL, list_of(fn(RT))(fn(LX<u32>)));
extended_op_31[535] = power_entry(power_op_lfsx, "lfsx", NULL, list_of(fn(FRT))(fn(LX<sp_float>)));
extended_op_31[536] = power_entry(power_op_srw, "srw", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[537] = power_entry(power_op_rrib, "rrib", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[538] = power_entry(power_op_cnttzw,"cnttzw",NULL,list_of(fn(RS))(fn(RA)));
extended_op_31[539] = power_entry(power_op_srd, "srd", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[541] = power_entry(power_op_maskir, "maskir", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[551] = power_entry(power_op_qvlfsux, "qvlfsux", NULL, list_of(fn(QFRTP))(fn(LUX<sp_float>)));
extended_op_31[566] = power_entry(power_op_tlbsync, "tlbsync", NULL, operandSpec());
extended_op_31[567] = power_entry(power_op_lfsux, "lfsux", NULL, list_of(fn(FRT))(fn(LUX<sp_float>)));
extended_op_31[570] = power_entry(power_op_cnttzd,"cnttzd",NULL,list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[576] = power_entry(power_op_mcrxrx,"mcrxrx",NULL,list_of(fn(BF)));
extended_op_31[582] = power_entry(power_op_lwat,"lwat",NULL,list_of(fn(RT))(fn(RA))(fn(FC)));
extended_op_31[583] = power_entry(power_op_qvlfdx, "qvlfdx", NULL, list_of(fn(QFRTP))(fn(LX<dbl128>)));

// fn(STU<u64>) = RA+DS
extended_op_31[588] = power_entry(power_op_lxsdx, "lxsdx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));

extended_op_31[595] = power_entry(power_op_mfsr, "mfsr", NULL, list_of(fn(RT))(fn(SR)));
extended_op_31[597] = power_entry(power_op_lswi, "lswi", NULL, list_of(fn(RT))(fn(L<u8>))(fn(NB)));
extended_op_31[598] = power_entry(power_op_sync, "sync", NULL, operandSpec());
extended_op_31[599] = power_entry(power_op_lfdx, "lfdx", NULL, list_of(fn(FRT))(fn(LX<dp_float>)));
extended_op_31[614] = power_entry(power_op_ldat,"ldat",NULL,list_of(fn(RT))(fn(RA))(fn(FC)));
extended_op_31[615] = power_entry(power_op_qvlfdux, "qvlfdux", NULL, list_of(fn(QFRTP))(fn(LUX<dbl128>)));
extended_op_31[627] = power_entry(power_op_mfsri, "mfsri", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[630] = power_entry(power_op_dclst, "dclst", NULL, list_of(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[631] = power_entry(power_op_lfdux, "lfdux", NULL, list_of(fn(FRT))(fn(LUX<dp_float>)));
extended_op_31[645] = power_entry(power_op_qvstfsxi, "qvstfsxi", NULL, list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[647] = power_entry(power_op_qvstfsx, "qvstfsx", NULL, list_of(fn(QFRSP))(fn(STX<sp_float>)));
extended_op_31[652] = power_entry(power_op_stxsspx, "stxsspx", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[654] = power_entry(power_op_tbegin,"tbegin",NULL,list_of(fn(A))(fn(R)));
extended_op_31[659] = power_entry(power_op_mfsrin, "mfsrin", NULL, list_of(fn(RT))(fn(RB)));
extended_op_31[660] = power_entry(power_op_stdbrx,"stdbrx",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[661] = power_entry(power_op_stswx, "stswx", NULL, list_of(fn(RS))(fn(STX<u8>)));
extended_op_31[662] = power_entry(power_op_stwbrx, "stwbrx", NULL, list_of(fn(RS))(fn(STX<u32>)));
extended_op_31[663] = power_entry(power_op_stfsx, "stfsx", NULL, list_of(fn(FRS))(fn(STX<sp_float>)));
extended_op_31[664] = power_entry(power_op_srq, "srq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[665] = power_entry(power_op_sre, "sre", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[679] = power_entry(power_op_qvstfsux, "qvstfsux", NULL, list_of(fn(QFRSP))(fn(STUX<sp_float>)));
extended_op_31[682] = power_entry(power_op_addex, "addex", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(CY)));
extended_op_31[686] = power_entry(power_op_tend,"tend",NULL,list_of(fn(A)));
extended_op_31[694] = power_entry(power_op_stbcx,"stbcx",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[695] = power_entry(power_op_stfsux, "stfsux", NULL, list_of(fn(FRS))(fn(STUX<sp_float>)));
extended_op_31[696] = power_entry(power_op_sriq, "sriq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[709] = power_entry(power_op_qvstfdxi, "qvstfdxi", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[711] = power_entry(power_op_qvstfdx, "qvstfdx", NULL, list_of(fn(QFRSP))(fn(STX<dbl128>)));
extended_op_31[716] = power_entry(power_op_stxsdx, "stxsdx", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[718] = power_entry(power_op_tcheck,"tcheck",NULL,list_of(fn(BF)));
extended_op_31[725] = power_entry(power_op_stswi, "stswi", NULL, list_of(fn(RS))(fn(ST<u8>))(fn(NB)));
extended_op_31[726] = power_entry(power_op_sthcx,"sthcx",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[727] = power_entry(power_op_stfdx, "stfdx", NULL, list_of(fn(FRS))(fn(STX<dp_float>)));
extended_op_31[728] = power_entry(power_op_srlq, "srlq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[729] = power_entry(power_op_sreq, "sreq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[710] = power_entry(power_op_stwat,"stwat",NULL,list_of(fn(RS))(fn(RA))(fn(FC)));
extended_op_31[742] = power_entry(power_op_stdat,"stdat",NULL,list_of(fn(RS))(fn(RA))(fn(FC)));
extended_op_31[743] = power_entry(power_op_qvlstdux, "qvstfdux", NULL, list_of(fn(QFRSP))(fn(STUX<dbl128>)));
extended_op_31[750] = power_entry(power_op_tsr,"tsr",NULL,list_of(fn(LL)));
extended_op_31[755] = power_entry(power_op_darn,"darn",NULL,list_of(fn(RT))(fn(L)));
extended_op_31[759] = power_entry(power_op_stfdux, "stfdux", NULL, list_of(fn(FRS))(fn(STUX<dp_float>)));
extended_op_31[760] = power_entry(power_op_srliq, "srliq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[774] = power_entry(power_op_copy,"copy",NULL,list_of(fn(RA))(fn(RB)));
extended_op_31[777] = power_entry(power_op_modsd,"modsd",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[779] = power_entry(power_op_modsw,"modsw",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[780] = power_entry(power_op_lxvw4x, "lxvw4x", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[781] = power_entry(power_op_lxsibzx, "lxsibzx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[782] = power_entry(power_op_tabortwc,"tabortwc",NULL,list_of(fn(TO))(fn(RA))(fn(RB)));
extended_op_31[789] = power_entry(power_op_lwzcix,"lwzcix",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[790] = power_entry(power_op_lhbrx, "lhbrx", NULL, list_of(fn(RT))(fn(LX<u16>)));
extended_op_31[791] = power_entry(power_op_lfdpx,"lfdpx",NULL,list_of(fn(FRTP))(fn(RA))(fn(RB)));
extended_op_31[792] = power_entry(power_op_sraw, "sraw", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[794] = power_entry(power_op_srad, "srad", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[812] = power_entry(power_op_lxvh8x, "lxvh8x", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[813] = power_entry(power_op_lxsihzx, "lxsihzx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[813] = power_entry(power_op_lxsihzx, "lxsihzx", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[814] = power_entry(power_op_tabortdc,"tabortdc",NULL,list_of(fn(TO))(fn(RA))(fn(RB)));
extended_op_31[818] = power_entry(power_op_rac, "rac", NULL, list_of(fn(RT))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[821] = power_entry(power_op_lhzcix,"lhzcix",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[823] = power_entry(power_op_lfqux, "lfqux", NULL, list_of(fn(FRT))(fn(LUX<dbl128>))(fn(Rc)));
extended_op_31[824] = power_entry(power_op_srawi, "srawi", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[838] = power_entry(power_op_cp_abort,"cp_abort",NULL,operandSpec());
extended_op_31[839] = power_entry(power_op_qvlfiwzx, "qvlfiwzx", NULL, list_of(fn(QFRTP))(fn(LX<u32>)));
extended_op_31[844] = power_entry(power_op_lxvd2x, "lxvd2x", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[846] = power_entry(power_op_tabortwci,"tabortwci",NULL,list_of(fn(TO))(fn(RA))(fn(SI)));
extended_op_31[850] = power_entry(power_op_slbiag,"slbiag",NULL,list_of(fn(RS)));
extended_op_31[851] = power_entry(power_op_slbmfev,"slbmfev",NULL,list_of(fn(RT))(fn(L))(fn(RB)));
extended_op_31[853] = power_entry(power_op_lbzcix,"lbzcix",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[854] = power_entry(power_op_eieio, "eieio", NULL, operandSpec());
extended_op_31[855] = power_entry(power_op_lfiwax,"lfiwax",NULL,list_of(fn(FRT))(fn(RA))(fn(RB)));
extended_op_31[871] = power_entry(power_op_qvlfiwax, "qvlfiwax", NULL, list_of(fn(QFRTP))(fn(LX<u32>)));
extended_op_31[876] = power_entry(power_op_lxvb16x, "lxvb16x", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_31[878] = power_entry(power_op_tabortdci,"tabortdci",NULL,list_of(fn(TO))(fn(RA))(fn(SI)));
extended_op_31[885] = power_entry(power_op_ldcix,"ldcix",NULL,list_of(fn(RT))(fn(RA))(fn(RB)));
extended_op_31[886] = power_entry(power_op_msgsync,"msgsync",NULL,operandSpec());
extended_op_31[887] = power_entry(power_op_lfiwzx,"lfiwzx",NULL,list_of(fn(FRT))(fn(RA))(fn(RB)));
extended_op_31[902] = power_entry(power_op_paste,"paste",NULL,list_of(fn(RA))(fn(RB)));
extended_op_31[908] = power_entry(power_op_stxvw4x, "stxvw4x", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[909] = power_entry(power_op_stxsibx, "stxsibx", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[910] = power_entry(power_op_tabort,"tabort",NULL,list_of(fn(RA)));
extended_op_31[915] = power_entry(power_op_slbmfee,"slbmfee",NULL,list_of(fn(RT))(fn(L))(fn(RB)));
extended_op_31[917] = power_entry(power_op_stwcix,"stwcix",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[918] = power_entry(power_op_sthbrx, "sthbrx", NULL, list_of(fn(RS))(fn(STX<u16>)));
//extended_op_31[919] = power_entry(power_op_slbfee,"slbfee",NULL,list_of(fn(RT))(fn(RB)));
extended_op_31[920] = power_entry(power_op_sraq, "sraq", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[921] = power_entry(power_op_srea, "srea", NULL, list_of(fn(RS))(fn(RA))(fn(RB))(fn(Rc)));
extended_op_31[922] = power_entry(power_op_extsh, "extsh", NULL, list_of(fn(RS))(fn(RA))(fn(OE))(fn(Rc)));
extended_op_31[940] = power_entry(power_op_stxvh8x, "stxvh8x", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[941] = power_entry(power_op_stxsihx, "stxsihx", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[942] = power_entry(power_op_treclaim,"treclaim",NULL,list_of(fn(RA)));
extended_op_31[949] = power_entry(power_op_sthcix,"sthcix",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[951] = power_entry(power_op_stfqux, "stfqux", NULL, list_of(fn(FRS))(fn(STUX<dbl128>))(fn(Rc)));
extended_op_31[952] = power_entry(power_op_sraiq, "sraiq", NULL, list_of(fn(RS))(fn(RA))(fn(SH))(fn(Rc)));
extended_op_31[954] = power_entry(power_op_extsb, "extsb", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[967] = power_entry(power_op_qvstfiwx, "qvstfiwx", NULL, list_of(fn(QFRSP))(fn(STX<u32>)));
extended_op_31[972] = power_entry(power_op_stxsibx, "stxvd2x", NULL, list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[974] = power_entry(power_op_stfpdx, "stfpdx", NULL, list_of(fn(FRSP))(fn(STX<dbl128>)));
extended_op_31[978] = power_entry(power_op_tlbld, "tlbld", NULL, list_of(fn(RB)));
extended_op_31[979] = power_entry(power_op_slbfee,"slbfee",NULL,list_of(fn(RT))(fn(RB)));
extended_op_31[981] = power_entry(power_op_stbcix,"stbcix",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[982] = power_entry(power_op_icbi, "icbi", NULL, list_of(fn(RA))(fn(RB)));
extended_op_31[983] = power_entry(power_op_stfiwx, "stfiwx", NULL, list_of(fn(FRS))(fn(STX<u32>)));
extended_op_31[986] = power_entry(power_op_extsw, "extsw", NULL, list_of(fn(RS))(fn(RA))(fn(Rc)));
extended_op_31[1004] = power_entry(power_op_stxvb16x,"stxvb16x",NULL,list_of(fn(XS))(fn(RA))(fn(RB)));
extended_op_31[1006] = power_entry(power_op_trechkpt,"trechkpt",NULL,operandSpec());
extended_op_31[1010] = power_entry(power_op_tlbli, "tlbli", NULL, list_of(fn(RB)));
extended_op_31[1013] = power_entry(power_op_stdcix,"stdcix",NULL,list_of(fn(RS))(fn(RA))(fn(RB)));
extended_op_31[1014] = power_entry(power_op_dcbz, "dcbz", NULL, list_of(fn(RA))(fn(RB)));























extended_op_57[2] = power_entry(power_op_lxsd, "lxsd", NULL, list_of(fn(VRT))(fn(STU<u64>)));
extended_op_57[3] = power_entry(power_op_lxssp, "lxssp", NULL, list_of(fn(VRT))(fn(STU<u64>)));


extended_op_58[0] = power_entry(power_op_ld, "ld", NULL, list_of(fn(RT))(fn(L<u64>)));
extended_op_58[1] = power_entry(power_op_ldu, "ldu", NULL, list_of(fn(RT))(fn(LU<u64>)));
extended_op_58[2] = power_entry(power_op_lwa, "lwa", NULL, list_of(fn(RT))(fn(L<s32>)));

  
  
  extended_op_59[2] = power_entry(power_op_dadd,"dadd",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_59[3] = power_entry(power_op_dqua,"dqua",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_59[18] = power_entry(power_op_fdivs, "fdivs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_59[20] = power_entry(power_op_fsubs, "fsubs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_59[21] = power_entry(power_op_fadds, "fadds", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_59[22] = power_entry(power_op_fsqrts, "fsqrts", NULL, list_of(fn(setFPMode))(fn(RT))(fn(RB))(fn(Rc)));
extended_op_59[24] = power_entry(power_op_fres, "fres", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_59[25] = power_entry(power_op_fmuls, "fmuls", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRC))(fn(Rc)));
extended_op_59[26] = power_entry(power_op_frsqrtes,"frsqrtes",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_59[28] = power_entry(power_op_fmsubs, "fmsubs", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[29] = power_entry(power_op_fmadds, "fmadds", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[30] = power_entry(power_op_fnmsubs, "fnmsubs", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[31] = power_entry(power_op_fnmadds, "fnmadds", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_59[34] = power_entry(power_op_dmul,"dmul",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_59[35] = power_entry(power_op_drrnd,"drrnd",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_59[66] = power_entry(power_op_dscli,"dscli",NULL,list_of(fn(FRT))(fn(FRA))(fn(SH))(fn(RC)));
extended_op_59[67] = power_entry(power_op_dquai,"dquai",NULL,list_of(fn(FRT))(fn(TE))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_59[98] = power_entry(power_op_dscri,"dscri",NULL,list_of(fn(FRT))(fn(FRA))(fn(SH))(fn(RC)));
extended_op_59[99] = power_entry(power_op_drintx,"drintx",NULL,list_of(fn(FRT))(fn(R))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_59[130] = power_entry(power_op_dcmpo,"dcmpo",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_59[162] = power_entry(power_op_dtstex,"dtstex",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_59[194] = power_entry(power_op_dtstdc,"dtstdc",NULL,list_of(fn(BF))(fn(FRA))(fn(DCM)));
extended_op_59[226] = power_entry(power_op_dtstdg,"dtstdg",NULL,list_of(fn(BF))(fn(FRA))(fn(DGM)));
extended_op_59[227] = power_entry(power_op_drintn,"drintn",NULL,list_of(fn(FRT))(fn(R))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_59[258] = power_entry(power_op_dctdp,"dctdp",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_59[290] = power_entry(power_op_dctfix,"dctfix",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_59[322] = power_entry(power_op_ddedpd,"ddedpd",NULL,list_of(fn(FRT))(fn(SP))(fn(FRB))(fn(RC)));
extended_op_59[354] = power_entry(power_op_dxex,"dxex",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_59[514] = power_entry(power_op_dsub,"dsub",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_59[546] = power_entry(power_op_ddiv,"ddiv",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_59[642] = power_entry(power_op_dcmpu,"dcmpu",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_59[674] = power_entry(power_op_dtstsf,"dtstsf",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_59[675] = power_entry(power_op_dtstsfi,"dtstsfi",NULL,list_of(fn(BF))(fn(UIM))(fn(FRB)));
extended_op_59[770] = power_entry(power_op_drsp,"drsp",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_59[802] = power_entry(power_op_dcffix,"dcffix",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_59[834] = power_entry(power_op_denbcd,"denbcd",NULL,list_of(fn(FRT))(fn(S))(fn(FRB))(fn(RC)));
extended_op_59[866] = power_entry(power_op_diex,"diex",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_59[974] = power_entry(power_op_fcfidus,"fcfidus",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));





// A list of special purpose entries requiring specific identification
extended_op_60_specials[0] =  power_entry(power_op_xxpermdi,"xxpermdi",NULL,list_of(fn(XT))(fn(XA))(fn(XB))(fn(DM)));
extended_op_60_specials[1] = power_entry(power_op_xvtdivsp,"xvtdivsp",NULL,list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60_specials[2] = power_entry(power_op_xxsel,"xxsel",NULL,list_of(fn(XT))(fn(XA))(fn(XB))(fn(XC)));
extended_op_60_specials[3] = power_entry(power_op_xxsldwi,"xxsldwi",NULL,list_of(fn(XT))(fn(XA))(fn(XB))(fn(SHW)));
extended_op_60_specials[4] = power_entry(power_op_xvnmaddasp,"xvnmaddasp",NULL,list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60_specials[5] = power_entry(power_op_xscmpexpdp, "xscmpexpdp", NULL, list_of(fn(BF))(fn(XB)));
extended_op_60_specials[6] = power_entry(power_op_xscvuxddp, "xscvuxddp", NULL, list_of(fn(XT))(fn(XB)));

extended_op_60[360] = power_entry(power_op_xxspltib,"xxspltib",NULL,list_of(fn(XT))(fn(IMM8)));


extended_op_60[0] = power_entry(power_op_xsaddsp, "xsaddsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[1] = power_entry(power_op_xsaddsp, "xsaddsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[2] = power_entry(power_op_xsmaddasp, "xsmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[3] = power_entry(power_op_xsmaddasp, "xsmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[6] = power_entry(power_op_xscmpeqdp, "xscmpeqdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[7] = power_entry(power_op_xscmpeqdp, "xscmpeqdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[10] = power_entry(power_op_xsrsqrtesp, "xsrsqrtesp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[11] = power_entry(power_op_xssqrtsp, "xssqrtsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[16] = power_entry(power_op_xssubsp, "xssubsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[17] = power_entry(power_op_xssubsp, "xssubsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[18] = power_entry(power_op_xsmaddmsp, "xsmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[19] = power_entry(power_op_xsmaddmsp, "xsmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[22] = power_entry(power_op_xscmpgtdp, "xscmpgtdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[23] = power_entry(power_op_xscmpgtdp, "xscmpgtdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[26] = power_entry(power_op_xsresp, "xsresp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[32] = power_entry(power_op_xsmulsp, "xsmulsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[33] = power_entry(power_op_xsmulsp, "xsmulsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[34] = power_entry(power_op_xsmsubasp, "xsmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[35] = power_entry(power_op_xsmsubasp, "xsmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[36] = power_entry(power_op_xxmrghw, "xxmrghw", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[37] = power_entry(power_op_xxmrghw, "xxmrghw", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[38] = power_entry(power_op_xscmpgedp, "xscmpgedp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[39] = power_entry(power_op_xscmpgedp, "xscmpgedp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[48] = power_entry(power_op_xsdivsp, "xsdivsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[49] = power_entry(power_op_xsdivsp, "xsdivsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[50] = power_entry(power_op_xsmsubmsp, "xsmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[51] = power_entry(power_op_xsmsubmsp, "xsmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[52] = power_entry(power_op_xxperm, "xxperm", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[53] = power_entry(power_op_xxperm, "xxperm", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[64] = power_entry(power_op_xsadddp, "xsadddp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[65] = power_entry(power_op_xsadddp, "xsadddp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[66] = power_entry(power_op_xsmaddadp, "xsmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[67] = power_entry(power_op_xsmaddadp, "xsmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[70] = power_entry(power_op_xscmpudp, "xscmpudp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[71] = power_entry(power_op_xscmpudp, "xscmpudp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[72] = power_entry(power_op_xscvdpuxws, "xscvdpuxws", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[73] = power_entry(power_op_xsrdpi,"xsrdpi",NULL,list_of(fn(XT))(fn(XB)));
extended_op_60[74] = power_entry(power_op_xsrsqrtedp, "xsrsqrtedp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[75] = power_entry(power_op_xssqrtdp, "xssqrtdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[78] = power_entry(power_op_xsrdpi, "xsrdpi", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[80] = power_entry(power_op_xssubdp, "xssubdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[81] = power_entry(power_op_xssubdp, "xssubdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[82] = power_entry(power_op_xsmaddmdp, "xsmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[83] = power_entry(power_op_xsmaddmdp, "xsmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));

extended_op_60[86] = power_entry(power_op_xscmpodp, "xscmpodp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[87] = power_entry(power_op_xscmpodp, "xscmpodp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));

extended_op_60[88] = power_entry(power_op_xscvdpsxws, "xscvdpsxws", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[89] = power_entry(power_op_xsrdpiz, "xsrdpiz", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[90] = power_entry(power_op_xsredp, "xsredp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[96] = power_entry(power_op_xsmuldp, "xsmuldp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[97] = power_entry(power_op_xsmuldp, "xsmuldp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[98] = power_entry(power_op_xsmsubadp, "xsmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[99] = power_entry(power_op_xsmsubadp, "xsmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[100] = power_entry(power_op_xxmrglw, "xxmrglw", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[101] = power_entry(power_op_xxmrglw, "xxmrglw", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[105] = power_entry(power_op_xsrdpip, "xsrdpip", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[106] = power_entry(power_op_xstsqrtdp, "xstsqrtdp", NULL, list_of(fn(BF))(fn(XB)));
extended_op_60[107] = power_entry(power_op_xsrdpic, "xsrdpic", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[112] = power_entry(power_op_xsdivdp, "xsdivdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[113] = power_entry(power_op_xsdivdp, "xsdivdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[114] = power_entry(power_op_xsmsubmdp, "xsmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[115] = power_entry(power_op_xsmsubmdp, "xsmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[116] = power_entry(power_op_xxpermr, "xxpermr", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[117] = power_entry(power_op_xxpermr, "xxpermr", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
//extended_op_60[118] = power_entry(power_op_xscmpexpdp, "xscmpexpdp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
//extended_op_60[119] = power_entry(power_op_xscmpexpdp, "xscmpexpdp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[121] = power_entry(power_op_xsrdpim, "xsrdpim", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[122] = power_entry(power_op_xstdivdp, "xstdivdp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[123] = power_entry(power_op_xstdivdp, "xstdivdp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[128] = power_entry(power_op_xvaddsp, "xvaddsp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[129] = power_entry(power_op_xvaddsp, "xvaddsp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[130] = power_entry(power_op_xvmaddasp, "xvmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[131] = power_entry(power_op_xvmaddasp, "xvmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[134] = power_entry(power_op_xvcmpeqsp, "xvcmpeqsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[135] = power_entry(power_op_xvcmpeqsp, "xvcmpeqsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[136] = power_entry(power_op_xvcvspuxws, "xvcvspuxws", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[137] = power_entry(power_op_xvrspi, "xvrspi", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[138] = power_entry(power_op_xvrsqrtesp, "xvrsqrtesp",  NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[139] = power_entry(power_op_xvsqrtsp, "xvsqrtsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[144] = power_entry(power_op_xvsubsp, "xvsubsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[145] = power_entry(power_op_xvsubsp, "xvsubsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[146] = power_entry(power_op_xvmaddmsp, "xvmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[147] = power_entry(power_op_xvmaddmsp, "xvmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[150] = power_entry(power_op_xvcmpgtsp, "xvcmpgtsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[151] = power_entry(power_op_xvcmpgtsp, "xvcmpgtsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[152] = power_entry(power_op_xvcvspsxws, "xvcvspsxws", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[153] = power_entry(power_op_xvrspiz, "xvrspiz", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[154] = power_entry(power_op_xvresp, "xvresp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[160] = power_entry(power_op_xvmulsp, "xvmulsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[161] = power_entry(power_op_xvmulsp, "xvmulsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[162] = power_entry(power_op_xvmsubasp, "xvmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[164] = power_entry(power_op_xxspltw, "xxspltw", NULL, list_of(fn(XT))(fn(XB))(fn(UIM)));
extended_op_60[165] = power_entry(power_op_xxextractuw, "xxextractuw", NULL, list_of(fn(XT))(fn(XB))(fn(UIM)));
extended_op_60[166] = power_entry(power_op_xvcmpgesp, "xvcmpgesp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[167] = power_entry(power_op_xvcmpgesp, "xvcmpgesp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[168] = power_entry(power_op_xvcvuxwsp, "xvcvuxwsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[169] = power_entry(power_op_xvrspip, "xvrspip", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[170] = power_entry(power_op_xvtsqrtsp, "xvtsqrtsp", NULL, list_of(fn(BF))(fn(XB)));
extended_op_60[171] = power_entry(power_op_xvrspic, "xvrspic", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[178] = power_entry(power_op_xvmsubmsp, "xvmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[181] = power_entry(power_op_xxinsertw, "xxinsertw", NULL, list_of(fn(XT))(fn(XB))(fn(UIM)));
extended_op_60[184] = power_entry(power_op_xvcvsxwsp, "xvcvsxwsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[185] = power_entry(power_op_xvrspim, "xvrspim", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[186] = power_entry(power_op_xvdivsp, "xvdivsp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[187] = power_entry(power_op_xvdivsp, "xvdivsp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[192] = power_entry(power_op_xvadddp, "xvadddp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[193] = power_entry(power_op_xvadddp, "xvadddp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[194] = power_entry(power_op_xvmaddadp, "xvmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[195] = power_entry(power_op_xvmaddadp, "xvmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[198] = power_entry(power_op_xvcmpeqdp, "xvcmpeqdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[199] = power_entry(power_op_xvcmpeqdp, "xvcmpeqdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[200] = power_entry(power_op_xvcvdpuxws, "xvcvdpuxws", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[201] = power_entry(power_op_xvrdpi, "xvrdpi", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[202] = power_entry(power_op_xvrsqrtedp, "xvrsqrtedp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[203] = power_entry(power_op_xvsqrtdp, "xvsqrtdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[208] = power_entry(power_op_xvsubdp, "xvsubdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[209] = power_entry(power_op_xvsubdp, "xvsubdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[210] = power_entry(power_op_xvmaddmdp, "xvmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[211] = power_entry(power_op_xvmaddmdp, "xvmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[214] = power_entry(power_op_xvcmpgtdp, "xvcmpgtdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[215] = power_entry(power_op_xvcmpgtdp, "xvcmpgtdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[216] = power_entry(power_op_xvcvdpsxws, "xvcvdpsxws", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[217] = power_entry(power_op_xvrdpiz, "xvrdpiz", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[218] = power_entry(power_op_xvredp, "xvredp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[224] = power_entry(power_op_xvmuldp, "xvmuldp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[226] = power_entry(power_op_xvmsubadp, "xvmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[227] = power_entry(power_op_xvmsubadp, "xvmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[230] = power_entry(power_op_xvcmpgedp, "xvcmpgedp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[231] = power_entry(power_op_xvcmpgedp, "xvcmpgedp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[232] = power_entry(power_op_xvcvuxwdp, "xvcvuxwdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[233] = power_entry(power_op_xvrdpip, "xvrdpip", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[234] = power_entry(power_op_xvtsqrtdp, "xvtsqrtdp", NULL, list_of(fn(BF))(fn(XB)));
extended_op_60[235] = power_entry(power_op_xvrdpic, "xvrdpic", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[240] = power_entry(power_op_xvdivdp, "xvdivdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[241] = power_entry(power_op_xvdivdp, "xvdivdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[242] = power_entry(power_op_xvmsubmdp, "xvmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[243] = power_entry(power_op_xvmsubmdp, "xvmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[248] = power_entry(power_op_xvcvsxwdp, "xvcvsxwdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[249] = power_entry(power_op_xvrdpim, "xvrdpim", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[250] = power_entry(power_op_xvtdivdp, "xvtdivdp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[251] = power_entry(power_op_xvtdivdp, "xvtdivdp", NULL, list_of(fn(BF))(fn(XA))(fn(XB)));
extended_op_60[256] = power_entry(power_op_xsmaxcdp, "xsmaxcdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[257] = power_entry(power_op_xsmaxcdp, "xsmaxcdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[258] = power_entry(power_op_xsnmaddasp, "xsnmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[259] = power_entry(power_op_xsnmaddasp, "xsnmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[260] = power_entry(power_op_xxland, "xxland", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[261] = power_entry(power_op_xxland, "xxland", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[265] = power_entry(power_op_xscvdpsp, "xscvdpsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[267] = power_entry(power_op_xscvdpspn, "xscvdpspn", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[272] = power_entry(power_op_xsmincdp, "xsmincdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[273] = power_entry(power_op_xsmincdp, "xsmincdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[274] = power_entry(power_op_xsnmaddmsp, "xsnmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[275] = power_entry(power_op_xsnmaddmsp, "xsnmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[276] = power_entry(power_op_xxlandc, "xxlandc", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[277] = power_entry(power_op_xxlandc, "xxlandc", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[281] = power_entry(power_op_xsrsp, "xsrsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[288] = power_entry(power_op_xsmaxjdp, "xsmaxjdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[289] = power_entry(power_op_xsmaxjdp, "xsmaxjdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[290] = power_entry(power_op_xsnmsubasp, "xsnmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[291] = power_entry(power_op_xsnmsubasp, "xsnmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[292] = power_entry(power_op_xxlor, "xxlor", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[293] = power_entry(power_op_xxlor, "xxlor", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[296] = power_entry(power_op_xscvuxdsp, "xscvuxdsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[298] = power_entry(power_op_xststdcsp, "xststdcsp", NULL, list_of(fn(BF))(fn(DCMX))(fn(XB)));

extended_op_60[304] = power_entry(power_op_xsminjdp, "xsminjdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[305] = power_entry(power_op_xsminjdp, "xsminjdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[306] = power_entry(power_op_xsnmsubmsp, "xsnmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[307] = power_entry(power_op_xsnmsubmsp, "xsnmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[308] = power_entry(power_op_xxlxor, "xxlxor", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[309] = power_entry(power_op_xxlxor, "xxlxor", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[312] = power_entry(power_op_xscvsxdsp, "xscvsxdsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[320] = power_entry(power_op_xsmaxdp, "xsmaxdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[321] = power_entry(power_op_xsmaxdp, "xsmaxdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[322] = power_entry(power_op_xsnmaddadp, "xsnmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[323] = power_entry(power_op_xsnmaddadp, "xsnmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[324] = power_entry(power_op_xxlnor, "xxlnor", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[325] = power_entry(power_op_xxlnor, "xxlnor", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[328] = power_entry(power_op_xscvdpuxds, "xscvdpuxds", NULL, list_of(fn(XT))(fn(XB))); //power_entry(power_op_xscmpexpdp, "xscmpexpdp", NULL, list_of(fn(BF))(fn(XB)));
extended_op_60[329] = power_entry(power_op_xscvspdp, "xscvspdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[331] = power_entry(power_op_xscvspdpn, "xscvspdpn", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[336] = power_entry(power_op_xsmindp, "xsmindp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[337] = power_entry(power_op_xsmindp, "xsmindp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[338] = power_entry(power_op_xsnmaddmdp, "xsnmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[339] = power_entry(power_op_xsnmaddmdp, "xsnmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[340] = power_entry(power_op_xxlorc, "xxlorc", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[341] = power_entry(power_op_xxlorc, "xxlorc", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[344] = power_entry(power_op_xscvdpsxds, "xscvdpsxds", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[345] = power_entry(power_op_xsabsdp, "xsabsdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[352] = power_entry(power_op_xscpsgndp, "xscpsgndp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[353] = power_entry(power_op_xscpsgndp, "xscpsgndp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[354] = power_entry(power_op_xsnmsubadp, "xsnmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[355] = power_entry(power_op_xsnmsubadp, "xsnmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[356] = power_entry(power_op_xxlnand, "xxlnand", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[357] = power_entry(power_op_xxlnand, "xxlnand", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[370] = power_entry(power_op_xsnmsubmdp, "xsnmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[371] = power_entry(power_op_xsnmsubmdp, "xsnmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[372] = power_entry(power_op_xxleqv, "xxleqv", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[373] = power_entry(power_op_xxleqv, "xxleqv", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[382] = power_entry(power_op_xvnmaddasp, "xvnmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[383] = power_entry(power_op_xvnmaddasp, "xvnmaddasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[384] = power_entry(power_op_xvmaxsp, "xvmaxsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[390] = power_entry(power_op_xvcmpeqsp, "xvcmpeqsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[391] = power_entry(power_op_xvcmpeqsp, "xvcmpeqsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[400] = power_entry(power_op_xvminsp, "xvminsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[401] = power_entry(power_op_xvminsp, "xvminsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[402] = power_entry(power_op_xvnmaddmsp, "xvnmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[403] = power_entry(power_op_xvnmaddmsp, "xvnmaddmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[406] = power_entry(power_op_xvcmpgtsp, "xvcmpgtsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[407] = power_entry(power_op_xvcmpgtsp, "xvcmpgtsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[416] = power_entry(power_op_xvcpsgnsp, "xvcpsgnsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[417] = power_entry(power_op_xvcpsgnsp, "xvcpsgnsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[422] = power_entry(power_op_xvcmpgesp, "xvcmpgesp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[423] = power_entry(power_op_xvcmpgesp, "xvcmpgesp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[424] = power_entry(power_op_xvcvuxdsp, "xvcvuxdsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[425] = power_entry(power_op_xvnabssp, "xvnabssp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[432] = power_entry(power_op_xviexpsp, "xviexpsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[433] = power_entry(power_op_xviexpsp, "xviexpsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[440] = power_entry(power_op_xvcvsxdsp, "xvcvsxdsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[441] = power_entry(power_op_xvnegsp, "xvnegsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[448] = power_entry(power_op_xvmaxdp, "xvmaxdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[449] = power_entry(power_op_xvmaxdp, "xvmaxdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[450] = power_entry(power_op_xvnmaddadp, "xvnmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[451] = power_entry(power_op_xvnmaddadp, "xvnmaddadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[454] = power_entry(power_op_xvcmpeqdp, "xvcmpeqdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[455] = power_entry(power_op_xvcmpeqdp, "xvcmpeqdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[464] = power_entry(power_op_xvmindp, "xvmindp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[466] = power_entry(power_op_xvnmaddmdp, "xvnmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[467] = power_entry(power_op_xvnmaddmdp, "xvnmaddmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[470] = power_entry(power_op_xvcmpgtdp, "xvcmpgtdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[471] = power_entry(power_op_xvcmpgtdp, "xvcmpgtdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[480] = power_entry(power_op_xvcpsgndp, "xvcpsgndp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[481] = power_entry(power_op_xvcpsgndp, "xvcpsgndp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[482] = power_entry(power_op_xvnmsubadp, "xvnmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[483] = power_entry(power_op_xvnmsubadp, "xvnmsubadp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[486] = power_entry(power_op_xvcmpgedp, "xvcmpgedp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[487] = power_entry(power_op_xvcmpgedp, "xvcmpgedp", NULL, list_of(fn(XT))(fn(XA))(fn(XB))(fn(Rc)));
extended_op_60[488] = power_entry(power_op_xvcvuxddp, "xvcvuxddp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[489] = power_entry(power_op_xvnabsdp, "xvnabsdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[496] = power_entry(power_op_xviexpdp, "xviexpdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[497] = power_entry(power_op_xviexpdp, "xviexpdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[498] = power_entry(power_op_xvnmsubmdp, "xvnmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[499] = power_entry(power_op_xvnmsubmdp, "xvnmsubmdp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[504] = power_entry(power_op_xvcvsxddp, "xvcvsxddp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[505] = power_entry(power_op_xvnegdp, "xvnegdp", NULL, list_of(fn(XT))(fn(XB)));
//third level opcode included:
//extended_op_60[347] = power_entry(power_op_extended, "extended", fn(extended_op_60_347), operandSpec()));
extended_op_60[361] = power_entry(power_op_xsnabsdp, "xsnabsdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[362] = power_entry(power_op_xststdcdp, "xststdcdp", NULL, list_of(fn(BF))(fn(DCMX))(fn(XB)));
extended_op_60[376] = power_entry(power_op_xscvsxddp, "xscvsxddp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[377] = power_entry(power_op_xsnegdp, "xsnegdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[392] = power_entry(power_op_xvcvspuxds, "xvcvspuxds", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[393] = power_entry(power_op_xvcvdpsp, "xvcvdpsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[408] = power_entry(power_op_xvcvspsxds, "xvcvspsxds", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[409] = power_entry(power_op_xvabssp, "xvabssp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[418] = power_entry(power_op_xvnmsubasp, "xvnmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[419] = power_entry(power_op_xvnmsubasp, "xvnmsubasp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[426] = power_entry(power_op_xvtstdcsp, "xvtstdcsp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[427] = power_entry(power_op_xvtstdcsp, "xvtstdcsp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[434] = power_entry(power_op_xvnmsubmsp, "xvnmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[435] = power_entry(power_op_xvnmsubmsp, "xvnmsubmsp", NULL, list_of(fn(XT))(fn(XA))(fn(XB)));
extended_op_60[442] = power_entry(power_op_xvtstdcsp, "xvtstdcsp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[443] = power_entry(power_op_xvtstdcsp, "xvtstdcsp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[456] = power_entry(power_op_xvcvdpuxds, "xvcvdpuxds", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[457] = power_entry(power_op_xvcvspdp, "xvcvspdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[459] = power_entry(power_op_xsiexpdp, "xsiexpdp", NULL, list_of(fn(XT))(fn(RA))(fn(RB)));
extended_op_60[472] = power_entry(power_op_xvcvdpsxds, "xvcvdpsxds", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60[473] = power_entry(power_op_xvabsdp, "xvabsdp", NULL, list_of(fn(XT))(fn(XB)));
//extended_op_60[475] = power_entry(power_op_extended, "extended", fn(extended_op_60_475), operandSpec()));
extended_op_60[490] = power_entry(power_op_xvtstdcdp, "xvtstdcdp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[491] = power_entry(power_op_xvtstdcdp, "xvtstdcdp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[506] = power_entry(power_op_xvtstdcdp, "xvtstdcdp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));
extended_op_60[507] = power_entry(power_op_xvtstdcdp, "xvtstdcdp", NULL, list_of(fn(XT))(fn(XB))(fn(DCMX)));



  extended_op_61[2] = power_entry(power_op_stxsd, "stxsd", NULL, list_of(fn(VRS))(fn(STU<u64>)));
extended_op_61[3] = power_entry(power_op_stxssp, "stxssp", NULL, list_of(fn(VRS))(fn(STU<u64>)));









	extended_op_63[0] = power_entry(power_op_fcmpu, "fcmpu", NULL, list_of(fn(setFPMode))(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[2] = power_entry(power_op_daddq,"daddq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_63[3] = power_entry(power_op_dquaq,"dquaq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_63[4] = power_entry(power_op_xsaddqp, "xsaddqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[5] = power_entry(power_op_xsrqpi, "xsrqpi", NULL, list_of(fn(R))(fn(VRT))(fn(VRB))(fn(RMC))(fn(EX)));
extended_op_63[8] = power_entry(power_op_fcpsgn,"fcpsgn",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_63[12] = power_entry(power_op_frsp, "frsp", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[14] = power_entry(power_op_fctiw, "fctiw", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[15] = power_entry(power_op_fctiwz, "fctiwz", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[18] = power_entry(power_op_fdiv, "fdiv", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_63[20] = power_entry(power_op_fsub, "fsub", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_63[21] = power_entry(power_op_fadd, "fadd", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(Rc)));
extended_op_63[22] = power_entry(power_op_fsqrt, "fsqrt", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[23] = power_entry(power_op_fsel, "fsel", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[24] = power_entry(power_op_fre,"fre",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[25] = power_entry(power_op_fmul, "fmul", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRC))(fn(Rc)));
extended_op_63[26] = power_entry(power_op_frsqrte, "frsqrte", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[28] = power_entry(power_op_fmsub, "fmsub", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[29] = power_entry(power_op_fmadd, "fmadd", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[30] = power_entry(power_op_fnmsub, "fnmsub", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[31] = power_entry(power_op_fnmadd, "fnmadd", NULL,
list_of(fn(setFPMode))(fn(FRT))(fn(FRA))(fn(FRB))(fn(FRC))(fn(Rc)));
extended_op_63[32] = power_entry(power_op_fcmpo, "fcmpo", NULL, list_of(fn(setFPMode))(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[34] = power_entry(power_op_dmulq,"dmulq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_63[35] = power_entry(power_op_drrndq,"drrndq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_63[36] = power_entry(power_op_xsmulqp, "xsmulqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[37] = power_entry(power_op_xsrqpxp,"xsrqpxp",NULL,list_of(fn(VRT))(fn(R))(fn(VRB))(fn(RMC)));
extended_op_63[38] = power_entry(power_op_mtfsb1, "mtfsb1", NULL, list_of(fn(setFPMode))(fn(BT))(fn(Rc)));
extended_op_63[40] = power_entry(power_op_fneg, "fneg", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[64] = power_entry(power_op_mcrfs, "mcrfs", NULL, list_of(fn(BF))(fn(BFA)));
extended_op_63[66] = power_entry(power_op_dscliq,"dscliq",NULL,list_of(fn(FRT))(fn(FRA))(fn(SH))(fn(RC)));
extended_op_63[67] = power_entry(power_op_dquaiq,"dquaiq",NULL,list_of(fn(FRT))(fn(TE))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_63[70] = power_entry(power_op_mtfsb0, "mtfsb0", NULL, list_of(fn(BT))(fn(Rc)));
extended_op_63[72] = power_entry(power_op_fmr, "fmr", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[98] = power_entry(power_op_dscriq,"dscriq",NULL,list_of(fn(FRT))(fn(FRA))(fn(SH))(fn(RC)));
extended_op_63[99] = power_entry(power_op_drintxq,"drintxq",NULL,list_of(fn(FRT))(fn(R))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_63[100] = power_entry(power_op_xscpsgnqp,"xscpsgnqp",NULL,list_of(fn(VRT))(fn(VRA))(fn(VRB)));
extended_op_63[128] = power_entry(power_op_ftdiv,"ftdiv",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[130] = power_entry(power_op_dcmpoq,"dcmpoq",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[132] = power_entry(power_op_xscmpoqp, "xscmpoqp", NULL, list_of(fn(BF))(fn(VRA))(fn(VRB)));
extended_op_63[134] = power_entry(power_op_mtfsfi, "mtfsfi", NULL, list_of(fn(BF))(fn(U))(fn(Rc)));
extended_op_63[136] = power_entry(power_op_fnabs, "fnabs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB)));
extended_op_63[142] = power_entry(power_op_fctiwu,"fctiwu",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[143] = power_entry(power_op_fctiwuz,"fctiwuz",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[160] = power_entry(power_op_ftsqrt,"ftsqrt",NULL,list_of(fn(BF))(fn(FRB)));
extended_op_63[162] = power_entry(power_op_dtstexq,"dtstexq",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[164] = power_entry(power_op_xscmpexpqp, "xscmpexpqp", NULL, list_of(fn(BF))(fn(VRA))(fn(VRB)));
extended_op_63[194] = power_entry(power_op_dtstdcq,"dtstdcq",NULL,list_of(fn(BF))(fn(FRA))(fn(DCM)));
extended_op_63[226] = power_entry(power_op_dtstdgq,"dtstdgq",NULL,list_of(fn(BF))(fn(FRA))(fn(DGM)));
extended_op_63[227] = power_entry(power_op_drintnq,"drintnq",NULL,list_of(fn(FRT))(fn(R))(fn(FRB))(fn(RMC))(fn(RC)));
extended_op_63[258] = power_entry(power_op_dctqpq,"dctqpq",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[264] = power_entry(power_op_fabs, "fabs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[290] = power_entry(power_op_dctfixq,"dctfixq",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[322] = power_entry(power_op_ddedpdq,"ddedpdq",NULL,list_of(fn(FRT))(fn(SP))(fn(FRB))(fn(RC)));
extended_op_63[354] = power_entry(power_op_dxexq,"dxexq",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[388] = power_entry(power_op_xsmaddqp, "xsmaddqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[392] = power_entry(power_op_frin,"frin",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[420] = power_entry(power_op_xsmsubqp, "xsmsubqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[424] = power_entry(power_op_friz,"friz",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[452] = power_entry(power_op_xsnmaddqp, "xsnmaddqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[456] = power_entry(power_op_frip,"frip",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[484] = power_entry(power_op_xsnmsubqp, "xsnmsubqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[488] = power_entry(power_op_frim,"frim",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[514] = power_entry(power_op_dsubq,"dsubq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_63[516] = power_entry(power_op_xssubqp, "xssubqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[546] = power_entry(power_op_ddivq,"ddivq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_63[548] = power_entry(power_op_xsdivqp,"xsdivqp",NULL,list_of(fn(VRT))(fn(VRA))(fn(VRB))(fn(RO)));
extended_op_63[642] = power_entry(power_op_dcmpuq,"dcmpuq",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[644] = power_entry(power_op_xscmpuqp, "xscmpuqp", NULL, list_of(fn(BF))(fn(VRA))(fn(VRB)));
extended_op_63[674] = power_entry(power_op_dtstsfq,"dtstsfq",NULL,list_of(fn(BF))(fn(FRA))(fn(FRB)));
extended_op_63[675] = power_entry(power_op_dtstsfiq,"dtstsfiq",NULL,list_of(fn(BF))(fn(UIM))(fn(FRB)));
extended_op_63[708] = power_entry(power_op_xststdcqp, "xststdcqp", NULL, list_of(fn(BF))(fn(DCMX))(fn(VRB)));
extended_op_63[711] = power_entry(power_op_mtfsf, "mtfsf", NULL, list_of(fn(setFPMode))(fn(FLM))(fn(FRB))(fn(Rc)));
extended_op_63[770] = power_entry(power_op_drdpq,"drdpq",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[802] = power_entry(power_op_dcffixq,"dcffixq",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[814] = power_entry(power_op_fctid, "fctid", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[815] = power_entry(power_op_fctidz, "fctidz", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
extended_op_63[834] = power_entry(power_op_denbcdq,"denbcdq",NULL,list_of(fn(FRT))(fn(S))(fn(FRB))(fn(RC)));
extended_op_63[838] = power_entry(power_op_fmrgow,"fmrgow",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB)));
//This two instructions share the same opcode
extended_op_63[846] = power_entry(power_op_fcfids,"fcfids",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[846] = power_entry(power_op_fcfid, "fcfid", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB))(fn(Rc)));
//
extended_op_63[866] = power_entry(power_op_diexq,"diexq",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB))(fn(RC)));
extended_op_63[868] = power_entry(power_op_xsiexpqp, "xsiexpqp", NULL, list_of(fn(VRT))(fn(VRA))(fn(VRB)));

//extended_op_63[583] = power_entry(power_op_extended, "extended", fn(extended_op_63_583), operandSpec()));
extended_op_63[942] = power_entry(power_op_fctidu,"fctidu",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[943] = power_entry(power_op_fctiduz,"fctiduz",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));
extended_op_63[966] = power_entry(power_op_fmrgew,"fmrgew",NULL,list_of(fn(FRT))(fn(FRA))(fn(FRB)));
extended_op_63[974] = power_entry(power_op_fcfidu,"fcfidu",NULL,list_of(fn(FRT))(fn(FRB))(fn(RC)));



//third level opcode included
//extended_op_63[804] = power_entry(power_op_extended, "extended", fn(extended_op_63_804), operandSpec()));
//extended_op_63[836] = power_entry(power_op_extended, "extended", fn(extended_op_63_836), operandSpec()));



extended_op_60_347[0] = power_entry(power_op_xsxexpdp, "xsxexpdp", NULL, list_of(fn(RT))(fn(XB)));
extended_op_60_347[1] = power_entry(power_op_xsxsigdp, "xsxsigdp", NULL, list_of(fn(RT))(fn(XB)));
extended_op_60_347[16] = power_entry(power_op_xscvdphp, "xscvhpdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_347[17] = power_entry(power_op_xscvhphp, "xscvdphp", NULL, list_of(fn(XT))(fn(XB)));


extended_op_60_475[0] = power_entry(power_op_xvxexpdp, "xvxexpdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[1] = power_entry(power_op_xvxsigdp, "xvxsigdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[7] = power_entry(power_op_xxbrh, "xxbrh", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[8] = power_entry(power_op_xvxexpsp, "xvxexpsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[9] = power_entry(power_op_xvxsigsp, "xvxsigsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[15] = power_entry(power_op_xxbrw, "xxbrw", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[23] = power_entry(power_op_xxbrd, "xxbrd", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[24] = power_entry(power_op_xvcvhpsp, "xvcvhpsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[25] = power_entry(power_op_xvcvsphp, "xvcvsphp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[31] = power_entry(power_op_xxbrq, "xxbrq", NULL, list_of(fn(XT))(fn(XB)));



//-------------------------
//Third level opcode follows:
/*  




extended_op_60_347[0] = power_entry(power_op_xsxexpdp, "xsxexpdp", NULL, list_of(fn(RT))(fn(XB)));
extended_op_60_347[1] = power_entry(power_op_xsxsigdp, "xsxsigdp", NULL, list_of(fn(RT))(fn(XB)));
extended_op_60_347[16] = power_entry(power_op_xscvdphp, "xscvhpdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_347[17] = power_entry(power_op_xscvhphp, "xscvdphp", NULL, list_of(fn(XT))(fn(XB)));


extended_op_60_475[0] = power_entry(power_op_xvxexpdp, "xvxexpdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[1] = power_entry(power_op_xvxsigdp, "xvxsigdp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[7] = power_entry(power_op_xxbrh, "xxbrh", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[8] = power_entry(power_op_xvxexpsp, "xvxexpsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[9] = power_entry(power_op_xvxsigsp, "xvxsigsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[15] = power_entry(power_op_xxbrw, "xxbrw", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[23] = power_entry(power_op_xxbrd, "xxbrd", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[24] = power_entry(power_op_xvcvhpsp, "xvcvhpsp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[25] = power_entry(power_op_xvcvsphp, "xvcvsphp", NULL, list_of(fn(XT))(fn(XB)));
extended_op_60_475[31] = power_entry(power_op_xxbrq, "xxbrq", NULL, list_of(fn(XT))(fn(XB)));

		extended_op_63_583[0] = power_entry(power_op_mffs, "mffs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(Rc)));
extended_op_63_583[1] = power_entry(power_op_mffsce, "mffsce", NULL, list_of(fn(setFPMode))(fn(FRT)));
extended_op_63_583[20] = power_entry(power_op_mffscdrn, "mffscdrn", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB)));
extended_op_63_583[21] = power_entry(power_op_mffscdrn, "mffscdrn", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(DRM)));
extended_op_63_583[22] = power_entry(power_op_mffscdrni, "mffscdrni", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(DRM)));
extended_op_63_583[23] = power_entry(power_op_mffscrn, "mffscrn", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB)));
extended_op_63_583[24] = power_entry(power_op_mffsl, "mffsl", NULL, list_of(fn(setFPMode))(fn(FRT)));
    
		extended_op_63_804[0] = power_entry(power_op_xsabsqp, "xsabsqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[2] = power_entry(power_op_xsxexpqp, "xsxexpqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[16] = power_entry(power_op_xsnegqp, "xsnegqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[18] = power_entry(power_op_xsxsigqp, "xsxsigqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[27] = power_entry(power_op_xssqrtqp, "xssqrtqp", list_of(fn(VRT))(fn(VRB))(fn(RO)));

    extended_op_63_836[1] = power_entry(power_op_xscvqpuwz, "xscvqpuwz", list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[2] = power_entry(power_op_xscvudqp, "xscvudqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[9] = power_entry(power_op_xscvqpswz, "xscvqpswz", list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[10] = power_entry(power_op_xscvsdqp, "xscvsdqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[17] = power_entry(power_op_xscvqpudz "xscvqpudz", list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[20] = power_entry(power_op_xscvqpdp, "xscvqpdp", list_of(fn(VRT))(fn(VRB))(fn(RO)));
extended_op_63_836[22] = power_entry(power_op_xscvdpqp, "xscvdpqp", list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[25] = power_entry(power_op_xscvqpsdz, "xscvqpsdz", list_of(fn(VRT))(fn(VRB)));
*/




        extended_op_63_583[0] = power_entry(power_op_mffs, "mffs", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(Rc)));
extended_op_63_583[1] = power_entry(power_op_mffsce, "mffsce", NULL, list_of(fn(setFPMode))(fn(FRT)));
extended_op_63_583[20] = power_entry(power_op_mffscdrn, "mffscdrn", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB)));
extended_op_63_583[21] = power_entry(power_op_mffscdrn, "mffscdrn", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(DRM)));
extended_op_63_583[22] = power_entry(power_op_mffscdrni, "mffscdrni", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(DRM)));
extended_op_63_583[23] = power_entry(power_op_mffscrn, "mffscrn", NULL, list_of(fn(setFPMode))(fn(FRT))(fn(FRB)));
extended_op_63_583[24] = power_entry(power_op_mffsl, "mffsl", NULL, list_of(fn(setFPMode))(fn(FRT)));
    
        extended_op_63_804[0] = power_entry(power_op_xsabsqp, "xsabsqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[2] = power_entry(power_op_xsxexpqp, "xsxexpqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[16] = power_entry(power_op_xsnegqp, "xsnegqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[18] = power_entry(power_op_xsxsigqp, "xsxsigqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_804[27] = power_entry(power_op_xssqrtqp, "xssqrtqp", NULL, list_of(fn(VRT))(fn(VRB))(fn(RO)));
extended_op_63_804[8] = power_entry(power_op_xsnabsqp, "xsnabsqp", NULL, list_of(fn(VRT))(fn(VRB)));

    extended_op_63_836[1] = power_entry(power_op_xscvqpuwz, "xscvqpuwz", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[2] = power_entry(power_op_xscvudqp, "xscvudqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[9] = power_entry(power_op_xscvqpswz, "xscvqpswz", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[10] = power_entry(power_op_xscvsdqp, "xscvsdqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[17] = power_entry(power_op_xscvqpudz, "xscvqpudz", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[20] = power_entry(power_op_xscvqpdp, "xscvqpdp", NULL, list_of(fn(VRT))(fn(VRB))(fn(RO)));
extended_op_63_836[22] = power_entry(power_op_xscvdpqp, "xscvdpqp", NULL, list_of(fn(VRT))(fn(VRB)));
extended_op_63_836[25] = power_entry(power_op_xscvqpsdz, "xscvqpsdz", NULL, list_of(fn(VRT))(fn(VRB)));
    });
}
